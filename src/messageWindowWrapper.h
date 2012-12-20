#ifndef MESSAGEWINDOWWRAPPER_H
#define MESSAGEWINDOWWRAPPER_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct MessageWindow MessageWindow;
/*extern*/ MessageWindow* messageWindow;
MessageWindow* C_postMsgEvent(MessageWindow* p, int type, char* msg);

#ifdef __cplusplus
}
#endif

#endif // MESSAGEWINDOWWRAPPER_H
