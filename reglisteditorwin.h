#ifndef REGLISTEDITORWIN_H
#define REGLISTEDITORWIN_H

#include <QMainWindow>


QT_BEGIN_NAMESPACE
namespace Ui { class RegListEditorWin; }
QT_END_NAMESPACE

class QItemSelection;
class QModelIndex;
class RegEntryDlg;
class RegListModel;
//class RegVarModel;
class RegDelegate;
class RegEntry;
class RegSelectDlg;



class RegListEditorWin : public QMainWindow
{
    Q_OBJECT

public:
    RegListEditorWin(QWidget *parent = nullptr);
    ~RegListEditorWin();

public slots:
    void on_actQuit_triggered(bool checked = false);
    void on_actAddItem_triggered(bool checked = false);
    void on_actAddSubItem_triggered(bool checked = false);
    void on_actDelItem_triggered(bool checked = false);
    void on_actDebugExec_triggered(bool checked = false);
    void on_tvRegList_activated(const QModelIndex &index);
    void tvRegList_selection_changed(const QItemSelection &selected, const QItemSelection &deselected);

private:
    Ui::RegListEditorWin *ui;
    RegEntryDlg* m_regEntryDlg;
    RegSelectDlg* m_regSelectDlg;

    RegListModel* m_regsListModel;
    //RegVarModel* m_regVarModel;

    RegDelegate* m_regListDelegate;
};
#endif // REGLISTEDITORWIN_H
