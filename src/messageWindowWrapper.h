#ifndef MESSAGEWINDOWWRAPPER_H
#define MESSAGEWINDOWWRAPPER_H

typedef struct MessageWindow MessageWindow;
extern MessageWindow* messageWindow;
MessageWindow* C_postMsgEvent(MessageWindow* p, int type, char* msg);

#endif // MESSAGEWINDOWWRAPPER_H
