#include "reglistdataexporter.h"
#include <QFileInfo>
#include <QFile>
#include <QDir>
#include "regentry.h"
#include "regvar.h"
#include "regutils.h"
#include "regtypes.h"
#include <QDebug>


#define DEBUG_OUTPUT 0
#define USE_STDINT_TYPES 1


RegListDataExporter::RegListDataExporter(QObject *parent)
    : RegListExporter{parent}
{
}

RegListDataExporter::~RegListDataExporter()
{
}


bool RegListDataExporter::doExport(const QString& filepath, const RegEntryList* regentrylist)
{
    QDir path(filepath);
    if(!path.exists()) return false;

    if(m_declFileName.isEmpty()) m_declFileName = "regs_data.h";
    if(m_implFileName.isEmpty()) m_implFileName = "regs_data.c";
    if(m_dataName.isEmpty()) m_dataName = "regs_data";

    QString declFileName = QFileInfo(m_declFileName).isAbsolute() ? m_declFileName : path.filePath(m_declFileName);
    QString implFileName = QFileInfo(m_implFileName).isAbsolute() ? m_implFileName : path.filePath(m_implFileName);

    if(!exportRegDataDecl(declFileName, regentrylist)) return false;
    if(!exportRegData(implFileName, regentrylist)) return false;

    return true;
}

RegListDataExporter& RegListDataExporter::setDeclFileName(const QString& fileName)
{
    m_declFileName = fileName;

    return *this;
}

RegListDataExporter& RegListDataExporter::setImplFileName(const QString& fileName)
{
    m_implFileName = fileName;

    return *this;
}

RegListDataExporter& RegListDataExporter::setUserCodeDecl(const QString& userCode)
{
    m_userCodeDecl = userCode;

    return *this;
}

RegListDataExporter& RegListDataExporter::setUserCodeImpl(const QString& userCode)
{
    m_userCodeImpl = userCode;

    return *this;
}

bool RegListDataExporter::exportRegDataDecl(const QString& filename, const RegEntryList* regentrylist)
{
    QFile file(filename);

    if(!file.open(QIODevice::WriteOnly)) return false;

    QFileInfo fileinfo(file);

#if defined(DEBUG_OUTPUT) && DEBUG_OUTPUT == 1
    QDebug out = qDebug();
#else
    QTextStream out(&file);
#endif

    QString struct_name;
    QString struct_type;

    auto write_struct_begin = [this, &out, &struct_name, &struct_type](const RegEntry* re){
        struct_name = makeDataStructName(re);
        struct_type = makeDataStructTypeName(struct_name);

        out << "\nstruct " << struct_type << " {\n";
    };

    auto write_struct_end = [&out, &struct_name](){
        out << QStringLiteral("} %1;").arg(struct_name) << "\n";
    };

    QString reg_list_name = fileinfo.fileName().replace(QChar('.'), QChar('_'));
    QString header_guard_name = reg_list_name.toUpper();

    // header
    out << "#ifndef " << header_guard_name << "\n"
        << "#define " << header_guard_name << "\n"
#if defined(USE_STDINT_TYPES) && USE_STDINT_TYPES == 1
        << "\n"
        << "#include <stdint.h>\n"
#else
        << "\n"
        << "#include \"reg/reg.h\"\n"
#endif
        << "\n\n"
        << "// THIS FILE IS GENERATED AUTOMATICALLY!\n"
        << "// DO NOT EDIT THIS FILE!\n"
        << "\n\n";

    out << "// user code begin\n"
        << m_userCodeDecl
        << "\n// user code end\n\n";

    out << "struct " << makeDataStructTypeName(m_dataName) << " {\n";

    for(auto reit = regentrylist->cbegin(); reit != regentrylist->cend(); ++ reit){
        const RegEntry* re = *reit;

        bool hasEmpty = std::any_of(re->cbegin(), re->cend(), [](const RegVar* rv){
            return rv->memAddr().isEmpty() && !RegTypes::isMemory(rv->dataType());
        });

        if(!hasEmpty) continue;

        write_struct_begin(re);

        for(auto rvit = re->cbegin(); rvit != re->cend(); ++ rvit){
            const RegVar* rv = *rvit;

            if(!rv->memAddr().isEmpty()){
                continue;
            }

            // domain.
            if(RegTypes::isMemory(rv->dataType())){
                continue;
            }

            QString fieldDecl;
            if(re->type() == ObjectType::ARR && rv->subIndex() != 0x0){
                fieldDecl = RegUtils::getArrDecl(re, rv, m_varNameMap, m_syntaxType);
            }else{
                fieldDecl = RegUtils::getVarDecl(re, rv, m_varNameMap, m_syntaxType);
            }
#if defined(USE_STDINT_TYPES) && USE_STDINT_TYPES == 1
            QString varTypeStr = RegTypes::varDataTypeStr(rv->dataType());
#else
            QString varTypeStr = RegTypes::varRegDataTypeStr(rv->dataType());
#endif
            QString description = rv->description().simplified();

            out << QStringLiteral("    %1 %2; /* %3 */")
                   .arg(varTypeStr,
                        fieldDecl,
                        description)
                << "\n";

            if(re->type() == ObjectType::ARR && rv->subIndex() != 0x0){
                break;
            }
        }

        write_struct_end();
    }

    out << "\n};\n";
    out << QStringLiteral("extern struct %1 %2;").arg(makeDataStructTypeName(m_dataName), m_dataName);
    out << "\n\n";

    // footer
    out << "\n#endif /* " << header_guard_name << " */\n";

    // Files closed in dtors, but, i want to do it manually.
    file.close();

    return true;
}

bool RegListDataExporter::exportRegData(const QString& filename, const RegEntryList* regentrylist)
{
    QFile file(filename);

    if(!file.open(QIODevice::WriteOnly)) return false;

    QFileInfo fileinfo(file);

#if defined(DEBUG_OUTPUT) && DEBUG_OUTPUT == 1
    QDebug out = qDebug();
#else
    QTextStream out(&file);
#endif

    QString struct_name;

    auto write_struct_begin = [this, &out, &struct_name](const RegEntry* re = nullptr){
        struct_name = makeDataStructName(re);

        out << "." << struct_name << " = {\n";
    };

    auto write_struct_end = [&out](){
        out << "\n}";
    };

    QString header_name = fileinfo.fileName().replace(QStringLiteral(".c"), QStringLiteral(".h"));

    // header
    out << "#include \"" << header_name << "\"\n"
        << "\n\n"
        << "// THIS FILE IS GENERATED AUTOMATICALLY!\n"
        << "// DO NOT EDIT THIS FILE!\n"
        << "\n\n";

    out << "// user code begin\n"
        << m_userCodeImpl
        << "\n// user code end\n\n";

    out << "struct " << makeDataStructTypeName(m_dataName) << " " << m_dataName << " = {\n";

    bool firstEntry = true;

    for(auto reit = regentrylist->cbegin(); reit != regentrylist->cend(); ++ reit){
        const RegEntry* re = *reit;

        bool hasEmpty = std::any_of(re->cbegin(), re->cend(), [](const RegVar* rv){
            return rv->memAddr().isEmpty() && !RegTypes::isMemory(rv->dataType());
        });

        if(!hasEmpty) continue;

        if(!firstEntry){
            out << ",\n";
        }
        firstEntry = false;

        write_struct_begin(re);

        bool firstVar = true;

        for(auto rvit = re->cbegin(); rvit != re->cend(); ++ rvit){
            const RegVar* rv = *rvit;

            if(!rv->memAddr().isEmpty()){
                continue;
            }

            // domain.
            if(RegTypes::isMemory(rv->dataType())){
                continue;
            }

            if(!firstVar){
                out << ",\n";
            }
            firstVar = false;

            QString fieldName;
            QString defval;
            if(re->type() == ObjectType::ARR && rv->subIndex() != 0x0){
                fieldName = RegUtils::getArrName(re, rv, m_varNameMap, m_syntaxType);
                defval = RegUtils::getArrDefValData(re);
            }else{
                fieldName = RegUtils::getVarName(re, rv, m_varNameMap, m_syntaxType);
                defval = RegUtils::getVarDefValData(rv);
            }
            QString description = rv->description().simplified();

            out << QStringLiteral("    .%1 = %2 /* %3 */")
                   .arg(fieldName,
                        defval,
                        description);

            if(re->type() == ObjectType::ARR && rv->subIndex() != 0x0){
                break;
            }
        }

        write_struct_end();
    }

    out << "\n};\n";

    // footer

    // Files closed in dtors, but, i want to do it manually.
    file.close();

    return true;
}

QString RegListDataExporter::makeDataStructName(const RegEntry* re) const
{
    return RegUtils::makeStructName(re, m_entryNameMap, m_syntaxType);
}

QString RegListDataExporter::makeDataStructTypeName(const QString& name) const
{
    return RegUtils::makeStructTypeName(name);
}
