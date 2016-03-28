/********************************************************************************
** Form generated from reading UI file 'screensaver_dialog.ui'
**
** Created by: Qt User Interface Compiler version 5.5.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_SCREENSAVER_DIALOG_H
#define UI_SCREENSAVER_DIALOG_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QDialog>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>

QT_BEGIN_NAMESPACE

class Ui_screensaver_dialog
{
public:
    QDialogButtonBox *buttonBox;
    QLineEdit *macro_edit;
    QLabel *label;
    QLabel *label_2;
    QLineEdit *uifile_edit;
    QPushButton *pushButton;

    void setupUi(QDialog *screensaver_dialog)
    {
        if (screensaver_dialog->objectName().isEmpty())
            screensaver_dialog->setObjectName(QStringLiteral("screensaver_dialog"));
        screensaver_dialog->resize(470, 261);
        buttonBox = new QDialogButtonBox(screensaver_dialog);
        buttonBox->setObjectName(QStringLiteral("buttonBox"));
        buttonBox->setGeometry(QRect(20, 220, 441, 32));
        buttonBox->setOrientation(Qt::Horizontal);
        buttonBox->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Ok);
        macro_edit = new QLineEdit(screensaver_dialog);
        macro_edit->setObjectName(QStringLiteral("macro_edit"));
        macro_edit->setGeometry(QRect(90, 70, 331, 25));
        label = new QLabel(screensaver_dialog);
        label->setObjectName(QStringLiteral("label"));
        label->setGeometry(QRect(40, 72, 51, 19));
        label_2 = new QLabel(screensaver_dialog);
        label_2->setObjectName(QStringLiteral("label_2"));
        label_2->setGeometry(QRect(37, 122, 51, 19));
        uifile_edit = new QLineEdit(screensaver_dialog);
        uifile_edit->setObjectName(QStringLiteral("uifile_edit"));
        uifile_edit->setGeometry(QRect(90, 120, 331, 25));
        pushButton = new QPushButton(screensaver_dialog);
        pushButton->setObjectName(QStringLiteral("pushButton"));
        pushButton->setGeometry(QRect(300, 160, 112, 34));

        retranslateUi(screensaver_dialog);
        QObject::connect(buttonBox, SIGNAL(accepted()), screensaver_dialog, SLOT(accept()));
        QObject::connect(buttonBox, SIGNAL(rejected()), screensaver_dialog, SLOT(reject()));

        QMetaObject::connectSlotsByName(screensaver_dialog);
    } // setupUi

    void retranslateUi(QDialog *screensaver_dialog)
    {
        screensaver_dialog->setWindowTitle(QApplication::translate("screensaver_dialog", "caQtDM Saver Settings", 0));
        label->setText(QApplication::translate("screensaver_dialog", "macro", 0));
        label_2->setText(QApplication::translate("screensaver_dialog", "UI file", 0));
        pushButton->setText(QApplication::translate("screensaver_dialog", "File", 0));
    } // retranslateUi

};

namespace Ui {
    class screensaver_dialog: public Ui_screensaver_dialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_SCREENSAVER_DIALOG_H
