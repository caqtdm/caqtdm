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


#if defined (_MSC_VER)
   #define _CRT_SECURE_NO_WARNINGS
#endif

#include <stdlib.h>
#include <ctype.h>
#include "parser.h"
#include "QtProperties.h"

int parsingCompositeFile;
int generateFlatFile;
int generateDeviceOnMenus;
int expandText;
int legendsForStripplot;

char filePrefix[128] = "";
static  string40 formatTable[] = { "decimal", "exponential", "engr. notation", "compact", "truncated",
                                   "hexadecimal", "octal", "string",
                                   "sexagesimal", "sexagesimal-hms", "sexagesimal-dms"};
static int nbFormats = 11;

TOKEN parseAndAppendDisplayList(DisplayInfo *displayInfo, FrameOffset *offset, char *firstToken, TOKEN firstTokenType);

DlColormap defaultDlColormap = {
    /* ncolors */
    65,
    /* r,  g,   b,   inten */
    {{ 255, 255, 255, 255, },
     { 236, 236, 236, 0, },
     { 218, 218, 218, 0, },
     { 200, 200, 200, 0, },
     { 187, 187, 187, 0, },
     { 174, 174, 174, 0, },
     { 158, 158, 158, 0, },
     { 145, 145, 145, 0, },
     { 133, 133, 133, 0, },
     { 120, 120, 120, 0, },
     { 105, 105, 105, 0, },
     { 90, 90, 90, 0, },
     { 70, 70, 70, 0, },
     { 45, 45, 45, 0, },
     { 0, 0, 0, 0, },
     { 0, 216, 0, 0, },
     { 30, 187, 0, 0, },
     { 51, 153, 0, 0, },
     { 45, 127, 0, 0, },
     { 33, 108, 0, 0, },
     { 253, 0, 0, 0, },
     { 222, 19, 9, 0, },
     { 190, 25, 11, 0, },
     { 160, 18, 7, 0, },
     { 130, 4, 0, 0, },
     { 88, 147, 255, 0, },
     { 89, 126, 225, 0, },
     { 75, 110, 199, 0, },
     { 58, 94, 171, 0, },
     { 39, 84, 141, 0, },
     { 251, 243, 74, 0, },
     { 249, 218, 60, 0, },
     { 238, 182, 43, 0, },
     { 225, 144, 21, 0, },
     { 205, 97, 0, 0, },
     { 255, 176, 255, 0, },
     { 214, 127, 226, 0, },
     { 174, 78, 188, 0, },
     { 139, 26, 150, 0, },
     { 97, 10, 117, 0, },
     { 164, 170, 255, 0, },
     { 135, 147, 226, 0, },
     { 106, 115, 193, 0, },
     { 77, 82, 164, 0, },
     { 52, 51, 134, 0, },
     { 199, 187, 109, 0, },
     { 183, 157, 92, 0, },
     { 164, 126, 60, 0, },
     { 125, 86, 39, 0, },
     { 88, 52, 15, 0, },
     { 153, 255, 255, 0, },
     { 115, 223, 255, 0, },
     { 78, 165, 249, 0, },
     { 42, 99, 228, 0, },
     { 10, 0, 184, 0, },
     { 235, 241, 181, 0, },
     { 212, 219, 157, 0, },
     { 187, 193, 135, 0, },
     { 166, 164, 98, 0, },
     { 139, 130, 57, 0, },
     { 115, 255, 107, 0, },
     { 82, 218, 59, 0, },
     { 60, 180, 32, 0, },
     { 40, 147, 21, 0, },
     { 26, 115, 9, 0, },
    }};

void parseDlColor(DisplayInfo *displayInfo, FILE *filePtr, DlColormapEntry *dlColor);
void parseObject(DisplayInfo *displayInfo, DlObject *object);
void parseGrid(DisplayInfo *displayInfo, char *widget);

DlColormap *parseAndExtractExternalColormap(DisplayInfo *displayInfo, char *filename);
void parseAndSkip(DisplayInfo *displayInfo);

TOKEN getToken(DisplayInfo *displayInfo, char *word);

void *parseRectangle(DisplayInfo *, FrameOffset *);
void *parseOval(DisplayInfo *, FrameOffset *);
void *parseArc(DisplayInfo *, FrameOffset *);
void *parseText(DisplayInfo *, FrameOffset *);
void *parsePolyline(DisplayInfo *displayInfo, FrameOffset *);
void *parseRelatedDisplay(DisplayInfo *, FrameOffset *);
void *parseShellCommand(DisplayInfo *, FrameOffset *);
void *parseBar(DisplayInfo *, FrameOffset *);
void *parseIndicator(DisplayInfo *, FrameOffset *);
void *parseChoiceButton(DisplayInfo *, FrameOffset *);
void *parseMessageButton(DisplayInfo *, FrameOffset *);
void *parseValuator(DisplayInfo *, FrameOffset *);
void *parseWheelSwitch(DisplayInfo *, FrameOffset *);
void *parseTextEntry(DisplayInfo *, FrameOffset *);
void *parseMenu(DisplayInfo *, FrameOffset *);
void parseControl(DisplayInfo *, char *widget);
void *parseImage(DisplayInfo *, FrameOffset *);
void *parseComposite(DisplayInfo *, FrameOffset *);
void *parsePolyline(DisplayInfo *, FrameOffset *);
void *parsePolygon(DisplayInfo *, FrameOffset *);
void *parseMeter(DisplayInfo *, FrameOffset *);
void *parseBar(DisplayInfo *, FrameOffset *);
void *parseByte(DisplayInfo *, FrameOffset *);
void *parseIndicator(DisplayInfo *, FrameOffset *);
void *parseTextUpdate(DisplayInfo *, FrameOffset *);
void *parseStripChart(DisplayInfo *, FrameOffset *);
void *parseCartesianPlot(DisplayInfo *, FrameOffset *);
void *getNextElement(DisplayInfo *pDI, char *token, FrameOffset *offset);


typedef void *(*medmParseFunc)(DisplayInfo *, FrameOffset *);
typedef struct {
    char *name;
    medmParseFunc func;
} ParseFuncEntry;

ParseFuncEntry parseFuncTable[] = {
    {"rectangle",            parseRectangle},
    {"oval",                 parseOval},
    {"arc",                  parseArc},
    {"text",                 parseText},
    {"falling line",         parsePolyline},
    {"rising line",          parsePolyline},
    {"related display",      parseRelatedDisplay},
    {"shell command",        parseShellCommand},
    {"bar",                  parseBar},
    {"indicator",            parseIndicator},
    {"meter",                parseMeter},
    {"byte",                 parseByte},
    {"strip chart",          parseStripChart},
    {"cartesian plot",       parseCartesianPlot},
    {"text update",          parseTextUpdate},
    {"choice button",        parseChoiceButton},
    {"button",               parseChoiceButton},
    {"message button",       parseMessageButton},
    {"menu",                 parseMenu},
    {"text entry",           parseTextEntry},
    {"valuator",             parseValuator},
    {"image",                parseImage},
    {"composite",            parseComposite},
    {"polyline",             parsePolyline},
    {"polygon",              parsePolygon},
    {"wheel switch",         parseWheelSwitch},
};

static int parseFuncTableSize = sizeof(parseFuncTable)/sizeof(ParseFuncEntry);

void correctOffset( DlObject *object, FrameOffset *offset, int *x, int *y, int *w, int *h)
{
    *w = object->width;
    *h = object->height;

    if(offset->frameX == 0) {
        *x = object->x;
    } else {
        *x = object->x - offset->frameX;
    }

    if(offset->frameY == 0) {
        *y = object->y;
    } else {
        *y = object->y - offset->frameY;
    }
}

void writeRectangleDimensions(DlObject *object, FrameOffset *offset, char *widget, int correct)
{
    char asc[MAX_ASCII];

    int x = offset->frameX;
    int y = offset->frameY;
    int w = offset->frameWidth;
    int h = offset->frameHeight;

    Qt_writeOpenProperty("geometry");
    Qt_writeOpenTag("rect", "", "");

    if(correct) correctOffset(object, offset, &x, &y, &w, &h);

    sprintf(asc, "%d", x);
    Qt_taggedString("x", asc);

    sprintf(asc, "%d", y);
    Qt_taggedString("y", asc);

    // add two pixels for frames
    if(!strcmp(widget,"caFrame")) {
        sprintf(asc, "%d", w + 2);
        Qt_taggedString("width", asc);
    } else {
        sprintf(asc, "%d", w);
        Qt_taggedString("width", asc);
    }
    if(!strcmp(widget,"caFrame")) {
        sprintf(asc, "%d", h + 2);
        Qt_taggedString("height", asc);
    } else {
        sprintf(asc, "%d", h);
        Qt_taggedString("height", asc);
    }

    Qt_writeCloseTag("rect", "", False);
    Qt_writeCloseProperty();
}

// decompose string
int parseDelimited(char *s, string40 items[], int nbItems, char token)
{
    int i, cnt;
    char * pch;
    char ctoken[2];
    memcpy(ctoken, &token, 1);
    ctoken[1] ='\0';
    for (i=0; i< (int) strlen(s); i++) if(s[i] < ' ') s[i] = '\0';
    cnt = 0;
    pch = strtok (s, ctoken);
    while (pch != NULL)
    {
        strcpy(items[cnt], pch);
        pch = strtok (NULL, ctoken);
        if(cnt++ >= nbItems) break;
    }
    return cnt;
}

void parseAndSkip(DisplayInfo *displayInfo)
{
    char token[MAX_TOKEN_LENGTH];
    TOKEN tokenType;
    int nestingLevel = 0;

    /* Just read and look for braces until we return to the same level */
    do {
        switch( (tokenType=getToken(displayInfo,token)) ) {
        case T_LEFT_BRACE:
            nestingLevel++;
            break;
        case T_RIGHT_BRACE:
            nestingLevel--;
            break;
        default:
            break;
        }
    } while( (tokenType != T_RIGHT_BRACE || nestingLevel > 0) &&
             (nestingLevel > 0) && (tokenType != T_EOF) );
}

/*** Name value table routines ***/

/*
 * generate a name-value table from the passed-in argument string
 *	returns a pointer to a NameValueTable as the function value,
 *	and the number of values in the second parameter
 *	Syntax: argsString: "a=b,c=d,..."
 */
NameValueTable *generateNameValueTable(char *argsString, int *numNameValues)
{
    char *copyOfArgsString,  *name, *value;
    char *s1;
    char nameEntry[80], valueEntry[80];
    int i, j, tableIndex, numPairs, numEntries;
    NameValueTable *nameTable;
    Boolean first;

    tableIndex = 0;
    nameTable = NULL;
    copyOfArgsString = NULL;

    if(argsString != NULL) {

        copyOfArgsString = STRDUP(argsString);
        /* see how many a=b name/value pairs are in the string */
        numPairs = 0;
        i = 0;
        while(copyOfArgsString[i++] != '\0')
            if(copyOfArgsString[i] == '=') numPairs++;


        tableIndex = 0;
        first = True;
        for(numEntries = 0; numEntries < numPairs; numEntries++) {

            /* at least one pair, proceed */
            if(first) {
                first = False;
                nameTable = (NameValueTable *) calloc(1,
                                                      numPairs*sizeof(NameValueTable));
                /* name = value, name = value, ...  therefore */
                /* name delimited by "=" and value delimited by ","  */
                s1 = copyOfArgsString;
            } else {
                s1 = NULL;
            }
            name = strtok(s1,"=");
            value = strtok(NULL,",");
            if(name != NULL && value != NULL) {
                /* found legitimate name/value pair, put in table */
                j = 0;
                for(i = 0; i < (int) strlen(name); i++) {
                    if(!isspace(name[i]))
                        nameEntry[j++] =  name[i];
                }
                nameEntry[j] = '\0';
                j = 0;
                /* Modified by A.Mezger : add possibility to have a space when entering the special character ¦ */
                for(i = 0; i < (int) strlen(value); i++) {
                    if(!isspace(value[i])) {
                        if(value[i] != -90) {
                            valueEntry[j++] =  value[i];
                        } else {
                            valueEntry[j++] =  ' ';
                        }
                    }
                }
                valueEntry[j] = '\0';
                nameTable[tableIndex].name = STRDUP(nameEntry);
                nameTable[tableIndex].value = STRDUP(valueEntry);
                tableIndex++;
            }
        }
        if(copyOfArgsString) free(copyOfArgsString);

    } else {

        /* no pairs */

    }
    *numNameValues = tableIndex;
    return (nameTable);
}

/*
 * lookup name in name-value table, return associated value (or NULL if no
 *	match)
 */
char *lookupNameValue(NameValueTable *nameValueTable, int numEntries, char *name)
{
    int i;
    if(nameValueTable != NULL && numEntries > 0) {
        for(i = 0; i < numEntries; i++)
            if(!strcmp(nameValueTable[i].name,name))
                return (nameValueTable[i].value);
    }

    return (NULL);

}

/*
 * free the name value table
 *	first, all the strings pointed to by its entries
 *	then the table itself
 */
void freeNameValueTable(NameValueTable *nameValueTable, int numEntries)
{
    int i;
    if(nameValueTable != NULL) {
        for(i = 0; i < numEntries; i++) {
            if(nameValueTable[i].name != NULL) free ((char *)nameValueTable[i].name);
            if(nameValueTable[i].value != NULL) free ((char *)
                                                      nameValueTable[i].value);
        }
        free ((char *)nameValueTable);
    }

}

void objectAttributeInit(DlObject *object)
{
    object->x = 0;
    object->y = 0;
    object->width = 10;
    object->height = 10;
}

void parseDynAttrMod(DisplayInfo *displayInfo, char *widget)
{
    char token[MAX_TOKEN_LENGTH];
    TOKEN tokenType;
    int nestingLevel = 0;

    do {
        switch( (tokenType=getToken(displayInfo,token)) ) {
        case T_WORD:
            if(!strcmp(token,"clr")) {
                getToken(displayInfo,token);
                getToken(displayInfo,token);
                if(!strcmp(token,"discrete")){
                    Qt_handleString("ColorMode", "value", "Discrete");
                } else if(!strcmp(token,"static")) {
                    Qt_handleString("ColorMode", "value", "Static");
                } else if(!strcmp(token,"alarm")) {
                    Qt_handleString("ColorMode", "value", "ALarm_Default");
                }
            } else if(!strcmp(token,"vis")) {
                getToken(displayInfo,token);
                getToken(displayInfo,token);
                Qt_setVisibilityMode(widget, token);
            }
            break;
        case T_LEFT_BRACE:
            nestingLevel++;
            break;
        case T_RIGHT_BRACE:
            nestingLevel--;
            break;
        default:
            break;
        }
    } while( (tokenType != T_RIGHT_BRACE) && (nestingLevel > 0)
             && (tokenType != T_EOF) );
}

void parseDynAttrParam(DisplayInfo *displayInfo, char *widget)
{
    char token[MAX_TOKEN_LENGTH];
    TOKEN tokenType;
    int nestingLevel = 0;
    int i;
    char chanName[6];

    UNUSED(widget);

    do {
        switch( (tokenType=getToken(displayInfo,token)) ) {
        case T_WORD:
            for(i=0; i < MAX_CALC_RECORDS; i++) {
                /* Names are chan, chanB, chanC, etc. */
                sprintf(chanName,"chan%c",i?'A'+i:'\0');
                if(!strcmp(token,chanName)) {
                    getToken(displayInfo,token);
                    getToken(displayInfo,token);
                    if(token[0]) {
                        Qt_handleString(chanName, "string", token);
                        break;
                    }
                }
                if(!strcmp(token,"calc")) {
                    getToken(displayInfo,token);
                    getToken(displayInfo,token);
                    if(token[0]) {
                        Qt_handleString(chanName, "calc", token);
                        break;
                    }
                }
            }
            break;
        case T_LEFT_BRACE:
            nestingLevel++;
            break;
        case T_RIGHT_BRACE:
            nestingLevel--;
            break;
        default:
            break;
        }
    } while( (tokenType != T_RIGHT_BRACE) && (nestingLevel > 0)
             && (tokenType != T_EOF) );
}

void parseAttr(DisplayInfo *displayInfo, char *widget)
{
    char token[MAX_TOKEN_LENGTH];
    TOKEN tokenType;
    int nestingLevel = 0;
    int clr = 0;

    UNUSED(widget);

    do {
        switch( (tokenType=getToken(displayInfo,token)) ) {
        case T_WORD:
            if(!strcmp(token,"clr")) {
                getToken(displayInfo,token);
                getToken(displayInfo,token);
                clr = atoi(token) % DL_MAX_COLORS;
                Qt_setColorForeground("",displayInfo->dlColormap->dl_color[clr].r,
                                      displayInfo->dlColormap->dl_color[clr].g,
                                      displayInfo->dlColormap->dl_color[clr].b,
                                      255);

            } else if(!strcmp(token,"style")) {
                getToken(displayInfo,token);
                getToken(displayInfo,token);
                if(!strcmp(token,"solid")) {
                    Qt_handleString("linestyle", "string", "Solid");
                } else if(!strcmp(token,"dash")) {
                    Qt_handleString("linestyle", "string", "Dash");
                }
            } else if(!strcmp(token,"fill")) {
                getToken(displayInfo,token);
                getToken(displayInfo,token);
                if(!strcmp(token,"solid")) {
                    Qt_handleString("fillstyle", "string", "Filled");
                } else if(!strcmp(token,"outline")) {
                    Qt_handleString("fillstyle", "string", "Outline");
                }
            } else if(!strcmp(token,"width")) {
                getToken(displayInfo,token);
                getToken(displayInfo,token);
                Qt_handleString("width", "string", token);
            }
            break;
        case T_LEFT_BRACE:
            nestingLevel++;
            break;
        case T_RIGHT_BRACE:
            nestingLevel--;
            break;
        default:
            break;
        }
    } while( (tokenType != T_RIGHT_BRACE) && (nestingLevel > 0)
             && (tokenType != T_EOF) ) ;     /* Do nothing */
}

void parseDynamicAttr(DisplayInfo *displayInfo, char *widget)
{
    char token[MAX_TOKEN_LENGTH];
    TOKEN tokenType;
    int nestingLevel = 0;

    do {
        switch( (tokenType=getToken(displayInfo,token)) ) {
        case T_WORD:
            if(!strcmp(token,"mod")) {
                parseDynAttrMod(displayInfo, widget);
            } else if(!strcmp(token,"param")) {
                parseDynAttrParam(displayInfo, widget);
            }
            break;
        case T_LEFT_BRACE:
            nestingLevel++;
            break;
        case T_RIGHT_BRACE:
            nestingLevel--;
            break;
        default:
            break;
        }
    } while( (tokenType != T_RIGHT_BRACE) && (nestingLevel > 0)
             && (tokenType != T_EOF) );
}

void parseMonitor(DisplayInfo *displayInfo, char *widget)
{
    char token[MAX_TOKEN_LENGTH];
    TOKEN tokenType;
    int nestingLevel = 0;
    int clr = 0;
    int bclr = 0;

    UNUSED(widget);

    do {
        switch( (tokenType=getToken(displayInfo,token)) ) {
        case T_WORD:
            if (!strcmp(token,"rdbk") ||
                    !strcmp(token,"chan")) {
                getToken(displayInfo,token);
                getToken(displayInfo,token);
                Qt_handleString("channel", "string", token);

            } else if (!strcmp(token,"clr")) {
                getToken(displayInfo,token);
                getToken(displayInfo,token);
                clr = atoi(token) % DL_MAX_COLORS;

                if(strstr(widget, "Gauge") != (char*) 0) {
                } else {
                   Qt_setColorForeground("", displayInfo->dlColormap->dl_color[clr].r,
                                      displayInfo->dlColormap->dl_color[clr].g,
                                      displayInfo->dlColormap->dl_color[clr].b,
                                      255);
                }
            } else if (!strcmp(token,"bclr")) {
                getToken(displayInfo,token);
                getToken(displayInfo,token);
                bclr = atoi(token) % DL_MAX_COLORS;

                if(strstr(widget, "Gauge") != (char*) 0) {
                } else {
                   Qt_setColorBackground("", displayInfo->dlColormap->dl_color[bclr].r,
                                      displayInfo->dlColormap->dl_color[bclr].g,
                                      displayInfo->dlColormap->dl_color[bclr].b,
                                      255);
                }

            }
            break;
        case T_LEFT_BRACE:
            nestingLevel++;
            break;
        case T_RIGHT_BRACE:
            nestingLevel--;
            break;
        default:
            break;
        }
    } while ( (tokenType != T_RIGHT_BRACE) && (nestingLevel > 0)
              && (tokenType != T_EOF) );
}

void parseBasicAttribute(DisplayInfo *displayInfo, char *widget)
{
    char token[MAX_TOKEN_LENGTH];
    TOKEN tokenType;
    int nestingLevel = 0;
    int clr=0;
    char fillstyle[MAX_TOKEN_LENGTH];
    char linestyle[MAX_TOKEN_LENGTH];

    strcpy(fillstyle, "solid");
    strcpy(linestyle, "solid");

    do {
        switch( (tokenType=getToken(displayInfo,token)) ) {
        case T_WORD:
            if(!strcmp(token,"clr")) {
                getToken(displayInfo,token);
                getToken(displayInfo,token);
                clr= atoi(token) % DL_MAX_COLORS;
            } else if(!strcmp(token,"style")) {
                getToken(displayInfo,token);
                getToken(displayInfo,token);
                strcpy(linestyle, token);
            } else if(!strcmp(token,"fill")) {
                getToken(displayInfo,token);
                getToken(displayInfo,token);
                strcpy(fillstyle, token);
            } else if(!strcmp(token,"width")) {
                getToken(displayInfo,token);
                getToken(displayInfo,token);
                Qt_handleString("lineSize", "number", token);
            }
            break;
        case T_LEFT_BRACE:
            nestingLevel++;
            break;
        case T_RIGHT_BRACE:
            nestingLevel--;
            break;
        default:
            break;
        }
    } while( (tokenType != T_RIGHT_BRACE) && (nestingLevel > 0)
             && (tokenType != T_EOF) );


    if(!strcmp(widget,"caLabel")) {
        Qt_setColorForeground("", displayInfo->dlColormap->dl_color[clr].r,
                              displayInfo->dlColormap->dl_color[clr].g,
                              displayInfo->dlColormap->dl_color[clr].b,
                              255);
        Qt_setColorBackground("", displayInfo->dlColormap->dl_color[clr].r,
                              displayInfo->dlColormap->dl_color[clr].g,
                              displayInfo->dlColormap->dl_color[clr].b,
                              0);

    } else if( (!strcmp(widget,"caPolyLine")) || (!strcmp(widget,"caGraphics")) || (!strcmp(widget,"caPolygon")) ) {
        if(!strcmp(fillstyle,"solid")) {
            Qt_setColorForeground("", displayInfo->dlColormap->dl_color[clr].r,
                                  displayInfo->dlColormap->dl_color[clr].g,
                                  displayInfo->dlColormap->dl_color[clr].b,
                                  255);
            Qt_setColorFill(widget);
            Qt_setColorLine("",  displayInfo->dlColormap->dl_color[clr].r,
                            displayInfo->dlColormap->dl_color[clr].g,
                            displayInfo->dlColormap->dl_color[clr].b,
                            255);
        } else if(!strcmp(fillstyle,"outline")) {
            Qt_setColorForeground("", displayInfo->dlColormap->dl_color[clr].r,
                                  displayInfo->dlColormap->dl_color[clr].g,
                                  displayInfo->dlColormap->dl_color[clr].b,
                                  255);
            Qt_setColorBackground("", displayInfo->dlColormap->dl_color[clr].r,
                                  displayInfo->dlColormap->dl_color[clr].g,
                                  displayInfo->dlColormap->dl_color[clr].b,
                                  0);
            Qt_setColorLine("",  displayInfo->dlColormap->dl_color[clr].r,
                            displayInfo->dlColormap->dl_color[clr].g,
                            displayInfo->dlColormap->dl_color[clr].b,
                            255);
        }


        if(!strcmp(linestyle,"solid")) {
            Qt_handleString("linestyle", "enum", "Solid");
        } else if(!strcmp(linestyle,"dash")) {
            Qt_handleString("linestyle", "enum", "Dash");
        } else if(!strcmp(linestyle,"bigdash")) {
            Qt_handleString("linestyle", "enum", "BigDash");
        }

    }

}

void parseLimits(DisplayInfo *displayInfo, char *widget, int pen, int DoNotWritePrec)
{
    char token[MAX_TOKEN_LENGTH];
    TOKEN tokenType;
    int nestingLevel = 0;
    int hoprSrc = False;
    int loprSrc = False;
    int precSrc = False;
    int hoprDefault = False;
    int loprDefault = False;

    do {
        switch( (tokenType=getToken(displayInfo,token)) ) {
        case T_WORD:
            if(!strcmp(token,"loprSrc")) {
                getToken(displayInfo,token);
                getToken(displayInfo,token);
                loprSrc = True;
                Qt_setMinimumLimitSource(widget, pen, token);
            } else if(!strcmp(token,"loprDefault")) {
                getToken(displayInfo,token);
                getToken(displayInfo,token);
                Qt_setMinimumLimit(widget, pen, token);
                loprDefault= True;
            } else if(!strcmp(token,"hoprSrc")) {
                getToken(displayInfo,token);
                getToken(displayInfo,token);
                hoprSrc = True;
                Qt_setMaximumLimitSource(widget, pen, token);
            } else if(!strcmp(token,"hoprDefault")) {
                getToken(displayInfo,token);
                getToken(displayInfo,token);
                Qt_setMaximumLimit(widget, pen, token);
                hoprDefault= True;
            } else if(!strcmp(token,"precSrc")) {
                getToken(displayInfo,token);
                getToken(displayInfo,token);
                precSrc = True;
                if(!DoNotWritePrec)Qt_setPrecisionSource(widget, pen, token);
            } else if(!strcmp(token,"precDefault")) {
                getToken(displayInfo,token);
                getToken(displayInfo,token);
                if(!DoNotWritePrec) Qt_setPrecision(widget, pen, token);
            }
            break;
        case T_LEFT_BRACE:
            nestingLevel++;
            break;
        case T_RIGHT_BRACE:
            nestingLevel--;
            break;
        default:
            break;
        }
    } while( (tokenType != T_RIGHT_BRACE) && (nestingLevel > 0)
             && (tokenType != T_EOF) );

    if(!loprSrc) Qt_setMinimumLimitSource(widget, pen, "channel");
    if(!hoprSrc) Qt_setMaximumLimitSource(widget, pen, "channel");
    if(!DoNotWritePrec) {
        if(!precSrc) Qt_setPrecisionSource(widget, pen, "channel");
    }
    if(!loprDefault && !hoprDefault) {
        char asc[MAX_ASCII];
        strcpy(asc, "0.0");
        Qt_setMinimumLimit(widget, pen, asc);
        strcpy(asc, "1.0");
        Qt_setMaximumLimit(widget, pen, asc);
    } else if(!loprDefault && hoprDefault) {
        char asc[MAX_ASCII];
        strcpy(asc, "0.0");
        Qt_setMinimumLimit(widget, pen, asc);
    }

    if(!loprSrc && !hoprSrc && !strcmp(widget, "caCircularGauge")) {
        Qt_handleString("displayLimits", "enum", "Channel_Limits");
        Qt_handleString("alarmLimits", "enum", "Channel_Alarms");
    } else if (!strcmp(widget, "caCircularGauge")) {
        Qt_handleString("displayLimits", "enum", "User_Limits");
        Qt_handleString("alarmLimits", "enum", "Channel_Alarms");
    }
}

void parseDynamicAttribute(DisplayInfo *displayInfo, char *widget, int *visibilityStatic) {

    char token[MAX_TOKEN_LENGTH];
    TOKEN tokenType;
    int nestingLevel = 0;

    *visibilityStatic = 0;  // default is first layer

    do {
        switch( (tokenType=getToken(displayInfo,token)) ) {
        case T_WORD:
            if(!strcmp(token,"clr")) {
                getToken(displayInfo,token);
                getToken(displayInfo,token);
                *visibilityStatic = Qt_setColorMode(widget, token);
            } else if(!strcmp(token,"vis")) {
                getToken(displayInfo,token);
                getToken(displayInfo,token);
                Qt_setVisibilityMode(widget, token);
            } else if(!strcmp(token,"calc")) {
                getToken(displayInfo,token);
                getToken(displayInfo,token);
                if((strlen(token) > (size_t)0)) {
                    Qt_handleString("visibilityCalc", "string", token);
                }
            } else {
                /* Channel names */
                int i;
                char chanName[10];

                for(i=0; i < MAX_CALC_RECORDS; i++) {
                    /* Names are chan, chanB, chanC, etc. */
                    sprintf(chanName,"chan%c",i?'A'+i:'\0');
                    if(!strcmp(token,chanName)) {
                        getToken(displayInfo,token);
                        getToken(displayInfo,token);
                        if((strlen(token) > (size_t)0)) {
                            sprintf(chanName,"channel%c",i?'A'+i:'\0');
                            Qt_handleString(chanName, "string", token);
                        }
                    }
                }
            }
            break;
        case T_LEFT_BRACE:
            nestingLevel++;
            break;
        case T_RIGHT_BRACE:
            nestingLevel--;
            break;
        default:
            break;
        }
    } while( (tokenType != T_RIGHT_BRACE) && (nestingLevel > 0)
             && (tokenType != T_EOF) );
}

void parseRelatedDisplayEntry(DisplayInfo *displayInfo, char *widget, char *label, char *file, char *arg, char *policy)
{
    char token[MAX_TOKEN_LENGTH];
    TOKEN tokenType;
    int nestingLevel = 0;

    UNUSED(widget);

    do {
        switch(tokenType=getToken(displayInfo,token)) {
        case T_WORD:
            if(!strcmp(token,"label")) {
                getToken(displayInfo,token);
                getToken(displayInfo,token);
                strcpy(label, token);

            } else if(!strcmp(token,"name")) {
                getToken(displayInfo,token);
                getToken(displayInfo,token);
                strcpy(file, token);

            } else if(!strcmp(token,"args")) {
                getToken(displayInfo,token);
                getToken(displayInfo,token);
                strcpy(arg, token);

            } else if(!strcmp(token,"policy")) {
                getToken(displayInfo,token);
                getToken(displayInfo,token);
                strcpy(policy, token);
            }
            break;
        case T_LEFT_BRACE:
            nestingLevel++;
            break;
        case T_RIGHT_BRACE:
            nestingLevel--;
            break;
        default:
            break;
        }
    } while((tokenType != T_RIGHT_BRACE) && (nestingLevel > 0)
            && (tokenType != T_EOF));

}

void parseGrid(DisplayInfo *displayInfo, char *widget)
{
    char token[MAX_TOKEN_LENGTH];
    TOKEN tokenType;
    int nestingLevel = 0;

    UNUSED(widget);

    do {
        switch( (tokenType=getToken(displayInfo,token)) ) {
        case T_WORD:
            if(!strcmp(token,"gridSpacing")) {
                getToken(displayInfo,token);
                getToken(displayInfo,token);
                //grid->gridSpacing = atoi(token);
            } else if(!strcmp(token,"gridOn")) {
                getToken(displayInfo,token);
                getToken(displayInfo,token);
                //grid->gridOn = atoi(token);
            } else if(!strcmp(token,"snapToGrid")) {
                getToken(displayInfo,token);
                getToken(displayInfo,token);
                //grid->snapToGrid = atoi(token);
            }
            break;
        case T_LEFT_BRACE:
            nestingLevel++;
            break;
        case T_RIGHT_BRACE:
            nestingLevel--;
            break;
        default:
            break;
        }
    } while( (tokenType != T_RIGHT_BRACE) && (nestingLevel > 0)
             && (tokenType != T_EOF) );
}

void parsePen(DisplayInfo *displayInfo, int pen, char *channels, char *widget)
{
    char token[MAX_TOKEN_LENGTH];
    TOKEN tokenType;
    int nestingLevel = 0;
    int clr = 0;

    do {
        switch( (tokenType=getToken(displayInfo,token)) ) {
        case T_WORD:
            if (!strcmp(token,"chan")) {
                getToken(displayInfo,token);
                getToken(displayInfo,token);
                // get rid of stupid signs and set to uppercase
                niceChannel(token);
                strcat(channels, token);
                strcat(channels,";");
            } else if (!strcmp(token,"clr")) {
                getToken(displayInfo,token);
                getToken(displayInfo,token);
                clr = atoi(token) % DL_MAX_COLORS;
                Qt_setColorTrace("", displayInfo->dlColormap->dl_color[clr].r,
                                 displayInfo->dlColormap->dl_color[clr].g,
                                 displayInfo->dlColormap->dl_color[clr].b,
                                 255, pen+1);
            } else if (!strcmp(token,"limits")) {
                parseLimits(displayInfo, widget, pen, False);
            }
            break;
        case T_LEFT_BRACE:
            nestingLevel++;
            break;
        case T_RIGHT_BRACE:
            nestingLevel--;
            break;
        default:
            break;
        }
    } while ( (tokenType != T_RIGHT_BRACE) && (nestingLevel > 0)
              && (tokenType != T_EOF) );

}

void parseObject(DisplayInfo *displayInfo, DlObject *object)
{

    // in case of labels, the width has not alwas been set properly by the user
    // do nothing now, and set the object properties later

    char token[MAX_TOKEN_LENGTH];
    TOKEN tokenType;
    int nestingLevel = 0;

    //Qt_writeOpenProperty("geometry");
    //Qt_writeOpenTag("rect", "", "");

    do {
        switch( (tokenType=getToken(displayInfo,token)) ) {
        case T_WORD:
            if(!strcmp(token,"x")) {
                getToken(displayInfo,token);
                getToken(displayInfo,token);
                object->x = atoi(token);
            } else if(!strcmp(token,"y")) {
                getToken(displayInfo,token);
                getToken(displayInfo,token);
                object->y = atoi(token);
            } else if(!strcmp(token,"width")) {
                getToken(displayInfo,token);
                getToken(displayInfo,token);
                object->width = atoi(token);
                if(object->width < 1) object->width=1;   // oh; line without thickness
            } else if(!strcmp(token,"height")) {
                getToken(displayInfo,token);
                getToken(displayInfo,token);
                object->height = atoi(token);
                if(object->height < 1) object->height=1;   // oh; line without thickness
            }
            break;
        case T_LEFT_BRACE:
            nestingLevel++;
            break;
        case T_RIGHT_BRACE:
            nestingLevel--;
            break;
        default:
            break;
        }
    } while( (tokenType != T_RIGHT_BRACE) && (nestingLevel > 0)
             && (tokenType != T_EOF) );

}

void parsePlotcom(DisplayInfo *displayInfo, char *widget)
{
    char token[MAX_TOKEN_LENGTH];
    TOKEN tokenType;
    int nestingLevel = 0;
    int clr = 0;
    int bclr = 0;

    UNUSED(widget);

    do {
        switch( (tokenType=getToken(displayInfo,token)) ) {
        case T_WORD:
            if (!strcmp(token,"title")) {
                getToken(displayInfo,token);
                getToken(displayInfo,token);
                Qt_handleString("Title", "string", token);
            } else if (!strcmp(token,"xlabel")) {
                getToken(displayInfo,token);
                getToken(displayInfo,token);
                Qt_handleString("TitleX", "string", token);
                /* this was desired by proscan and hipa, now taken out
                if(strstr(token, "[") != (char*) 0 && strstr(widget, "caStripPlot") != (char*) 0) {
                    char asc[MAX_ASCII];
                    sprintf(asc, "%s::FillUnder", widget);
                    Qt_handleString("Style_1", "enum", asc);
                }*/

            } else if (!strcmp(token,"ylabel")) {
                getToken(displayInfo,token);
                getToken(displayInfo,token);
                Qt_handleString("TitleY", "string", token);
            } else if (!strcmp(token,"package")) {
                getToken(displayInfo,token);
                getToken(displayInfo,token);
                printf("adl2ui -- package not supported\n");
            } else if (!strcmp(token,"clr")) {
                getToken(displayInfo,token);
                getToken(displayInfo,token);
                clr = atoi(token) % DL_MAX_COLORS;

                Qt_setColorScale("", displayInfo->dlColormap->dl_color[clr].r,
                                 displayInfo->dlColormap->dl_color[clr].g,
                                 displayInfo->dlColormap->dl_color[clr].b,
                                 255);

            } else if (!strcmp(token,"bclr")) {
                getToken(displayInfo,token);
                getToken(displayInfo,token);
                bclr = atoi(token) % DL_MAX_COLORS;

                Qt_setColorForeground("", displayInfo->dlColormap->dl_color[bclr].r,
                                      displayInfo->dlColormap->dl_color[bclr].g,
                                      displayInfo->dlColormap->dl_color[bclr].b,
                                      255);
                Qt_setColorBackground("", displayInfo->dlColormap->dl_color[bclr].r,
                                      displayInfo->dlColormap->dl_color[bclr].g,
                                      displayInfo->dlColormap->dl_color[bclr].b,
                                      255);
            }
            break;
        case T_LEFT_BRACE:
            nestingLevel++;
            break;
        case T_RIGHT_BRACE:
            nestingLevel--;
            break;
        default:
            break;
        }
    } while ( (tokenType != T_RIGHT_BRACE) && (nestingLevel > 0)
              && (tokenType != T_EOF) );
}


void parseTrace(DisplayInfo *displayInfo, int traceNr)
{
    char token[MAX_TOKEN_LENGTH];
    TOKEN tokenType;
    int nestingLevel = 0;
    char pvX[MAX_TOKEN_LENGTH];
    char pvY[MAX_TOKEN_LENGTH];
    char mon[2*MAX_TOKEN_LENGTH+1];
    char channel[MAX_TOKEN_LENGTH];
    int clr = 0;

    pvX[0] = '\0';
    pvY[0] = '\0';

    do {
        switch( (tokenType=getToken(displayInfo,token)) ) {
        case T_WORD:
            if (!strcmp(token,"xdata")) {
                getToken(displayInfo,token);
                getToken(displayInfo,token);
                strcpy(pvX,token);
            } else if (!strcmp(token,"ydata")) {
                getToken(displayInfo,token);
                getToken(displayInfo,token);
                strcpy(pvY,token);
            } else if (!strcmp(token,"data_clr")) {
                getToken(displayInfo,token);
                getToken(displayInfo,token);
                clr = atoi(token) % DL_MAX_COLORS;

                Qt_setColorTrace("", displayInfo->dlColormap->dl_color[clr].r,
                                 displayInfo->dlColormap->dl_color[clr].g,
                                 displayInfo->dlColormap->dl_color[clr].b,
                                 255, traceNr+1);
            }
            break;
        case T_LEFT_BRACE:
            nestingLevel++;
            break;
        case T_RIGHT_BRACE:
            nestingLevel--;
            break;
        default:
            break;
        }
    } while ( (tokenType != T_RIGHT_BRACE) && (nestingLevel > 0)
              && (tokenType != T_EOF) );

    sprintf(mon, "%s;%s", pvX, pvY);
    sprintf(channel,"channels_%d", traceNr+1);
    Qt_handleString(channel, "string", mon);
}

void parsePlotAxisDefinition(DisplayInfo *displayInfo, int axis)
{
    char token[MAX_TOKEN_LENGTH];
    TOKEN tokenType;
    int nestingLevel = 0;
    char minRange[MAX_TOKEN_LENGTH];
    char maxRange[MAX_TOKEN_LENGTH];
    int rangeStyleFound = False;

    minRange[0] = '\0';
    maxRange[0] = '\0';

    do {
        switch( (tokenType=getToken(displayInfo,token)) ) {
        case T_WORD:
            if (!strcmp(token,"axisStyle")) {
                getToken(displayInfo,token);
                getToken(displayInfo,token);
                if (!strcmp(token,"linear")) {
                    printf("adl2ui -- only linear axis is supported by now\n");
                } else if (!strcmp(token,"log10")) {
                    printf("adl2ui -- no logarithmic is supported by now\n");
                } else  if (!strcmp(token,"time")) {
                    printf("adl2ui -- no time axis is supported by now\n");
                }
            } else if (!strcmp(token,"rangeStyle")) {
                rangeStyleFound = True;
                getToken(displayInfo,token);
                getToken(displayInfo,token);
                if(axis == X_AXIS_ELEMENT) {
                    Qt_setXaxisLimitSource("caCartesianPlot", token);
                } else if(axis == Y1_AXIS_ELEMENT){
                    Qt_setYaxisLimitSource("caCartesianPlot", token);
                }
            } else if (!strcmp(token,"minRange")) {
                getToken(displayInfo,token);
                getToken(displayInfo,token);
                strcpy(minRange, token);
            } else if (!strcmp(token,"maxRange")) {
                getToken(displayInfo,token);
                getToken(displayInfo,token);
                strcpy(maxRange, token);
            } else if (!strcmp(token,"timeFormat")) {
                getToken(displayInfo,token);
                getToken(displayInfo,token);
                printf("adl2ui -- no time format is supported by now\n");
            }
            break;
        case T_LEFT_BRACE:
            nestingLevel++;
            break;
        case T_RIGHT_BRACE:
            nestingLevel--;
            break;
        default:
            break;
        }
    } while ( (tokenType != T_RIGHT_BRACE) && (nestingLevel > 0)
              && (tokenType != T_EOF) );

    if(minRange[0] != '\0' || maxRange[0] != '\0') {
        char range[MAX_TOKEN_LENGTH];
        if(minRange[0] == '\0') strcpy(minRange, "0");
        if(maxRange[0] == '\0') strcpy(maxRange, "0");
        if(axis == X_AXIS_ELEMENT) {
            sprintf(range, "%s;%s", minRange, maxRange);
            Qt_handleString("XaxisLimits", "string", range);
        } else if(axis == Y1_AXIS_ELEMENT){
            sprintf(range, "%s;%s", minRange, maxRange);
            Qt_handleString("YaxisLimits", "string", range);
        }
    } else {
        if(axis == X_AXIS_ELEMENT)Qt_handleString("XaxisLimits", "string", "0;1");
        else if(axis == Y1_AXIS_ELEMENT) Qt_handleString("YaxisLimits", "string", "0;1");
    }

    if(!rangeStyleFound) {
        if(axis == X_AXIS_ELEMENT) {
            Qt_setXaxisLimitSource("caCartesianPlot", "channel");
        } else if(axis == Y1_AXIS_ELEMENT) {
            Qt_setYaxisLimitSource("caCartesianPlot", "channel");
        }
    }

}

void *parseCartesianPlot(DisplayInfo *displayInfo, FrameOffset * offset)
{
    char token[MAX_TOKEN_LENGTH];
    TOKEN tokenType;
    int nestingLevel = 0;
    DlObject object={0,0,0,0};
    int traceNumber;
    int styleFound = False;
    int XrangeFound = False;
    int Y1rangeFound = False;
    int countDone = False;
    int visibilityStatic = 2; // top layer

    static int number = 0;
    char widgetName[MAX_ASCII];
    sprintf(widgetName, "caCartesianPlot_%d", number++);
    Qt_writeOpenTag("widget", "caCartesianPlot", widgetName);

    do {
        switch( (tokenType=getToken(displayInfo,token)) ) {
        case T_WORD:
            if(!strcmp(token,"object")) {
                parseObject(displayInfo, &object);
                writeRectangleDimensions(&object, offset, "caCartesianPlot", True);
            } else if(!strcmp(token,"plotcom")) {
                parsePlotcom(displayInfo, "caCartesianPlot");
            } else if(!strcmp(token,"count")) {
                getToken(displayInfo,token);
                getToken(displayInfo,token);
                if((strlen(token) > 0) && (!countDone)){
                    countDone = True;
                    Qt_handleString("countNumOrChannel", "string", token);
                }
            } else if(!strcmp(token,"style")) {
                getToken(displayInfo,token);
                getToken(displayInfo,token);
                styleFound = True;
                if(!strcmp(token,"point plot")) {
                    Qt_handleString("Style_1", "enum", "caCartesianPlot::Dots");
                    Qt_handleString("Style_2", "enum", "caCartesianPlot::Dots");
                    Qt_handleString("Style_3", "enum", "caCartesianPlot::Dots");
                    Qt_handleString("symbol_1", "enum", "caCartesianPlot::NoSymbol");
                    Qt_handleString("symbol_2", "enum", "caCartesianPlot::Rect");
                    Qt_handleString("symbol_3", "enum", "caCartesianPlot::Triangle");
                } else if(!strcmp(token,"point")) {
                    Qt_handleString("Style_1", "enum", "caCartesianPlot::Dots");
                    Qt_handleString("Style_2", "enum", "caCartesianPlot::Dots");
                    Qt_handleString("Style_3", "enum", "caCartesianPlot::Dots");
                    Qt_handleString("symbol_1", "enum", "caCartesianPlot::NoSymbol");
                    Qt_handleString("symbol_2", "enum", "caCartesianPlot::Rect");
                    Qt_handleString("symbol_3", "enum", "caCartesianPlot::Triangle");
                } else if(!strcmp(token,"line plot")) {
                    Qt_handleString("Style_1", "enum", "caCartesianPlot::Lines");
                    Qt_handleString("Style_2", "enum", "caCartesianPlot::Lines");
                    Qt_handleString("Style_3", "enum", "caCartesianPlot::Lines");
                    Qt_handleString("symbol_1", "enum", "caCartesianPlot::NoSymbol");
                    Qt_handleString("symbol_2", "enum", "caCartesianPlot::Rect");
                    Qt_handleString("symbol_3", "enum", "caCartesianPlot::Triangle");
                } else if(!strcmp(token,"line")) {
                    Qt_handleString("Style_1", "enum", "caCartesianPlot::Lines");
                    Qt_handleString("Style_2", "enum", "caCartesianPlot::Lines");
                    Qt_handleString("Style_3", "enum", "caCartesianPlot::Lines");
                    Qt_handleString("symbol_1", "enum", "caCartesianPlot::NoSymbol");
                    Qt_handleString("symbol_2", "enum", "caCartesianPlot::Rect");
                    Qt_handleString("symbol_3", "enum", "caCartesianPlot::Triangle");
                    /* Modified by A.Mezger 6-8-10: added option lines without marks and thick lines */
                } else if(!strcmp(token,"line-bigmarks")) {
                    Qt_handleString("Style_1", "enum", "caCartesianPlot::Lines");
                    Qt_handleString("Style_2", "enum", "caCartesianPlot::Lines");
                    Qt_handleString("Style_3", "enum", "caCartesianPlot::Lines");
                    Qt_handleString("symbol_1", "enum", "caCartesianPlot::Ellipse");
                    Qt_handleString("symbol_2", "enum", "caCartesianPlot::Rect");
                    Qt_handleString("symbol_3", "enum", "caCartesianPlot::Triangle");
                } else if(!strcmp(token,"line-nomarks")) {
                    Qt_handleString("Style_1", "enum", "caCartesianPlot::Lines");
                    Qt_handleString("Style_2", "enum", "caCartesianPlot::Lines");
                    Qt_handleString("Style_3", "enum", "caCartesianPlot::Lines");
                    Qt_handleString("symbol_1", "enum", "caCartesianPlot::NoSymbol");
                    Qt_handleString("symbol_2", "enum", "caCartesianPlot::NoSymbol");
                    Qt_handleString("symbol_3", "enum", "caCartesianPlot::NoSymbol");
                } else if(!strcmp(token,"line-thick")) {
                    Qt_handleString("Style_1", "enum", "caCartesianPlot::Lines");
                    Qt_handleString("Style_2", "enum", "caCartesianPlot::Lines");
                    Qt_handleString("Style_3", "enum", "caCartesianPlot::Lines");
                    Qt_handleString("symbol_1", "enum", "caCartesianPlot::NoSymbol");
                    Qt_handleString("symbol_2", "enum", "caCartesianPlot::Rect");
                    Qt_handleString("symbol_3", "enum", "caCartesianPlot::Triangle");
                } else if(!strcmp(token,"fill under")) {
                    Qt_handleString("Style_1", "enum", "caCartesianPlot::FillUnder");
                    Qt_handleString("Style_2", "enum", "caCartesianPlot::FillUnder");
                    Qt_handleString("Style_3", "enum", "caCartesianPlot::FillUnder");
                    Qt_handleString("symbol_1", "enum", "caCartesianPlot::NoSymbol");
                    Qt_handleString("symbol_2", "enum", "caCartesianPlot::Rect");
                    Qt_handleString("symbol_3", "enum", "caCartesianPlot::Triangle");
                } else if(!strcmp(token,"fill-under")) {
                    Qt_handleString("Style_1", "enum", "caCartesianPlot::FillUnder");
                    Qt_handleString("Style_2", "enum", "caCartesianPlot::FillUnder");
                    Qt_handleString("Style_3", "enum", "caCartesianPlot::FillUnder");
                    Qt_handleString("symbol_1", "enum", "caCartesianPlot::NoSymbol");
                    Qt_handleString("symbol_2", "enum", "caCartesianPlot::Rect");
                    Qt_handleString("symbol_3", "enum", "caCartesianPlot::Triangle");
                }
            } else if(!strcmp(token,"erase_oldest")) {
                getToken(displayInfo,token);
                getToken(displayInfo,token);
                if(!strcmp(token,"on")) {
                    Qt_handleString("plotMode", "enum", "caCartesianPlot::PlotLastNPoints");
                } else if(!strcmp(token,"off")) {
                    Qt_handleString("plotMode", "enum", "caCartesianPlot::PlotNPointsAndStop");
                } else if(!strcmp(token,"plot last n pts")) {
                    Qt_handleString("plotMode", "enum", "caCartesianPlot::PlotLastNPoints");
                } else if(!strcmp(token,"plot n pts & stop")) {
                    Qt_handleString("plotMode", "enum", "caCartesianPlot::PlotNPointsAndStop");
                }
            } else if(!strncmp(token,"trace",5)) {
                traceNumber = MIN(token[6] - '0', MAX_TRACES - 1);
                parseTrace(displayInfo,traceNumber);
            } else if(!strcmp(token,"x_axis")) {
                XrangeFound = True;
                parsePlotAxisDefinition(displayInfo, X_AXIS_ELEMENT);
            } else if(!strcmp(token,"y1_axis")) {
                Y1rangeFound = True;
                parsePlotAxisDefinition(displayInfo, Y1_AXIS_ELEMENT);
            } else if(!strcmp(token,"y2_axis")) {
                parsePlotAxisDefinition(displayInfo, Y2_AXIS_ELEMENT);
            } else if(!strcmp(token,"trigger")) {
                getToken(displayInfo,token);
                getToken(displayInfo,token);
                Qt_handleString("triggerChannel", "string", token);
            } else if(!strcmp(token,"erase")) {
                getToken(displayInfo,token);
                getToken(displayInfo,token);
                Qt_handleString("eraseChannel", "string", token);
            } else if(!strcmp(token,"countPvName")) {
                getToken(displayInfo,token);
                getToken(displayInfo,token);
                if(!countDone) {
                    countDone = True;
                    Qt_handleString("countNumOrChannel", "string", token);
                }
            } else if(!strcmp(token,"eraseMode")) {
                getToken(displayInfo,token);
                getToken(displayInfo,token);
                if(!strcmp(token,"if not zero")) {
                    Qt_handleString("eraseMode", "enum", "caCartesianPlot::ifnotzero");
                } else if(!strcmp(token,"if zero")) {
                    Qt_handleString("eraseMode", "enum", "caCartesianPlot::ifzero");
                }
            }
            break;
        case T_EQUAL:
            break;
        case T_LEFT_BRACE:
            nestingLevel++;
            break;
        case T_RIGHT_BRACE:
            nestingLevel--;
            break;
        default:
            break;
        }
    } while( (tokenType != T_RIGHT_BRACE) && (nestingLevel > 0)
             && (tokenType != T_EOF) );

    if(!styleFound) {
        Qt_handleString("Style_1", "enum", "caCartesianPlot::Dots");
        Qt_handleString("Style_2", "enum", "caCartesianPlot::Dots");
        Qt_handleString("Style_3", "enum", "caCartesianPlot::Dots");
        Qt_handleString("symbol_1", "enum", "caCartesianPlot::NoSymbol");
        Qt_handleString("symbol_2", "enum", "caCartesianPlot::Rect");
        Qt_handleString("symbol_3", "enum", "caCartesianPlot::Triangle");
    }

    if(!XrangeFound) Qt_setXaxisLimitSource("caCartesianPlot", "channel");
    if(!Y1rangeFound) Qt_setYaxisLimitSource("caCartesianPlot", "channel");

    Qt_writeCloseTag("widget", widgetName, visibilityStatic);

    return (void*) 0;
}

void *parseRelatedDisplay(DisplayInfo *displayInfo, FrameOffset * offset)
{
    char token[MAX_TOKEN_LENGTH];
    char label[MAX_TOKEN_LENGTH] = "\0";
    char name[MAX_TOKEN_LENGTH] = "\0";
    char arg[MAX_TOKEN_LENGTH] = "\0";
    char pol[MAX_TOKEN_LENGTH] = "\0";
    int visibilityStatic = 2; // top layer

    TOKEN tokenType;
    int nestingLevel = 0;
    int displayNumber;
    int rc;
    DlObject object={0,0,0,0};
    char visual[MAX_TOKEN_LENGTH] = "Menu";
    char labels[LONGSTRING] = "\0";
    char names[LONGSTRING] = "\0";
    char argus[LONGSTRING]  = "\0";
    char remos[LONGSTRING]  = "\0";

    static int number = 0;
    int clr = 0;
    char widgetName[MAX_ASCII];
    sprintf(widgetName, "caRelatedDisplay_%d", number++);
    Qt_writeOpenTag("widget", "caRelatedDisplay", widgetName);

    do {
        switch(tokenType=getToken(displayInfo,token)) {
        case T_WORD:
            if(!strcmp(token,"object")) {
                parseObject(displayInfo, &object);
                writeRectangleDimensions(&object, offset, "caRelatedDisplay", True);
            } else if(!strncmp(token,"display",7)) {
                /* Get the display number */
                displayNumber=MAX_RELATED_DISPLAYS-1;
                rc=sscanf(token,"display[%d]",&displayNumber);
                if(rc == 0 || rc == EOF || displayNumber < 0 ||
                        displayNumber > MAX_RELATED_DISPLAYS-1) {
                    displayNumber=MAX_RELATED_DISPLAYS-1;
                }
                parseRelatedDisplayEntry(displayInfo, "caRelatedDisplay", label, name, arg, pol);

                strcat(names, name); strcat(names, ";");
                strcat(argus, arg); strcat(argus, ";");
                strcat(labels, label); strcat(labels, ";");

                if(!strncmp(pol,"replace", 7)) {
                    strcpy(pol, "true");
                } else {
                    strcpy(pol, "false");
                }
                strcat(remos, pol); strcat(remos, ";");

            } else if(!strcmp(token,"clr")) {
                getToken(displayInfo,token);
                getToken(displayInfo,token);
                clr = atoi(token) % DL_MAX_COLORS;
                Qt_setColorForeground("",displayInfo->dlColormap->dl_color[clr].r,
                                      displayInfo->dlColormap->dl_color[clr].g,
                                      displayInfo->dlColormap->dl_color[clr].b,
                                      255);
            } else if(!strcmp(token,"bclr")) {
                getToken(displayInfo,token);
                getToken(displayInfo,token);
                clr = atoi(token) % DL_MAX_COLORS;
                Qt_setColorBackground("",displayInfo->dlColormap->dl_color[clr].r,
                                      displayInfo->dlColormap->dl_color[clr].g,
                                      displayInfo->dlColormap->dl_color[clr].b,
                                      255);
            } else if(!strcmp(token,"label")) {
                getToken(displayInfo,token);
                getToken(displayInfo,token);
                Qt_handleString("label", "string", token);
            } else if(!strcmp(token,"visual")) {
                getToken(displayInfo,token);
                getToken(displayInfo,token);
                if(!strcmp(token,"a row of buttons")) {
                    strcpy(visual, "Row");
                } else if(!strcmp(token,"a column of buttons")) {
                    strcpy(visual, "Column");
                } else if(!strcmp(token, "invisible")) {
                    strcpy(visual, "Hidden");
                }
            }
            break;
        case T_EQUAL:
            break;
        case T_LEFT_BRACE:
            nestingLevel++;
            break;
        case T_RIGHT_BRACE:
            nestingLevel--;
            break;
        default:
            break;
        }
    } while((tokenType != T_RIGHT_BRACE) && (nestingLevel > 0)
            && (tokenType != T_EOF));

    Qt_handleString("stackingMode", "enum", visual);

    if(strlen(labels) > 0) {
        labels[strlen(labels) -1] = '\0';
        Qt_handleString("labels", "string", labels);
    }
    if(strlen(names) > 0) {
        names[strlen(names) -1] = '\0';
        Qt_handleString("files", "string", names);
    }
    if(strlen(argus) > 0) {
        argus[strlen(argus) -1] = '\0';
        Qt_handleString("args", "string", argus);
    }
    if(strlen(remos) > 0) {
        remos[strlen(remos) -1] = '\0';
        Qt_handleString("removeParent", "string", remos);
    }

    Qt_writeCloseTag("widget", widgetName, visibilityStatic);

    return (void*) 0;

}

void parseShellCommandEntry(DisplayInfo *displayInfo, char *widget, char *label, char *file, char *arg)
{
    char token[MAX_TOKEN_LENGTH];
    TOKEN tokenType;
    int nestingLevel = 0;

    UNUSED(widget);

    do {
        switch( (tokenType=getToken(displayInfo,token)) ) {
        case T_WORD:
            if(!strcmp(token,"label")) {
                getToken(displayInfo,token);
                getToken(displayInfo,token);
                strcpy(label, token);
            } else if(!strcmp(token,"name")) {
                getToken(displayInfo,token);
                getToken(displayInfo,token);
                strcpy(file, token);
            } else if(!strcmp(token,"args")) {
                getToken(displayInfo,token);
                getToken(displayInfo,token);
                strcpy(arg, token);
            }
            break;
        case T_LEFT_BRACE:
            nestingLevel++;
            break;
        case T_RIGHT_BRACE:
            nestingLevel--;
            break;
        default:
            break;
        }
    } while( (tokenType != T_RIGHT_BRACE) && (nestingLevel > 0)
             && (tokenType != T_EOF) );
}

void *parseShellCommand(DisplayInfo *displayInfo, FrameOffset * offset)
{
    char token[MAX_TOKEN_LENGTH];
    char label[MAX_TOKEN_LENGTH] = "\0";
    char name[MAX_TOKEN_LENGTH] = "\0";
    char arg[MAX_TOKEN_LENGTH] = "\0";
    TOKEN tokenType;
    int nestingLevel = 0;
    int cmdNumber;
    int rc;
    DlObject object={0,0,0,0};
    int clr = 0;
    int bclr = 0;

    char labels[LONGSTRING] = "\0";
    char names[LONGSTRING] = "\0";
    char argus[LONGSTRING]  = "\0";

    int visibilityStatic = 2; // top layer

    static int number = 0;
    char widgetName[MAX_ASCII];
    sprintf(widgetName, "caShellCommand_%d", number++);
    Qt_writeOpenTag("widget", "caShellCommand", widgetName);

    do {
        switch( (tokenType=getToken(displayInfo,token)) ) {
        case T_WORD:
            if(!strcmp(token,"object")) {
                parseObject(displayInfo, &object);
                writeRectangleDimensions(&object, offset, "caParseShellCommand", True);
            } else if(!strncmp(token,"command",7)) {
                /* Get the command number */
                cmdNumber=MAX_SHELL_COMMANDS-1;
                rc=sscanf(token,"command[%d]",&cmdNumber);
                if(rc == 0 || rc == EOF || cmdNumber < 0 ||
                        cmdNumber > MAX_SHELL_COMMANDS-1) {
                    cmdNumber=MAX_SHELL_COMMANDS-1;
                }
                parseShellCommandEntry(displayInfo, "caShellCommand", label, name, arg);
                strcat(names, name); strcat(names, ";");
                strcat(argus, arg); strcat(argus, ";");
                strcat(labels, label); strcat(labels, ";");
            } else if(!strcmp(token,"clr")) {
                getToken(displayInfo,token);
                getToken(displayInfo,token);
                clr = atoi(token) % DL_MAX_COLORS;

                Qt_setColorForeground("", displayInfo->dlColormap->dl_color[clr].r,
                                      displayInfo->dlColormap->dl_color[clr].g,
                                      displayInfo->dlColormap->dl_color[clr].b,
                                      255);
            } else if(!strcmp(token,"bclr")) {
                getToken(displayInfo,token);
                getToken(displayInfo,token);
                bclr = atoi(token) % DL_MAX_COLORS;

                Qt_setColorBackground("", displayInfo->dlColormap->dl_color[bclr].r,
                                      displayInfo->dlColormap->dl_color[bclr].g,
                                      displayInfo->dlColormap->dl_color[bclr].b,
                                      255);
            } else if(!strcmp(token,"label")) {
                getToken(displayInfo,token);
                getToken(displayInfo,token);
                Qt_handleString("label", "string", token);
            }
            break;
        case T_EQUAL:
            break;
        case T_LEFT_BRACE:
            nestingLevel++;
            break;
        case T_RIGHT_BRACE:
            nestingLevel--;
            break;
        default:
            break;
        }
    } while( (tokenType != T_RIGHT_BRACE) && (nestingLevel > 0)
             && (tokenType != T_EOF) );

    if(strlen(labels) > 0) {
        labels[strlen(labels) -1] = '\0';
        Qt_handleString("labels", "string", labels);
    }
    if(strlen(names) > 0) {
        names[strlen(names) -1] = '\0';
        Qt_handleString("files", "string", names);
    }
    if(strlen(argus) > 0) {
        argus[strlen(argus) -1] = '\0';
        Qt_handleString("args", "string", argus);
    }

    Qt_writeCloseTag("widget", widgetName, visibilityStatic);

    return (void*) 0;
}

void *parseMeter(DisplayInfo *displayInfo, FrameOffset * offset)
{
    char token[MAX_TOKEN_LENGTH];
    char COLORMODE[MAX_TOKEN_LENGTH] = "static";
    TOKEN tokenType;
    int nestingLevel = 0;
    static int number = 0;
    DlObject object={0,0,0,0};
    int visibilityStatic = 2; // top layer

    char widgetName[MAX_ASCII];
    sprintf(widgetName, "caCircularGauge_%d", number++);
    Qt_writeOpenTag("widget", "caCircularGauge", widgetName);

    do {
        switch( (tokenType=getToken(displayInfo,token)) ) {
        case T_WORD:
            if(!strcmp(token,"object")) {
                parseObject(displayInfo, &object);
                writeRectangleDimensions(&object, offset, "caCircularGauge", True);
            } else if(!strcmp(token,"monitor")) {
                parseMonitor(displayInfo, "caCircularGauge");
            } else if(!strcmp(token,"label")) {
                getToken(displayInfo,token);
                getToken(displayInfo,token);

                // labeltype none, no decorations, outline, limits, channel
                // Qt_handleString("labelType", "enum", token);

            } else if(!strcmp(token,"clrmod")) {
                getToken(displayInfo,token);
                getToken(displayInfo,token);
                strcpy(COLORMODE, token);
            } else if(!strcmp(token,"limits")) {
                parseLimits(displayInfo, "caCircularGauge", 0, False);
            }
            break;
        case T_EQUAL:
            break;
        case T_LEFT_BRACE:
            nestingLevel++;
            break;
        case T_RIGHT_BRACE:
            nestingLevel--;
            break;
        default:
            break;
        }
    } while( (tokenType != T_RIGHT_BRACE) && (nestingLevel > 0)
             && (tokenType != T_EOF) );

    Qt_setColorMode("caCircularGauge", COLORMODE);
    Qt_handleString("referenceEnabled", "bool", "false");
    Qt_handleString("externalScale", "bool", "true");

    Qt_writeCloseTag("widget", widgetName, visibilityStatic);

    return (void*) 0;
}

void *parseByte( DisplayInfo *displayInfo, FrameOffset * offset) {
    char start[50], end[50];
    char token[MAX_TOKEN_LENGTH];
    char direction[MAX_TOKEN_LENGTH] = "Right";
    char COLORMODE[MAX_TOKEN_LENGTH] = "static";
    TOKEN tokenType;
    int nestingLevel = 0;
    DlObject object={0,0,0,0};
    int visibilityStatic = 2; // top layer
    int startBit = 15;
    int endBit = 0;

    static int number = 0;
    char widgetName[MAX_ASCII];
    sprintf(widgetName, "caByte_%d", number++);
    Qt_writeOpenTag("widget", "caByte", widgetName);

    do {
        switch( (tokenType=getToken(displayInfo,token)) ) {
        case T_WORD:
            if(!strcmp(token,"object")) {
                parseObject(displayInfo, &object);
                writeRectangleDimensions(&object, offset, "caByte", True);
            } else if(!strcmp(token,"monitor")) {
                parseMonitor(displayInfo, "caByte");
            } else if(!strcmp(token,"clrmod")) {
                getToken(displayInfo,token);
                getToken(displayInfo,token);
                strcpy(COLORMODE, token);
            } else if(!strcmp(token,"direction")) {
                getToken(displayInfo,token);
                getToken(displayInfo,token);
                if(!strcmp(token,"up")) {
                    strcpy(direction, "Up");
                } else if(!strcmp(token,"right")) {
                    strcpy(direction, "Right");
                } else if(!strcmp(token,"down"))  {
                    strcpy(direction, "Down");
                } else if(!strcmp(token,"left")) {
                    strcpy(direction, "Left");
                }
            } else if(!strcmp(token,"sbit")) {
                getToken(displayInfo,token);
                getToken(displayInfo,token);
                startBit = atoi(token);
                Qt_handleString("startBit", "number", token);
            } else if(!strcmp(token,"ebit")) {
                getToken(displayInfo,token);
                getToken(displayInfo,token);
                endBit = atoi(token);
                Qt_handleString("endBit", "number", token);
            }
            break;
        case T_EQUAL:
            break;
        case T_LEFT_BRACE:
            nestingLevel++;
            break;
        case T_RIGHT_BRACE:
            nestingLevel--;
            break;
        default:
            break;
        }
    } while( (tokenType != T_RIGHT_BRACE) && (nestingLevel > 0)
             && (tokenType != T_EOF) );

    Qt_setColorMode("caByte", COLORMODE);

    // things were a little bit more complication, while bits can be missing and right and up direction are determined by
    // the start end endbit
    sprintf(start, "%d", startBit);
    sprintf(end, "%d", endBit);

    if(!strcmp(direction,"Right") && startBit > endBit)  {
        int aux = startBit;
        strcpy(direction, "Left");
        startBit = endBit;
        endBit = aux;
    }
    if(!strcmp(direction,"Down") && startBit > endBit)  {
        int aux = startBit;
        strcpy(direction, "Up");
        startBit = endBit;
        endBit = aux;
    }

    sprintf(start, "%d", startBit);
    sprintf(end, "%d", endBit);
    Qt_handleString("startBit", "number", start);
    Qt_handleString("endBit", "number", end);
    Qt_handleString("direction", "enum", direction);

    Qt_writeCloseTag("widget", widgetName, visibilityStatic);

    return (void*) 0;
}

void *parseStripChart(DisplayInfo *displayInfo, FrameOffset * offset)
{
    char token[MAX_TOKEN_LENGTH];
    char periodValue[MAX_TOKEN_LENGTH] = "";
    char periodUnits[MAX_TOKEN_LENGTH] = "second";
    char channels[LONGSTRING] = "";
    TOKEN tokenType;
    int nestingLevel = 0;
    DlObject object={0,0,0,0};
    int penNumber;
    int visibilityStatic = 2; // top layer

    static int number = 0;
    char widgetName[MAX_ASCII];
    sprintf(widgetName, "caStripPlot_%d", number++);
    Qt_writeOpenTag("widget", "caStripPlot", widgetName);

    do {
        switch( (tokenType=getToken(displayInfo, token)) ) {
        case T_WORD:
            if(!strcmp(token, "object")) {
                parseObject(displayInfo,  &object);
                writeRectangleDimensions(&object, offset, "caStripPlot", True);
            } else if(!strcmp(token, "plotcom")) {
                parsePlotcom(displayInfo, "caStripPlot");
            } else if(!strcmp(token, "period")) {
                getToken(displayInfo, token);
                getToken(displayInfo, token);
                strcpy(periodValue, token);
            } else if(!strcmp(token, "delay")) {
                getToken(displayInfo, token);
                getToken(displayInfo, token);
                Qt_handleString("delay", "enum", token);
            } else if(!strcmp(token, "units")) {
                getToken(displayInfo, token);
                getToken(displayInfo, token);
                strcpy(periodUnits, token);
            } else if(!strncmp(token, "pen", 3)) {
                penNumber = MIN(token[4] - '0', MAX_PENS-1);
                parsePen(displayInfo, penNumber, channels, "caStripPlot");
            }
            break;
        case T_EQUAL:
            break;
        case T_LEFT_BRACE:
            nestingLevel++;
            break;
        case T_RIGHT_BRACE:
            nestingLevel--;
            break;
        default:
            break;
        }
    } while( (tokenType != T_RIGHT_BRACE) && (nestingLevel > 0)
             && (tokenType != T_EOF) );

    if(!strcmp(periodUnits, "minute")) {
        Qt_handleString("units", "enum", "Minute");
    } else if(!strcmp(periodUnits, "second")) {
        Qt_handleString("units", "enum", "Second");
    } else if(!strcmp(periodUnits, "milli second")) {
        Qt_handleString("units", "enum", "Millisecond");
    } else if(!strcmp(periodUnits, "milli-second")) {
        Qt_handleString("units", "enum", "Millisecond");
    } else {
        Qt_handleString("units", "enum", "Second");
    }

    if(strlen(periodValue) < 1) {
        Qt_handleString("period", "double", "60");
    } else {
        Qt_handleString("period", "double", periodValue);
    }

    channels[strlen(channels)-1] = '\0';
    Qt_handleString("channels", "string", channels);

    if(!legendsForStripplot) Qt_handleString("LegendEnabled", "bool", "false");
    Qt_handleString("XaxisType", "enum", "ValueScale");

    Qt_writeCloseTag("widget", widgetName, visibilityStatic);

    return (void*) 0;

}

void *parseTextUpdate(DisplayInfo *displayInfo, FrameOffset * offset)
{
    char token[MAX_TOKEN_LENGTH];
    char COLORMODE[MAX_TOKEN_LENGTH] = "static";
    TOKEN tokenType;
    int nestingLevel = 0;
    DlObject object={0,0,0,0};
    int alignFound=False;
    int i= 0, format=0;
    int visibilityStatic = 2; // top layer

    static int number = 0;
    char widgetName[MAX_ASCII];
    sprintf(widgetName, "caLineEdit_%d", number++);
    Qt_writeOpenTag("widget", "caLineEdit", widgetName);

    do {
        switch( (tokenType=getToken(displayInfo,token)) ) {
        case T_WORD:
            if(!strcmp(token,"object")) {
                parseObject(displayInfo, &object);
                writeRectangleDimensions(&object, offset, "caLineEdit", True);
                Qt_handleString("fontScaleMode", "enum", "caLineEdit::WidthAndHeight");
            } else if(!strcmp(token,"monitor")) {
                parseMonitor(displayInfo, "caLineEdit");
            } else if(!strcmp(token,"clrmod")) {
                getToken(displayInfo,token);
                getToken(displayInfo,token);
                strcpy(COLORMODE, token);
            } else if(!strcmp(token,"format")) {
                int found=0;
                getToken(displayInfo,token);
                getToken(displayInfo,token);
                for(i=0;i<nbFormats; i++) {
                    if(!strcmp(token, formatTable[i])) {
                        format = i;
                        found = 1;
                        break;
                    }
                }
                // Backward compatibility
                if(!found) {
                    if(!strcmp(token,"decimal")) {
                        format = 0;
                    } else if(!strcmp(token,"decimal- exponential notation")) {
                        format = 1;
                    } else if(!strcmp(token,"engr. notation")) {
                        format = 2;
                    } else if(!strcmp(token,"decimal- compact")) {
                        format = 3;
                    } else if(!strcmp(token,"decimal- truncated")) {
                        format = 4;
                    } else if(!strcmp(token,"decimal- truncated ")) {
                        format = 4;
                    } else if(!strcmp(token,"hexidecimal")) {
                        format = 5;
                    }
                }
            } else if(!strcmp(token,"align")) {
                getToken(displayInfo,token);
                getToken(displayInfo,token);
                if(strstr(token, "left") != (char*) 0) {
                    alignFound = True;
                    Qt_handleString("alignment", "set", "Qt::AlignAbsolute|Qt::AlignLeft|Qt::AlignVCenter");
                } else if(strstr(token, "centered") != (char*) 0) {
                    alignFound = True;
                    Qt_handleString("alignment", "set", "Qt::AlignAbsolute|Qt::AlignHCenter|Qt::AlignVCenter");
                } else if(strstr(token, "right") != (char*) 0) {
                    alignFound = True;
                    Qt_handleString("alignment", "set", "Qt::AlignAbsolute|Qt::AlignRight|Qt::AlignVCenter");
                }

            } else if(!strcmp(token,"limits")) {
                parseLimits(displayInfo, "caLineEdit", 0, False);
            }
            break;
        case T_EQUAL:
            break;
        case T_LEFT_BRACE:
            nestingLevel++;
            break;
        case T_RIGHT_BRACE:
            nestingLevel--;
            break;
        default:
            break;
        }
    } while( (tokenType != T_RIGHT_BRACE) && (nestingLevel > 0)
             && (tokenType != T_EOF) );

    if(!alignFound) Qt_handleString("alignment", "set", "Qt::AlignAbsolute|Qt::AlignLeft|Qt::AlignVCenter");

    Qt_handleString("formatType", "enum", formatTable[format]);

    Qt_setColorMode("caLineEdit", COLORMODE);

    Qt_writeCloseTag("widget", widgetName, visibilityStatic);

    return (void*) 0;
}

void *parseChoiceButton(DisplayInfo *displayInfo, FrameOffset * offset)
{
    char token[MAX_TOKEN_LENGTH];
    char COLORMODE[MAX_TOKEN_LENGTH] = "static";
    TOKEN tokenType;
    int nestingLevel = 0;
    DlObject object={0,0,0,0};
    int visibilityStatic = 2; // top layer

    static int number = 0;
    char widgetName[MAX_ASCII];
    sprintf(widgetName, "caChoice_%d", number++);
    Qt_writeOpenTag("widget", "caChoice", widgetName);

    do {
        switch( (tokenType=getToken(displayInfo,token)) ) {
        case T_WORD:
            if(!strcmp(token,"object")) {
                parseObject(displayInfo, &object);
                writeRectangleDimensions(&object, offset, "caChoice", True);
            } else if(!strcmp(token,"control")) {
                parseControl(displayInfo, "caChoice");
            } else if(!strcmp(token,"clrmod")) {
                getToken(displayInfo,token);
                getToken(displayInfo,token);
                strcpy(COLORMODE, token);
            } else if(!strcmp(token,"stacking")) {
                getToken(displayInfo,token);
                getToken(displayInfo,token);
                if(!strcmp(token,"row")) {
                    Qt_handleString("stackingMode", "enum", "Row");
                } else if(!strcmp(token,"column")) {
                    Qt_handleString("stackingMode", "enum", "Column");
                } else if(!strcmp(token,"row column")) {
                    Qt_handleString("stackingMode", "enum", "RowColumn");
                }
            }
            break;
        case T_EQUAL:
            break;
        case T_LEFT_BRACE:
            nestingLevel++;
            break;
        case T_RIGHT_BRACE:
            nestingLevel--;
            break;
        default:
            break;
        }
    } while( (tokenType != T_RIGHT_BRACE) && (nestingLevel > 0)
             && (tokenType != T_EOF) );

    Qt_setColorMode("caChoice", COLORMODE);
    Qt_writeCloseTag("widget", widgetName, visibilityStatic);

    return (void*) 0;
}

void *parseMessageButton(DisplayInfo *displayInfo, FrameOffset * offset)
{
    char token[MAX_TOKEN_LENGTH];
    char COLORMODE[MAX_TOKEN_LENGTH] = "static";
    TOKEN tokenType;
    int nestingLevel = 0;
    DlObject object={0,0,0,0};
    int visibilityStatic = 2; // top layer

    static int number = 0;
    char widgetName[MAX_ASCII];
    sprintf(widgetName, "caMessageButton_%d", number++);
    Qt_writeOpenTag("widget", "caMessageButton", widgetName);

    do {
        switch( (tokenType=getToken(displayInfo,token)) ) {
        case T_WORD:
            if(!strcmp(token,"object")) {
                parseObject(displayInfo, &object);
                writeRectangleDimensions(&object, offset, "caMessageButton", True);
                Qt_handleString("fontScaleMode", "enum", "EPushButton::WidthAndHeight");
            } else if(!strcmp(token,"control")) {
                parseControl(displayInfo, "caMessageButton");
            } else if(!strcmp(token,"press_msg")) {
                getToken(displayInfo,token);
                getToken(displayInfo,token);
                Qt_handleString("pressMessage", "string", token);
            } else if(!strcmp(token,"release_msg")) {
                getToken(displayInfo,token);
                getToken(displayInfo,token);
                Qt_handleString("releaseMessage", "string", token);
            } else if(!strcmp(token,"label")) {
                getToken(displayInfo,token);
                getToken(displayInfo,token);
                Qt_handleString("label", "string", token);
            } else if(!strcmp(token,"clrmod")) {
                getToken(displayInfo,token);
                getToken(displayInfo,token);
                strcpy(COLORMODE, token);
            }
            break;
        case T_EQUAL:
            break;
        case T_LEFT_BRACE:
            nestingLevel++;
            break;
        case T_RIGHT_BRACE:
            nestingLevel--;
            break;
        default:
            break;
        }
    } while( (tokenType != T_RIGHT_BRACE) && (nestingLevel > 0)
             && (tokenType != T_EOF) );

    Qt_setColorMode("caMessageButton", COLORMODE);

    Qt_writeCloseTag("widget", widgetName, visibilityStatic);

    return (void*) 0;
}

void *parseMenu(DisplayInfo *displayInfo, FrameOffset * offset)
{
    char token[MAX_TOKEN_LENGTH];
    char COLORMODE[MAX_TOKEN_LENGTH] = "static";
    TOKEN tokenType;
    int nestingLevel = 0;
    DlObject object={0,0,0,0};
    int visibilityStatic = 2; // top layer

    static int number = 0;
    char widgetName[MAX_ASCII];
    sprintf(widgetName, "caMenu_%d", number++);
    Qt_writeOpenTag("widget", "caMenu", widgetName);

    do {
        switch( (tokenType=getToken(displayInfo,token)) ) {
        case T_WORD:
            if(!strcmp(token,"object")) {
                parseObject(displayInfo, &object);
                writeRectangleDimensions(&object, offset, "caMenu", True);
            } else if(!strcmp(token,"control")) {
                parseControl(displayInfo, "caMenu");
            } else if(!strcmp(token,"clrmod")) {
                getToken(displayInfo,token);
                getToken(displayInfo,token);
                strcpy(COLORMODE, token);
            }
            break;
        case T_EQUAL:
            break;
        case T_LEFT_BRACE:
            nestingLevel++;
            break;
        case T_RIGHT_BRACE:
            nestingLevel--;
            break;
        default:
            break;
        }
    } while( (tokenType != T_RIGHT_BRACE) && (nestingLevel > 0)
             && (tokenType != T_EOF) );


    Qt_setColorMode("caMenu", COLORMODE);
    // case for proscan
    if(generateDeviceOnMenus) Qt_handleString("labelDisplay", "bool", "true");

    Qt_writeCloseTag("widget", widgetName, visibilityStatic);

    return (void*) 0;
}

void *parseTextEntry(DisplayInfo *displayInfo, FrameOffset * offset)
{
    char token[MAX_TOKEN_LENGTH];
    char COLORMODE[MAX_TOKEN_LENGTH] = "static";
    TOKEN tokenType;
    int nestingLevel = 0;
    DlObject object={0,0,0,0};
    int i= 0, format=0;
    int visibilityStatic = 2; // top layer

    static int number = 0;
    char widgetName[MAX_ASCII];
    sprintf(widgetName, "caTextEntry_%d", number++);
    Qt_writeOpenTag("widget", "caTextEntry", widgetName);

    do {
        switch( (tokenType=getToken(displayInfo,token)) ) {
        case T_WORD:
            if(!strcmp(token,"object")) {
                parseObject(displayInfo, &object);
                writeRectangleDimensions(&object, offset, "caLineEdit", True);
                Qt_handleString("fontScaleMode", "enum", "caLineEdit::WidthAndHeight");
            } else if(!strcmp(token,"control")) {
                parseControl(displayInfo, "caTextEntry");
            } else if(!strcmp(token,"clrmod")) {
                getToken(displayInfo,token);
                getToken(displayInfo,token);
                strcpy(COLORMODE, token);
            } else if(!strcmp(token,"format")) {
                getToken(displayInfo,token);
                getToken(displayInfo,token);
                for(i=0;i<nbFormats; i++) {
                    if(!strcmp(token, formatTable[i])) {
                        format = i;
                        break;
                    }
                }
            } else if(!strcmp(token,"limits")) {
                parseLimits(displayInfo, "caLineEdit", 0, False);
            }
            break;
        case T_EQUAL:
            break;
        case T_LEFT_BRACE:
            nestingLevel++;
            break;
        case T_RIGHT_BRACE:
            nestingLevel--;
            break;
        default:
            break;
        }
    } while( (tokenType != T_RIGHT_BRACE) && (nestingLevel > 0)
             && (tokenType != T_EOF) );

    Qt_setColorMode("caLineEdit", COLORMODE);
    Qt_handleString("formatType", "enum", formatTable[format]);
    Qt_writeCloseTag("widget", widgetName, visibilityStatic);

    return (void*) 0;

}

void *parseValuator(DisplayInfo *displayInfo, FrameOffset * offset)
{
    char token[MAX_TOKEN_LENGTH];
    char COLORMODE[MAX_TOKEN_LENGTH] = "static";
    char direction[MAX_TOKEN_LENGTH] = "Right";
    char look[MAX_TOKEN_LENGTH] = "noDeco";
    TOKEN tokenType;
    int nestingLevel = 0;
    DlObject object={0,0,0,0};
    int visibilityStatic = 2; // top layer

    static int number = 0;
    char widgetName[MAX_ASCII];
    sprintf(widgetName, "caSlider_%d", number++);
    Qt_writeOpenTag("widget", "caSlider", widgetName);

    do {
        switch((tokenType=getToken(displayInfo,token)) ) {
        case T_WORD:
            if(!strcmp(token,"object")) {
                parseObject(displayInfo, &object);
                writeRectangleDimensions(&object, offset, "caSlider", True);
            } else if(!strcmp(token,"control")) {
                parseControl(displayInfo, "caSlider");
            } else if(!strcmp(token,"label")) {
                getToken(displayInfo,token);
                getToken(displayInfo,token);
                if(!strcmp(token,"none")) {
                    strcpy(look, "noLabel");
                } else if(!strcmp(token,"no decorations")) {
                    strcpy(look, "noDeco");
                } else if(!strcmp(token,"outline")) {
                    strcpy(look, "Outline");
                } else if(!strcmp(token,"limits")) {
                    strcpy(look, "Limits");
                } else {
                    if(!strcmp(token,"channel")) {
                        strcpy(look, "channelV");
                    }
                }
            } else if(!strcmp(token,"clrmod")) {
                getToken(displayInfo,token);
                getToken(displayInfo,token);
                strcpy(COLORMODE, token);
            } else  if(!strcmp(token,"direction")) {
                getToken(displayInfo,token);
                getToken(displayInfo,token);

                if(!strcmp(token,"up")) {
                    strcpy(direction, "Up");
                } else if(!strcmp(token,"right")) {
                    strcpy(direction, "Right");
                } else if(!strcmp(token,"down"))  {
                    strcpy(direction, "Down");
                } else if(!strcmp(token,"left")) {
                    strcpy(direction, "Left");
                }


            } else if(!strcmp(token,"dPrecision")) {
                getToken(displayInfo,token);
                getToken(displayInfo,token);
                Qt_handleString("incrementValue", "double", token);
            } else if(!strcmp(token,"limits")) {
                parseLimits(displayInfo,  "caSlider", 0, False);
            }
            break;
        case T_EQUAL:
            break;
        case T_LEFT_BRACE:
            nestingLevel++;
            break;
        case T_RIGHT_BRACE:
            nestingLevel--;
            break;
        default:
            break;
        }
    } while((tokenType != T_RIGHT_BRACE) && (nestingLevel > 0)
            && (tokenType != T_EOF) );

    Qt_handleString("direction", "enum", direction);

    if(!strcmp(look,"noLabel") || !strcmp(look,"noDeco")) {
        Qt_handleString("scalePosition", "enum", "NoScale");
    } else if(!strcmp(direction, "Up") || !strcmp(direction, "Down")) {
        Qt_handleString("scalePosition", "enum", "LeftScale");
    } else if(!strcmp(direction, "Right") || !strcmp(direction, "Left")) {
        Qt_handleString("scalePosition", "enum", "BottomScale");
    }

    Qt_setColorMode("caSlider", COLORMODE);

    Qt_writeCloseTag("widget", widgetName, visibilityStatic);

    return (void*)0;
}

void parseControl(DisplayInfo *displayInfo, char *widget)
{
    char token[MAX_TOKEN_LENGTH];
    TOKEN tokenType;
    int nestingLevel = 0;
    string40 items[10];
    int nbitems;

    do {
        switch( (tokenType=getToken(displayInfo,token)) ) {
        case T_WORD:
            if (!strcmp(token,"ctrl") || !strcmp(token,"chan")) {
                getToken(displayInfo,token);
                getToken(displayInfo,token);
                //printf("==================== token = <%s>\n", token);
                // specialty for proscan, we added bitstart and bitend to the channel sperated by a blanc
                // and separated with a comma
                nbitems = parseDelimited(token, items, 10, ' ');
                if(nbitems > 0) {
                    Qt_handleString("channel", "string", items[0]);
                    //printf("==================== token = <%s> nbitems=%d\n", token, nbitems);
                    if(nbitems > 1) {
                        nbitems = parseDelimited(items[1], items, 10, ',');
                        //printf("==================== nbitems=%d <%s> <%s>\n", nbitems, items[0], items[1]);
                        Qt_handleString("startBit", "number", items[0]);
                        Qt_handleString("endBit", "number", items[1]);
                    }
                }
            } else if (!strcmp(token,"clr")) {
                getToken(displayInfo,token);
                getToken(displayInfo,token);
                if((!strcmp(widget, "caTextEntry")) || (!strcmp(widget, "caMessageButton")) || (!strcmp(widget, "caChoice")) ||
                   (!strcmp(widget, "caMenu")) || (!strcmp(widget, "caNumeric")) || (!strcmp(widget, "caSlider"))) {
                    int clr = atoi(token) % DL_MAX_COLORS;
                    Qt_setColorForeground("",displayInfo->dlColormap->dl_color[clr].r,
                                          displayInfo->dlColormap->dl_color[clr].g,
                                          displayInfo->dlColormap->dl_color[clr].b,
                                          255);
                } else {
                    //printf("adl2ui -- forecolor  for control %s not supported (use stylesheet)\n", widget);
                }
            } else if (!strcmp(token,"bclr")) {
                getToken(displayInfo,token);
                getToken(displayInfo,token);
                if((!strcmp(widget, "caTextEntry")) || (!strcmp(widget, "caMessageButton"))|| (!strcmp(widget, "caChoice")) ||
                   (!strcmp(widget, "caMenu")) || (!strcmp(widget, "caNumeric")) || (!strcmp(widget, "caSlider"))) {
                    int bclr = atoi(token) % DL_MAX_COLORS;
                    Qt_setColorBackground("",displayInfo->dlColormap->dl_color[bclr].r,
                                          displayInfo->dlColormap->dl_color[bclr].g,
                                          displayInfo->dlColormap->dl_color[bclr].b,
                                          255);
                } else {
                    //printf("adl2ui -- backcolor  for control %s not supported (use stylesheet)\n", widget);
                }
            }
            break;
        case T_LEFT_BRACE:
            nestingLevel++;
            break;
        case T_RIGHT_BRACE:
            nestingLevel--;
            break;
        default:
            break;
        }
    } while ( (tokenType != T_RIGHT_BRACE) && (nestingLevel > 0)
              && (tokenType != T_EOF) );

}

void *parseImage(DisplayInfo *displayInfo, FrameOffset * offset)
{
    char token[MAX_TOKEN_LENGTH];
    TOKEN tokenType;
    int nestingLevel = 0;
    DlObject object={0,0,0,0};
    int visibilityStatic = 0;

    static int number = 0;
    char widgetName[MAX_ASCII];
    sprintf(widgetName, "caImage_%d", number++);
    Qt_writeOpenTag("widget", "caImage", widgetName);

    do {
        switch( (tokenType = getToken(displayInfo,token)) ) {
        case T_WORD:
            if(!strcmp(token,"object")) {
                parseObject(displayInfo, &object);
                writeRectangleDimensions(&object, offset, "caImage", True);
            } else if(!strcmp(token,"dynamic attribute")) {
                parseDynamicAttribute(displayInfo,"caImage", &visibilityStatic);
            } else if(!strcmp(token,"type")) {
                getToken(displayInfo,token);
                getToken(displayInfo,token);
            } else if(!strcmp(token,"image name")) {
                getToken(displayInfo,token);
                getToken(displayInfo,token);
                Qt_handleString("filename", "string", token);
            } else if(!strcmp(token,"calc")) {
                getToken(displayInfo,token);
                getToken(displayInfo,token);
                Qt_handleString("imageCalc", "string", token);
            }
            break;
        case T_EQUAL:
            break;
        case T_LEFT_BRACE:
            nestingLevel++;
            break;
        case T_RIGHT_BRACE:
            nestingLevel--;
            break;
        default:
            break;
        }
    } while( (tokenType != T_RIGHT_BRACE) && (nestingLevel > 0)
             && (tokenType != T_EOF) );

    Qt_writeCloseTag("widget", widgetName, visibilityStatic);

    return (void*) 0;
}

static int getWidthFromIncludeFile(DisplayInfo *displayInfo, char *filename,
                                   int *compositeFileWidth,  int *compositeFileHeight)
{
    char token[MAX_TOKEN_LENGTH];
    TOKEN tokenType;
    int savedVersionNumber;
    FILE *savedFilePtr;

    FILE *filePtr = fopen(filename, "r");
    if(!filePtr) {
        printf("\ncompositeFileParse: Cannot open file: %s\n", filename);
        return False;
    }

    savedFilePtr = displayInfo->filePtr;
    savedVersionNumber = displayInfo->versionNumber;

    displayInfo->filePtr = filePtr;
    parsingCompositeFile = True;

    // Read the display block
    tokenType=getToken(displayInfo,token);
    if(tokenType == T_WORD && !strcmp(token,"display")) {
        parseAndSkip(displayInfo);
        tokenType=getToken(displayInfo,token);
    }

    // Read the colormap
    if(tokenType == T_WORD && (!strcmp(token,"color map") ||
                               !strcmp(token,"<<color map>>"))) {
        parseAndSkip(displayInfo);
        tokenType=getToken(displayInfo,token);
    }

    while( (tokenType=getToken(displayInfo,token)) != T_EOF) {

        if(tokenType == T_WORD && !strcmp(token,"width")) {
            getToken(displayInfo,token);
            getToken(displayInfo,token);
            *compositeFileWidth = atoi(token);
            //printf("found width =%d instead off %d\n", atoi(token), newoffset->frameWidth);
        }
        if(tokenType == T_WORD && !strcmp(token,"height")) {
            getToken(displayInfo,token);
            getToken(displayInfo,token);
            *compositeFileHeight = atoi(token);
            //printf("found height =%d instead off %d\n", atoi(token), newoffset->frameHeight);
            break;
        }
    }
    // Restore displayInfo file parameters
    displayInfo->filePtr = savedFilePtr;
    displayInfo->versionNumber = savedVersionNumber;

    fclose(filePtr);
    return True;
}

static void compositeFileParse(DisplayInfo *displayInfo, char *filename, DlObject *object,
                               FrameOffset *frameoffset,
                               FrameOffset *newoffset)
{
    FILE *filePtr, *savedFilePtr;
    int savedVersionNumber;
    NameValueTable *savedNameValueTable = NULL;
    int savedNumNameValues = 0;
    char macroString[MAX_TOKEN_LENGTH];
    char token[MAX_TOKEN_LENGTH];
    TOKEN tokenType;
    char newFileName[LONGSTRING];

    int compositeFileWidth = newoffset->frameWidth + 2;
    int compositeFileHeight = newoffset->frameHeight + 2;

    if(!displayInfo) return;

    /* Separate filename and macros from dlComposite->compositeFile  */

    *macroString = '\0';
    filename[MAX_TOKEN_LENGTH-1]='\0';
    if(filename && filename[0]) {
        /* Is of the form filename;a=xxx,b=yyy,... */
        char *ptr = NULL;
        ptr = strchr(filename, ';');
        if(ptr) {
            /* End the file name at the ; */
            *ptr = '\0';
            /* Copy the remainder to the macroString */
            strcpy(macroString, ++ptr);
        }
    }

    // filename
    strcpy(newFileName, filePrefix);
    strcat(newFileName, "/");
    strcat(newFileName, filename);

    // include file instead of flat parsing

    if(!generateFlatFile) {

        // we would like to correct the composite width and height, so open the file to find it

        if(!getWidthFromIncludeFile(displayInfo, newFileName, &compositeFileWidth, &compositeFileHeight)) {
            printf("\ncompositeFileParse: Cannot open file: %s\n", newFileName);
        }

        object->height = compositeFileHeight;
        object->width = compositeFileWidth;
        writeRectangleDimensions(object, frameoffset, "caFrame", True);  // delayed write

        newoffset->frameX = newoffset->frameX - frameoffset->frameX;
        newoffset->frameY = newoffset->frameY - frameoffset->frameY;
        newoffset->frameHeight = compositeFileHeight;
        newoffset->frameWidth = compositeFileWidth;

        writeRectangleDimensions(object, newoffset, "caInclude", False);

        Qt_handleString("filename", "string", filename);
        Qt_handleString("macro", "string", macroString);

        return;
    }

    // generate flat file, not always so good

    // we would like to correct the composite width and height, so open the file to find it

    if(!getWidthFromIncludeFile(displayInfo, newFileName, &compositeFileWidth, &compositeFileHeight)) {
        printf("\ncompositeFileParse: Cannot open file: %s\n", newFileName);
        object->height = compositeFileHeight;
        object->width = compositeFileWidth;
        writeRectangleDimensions(object, frameoffset, "caFrame", True);  // delayed write
        return;
    }

    object->height = compositeFileHeight;
    object->width = compositeFileWidth;
    writeRectangleDimensions(object, frameoffset, "caFrame", True);  // delayed write

    // now use the original parsing

    //printf("compositeFileParse file=%s macro=%s\n", filename, macroString);

    savedFilePtr = displayInfo->filePtr;
    savedVersionNumber = displayInfo->versionNumber;

    // Open the file
    filePtr = fopen(newFileName, "r");
    if(!filePtr) {
        printf("\ncompositeFileParse: Cannot open file: %s\n", newFileName);
        return;
    }

    parsingCompositeFile = True;

    displayInfo->filePtr = filePtr;
    // Only do this if there is a macro string, otherwise use the  existing macros
    if(*macroString) {
        int numPairs;
        savedNameValueTable = displayInfo->nameValueTable;
        savedNumNameValues = displayInfo->numNameValues;
        displayInfo->nameValueTable =
                generateNameValueTable(macroString, &numPairs);
        displayInfo->numNameValues = numPairs;
        Qt_handleString("macro", "string", macroString);
    }

    // Read the file block (Must be there)
    // If first token isn't "file" then bail out
    tokenType=getToken(displayInfo,token);
    if(tokenType == T_WORD && !strcmp(token,"file")) {
        parseAndSkip(displayInfo);
    } else {
        printf("compositeFileParse: Invalid .adl file (First block is not file block) file: %s\n",filename);
        fclose(filePtr);
        goto RETURN;
    }

    // Read the display block
    tokenType=getToken(displayInfo,token);
    if(tokenType == T_WORD && !strcmp(token,"display")) {
        parseAndSkip(displayInfo);
        tokenType=getToken(displayInfo,token);
    }

    // Read the colormap
    if(tokenType == T_WORD &&
            (!strcmp(token,"color map") ||
             !strcmp(token,"<<color map>>"))) {
        parseAndSkip(displayInfo);
        tokenType=getToken(displayInfo,token);
    }

    // Proceed with parsing
    while(parseAndAppendDisplayList(displayInfo, newoffset, token, tokenType) != T_EOF) {
        tokenType=getToken(displayInfo,token);
    }

RETURN:

    // Restore displayInfo file parameters
    displayInfo->filePtr = savedFilePtr;
    displayInfo->versionNumber = savedVersionNumber;
    if(*macroString) {
        free ((char *)displayInfo->nameValueTable);
        displayInfo->nameValueTable = savedNameValueTable;
        displayInfo->numNameValues = savedNumNameValues;
    }
    parsingCompositeFile = False;
    fclose(filePtr);
}

void *parseComposite(DisplayInfo *displayInfo, FrameOffset *offset)
{
    char token[MAX_TOKEN_LENGTH];
    TOKEN tokenType;
    int nestingLevel = 0;
    DlObject *object;
    char compositeName[MAX_TOKEN_LENGTH];
    char compositeFile[MAX_TOKEN_LENGTH];
    int visibilityStatic = 0;
    int includeSet = False;

    static int number = 0;
    char widgetName[MAX_ASCII];
    FrameOffset *frameoffset;
    FrameOffset *actoffset;
    FrameOffset *newoffset;

    object = (DlObject *)malloc(sizeof(DlObject));
    memset(object, 0, sizeof(DlObject));

    frameoffset = (FrameOffset *)malloc(sizeof(FrameOffset));
    actoffset = (FrameOffset *)malloc(sizeof(FrameOffset));
    memset(frameoffset, 0, sizeof(FrameOffset));
    memset(actoffset, 0, sizeof(FrameOffset));

    do {
        switch(tokenType=getToken(displayInfo,token)) {
        case T_WORD:

            if(!strcmp(token,"object")) {

                parseObject(displayInfo, object);
                // do not write yet any dimensions, have to be delayed for composite file to add width and height
                frameoffset = offset;
                actoffset->frameX = object->x;
                actoffset->frameY = object->y;
                offset=actoffset;
                /*
                printf("parseobject object=%d %d actoffset=%d %d\n", object->x, object->y,
                                                                     actoffset->frameX, actoffset->frameY);
*/
            } else if(!strcmp(token,"dynamic attribute")) {
                // dynamic attribute is normally after composite file
                if(!includeSet) {
                    parseDynamicAttribute(displayInfo, "caFrame", &visibilityStatic);
                } else {
                    parseDynamicAttribute(displayInfo, "caInclude", &visibilityStatic);
                }

            } else if(!strcmp(token,"composite name")) {

                getToken(displayInfo,token);
                getToken(displayInfo,token);
                strcpy(compositeName,token);

            } else if(!strcmp(token,"composite file")) {

                newoffset = (FrameOffset *)malloc(sizeof(FrameOffset));
                getToken(displayInfo,token);
                getToken(displayInfo,token);
                strcpy(compositeFile,token);
                // will be an caInclude
                if(!generateFlatFile) {
                    includeSet = True;

                    sprintf(widgetName, "caInclude_%d", number++);
                    Qt_writeOpenTag("widget", "caInclude", widgetName);

                    newoffset->frameX = object->x;
                    newoffset->frameY = object->y;
                    newoffset->frameWidth = object->width;
                    newoffset->frameHeight = object->height;
                    // will be a caFrame
                } else {

                    sprintf(widgetName, "caFrame_%d", number++);
                    Qt_writeOpenTag("widget", "caFrame", widgetName);

                    newoffset->frameX = 0;
                    newoffset->frameY = 0;
                    newoffset->frameWidth = object->width;
                    newoffset->frameHeight = object->height;
                }
                /*
                printf("parsefile object=%d %d frameoffset=%d %d newoffset = %d %d\n", object->x, object->y,
                                                                                      frameoffset->frameX, frameoffset->frameY,
                                                                                       newoffset->frameX, newoffset->frameY);
*/
                compositeFileParse(displayInfo, compositeFile, object, frameoffset, newoffset);

            } else if(!strcmp(token,"children")) {
				FrameOffset *newoffset;
                sprintf(widgetName, "caFrame_%d", number++);
                Qt_writeOpenTag("widget", "caFrame", widgetName);

                newoffset = (FrameOffset *)malloc(sizeof(FrameOffset));
                newoffset->frameX = offset->frameX;
                newoffset->frameY = offset->frameY;
                /*
                printf("children object=%d %d newoffset = %d %d\n", object->x, object->y, newoffset->frameX, newoffset->frameY);
*/
                tokenType=getToken(displayInfo,token);
                writeRectangleDimensions(object, frameoffset, "caFrame", True);  // delayed write
                parseAndAppendDisplayList(displayInfo, newoffset, token, tokenType);
            }
            break;
        case T_EQUAL:
            break;
        case T_LEFT_BRACE:
            nestingLevel++;
            break;
        case T_RIGHT_BRACE:
            nestingLevel--;
            break;
        default:
            break;
        }
    } while( (tokenType != T_RIGHT_BRACE) && (nestingLevel > 0)
             && (tokenType != T_EOF) );

    Qt_writeCloseTag("widget", widgetName, visibilityStatic);

    return (void*) 0;
}

#define INITIAL_NUM_POINTS 16
void parsePolygonPoints(DisplayInfo *displayInfo, char *widget, int offsetX, int offsetY)
{
    char token[MAX_TOKEN_LENGTH];
    TOKEN tokenType;
    int nestingLevel;
    int x, y;
    char points[LONGSTRING];

    UNUSED(widget);

    points[0] = '\0';
    nestingLevel = 0;
    do {
        switch( (tokenType=getToken(displayInfo,token)) ) {
        case T_WORD:

            if(!strcmp(token,"(")) {

                getToken(displayInfo,token);
                x = atoi(token);
                getToken(displayInfo,token);	// separator
                getToken(displayInfo,token);
                y = atoi(token);
                getToken(displayInfo,token);	//   ")"
                sprintf(points, "%s%d,%d;", points,  x-offsetX, y-offsetY);
            }

            break;
        case T_EQUAL:
            break;
        case T_LEFT_BRACE:
            nestingLevel++;
            break;
        case T_RIGHT_BRACE:
            nestingLevel--;
            break;
        default:
            break;
        }
    } while( (tokenType != T_RIGHT_BRACE) && (nestingLevel > 0)
             && (tokenType != T_EOF) );

    if(strlen(points) > 0) Qt_handleString("xyPairs", "string", points);

}

void *parseWheelSwitch(DisplayInfo *displayInfo, FrameOffset * offset)
{
    char token[MAX_TOKEN_LENGTH];
    char COLORMODE[MAX_TOKEN_LENGTH] = "static";
    TOKEN tokenType;
    int nestingLevel = 0;
    DlObject object={0,0,0,0};
    int visibilityStatic = 2; // top layer
    int formatFound=False;

    static int number = 0;
    char widgetName[MAX_ASCII];
    sprintf(widgetName, "caNumeric_%d", number++);
    Qt_writeOpenTag("widget", "caNumeric", widgetName);

    do {
        switch( (tokenType=getToken(displayInfo,token)) ) {
        case T_WORD:
            if(!strcmp(token,"object")) {
                parseObject(displayInfo, &object);
                writeRectangleDimensions(&object, offset, "caNumeric", True);
            } else if(!strcmp(token,"control")) {
                parseControl(displayInfo, "caNumeric");
            } else if(!strcmp(token,"clrmod")) {
                getToken(displayInfo,token);
                getToken(displayInfo,token);
                strcpy(COLORMODE, token);
            } else if(!strcmp(token,"limits")) {
                parseLimits(displayInfo, "caNumeric", 0, formatFound);
            } else if(!strcmp(token,"format")) {
                getToken(displayInfo,token);
                getToken(displayInfo,token);
                formatFound = True;
                Qt_setWheelSwitchForm("", token);
                Qt_setPrecisionSource("caNumeric", 0, "Default");
                Qt_handleString("fixedFormat", "bool", "true");
            }
            break;
        case T_EQUAL:
            break;
        case T_LEFT_BRACE:
            nestingLevel++;
            break;
        case T_RIGHT_BRACE:
            nestingLevel--;
            break;
        default:
            break;
        }
    } while( (tokenType != T_RIGHT_BRACE) && (nestingLevel > 0)
             && (tokenType != T_EOF) );

    Qt_handleString("autoFillBackground", "bool", "true");
    Qt_handleString("digitsFontScaleEnabled", "bool", "true");
    if(!formatFound) Qt_handleString("fixedFormat", "bool", "false");

    Qt_writeCloseTag("widget", widgetName, visibilityStatic);

    return (void*) 0;
}


void *parsePolygon(DisplayInfo *displayInfo, FrameOffset * offset)
{
    char token[MAX_TOKEN_LENGTH];
    TOKEN tokenType;
    int nestingLevel = 0;
    DlObject object={0,0,0,0};
    int visibilityStatic = 0;
    int offsetX=0, offsetY=0;

    static int number = 0;
    char widgetName[MAX_ASCII];
    sprintf(widgetName, "caPolygon_%d", number++);
    Qt_writeOpenTag("widget", "caPolyLine", widgetName);

    do {
        switch( (tokenType=getToken(displayInfo,token)) ) {
        case T_WORD:
            if(!strcmp(token,"object")) {
                parseObject(displayInfo, &object);
                writeRectangleDimensions(&object, offset, "caPolyLine", True);
                offsetX = object.x;
                offsetY = object.y;
            } else {
                if(!strcmp(token,"basic attribute"))
                    parseBasicAttribute(displayInfo, "caPolyLine");
                else
                    if(!strcmp(token,"dynamic attribute"))
                        parseDynamicAttribute(displayInfo, "caPolyLine", &visibilityStatic);
                    else
                        if(!strcmp(token,"points"))
                            parsePolygonPoints(displayInfo, "caPolyLine", offsetX, offsetY);
            }
            break;
        case T_EQUAL:
            break;
        case T_LEFT_BRACE:
            nestingLevel++;
            break;
        case T_RIGHT_BRACE:
            nestingLevel--;
            break;
        default:
            break;
        }
    } while( (tokenType != T_RIGHT_BRACE) && (nestingLevel > 0)
             && (tokenType != T_EOF) );

    Qt_handleString("polystyle", "enum", "caPolyLine::Polygon");

    Qt_writeCloseTag("widget", widgetName, visibilityStatic);

    return (void*) 0;
}

void *parseRectangle(DisplayInfo *displayInfo, FrameOffset * offset)
{
    char token[MAX_TOKEN_LENGTH];
    TOKEN tokenType;
    int nestingLevel = 0;
    DlObject object={0,0,0,0};
    int visibilityStatic = 0;

    static int number = 0;
    char widgetName[MAX_ASCII];
    sprintf(widgetName, "caRectangle_%d", number++);
    Qt_writeOpenTag("widget", "caGraphics", widgetName);

    Qt_handleString("form", "enum", "caGraphics::Rectangle");

    do {
        switch( (tokenType=getToken(displayInfo,token)) ) {
        case T_WORD:
            if(!strcmp(token,"object")) {
                parseObject(displayInfo, &object);
                writeRectangleDimensions(&object, offset, "caGraphics", True);
            } else if(!strcmp(token,"basic attribute")) {
                parseBasicAttribute(displayInfo, "caGraphics");
            } else if(!strcmp(token,"dynamic attribute")) {
                parseDynamicAttribute(displayInfo, "caGraphics", &visibilityStatic);
            }
            break;
        case T_EQUAL:
            break;
        case T_LEFT_BRACE:
            nestingLevel++;
            break;
        case T_RIGHT_BRACE:
            nestingLevel--;
            break;
        default:
            break;
        }
    } while( (tokenType != T_RIGHT_BRACE) && (nestingLevel > 0)
             && (tokenType != T_EOF) );

    Qt_writeCloseTag("widget", widgetName, visibilityStatic);

    return (void*) 0;
}

void *parseOval(DisplayInfo *displayInfo, FrameOffset * offset)
{
    char token[MAX_TOKEN_LENGTH];
    TOKEN tokenType;
    int nestingLevel = 0;
    DlObject object={0,0,0,0};
    int visibilityStatic = 0;

    static int number = 0;
    char widgetName[MAX_ASCII];
    sprintf(widgetName, "caGraphics_%d", number++);
    Qt_writeOpenTag("widget", "caGraphics", widgetName);

    Qt_handleString("form", "enum", "caGraphics::Circle");

    do {
        switch( (tokenType=getToken(displayInfo,token)) ) {
        case T_WORD:
            if(!strcmp(token,"object")) {
                parseObject(displayInfo, &object);
                writeRectangleDimensions(&object, offset, "caGraphics", True);
            } else if(!strcmp(token,"basic attribute")) {
                parseBasicAttribute(displayInfo, "caGraphics");
            } else if(!strcmp(token,"dynamic attribute")) {
                parseDynamicAttribute(displayInfo, "caGraphics", &visibilityStatic);
            }
            break;
        case T_EQUAL:
            break;
        case T_LEFT_BRACE:
            nestingLevel++;
            break;
        case T_RIGHT_BRACE:
            nestingLevel--;
            break;
        default:
            break;
        }
    } while( (tokenType != T_RIGHT_BRACE) && (nestingLevel > 0)
             && (tokenType != T_EOF) );

    Qt_writeCloseTag("widget", widgetName, visibilityStatic);

    return (void*) 0;
}

void *parseArc(DisplayInfo *displayInfo, FrameOffset * offset)
{
    char token[MAX_TOKEN_LENGTH];
    char asc[MAX_ASCII];
    TOKEN tokenType;
    int nestingLevel = 0;
    DlObject object = {0,0,0,0};
    int visibilityStatic = 0;
    int startAngle = 0;
    int spanAngle = 0;

    static int number = 0;
    char widgetName[MAX_ASCII];

    sprintf(widgetName, "caArc_%d", number++);
    Qt_writeOpenTag("widget", "caGraphics", widgetName);

    Qt_handleString("form", "enum", "caGraphics::Arc");

    do {
        switch( (tokenType=getToken(displayInfo,token)) ) {
        case T_WORD:
            if (!strcmp(token,"object"))
                parseObject(displayInfo, &object);
            writeRectangleDimensions(&object, offset, "caGraphics", True);
            if (!strcmp(token,"basic attribute")) {
                parseBasicAttribute(displayInfo, "caGraphics");
            } else {
                if (!strcmp(token,"dynamic attribute")) {
                    parseDynamicAttribute(displayInfo, "caGraphics", &visibilityStatic);
                } else {
                    if (!strcmp(token,"begin")) {
                        getToken(displayInfo,token);
                        getToken(displayInfo,token);
                        startAngle = atoi(token);
                        sprintf(asc, "%d", startAngle / 64);
                        Qt_handleString("startAngle", "number", asc);
                    } else
                        if (!strcmp(token,"path")) {
                            getToken(displayInfo,token);
                            getToken(displayInfo,token);
                            spanAngle = atoi(token);
                            sprintf(asc, "%d", spanAngle / 64);
                            Qt_handleString("spanAngle", "number", asc);
                        }
                }
            }
            break;
        case T_EQUAL:
            break;
        case T_LEFT_BRACE:
            nestingLevel++;
            break;
        case T_RIGHT_BRACE:
            nestingLevel--;
            break;
        default:
            break;
        }
    } while( (tokenType != T_RIGHT_BRACE) && (nestingLevel > 0)
             && (tokenType != T_EOF) );

    Qt_writeCloseTag("widget", widgetName, visibilityStatic);

    return (void*) 0;
}


void *parseText(DisplayInfo *displayInfo, FrameOffset * offset)
{
    char token[MAX_TOKEN_LENGTH];
    char textix[MAX_TOKEN_LENGTH];
    TOKEN tokenType;
    int nestingLevel = 0;
    DlObject object={0,0,0,0};
    int alignFound=False;
    int visibilityStatic = 0;
    unsigned int newWidth = 0;

    static int number = 0;
    char widgetName[MAX_ASCII];
    sprintf(widgetName, "caLabel_%d", number++);
    Qt_writeOpenTag("widget", "caLabel", widgetName);

    Qt_handleString("frameShape", "enum", "QFrame::NoFrame");

    textix[0] = '\0';

    do {
        switch( (tokenType=getToken(displayInfo,token)) ) {
        case T_WORD:
            if(!strcmp(token,"object")) {
                parseObject(displayInfo, &object);
                //writeRectangleDimensions(&object, offset, "caLabel", True);
            } else if(!strcmp(token,"basic attribute")) {
                parseBasicAttribute(displayInfo, "caLabel");
            } else if(!strcmp(token,"dynamic attribute")) {
                parseDynamicAttribute(displayInfo, "caLabel", &visibilityStatic);
            }
            else if(!strcmp(token,"textix")) {
                getToken(displayInfo,token);
                getToken(displayInfo,token);
                strcpy(textix, token);
                Qt_handleString("text", "string",  token);
                Qt_handleString("fontScaleMode", "enum",  "ESimpleLabel::WidthAndHeight");
            } else if(!strcmp(token,"align")) {

                getToken(displayInfo,token);
                getToken(displayInfo,token);

                if(strstr(token, "left") != (char*) 0) {
                    alignFound = True;
                    Qt_handleString("alignment", "set", "Qt::AlignAbsolute|Qt::AlignLeft|Qt::AlignVCenter");
                } else if(strstr(token, "centered") != (char*) 0) {
                    alignFound = True;
                    Qt_handleString("alignment", "set", "Qt::AlignAbsolute|Qt::AlignHCenter|Qt::AlignVCenter");
                } else if(strstr(token, "right") != (char*) 0) {
                    alignFound = True;
                    Qt_handleString("alignment", "set", "Qt::AlignAbsolute|Qt::AlignRight|Qt::AlignVCenter");
                }
            }
            break;
        case T_EQUAL:
            break;
        case T_LEFT_BRACE:
            nestingLevel++;
            break;
        case T_RIGHT_BRACE:
            nestingLevel--;
            break;
        default:
            break;
        }
    } while( (tokenType != T_RIGHT_BRACE) && (nestingLevel > 0)
             && (tokenType != T_EOF) );

    if(strlen(textix) > 0  && expandText) {
        newWidth = .6 * object.height * strlen(textix);
        if(newWidth > object.width) object.width = newWidth;
    }

    writeRectangleDimensions(&object, offset, "caLabel", True);

    if(!alignFound) Qt_handleString("alignment", "set", "Qt::AlignAbsolute|Qt::AlignLeft|Qt::AlignVCenter");

    Qt_writeCloseTag("widget", widgetName, visibilityStatic);

    return (void*) 0;
}

void parsePolylinePoints(DisplayInfo *displayInfo, char *widget, int offsetX, int offsetY)
{
    char token[MAX_TOKEN_LENGTH];
    TOKEN tokenType;
    int nestingLevel;
    int x, y;
    char points[LONGSTRING];

    UNUSED(widget);

    points[0] = '\0';
    nestingLevel = 0;
    do {
        switch( (tokenType=getToken(displayInfo,token)) ) {
        case T_WORD:
            if(!strcmp(token,"(")) {

                getToken(displayInfo,token);
                x = atoi(token);
                getToken(displayInfo,token);	// separator
                getToken(displayInfo,token);
                y = atoi(token);
                getToken(displayInfo,token);	//   ")"
                sprintf(points, "%s%d,%d;", points,  x-offsetX, y-offsetY);
            }
            break;
        case T_EQUAL:
            break;
        case T_LEFT_BRACE:
            nestingLevel++;
            break;
        case T_RIGHT_BRACE:
            nestingLevel--;
            break;
        default:
            break;
        }
    } while( (tokenType != T_RIGHT_BRACE) && (nestingLevel > 0)
             && (tokenType != T_EOF) );

    if(strlen(points) > 0) Qt_handleString("xyPairs", "string", points);

}


void *parsePolyline(DisplayInfo *displayInfo, FrameOffset * offset)
{
    char token[MAX_TOKEN_LENGTH];
    TOKEN tokenType;
    int nestingLevel = 0;
    DlObject object={0,0,0,0};
    int offsetX=0, offsetY=0;
    int visibilityStatic = 0;

    static int number = 0;
    char widgetName[MAX_ASCII];
    sprintf(widgetName, "caPolyLine_%d", number++);
    Qt_writeOpenTag("widget", "caPolyLine", widgetName);

    do {
        switch( (tokenType=getToken(displayInfo,token)) ) {
        case T_WORD:
            if(!strcmp(token,"object")) {
                parseObject(displayInfo, &object);
                writeRectangleDimensions(&object, offset, "caPolyLine", True);
                offsetX = object.x;
                offsetY = object.y;
            } else if(!strcmp(token,"basic attribute")) {
                parseBasicAttribute(displayInfo, "caPolyLine");
            } else if(!strcmp(token,"dynamic attribute")) {
                parseDynamicAttribute(displayInfo, "caPolyLine", &visibilityStatic);
            } else if(!strcmp(token,"points")) {
                parsePolylinePoints(displayInfo, "caPolyLine", offsetX, offsetY);
            }
            break;
        case T_EQUAL:
            break;
        case T_LEFT_BRACE:
            nestingLevel++;
            break;
        case T_RIGHT_BRACE:
            nestingLevel--;
            break;
        default:
            break;
        }
    } while( (tokenType != T_RIGHT_BRACE) && (nestingLevel > 0)
             && (tokenType != T_EOF) );

    Qt_writeCloseTag("widget", widgetName, visibilityStatic);

    return (void*) 0;
}

void *parseBar(DisplayInfo *displayInfo, FrameOffset * offset)
{
    char token[MAX_TOKEN_LENGTH];
    char COLORMODE[MAX_TOKEN_LENGTH] = "static";
    char look[MAX_TOKEN_LENGTH] = "noDeco";
    char direction[MAX_TOKEN_LENGTH] = "Right";
    TOKEN tokenType;
    int nestingLevel = 0;
    DlObject object={0,0,0,0};
    int visibilityStatic = 2; // top layer

    static int number = 0;
    char widgetName[MAX_ASCII];
    sprintf(widgetName, "caThermo_%d", number++);
    Qt_writeOpenTag("widget", "caThermo", widgetName);

    do {
        switch( (tokenType=getToken(displayInfo,token)) ) {
        case T_WORD:
            if(!strcmp(token,"object")) {
                parseObject(displayInfo, &object);
                if(object.width < 3) object.width=3;
                if(object.height < 3) object.height=3;
                writeRectangleDimensions(&object, offset, "caThermo", True);
            } else if(!strcmp(token,"monitor")) {
                parseMonitor(displayInfo, "caThermo");
            } else if(!strcmp(token,"label")) {
                getToken(displayInfo,token);
                getToken(displayInfo,token);
                if(!strcmp(token,"none")) {
                    strcpy(look, "noLabel");
                } else if(!strcmp(token,"no decorations")) {
                    strcpy(look, "noDeco");
                } else if(!strcmp(token,"outline")) {
                    strcpy(look, "Outline");
                } else if(!strcmp(token,"limits")) {
                    strcpy(look, "Limits");
                } else {
                    if(!strcmp(token,"channel")) {
                        strcpy(look, "channelV");
                    }
                }
            } else if(!strcmp(token,"clrmod")) {
                getToken(displayInfo,token);
                getToken(displayInfo,token);
                strcpy(COLORMODE, token);
            } else if(!strcmp(token,"direction")) {
                getToken(displayInfo,token);
                getToken(displayInfo,token);
                if(!strcmp(token,"up")) {
                    strcpy(direction, "Up");
                } else if(!strcmp(token,"right")) {
                    strcpy(direction, "Right");
                } else if(!strcmp(token,"down"))  {
                    strcpy(direction, "Down");
                } else if(!strcmp(token,"left")) {
                    strcpy(direction, "Left");
                }

            } else if(!strcmp(token,"fillmod")) {
                getToken(displayInfo,token);
                getToken(displayInfo,token);
                if(!strcmp(token,"from edge")) {
                } else if(!strcmp(token,"from center")) {
                    Qt_handleString("type", "enum", "QwtThermoMarker::PipeFromCenter");
                }
            } else if(!strcmp(token,"limits")) {
                parseLimits(displayInfo, "caThermo", 0, False);
            }
            break;
        case T_EQUAL:
            break;
        case T_LEFT_BRACE:
            nestingLevel++;
            break;
        case T_RIGHT_BRACE:
            nestingLevel--;
            break;
        default:
            break;
        }
    } while( (tokenType != T_RIGHT_BRACE) && (nestingLevel > 0)
             && (tokenType != T_EOF) );

    Qt_handleString("direction", "enum", direction);
    Qt_handleString("look", "enum", look);

    if(!strcmp(look,"noLabel") || !strcmp(look,"noDeco")) {
        char pipeWidth[10];
        Qt_handleString("scalePosition", "enum", "NoScale");
        if(!strcmp(direction, "Up") || !strcmp(direction, "Down")) {
            sprintf(pipeWidth, "%d", object.width);
            Qt_handleString("pipeWidth", "number", pipeWidth);
        } else {
            sprintf(pipeWidth, "%d", object.height);
            Qt_handleString("pipeWidth", "number", pipeWidth);
        }
    } else if(!strcmp(direction, "Up") || !strcmp(direction, "Down")) {
        Qt_handleString("scalePosition", "enum", "LeftScale");
    } else if(!strcmp(direction, "Right") || !strcmp(direction, "Left")) {
        Qt_handleString("scalePosition", "enum", "BottomScale");
    }

    Qt_setColorMode("caThermo", COLORMODE);

    Qt_writeCloseTag("widget", widgetName, visibilityStatic);

    return (void *) 0;
}

void *parseIndicator(DisplayInfo *displayInfo, FrameOffset * offset)
{
    char token[MAX_TOKEN_LENGTH];
    char COLORMODE[MAX_TOKEN_LENGTH] = "static";
    char look[MAX_TOKEN_LENGTH] = "noDeco";
    char direction[MAX_TOKEN_LENGTH] = "Right";
    TOKEN tokenType;
    int nestingLevel = 0;
    DlObject object={0,0,0,0};
    int visibilityStatic = 2; // top layer
	char pipeWidth[10];
    static int number = 0;
    char widgetName[MAX_ASCII];
    sprintf(widgetName, "caThermoM_%d", number++);
    Qt_writeOpenTag("widget", "caThermo", widgetName);

    do {
        switch( (tokenType=getToken(displayInfo,token)) ) {
        case T_WORD:
            if(!strcmp(token,"object")) {
                parseObject(displayInfo, &object);
                writeRectangleDimensions(&object, offset, "caThermo", True);
            } else if(!strcmp(token,"monitor")) {
                parseMonitor(displayInfo, "caThermo");
            } else if(!strcmp(token,"label")) {
                getToken(displayInfo,token);
                getToken(displayInfo,token);
                if(!strcmp(token,"none")) {
                    strcpy(look, "noLabel");
                } else if(!strcmp(token,"no decorations")) {
                    strcpy(look, "noDeco");
                } else if(!strcmp(token,"outline")) {
                    strcpy(look, "Outline");
                } else if(!strcmp(token,"limits")) {
                    strcpy(look, "Limits");
                } else {
                    if(!strcmp(token,"channel")) {
                        strcpy(look, "channelV");
                    }
                }
            } else if(!strcmp(token,"clrmod")) {
                getToken(displayInfo,token);
                getToken(displayInfo,token);
                strcpy(COLORMODE, token);
            } else if(!strcmp(token,"direction")) {
                getToken(displayInfo,token);
                getToken(displayInfo,token);
                if(!strcmp(token,"up")) {
                    strcpy(direction, "Up");
                } else if(!strcmp(token,"right")) {
                    strcpy(direction, "Right");
                } else if(!strcmp(token,"down"))  {
                    strcpy(direction, "Down");
                } else if(!strcmp(token,"left")) {
                    strcpy(direction, "Left");
                }
            } else if(!strcmp(token,"limits")) {
                parseLimits(displayInfo, "caThermo", 0, False);
            }
            break;
        case T_EQUAL:
            break;
        case T_LEFT_BRACE:
            nestingLevel++;
            break;
        case T_RIGHT_BRACE:
            nestingLevel--;
            break;
        default:
            break;
        }
    } while( (tokenType != T_RIGHT_BRACE) && (nestingLevel > 0)
             && (tokenType != T_EOF) );

    Qt_handleString("direction", "enum", direction);
    Qt_handleString("look", "enum", look);

    if(!strcmp(look,"noLabel") || !strcmp(look,"noDeco")) {

        Qt_handleString("scalePosition", "enum", "NoScale");
        Qt_handleString("scalePosition", "enum", "NoScale");
        if(!strcmp(direction, "Up") || !strcmp(direction, "Down")) {
            sprintf(pipeWidth, "%d", object.width-4);
            Qt_handleString("pipeWidth", "number", pipeWidth);
        } else {
            sprintf(pipeWidth, "%d", object.height-4);
            Qt_handleString("pipeWidth", "number", pipeWidth);
        }
    } else if(!strcmp(direction, "Up") || !strcmp(direction, "Down")) {
        Qt_handleString("scalePosition", "enum", "LeftScale");
    } else if(!strcmp(direction, "Right") || !strcmp(direction, "Left")) {
        Qt_handleString("scalePosition", "enum", "BottomScale");
    }

    Qt_handleString("type", "enum", "QwtThermoMarker::Marker");

    Qt_setColorMode("caThermo", COLORMODE);

    Qt_writeCloseTag("widget", widgetName, visibilityStatic);

    return (void*) 0;
}


TOKEN getToken(DisplayInfo *displayInfo, char *word)
{
    FILE *filePtr;
    enum {NEUTRAL,INQUOTE,INWORD,INMACRO} state = NEUTRAL, savedState = NEUTRAL;
    int c;
    char *w, *value;
    char *m, macro[MAX_TOKEN_LENGTH];
    int j;

    filePtr = displayInfo->filePtr;

    w = word;
    m = macro;

    while( (c=getc(filePtr)) != EOF) {

        switch (state) {
        case NEUTRAL:
            switch(c) {
            case '=' : return(T_EQUAL);
            case '{' : return(T_LEFT_BRACE);
            case '}' : return(T_RIGHT_BRACE);
            case '"' : state = INQUOTE;
                break;
                /*
     case '$' : c=getc(filePtr);
                // only do macro substitution if in execute mode or parsing a composite file

                if((parsingCompositeFile) && c == '(' ) {
      state = INMACRO;
  } else {
      *w++ = '$';
      *w++ = c;
  }
*/
                break;
            case ' ' :
            case '\t':
            case '\r':
            case '\n': break;

                /* for constructs of the form (a,b) */
            case '(' :
            case ',' :
            case ')' : *w++ = c; *w = '\0'; return(T_WORD);

            default  : state = INWORD;
                *w++ = c;
                break;
            }
            break;
        case INQUOTE:
            switch(c) {
            case '"' : *w = '\0'; return(T_WORD);
                break;
            default  : *w++ = c;
                break;
            }
            break;
        case INMACRO:
            switch(c) {
            case ')' : *m = '\0';
                value = lookupNameValue(displayInfo->nameValueTable,
                                        displayInfo->numNameValues,macro);
                if(value != NULL) {
                    for(j = 0; j < (int) strlen(value); j++) {
                        *w++ = value[j];
                    }
                } else {
                    *w++ = '$';
                    *w++ = '(';
                    for(j = 0; j < (int) strlen(macro); j++) {
                        *w++ = macro[j];
                    }
                    *w++ = ')';
                }
                state = savedState;
                m = macro;
                break;

            default  : *m++ = c;
                break;
            }
            break;
        case INWORD:
            switch(c) {
            case ' ' :
            case '\r':
            case '\n':
            case '\t':
            case '=' :
            case '(' :
            case ',' :
            case ')' :
            case '"' : ungetc(c,filePtr); *w = '\0'; return(T_WORD);
            default  : *w++ = c;
                break;
            }
            break;
        }
    }

    return(T_EOF);
}

/***  Parsing routines ***/

void *getNextElement(DisplayInfo *pDI, char *token, FrameOffset *offset) {
    int i;
    for(i=0; i < parseFuncTableSize; i++) {
        if(!strcmp(token,parseFuncTable[i].name)) {
            return (void *) parseFuncTable[i].func(pDI, offset);
        }
    }
    return 0;
}

TOKEN parseAndAppendDisplayList(DisplayInfo *displayInfo, FrameOffset *offset, char *firstToken, TOKEN firstTokenType)
{
    TOKEN tokenType;
    char token[MAX_TOKEN_LENGTH];
    int nestingLevel = 0;
    int first = 1;
    static int veryFirst = True;
    int bclr = 0;

    if(veryFirst) {
        veryFirst = False;
        bclr = displayInfo->drawingAreaBackgroundColor;

        Qt_writeStyleSheet(displayInfo->dlColormap->dl_color[bclr].r,
                           displayInfo->dlColormap->dl_color[bclr].g,
                           displayInfo->dlColormap->dl_color[bclr].b);

        Qt_writeOpenTag("widget", "QWidget", "centralWidget");
    }

    /* Loop over tokens until T_EOF */
    do {
        if(first) {
            tokenType=firstTokenType;
            strcpy(token,firstToken);
            first = 0;
        } else {
            tokenType=getToken(displayInfo,token);
        }
        switch(tokenType) {
        case T_WORD : {
            getNextElement(displayInfo, token, offset);
            break;
        }
        case T_EQUAL:
            break;
        case T_LEFT_BRACE:
            nestingLevel++; break;
        case T_RIGHT_BRACE:
            nestingLevel--; break;
        default :
            break;
        }
    } while((tokenType != T_RIGHT_BRACE) && (nestingLevel > 0)
            && (tokenType != T_EOF));

    /* Reset the init flag */
    return tokenType;
}

void *parseDisplay(DisplayInfo *displayInfo)
{
    char token[MAX_TOKEN_LENGTH];
    TOKEN tokenType;
    int nestingLevel = 0;
    DlObject object={0,0,0,0};
    int clr, bclr;
    char cmap[MAX_TOKEN_LENGTH];

    FrameOffset offset ={0,0,0,0};

    do {
        switch( (tokenType=getToken(displayInfo,token)) ) {
        case T_WORD:
            if(!strcmp(token,"object")) {
                parseObject(displayInfo, &object);
                writeRectangleDimensions(&object, &offset, "", True);
            } else if(!strcmp(token,"grid")) {
                parseGrid(displayInfo, "");
            } else if(!strcmp(token,"cmap")) {
                /* Parse separate display list to get and use that colormap */
                getToken(displayInfo,token);
                getToken(displayInfo,token);
                if(strlen(token) > (size_t) 0) {
                    strcpy(cmap,token);
                }
            } else if(!strcmp(token,"bclr")) {
                getToken(displayInfo,token);
                getToken(displayInfo,token);
                bclr = atoi(token) % DL_MAX_COLORS;
                displayInfo->drawingAreaBackgroundColor = bclr;
            } else if(!strcmp(token,"clr")) {
                getToken(displayInfo,token);
                getToken(displayInfo,token);
                clr = atoi(token) % DL_MAX_COLORS;
                displayInfo->drawingAreaForegroundColor = clr;
            } else if(!strcmp(token,"gridSpacing")) {
                getToken(displayInfo,token);
                getToken(displayInfo,token);
            } else if(!strcmp(token,"gridOn")) {
                getToken(displayInfo,token);
                getToken(displayInfo,token);
            } else if(!strcmp(token,"snapToGrid")) {
                getToken(displayInfo,token);
                getToken(displayInfo,token);
            }
            break;
        case T_EQUAL:
            break;
        case T_LEFT_BRACE:
            nestingLevel++;
            break;
        case T_RIGHT_BRACE:
            nestingLevel--;
            break;
        default:
            break;
        }
    } while( (tokenType != T_RIGHT_BRACE) && (nestingLevel > 0)
             && (tokenType != T_EOF) );

    return (void*) 0;
}


void parseFile(DisplayInfo *displayInfo)
{
    char token[MAX_TOKEN_LENGTH];
    TOKEN tokenType;
    int nestingLevel = 0;

    do {
        switch( (tokenType=getToken(displayInfo,token)) ) {
        case T_WORD:
            if(!strcmp(token,"name")) {
                getToken(displayInfo,token);
                getToken(displayInfo,token);
            }
            if(!strcmp(token,"version")) {
                getToken(displayInfo,token);
                getToken(displayInfo,token);
            }
            break;
        case T_EQUAL:
            break;
        case T_LEFT_BRACE:
            nestingLevel++;
            break;
        case T_RIGHT_BRACE:
            nestingLevel--;
            break;
        default:
            break;
        }
    } while( (tokenType != T_RIGHT_BRACE) && (nestingLevel > 0)
             && (tokenType != T_EOF) );

    //return dlFile;
}

void parseDlColor(DisplayInfo *displayInfo, FILE *filePtr,
                  DlColormapEntry *dlColor)
{
    char token[MAX_TOKEN_LENGTH];
    TOKEN tokenType;
    int nestingLevel = 0;
    int counter = 0;

    FILE *savedFilePtr;

    /* (MDA) have to be sneaky for these colormap parsing routines: *
  * since possibly external colormap, save and restore * external file
  * ptr in displayInfo so that getToken() * works with displayInfo and
  * not the filePtr directly */
    savedFilePtr = displayInfo->filePtr;
    displayInfo->filePtr = filePtr;

    do {
        switch( (tokenType=getToken(displayInfo, token)) ) {
        case T_WORD: {
            char *end;
            unsigned long color = strtoul(token,&end,16);
            if(counter < DL_MAX_COLORS) {
                dlColor[counter].r = (color & 0x00ff0000) >> 16;
                dlColor[counter].g = (color & 0x0000ff00) >> 8;
                dlColor[counter].b = color & 0x000000ff;
                counter++;
            }
            getToken(displayInfo,token);
            break;
        }
        case T_LEFT_BRACE:
            nestingLevel++;
            break;
        case T_RIGHT_BRACE:
            nestingLevel--;
            break;
        default:
            break;
        }
    } while( (tokenType != T_RIGHT_BRACE) && (nestingLevel > 0)
             && (tokenType != T_EOF) );

    /* Restore displayInfo->filePtr to previous value */
    displayInfo->filePtr = savedFilePtr;
}

void parseOldDlColor(DisplayInfo *displayInfo, FILE *filePtr,
                     DlColormapEntry *dlColor)
{
    char token[MAX_TOKEN_LENGTH];
    TOKEN tokenType;
    int nestingLevel = 0;
    FILE *savedFilePtr;

    /*
   * (MDA) have to be sneaky for these colormap parsing routines:
   *      since possibly external colormap, save and restore
   *      external file ptr in  displayInfo so that getToken()
   *      works with displayInfo and not the filePtr directly
   */
    savedFilePtr = displayInfo->filePtr;
    displayInfo->filePtr = filePtr;

    do {
        switch( (tokenType=getToken(displayInfo,token)) ) {
        case T_WORD:
            if(!strcmp(token,"r")) {
                getToken(displayInfo,token);getToken(displayInfo,token);
                dlColor->r = atoi(token);
            } else if(!strcmp(token,"g")) {
                getToken(displayInfo,token);getToken(displayInfo,token);
                dlColor->g = atoi(token);
            } else if(!strcmp(token,"b")) {
                getToken(displayInfo,token);getToken(displayInfo,token);
                dlColor->b = atoi(token);
            } else if(!strcmp(token,"inten")) {
                getToken(displayInfo,token);getToken(displayInfo,token);
                dlColor->inten = atoi(token);
            }
            break;
        case T_LEFT_BRACE:
            nestingLevel++;
            break;
        case T_RIGHT_BRACE:
            nestingLevel--;
            break;
        default:
            break;
        }
    } while( (tokenType != T_RIGHT_BRACE) && (nestingLevel > 0)
             && (tokenType != T_EOF) );

    /* and restore displayInfo->filePtr to previous value */
    displayInfo->filePtr = savedFilePtr;
}

DlColormap *createDlColormap(DisplayInfo *displayInfo)
{
    DlColormap *dlColormap;

    UNUSED(displayInfo);

    dlColormap = (DlColormap *)malloc(sizeof(DlColormap));
    if(!dlColormap) return 0;
    /* structure copy */
    *dlColormap = defaultDlColormap;

    return(dlColormap);
}


DlColormap *parseColormap(DisplayInfo *displayInfo, FILE *filePtr)
{
    char token[MAX_TOKEN_LENGTH];
    TOKEN tokenType;
    int nestingLevel = 0;
    DlColormap *dlColormap;
    DlColormapEntry dummyColormapEntry;
    DlColormapEntry dl_color[DL_MAX_COLORS];
    int counter;
    FILE *savedFilePtr;

    dlColormap = createDlColormap(displayInfo);
    if(!dlColormap) return NULL;

    savedFilePtr = displayInfo->filePtr;

    displayInfo->filePtr = filePtr;

    /* initialize some data in structure */
    dlColormap->ncolors = 0;

    /* new colormap, get values (pixel values are being stored) */
    counter = 0;

    do {
        switch( (tokenType=getToken(displayInfo, token)) ) {
        case T_WORD:
            if(!strcmp(token,"ncolors")) {
                getToken(displayInfo,token);
                getToken(displayInfo,token);
                dlColormap->ncolors = atoi(token);
                if(dlColormap->ncolors > DL_MAX_COLORS) {
                    printf("\nMaximum # of colors in colormap exceeded\n"
                           "  Will continue with truncated color space\n"
                           "(You may want to change the colors of some objects)\n");
                    //dmSetAndPopupWarningDialog(displayInfo, msg,"OK",NULL,NULL);
                }
            } else if(!strcmp(token,"dl_color")) {
                /* continue parsing but throw away "excess" colormap entries */
                if(counter < DL_MAX_COLORS) {
                    parseOldDlColor(displayInfo,filePtr,&(dl_color[counter]));
                    counter++;
                } else {
                    parseOldDlColor(displayInfo,filePtr,&dummyColormapEntry);
                    counter++;
                }
            } else if(!strcmp(token,"colors")) {
                parseDlColor(displayInfo,filePtr, dl_color);
            }
            break;
        case T_EQUAL:
            break;
        case T_LEFT_BRACE:
            nestingLevel++;
            break;
        case T_RIGHT_BRACE:
            nestingLevel--;
            break;
        default:
            break;
        }
    } while((tokenType != T_RIGHT_BRACE) && (nestingLevel > 0) &&
            (tokenType != T_EOF));

    /* restore the previous filePtr */
    displayInfo->filePtr = savedFilePtr;


    return (dlColormap);
}



