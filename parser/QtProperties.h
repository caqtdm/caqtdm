#ifndef QTPROPERTIES_H
#define QTPROPERTIES_H

typedef struct myParser myParser;

typedef char string40[40];
typedef struct _zOrder {
    int indx;
    int vis;
    string40 z;
} zOrder;

#define True 1==1
#define False !True

#define UNUSED(x) (void)(x)

extern myParser* myParserPtr;
extern myParser* C_Parser(myParser* p, char *strng);
extern myParser* C_writeOpenTag(myParser* p, char *type, char *cls, char *name );
extern myParser* C_writeCloseTag(myParser* p, char *type);
extern myParser* C_writeProperty(myParser* p, char *name, char *type, char *value);
extern myParser* C_writeOpenProperty(myParser* p, char *name);
extern myParser* C_writeTaggedString(myParser* p, char *type, char *value );
extern myParser* C_writeCloseProperty(myParser* p);
extern myParser* C_writeStyleSheet(myParser* p, int r, int g, int b);

void Qt_handleString(char *property, char *tag, char *value);
void Qt_taggedString(char *tag, char *value);
void Qt_writeOpenTag(char *tag, char *typ, char *value);
void Qt_writeCloseTag(char *tag, char *value, int visibilityStatic);
void Qt_writeOpenProperty(char *property);
void Qt_writeCloseProperty();
void Qt_writeStyleSheet();
void Qt_setWheelSwitchForm(char *widget, char *token);
int Qt_setColorMode(char *widget, char *token);
int Qt_setVisibilityMode(char *widget, char *token);

void Qt_setColorBackground(char *widget, int r, int g, int b, int alpha);
void Qt_setColorForeground(char *widget, int r, int g, int b, int alpha);
void Qt_setColorLine(char *widget, int r, int g, int b, int alpha);
void Qt_setColorTrace(char *widget, int r, int g, int b, int alpha, int trace);
void Qt_setColorScale(char *widget, int r, int g, int b, int alpha);
void Qt_setColorFill(char *widget);

void Qt_setMinimumLimit(char *widget, int pen, char *token);
void Qt_setMaximumLimit(char *widget, int pen, char *token);
void Qt_setPrecision(char *widget, int pen, char *token);
void Qt_setXaxisLimitSource(char *widget, char *token);
void Qt_setYaxisLimitSource(char *widget, char *token);
void Qt_setMinimumLimitSource(char *widget, int pen, char *token);
void Qt_setMaximumLimitSource(char *widget, int pen, char *token);
void Qt_setPrecisionSource(char *widget, int pen, char *token);

void Qt_setColormain(char *widget, int r, int g, int b, int alpha);

void niceChannel(char *value);

#endif
