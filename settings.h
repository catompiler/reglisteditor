#ifndef SETTINGS_H
#define SETTINGS_H

#include <QObject>
#include <QString>


class QSettings;


class Settings : public QObject
{
    Q_OBJECT
public:
    explicit Settings(QObject *parent = nullptr);
    ~Settings();

    void read();
    void write() const;

    struct _S_Export{
        // general export.
        QString path;
        QString dataName;
        QString odName;
        QString edsVendorName;
        QString edsProductName;
        //QString edsFileDescr;
        QString edsFileAuthor;
        //QString edsVendorName;
        //QString edsProductName;
        // reglistexporter.
        QString reglistFileName;
        QString regIdsFileName;
        QString reglistUserCode;
        QString regIdsUserCode;
        //bool regIdsExport;
        bool reglistExport;
        // regdata exporter.
        QString regdataDeclFileName;
        QString regdataImplFileName;
        QString regdataDeclUserCode;
        QString regdataImplUserCode;
        bool regDataExport;
        // co exporter.
        QString cohFileName;
        QString cocFileName;
        QString cohUserCode;
        QString cocUserCode;
        bool coExport;
        // eds exporter.
        QString edsFileName;
        bool edsExport;
    } exporting;

signals:

private:
    void readExporting(QSettings& settings);
    void writeExporting(QSettings& settings) const;
};

#endif // SETTINGS_H
