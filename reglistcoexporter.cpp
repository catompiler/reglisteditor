#include "reglistcoexporter.h"
#include <algorithm>
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QIODevice>
#include <QTextStream>
#include <QSet>
#include "regentry.h"
#include "regvar.h"
#include "regtypes.h"
#include "regutils.h"
#include <QDebug>


#define DEBUG_OUTPUT 0


RegListCoExporter::RegListCoExporter(QObject *parent)
    : RegListExporter{parent}
{
}

RegListCoExporter::~RegListCoExporter()
{
}


bool RegListCoExporter::doExport(const QString& filename, const RegEntryList* regentrylist)
{
    QFileInfo fi(filename);
    QDir dir = fi.dir();

    QString baseName = fi.baseName();
    QString co_name_c = baseName + ".c";
    QString co_name_h = baseName + ".h";

    if(m_dataName.isEmpty()) m_dataName = baseName;

    QString co_filename_c = dir.filePath(co_name_c);
    QString co_filename_h = dir.filePath(co_name_h);

    if(!exportCoH(co_filename_h, regentrylist)) return false;
    if(!exportCoC(co_filename_c, regentrylist)) return false;

    return true;
}

bool RegListCoExporter::exportCoH(const QString& filename, const RegEntryList* regentrylist)
{
    QFile file(filename);

    if(!file.open(QIODevice::WriteOnly)) return false;

    QTextStream out_stream(&file);

    QFileInfo fileinfo(file);

#if defined(DEBUG_OUTPUT) && DEBUG_OUTPUT == 1
    QDebug out = qDebug();
#else
    QTextStream& out = out_stream;
#endif

    QString co_h_name = fileinfo.fileName().replace(QChar('.'), QChar('_'));
    QString header_guard_name = co_h_name.toUpper();

    // header
    out << "#ifndef " << header_guard_name << "\n"
        << "#define " << header_guard_name << "\n"
        << "\n\n"
        << "// THIS FILE IS GENERATED AUTOMATICALLY!\n"
        << "// DO NOT EDIT THIS FILE!\n"
        << "\n\n";

    bool res = true;

    res &= writeCOCounters(out_stream, regentrylist);
    res &= writeCOArraySizes(out_stream, regentrylist);
    res &= writeCOexternOd(out_stream);
    res &= writeCOShortcuts(out_stream, regentrylist);
    res &= writeCOShortcutsWithNames(out_stream, regentrylist);

    // footer
    out << "\n#endif /* " << header_guard_name << " */\n";

    // Files closed in dtors, but, i want to do it manually.
    file.close();

    return res;
}

bool RegListCoExporter::writeCOCounters(QTextStream& out_stream, const RegEntryList* regentrylist)
{
#if defined(DEBUG_OUTPUT) && DEBUG_OUTPUT == 1
    Q_UNUSED(out_stream)
    QDebug out = qDebug();
#else
    QTextStream& out = out_stream;
#endif

    QMap<uint, QPair<uint, QString>> cnts;

    cnts[RegEFlag::CON_CNT_NMT] = qMakePair(0, QStringLiteral("OD_CNT_NMT"));
    cnts[RegEFlag::CON_CNT_EM] = qMakePair(0, QStringLiteral("OD_CNT_EM"));
    cnts[RegEFlag::CON_CNT_SYNC] = qMakePair(0, QStringLiteral("OD_CNT_SYNC"));
    cnts[RegEFlag::CON_CNT_SYNC_PROD] = qMakePair(0, QStringLiteral("OD_CNT_SYNC_PROD"));
    cnts[RegEFlag::CON_CNT_STORAGE] = qMakePair(0, QStringLiteral("OD_CNT_STORAGE"));
    cnts[RegEFlag::CON_CNT_TIME] = qMakePair(0, QStringLiteral("OD_CNT_TIME"));
    cnts[RegEFlag::CON_CNT_EM_PROD] = qMakePair(0, QStringLiteral("OD_CNT_EM_PROD"));
    cnts[RegEFlag::CON_CNT_HB_CONS] = qMakePair(0, QStringLiteral("OD_CNT_HB_CONS"));
    cnts[RegEFlag::CON_CNT_HB_PROD] = qMakePair(0, QStringLiteral("OD_CNT_HB_PROD"));
    cnts[RegEFlag::CON_CNT_SDO_SRV] = qMakePair(0, QStringLiteral("OD_CNT_SDO_SRV"));
    cnts[RegEFlag::CON_CNT_SDO_CLI] = qMakePair(0, QStringLiteral("OD_CNT_SDO_CLI"));
    cnts[RegEFlag::CON_CNT_RPDO] = qMakePair(0, QStringLiteral("OD_CNT_RPDO"));
    cnts[RegEFlag::CON_CNT_TPDO] = qMakePair(0, QStringLiteral("OD_CNT_TPDO"));

    QMap<uint, uint> cnts_var;
    for(auto it = cnts.begin(); it != cnts.end(); ++ it){
        cnts_var[it.key()] = 0;
    }

    for(auto reit = regentrylist->cbegin(); reit != regentrylist->cend(); ++ reit){
        const RegEntry* re = *reit;

        for(auto rvit = re->cbegin(); rvit != re->cend(); ++ rvit){
            const RegVar* rv = *rvit;

            for(auto it = cnts_var.begin(); it != cnts_var.end(); ++ it){
                if(rv->eflags() & it.key()){
                    it.value() ++;
                }
            }
        }

        for(auto it = cnts_var.begin(); it != cnts_var.end(); ++ it){
            if(it.value() != 0) cnts[it.key()].first ++;
            it.value() = 0;
        }
    }

    out << "// Counters of OD objects\n";

    for(auto it = cnts.begin(); it != cnts.end(); ++ it){
        out << QStringLiteral("#define %1 %2")
               .arg(it.value().second)
               .arg(it.value().first)
            << "\n";
    }

    out << "\n\n";

    return true;
}

bool RegListCoExporter::writeCOArraySizes(QTextStream& out_stream, const RegEntryList* regentrylist)
{
#if defined(DEBUG_OUTPUT) && DEBUG_OUTPUT == 1
    Q_UNUSED(out_stream)
    QDebug out = qDebug();
#else
    QTextStream& out = out_stream;
#endif

    out << "// Sizes of OD arrays\n";

    for(auto reit = regentrylist->cbegin(); reit != regentrylist->cend(); ++ reit){
        const RegEntry* re = *reit;

        if(re->type() == ObjectType::ARR){

            uint size = 0;

            for(auto rvit = re->cbegin(); rvit != re->cend(); ++ rvit){
                const RegVar* rv = *rvit;
                if((rv->eflags() & RegEFlag::CO_COUNT) == 0){
                    size = qMax(rv->count(), size);
                }
            }

            out << QStringLiteral("#define OD_CNT_ARR_%1 %2")
                   .arg(static_cast<uint>(re->index()), 0, 16)
                   .arg(size)
                << "\n";
        }
    }

    out << "\n\n";

    return true;
}

bool RegListCoExporter::writeCOexternOd(QTextStream& out_stream)
{
#if defined(DEBUG_OUTPUT) && DEBUG_OUTPUT == 1
    Q_UNUSED(out_stream)
    QDebug out = qDebug();
#else
    QTextStream& out = out_stream;
#endif

    out << "#ifndef OD_ATTR_OD\n"
        << "#define OD_ATTR_OD\n"
        << "#endif\n"
        << "extern OD_ATTR_OD OD_t *OD;\n";

    out << "\n\n";

    return true;
}

bool RegListCoExporter::writeCOShortcuts(QTextStream& out_stream, const RegEntryList* regentrylist)
{
#if defined(DEBUG_OUTPUT) && DEBUG_OUTPUT == 1
    Q_UNUSED(out_stream)
    QDebug out = qDebug();
#else
    QTextStream& out = out_stream;
#endif

    out << "// Object dictionary entries - shortcuts\n";

    uint list_index = 0;

    for(auto reit = regentrylist->cbegin(); reit != regentrylist->cend(); ++ reit){
        const RegEntry* re = *reit;

        out << QStringLiteral("#define OD_ENTRY_H%1 &OD->list[%2]")
               .arg(static_cast<uint>(re->index()), 0, 16)
               .arg(list_index)
            << "\n";
    }

    out << "\n\n";

    return true;
}

bool RegListCoExporter::writeCOShortcutsWithNames(QTextStream& out_stream, const RegEntryList* regentrylist)
{
#if defined(DEBUG_OUTPUT) && DEBUG_OUTPUT == 1
    Q_UNUSED(out_stream)
    QDebug out = qDebug();
#else
    QTextStream& out = out_stream;
#endif

    out << "// Object dictionary entries - shortcuts with names\n";

    uint list_index = 0;

    for(auto reit = regentrylist->cbegin(); reit != regentrylist->cend(); ++ reit){
        const RegEntry* re = *reit;

        out << QStringLiteral("#define OD_ENTRY_H%1_%2 &OD->list[%3]")
               .arg(static_cast<uint>(re->index()), 0, 16)
               .arg(RegUtils::makeName(re->name(), m_syntaxType))
               .arg(list_index)
            << "\n";

        list_index ++;
    }

    out << "\n\n";

    return true;
}

bool RegListCoExporter::exportCoC(const QString& filename, const RegEntryList* regentrylist)
{
    QFile file(filename);

    if(!file.open(QIODevice::WriteOnly)) return false;

    QTextStream out_stream(&file);

    QFileInfo fileinfo(file);

#if defined(DEBUG_OUTPUT) && DEBUG_OUTPUT == 1
    Q_UNUSED(out_stream)
    QDebug out = qDebug();
#else
    QTextStream& out = out_stream;
#endif

    QString co_h_name = fileinfo.fileName().replace(".c", ".h");

    // header
    out << QStringLiteral("#include \"%1\"\n").arg(co_h_name)
        << "\n\n"
        << "#define OD_DEFINITION\n"
        << "#include \"301/CO_ODinterface.h\"\n"
        << "#include \"OD.h\"\n"
        << "\n"
        << "#if CO_VERSION_MAJOR < 4\n"
        << "#error This Object dictionary is compatible with CANopenNode V4.0 and above!\n"
        << "#endif\n"
        << "\n\n";

    bool res = true;

    res &= writeAllOdObjConstDefs(out_stream, regentrylist);
    res &= writeOd(out_stream, regentrylist);


    // footer

    // Files closed in dtors, but, i want to do it manually.
    file.close();

    return res;
}

bool RegListCoExporter::writeAllOdObjConstDefs(QTextStream& out_stream, const RegEntryList* regentrylist)
{
#if defined(DEBUG_OUTPUT) && DEBUG_OUTPUT == 1
    Q_UNUSED(out_stream)
    QDebug out = qDebug();
#else
    QTextStream& out = out_stream;
#endif

    out << "// All OD objects (constant definitions)\n";

    out << "typedef struct {\n";

    for(auto reit = regentrylist->cbegin(); reit != regentrylist->cend(); ++ reit){
        const RegEntry* re = *reit;

        out << QStringLiteral("%1 %2;")
               .arg(getOdEntryTypeStr(re->type()), getOdEntryFieldDecl(re))
            << "\n";
    }

    out << "} ODObjs_t;\n\n";

    out << "static CO_PROGMEM ODObjs_t ODObjs = {\n";

    bool firstEntry = true;

    for(auto reit = regentrylist->cbegin(); reit != regentrylist->cend(); ++ reit){
        const RegEntry* re = *reit;

        if(!firstEntry){
            out << ",\n";
        }
        firstEntry = false;

        out << QStringLiteral(".%1 = {")
               .arg(getOdEntryFieldName(re))
            << "\n";

        if(!writeOdObjConstDef(out_stream, re)) return false;

        out << "}";
    }

    out << "\n};\n";

    return true;
}

bool RegListCoExporter::writeOdObjConstDef(QTextStream& out_stream, const RegEntry* re)
{
    switch(re->type()){
    case ObjectType::VAR:
        if(!writeOdVarConstDef(out_stream, re)) return false;
        break;
    case ObjectType::ARR:
        if(!writeOdArrConstDef(out_stream, re)) return false;
        break;
    case ObjectType::REC:
        if(!writeOdRecConstDef(out_stream, re)) return false;
        break;
    }

    return true;
}

bool RegListCoExporter::writeOdVarConstDef(QTextStream& out_stream, const RegEntry* re)
{
#if defined(DEBUG_OUTPUT) && DEBUG_OUTPUT == 1
    Q_UNUSED(out_stream)
    QDebug out = qDebug();
#else
    QTextStream& out = out_stream;
#endif

    for(auto rvit = re->cbegin(); rvit != re->cend(); ++ rvit){
        const RegVar* rv = *rvit;

        QString data_str;
        if(re->type() != ObjectType::ARR || rv->subIndex() == 0){
            data_str = RegUtils::getVarMem(m_dataName, re, rv, 0, m_entryNameMap, m_varNameMap, m_syntaxType);
        }else{
            data_str = RegUtils::getArrMem(m_dataName, re, rv, 0, m_entryNameMap, m_varNameMap, m_syntaxType);
        }

        uint dataSize = RegTypes::sizeBytes(rv->dataType());

        co_attributes_t attrs = RegTypes::eflagsToCoAttributes(rv->eflags());
        if(dataSize > 1) attrs |= COAttribute::MB;
        QString attrs_str = RegTypes::getNames(attrs, RegTypes::coAttributeFullName).join(" | ");

        out << QStringLiteral("    .dataOrig = &%1,")
               .arg(data_str)
            << "\n";

        out << QStringLiteral("    .attribute = %1,")
               .arg(attrs_str)
            << "\n";

        out << QStringLiteral("    .dataLength = %1")
               .arg(dataSize)
            << "\n";
    }

    return true;
}

bool RegListCoExporter::writeOdRecConstDef(QTextStream& out_stream, const RegEntry* re)
{
#if defined(DEBUG_OUTPUT) && DEBUG_OUTPUT == 1
    Q_UNUSED(out_stream)
    QDebug out = qDebug();
#else
    QTextStream& out = out_stream;
#endif

    bool firstVar = true;

    for(auto rvit = re->cbegin(); rvit != re->cend(); ++ rvit){
        const RegVar* rv = *rvit;

        if(!firstVar){
            out << ",\n";
        }
        firstVar = false;

        out << "    {\n";

        QString data_str;
        if(re->type() != ObjectType::ARR || rv->subIndex() == 0){
            data_str = RegUtils::getVarMem(m_dataName, re, rv, 0, m_entryNameMap, m_varNameMap, m_syntaxType);
        }else{
            data_str = RegUtils::getArrMem(m_dataName, re, rv, 0, m_entryNameMap, m_varNameMap, m_syntaxType);
        }

        uint dataSize = RegTypes::sizeBytes(rv->dataType());

        co_attributes_t attrs = RegTypes::eflagsToCoAttributes(rv->eflags());
        if(dataSize > 1) attrs |= COAttribute::MB;
        QString attrs_str = RegTypes::getNames(attrs, RegTypes::coAttributeFullName).join(" | ");

        out << QStringLiteral("    .dataOrig = &%1,")
               .arg(data_str)
            << "\n";

        out << QStringLiteral("    .subIndex = %1,")
               .arg(static_cast<uint>(rv->subIndex()))
            << "\n";

        out << QStringLiteral("    .attribute = %1,")
               .arg(attrs_str)
            << "\n";

        out << QStringLiteral("    .dataLength = %1")
               .arg(dataSize)
            << "\n";

        out << "    }";
    }
    out << "\n";

    return true;
}

bool RegListCoExporter::writeOdArrConstDef(QTextStream& out_stream, const RegEntry* re)
{
#if defined(DEBUG_OUTPUT) && DEBUG_OUTPUT == 1
    Q_UNUSED(out_stream)
    QDebug out = qDebug();
#else
    QTextStream& out = out_stream;
#endif

    //const RegVar* rv0;
    QString data_str0;
    QString data_str;
    //const RegVar* rvarr;
    co_attributes_t attrs0 = COAttribute::NONE;
    co_attributes_t attrs = COAttribute::NONE;
    uint dataSize = 0;
    QString dataTypeStr = 0;

    for(auto rvit = re->cbegin(); rvit != re->cend(); ++ rvit){
        const RegVar* rv = *rvit;

        QString cur_data_str;
        if(re->type() != ObjectType::ARR || rv->subIndex() == 0){
            cur_data_str = RegUtils::getVarMem(m_dataName, re, rv, 0, m_entryNameMap, m_varNameMap, m_syntaxType);
        }else{
            cur_data_str = RegUtils::getArrMem(m_dataName, re, rv, 0, m_entryNameMap, m_varNameMap, m_syntaxType);
        }

        uint cur_dataSize = RegTypes::sizeBytes(rv->dataType());

        co_attributes_t cur_attrs = RegTypes::eflagsToCoAttributes(rv->eflags());
        if(cur_dataSize > 1) cur_attrs |= COAttribute::MB;


        if(rv->subIndex() == 0x0){
            data_str0 = cur_data_str;
            attrs0 = cur_attrs;
            //rv0 = rv;
        }else{
            data_str = cur_data_str;
            attrs = cur_attrs;
            dataSize = cur_dataSize;
            dataTypeStr = RegTypes::varDataTypeStr(rv->dataType());
            //rvarr = rv;
            break;
        }
    }

    QString attrs_str0 = RegTypes::getNames(attrs0, RegTypes::coAttributeFullName).join(" | ");
    QString attrs_str = RegTypes::getNames(attrs, RegTypes::coAttributeFullName).join(" | ");

    out << QStringLiteral("    .dataOrig0 = &%1,")
           .arg(data_str0)
        << "\n";
    out << QStringLiteral("    .dataOrig = &%1,")
           .arg(data_str)
        << "\n";
    out << QStringLiteral("    .attribute0 = %1,")
           .arg(attrs_str0)
        << "\n";
    out << QStringLiteral("    .attribute = %1,")
           .arg(attrs_str)
        << "\n";
    out << QStringLiteral("    .dataElementLength = %1,")
           .arg(dataSize)
        << "\n";
    out << QStringLiteral("    .dataElementSizeof = sizeof(%1)")
           .arg(dataTypeStr)
        << "\n";

    return true;
}

bool RegListCoExporter::writeOd(QTextStream& out_stream, const RegEntryList* regentrylist)
{
#if defined(DEBUG_OUTPUT) && DEBUG_OUTPUT == 1
    Q_UNUSED(out_stream)
    QDebug out = qDebug();
#else
    QTextStream& out = out_stream;
#endif

    out << "// Object dictionary\n";
    out << "static OD_ATTR_OD OD_entry_t ODList[] = {\n";

    for(auto reit = regentrylist->cbegin(); reit != regentrylist->cend(); ++ reit){
        const RegEntry* re = *reit;

        uint count = 0;
        std::for_each(re->cbegin(), re->cend(), [&count](const RegVar* rv){
            count += qMax(1U, rv->count());
        });

        out << QStringLiteral("    {0x%1, 0x%2, %3, &ODObjs.%4, NULL},")
               .arg(static_cast<uint>(re->index()), 0, 16)
               .arg(count, 2, 16, QChar('0'))
               .arg(getOdObjectTypeStr(re->type()))
               .arg(getOdEntryFieldName(re))
            << "\n";
    }
    out << "    {0x0000, 0x00, 0, NULL, NULL}\n"
        << "};\n"
        << "\n"
        << "static OD_t _OD = {\n"
        << "    (sizeof(ODList) / sizeof(ODList[0])) - 1,\n"
        << "    &ODList[0]\n"
        << "};\n"
        << "\n"
        << "OD_t *OD = &_OD;\n"
        << "\n\n";

    return true;
}

QString RegListCoExporter::getOdObjectTypeStr(ObjectType objType) const
{
    switch(objType){
    case ObjectType::VAR:
        return QStringLiteral("ODT_VAR");
    case ObjectType::ARR:
        return QStringLiteral("ODT_ARR");
    case ObjectType::REC:
        return QStringLiteral("ODT_REC");
    }
    return QStringLiteral("#error Unknown type!");
}

QString RegListCoExporter::getOdEntryTypeStr(ObjectType objType) const
{
    switch(objType){
    case ObjectType::VAR:
        return QStringLiteral("OD_obj_var_t");
    case ObjectType::ARR:
        return QStringLiteral("OD_obj_array_t");
    case ObjectType::REC:
        return QStringLiteral("OD_obj_record_t");
    }
    return QStringLiteral("#error Unknown type!");
}

QString RegListCoExporter::getOdEntryFieldName(const RegEntry* re) const
{
    return QStringLiteral("o_%2_%3")
            .arg(re->index(), 0, 16)
            .arg(RegUtils::getEntryName(re, m_entryNameMap, m_syntaxType));
}

QString RegListCoExporter::getOdEntryFieldDecl(const RegEntry* re) const
{
    QString name = getOdEntryFieldName(re);
    if(re->type() == ObjectType::REC){
        name = QStringLiteral("%1[%2]")
               .arg(name)
               .arg(re->count());
    }
    return name;
}
