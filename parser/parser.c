#include <stdlib.h>
#include <ctype.h>
#include "parser.h"
#include "QtProperties.h"

TOKEN parseAndAppendDisplayList(DisplayInfo *displayInfo, FrameOffset *offset, char *firstToken, TOKEN firstTokenType);


/*********************************************************************
 * stringValueTable for string-valued tokens - position sensitive!   *
 * any changes of types or ordering of above must have matching      *
 * changes in this table!                                            *
 *********************************************************************/

char *stringValueTable[NUMBER_STRING_VALUES] = {
    "execute", "edit",
    "none", "no decorations", "outline", "limits", "channel",
    "static", "alarm", "discrete",
    "static", "if not zero", "if zero", "calc",
    "up", "right", "down", "left",
    "solid", "dash","bigdash",
    "solid", "outline",
    "decimal", "exponential", "engr. notation", "compact", "truncated",
    "hexadecimal", "octal", "string",
    "sexagesimal", "sexagesimal-hms", "sexagesimal-dms",
    "horiz. left", "horiz. centered", "horiz. right",
    "column", "row", "row column",
    "from edge", "from center",
    "milli-second", "second", "minute",
    "point", "line", "line-bigmarks", "line-nomarks", "line-thick", "fill-under",
    "plot n pts & stop", "plot last n pts",
    "no image", "gif", "tiff",
    "linear", "log10", "time",
    "from channel", "user-specified", "auto-scale",
    "if not zero", "if zero",
    "create new display", "replace display",
    "menu", "a row of buttons", "a column of buttons", "invisible",
    "hh:mm:ss", "hh:mm", "hh:00", "MMM DD YYYY", "MMM DD", "MMM DD hh:00",
    "wd hh:00",
    "false", "true",
    "channel", "default", "user", "unused",
};

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
void parseObject(DisplayInfo *displayInfo, DlObject *object, FrameOffset * offset, char *widget);
void parseGrid(DisplayInfo *displayInfo, char *widget);
//void parseRelatedDisplayEntry(DisplayInfo *displayInfo, DlRelatedDisplayEntry *relatedDisplay);
//void parseShellCommandEntry(DisplayInfo *displayInfo, DlShellCommandEntry *shellCommand);
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
void parseControl(DisplayInfo *);
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

/****************************************************************************/
/* string parsing                                                           */
/****************************************************************************/

typedef char string40[40];
int parseDelimited(char *s, string40 items[], int nbItems, char token)
{
  int i, cnt;
  char * pch;
  char ctoken[2];
  memcpy(ctoken, &token, 1);
  ctoken[1] ='\0';
  for (i=0; i<strlen(s); i++) if(s[i] < ' ') s[i] = '\0';
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

    //printf("parseDynAttrMod\n");
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
    //printf("parseDynAttrParam\n");
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
    //printf("parseAttr\n");
    do {
	switch( (tokenType=getToken(displayInfo,token)) ) {
	case T_WORD:
	    if(!strcmp(token,"clr")) {
		getToken(displayInfo,token);
		getToken(displayInfo,token);
                int clr = atoi(token) % DL_MAX_COLORS;
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
    //printf("parseDynamicAttr\n");
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
    //printf("parseMonitor\n");
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
                int clr = atoi(token) % DL_MAX_COLORS;

                Qt_setColorForeground("", displayInfo->dlColormap->dl_color[clr].r,
                                          displayInfo->dlColormap->dl_color[clr].g,
                                          displayInfo->dlColormap->dl_color[clr].b,
                                          255);

	    } else if (!strcmp(token,"bclr")) {
		getToken(displayInfo,token);
		getToken(displayInfo,token);
                int bclr = atoi(token) % DL_MAX_COLORS;

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

void parseBasicAttribute(DisplayInfo *displayInfo, char *widget)
{
    char token[MAX_TOKEN_LENGTH];
    TOKEN tokenType;
    int nestingLevel = 0;
    int clr=0;
    char fillstyle[80];
    char linestyle[80];

    strcpy(fillstyle, "solid");
    strcpy(linestyle, "solid");

    //printf("parseBasicAttribute\n");
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
        Qt_setColorLine("",  displayInfo->dlColormap->dl_color[clr].r,
                        displayInfo->dlColormap->dl_color[clr].g,
                        displayInfo->dlColormap->dl_color[clr].b,
                        255);

    } else if((!strcmp(widget,"caPolyLine")) || (!strcmp(widget,"caCircleBar"))) {
        if(!strcmp(fillstyle,"solid")) {
            Qt_setColorForeground("", displayInfo->dlColormap->dl_color[clr].r,
                                  displayInfo->dlColormap->dl_color[clr].g,
                                  displayInfo->dlColormap->dl_color[clr].b,
                                  255);
            if(strcmp(widget,"caPolyLine"))Qt_setColorFill("");
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
            Qt_handleString("style", "enum", "Solid");
        } else if(!strcmp(linestyle,"dash")) {
            Qt_handleString("style", "enum", "Dash");
        } else if(!strcmp(linestyle,"bigdash")) {
            Qt_handleString("style", "enum", "BigDash");
        }
    }

}

void parseLimits(DisplayInfo *displayInfo, char *widget, int pen)
{
    char token[MAX_TOKEN_LENGTH];
    TOKEN tokenType;
    int nestingLevel = 0;
    int hoprSrc = False;
    int loprSrc = False;
    int precSrc = False;
    int precDefault = False;

    //printf("parseLimits\n");
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
	    } else if(!strcmp(token,"hoprSrc")) {
		getToken(displayInfo,token);
		getToken(displayInfo,token);
                hoprSrc = True;
                Qt_setMaximumLimitSource(widget, pen, token);
	    } else if(!strcmp(token,"hoprDefault")) {
		getToken(displayInfo,token);
                getToken(displayInfo,token);
                Qt_setMaximumLimit(widget, pen, token);
	    } else if(!strcmp(token,"precSrc")) {
		getToken(displayInfo,token);
		getToken(displayInfo,token);
                precSrc = True;
                Qt_setPrecisionSource(widget, pen, token);
	    } else if(!strcmp(token,"precDefault")) {
		getToken(displayInfo,token);
		getToken(displayInfo,token);
                Qt_setPrecision(widget, pen, token);
                printf("got precision\n");
                precDefault = True;
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

    if(!loprSrc) Qt_setMinimumLimitSource(widget, pen, "Channel");
    if(!hoprSrc) Qt_setMaximumLimitSource(widget, pen, "Channel");
    if(!precSrc) {
        Qt_setPrecisionSource(widget, pen, "Channel");
        //if(!precDefault) Qt_setPrecisionSource(widget, pen, "Channel");
        //else Qt_setPrecisionSource(widget, pen, "User");
    }
}

void parseDynamicAttribute(DisplayInfo *displayInfo, char *widget) {

    char token[MAX_TOKEN_LENGTH];
    TOKEN tokenType;
    int nestingLevel = 0;

    //printf("parseDynamicAttribute\n");
    do {
	switch( (tokenType=getToken(displayInfo,token)) ) {
	case T_WORD:
	    if(!strcmp(token,"clr")) {
		getToken(displayInfo,token);
		getToken(displayInfo,token);
                Qt_setColorMode(widget, token);
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

void parseRelatedDisplayEntry(DisplayInfo *displayInfo, char *widget, char *label, char *file, char *arg)
{
    char token[MAX_TOKEN_LENGTH];
    TOKEN tokenType;
    int nestingLevel = 0;

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
    //printf("==> parseGrid\n");
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

    do {
        switch( (tokenType=getToken(displayInfo,token)) ) {
        case T_WORD:
            if (!strcmp(token,"chan")) {
                getToken(displayInfo,token);
                getToken(displayInfo,token);
                strcat(channels, token);
                strcat(channels,";");
            } else if (!strcmp(token,"clr")) {
                getToken(displayInfo,token);
                getToken(displayInfo,token);
                int clr = atoi(token) % DL_MAX_COLORS;
                Qt_setColorTrace("", displayInfo->dlColormap->dl_color[clr].r,
                                     displayInfo->dlColormap->dl_color[clr].g,
                                     displayInfo->dlColormap->dl_color[clr].b,
                                     255, pen+1);
            } else if (!strcmp(token,"limits")) {
                parseLimits(displayInfo, widget, pen);
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


void parseObject(DisplayInfo *displayInfo, DlObject *object, FrameOffset * offset, char *widget)
{
    char token[MAX_TOKEN_LENGTH];
    TOKEN tokenType;
    int nestingLevel = 0;
    //printf("parseObject\n");
    C_writeOpenProperty(myParserPtr, "geometry");
    C_writeOpenTag(myParserPtr, "rect", "", "");

    do {
	switch( (tokenType=getToken(displayInfo,token)) ) {
	case T_WORD:
	    if(!strcmp(token,"x")) {
		getToken(displayInfo,token);
		getToken(displayInfo,token);
                object->x = atoi(token);
                //printf("correct position with x=%d\n", offset->frameX);
                if(offset->frameX == 0) {
                    C_writeTaggedString(myParserPtr, "x", token);
                } else {
                    char asc[20];
                    int xx = atoi(token) - offset->frameX;
                    sprintf(asc, "%d", xx);
                    C_writeTaggedString(myParserPtr, "x", asc);
                }
	    } else if(!strcmp(token,"y")) {
		getToken(displayInfo,token);
		getToken(displayInfo,token);
                object->y = atoi(token);
                //printf("correct position with y=%d\n", offset->frameY);
                if(offset->frameY == 0) {
                    C_writeTaggedString(myParserPtr, "y", token);
                } else {
                    char asc[20];
                    int yy = atoi(token) - offset->frameY;
                    sprintf(asc, "%d", yy);
                    C_writeTaggedString(myParserPtr, "y", asc);
                }
	    } else if(!strcmp(token,"width")) {
		getToken(displayInfo,token);
		getToken(displayInfo,token);
                object->width = atoi(token);
                // add two pixels for frames
                if(!strcmp(widget,"caFrame")) {
                    char asc[10];
                    printf("width add two\n");
                    sprintf(asc, "%d", object->width + 2);
                    C_writeTaggedString(myParserPtr, "width", asc);
                } else {
                  C_writeTaggedString(myParserPtr, "width", token);
                }
	    } else if(!strcmp(token,"height")) {
		getToken(displayInfo,token);
		getToken(displayInfo,token);
                object->height = atoi(token);
                // add two pixels for frames
                if(!strcmp(widget,"caFrame")) {
                    char asc[10];
                    sprintf(asc, "%d", object->height + 2);
                    C_writeTaggedString(myParserPtr, "height", asc);
                } else {
                    C_writeTaggedString(myParserPtr, "height", token);
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

    C_writeCloseTag(myParserPtr, "rect" );
    C_writeCloseProperty(myParserPtr);
}

void parsePlotcom(DisplayInfo *displayInfo, char *widget)
{
    char token[MAX_TOKEN_LENGTH];
    TOKEN tokenType;
    int nestingLevel = 0;

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

                if(strstr(token, "[") != (char*) 0 && strstr(widget, "caStripPlot") != (char*) 0) {
                   char asc[80];
                   sprintf(asc, "%s::FillUnder", widget);
                   Qt_handleString("Style_1", "enum", asc);
                }

            } else if (!strcmp(token,"ylabel")) {
                getToken(displayInfo,token);
                getToken(displayInfo,token);
                Qt_handleString("TitleY", "string", token);
            } else if (!strcmp(token,"package")) {
                getToken(displayInfo,token);
                getToken(displayInfo,token);
                //strcpy(plotcom->package,token);
            } else if (!strcmp(token,"clr")) {
                getToken(displayInfo,token);
                getToken(displayInfo,token);
                int clr = atoi(token) % DL_MAX_COLORS;

                Qt_setColorScale("", displayInfo->dlColormap->dl_color[clr].r,
                                     displayInfo->dlColormap->dl_color[clr].g,
                                     displayInfo->dlColormap->dl_color[clr].b,
                                     255);

            } else if (!strcmp(token,"bclr")) {
                getToken(displayInfo,token);
                getToken(displayInfo,token);
                int bclr = atoi(token) % DL_MAX_COLORS;

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
    char pvX[40];
    char pvY[40];
    char mon[80];
    char channel[40];

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
                int clr = atoi(token) % DL_MAX_COLORS;

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

    minRange[0] = '\0';
    maxRange[0] = '\0';

    do {
        switch( (tokenType=getToken(displayInfo,token)) ) {
        case T_WORD:
            if (!strcmp(token,"axisStyle")) {
                getToken(displayInfo,token);
                getToken(displayInfo,token);
                if (!strcmp(token,"linear")) {
                  //->axisStyle = LINEAR_AXIS;
                } else if (!strcmp(token,"log10")) {
                  //dlPlotAxisDefinition->axisStyle = LOG10_AXIS;
                } else  if (!strcmp(token,"time")) {
                  //dlPlotAxisDefinition->axisStyle = TIME_AXIS;
                }
            } else if (!strcmp(token,"rangeStyle")) {
                getToken(displayInfo,token);
                getToken(displayInfo,token);
                if(axis == X_AXIS_ELEMENT) {
                   Qt_setXaxisLimitSource("caCartesianPlot", token);
                } else {
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
                int i;
                getToken(displayInfo,token);
                getToken(displayInfo,token);
                for (i=(int) FIRST_CP_TIME_FORMAT;
                     i<(int)(FIRST_CP_TIME_FORMAT+NUM_CP_TIME_FORMAT);i++) {
                    if (!strcmp(token,stringValueTable[i])) {
                        //dlPlotAxisDefinition->timeFormat = i;
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
    } while ( (tokenType != T_RIGHT_BRACE) && (nestingLevel > 0)
      && (tokenType != T_EOF) );

    if(minRange[0] != '\0' || maxRange[0] != '\0') {
        char range[80];
        if(minRange[0] == '\0') strcpy(minRange, "0");
        if(maxRange[0] == '\0') strcpy(maxRange, "0");
        if(axis == X_AXIS_ELEMENT) {
            sprintf(range, "%s;%s", minRange, maxRange);
            Qt_handleString("XaxisLimits", "string", range);
        } else {
            sprintf(range, "%s;%s", minRange, maxRange);
            Qt_handleString("YaxisLimits", "string", range);
        }
    } else {
        if(axis == X_AXIS_ELEMENT)Qt_handleString("XaxisLimits", "string", "0;1");
        else Qt_handleString("YaxisLimits", "string", "0;1");
    }

}

void *parseCartesianPlot(DisplayInfo *displayInfo, FrameOffset * offset)
{
    char token[MAX_TOKEN_LENGTH];
    TOKEN tokenType;
    int nestingLevel = 0;
    DlObject object;
    int traceNumber;

    printf("==> parseCartesianPlot\n");

    static int number = 0;
    char asc[40];
    sprintf(asc, "caCartesianPlot_%d", number++);
    C_writeOpenTag(myParserPtr, "widget", "caCartesianPlot", asc);

    do {
        switch( (tokenType=getToken(displayInfo,token)) ) {
        case T_WORD:
            if(!strcmp(token,"object")) {
                parseObject(displayInfo, &object, offset, "caCartesianPlot");
            } else if(!strcmp(token,"plotcom")) {
                parsePlotcom(displayInfo, "caCartesianPlot");
            } else if(!strcmp(token,"count")) {
                getToken(displayInfo,token);
                getToken(displayInfo,token);
                //dlCartesianPlot->count= atoi(token);
                //strcpy(dlCartesianPlot->countPvName,token);
            } else if(!strcmp(token,"style")) {
                getToken(displayInfo,token);
                getToken(displayInfo,token);
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
                    Qt_handleString("symbol_1", "enum", "caCartesianPlot::Ellipse");
                    Qt_handleString("symbol_2", "enum", "caCartesianPlot::Rect");
                    Qt_handleString("symbol_3", "enum", "caCartesianPlot::Triangle");
                } else if(!strcmp(token,"line")) {
                    Qt_handleString("Style_1", "enum", "caCartesianPlot::Lines");
                    Qt_handleString("Style_2", "enum", "caCartesianPlot::Lines");
                    Qt_handleString("Style_3", "enum", "caCartesianPlot::Lines");
                    Qt_handleString("symbol_1", "enum", "caCartesianPlot::Ellipse");
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
                    Qt_handleString("symbol_1", "enum", "caCartesianPlot::Ellipse");
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
                    //dlCartesianPlot->erase_oldest = ERASE_OLDEST_ON;
                } else if(!strcmp(token,"off")) {
                    //dlCartesianPlot->erase_oldest = ERASE_OLDEST_OFF;
                } else if(!strcmp(token,"plot last n pts")) {
                    //dlCartesianPlot->erase_oldest = ERASE_OLDEST_ON;
                } else if(!strcmp(token,"plot n pts & stop")) {
                    //dlCartesianPlot->erase_oldest = ERASE_OLDEST_OFF;
                }
            } else if(!strncmp(token,"trace",5)) {
                traceNumber = MIN(token[6] - '0', MAX_TRACES - 1);
                printf("----------------------------------------parseTrace\n");
                parseTrace(displayInfo,traceNumber);
            } else if(!strcmp(token,"x_axis")) {
                parsePlotAxisDefinition(displayInfo, X_AXIS_ELEMENT);
            } else if(!strcmp(token,"y1_axis")) {
                parsePlotAxisDefinition(displayInfo, Y1_AXIS_ELEMENT);
            } else if(!strcmp(token,"y2_axis")) {
                parsePlotAxisDefinition(displayInfo, Y2_AXIS_ELEMENT);
            } else if(!strcmp(token,"trigger")) {
                getToken(displayInfo,token);
                getToken(displayInfo,token);
                //strcpy(dlCartesianPlot->trigger,token);
            } else if(!strcmp(token,"erase")) {
                getToken(displayInfo,token);
                getToken(displayInfo,token);
                //strcpy(dlCartesianPlot->erase,token);
            } else if(!strcmp(token,"countPvName")) {
                getToken(displayInfo,token);
                getToken(displayInfo,token);
                //dlCartesianPlot->count= atoi(token);
                //strcpy(dlCartesianPlot->countPvName,token);
            } else if(!strcmp(token,"eraseMode")) {
                getToken(displayInfo,token);
                getToken(displayInfo,token);
                if(!strcmp(token,"if not zero")) {
                    //dlCartesianPlot->eraseMode = ERASE_IF_NOT_ZERO;
                } else if(!strcmp(token,"if zero")) {
                    //dlCartesianPlot->eraseMode = ERASE_IF_ZERO;
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

    C_writeCloseTag(myParserPtr, "widget");

    return (void*) 0;
}

void *parseRelatedDisplay(DisplayInfo *displayInfo, FrameOffset * offset)
{
    char token[MAX_TOKEN_LENGTH];
    char label[MAX_TOKEN_LENGTH] = "\0";
    char name[MAX_TOKEN_LENGTH] = "\0";
    char arg[MAX_TOKEN_LENGTH] = "\0";

    TOKEN tokenType;
    int nestingLevel = 0;
    int displayNumber;
    int rc;
    DlObject object;
    char visual[MAX_TOKEN_LENGTH] = "Menu";
    char labels[1000] = "\0";
    char names[1000] = "\0";
    char argus[1000]  = "\0";

    static int number = 0;
    char asc[40];
    sprintf(asc, "caRelatedDisplay_%d", number++);
    C_writeOpenTag(myParserPtr, "widget", "caRelatedDisplay", asc);

    printf("parseRelatedDisplay\n");
    do {
        switch(tokenType=getToken(displayInfo,token)) {
        case T_WORD:
            if(!strcmp(token,"object")) {
                printf("got object %s\n", token);
                parseObject(displayInfo, &object, offset, "caRelatedDisplay");
            } else if(!strncmp(token,"display",7)) {
                /* Get the display number */
                displayNumber=MAX_RELATED_DISPLAYS-1;
                rc=sscanf(token,"display[%d]",&displayNumber);
                if(rc == 0 || rc == EOF || displayNumber < 0 ||
                        displayNumber > MAX_RELATED_DISPLAYS-1) {
                    displayNumber=MAX_RELATED_DISPLAYS-1;
                }
                parseRelatedDisplayEntry(displayInfo, "caRelatedDisplay", label, name, arg);

                strcat(names, name); strcat(names, ";");
                strcat(argus, arg); strcat(argus, ";");
                strcat(labels, label); strcat(labels, ";");

            } else if(!strcmp(token,"clr")) {
                getToken(displayInfo,token);
                getToken(displayInfo,token);
                int clr = atoi(token) % DL_MAX_COLORS;
                Qt_setColorForeground("",displayInfo->dlColormap->dl_color[clr].r,
                                         displayInfo->dlColormap->dl_color[clr].g,
                                         displayInfo->dlColormap->dl_color[clr].b,
                                         255);
            } else if(!strcmp(token,"bclr")) {
                getToken(displayInfo,token);
                getToken(displayInfo,token);
                int clr = atoi(token) % DL_MAX_COLORS;
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

    printf("labels <%s> len=%d\n", labels, strlen(labels));
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

    C_writeCloseTag(myParserPtr, "widget");

    return (void*) 0;

}

void parseShellCommandEntry(DisplayInfo *displayInfo)
{
    char token[MAX_TOKEN_LENGTH];
    TOKEN tokenType;
    int nestingLevel = 0;

    do {
        switch( (tokenType=getToken(displayInfo,token)) ) {
        case T_WORD:
            if(!strcmp(token,"label")) {
                getToken(displayInfo,token);
                getToken(displayInfo,token);
                //strcpy(shellCommand->label,token);
            } else if(!strcmp(token,"name")) {
                getToken(displayInfo,token);
                getToken(displayInfo,token);
                //strcpy(shellCommand->command,token);
            } else if(!strcmp(token,"args")) {
                getToken(displayInfo,token);
                getToken(displayInfo,token);
                //strcpy(shellCommand->args,token);
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
    TOKEN tokenType;
    int nestingLevel = 0;
    int cmdNumber;
    int rc;
    DlObject object;

    static int number = 0;
    char asc[40];
    sprintf(asc, "caParseShellCommand_%d", number++);
    C_writeOpenTag(myParserPtr, "widget", "caParseShellCommand", asc);

    printf("parseShellCommand\n");
    do {
        switch( (tokenType=getToken(displayInfo,token)) ) {
        case T_WORD:
            if(!strcmp(token,"object")) {
                //parseObject(displayInfo, &object, offset, "caParseShellCommand");
            } else if(!strncmp(token,"command",7)) {
                /* Get the command number */
                cmdNumber=MAX_SHELL_COMMANDS-1;
                rc=sscanf(token,"command[%d]",&cmdNumber);
                if(rc == 0 || rc == EOF || cmdNumber < 0 ||
                        cmdNumber > MAX_SHELL_COMMANDS-1) {
                    cmdNumber=MAX_SHELL_COMMANDS-1;
                }
                parseShellCommandEntry(displayInfo );
            } else if(!strcmp(token,"clr")) {
                getToken(displayInfo,token);
                getToken(displayInfo,token);
                //dlShellCommand->clr = atoi(token) % DL_MAX_COLORS;
            } else if(!strcmp(token,"bclr")) {
                getToken(displayInfo,token);
                getToken(displayInfo,token);
                //dlShellCommand->bclr = atoi(token) % DL_MAX_COLORS;
            } else if(!strcmp(token,"label")) {
                getToken(displayInfo,token);
                getToken(displayInfo,token);
                //strcpy(dlShellCommand->label,token);
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

    C_writeCloseTag(myParserPtr, "widget");

     printf("parseShellCommand finished\n");

    return (void*) 0;
}


void *parseMeter(DisplayInfo *displayInfo, FrameOffset * offset)
{
    char token[MAX_TOKEN_LENGTH];
    char COLORMODE[MAX_TOKEN_LENGTH] = "static";
    TOKEN tokenType;
    int nestingLevel = 0;
    static int number = 0;
    DlObject object;

    char asc[40];
    sprintf(asc, "caCircularGauge_%d", number++);
    C_writeOpenTag(myParserPtr, "widget", "caCircularGauge", asc);

    printf("==> parseMeter\n");
    do {
	switch( (tokenType=getToken(displayInfo,token)) ) {
	case T_WORD:
	    if(!strcmp(token,"object"))
                parseObject(displayInfo, &object, offset, "caCircularGauge");
	    else if(!strcmp(token,"monitor"))
                parseMonitor(displayInfo, "caCircularGauge");
	    else if(!strcmp(token,"label")) {
		getToken(displayInfo,token);
		getToken(displayInfo,token);

                // labeltype none, no decorations, outline, limits, channel
                Qt_handleString("labelType", "enum", token);

	    } else if(!strcmp(token,"clrmod")) {
		getToken(displayInfo,token);
		getToken(displayInfo,token);
                strcpy(COLORMODE, token);
	    } else if(!strcmp(token,"limits")) {
                parseLimits(displayInfo, "caCircularGauge", 0);
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
    C_writeCloseTag(myParserPtr, "widget");

    return (void*) 0;
}

void *parseByte( DisplayInfo *displayInfo, FrameOffset * offset) {
    char token[MAX_TOKEN_LENGTH];
    char COLORMODE[MAX_TOKEN_LENGTH] = "static";
    TOKEN tokenType;
    int nestingLevel = 0;
    DlObject object;
    int dirFound = False;
    int sbitFound = False;
    int ebitFound = False;

    static int number = 0;
    char asc[40];
    sprintf(asc, "caByte_%d", number++);
    C_writeOpenTag(myParserPtr, "widget", "caByte", asc);

    printf("==> parseByte\n");
    do {
	switch( (tokenType=getToken(displayInfo,token)) ) {
	case T_WORD:
	    if(!strcmp(token,"object")) {
                parseObject(displayInfo, &object, offset, "caByte");
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
                    dirFound = True;
                    Qt_handleString("direction", "enum", "caByte::Up");
                } else if(!strcmp(token,"right")) {
                    dirFound = True;
                    Qt_handleString("direction", "enum", "caByte::Right");
                } else if(!strcmp(token,"down"))  {
                    dirFound = True;
                    Qt_handleString("direction", "enum", "caByte::Down");
                } else if(!strcmp(token,"left")) {
                    dirFound = True;
                    Qt_handleString("direction", "enum", "caByte::Left");
                }
	    } else if(!strcmp(token,"sbit")) {
		getToken(displayInfo,token);
		getToken(displayInfo,token);
                sbitFound = True;
                 Qt_handleString("startBit", "number", token);
	    } else if(!strcmp(token,"ebit")) {
		getToken(displayInfo,token);
		getToken(displayInfo,token);
                ebitFound = True;
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

    if(!sbitFound || !ebitFound) {
        Qt_handleString("startBit", "enum", "0");
        Qt_handleString("endBit", "enum", "15");
    }
    if(!dirFound) {
      Qt_handleString("direction", "enum", "caByte::down");
    }
    //Qt_setColorMode("caByte", COLORMODE);

    C_writeCloseTag(myParserPtr, "widget");

    return (void*) 0;
}

void *parseStripChart(DisplayInfo *displayInfo, FrameOffset * offset)
{
    char token[MAX_TOKEN_LENGTH];
    char periodValue[MAX_TOKEN_LENGTH];
    TOKEN tokenType;
    int nestingLevel = 0;
    DlObject object;
    int penNumber;
    int isVersion2_1_x = False;
    char channels[200] = "";
    int periodFound = False;

    static int number = 0;
    char asc[40];
    sprintf(asc, "caStripPlot_%d", number++);
    C_writeOpenTag(myParserPtr, "widget", "caStripPlot", asc);

    printf("======================> parseStripPlot\n");
    do {
	switch( (tokenType=getToken(displayInfo, token)) ) {
	case T_WORD:
	    if(!strcmp(token, "object")) {
                parseObject(displayInfo,  &object, offset, "caStripPlot");
            } else if(!strcmp(token, "plotcom")) {
                parsePlotcom(displayInfo, "caStripPlot");
            } else if(!strcmp(token, "period")) {
		getToken(displayInfo, token);
		getToken(displayInfo, token);
                periodFound = True;
                strcpy(periodValue, token);
	    } else if(!strcmp(token, "delay")) {
		getToken(displayInfo, token);
		getToken(displayInfo, token);
                Qt_handleString("delay", "enum", token);
            } else if(!strcmp(token, "units")) {
		getToken(displayInfo, token);
		getToken(displayInfo, token);
		if(!strcmp(token, "minute")) {
                    Qt_handleString("units", "enum", "Minute");
                } else if(!strcmp(token, "second")) {
                    Qt_handleString("units", "enum", "Second");
                } else if(!strcmp(token, "milli second")) {
                    Qt_handleString("units", "enum", "Millisecond");
                } else if(!strcmp(token, "milli-second")) {
                    Qt_handleString("units", "enum", "Millisecond");
                } else {
                    printf("parseStripChart: Illegal units %s\n"
                           "  Using: %s\n", token, stringValueTable[SC_DEFAULT_UNITS]);
                }
	    } else if(!strncmp(token, "pen", 3)) {
		penNumber = MIN(token[4] - '0', MAX_PENS-1);
                printf("parse pen %d\n", penNumber);
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

    if(!periodFound) {
        Qt_handleString("period", "double", "60");
    } else {
        Qt_handleString("period", "double", periodValue);
    }

    channels[strlen(channels)-1] = '\0';
    Qt_handleString("channels", "string", channels);

    if(isVersion2_1_x) {
	//dlStripChart->delay = -1.0;  /* -1.0 is used as a indicator to save

    } else {
        /*
      if(dlStripChart->delay > 0) {
   double val, dummy1, dummy2;
   switch (dlStripChart->units) {
   case MILLISECONDS:
       dummy1 = -0.060 * (double) dlStripChart->delay;
       break;
   case SECONDS:
       dummy1 = -60 * (double) dlStripChart->delay;
       break;
   case MINUTES:
       dummy1 = -3600.0 * (double) dlStripChart->delay;
       break;
   default:
       dummy1 = -60 * (double) dlStripChart->delay;
       break;
   }

   //linear_scale(dummy1, 0.0, 2, &val, &dummy1, &dummy2);
   dlStripChart->period = -val;
   dlStripChart->oldUnits = dlStripChart->units;
   dlStripChart->units = SC_DEFAULT_UNITS;
      }
      */
    }

    C_writeCloseTag(myParserPtr, "widget");

    return (void*) 0;

}

void *parseTextUpdate(DisplayInfo *displayInfo, FrameOffset * offset)
{
    char token[MAX_TOKEN_LENGTH];
    char COLORMODE[MAX_TOKEN_LENGTH] = "static";
    TOKEN tokenType;
    int nestingLevel = 0;
    DlObject object;
    int alignFound=False;
    TextFormat format;
    int i= 0;

    static int number = 0;

    char asc[40];
    sprintf(asc, "caLineEdit_%d", number++);
    C_writeOpenTag(myParserPtr, "widget", "caLineEdit", asc);

    printf("==> parseTextUpdate\n");
    do {
	switch( (tokenType=getToken(displayInfo,token)) ) {
	case T_WORD:
	    if(!strcmp(token,"object")) {
                parseObject(displayInfo, &object, offset, "caLineEdit");
                Qt_handleString("fontScaleMode", "enum", "caLineEdit::Height");
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
                for(i=(int)FIRST_TEXT_FORMAT;i<(int)(FIRST_TEXT_FORMAT+NUM_TEXT_FORMATS); i++) {
		    if(!strcmp(token,stringValueTable[i])) {
			format = (TextFormat) i;
			found = 1;
			break;
		    }
		}
                /* Backward compatibility */
		if(!found) {
		    if(!strcmp(token,"decimal")) {
			format = MEDM_DECIMAL;
		    } else if(!strcmp(token,
                                      "decimal- exponential notation")) {
			format = EXPONENTIAL;
		    } else if(!strcmp(token,"engr. notation")) {
			format = ENGR_NOTATION;
		    } else if(!strcmp(token,"decimal- compact")) {
			format = COMPACT;
		    } else if(!strcmp(token,"decimal- truncated")) {
			format = TRUNCATED;
                        /* (MDA) allow for LANL spelling errors {like
                         above, but with trailing space} */
		    } else if(!strcmp(token,"decimal- truncated ")) {
			format = TRUNCATED;
                        /* (MDA) allow for LANL spelling errors
                         {hexidecimal vs. hexadecimal} */
		    } else if(!strcmp(token,"hexidecimal")) {
			format = HEXADECIMAL;
		    }
		}
	    } else if(!strcmp(token,"align")) {

		getToken(displayInfo,token);
		getToken(displayInfo,token);
                printf("token=%s\n", token);
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
                parseLimits(displayInfo, "caLineEdit", 0);
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

    Qt_setColorMode("caLineEdit", COLORMODE);
    C_writeCloseTag(myParserPtr, "widget");

    return (void*) 0;
}

void *parseChoiceButton(DisplayInfo *displayInfo, FrameOffset * offset)
{
    char token[MAX_TOKEN_LENGTH];
    char COLORMODE[MAX_TOKEN_LENGTH] = "static";
    TOKEN tokenType;
    int nestingLevel = 0;
    DlObject object;

    static int number = 0;
    char asc[40];
    sprintf(asc, "caChoice_%d", number++);
    C_writeOpenTag(myParserPtr, "widget", "caChoice", asc);

    printf("==> parseChoice\n");
    do {
	switch( (tokenType=getToken(displayInfo,token)) ) {
	case T_WORD:
	    if(!strcmp(token,"object"))
                parseObject(displayInfo, &object, offset, "caChoice");
	    else if(!strcmp(token,"control"))
                parseControl(displayInfo);
	    else if(!strcmp(token,"clrmod")) {
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


    //Qt_setColorMode("caChoiceButton", COLORMODE);
    C_writeCloseTag(myParserPtr, "widget");

    return (void*) 0;
}

void *parseMessageButton(DisplayInfo *displayInfo, FrameOffset * offset)
{
    char token[MAX_TOKEN_LENGTH];
    char COLORMODE[MAX_TOKEN_LENGTH] = "static";
    TOKEN tokenType;
    int nestingLevel = 0;
    DlObject object;

    static int number = 0;
    char asc[40];
    sprintf(asc, "caMessageButton_%d", number++);
    C_writeOpenTag(myParserPtr, "widget", "caMessageButton", asc);

    printf("==> parseMessageButton\n");
    do {
	switch( (tokenType=getToken(displayInfo,token)) ) {
	case T_WORD:
	    if(!strcmp(token,"object"))
                parseObject(displayInfo, &object, offset, "caMessageButton");
	    else if(!strcmp(token,"control"))
                parseControl(displayInfo);
	    else if(!strcmp(token,"press_msg")) {
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
    C_writeCloseTag(myParserPtr, "widget");

    return (void*) 0;
}

void *parseMenu(DisplayInfo *displayInfo, FrameOffset * offset)
{
    char token[MAX_TOKEN_LENGTH];
    char COLORMODE[MAX_TOKEN_LENGTH] = "static";
    TOKEN tokenType;
    int nestingLevel = 0;
    DlObject object;

    static int number = 0;
    char asc[40];
    sprintf(asc, "caMenu_%d", number++);
    C_writeOpenTag(myParserPtr, "widget", "caMenu", asc);

    printf("==> parseMenu\n");
    do {
	switch( (tokenType=getToken(displayInfo,token)) ) {
	case T_WORD:
	    if(!strcmp(token,"object"))
                parseObject(displayInfo, &object, offset, "caMenu");
	    else if(!strcmp(token,"control"))
                parseControl(displayInfo);
	    else if(!strcmp(token,"clrmod")) {
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

    //Qt_setColorMode("caMenu", COLORMODE);
    C_writeCloseTag(myParserPtr, "widget");

    return (void*) 0;
}

void *parseTextEntry(DisplayInfo *displayInfo, FrameOffset * offset)
{
    char token[MAX_TOKEN_LENGTH];
    char COLORMODE[MAX_TOKEN_LENGTH] = "static";
    TOKEN tokenType;
    int nestingLevel = 0;
    DlObject object;
    int i = 0;

    static int number = 0;
    char asc[40];
    sprintf(asc, "caTextEntry_%d", number++);
    C_writeOpenTag(myParserPtr, "widget", "caTextEntry", asc);


    printf("==> parseTextEntry\n");
    do {
	switch( (tokenType=getToken(displayInfo,token)) ) {
	case T_WORD:
	    if(!strcmp(token,"object"))
                parseObject(displayInfo, &object, offset, "caTextEntry");
	    else if(!strcmp(token,"control"))
                parseControl(displayInfo);
	    else if(!strcmp(token,"clrmod")) {
		getToken(displayInfo,token);
		getToken(displayInfo,token);
                strcpy(COLORMODE, token);
	    } else if(!strcmp(token,"format")) {
		getToken(displayInfo,token);
		getToken(displayInfo,token);
                for(i=(int)FIRST_TEXT_FORMAT;i<(int)(FIRST_TEXT_FORMAT+NUM_TEXT_FORMATS); i++) {
		    if(!strcmp(token,stringValueTable[i])) {
                        //dlTextEntry->format = (TextFormat) i;
			break;
		    }
		}
	    } else if(!strcmp(token,"limits")) {
                parseLimits(displayInfo, "caTextEntry", 0);
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

    Qt_setColorMode("caTextEntry", COLORMODE);
    C_writeCloseTag(myParserPtr, "widget");

    return (void*) 0;

}

void *parseValuator(DisplayInfo *displayInfo, FrameOffset * offset)
{
    char token[MAX_TOKEN_LENGTH];
    char COLORMODE[MAX_TOKEN_LENGTH] = "static";
    TOKEN tokenType;
    int nestingLevel = 0;
    DlObject object;

    static int number = 0;
    char asc[40];
    sprintf(asc, "caSlider_%d", number++);
    C_writeOpenTag(myParserPtr, "widget", "caSlider", asc);

    printf("==> parseValuator\n");
    do {
	switch((tokenType=getToken(displayInfo,token)) ) {
	case T_WORD:
	    if(!strcmp(token,"object"))
                parseObject(displayInfo, &object, offset, "caSlider");
	    else if(!strcmp(token,"control"))
                parseControl(displayInfo);
	    else if(!strcmp(token,"label")) {
		getToken(displayInfo,token);
		getToken(displayInfo,token);
                if(!strcmp(token,"none")) {
                    //dlValuator->label = LABEL_NONE;
                } else if(!strcmp(token,"no decorations")) {
                    //dlValuator->label = NO_DECORATIONS;
                } else if(!strcmp(token,"outline")) {
                    //dlValuator->label = OUTLINE;
                } else if(!strcmp(token,"limits")) {
                    //dlValuator->label = LIMITS;
                } else if(!strcmp(token,"channel")) {
                    //dlValuator->label = CHANNEL;
                }
	    } else if(!strcmp(token,"clrmod")) {
		getToken(displayInfo,token);
		getToken(displayInfo,token);
                strcpy(COLORMODE, token);
	    } else  if(!strcmp(token,"direction")) {
		getToken(displayInfo,token);
		getToken(displayInfo,token);
                if(!strcmp(token,"up")) {
                    //dlValuator->direction = UP;
                } else if(!strcmp(token,"right")) {
                    //dlValuator->direction = RIGHT;
                } else if(!strcmp(token,"down")){
                   //dlValuator->direction = DOWN;
                } else if(!strcmp(token,"left")) {
                    //dlValuator->direction = LEFT;
                }
	    } else if(!strcmp(token,"dPrecision")) {
		getToken(displayInfo,token);
		getToken(displayInfo,token);
                //dlValuator->dPrecision = atof(token);
	    } else if(!strcmp(token,"limits")) {
                parseLimits(displayInfo,  "caSlider", 0);
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

    Qt_setColorMode("caSlider", COLORMODE);
    C_writeCloseTag(myParserPtr, "widget");

    return (void*)0;
}

void parseControl(DisplayInfo *displayInfo)
{
    char token[MAX_TOKEN_LENGTH];
    TOKEN tokenType;
    int nestingLevel = 0;
    string40 items[10];
    int nbitems;
    //printf("parseControl\n");
    do {
        switch( (tokenType=getToken(displayInfo,token)) ) {
	case T_WORD:
            if (!strcmp(token,"ctrl") || !strcmp(token,"chan")) {
		getToken(displayInfo,token);
		getToken(displayInfo,token);
                printf("==================== token = <%s>\n", token);
                // specialty for proscan, we added bitstart and bitend to the channel sperated by a blanc
                // and separated with a comma
                nbitems = parseDelimited(token, items, 10, ' ');
                if(nbitems > 0) {
                    Qt_handleString("channel", "string", items[0]);
                    printf("==================== token = <%s> nbitems=%d\n", token, nbitems);
                    if(nbitems > 1) {
                        nbitems = parseDelimited(items[1], items, 10, ',');
                        printf("==================== nbitems=%d <%s> <%s>\n", nbitems, items[0], items[1]);
                        Qt_handleString("startBit", "number", items[0]);
                        Qt_handleString("endBit", "number", items[1]);
                    }
                }
	    } else if (!strcmp(token,"clr")) {
		getToken(displayInfo,token);
		getToken(displayInfo,token);
                //control->clr = atoi(token) % DL_MAX_COLORS;
	    } else if (!strcmp(token,"bclr")) {
		getToken(displayInfo,token);
		getToken(displayInfo,token);
                //control->bclr = atoi(token) % DL_MAX_COLORS;
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
    DlObject object;

    static int number = 0;
    char asc[40];
    sprintf(asc, "caImage_%d", number++);
    C_writeOpenTag(myParserPtr, "widget", "caImage", asc);

    printf("==> parseImage\n");
    do {
        switch( (tokenType = getToken(displayInfo,token)) ) {
	case T_WORD:
	    if(!strcmp(token,"object")) {
                parseObject(displayInfo, &object, offset, "caImage");
	    } else if(!strcmp(token,"dynamic attribute")) {
                parseDynamicAttribute(displayInfo,"caImage");
	    } else if(!strcmp(token,"type")) {
		getToken(displayInfo,token);
		getToken(displayInfo,token);
                if(!strcmp(token,"none")) {
                    //imageType = NO_IMAGE;
                } else if(!strcmp(token,"gif")) {
                    //imageType = GIF_IMAGE;
                } else if(!strcmp(token,"tiff")) {
                    /* KE: There is no TIFF capability */
                    //imageType = TIFF_IMAGE;
                }
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

    C_writeCloseTag(myParserPtr, "widget");

    return (void*) 0;
}

static void compositeFileParse(DisplayInfo *displayInfo, char *filename, FrameOffset *newoffset)
{
    FILE *filePtr, *savedFilePtr;
    int savedVersionNumber;
    NameValueTable *savedNameValueTable = NULL;
    int savedNumNameValues = 0;
    char macroString[MAX_TOKEN_LENGTH];
    char token[MAX_TOKEN_LENGTH];
    TOKEN tokenType;

    if(!displayInfo) return;
    /*
    dlComposite = dlElement->structure.composite;
*/
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

    printf("compositeFileParse file=%s macro=%s\n", filename, macroString);

    savedFilePtr = displayInfo->filePtr;
    savedVersionNumber = displayInfo->versionNumber;

    // Open the file
    filePtr = fopen(filename, "r");
    if(!filePtr) {
        printf("\ncompositeFileParse: Cannot open file: %s\n",filename);
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
    //dlFile = createDlFile(displayInfo);
    // If first token isn't "file" then bail out
    tokenType=getToken(displayInfo,token);
    if(tokenType == T_WORD && !strcmp(token,"file")) {
        parseAndSkip(displayInfo);
    } else {
        printf("compositeFileParse: Invalid .adl file (First block is not file block) file: %s\n",filename);
        fclose(filePtr);
        goto RETURN;
    }

    // Plug the current version number into the displayInfo
    //displayInfo->versionNumber = dlFile->versionNumber;
    //free((char *)dlFile);

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

    /*
  // Rearrange the composite to fit its contents
    minX = INT_MAX; minY = INT_MAX;
    maxX = INT_MIN; maxY = INT_MIN;
    pE = FirstDlElement(dlComposite->dlElementList);
    while(pE) {
        pO = &(pE->structure.composite->object);

        minX = MIN(minX,pO->x);
        maxX = MAX(maxX,(int)(pO->x+pO->width));
        minY = MIN(minY,pO->y);
        maxY = MAX(maxY,(int)(pO->y+pO->height));
#if DEBUG_FILE
        print("  %-20s %3d %3d %3d %3d %3d %3d\n",
          elementType(pE->type),
          pO->x,pO->y,pO->width,pO->height,
          (int)(pO->x+pO->width),(int)(pO->x+pO->height));
#endif
        pE = pE->next;
    }
    oldX = dlComposite->object.x;
    oldY = dlComposite->object.y;
    dlComposite->object.x = minX;
    dlComposite->object.y = minY;
    dlComposite->object.width = maxX - minX;
    dlComposite->object.height = maxY - minY;

  // Move the rearranged composite to its original x and y coordinates
    compositeMove(dlElement, oldX - minX, oldY - minY);

  // Check composite is in bounds
    displayW = displayH = 0;
    pD = FirstDlElement(displayInfo->dlElementList);
    pO = &(dlComposite->object);
    if(pD && pO) {
        displayW = pD->structure.display->object.width;
        displayH = pD->structure.display->object.height;
        if((pO->x) > displayW ||
          (pO->x + (int)pO->width) < 0 ||
          (pO->y) > displayH ||
          (pO->y + (int)pO->height) < 0) {
            medmPrintf(1,"\ncompositeFileParse:"
              " Composite from file extends beyond display:\n"
              "  File: %s\n", filename);
        } else if((pO->x) < 0 ||
          (pO->x + (int)pO->width) > displayW ||
          (pO->y) < 0 ||
          (pO->y + (int)pO->height) > displayH) {
            medmPrintf(1,"\ncompositeFileParse:"
              " Composite from file extends beyond display:\n"
              "  File: %s\n", filename);
        }
    }
#if DEBUG_FILE
    print("  displayW=%d displayH=%d width=%d height=%d\n",
      displayW,displayH,(int)dlComposite->object.width,
      (int)dlComposite->object.height);
#endif
*/
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

void *parseComposite(DisplayInfo *displayInfo, FrameOffset * offset)
{
    char token[MAX_TOKEN_LENGTH];
    TOKEN tokenType;
    int nestingLevel = 0;
    DlObject *object;
    char compositeName[MAX_TOKEN_LENGTH];
    char compositeFile[MAX_TOKEN_LENGTH];

    static int number = 0;
    char asc[40];
    sprintf(asc, "caFrame_%d", number++);
    C_writeOpenTag(myParserPtr, "widget", "caFrame", asc);

    printf("create frame %s\n", asc);

    object = (DlObject *)malloc(sizeof(DlObject));

    FrameOffset *actoffset = (FrameOffset *)malloc(sizeof(FrameOffset));

    do {
        switch(tokenType=getToken(displayInfo,token)) {
	case T_WORD:
	    if(!strcmp(token,"object")) {
                parseObject(displayInfo, (object), offset, "caFrame");
                actoffset->frameX = object->x;
                actoffset->frameY = object->y;
                printf("new frame xy= %d %d\n",actoffset->frameX, actoffset->frameY);
                offset=actoffset;
	    } else if(!strcmp(token,"dynamic attribute")) {
                parseDynamicAttribute(displayInfo, "caFrame");
	    } else if(!strcmp(token,"composite name")) {
		getToken(displayInfo,token);
		getToken(displayInfo,token);
		strcpy(compositeName,token);
	    } else if(!strcmp(token,"composite file")) {
                printf("composite file\n");
		getToken(displayInfo,token);
		getToken(displayInfo,token);
		strcpy(compositeFile,token);
                FrameOffset *newoffset = (FrameOffset *)malloc(sizeof(FrameOffset));
                newoffset->frameX = 0; //actoffset->frameX;
                newoffset->frameY = 0; //actoffset->frameY;
                compositeFileParse(displayInfo, compositeFile, newoffset);
                /* Handle composite file here */
	    } else if(!strcmp(token,"children")) {
                printf("children\n");
                //displayInfo->object = object;
                FrameOffset *newoffset = (FrameOffset *)malloc(sizeof(FrameOffset));
                newoffset->frameX = actoffset->frameX;
                newoffset->frameY = actoffset->frameY;
		tokenType=getToken(displayInfo,token);
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

    C_writeCloseTag(myParserPtr, "widget");
    return (void*) 0;
}

void parsePolygonPoints(DisplayInfo *displayInfo, char *widget)
{
    char token[MAX_TOKEN_LENGTH];
    TOKEN tokenType;
    int nestingLevel;
#define INITIAL_NUM_POINTS 16
    //int pointsArraySize = INITIAL_NUM_POINTS;
    printf("parsePolygonPoints\n");
    /* initialize some data in structure */
    //dlPolygon->nPoints = 0;
    //dlPolygon->points = (XPoint *)malloc(pointsArraySize*sizeof(XPoint));

    nestingLevel = 0;
    do {
	switch( (tokenType=getToken(displayInfo,token)) ) {
	case T_WORD:
	    if(!strcmp(token,"(")) {
                //if(dlPolygon->nPoints >= pointsArraySize) {
                    /* reallocate the points array: enlarge by 4X, etc */
                    //pointsArraySize *= 4;
		    //dlPolygon->points = (XPoint *)realloc(dlPolygon->points,(pointsArraySize+1)*sizeof(XPoint));
                //}
		getToken(displayInfo,token);
		//dlPolygon->points[dlPolygon->nPoints].x = atoi(token);
		getToken(displayInfo,token);	/* separator	*/
		getToken(displayInfo,token);
		//dlPolygon->points[dlPolygon->nPoints].y = atoi(token);
		getToken(displayInfo,token);	/*   ")"	*/
                //dlPolygon->nPoints++;
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

    /* ensure closure of the polygon... */
    /*
    if(dlPolygon->points[0].x != dlPolygon->points[dlPolygon->nPoints-1].x &&
      dlPolygon->points[0].y != dlPolygon->points[dlPolygon->nPoints-1].y) {
 if(dlPolygon->nPoints >= pointsArraySize) {
     dlPolygon->points = (XPoint *)realloc(dlPolygon->points,
       (dlPolygon->nPoints+2)*sizeof(XPoint));
 }
 dlPolygon->points[dlPolygon->nPoints].x = dlPolygon->points[0].x;
 dlPolygon->points[dlPolygon->nPoints].y = dlPolygon->points[0].y;
 dlPolygon->nPoints++;
    }
 */ 
}

void *parseWheelSwitch(DisplayInfo *displayInfo, FrameOffset * offset)
{
    char token[MAX_TOKEN_LENGTH];
    char COLORMODE[MAX_TOKEN_LENGTH] = "static";
    TOKEN tokenType;
    int nestingLevel = 0;
    DlObject object;
    int i = 0;

    static int number = 0;
    char asc[40];
    sprintf(asc, "caNumeric_%d", number++);
    C_writeOpenTag(myParserPtr, "widget", "caNumeric", asc);

    printf("==> parseWheelSwitch\n");
    do {
	switch( (tokenType=getToken(displayInfo,token)) ) {
	case T_WORD:
	    if(!strcmp(token,"object")) {
                parseObject(displayInfo, &object, offset, "caNumeric");
	    } else if(!strcmp(token,"control")) {
                parseControl(displayInfo);
	    } else if(!strcmp(token,"clrmod")) {
		getToken(displayInfo,token);
		getToken(displayInfo,token);
                strcpy(COLORMODE, token);
	    } else if(!strcmp(token,"limits")) {
                parseLimits(displayInfo, "caNumeric", 0);
	    } else if(!strcmp(token,"format")) {
		getToken(displayInfo,token);
		getToken(displayInfo,token);
                Qt_setWheelSwitchForm("", token);
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

    //Qt_setColorMode("caNumeric", COLORMODE);
    C_writeCloseTag(myParserPtr, "widget");

    return (void*) 0;
}


void *parsePolygon(DisplayInfo *displayInfo, FrameOffset * offset)
{
    char token[MAX_TOKEN_LENGTH];
    TOKEN tokenType;
    int nestingLevel = 0;
    DlObject object;

    static int number = 0;
    char asc[40];
    sprintf(asc, "caPolygon_%d", number++);
    C_writeOpenTag(myParserPtr, "widget", "caPolygon", asc);

    printf("==> parsePolygon\n");
    do {
	switch( (tokenType=getToken(displayInfo,token)) ) {
	case T_WORD:
	    if(!strcmp(token,"object"))
                parseObject(displayInfo, &object, offset, "caPolygon");
	    else
                if(!strcmp(token,"basic attribute"))
                    parseBasicAttribute(displayInfo, "caPolygon");
                else
                    if(!strcmp(token,"dynamic attribute"))
                        parseDynamicAttribute(displayInfo, "caPolygon");
                    else
                        if(!strcmp(token,"points"))
                            parsePolygonPoints(displayInfo, "caPolygon");
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

    C_writeCloseTag(myParserPtr, "widget");

    return (void*) 0;
}

void *parseRectangle(DisplayInfo *displayInfo, FrameOffset * offset)
{
    char token[MAX_TOKEN_LENGTH];
    TOKEN tokenType;
    int nestingLevel = 0;
    DlObject object;

    static int number = 0;
    char asc[40];
    sprintf(asc, "caRectangle_%d", number++);
    C_writeOpenTag(myParserPtr, "widget", "caCircleBar", asc);

    Qt_handleString("form", "enum", "caCircleBar::Rectangle");

    printf("==> parseRectangle\n");
    do {
	switch( (tokenType=getToken(displayInfo,token)) ) {
	case T_WORD:
	    if(!strcmp(token,"object"))
                parseObject(displayInfo, &object, offset, "caCircleBar");
	    else if(!strcmp(token,"basic attribute")) {
                parseBasicAttribute(displayInfo, "caCircleBar");
            } else if(!strcmp(token,"dynamic attribute")) {
                parseDynamicAttribute(displayInfo, "caCircleBar");
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
    
    C_writeCloseTag(myParserPtr, "widget");
    return (void*) 0;
}

void *parseOval(DisplayInfo *displayInfo, FrameOffset * offset)
{
    char token[MAX_TOKEN_LENGTH];
    TOKEN tokenType;
    int nestingLevel = 0;
    DlObject object;

    static int number = 0;
    char asc[40];
    sprintf(asc, "caCircleBar_%d", number++);
    C_writeOpenTag(myParserPtr, "widget", "caCircleBar", asc);

    Qt_handleString("form", "enum", "caCircleBar::Circle");

    printf("==> parseOval\n");
    do {
	switch( (tokenType=getToken(displayInfo,token)) ) {
	case T_WORD:
	    if(!strcmp(token,"object"))
                parseObject(displayInfo, &object, offset, "caCircleBar");
	    else if(!strcmp(token,"basic attribute")) {
                parseBasicAttribute(displayInfo, "caCircleBar");
            } else if(!strcmp(token,"dynamic attribute")) {
                parseDynamicAttribute(displayInfo, "caCircleBar");
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

    C_writeCloseTag(myParserPtr, "widget");
    
    return (void*) 0;
}

void *parseArc(DisplayInfo *displayInfo, FrameOffset * offset)
{
    char token[MAX_TOKEN_LENGTH];
    TOKEN tokenType;
    int nestingLevel = 0;
    DlObject object;

    static int number = 0;
    char asc[40];
    sprintf(asc, "caArc_%d", number++);
    C_writeOpenTag(myParserPtr, "widget", "caArc", asc);

    printf("==> parseArc\n");
    do {
	switch( (tokenType=getToken(displayInfo,token)) ) {
	case T_WORD:
	    if (!strcmp(token,"object"))
                parseObject(displayInfo, &object, offset, "caArc");
	    if (!strcmp(token,"basic attribute")) {
                //parseBasicAttribute(displayInfo,&(dlArc->attr));
            } else {
                if (!strcmp(token,"dynamic attribute")) {
                    //parseDynamicAttribute(displayInfo,&(dlArc->dynAttr));
                } else {
                    if (!strcmp(token,"begin")) {
                        getToken(displayInfo,token);
                        getToken(displayInfo,token);
                        //dlArc->begin = atoi(token);
                    } else
                        if (!strcmp(token,"path")) {
                            getToken(displayInfo,token);
                            getToken(displayInfo,token);
                            //dlArc->path = atoi(token);
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

    C_writeCloseTag(myParserPtr, "widget");
    
    return (void*) 0;
}


void *parseText(DisplayInfo *displayInfo, FrameOffset * offset)
{
    char token[MAX_TOKEN_LENGTH];
    TOKEN tokenType;
    int nestingLevel = 0;
    DlObject object;
    TextAlign align;
    int alignFound=False;
    int i = 0;

    static int number = 0;
    char asc[40];
    sprintf(asc, "caLabel_%d", number++);
    C_writeOpenTag(myParserPtr, "widget", "caLabel", asc);
    Qt_handleString("frameShape", "enum", "QFrame::NoFrame");

    printf("==> parseText\n");

    do {
	switch( (tokenType=getToken(displayInfo,token)) ) {
	case T_WORD:
	    if(!strcmp(token,"object")) {
                parseObject(displayInfo, &object, offset, "caLabel");
	    } else if(!strcmp(token,"basic attribute")) {
                parseBasicAttribute(displayInfo, "caLabel");
            } else if(!strcmp(token,"dynamic attribute")) {
                parseDynamicAttribute(displayInfo, "caLabel");
            }
	    else if(!strcmp(token,"textix")) {
		getToken(displayInfo,token);
		getToken(displayInfo,token);
                Qt_handleString("text", "string",  token);
                Qt_handleString("fontScaleMode", "enum",  "ESimpleLabel::Height");
	    } else if(!strcmp(token,"align")) {
		int found=0;

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

                for(i=(int)FIRST_TEXT_ALIGN;i<(int)(FIRST_TEXT_ALIGN+NUM_TEXT_ALIGNS); i++) {
		    if(!strcmp(token,stringValueTable[i])) {
			align = (TextAlign) i;
			found=1;
			break;
		    }
		}
                /* Backward compatibility */
		if(!found) {
		    if(!strcmp(token,"vert. top")) {
			align = HORIZ_LEFT;
		    } else if(!strcmp(token,"vert. centered")) {
			align = HORIZ_CENTER;
		    } else if(!strcmp(token,"vert. bottom")) {
			align = HORIZ_RIGHT;
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

     if(!alignFound) Qt_handleString("alignment", "set", "Qt::AlignAbsolute|Qt::AlignLeft|Qt::AlignVCenter");

    C_writeCloseTag(myParserPtr, "widget");

    return (void*) 0;
}

void parsePolylinePoints(DisplayInfo *displayInfo, char *widget, int offsetX, int offsetY)
{
    char token[MAX_TOKEN_LENGTH];
    TOKEN tokenType;
    int nestingLevel;
    int x, y;
    char points[1024];

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
    DlObject object;
    int offsetX=0, offsetY=0;

    static int number = 0;
    char asc[40];

    sprintf(asc, "caPolyLine_%d", number++);
    C_writeOpenTag(myParserPtr, "widget", "caPolyLine", asc);
    
    printf("==> parsePolyline\n");
    do {
	switch( (tokenType=getToken(displayInfo,token)) ) {
	case T_WORD:
            if(!strcmp(token,"object")) {
                parseObject(displayInfo, &object, offset, "caPolyLine");
                offsetX = object.x;
                offsetY = object.y;
            } else if(!strcmp(token,"basic attribute")) {
                parseBasicAttribute(displayInfo, "caPolyLine");
            } else if(!strcmp(token,"dynamic attribute")) {
                parseDynamicAttribute(displayInfo, "caPolyLine");
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


    C_writeCloseTag(myParserPtr, "widget");

    return (void*) 0;
}

void *parseBar(DisplayInfo *displayInfo, FrameOffset * offset)
{
    char token[MAX_TOKEN_LENGTH];
    char COLORMODE[MAX_TOKEN_LENGTH] = "static";
    TOKEN tokenType;
    int nestingLevel = 0;
    DlObject object;

    static int number = 0;
    char asc[40];
    sprintf(asc, "caThermo_%d", number++);
    C_writeOpenTag(myParserPtr, "widget", "caThermo", asc);


    printf("==> parseBar\n");
    do {
	switch( (tokenType=getToken(displayInfo,token)) ) {
	case T_WORD:
	    if(!strcmp(token,"object")) {
                parseObject(displayInfo, &object, offset, "caThermo");
	    } else if(!strcmp(token,"monitor")) {
                parseMonitor(displayInfo, "caThermo");
	    } else if(!strcmp(token,"label")) {
		getToken(displayInfo,token);
		getToken(displayInfo,token);
                if(!strcmp(token,"none")) {
                    //dlBar->label = LABEL_NONE;
                } else if(!strcmp(token,"no decorations")) {
                    //dlBar->label = NO_DECORATIONS;
                } else if(!strcmp(token,"outline")) {
                    //dlBar->label = OUTLINE;
                } else if(!strcmp(token,"limits")) {
                    //dlBar->label = LIMITS;
                } else {
                    if(!strcmp(token,"channel")) {
                        //dlBar->label = CHANNEL;
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
                    //dlBar->direction = UP;
                } else if(!strcmp(token,"right")) {
                    //dlBar->direction = RIGHT;
                } else if(!strcmp(token,"down")) {
                    //dlBar->direction = DOWN;
                } else if(!strcmp(token,"left")) {
                    //dlBar->direction = LEFT;
                }
	    } else if(!strcmp(token,"fillmod")) {
		getToken(displayInfo,token);
		getToken(displayInfo,token);
                if(!strcmp(token,"from edge")) {
                    //dlBar->fillmod = FROM_EDGE;
                } else if(!strcmp(token,"from center")) {
                    //dlBar->fillmod = FROM_CENTER;
                }
	    } else if(!strcmp(token,"limits")) {
                parseLimits(displayInfo, "caThermo", 0);
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

    Qt_setColorMode("caThermo", COLORMODE);
    C_writeCloseTag(myParserPtr, "widget");
    
    return (void *) 0;
}

void *parseIndicator(DisplayInfo *displayInfo, FrameOffset * offset)
{
    char token[MAX_TOKEN_LENGTH];
    char COLORMODE[MAX_TOKEN_LENGTH] = "static";
    TOKEN tokenType;
    int nestingLevel = 0;
    DlObject object;

    static int number = 0;
    char asc[40];
    sprintf(asc, "caIndicator_%d", number++);
    C_writeOpenTag(myParserPtr, "widget", "caIndicator", asc);


    printf("==> parseIndicator\n");
    do {
	switch( (tokenType=getToken(displayInfo,token)) ) {
	case T_WORD:
	    if(!strcmp(token,"object")) {
                parseObject(displayInfo, &object, offset, "caIndicator");
	    } else if(!strcmp(token,"monitor")) {
                parseMonitor(displayInfo, "caIndicator");
	    } else if(!strcmp(token,"label")) {
		getToken(displayInfo,token);
		getToken(displayInfo,token);
                if(!strcmp(token,"none")) {
                    //dlIndicator->label = LABEL_NONE;
                } else if(!strcmp(token,"no decorations")) {
                    //dlIndicator->label = NO_DECORATIONS;
                } else if(!strcmp(token,"outline")) {
                    //dlIndicator->label = OUTLINE;
                } else if(!strcmp(token,"limits")) {
                    //dlIndicator->label = LIMITS;
                } else if(!strcmp(token,"channel")) {
                    //dlIndicator->label = CHANNEL;
                }
	    } else if(!strcmp(token,"clrmod")) {
		getToken(displayInfo,token);
		getToken(displayInfo,token);
                strcpy(COLORMODE, token);
	    } else if(!strcmp(token,"direction")) {
		getToken(displayInfo,token);
		getToken(displayInfo,token);
                if(!strcmp(token,"up")){
                    //dlIndicator->direction = UP;
                } else if(!strcmp(token,"right")) {
                    //dlIndicator->direction = RIGHT;
                } else if(!strcmp(token,"down")) {
                    //dlIndicator->direction = DOWN;
                } else if(!strcmp(token,"left")) {
                    //dlIndicator->direction = LEFT;
                }
	    } else if(!strcmp(token,"limits")) {
                parseLimits(displayInfo, "caIndicator", 0);
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

    Qt_setColorMode("caIndicator", COLORMODE);
    C_writeCloseTag(myParserPtr, "widget");

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
/*
            case '$' : c=getc(filePtr);
//only do macro substitution if in execute mode

                if((parsingCompositeFile) && c == '(' ) {
		    savedState = INQUOTE;
		    state = INMACRO;
		} else {
		    *w++ = '$';
		    *w++ = c;
		}
*/
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
    static Boolean init = True;
    int first = 1;
    static int veryFirst = True;

    if(veryFirst) {
        veryFirst = False;
        int bclr = displayInfo->drawingAreaBackgroundColor;

        Qt_setColormain("", displayInfo->dlColormap->dl_color[bclr].r,
                            displayInfo->dlColormap->dl_color[bclr].g,
                            displayInfo->dlColormap->dl_color[bclr].b,
                            255);

        C_writeStyleSheet(myParserPtr);

        C_writeOpenTag(myParserPtr, "widget", "QWidget", "centralWidget");
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
    if(tokenType == T_EOF) init = True;
    return tokenType;
}

void *parseDisplay(DisplayInfo *displayInfo)
{
    char token[MAX_TOKEN_LENGTH];
    TOKEN tokenType;
    int nestingLevel = 0;
    DlObject object;
    int clr, bclr;
    char cmap[MAX_TOKEN_LENGTH];

    printf("parseDisplay\n");
    FrameOffset offset;
    offset.frameX = offset.frameY =0; 

    do {
        switch( (tokenType=getToken(displayInfo,token)) ) {
	case T_WORD:
	    if(!strcmp(token,"object")) {
                parseObject(displayInfo, &object, &offset, "");
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
                //grid.gridSpacing = atoi(token);
	    } else if(!strcmp(token,"gridOn")) {
		getToken(displayInfo,token);
		getToken(displayInfo,token);
                //grid.gridOn = atoi(token);
	    } else if(!strcmp(token,"snapToGrid")) {
		getToken(displayInfo,token);
		getToken(displayInfo,token);
                //grid.snapToGrid = atoi(token);
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

    //C_writeOpenTag(myParserPtr, "widget", "QWidget", "centralWidget");


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
                //strcpy(dlFile->name,token);
	    }
	    if(!strcmp(token,"version")) {
		getToken(displayInfo,token);
		getToken(displayInfo,token);
                //dlFile->versionNumber = atoi(token);
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

    printf("parseDlColor\n");

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
    printf("parseOldDlColor\n");
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

    printf("parseColormap\n");

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



