#include "exportdlg.h"
#include "ui_exportdlg.h"
#include <QFileInfo>
#include <QDir>
#include <QFileDialog>


ExportDlg::ExportDlg(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ExportDlg)
{
    ui->setupUi(this);

    m_varRE.setPattern("\\$\\([A-Za-z0-9_]*\\)");

    ui->lePath->setText(QDir::currentPath());
}

ExportDlg::~ExportDlg()
{
    delete ui;
}

QString ExportDlg::path() const
{
    return ui->lePath->text();
}

void ExportDlg::setPath(const QString& newPath)
{
    ui->lePath->setText(newPath);
}

QString ExportDlg::regIdsFileName() const
{
    return ui->leRegIds->text();
}

void ExportDlg::setRegIdsFileName(const QString& newRegIdsFileName)
{
    ui->leRegIds->setText(newRegIdsFileName);
}

QString ExportDlg::regListFileName() const
{
    return ui->leRegList->text();
}

void ExportDlg::setRegListFileName(const QString& newRegListFileName)
{
    ui->leRegList->setText(newRegListFileName);
}

QString ExportDlg::regDataDeclFileName() const
{
    return replaceVars(ui->leDataDecl->text());
}

QString ExportDlg::regDataDeclFileNameRaw() const
{
    return ui->leDataDecl->text();
}

void ExportDlg::setRegDataDeclFileName(const QString& newRegDataDeclFileName)
{
    ui->leDataImpl->setText(newRegDataDeclFileName);
}

QString ExportDlg::regDataImplFileName() const
{
    return replaceVars(ui->leDataImpl->text());
}

QString ExportDlg::regDataImplFileNameRaw() const
{
    return ui->leDataImpl->text();
}

void ExportDlg::setRegDataImplFileName(const QString& newRegDataImplFileName)
{
    ui->leDataImpl->setText(newRegDataImplFileName);
}

QString ExportDlg::cohFileName() const
{
    return replaceVars(ui->leCOH->text());
}

QString ExportDlg::cohFileNameRaw() const
{
    return ui->leCOH->text();
}

void ExportDlg::setCohFileName(const QString& newCohFileName)
{
    ui->leCOH->setText(newCohFileName);
}

QString ExportDlg::cocFileName() const
{
    return replaceVars(ui->leCOC->text());
}

QString ExportDlg::cocFileNameRaw() const
{
    return ui->leCOC->text();
}

void ExportDlg::setCocFileName(const QString& newCocFileName)
{
    ui->leCOC->setText(newCocFileName);
}

QString ExportDlg::edsFileName() const
{
    return ui->leEds->text();
}

void ExportDlg::setEdsFileName(const QString& newEdsFileName)
{
    ui->leEds->setText(newEdsFileName);
}

QString ExportDlg::dataName() const
{
    return ui->leDataName->text();
}

void ExportDlg::setDataName(const QString& newDataName)
{
    ui->leDataName->setText(newDataName);
}

QString ExportDlg::odName() const
{
    return ui->leOdName->text();
}

void ExportDlg::setOdName(const QString& newOdName)
{
    ui->leOdName->setText(newOdName);
}

void ExportDlg::on_pbDefault_clicked(bool checked)
{
    Q_UNUSED(checked)

    //ui->lePath->setText(QDir::currentPath());
    ui->leRegIds->setText("reg_ids.h");
    ui->leRegList->setText("reg_list_data.h");
    ui->leDataDecl->setText("$(DATA_NAME).h");
    ui->leDataImpl->setText("$(DATA_NAME).c");
    ui->leCOC->setText("$(OD_NAME).c");
    ui->leCOH->setText("$(OD_NAME).h");
    ui->leEds->setText("reg_list.eds");
    ui->cbExportRegs->setChecked(true);
    ui->cbExportData->setChecked(true);
    ui->cbExportCO->setChecked(true);
    ui->cbExportEds->setChecked(true);
    /*ui->pteUCRegIds->clear();
    ui->pteUCRegList->clear();
    ui->pteUCDataDecl->clear();
    ui->pteUCDataImpl->clear();
    ui->pteUCCOc->clear();
    ui->pteUCCOh->clear();
    ui->leDataName->setText("regs_data");
    ui->leOdName->setText("OD");
    ui->leEdsProductName->clear();
    ui->leEdsVendorName->clear();
    ui->leEdsAuthor->clear();*/
}

void ExportDlg::on_tbPath_clicked(bool checked)
{
    Q_UNUSED(checked);

    QString path = QFileDialog::getExistingDirectory(this, tr("Выбор папки для экспорта"), ui->lePath->text(), QFileDialog::ShowDirsOnly);

    if(!path.isEmpty()){
        ui->lePath->setText(path);
    }
}

void ExportDlg::on_tbRegIdsSel_clicked(bool checked)
{
    Q_UNUSED(checked);

    selectFileNameTo(ui->leRegIds, tr("Выбор файла идентификаторов"), QStringLiteral("Файлы языка С (*.c *.h)"));
}

void ExportDlg::on_tbRegListSel_clicked(bool checked)
{
    Q_UNUSED(checked);

    selectFileNameTo(ui->leRegList, tr("Выбор файла списка"), QStringLiteral("Файлы языка С (*.c *.h)"));
}

void ExportDlg::on_tbDataDeclSel_clicked(bool checked)
{
    Q_UNUSED(checked);

    selectFileNameTo(ui->leDataDecl, tr("Выбор файла декларации данных"), QStringLiteral("Файлы языка С (*.c *.h)"));
}

void ExportDlg::on_tbDataImplSel_clicked(bool checked)
{
    Q_UNUSED(checked);

    selectFileNameTo(ui->leDataImpl, tr("Выбор файла реализации данных"), QStringLiteral("Файлы языка С (*.c *.h)"));
}

void ExportDlg::on_tbCOHSel_clicked(bool checked)
{
    Q_UNUSED(checked);

    selectFileNameTo(ui->leCOH, tr("Выбор файла декларации словаря"), QStringLiteral("Файлы языка С (*.c *.h)"));
}

void ExportDlg::on_tbCOCSel_clicked(bool checked)
{
    Q_UNUSED(checked);

    selectFileNameTo(ui->leCOC, tr("Выбор файла реализации словаря"), QStringLiteral("Файлы языка С (*.c *.h)"));
}

void ExportDlg::on_tbEdsSel_clicked(bool checked)
{
    Q_UNUSED(checked);

    selectFileNameTo(ui->leEds, tr("Выбор файла EDS"), QStringLiteral("Файлы EDS (*.eds)"));
}

QString ExportDlg::edsAuthor() const
{
    return ui->leEdsAuthor->text();
}

void ExportDlg::setEdsAuthor(const QString& newEdsAuthor)
{
    ui->leEdsAuthor->setText(newEdsAuthor);
}

QString ExportDlg::edsVendorName() const
{
    return ui->leEdsVendorName->text();
}

void ExportDlg::setEdsVendorName(const QString& newEdsVendorName)
{
    ui->leEdsVendorName->setText(newEdsVendorName);
}

QString ExportDlg::edsProductName() const
{
    return ui->leEdsProductName->text();
}

void ExportDlg::setEdsProductName(const QString& newEdsProductName)
{
    ui->leEdsProductName->setText(newEdsProductName);
}

QString ExportDlg::userCodeCOc() const
{
    return ui->pteUCCOc->toPlainText();
}

void ExportDlg::setUserCodeCOc(const QString& newUserCodeCOc)
{
    ui->pteUCCOc->setPlainText(newUserCodeCOc);
}

QString ExportDlg::userCodeCOh() const
{
    return ui->pteUCCOh->toPlainText();
}

void ExportDlg::setUserCodeCOh(const QString& newUserCodeCOh)
{
    ui->pteUCCOh->setPlainText(newUserCodeCOh);
}

QString ExportDlg::userCodeDataImpl() const
{
    return ui->pteUCDataImpl->toPlainText();
}

void ExportDlg::setUserCodeDataImpl(const QString& newUserCodeDataImpl)
{
    ui->pteUCDataImpl->setPlainText(newUserCodeDataImpl);
}

QString ExportDlg::userCodeDataDecl() const
{
    return ui->pteUCDataDecl->toPlainText();
}

void ExportDlg::setUserCodeDataDecl(const QString& newUserCodeDataDecl)
{
    ui->pteUCDataDecl->setPlainText(newUserCodeDataDecl);
}

QString ExportDlg::userCodeRegList() const
{
    return ui->pteUCRegList->toPlainText();
}

void ExportDlg::setUserCodeRegList(const QString& newUserCodeRegList)
{
    ui->pteUCRegList->setPlainText(newUserCodeRegList);
}

QString ExportDlg::userCodeRegIds() const
{
    return ui->pteUCRegIds->toPlainText();
}

void ExportDlg::setUserCodeRegIds(const QString& newUserCodeRegIds)
{
    ui->pteUCRegIds->setPlainText(newUserCodeRegIds);
}

void ExportDlg::selectFileNameTo(QLineEdit* le, const QString& caption, const QString& filter)
{
    QString cur_fileName = le->text();
    QString cur_path = ui->lePath->text();

    if(!QFileInfo(cur_fileName).isAbsolute()){
        cur_fileName = QDir(cur_path).filePath(cur_fileName);
    }

    QString fileName = QFileDialog::getSaveFileName(this, caption, cur_fileName, filter);

    if(!fileName.isEmpty()){
        QFileInfo finfo(fileName);

        if(QDir(cur_path) == finfo.dir()){
            le->setText(QFileInfo(fileName).fileName());
        }else{
            le->setText(fileName);
        }
    }
}

QString ExportDlg::replaceVars(const QString& text) const
{
    const QMap<QString, QString> vars = {
        {QStringLiteral("DATA_NAME"), ui->leDataName->text()},
        {QStringLiteral("OD_NAME"), ui->leOdName->text()}
    };

    QString str = text;

    if(m_varRE.isValid()){
        int pos = 0;
        int len = 0;
        while((pos = m_varRE.indexIn(str, 0)) != -1){
            len = m_varRE.matchedLength();
            QString var_name = str.mid(pos+2, len-3).trimmed();
            str.replace(pos, len, vars.value(var_name, var_name));
        }
    }

    return str;
}

bool ExportDlg::exportCO() const
{
    return ui->cbExportCO->isChecked();
}

void ExportDlg::setExportCO(bool newExportCO)
{
    ui->cbExportCO->setChecked(newExportCO);
}

bool ExportDlg::exportEds() const
{
    return ui->cbExportEds->isChecked();
}

void ExportDlg::setExportEds(bool newExportEds)
{
    ui->cbExportEds->setChecked(newExportEds);
}

bool ExportDlg::exportData() const
{
    return ui->cbExportData->isChecked();
}

void ExportDlg::setExportData(bool newExportData)
{
    ui->cbExportData->setChecked(newExportData);
}

bool ExportDlg::exportRegs() const
{
    return ui->cbExportRegs->isChecked();
}

void ExportDlg::setExportRegs(bool newExportRegs)
{
    ui->cbExportRegs->setChecked(newExportRegs);
}
