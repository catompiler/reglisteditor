#ifndef REGDELEGATE_H
#define REGDELEGATE_H

#include <QItemDelegate>

class RegSelectDlg;
class FlagsEditDlg;


class RegDelegate : public QItemDelegate
{
    Q_OBJECT
public:
    RegDelegate();
    ~RegDelegate();

    // QAbstractItemDelegate interface
public:
    QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
    void setEditorData(QWidget* editor, const QModelIndex& index) const override;
    void setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const override;
    void updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex& index) const override;

    RegSelectDlg* regSelectDialog() const;
    void setRegSelectDialog(RegSelectDlg* newRegSelectDialog);

    FlagsEditDlg* flagsEditDialog() const;
    void setFlagsEditDialog(FlagsEditDlg* newFlagsEditDialog);

private slots:
    void selectReg();
    void selectFlags();
    void selectEFlags();
    void editingFinished();

private:
    RegSelectDlg* m_regSelectDialog;
    FlagsEditDlg* m_flagsEditDialog;
};

#endif // REGDELEGATE_H
