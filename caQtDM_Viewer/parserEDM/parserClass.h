#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "utility.h"
#include "tag_pkg.h"
#include "myParserEDM.h"
#include <vector>

#define AWC_MAXTMPLPARAMS 30
#define AWC_TMPLPARAMSIZE 35
#define AWC_MAXTEMPLINFO 600
#define LONGSTRING       4096

enum classType {activeLine, activeRectangle, activeCircle, activeArc ,
                xyGraph, activePip, activeXText, activeXTextDsp, TextUpdate, TextEntry, Byte, // Zai added three items
                activeMeter, activeBar, activeMessageBox,
                activeMotifSlider, activeButton, activeMenuButton, activeRadioButton, activeChoiceButton, // Zai added one item
                activeMessageButton, activeExitButton, relatedDisplay, shellCmdButton, activeGroup,
               unknown};

struct RGB {
    int r,g,b;
};

class parserClass
{
public:
    parserClass(char *filename);
    ~parserClass();

    int loadFile(myParserEDM *myParser);
    int loadHeader (myParserEDM *myParser, FILE *f);
    void readCommentsAndVersion (FILE *f);
    void incLine ( void );
    void addVisibilityCalc(myParserEDM *myParser, char *visPvExpStr, char *minVisString, char *maxVisString);

private:

    int invalidFile, invalidBgColor;
    int fileLineNumber;
    FILE *openAny (char *name,char *mode );
    int fileClose ( FILE *f);
    FILE *fileOpen (char *name, char *mode);
    char fileRev[31+1], fileNameAndRev[287+1];
    char fileName[1000];
    int haveComments;
    unknownTagList unknownTags;

    struct group
    {
	int major, minor, release, x, y, w, h;
	char name[80];
    };
 
    vector<group> groups;
    int openGroups;


    int x,y,w,h,major, minor,release, alignment;
    void *ci;
    char fontTag[80] ;
    int left;
    int fgColor, bgColor, fgColorMode, bgColorMode, visInverted, autoSize, border, lineThk;
    int zero;
    double dzero;
    int one ;
    int ten;
    int useDisplayBg;

    char minVisString[39+1];
    char maxVisString[39+1];
    expStringClass alarmPvExpStr, visPvExpStr;
    expStringClass value;

    char emptyStr[80];
    char myCopy[1024];

    int widgetNumber;
    char widgetName[80];

    char defaultFontTag[127+1];
    char defaultCtlFontTag[127+1];
    char defaultBtnFontTag[127+1];
    char defaultPvType[15+1], bufDefaultPvType[15+1];
    int defaultAlignment;
    int defaultCtlAlignment;
    int defaultBtnAlignment;
    char title[127+1];
    char paramValue[AWC_MAXTMPLPARAMS][AWC_TMPLPARAMSIZE+1];
    char templInfo[AWC_MAXTEMPLINFO+1];

    // reading screen data
    int defaultTextFgColor, bufDefaultTextFgColor;
    int defaultFg1Color, bufDefaultFg1Color;
    int defaultFg2Color, bufDefaultFg2Color;
    int defaultBgColor, bufDefaultBgColor;
    int defaultTopShadowColor, bufDefaultTopShadowColor;
    int defaultBotShadowColor, bufDefaultBotShadowColor;
    int defaultOffsetColor, bufDefaultOffsetColor;
    int gridSpacing;
    int gridActive;
    int gridShow;
    int orthogonal;
    int disableScroll;
    int bgPixmapFlag;
    int  numParamValues;

    // reading activeXTextDspClass data
    expStringClass pvExpStr, svalPvExpStr, fgPvExpStr;
    expStringClass defDir, pattern;
    int formatType, colormode, editable, autoHeight, isWidget, limitsFromDb, colorMode, index;
    int smartRefresh, useKp, changeValOnLoseFocus, fastUpdate, isDate, isFile, autoSelect;
    int updatePvOnDrop, useHexPrefix,fileComponent, dateAsFileName, showUnits, useAlarmBorder;
    int newPositioning, inputFocusUpdatesAllowed, clipToDspLimits, isPassword, characterMode;
    int noExecuteClipMask, changeCallbackFlag, objType;
    efInt efPrecision;
    char fieldLenInfo[7+1];
    char id[31+1];
    int nullDetectMode;
    // Zai added
    // reading ByteClass data
    int numBits;

    // reading activeRectangleClass data
    int lineColor, fillColor, lineColorMode, fill, fillColorMode, lineWidth, lineStyle, invisible;

    // reading activeLineClass data
    int closePolygon, arrows, numPoints;

    // reading activeArcClass data
    efDouble efStartAngle, efTotalAngle;
    int fillMode;

    // reading activeBarClass
    int barColor, barColorMode, labelType, showScale, horizontal;
    expStringClass readPvExpStr,  readMinExpStr, readMaxExpStr, nullPvExpStr, label, precisionExpStr ;
    expStringClass labelTicksExpStr, majorTicksExpStr, minorTicksExpStr, barOriginValExpStr;
    char scaleFormat[15+1];

    // reading activeMeterClass
    int meterColorMode, meterColor, scaleColor, scaleColorMode, labelColor, tsColor, bsColor;
    int trackDelta, labelIntervals, majorIntervals, minorIntervals, needleType, shadowMode;
    int scaleLimitsFromDb;
    expStringClass controlPvExpStr, scaleMinExpStr,scaleMaxExpStr, scalePrecExpStr, labIntExpStr;
    char literalLabel[39+1], scaleFontTag[63+1], labelFontTag[63+1];
    double meterAngle;
    double a180;

    // reading  activeMotifSliderClass
    int shadeColor, topColor, botColor, increment, controlLabelType, showLimits, showLabel, showValue;
    int showSavedValue, orientation;
    double dincrement;
    expStringClass controlPvName, controlLabelName, savedValuePvName;
    efDouble efScaleMin, efScaleMax;

    // reading relatedDisplayClass
    int topShadowColor, botShadowColor, icon, swapButtons;
    int noEdit, ofsX, ofsY, useFocus, button3Popup, numPvs, n1, n2, numDsps;
    static const int NUMPVS = 4;
    static const int maxDsps = 24;
    expStringClass sourceExpString[NUMPVS];
    expStringClass destPvExpString[NUMPVS];
    expStringClass displayFileName[maxDsps];
    expStringClass symbolsExpStr[maxDsps];
    expStringClass relLabel[maxDsps];
    int setPostion[maxDsps];
    int allowDups[maxDsps];
    int cascade[maxDsps];
    int propagateMacros[maxDsps];
    int closeAction[maxDsps];
    int replaceSymbols[maxDsps];
    expStringClass buttonLabel;
    expStringClass colorPvExpString;
    expStringClass helpCommandExpString;

    // reading activePipClass
   int displaySource, setSize, sizeOfs, noScroll, ignoreMultiplexors, center;
   expStringClass labelPvExpStr, fileNameExpStr;
   expStringClass pipLabel[maxDsps];

   // reading xyGraphClass
   #define XYGC_K_MAX_TRACES 20
   static const int NUM_Y_AXES = 2;
   int plotAreaBorder, autoScaleBothDirections, autoScaleTimerMs, autoScaleThreshPct;
   int gridColor, plotMode, updateTimerValue, resetMode, numTraces;
   int xAxis, xAxisStyle, xAxisSource, xAxisTimeFormat;
   expStringClass graphTitle, xLabel, yLabel, y2Label;
   expStringClass traceCtlPvExpStr, trigPvExpStr, resetPvExpStr;
   expStringClass xPvExpStr[XYGC_K_MAX_TRACES], yPvExpStr[XYGC_K_MAX_TRACES], nPvExpStr[XYGC_K_MAX_TRACES];
   int plotStyle[XYGC_K_MAX_TRACES], y2Scale[XYGC_K_MAX_TRACES];
   int plotColor[XYGC_K_MAX_TRACES], xylineThk[XYGC_K_MAX_TRACES], xylineStyle[XYGC_K_MAX_TRACES];
   int plotSymbolType[XYGC_K_MAX_TRACES], plotUpdateMode[XYGC_K_MAX_TRACES], opMode[XYGC_K_MAX_TRACES];
   int xSigned[XYGC_K_MAX_TRACES], ySigned[XYGC_K_MAX_TRACES];
   efDouble xMin, xMax;
   efInt xNumLabelIntervals, xNumMajorPerLabel,  xNumMinorPerMajor,xAnnotationPrecision ;
   int xLabelGrid, xMajorGrid, xMinorGrid, xAnnotationFormat, xGridMode, xAxisSmoothing;
   int y1Axis[NUM_Y_AXES], y1AxisStyle[NUM_Y_AXES], y1AxisSource[NUM_Y_AXES];
   efDouble y1Min[NUM_Y_AXES], y1Max[NUM_Y_AXES];
   efInt y1NumLabelIntervals[NUM_Y_AXES],  y1NumMajorPerLabel[NUM_Y_AXES];
   efInt y1NumMinorPerMajor[NUM_Y_AXES], y1AnnotationPrecision[NUM_Y_AXES];
   int y1LabelGrid[NUM_Y_AXES], y1MajorGrid[NUM_Y_AXES], y1MinorGrid[NUM_Y_AXES];
   int y1AnnotationFormat[NUM_Y_AXES], y1GridMode[NUM_Y_AXES], y1AxisSmoothing[NUM_Y_AXES];

   // reading activeMenuButtonClass
   int inconsistentColor;
   expStringClass colorPvExpStr;

   // reading activeRadioButtonClass
   int buttonColor, selectColor;

   // reading activeMessageButtonClass
   int onColor, offColor, toggle,  pressAction, releaseAction, _3D;
   int useEnumNumeric, usePassword,lock ;
   expStringClass sourcePressPvExpString, sourceReleasePvExpString, destPvExpStringM, visPvExpString;
   expStringClass onLabel, offLabel;
   char pw[31+1];

   // reading shellCmdButtonClass

   static const int maxCmds = 20;
   double threadSecondsToDelay, autoExecInterval;
   int numCmds, includeHelpIcon, execCursor;
   int oneShot, timerActive, timerValue, multipleInstancesAllowed;
   char requiredHostName[15+1];
   expStringClass labelS[maxCmds];
   expStringClass shellCommand[maxCmds];

   // set font based on EDM font string - Zai added
   void setFont(myParserEDM *myParser, char fontTag[80]);

   // verify the actual x and y for widgets inside groups. Necessary because while EDM widgets work with absolute values
   // inside groups, QT widgets work with relative values inside caFrame

   void adjustGeometry(int *x, int *y);
};
