#include "reglistmodel.h"
#include "regentry.h"
#include "regobject.h"
#include "regvar.h"
#include <QSize>
#include <algorithm>
#include <iterator>
#include <QDebug>


static const char* col_names[] = {
    "Индекс",
    "Имя",
    "Тип",
    "Кол-во",
    "Память",
    "Минимум",
    "Максимум",
    "По-умолчанию",
    "Баз. знач.",
    "Флаги",
    "Доп. флаги",
    "Описание"
};

static const auto col_count = sizeof(col_names) / sizeof(col_names[0]);


static const int HEADER_WIDTH_SIZE_HINT = 100;
static const int HEADER_HEIGHT_SIZE_HINT = 20;

static const int ITEM_WIDTH_SIZE_HINT = 100;
static const int ITEM_HEIGHT_SIZE_HINT = 20;


RegListModel::RegListModel(QObject *parent)
    : QAbstractItemModel{parent}
{
    m_reglist = new RegEntryList();
}

RegListModel::~RegListModel()
{
    for(auto& entry: qAsConst(*m_reglist)){
        delete entry;
    }
    delete m_reglist;
}

const RegEntryList* RegListModel::regEntryList() const
{
    return m_reglist;
}

void RegListModel::setRegList(const RegEntryList& reglist)
{
    beginResetModel();

    for(auto& entry: qAsConst(*m_reglist)){
        delete entry;
    }

    m_reglist->clear();

    m_reglist->append(reglist);

    fixSortingAll();

    endResetModel();
}

void RegListModel::addRegList(RegEntryList& reglist)
{
    beginResetModel();

    for(auto it = reglist.begin(); it != reglist.end();){
        RegEntry* re = *it;
        if(std::find_if(m_reglist->begin(), m_reglist->end(), [&re](RegEntry* ref){
                        return ref->index() == re->index();}) != m_reglist->end()){
            ++ it;
            continue;
        }
        m_reglist->append(re);
        it = reglist.erase(it);
    }

    fixSortingAll();

    endResetModel();
}

bool RegListModel::hasEntryByRegIndex(reg_index_t index) const
{
    return entryByRegIndex(index) != nullptr;
}

RegEntry* RegListModel::entryByRegIndex(reg_index_t index) const
{
    auto it = std::find_if(m_reglist->begin(), m_reglist->end(),
                       [index](const RegEntry* ri){
                           return ri->index() == index;
    });
    if(it == m_reglist->end()) return nullptr;

    return *it;
}

RegVar* RegListModel::varByRegIndex(reg_index_t index, reg_subindex_t subIndex) const
{
    RegEntry* re = entryByRegIndex(index);

    if(re == nullptr) return nullptr;

    return re->varBySubIndex(subIndex);
}

QModelIndex RegListModel::entryModelIndexByRegIndex(reg_index_t index) const
{
    auto it = std::find_if(m_reglist->begin(), m_reglist->end(),
                       [index](const RegEntry* ri){
                           return ri->index() == index;
    });
    if(it == m_reglist->end()) return QModelIndex();

    return createIndex(std::distance(m_reglist->begin(), it), 0, static_cast<void*>(*it));
}

QModelIndex RegListModel::objectModelIndexByRegIndex(reg_index_t index, reg_subindex_t subIndex) const
{
    RegEntry* re = entryByRegIndex(index);
    if(re == nullptr) return QModelIndex();

    auto it = std::find_if(re->cbegin(), re->cend(),
                       [subIndex](const RegVar* ri){
                           return ri->subIndex() == subIndex;
    });
    if(it == re->cend()) return QModelIndex();

    return createIndex(std::distance(re->cbegin(), it), 0, static_cast<void*>(*it));
}

QModelIndex RegListModel::entryModelIndex(const RegEntry* entry) const
{
    auto it = std::find_if(m_reglist->begin(), m_reglist->end(),
                       [entry](const RegEntry* ri){
                           return ri->index() == entry->index();
    });

    if(it == m_reglist->end()) return QModelIndex();

    return createIndex(std::distance(m_reglist->begin(), it), 0, static_cast<void*>(*it));
}

QModelIndex RegListModel::entryModelIndexByModelIndex(const QModelIndex& index) const
{
    if(!index.isValid()) return QModelIndex();

    QModelIndex entry_index = index;
    QModelIndex parent_index = entry_index.parent();

    while(parent_index.isValid()){
        entry_index = parent_index;
        parent_index = entry_index.parent();
    }

    return entry_index;
}

RegEntry* RegListModel::entryByModelIndex(const QModelIndex& index) const
{
    QModelIndex entry_index = entryModelIndexByModelIndex(index);

    if(!entry_index.isValid()) return nullptr;

    int n = entry_index.row();

    if(n >= m_reglist->count()) return nullptr;

    return m_reglist->at(n);
}

RegObject* RegListModel::objectByModelIndex(const QModelIndex& index) const
{
    if(!index.isValid()) return nullptr;

    return static_cast<RegObject*>(index.internalPointer());
}

RegVar* RegListModel::varByModelIndex(const QModelIndex& index) const
{
    RegObject* ro = objectByModelIndex(index);
    if(ro == nullptr) return nullptr;

    if(ro->parent() == nullptr) return nullptr;

    return static_cast<RegVar*>(ro);
}

bool RegListModel::addEntry(RegEntry* r)
{
    reg_index_t index = r->index();

    auto it = std::find_if(m_reglist->begin(), m_reglist->end(),
                                [index](const RegEntry* ri){
        return ri->index() >= index;
    });

    // index already exist.
    if(it != m_reglist->end()){
        if((*it)->index() == index){
            return false;
        }
    }

    emit layoutAboutToBeChanged();

    m_reglist->insert(it, r);

    emit layoutChanged();

    return true;
}

bool RegListModel::addSubObject(RegVar* r, const QModelIndex& parent)
{
    if(!parent.isValid()) return false;

    RegEntry* re = static_cast<RegEntry*>(parent.internalPointer());
    if(re == nullptr) return false;
    if(re->hasVarBySubIndex(r->subIndex())) return false;

    const QList<QPersistentModelIndex> parents = {parent};

    reg_subindex_t last_subindex = re->lastVarSubIndex();

    emit layoutAboutToBeChanged(parents);

    bool res = re->add(r);

    if(res){
        if(r->subIndex() < last_subindex){
            fixSortingVars(re);
        }
        if(re->type() == ObjectType::ARR){
            fixArrSubIndices(re, parent);
        }
        emit layoutChanged(parents);
    }

    fixCountVar(re, parent);

    return res;
}

void RegListModel::entryAtIndexModified(const QModelIndex& index)
{
    QModelIndex topleft = createIndex(index.row(), 0);
    QModelIndex botright = createIndex(index.row(), col_count - 1);
    emit dataChanged(topleft, botright);
    fixSortingModelIndex(index.parent());
}

bool RegListModel::removeRows(int row, int count, const QModelIndex& parent)
{
    //qDebug() << "RegListModel::removeRows(" << row << ", " << count << ", " << parent << ")";

    if(count == 0) return false;

    if(parent.isValid()){
        RegEntry* re = static_cast<RegEntry*>(parent.internalPointer());
        if(re == nullptr) return false;

        if(row < 0 || row >= re->count()) return false;
        if(row + count > re->count()) return false;

        beginRemoveRows(parent, row, row + count - 1);

        while(count --){
            re->remove(row);
        }

        endRemoveRows();

        fixCountVar(re, parent);

    }else{
        if(row < 0 || row >= m_reglist->count()) return false;
        if(row + count > m_reglist->count()) return false;

        auto first = m_reglist->begin();
        std::advance(first, row);

        auto last = first;
        std::advance(last, count);

        beginRemoveRows(parent, row, row + count - 1);

        qDeleteAll(first, last);
        m_reglist->erase(first, last);

        endRemoveRows();
    }

    return true;
}

bool RegListModel::moveRows(const QModelIndex& sourceParent, int sourceRow, int count, const QModelIndex& destinationParent, int destinationChild)
{
    if(count == 0) return false;
    if(sourceParent != destinationParent) return false;
    if(destinationChild >= sourceRow && destinationChild < (sourceRow + count)) return false;

    // Var.
    if(sourceParent.isValid()){
        RegEntry* re = static_cast<RegEntry*>(sourceParent.internalPointer());
        if(re == nullptr) return false;
        if(sourceRow < 0 || sourceRow >= re->count()) return false;
        if(sourceRow + count - 1 >= re->count()) return false;
        if(destinationChild < 0 || destinationChild >= re->count()) return false;

        const QList<QPersistentModelIndex> parents = {sourceParent};
        emit layoutAboutToBeChanged(parents);

        auto beg = re->begin(); std::advance(beg, sourceRow);
        auto end = re->begin(); std::advance(end, sourceRow + count);
        auto to  = re->begin(); std::advance(to, destinationChild);

        auto iter_data_xchg = [](decltype(beg) it1, decltype(end) it2){
            RegVar* rv1 = *it1;
            RegVar* rv2 = *it2;

            reg_subindex_t si1 = rv1->subIndex();
            reg_subindex_t si2 = rv2->subIndex();
            rv1->setSubIndex(si2);
            rv2->setSubIndex(si1);

            *it1 = rv2;
            *it2 = rv1;
        };

        for(; beg != end; ++ beg, ++ to){
            iter_data_xchg(beg, to);
        }

        emit layoutChanged(parents);
    }
    // Entry.
    else{
        if(sourceRow < 0 || sourceRow >= m_reglist->count()) return false;
        if(sourceRow + count - 1 >= m_reglist->count()) return false;
        if(destinationChild < 0 || destinationChild >= m_reglist->count()) return false;

        const QList<QPersistentModelIndex> parents = {};
        emit layoutAboutToBeChanged(parents/*, QAbstractItemModel::VerticalSortHint*/);

        auto beg = m_reglist->begin(); std::advance(beg, sourceRow);
        auto end = m_reglist->begin(); std::advance(end, sourceRow + count);
        auto to  = m_reglist->begin(); std::advance(to, destinationChild);

        auto iter_data_xchg = [](decltype(beg) it1, decltype(end) it2){
            RegEntry* re1 = *it1;
            RegEntry* re2 = *it2;

            reg_index_t si1 = re1->index();
            reg_index_t si2 = re2->index();
            re1->setIndex(si2);
            re2->setIndex(si1);

            *it1 = re2;
            *it2 = re1;
        };

        for(; beg != end; ++ beg, ++ to){
            iter_data_xchg(beg, to);
        }

        emit layoutChanged(parents);
    }

    return true;
}

QModelIndex RegListModel::index(int row, int column, const QModelIndex &parent) const
{
    //qDebug() << "RegListModel::index(" << row << ", " << column << ", " << parent << ")";

    if(!parent.isValid()){
        if(column < 0 || column >= static_cast<int>(col_count)) return QModelIndex();
        if(row < 0 || row >= m_reglist->count()) return QModelIndex();

        RegEntry* re = m_reglist->at(row);
        if(re == nullptr) return QModelIndex();

        return createIndex(row, column, static_cast<void*>(re));
    }else{ // parent is valid.
        if(column < 0 || column >= static_cast<int>(col_count)) return QModelIndex();

        RegEntry* re = static_cast<RegEntry*>(parent.internalPointer());
        if(re == nullptr) return QModelIndex();

        if(row < 0 || row >= re->count()) return QModelIndex();

        return createIndex(row, column, static_cast<void*>(re->at(row)));
    }

    return QModelIndex();
}

QModelIndex RegListModel::parent(const QModelIndex &child) const
{
    //qDebug() << "RegListModel::parent(" << child << ")";

    if(!child.isValid()) return QModelIndex();

    RegObject* ro = static_cast<RegObject*>(child.internalPointer());
    if(ro == nullptr) return QModelIndex();

    RegObject* po = ro->parent();
    if(po == nullptr) return QModelIndex();

    RegEntry* re = static_cast<RegEntry*>(po);
    int child_index = m_reglist->indexOf(re);

    if(child_index == -1) return QModelIndex();

    return createIndex(child_index, 0, static_cast<void*>(re));
}

int RegListModel::rowCount(const QModelIndex &parent) const
{
    //qDebug() << "RegListModel::rowCount(" << parent << ")";

    if(!parent.isValid()){
        return m_reglist->count();
    }

    RegObject* ro = static_cast<RegObject*>(parent.internalPointer());
    if(ro == nullptr) return 0;

    if(ro->parent() != nullptr) return 0;

    RegEntry* re = static_cast<RegEntry*>(ro);

    return re->count();
}

int RegListModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
//    qDebug() << "RegListModel::columnCount(" << parent << ")";

//    if(!parent.isValid()){
//        return col_count;
//    }

//    RegObject* ro = static_cast<RegObject*>(parent.internalPointer());
//    if(ro == nullptr) return 0;

//    if(ro->type() != ObjectType::ARR && ro->type() != ObjectType::REC) return 0;

    return col_count;
}

QVariant RegListModel::dataDisplayRole(const QModelIndex& index) const
{
    if(!index.isValid()) return QVariant();
    if(index.column() >= static_cast<int>(col_count)) return QVariant();

    RegObject* ro = static_cast<RegObject*>(index.internalPointer());
    if(ro == nullptr) return false;

    RegEntry* pe = ro->parent();

    // entry.
    if(pe == nullptr){
        RegEntry* re = static_cast<RegEntry*>(ro);

        switch(static_cast<ColId>(index.column())){
        default:
            break;
        case COL_INDEX:
            return QStringLiteral("0x%1").arg(
                        QStringLiteral("%1").arg(
                            static_cast<unsigned int>(re->index()), 4, 16, QChar('0')
                            ).toUpper());
        case COL_NAME:
            return re->name();
        case COL_TYPE:
            return RegTypes::typeStr(re->type());
        case COL_COUNT:
            return static_cast<unsigned int>(re->count());
        case COL_MEM_ADDR:
        case COL_MIN_VAL:
        case COL_MAX_VAL:
        case COL_DEF_VAL:
        case COL_BASE:
        case COL_FLAGS:
        case COL_EXTFLAGS:
            break;
        case COL_DESCR:
            return re->description();
        }
    }
    // var.
    else{
        RegVar* rv = static_cast<RegVar*>(ro);

        switch(static_cast<ColId>(index.column())){
        default:
            break;
        case COL_INDEX:
            /*return QStringLiteral("0x%1").arg(
                        QStringLiteral("%1").arg(
                            static_cast<unsigned int>(rv->subIndex()), 2, 16, QChar('0')
                            ).toUpper());*/
            return static_cast<unsigned int>(rv->subIndex());
        case COL_NAME:
            return ro->name();
        case COL_TYPE:
            return RegTypes::dataTypeStr(rv->dataType());
        case COL_COUNT:
            if(pe->type() != ObjectType::ARR && !RegTypes::isMemory(rv->dataType())) return QVariant();
            if(rv->eflags() & RegEFlag::CO_COUNT) return QVariant();
            return QString("[%1]").arg(static_cast<unsigned int>(rv->count()));
        case COL_MEM_ADDR:
            return rv->memAddr();
        case COL_MIN_VAL:
            return rv->minValue();
        case COL_MAX_VAL:
            return rv->maxValue();
        case COL_DEF_VAL:
            return rv->defaultValue();
        case COL_BASE:{
            unsigned int base_index = rv->baseIndex();
            unsigned int base_subindex = rv->baseSubIndex();

            if(base_index == 0 && base_subindex == 0) return QStringLiteral("-");

            RegVar* brv = varByRegIndex(base_index, base_subindex);
            if(brv){
                QString base_name;
                RegEntry* bre = brv->parent();
                if(bre){
                    base_name = QStringLiteral("%1.").arg(bre->name());
                }
                base_name += brv->name();
                return base_name;
            }
            return QString("0x%1.%2")
                    .arg(base_index, 4, 16, QChar('0'))
                    .arg(base_subindex, 2, 16, QChar('0'));
        }
        case COL_FLAGS:{
            return RegTypes::getNames(rv->flags(), RegTypes::flagName).join(" | ");
        }
        case COL_EXTFLAGS:{
            return RegTypes::getNames(rv->eflags(), RegTypes::eflagName).join(" | ");
        }
        case COL_DESCR:
            return rv->description();
        }
    }

    return QVariant();
}

QVariant RegListModel::dataEditRole(const QModelIndex& index) const
{
    if(!index.isValid()) return QVariant();
    if(index.column() >= static_cast<int>(col_count)) return QVariant();

    RegObject* ro = static_cast<RegObject*>(index.internalPointer());
    if(ro == nullptr) return false;

    RegEntry* pe = ro->parent();

    // entry.
    if(pe == nullptr){
        RegEntry* re = static_cast<RegEntry*>(ro);

        switch(static_cast<ColId>(index.column())){
        default:
            break;
        case COL_INDEX:
            return static_cast<unsigned int>(re->index());
        case COL_NAME:
            return re->name();
        case COL_TYPE:
            return static_cast<unsigned int>(re->type());
        case COL_COUNT:
            return static_cast<unsigned int>(re->count());
        case COL_MEM_ADDR:
        case COL_MIN_VAL:
        case COL_MAX_VAL:
        case COL_DEF_VAL:
        case COL_BASE:
        case COL_FLAGS:
        case COL_EXTFLAGS:
            break;
        case COL_DESCR:
            return re->description();
        }
    }
    // var.
    else{
        RegVar* rv = static_cast<RegVar*>(ro);

        switch(static_cast<ColId>(index.column())){
        default:
            break;
        case COL_INDEX:
            return rv->subIndex();
        case COL_NAME:
            return ro->name();
        case COL_TYPE:
            return static_cast<unsigned int>(rv->dataType());
        case COL_COUNT:
            if(pe->type() != ObjectType::ARR && !RegTypes::isMemory(rv->dataType())) return QVariant();
            return static_cast<unsigned int>(rv->count());
        case COL_MEM_ADDR:
            return rv->memAddr();
        case COL_MIN_VAL:
            return rv->minValue();
        case COL_MAX_VAL:
            return rv->maxValue();
        case COL_DEF_VAL:
            return rv->defaultValue();
        case COL_BASE:{
            unsigned int base_index = rv->baseIndex();
            unsigned int base_subindex = rv->baseSubIndex();
            return static_cast<unsigned int>((base_index << 8) | base_subindex);
        }
        case COL_FLAGS:
            return static_cast<unsigned int>(rv->flags());
        case COL_EXTFLAGS:
            return static_cast<unsigned int>(rv->eflags());
        case COL_DESCR:
            return rv->description();
        }
    }

    return QVariant();
}

QVariant RegListModel::dataSizeHintRole(const QModelIndex& index) const
{
    if(!index.isValid()) return QVariant();

    QSize size = QSize(ITEM_WIDTH_SIZE_HINT, ITEM_HEIGHT_SIZE_HINT);

    return size;
}

void RegListModel::fixCountVar(RegEntry* re, const QModelIndex& parent)
{
    if(re->count() == 0) return;

    int count = re->count() - 1;
    // calc count of sub-entries for array;
    if(re->type() == ObjectType::ARR){
        std::for_each(re->begin(), re->end(), [&](RegVar* rv){
            if(rv->count() > 1) count += rv->count() - 1;
        });
    }

    int row = 0;
    std::for_each(re->begin(), re->end(), [&](RegVar* rv){
        if(rv->eflags() & static_cast<uint>(RegEFlag::CO_COUNT)){
            rv->setDefaultValue(count);

            QModelIndex count_index = index(row, COL_COUNT, parent);
            emit dataChanged(count_index, count_index);

            row ++;
        }
    });
}

void RegListModel::fixArrSubIndices(RegEntry* re, const QModelIndex& parent)
{
    if(re->count() == 0) return;

    reg_subindex_t oldSubIndex;
    reg_subindex_t subIndex = 0;
    int row = 0;
    for(auto it = re->begin(); it != re->end(); ++ it){
        oldSubIndex = (*it)->subIndex();
        if(oldSubIndex != subIndex){
            (*it)->setSubIndex(subIndex);

            QModelIndex subIndex_index = index(row, COL_INDEX, parent);
            emit dataChanged(subIndex_index, subIndex_index);

            fixBaseSubIndex(re->index(), oldSubIndex, subIndex);
        }
        subIndex += static_cast<reg_subindex_t>(qMax(1U, (*it)->count()));
        row ++;
    }
}

void RegListModel::fixSortingAll()
{
    fixSortingEntries();
    std::for_each(m_reglist->begin(), m_reglist->end(), [this](RegEntry* re){
        fixSortingVars(re);
    });
}

void RegListModel::fixSortingModelIndex(const QModelIndex& parent)
{
    const auto parents_list = QList<QPersistentModelIndex>({parent});

    emit layoutAboutToBeChanged(parents_list, QAbstractItemModel::VerticalSortHint);
    // Entry.
    if(!parent.isValid()){
        fixSortingEntries();
    }
    // Var.
    else{
        RegEntry* re = static_cast<RegEntry*>(parent.internalPointer());
        fixSortingVars(re);
    }
    emit layoutChanged(parents_list, QAbstractItemModel::VerticalSortHint);
}

void RegListModel::fixSortingEntries()
{
    std::sort(m_reglist->begin(), m_reglist->end(), [](RegEntry* rl, RegEntry* rr){
        return rl->index() < rr->index();
    });
}

void RegListModel::fixSortingVars(RegEntry* re)
{
    std::sort(re->begin(), re->end(), [](RegVar* rl, RegVar* rr){
        return rl->subIndex() < rr->subIndex();
    });
}

/*void RegListModel::fixBaseIndexSubIndex(reg_index_t old_index, reg_subindex_t old_subIndex, reg_index_t new_index, reg_subindex_t new_subIndex)
{
    bool index_changed = old_index != new_index;
    bool subindex_changed = old_subIndex != new_subIndex;

    if(!index_changed || !subindex_changed) return;

    int child_row = 0;

    std::for_each(m_reglist->begin(), m_reglist->end(), [&](RegEntry* re){
        child_row = 0;
        QModelIndex parent_index = QModelIndex();
        std::for_each(re->begin(), re->end(), [&](RegVar* rv){
            if((index_changed && (rv->baseIndex() == old_index)) ||
                (subindex_changed && (rv->baseSubIndex() == old_subIndex))){

                rv->setBaseIndex(new_index);
                rv->setBaseSubIndex(new_subIndex);

                if(!parent_index.isValid()){
                    parent_index = entryModelIndex(re);
                }
                QModelIndex child_index = index(child_row, COL_BASE, parent_index);
                emit dataChanged(child_index, child_index);
            }
            child_row ++;
        });
    });
}*/

void RegListModel::fixBaseIndex(reg_index_t old_index, reg_index_t new_index)
{
    if(old_index == new_index) return;

    int child_row = 0;

    std::for_each(m_reglist->begin(), m_reglist->end(), [&](RegEntry* re){
        child_row = 0;
        QModelIndex parent_index = QModelIndex();
        std::for_each(re->begin(), re->end(), [&](RegVar* rv){
            if(rv->baseIndex() == old_index){

                rv->setBaseIndex(new_index);

                if(!parent_index.isValid()){
                    parent_index = entryModelIndex(re);
                }
                QModelIndex child_index = index(child_row, COL_BASE, parent_index);
                emit dataChanged(child_index, child_index);
            }
            child_row ++;
        });
    });
}

void RegListModel::fixBaseSubIndex(reg_index_t base_index, reg_subindex_t old_subIndex, reg_subindex_t new_subIndex)
{
    if(old_subIndex == new_subIndex) return;

    int child_row = 0;

    std::for_each(m_reglist->begin(), m_reglist->end(), [&](RegEntry* re){
        child_row = 0;
        QModelIndex parent_index = QModelIndex();
        std::for_each(re->begin(), re->end(), [&](RegVar* rv){
            if((rv->baseIndex() == base_index) && (rv->baseSubIndex() == old_subIndex)){

                rv->setBaseSubIndex(new_subIndex);

                if(!parent_index.isValid()){
                    parent_index = entryModelIndex(re);
                }
                QModelIndex child_index = index(child_row, COL_BASE, parent_index);
                emit dataChanged(child_index, child_index);
            }
            child_row ++;
        });
    });
}

QVariant RegListModel::data(const QModelIndex &index, int role) const
{
    //qDebug() << "RegListModel::data(" << index <<", " << static_cast<Qt::ItemDataRole>(role) << ")";

    switch(role){
    default:
        break;
    case Qt::DisplayRole:
    case Qt::ToolTipRole:
        return dataDisplayRole(index);
    case Qt::EditRole:
        return dataEditRole(index);
    case Qt::SizeHintRole:
        return dataSizeHintRole(index);
    }

    return QVariant();
}

bool RegListModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    //qDebug() << "RegListModel::setData(" << index <<", " << value << ", " << static_cast<Qt::ItemDataRole>(role) << ")";

    if(role != Qt::EditRole) return false;

    if(!index.isValid()) return false;
    if(index.column() >= static_cast<int>(col_count)) return false;

    RegObject* ro = static_cast<RegObject*>(index.internalPointer());
    if(ro == nullptr) return false;

    RegEntry* pe = ro->parent();

    // entry.
    if(pe == nullptr){
        RegEntry* re = static_cast<RegEntry*>(ro);

        switch(static_cast<ColId>(index.column())){
        case COL_INDEX:{
            reg_index_t newIndex = value.toUInt();
            if(newIndex != re->index()){
                if(hasEntryByRegIndex(newIndex)){
                    return false;
                }
                reg_index_t oldIndex = re->index();

                re->setIndex(newIndex);

                fixSortingModelIndex(index.parent());
                fixBaseIndex(oldIndex, newIndex);
            }
        }break;
        case COL_NAME:{
            QString newName = value.toString().trimmed();
            if(newName.isEmpty()) return false;
            ro->setName(newName);
        }break;
        case COL_TYPE:
            re->setType(static_cast<ObjectType>(value.toUInt()));
            break;
        case COL_COUNT:
        case COL_MEM_ADDR:
        case COL_MIN_VAL:
        case COL_MAX_VAL:
        case COL_DEF_VAL:
        case COL_BASE:
        case COL_FLAGS:
        case COL_EXTFLAGS:
            return false;
        case COL_DESCR:
            ro->setDescription(value.toString());
            break;
        }
    }
    // var.
    else{
        RegVar* rv = static_cast<RegVar*>(ro);

        switch(static_cast<ColId>(index.column())){
        case COL_INDEX:{
            reg_index_t newSubIndex = value.toUInt();
            if(newSubIndex != rv->subIndex()){
                if(pe->hasVarBySubIndex(newSubIndex)){
                    return false;
                }
                reg_subindex_t oldSubIndex = rv->subIndex();

                rv->setSubIndex(newSubIndex);
                fixBaseSubIndex(pe->index(), oldSubIndex, newSubIndex);

                fixSortingModelIndex(index.parent());

                if(pe->type() == ObjectType::ARR){
                    fixArrSubIndices(pe, index.parent());
                }
            }
        }break;
        case COL_NAME:{
            QString newName = value.toString().trimmed();
            if(newName.isEmpty()) return false;
            ro->setName(newName);
        }break;
        case COL_TYPE:
            rv->setDataType(static_cast<DataType>(value.toUInt()));
            if(pe->type() == ObjectType::ARR && rv->subIndex() != 0){
                auto parent_index = index.parent();
                for(int i = 0; i < pe->count(); i ++){
                    RegVar* v = pe->at(i);
                    if(v == rv || v->subIndex() == 0) continue;

                    v->setDataType(rv->dataType());

                    auto vIndex = RegListModel::index(i, 0, parent_index);
                    emit dataChanged(vIndex, vIndex);
                }
            }
            break;
        case COL_COUNT:
            if(pe->type() != ObjectType::ARR && !RegTypes::isMemory(rv->dataType())) return false;
            rv->setCount(value.toUInt());
            fixCountVar(pe, index.parent());
            if(pe->type() == ObjectType::ARR){
                fixArrSubIndices(pe, index.parent());
            }
            break;
        case COL_MEM_ADDR:
            rv->setMemAddr(value.toString().trimmed());
            break;
        case COL_MIN_VAL:
            rv->setMinValue(value.toString().trimmed());
            break;
        case COL_MAX_VAL:
            rv->setMaxValue(value.toString().trimmed());
            break;
        case COL_DEF_VAL:
            rv->setDefaultValue(value.toString().trimmed());
            break;
        case COL_BASE:{
            unsigned int base = value.toUInt();
            rv->setBaseIndex(base >> 8);
            rv->setBaseSubIndex(base & 0xff);
        }break;
        case COL_FLAGS:
            rv->setFlags(static_cast<reg_flags_t>(value.toUInt()));
            break;
        case COL_EXTFLAGS:
            rv->setEFlags(static_cast<reg_flags_t>(value.toUInt()));
            break;
        case COL_DESCR:
            ro->setDescription(value.toString());
            break;
        }
    }

    emit dataChanged(index, index); // QVector<int>{Qt::EditRole, Qt::DisplayRole}

    return true;
}

QVariant RegListModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    //qDebug() << "RegListModel::headerData(" << section <<", " << orientation <<", " << static_cast<Qt::ItemDataRole>(role) << ")";

    if(orientation != Qt::Horizontal){
        return QAbstractItemModel::headerData(section, orientation, role);
    }
    if(section < 0 || section >= static_cast<int>(col_count)){
        return QAbstractItemModel::headerData(section, orientation, role);
    }

    switch(role){
    default:
        return QAbstractItemModel::headerData(section, orientation, role);
    case Qt::DisplayRole:
    case Qt::ToolTipRole:
        break;
    case Qt::TextAlignmentRole:
        return static_cast<int>(Qt::AlignHCenter | Qt::AlignVCenter);
    case Qt::SizeHintRole:
        return QSize(HEADER_WIDTH_SIZE_HINT, HEADER_HEIGHT_SIZE_HINT);
    }

    return QVariant(col_names[section]);
}

Qt::ItemFlags RegListModel::flags(const QModelIndex& index) const
{
    Qt::ItemFlags flags = QAbstractItemModel::flags(index);

    if(!index.isValid()) return flags;

    RegObject* ro = static_cast<RegObject*>(index.internalPointer());
    if(ro == nullptr) return flags;

    RegEntry* pe = ro->parent();

    // entry.
    if(pe == nullptr){
        //RegEntry* re = static_cast<RegEntry*>(ro);

        switch(static_cast<ColId>(index.column())){
        case COL_INDEX:
            flags |= Qt::ItemIsEditable;
            break;
        case COL_NAME:
            flags |= Qt::ItemIsEditable;
            break;
        case COL_TYPE:
            flags |= Qt::ItemIsEditable;
            break;
        case COL_COUNT:
            break;
        case COL_MEM_ADDR:
            break;
        case COL_MIN_VAL:
            break;
        case COL_MAX_VAL:
            break;
        case COL_DEF_VAL:
            break;
        case COL_BASE:
            break;
        case COL_FLAGS:
            break;
        case COL_EXTFLAGS:
            break;
        case COL_DESCR:
            break;
        default:
            break;
        }
    }
    // var.
    else{
        RegVar* rv = static_cast<RegVar*>(ro);

        flags |= Qt::ItemNeverHasChildren;

        switch(static_cast<ColId>(index.column())){
        case COL_INDEX:
            flags |= Qt::ItemIsEditable;
            break;
        case COL_NAME:
            flags |= Qt::ItemIsEditable;
            break;
        case COL_TYPE:
            flags |= Qt::ItemIsEditable;
            break;
        case COL_COUNT:
            if((pe->type() == ObjectType::ARR) &&
               (rv->eflags() & RegEFlag::CO_COUNT) == 0) flags |= Qt::ItemIsEditable;
            if(RegTypes::isMemory(rv->dataType())) flags |= Qt::ItemIsEditable;
            break;
        case COL_MEM_ADDR:
            flags |= Qt::ItemIsEditable;
            break;
        case COL_MIN_VAL:
            flags |= Qt::ItemIsEditable;
            break;
        case COL_MAX_VAL:
            flags |= Qt::ItemIsEditable;
            break;
        case COL_DEF_VAL:
            flags |= Qt::ItemIsEditable;
            break;
        case COL_BASE:
            flags |= Qt::ItemIsEditable;
            break;
        case COL_FLAGS:
            flags |= Qt::ItemIsEditable;
            break;
        case COL_EXTFLAGS:
            flags |= Qt::ItemIsEditable;
            break;
        case COL_DESCR:
            flags |= Qt::ItemIsEditable;
            break;
        default:
            break;
        }
    }

    return flags;
}

