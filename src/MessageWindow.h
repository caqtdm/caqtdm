//******************************************************************************
// Copyright (c) 2012 Paul Scherrer Institut PSI), Villigen, Switzerland
// Disclaimer: neither  PSI, nor any of their employees makes any warranty
// or assumes any legal liability or responsibility for the use of this software
//******************************************************************************
//******************************************************************************
//
//     Author : Anton Chr. Mezger
//
//******************************************************************************


#ifndef MESSAGEWINDOW_H
#define MESSAGEWINDOW_H

#include <QDockWidget>
#include <QTextEdit>
#include <QEvent>
#include <caQtDM_Lib_global.h>

/**
 * Message Window. Handling errors and other messages.
 */
class CAQTDM_LIBSHARED_EXPORT MessageWindow: public QDockWidget
{
    Q_OBJECT

    QTextEdit msgTextEdit;                  // Main widget.
    static MessageWindow* MsgHandler;       // Set in constructor.
    static const char* WINDOW_TITLE;        // Window title.

private:
    static QString QtMsgToQString(QtMsgType type, const char *msg);

protected:

    virtual void customEvent(QEvent* event);

public:
    enum EventType {MessageEvent = 1001};   // Custom event types.
    void closeEvent(QCloseEvent* ce);
    MessageWindow(QWidget* parent = 0);
    static void AppendMsgWrapper(QtMsgType type, char *msg);
    void postMsgEvent(QtMsgType type, char *msg);
};

class CAQTDM_LIBSHARED_EXPORT MessageEvent: public QEvent
{
public:
    QString msg;    // Message string.
    MessageEvent(QString & msg);
};

#endif
