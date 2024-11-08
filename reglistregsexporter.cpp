#include "reglistregsexporter.h"
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


RegListRegsExporter::RegListRegsExporter(QObject *parent)
    : RegListExporter{parent}
{
}

RegListRegsExporter::~RegListRegsExporter()
{
}

bool RegListRegsExporter::doExport(const QString& filename, const RegEntryList* regentrylist)
{
    QDir regs_dir(filename);

    if(!regs_dir.exists()) return false;

    if(m_listFileName.isEmpty()) m_listFileName = regs_dir.filePath("reg_list_data.h");
    if(m_idsFileName.isEmpty()) m_idsFileName = regs_dir.filePath("reg_ids.h");
    if(m_dataName.isEmpty()) m_dataName = "regs_data";

    m_reg_id_names.clear();

    if(!exportRegIds(m_idsFileName, regentrylist)) return false;
    if(!exportRegList(m_listFileName, regentrylist)) return false;

    m_reg_id_names.clear();

    return true;
}

RegListRegsExporter& RegListRegsExporter::setListFileName(const QString& fileName)
{
    m_listFileName = fileName;

    return *this;
}

RegListRegsExporter& RegListRegsExporter::setIdsFileName(const QString& fileName)
{
    m_idsFileName = fileName;

    return *this;
}

RegListRegsExporter& RegListRegsExporter::setDataFileName(const QString& fileName)
{
    m_dataFileName = fileName;

    return *this;
}

RegListRegsExporter& RegListRegsExporter::setUserCodeIds(const QString& userCode)
{
    m_userCodeIds = userCode;

    return *this;
}

RegListRegsExporter& RegListRegsExporter::setUserCodeList(const QString& userCode)
{
    m_userCodeList = userCode;

    return *this;
}

bool RegListRegsExporter::exportRegIds(const QString& filename, const RegEntryList* regentrylist)
{
    QFile file(filename);

    if(!file.open(QIODevice::WriteOnly)) return false;

    QFileInfo fileinfo(file);

#if defined(DEBUG_OUTPUT) && DEBUG_OUTPUT == 1
    QDebug out = qDebug();
#else
    QTextStream out(&file);
#endif

    QString reg_list_name = fileinfo.fileName().replace(QChar('.'), QChar('_'));
    QString header_guard_name = reg_list_name.toUpper();

    // header
    out << "#ifndef " << header_guard_name << "\n"
        << "#define " << header_guard_name << "\n"
        << "\n\n"
        << "// THIS FILE IS GENERATED AUTOMATICALLY!\n"
        << "// DO NOT EDIT THIS FILE!\n"
        << "\n\n";

    out << "// user code begin\n"
        << m_userCodeIds
        << "\n// user code end\n\n";

    for(auto reit = regentrylist->cbegin(); reit != regentrylist->cend(); ++ reit){
        const RegEntry* re = *reit;

        for(auto rvit = re->cbegin(); rvit != re->cend(); ++ rvit){
            const RegVar* rv = *rvit;

            if(rv->eflags() & RegEFlag::RL_HIDE){
                continue;
            }

            QString description = rv->description().simplified();

            for(uint i = 0; i < rv->count(); i ++){

                reg_fullindex_t id = RegUtils::makeFullIndex(re->index(), rv->subIndex() + i);
                QString regIdStr = makeRegIdName(re, rv, i);

                out << QString("#define %1 %2 /* %6 */")
                       .arg(regIdStr,
                            idToStr(id),
                            description)
                    << "\n";

                m_reg_id_names.insert(id, regIdStr);
            }
        }
    }

    // footer
    out << "\n#endif /* " << header_guard_name << " */\n";

    // Files closed in dtors, but, i want to do it manually.
    file.close();

    return true;
}

bool RegListRegsExporter::exportRegList(const QString& filename, const RegEntryList* regentrylist)
{
    QFile file(filename);

    if(!file.open(QIODevice::WriteOnly)) return false;

    QFileInfo fileinfo(file);

#if defined(DEBUG_OUTPUT) && DEBUG_OUTPUT == 1
    QDebug out = qDebug();
#else
    QTextStream out(&file);
#endif

    QString reg_list_name = fileinfo.fileName().replace(QChar('.'), QChar('_'));
    QString header_guard_name = reg_list_name.toUpper();

    // header
    out << "#ifndef " << header_guard_name << "\n"
        << "#define " << header_guard_name << "\n"
        << "\n\n"
        << "// THIS FILE IS GENERATED AUTOMATICALLY!\n"
        << "// DO NOT EDIT THIS FILE!\n"
        << "\n\n";

    if(!m_dataFileName.isEmpty()){
        out << "#include \"" << m_dataFileName << "\"\n\n";
    }

    out << "// user code begin\n"
        << m_userCodeList
        << "\n// user code end\n\n";

    out << "REGS_BEGIN(REG_ARRAY_NAME)\n\n";

    for(auto reit = regentrylist->cbegin(); reit != regentrylist->cend(); ++ reit){
        const RegEntry* re = *reit;

        for(auto rvit = re->cbegin(); rvit != re->cend(); ++ rvit){
            const RegVar* rv = *rvit;

            if(rv->eflags() & RegEFlag::RL_HIDE){
                continue;
            }

            reg_fullindex_t base_id = RegUtils::makeFullIndex(rv->baseIndex(), rv->baseSubIndex());
            QString baseIdStr;
            auto it_base = m_reg_id_names.find(base_id);
            if(it_base != m_reg_id_names.end()){
                baseIdStr = it_base.value();
            }else{
                baseIdStr = idToStr(base_id);
            }

            QString description = rv->description().simplified();
            QString regDataType = regDataTypeStr(rv->dataType());
            QString flagsStr = flagsToStr(rv->flags());

            for(uint i = 0; i < rv->count(); i ++){

                QString regIdStr = makeRegIdName(re, rv, i);
                QString data_str;

                if(re->type() != ObjectType::ARR || rv->subIndex() == 0){
                    data_str = RegUtils::getVarMem(m_dataName, re, rv, i, m_entryNameMap, m_varNameMap, m_syntaxType);
                }else{
                    data_str = RegUtils::getArrMem(m_dataName, re, rv, i, m_entryNameMap, m_varNameMap, m_syntaxType);
                }

                out << QString("REG(%1, %2, %3, %4, %5) /* %6 */")
                       .arg(regIdStr,
                            data_str,
                            regDataType,
                            flagsStr,
                            baseIdStr,
                            description)
                    << "\n";
            }
        }
    }

    out << "\nREGS_END()\n\n";

    // footer
    out << "\n#endif /* " << header_guard_name << " */\n";

    // Files closed in dtors, but, i want to do it manually.
    file.close();

    return true;
}

QString RegListRegsExporter::makeRegName(const RegEntry* re, const RegVar* rv, uint index) const
{
    QString name;

    if(re->type() != ObjectType::ARR || rv->subIndex() == 0x0){
        QString entry_name = RegUtils::getEntryName(re, m_entryNameMap, RegUtils::SyntaxType::UPPER_CASE);
        QString var_name = RegUtils::getVarName(re, rv, m_varNameMap, RegUtils::SyntaxType::UPPER_CASE);

        name = QStringLiteral("%1_%2").arg(entry_name, var_name);

        if(rv->count() > 1){
            name = QStringLiteral("%1_%2").arg(name).arg(index);
        }
    }else{
        QString entry_name = RegUtils::getEntryName(re, m_entryNameMap, RegUtils::SyntaxType::UPPER_CASE);
        QString var_name = RegUtils::getArrName(re, rv, m_varNameMap, RegUtils::SyntaxType::UPPER_CASE);

        name = QStringLiteral("%1_%2_%3").arg(entry_name, var_name).arg(RegUtils::getArrDataIndex(re, rv, index));
    }

    return name;
}

QString RegListRegsExporter::makeRegIdName(const RegEntry* re, const RegVar* rv, uint index) const
{
    return QStringLiteral("REG_ID_%1").arg(makeRegName(re, rv, index));//.toUpper());
}

QString RegListRegsExporter::regDataTypeStr(DataType type) const
{
    QString type_name = RegTypes::dataTypeStr(static_cast<DataType>(type));
    if(!type_name.isEmpty()) return QStringLiteral("REG_TYPE_") + type_name;
    return QStringLiteral("0x%1").arg(static_cast<uint>(type));
}

QString RegListRegsExporter::flagToStr(uint flag) const
{
    QString flag_name = RegTypes::flagName(static_cast<RegFlag::Value>(flag));
    if(!flag_name.isEmpty()) return QStringLiteral("REG_FLAG_") + flag_name;
    return QStringLiteral("0x%1").arg(flag);
}

QString RegListRegsExporter::flagsToStr(uint flags) const
{
    //auto flags_str_list = RegTypes::getNames(flags, std::function<QString(uint)>(std::bind(&RegListRegsExporter::flagToStr, this, std::placeholders::_1)));
    auto flags_str_list = RegTypes::getNames(flags, std::bind(&RegListRegsExporter::flagToStr, this, std::placeholders::_1));

    return flags_str_list.join(" | ");
}

QString RegListRegsExporter::idToStr(uint id) const
{
    return QStringLiteral("0x%1").arg(id, 6, 16, QChar('0'));
}
