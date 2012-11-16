/********************************************************************************
** Form generated from reading UI file 'main.ui'
**
** Created: Fri Nov 16 17:38:58 2012
**      by: Qt User Interface Compiler version 4.8.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAIN_H
#define UI_MAIN_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QHeaderView>
#include <QtGui/QMainWindow>
#include <QtGui/QMenu>
#include <QtGui/QMenuBar>
#include <QtGui/QPushButton>
#include <QtGui/QStatusBar>
#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QAction *aboutAction;
    QAction *exitAction;
    QAction *reloadAction;
    QAction *unconnectedAction;
    QWidget *centralwidget;
    QPushButton *btnOpen;
    QStatusBar *statusbar;
    QMenuBar *menuBar;
    QMenu *menuMenu;
    QMenu *menuPV;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName(QString::fromUtf8("MainWindow"));
        MainWindow->setWindowModality(Qt::NonModal);
        MainWindow->resize(297, 112);
        aboutAction = new QAction(MainWindow);
        aboutAction->setObjectName(QString::fromUtf8("aboutAction"));
        exitAction = new QAction(MainWindow);
        exitAction->setObjectName(QString::fromUtf8("exitAction"));
        reloadAction = new QAction(MainWindow);
        reloadAction->setObjectName(QString::fromUtf8("reloadAction"));
        unconnectedAction = new QAction(MainWindow);
        unconnectedAction->setObjectName(QString::fromUtf8("unconnectedAction"));
        centralwidget = new QWidget(MainWindow);
        centralwidget->setObjectName(QString::fromUtf8("centralwidget"));
        btnOpen = new QPushButton(centralwidget);
        btnOpen->setObjectName(QString::fromUtf8("btnOpen"));
        btnOpen->setGeometry(QRect(60, 20, 171, 27));
        MainWindow->setCentralWidget(centralwidget);
        statusbar = new QStatusBar(MainWindow);
        statusbar->setObjectName(QString::fromUtf8("statusbar"));
        MainWindow->setStatusBar(statusbar);
        menuBar = new QMenuBar(MainWindow);
        menuBar->setObjectName(QString::fromUtf8("menuBar"));
        menuBar->setGeometry(QRect(0, 0, 297, 27));
        menuMenu = new QMenu(menuBar);
        menuMenu->setObjectName(QString::fromUtf8("menuMenu"));
        menuPV = new QMenu(menuBar);
        menuPV->setObjectName(QString::fromUtf8("menuPV"));
        MainWindow->setMenuBar(menuBar);

        menuBar->addAction(menuMenu->menuAction());
        menuBar->addAction(menuPV->menuAction());
        menuMenu->addAction(aboutAction);
        menuMenu->addSeparator();
        menuMenu->addSeparator();
        menuMenu->addAction(exitAction);
        menuMenu->addAction(reloadAction);
        menuPV->addAction(unconnectedAction);

        retranslateUi(MainWindow);

        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QApplication::translate("MainWindow", "MainWindow", 0, QApplication::UnicodeUTF8));
        aboutAction->setText(QApplication::translate("MainWindow", "&About", 0, QApplication::UnicodeUTF8));
        exitAction->setText(QApplication::translate("MainWindow", "&Exit", 0, QApplication::UnicodeUTF8));
        reloadAction->setText(QApplication::translate("MainWindow", "&Reload", 0, QApplication::UnicodeUTF8));
        unconnectedAction->setText(QApplication::translate("MainWindow", "&Unconnected", 0, QApplication::UnicodeUTF8));
        btnOpen->setText(QApplication::translate("MainWindow", "Open ui File", 0, QApplication::UnicodeUTF8));
        menuMenu->setTitle(QApplication::translate("MainWindow", "Menu", 0, QApplication::UnicodeUTF8));
        menuPV->setTitle(QApplication::translate("MainWindow", "PV", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAIN_H
