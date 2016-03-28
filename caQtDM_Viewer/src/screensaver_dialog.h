#ifndef SCREENSAVER_DIALOG_H
#define SCREENSAVER_DIALOG_H
#include <QString>
#include <QDialog>

namespace Ui {
class screensaver_dialog;
}

class screensaver_dialog : public QDialog
{
    Q_OBJECT

public:
    explicit screensaver_dialog(QWidget *parent = 0);
    ~screensaver_dialog();

    void setParameter(QString macro,QString uifile);
    void getParameter(QString* macro,QString* uifile);
    //QString get_macro();
   // QString get_uifile();
protected:
    QString pmacro;
    QString puifile;

private slots:
    void on_pushButton_clicked();

    void on_uifile_edit_editingFinished();

    void on_macro_edit_editingFinished();

private:
    Ui::screensaver_dialog *ui;
};

#endif // SCREENSAVER_DIALOG_H
