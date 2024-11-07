#ifndef EXPORTDLG_H
#define EXPORTDLG_H

#include <QDialog>
#include <QString>

class QLineEdit;

namespace Ui {
class ExportDlg;
}

class ExportDlg : public QDialog
{
    Q_OBJECT

public:
    explicit ExportDlg(QWidget *parent = nullptr);
    ~ExportDlg();

    QString path() const;
    void setPath(const QString& newPath);

    QString regIdsFileName() const;
    void setRegIdsFileName(const QString& newRegIdsFileName);

    QString regListFileName() const;
    void setRegListFileName(const QString& newRegListFileName);

    QString regDataDeclFileName() const;
    void setRegDataDeclFileName(const QString& newRegDataDeclFileName);

    QString regDataImplFileName() const;
    void setRegDataImplFileName(const QString& newRegDataImplFileName);

    QString cohFileName() const;
    void setCohFileName(const QString& newCohFileName);

    QString cocFileName() const;
    void setCocFileName(const QString& newCocFileName);

    QString dataName() const;
    void setDataName(const QString& newDataName);

    bool exportRegs() const;
    void setExportRegs(bool newExportRegs);

    bool exportData() const;
    void setExportData(bool newExportData);

    bool exportCO() const;
    void setExportCO(bool newExportCO);

private slots:
    void on_tbPath_clicked(bool checked = false);
    void on_tbRegIdsSel_clicked(bool checked = false);
    void on_tbRegListSel_clicked(bool checked = false);
    void on_tbDataDeclSel_clicked(bool checked = false);
    void on_tbDataImplSel_clicked(bool checked = false);
    void on_tbCOHSel_clicked(bool checked = false);
    void on_tbCOCSel_clicked(bool checked = false);

private:
    Ui::ExportDlg *ui;

    void selectFileNameTo(QLineEdit* le, const QString& caption, const QString& filter);
};

#endif // EXPORTDLG_H