/*
 *  This file is part of the caQtDM Framework, developed at the Paul Scherrer Institut,
 *  Villigen, Switzerland
 *
 *  The caQtDM Framework is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  The caQtDM Framework is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with the caQtDM Framework.  If not, see <http://www.gnu.org/licenses/>.
 *
 *  Copyright (c) 2010 - 2014
 *
 *  Author:
 *    Anton Mezger
 *  Contact details:
 *    anton.mezger@psi.ch
 */


#ifndef MESSAGEWINDOW_H
#define MESSAGEWINDOW_H

#include <QDockWidget>
#include <QTextEdit>
#include <QEvent>
#include <QMenu>
#include <QAction>
#include <QContextMenuEvent>
#include "caQtDM_Lib_global.h"

class MsgTextEdit : public  QTextEdit {
public:
    MsgTextEdit() {
    }

    void contextMenuEvent(QContextMenuEvent *event)
    {
        QMenu *menu = createStandardContextMenu();
        menu->addAction(tr("Clear"));

        QAction* selectedItem = menu->exec(event->globalPos());

        if (selectedItem) {
           if(selectedItem->text().contains("Clear")) {
               setPlainText("");
           }
        }
        delete menu;
    }

private:

};


/**
 * Message Window. Handling errors and other messages.
 */
class CAQTDM_LIBSHARED_EXPORT MessageWindow: public QDockWidget
{
    Q_OBJECT

    MsgTextEdit msgTextEdit;                  // Main widget.
    static MessageWindow* MsgHandler;       // Set in constructor.
    static const char* WINDOW_TITLE;        // Window title.

private:
    static QString QtMsgToQString(QtMsgType type, const char *msg);
    QString m_logFilePath;
protected:

    virtual void customEvent(QEvent* event);

public:
    enum EventType {MessageEvent = 1001};   // Custom event types.
    void closeEvent(QCloseEvent* ce);
    MessageWindow(QWidget* parent = 0);
    static void AppendMsgWrapper(QtMsgType type, char *msg);
    void postMsgEvent(QtMsgType type, char *msg);
    void clearText();
    QString getMessageBoxContents();
    QString getLogFilePath();
};

class CAQTDM_LIBSHARED_EXPORT MessageEvent: public QEvent
{
public:
    QString msg;    // Message string.
    MessageEvent(QString & msg);
};

#endif
