#ifndef REGLISTMODEL_H
#define REGLISTMODEL_H

#include <QAbstractItemModel>
#include <QModelIndex>
#include <QVariant>
#include <QMap>
#include "regtypes.h"


class RegObject;
class RegEntry;
class RegVar;


typedef QVector<RegEntry*> RegEntryList;


class RegListModel : public QAbstractItemModel
{
    Q_OBJECT
public:

    enum ColId {
        COL_INDEX = 0,
        COL_NAME,
        COL_TYPE,
        COL_COUNT,
        COL_MEM_ADDR,
        COL_MIN_VAL,
        COL_MAX_VAL,
        COL_DEF_VAL,
        COL_BASE,
        COL_FLAGS,
        COL_EXTFLAGS,
        COL_DESCR
    };

    explicit RegListModel(QObject *parent = nullptr);
    ~RegListModel();

    const RegEntryList* regEntryList() const;

    void setRegList(const RegEntryList& reglist);
    void addRegList(RegEntryList& reglist);

    bool hasEntryByRegIndex(reg_index_t index) const;
    RegEntry* entryByRegIndex(reg_index_t index) const;
    RegVar* varByRegIndex(reg_index_t index, reg_subindex_t subIndex) const;
    QModelIndex entryModelIndexByRegIndex(reg_index_t index) const;
    QModelIndex objectModelIndexByRegIndex(reg_index_t index, reg_subindex_t subIndex) const;

    QModelIndex entryModelIndex(const RegEntry* entry) const;
    QModelIndex entryModelIndexByModelIndex(const QModelIndex& index) const;
    RegEntry* entryByModelIndex(const QModelIndex& index) const;
    RegObject* objectByModelIndex(const QModelIndex& index) const;
    RegVar* varByModelIndex(const QModelIndex& index) const;

    bool addEntry(RegEntry* r);
    bool addSubObject(RegVar* r, const QModelIndex& parent);

    // QAbstractItemModel interface
public:
    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &child) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role) override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;
    bool removeRows(int row, int count, const QModelIndex& parent) override;
    bool moveRows(const QModelIndex& sourceParent, int sourceRow, int count, const QModelIndex& destinationParent, int destinationChild) override;


public slots:
    void entryAtIndexModified(const QModelIndex& index);

private:
    RegEntryList* m_reglist;

    //bool setDataEntry(const QModelIndex &index, const QVariant &value, int role);
    //bool setDataVar(const QModelIndex &index, const QVariant &value, int role);

    QVariant dataDisplayRole(const QModelIndex &index) const;
    QVariant dataEditRole(const QModelIndex &index) const;
    QVariant dataSizeHintRole(const QModelIndex &index) const;

    void fixCountVar(RegEntry* re, const QModelIndex& parent);
    void fixArrSubIndices(RegEntry* re, const QModelIndex& parent);
    void fixSortingAll();
    void fixSortingModelIndex(const QModelIndex& parent);
    void fixSortingEntries();
    void fixSortingVars(RegEntry* re);
    void fixBaseIndex(reg_index_t old_index, reg_index_t new_index);
    void fixBaseSubIndex(reg_index_t base_index, reg_subindex_t old_subIndex, reg_subindex_t new_subIndex);
};

#endif // REGLISTMODEL_H
