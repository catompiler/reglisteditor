#include "settings.h"
#include <QSettings>
#include <QDir>


Settings::Settings(QObject *parent)
    : QObject{parent}
{
}

Settings::~Settings()
{
}

void Settings::read()
{
    QSettings settings("settings.ini", QSettings::IniFormat);

    readExporting(settings);
}

void Settings::write() const
{
    QSettings settings("settings.ini", QSettings::IniFormat);
    if(!settings.isWritable()) return;

    writeExporting(settings);
}

void Settings::readExporting(QSettings& settings)
{
    auto& v = exporting;
    auto& s = settings;

    v.path = s.value("export/path", QDir::currentPath()).toString();
    v.dataName = s.value("export/dataName", "regs_data").toString();
    v.odName = s.value("export/odName", "OD").toString();
    v.edsVendorName = s.value("export/edsVendorName", "").toString();
    v.edsProductName = s.value("export/edsProductName", "").toString();
    //v.edsFileDescr = s.value("export/edsFileDescr", "").toString();
    v.edsFileAuthor = s.value("export/edsFileAuthor", "").toString();
    //v.edsVendorName = s.value("export/edsVendorName", "").toString();
    //v.edsProductName = s.value("export/edsProductName", "").toString();
    v.reglistFileName = s.value("export/reglistFileName", "reg_list_data.h").toString();
    v.regIdsFileName = s.value("export/regIdsFileName", "reg_ids.h").toString();
    v.reglistUserCode = s.value("export/reglistUserCode", "").toString();
    v.regIdsUserCode = s.value("export/regIdsUserCode", "").toString();
    //v.regIdsExport = s.value("export/regIdsExport", true).toBool();
    v.reglistExport = s.value("export/reglistExport", true).toBool();
    v.regdataDeclFileName = s.value("export/regdataDeclFileName", "$(DATA_NAME).h").toString();
    v.regdataImplFileName = s.value("export/regdataImplFileName", "$(DATA_NAME).c").toString();
    v.regdataDeclUserCode = s.value("export/regdataDeclUserCode", "").toString();
    v.regdataImplUserCode = s.value("export/regdataImplUserCode", "").toString();
    v.regDataExport = s.value("export/regDataExport", true).toBool();
    v.cohFileName = s.value("export/cohFileName", "(OD_NAME).h").toString();
    v.cocFileName = s.value("export/cocFileName", "(OD_NAME).c").toString();
    v.cohUserCode = s.value("export/cohUserCode", "").toString();
    v.cocUserCode = s.value("export/cocUserCode", "").toString();
    v.coExport = s.value("export/coExport", true).toBool();
    v.edsFileName = s.value("export/edsFileName", "regs.eds").toString();
    v.edsExport = s.value("export/edsExport", true).toBool();
}

void Settings::writeExporting(QSettings& settings) const
{
    auto& v = exporting;
    auto& s = settings;

    s.setValue("export/path", v.path);
    s.setValue("export/dataName", v.dataName);
    s.setValue("export/odName", v.odName);
    s.setValue("export/edsVendorName", v.edsVendorName);
    s.setValue("export/edsProductName", v.edsProductName);
    //s.setValue("export/edsFileDescr", v.edsFileDescr);
    s.setValue("export/edsFileAuthor", v.edsFileAuthor);
    //s.setValue("export/edsVendorName", v.edsVendorName);
    //s.setValue("export/edsProductName", v.edsProductName);
    s.setValue("export/reglistFileName", v.reglistFileName);
    s.setValue("export/regIdsFileName", v.regIdsFileName);
    s.setValue("export/reglistUserCode", v.reglistUserCode);
    s.setValue("export/regIdsUserCode", v.regIdsUserCode);
    //s.setValue("export/regIdsExport", v.regIdsExport);
    s.setValue("export/reglistExport", v.reglistExport);
    s.setValue("export/regdataDeclFileName", v.regdataDeclFileName);
    s.setValue("export/regdataImplFileName", v.regdataImplFileName);
    s.setValue("export/regdataDeclUserCode", v.regdataDeclUserCode);
    s.setValue("export/regdataImplUserCode", v.regdataImplUserCode);
    s.setValue("export/regDataExport", v.regDataExport);
    s.setValue("export/cohFileName", v.cohFileName);
    s.setValue("export/cocFileName", v.cocFileName);
    s.setValue("export/cohUserCode", v.cohUserCode);
    s.setValue("export/cocUserCode", v.cocUserCode);
    s.setValue("export/coExport", v.coExport);
    s.setValue("export/edsFileName", v.edsFileName);
    s.setValue("export/edsExport", v.edsExport);
}
