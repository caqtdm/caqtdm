//  edm - extensible display manager

//  Copyright (C) 1999 John W. Sinclair

//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.

//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.

//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.


#include <math.h>
#include "parserClass.h"
#include <QStringList>
#include <QDebug>

static char *alignEnumStr[3] = {"left","center","right"};
static int alignEnum[3] = {0,1,2};

static char *pixmapEnumStr[3] = {"perEnvVar","never","always"};
static int pixmapEnum[3] = {0,1,2};

static int nullCondNullEqCtl = 0;
static char *nullCondEnumStr[3] = {"nullEqCtl","nullEq0","disabled"};
static int nullCondEnum[3] = {0,1,2};

static int formatTypeDefault = 0;
static char *formatTypeEnumStr[7] = {"default","float","gfloat","exponential","decimal","hex","string"};
static int formatTypeEnum[7] = {0,1,2,3,4,5,6};

static int fileCompFullPath = 0;
static char *fileCompEnumStr[3] = {"fullPath","nameAndExt","name"};
static int fileCompEnum[3] = {0,1,2};

static int objTypeUnknown = 0;
static char *objTypeEnumStr[4] = {"unknown","graphics","monitors","controls"};
static int objTypeEnum[4] = {0,1,2,3};

static int styleSolid = 0;
static char *styleEnumStr[2] = {"solid","dash"};
static int styleEnum[2] = {0,1};

static int lit = 1;
static char *labelTypeEnumStr[2] = {"pvName","literal"};
static int labelTypeEnum[2] = {0,1};

static int horz = 1;
static char *orienTypeEnumStr[2] = {"vertical","horizontal"};
static int orienTypeEnum[2] = {0,1};

static int labelTypeLiteral = 0;
static char *labelEnumStr[3] = {"literal","pvLabel","pvName"};
static int labelEnum[3] = {0,1,2};

static int formatTypeFfloat = 0;
static char *formatTypeEnumStrM[3] = {"ffloat","exponential","gfloat"};
static int formatTypeEnumM[3] = {0,1,2};

static int setPosOriginal = 0;
static char *setPosEnumStr[3] = {"original","button","parentWindow"};
static int setPosEnum[3] = {0,1,2};

static int displaySourceFromPV = 0;
static char *displaySourceEnumStr[3] = {"stringPV","file", "menu"};
static int displaySourceEnum[3] = {0,1,2};

static int plotModePlotNPtsAndStop = 0;
static char *plotModeEnumStr[2] = {"plotNPtsAndStop", "plotLastNPts"};
static int plotModeEnum[2] = {0,1};

static int opModeScope = 0;
static char *opModeEnumStr[2] = {"scope","plot"};
static int opModeEnum[2] = {0,1};

static int plotStyleLine = 0;
static char *plotStyleEnumStr[4] = {"line","point","needle","single point"};
static int plotStyleEnum[4] = {0,1,2,3};

static int updateModexAndY = 0;
static char *updateModeEnumStr[5] = {"xAndY","xOrY","x","y","trigger"};
static int updateModeEnum[5] = {0,1,2,3,4};

static int symbolNone = 0;
static char *symbolEnumStr[4] = {"none","circle","square","diamond"};
static int symbolEnum[4] = {0,1,2,3};

static int xAxisStyleLinear = 0;
static char *xAxisStyleEnumStr[4] = {"linear","log10","time","loc_log10(time)"};
static int xAxisStyleEnum[4] = {0,1,2,3};

static int yAxisStyleLinear = 0;
static char *yAxisStyleEnumStr[2] = {"linear","log10"};
static int yAxisStyleEnum[2] = {0,1};

static int axisSrcFromPv = 0;
static char *axisSrcEnumStr[3] = {"fromPv","fromUser","AutoScale"};
static int axisSrcEnum[3] = {0,1, 2};

static int timeFormatSec = 0;
static char *timeFormatEnumStr[2] = {"seconds","dateTime"};
static int timeFormatEnum[2] = {0,1};

static int annoFormatF = 0;
static char *annoFormatEnumStr[2] = {"f","g"};
static int annoFormatEnum[2] = {0,1};

static int resetModeIfNotZero = 0;
static char *resetModeEnumStr[2] = {"ifNotZero","ifZero"};
static int resetModeEnum[2] = {0,1};

static int arrowsNone = 0;
static char *arrowsEnumStr[4] = {"none","from","to","both"};
static int arrowsEnum[4] = {0,1,2,3};

static int fillModeChord = 0;
static char *fillModeEnumStr[2] = {"chord","pie"};
static int fillModeEnum[2] = {0,1};


static int perEnvVar = 0;

static RGB rgb[100] = {
    { 65535, 65535, 65535 },
    { 60652, 60652, 60652 },
    { 56026, 56026, 56026 },
    { 51400, 51400, 51400 },
    { 48059, 48059, 48059 },
    { 44718, 44718, 44718 },
    { 40606, 40606, 40606 },
    { 37265, 37265, 37265 },
    { 34181, 34181, 34181 },
    { 30840, 30840, 30840 },
    { 26985, 26985, 26985 },
    { 23130, 23130, 23130 },
    { 17990, 17990, 17990 },
    { 11565, 11565, 11565 },
    {     0,     0,     0 },
    {     0 ,65535,     0 },
    {     0, 57568,     0 },
    {     0, 49344,     0 },
    {     0, 41120,     0 },
    {     0, 32896,     0 },
    { 65535,     0,     0 },
    { 57568,     0,     0 },
    { 49344,     0,     0 },
    { 41120,     0,     0 },
    { 32896,     0,     0 },
    {     0,     0, 65535 },
    {     0,     0, 57568 },
    {     0,     0, 49344 },
    {     0,     0, 41120 },
    {     0,     0, 32896 },
    {     0, 65535, 65535 },
    {     0, 57568, 57568 },
    {     0, 49344, 49344 },
    {     0, 41120, 41120 },
    {     0, 32896, 32896 },
    { 65535, 65535,     0 },
    { 57568, 57568,     0 },
    { 49344, 49344,     0 },
    { 41120, 41120,     0 },
    { 32896, 32896,     0 },
    { 65535, 45232, 24672 },
    { 57568, 39578, 21588 },
    { 49344, 33924, 18504 },
    { 41120, 28270, 15420 },
    { 32896, 22616, 12336 },
    { 65535,     0, 65535 },
    { 49344,     0, 49344 },
    { 32896,     0, 32896 },
    {     0,     0,     0 },
    {     0,     0,     0 },
    { 52736, 56320, 52480 },
    { 47360, 50688, 47104 },
    { 42496, 45568, 42240 },
    { 57600, 63488, 45312 },
    { 51712, 57088, 40704 },
    { 62708, 56026, 43176 },
    { 47031, 42148, 32382 },
    { 31354, 28013, 21588 },
    { 46336, 63744, 55040 },
    { 41472, 57344, 49408 },
    { 49664, 55808, 55552 },
    { 44544, 50176, 49920 },
    { 39936, 45056, 44800 },
    { 45056, 55808, 63744 },
    { 40448, 50176, 57344 },
    { 52480, 51712, 56576 },
    { 47104, 46336, 50688 },
    { 42240, 41472, 45568 },
    { 56832, 50176, 64256 },
    { 50944, 45045, 57600 },
    { 56576, 51712, 56576 },
    { 50688, 46336, 50688 },
    { 45568, 41472, 45568 },
    { 64256, 60176, 60416 },
    { 57600, 45056, 54272 },
    { 65535, 38550, 43176 },
    { 49344, 29041, 32382 }
};


parserClass::parserClass (char *filename)
{
    strcpy( fileName, filename);
    fileLineNumber = 0;
    zero = 0;
    dzero = 0.0;
    one = 1;
    ten = 10;
    a180 = 180;
    useDisplayBg = 1;
    widgetNumber = 0;
    openGroups = 0;
}

void parserClass::adjustGeometry(int *x, int *y)
{
	int i;
	if(openGroups > 0)
	{
		for(i = 0; i < openGroups; i++)
		{
			*x = *x - groups[i].x;
			*y = *y - groups[i].y;
		}
	}
} 


void parserClass::readCommentsAndVersion (FILE *f)
{
    char oneLine[255+1], buf[255+1], buf2[255+1], *tk, *context, *context2;
    int moreComments = 1, checkForRev = 1, checkForEdmComments = 1;
    bool isSymbolFile = false;
    haveComments = 0;

    strcpy( fileNameAndRev, fileName );

    do {
        readStringFromFile( oneLine, 255+1, f ); incLine();
        strcpy( buf, oneLine );
        context = NULL;
        tk = strtok_r( buf, " \t\n", &context );

        if ( !tk || ( tk[0] == '#' ) ) {

            if ( !isSymbolFile ) {

                // check for cvs/rcs revision info
                if ( tk && ( tk[0] == '#' ) ) {

                    if ( checkForEdmComments ) {
                        strcpy( buf2, oneLine );
                        context2 = NULL;
                        tk = strtok_r( buf2, " \t\n#", &context2 );
                        if ( tk ) {
                            if ( strcmp( tk, "<<<edm-generated-comments>>>" ) == 0 ) {
                                checkForEdmComments = 0;
                                haveComments = 1;
                            }
                        }
                    }

                    if ( checkForRev ) {
                        strcpy( buf2, oneLine );
                        context2 = NULL;
                        tk = strtok_r( buf2, " \t\n#", &context2 );
                        if ( tk ) {
                            if ( strcmp( tk, "$InvalidBgColor:" ) == 0 ) {
                                invalidFile = 1;
                                invalidBgColor = 0;
                                checkForRev = 0; // use first rev found, don't check any more
                                tk = strtok_r( NULL, " \t\n#", &context2 );
                                if ( tk ) {
                                    char *nonInt;
                                    invalidBgColor = strtol( tk, &nonInt, 10 );
                                    Strncat( fileNameAndRev, " (", 287 );
                                    //Strncat( fileNameAndRev, parserClass_str214, 287 );
                                    Strncat( fileNameAndRev, ")", 287 );
                                    //strncpy( fileRev, parserClass_str214, 31 );
                                    fileRev[31] = 0;
                                }
                            } else if ( strcmp( tk, "$Revision:" ) == 0 ) {
                                checkForRev = 0; // use first rev found, don't check any more
                                tk = strtok_r( NULL, " \t\n#", &context2 );
                                if ( tk ) {
                                    Strncat( fileNameAndRev, " (", 287 );
                                    Strncat( fileNameAndRev, tk, 287 );
                                    Strncat( fileNameAndRev, ")", 287 );
                                    strncpy( fileRev, tk, 31 );
                                    fileRev[31] = 0;
                                }
                            }
                        }
                    }
                }
            }
        }
        else {
            moreComments = 0;
        }
    } while ( moreComments );

    sscanf( oneLine, "%d %d %d\n", &major, &minor, &release );
    printf("major=%d minor=%d release=%d\n", major, minor, release);

}

FILE *parserClass::openAny (char *name, char *mode )
{
    FILE *f;

    f = fopen( name, mode );
    if ( f ) {
        return f;
    }
    return NULL;
}


void parserClass::incLine ( void )
{
    fileLineNumber++;
}

void parserClass::addVisibilityCalc(myParserEDM *myParser, char *visPvExpStr, char *minVisString, char *maxVisString)
{
    int status;
    char visibilityCalc[100];
    char newStr[80];
    visibilityCalc[0] = '\0';
    if(strlen(visPvExpStr) > 0) {
        myParser->Qt_extractString(visPvExpStr, newStr, &status);
        if(status) {
            myParser->Qt_handleString("channel", "string", newStr);
        } else {
            myParser->Qt_handleString("channel", "string", visPvExpStr);
        }
        myParser->Qt_handleString("visibility", "string", "Calc");
    }
    if(strlen(minVisString) > 0) sprintf(visibilityCalc, "A>%s", minVisString);
    if((strlen(maxVisString) > 0) && (strlen(minVisString) > 0)) strcat(visibilityCalc, "&&A<=");
    if(strlen(maxVisString) > 0) strcat(visibilityCalc, maxVisString);
    if(strlen(visibilityCalc) > 0) myParser->Qt_handleString("visibilityCalc", "string", visibilityCalc);
}


int parserClass::loadHeader (myParserEDM *myParser, FILE *f)  {

    int stat, retStat = 1;
    x=0; y=0; w=100; h=150;
    tagClass tag;

    tag.init();
    tag.loadR( "beginScreenProperties" );
    tag.loadR( unknownTags );
    tag.loadR( "major", &major );
    tag.loadR( "minor", &minor );
    tag.loadR( "release", &release );
    tag.loadR( "x", &x );
    tag.loadR( "y", &y );
    tag.loadR( "w", &w );
    tag.loadR( "h", &h );
    tag.loadR( "font", 63, defaultFontTag );
    tag.loadR( "fontAlign", 3, alignEnumStr, alignEnum, &defaultAlignment, &left );
    tag.loadR( "ctlFont", 63, defaultCtlFontTag );
    tag.loadR( "ctlFontAlign", 3, alignEnumStr, alignEnum, &defaultCtlAlignment, &left );
    tag.loadR( "btnFont", 63, defaultBtnFontTag );
    tag.loadR( "btnFontAlign", 3, alignEnumStr, alignEnum, &defaultBtnAlignment, &left );
    tag.loadR( "fgColor", ci, &fgColor );
    tag.loadR( "bgColor", ci, &bgColor );
    tag.loadR( "textColor", ci, &defaultTextFgColor );
    tag.loadR( "ctlFgColor1", ci, &defaultFg1Color );
    tag.loadR( "ctlFgColor2", ci, &defaultFg2Color );
    tag.loadR( "ctlBgColor1", ci, &defaultBgColor );
    tag.loadR( "ctlBgColor2", ci, &defaultOffsetColor );
    tag.loadR( "topShadowColor", ci, &defaultTopShadowColor );
    tag.loadR( "botShadowColor", ci, &defaultBotShadowColor );
    tag.loadR( "title", 127, title, emptyStr );
    tag.loadR( "showGrid", &gridShow, &zero );
    tag.loadR( "snapToGrid", &gridActive, &zero );
    tag.loadR( "gridSize", &gridSpacing, &ten );
    tag.loadR( "orthoLineDraw", &orthogonal, &zero );
    tag.loadR( "pvType", 15, defaultPvType, emptyStr );
    tag.loadR( "disableScroll", &disableScroll, &zero );
    tag.loadR( "pixmapFlag", 3, pixmapEnumStr, pixmapEnum, &bgPixmapFlag, &perEnvVar );
    tag.loadR( "templateParams", AWC_MAXTMPLPARAMS, AWC_TMPLPARAMSIZE+1, (char *) paramValue, &numParamValues, emptyStr );
    tag.loadR( "templateInfo", AWC_MAXTEMPLINFO, (char *) templInfo, emptyStr );
    tag.loadR( "endScreenProperties" );

    stat = tag.readTags( f, "endScreenProperties" );

    if ( !( stat & 1 ) ) {
        retStat = stat;
    }

    myParser->writeRectangleDimensions(x, y, w, h);

    return retStat;
}

int parserClass::loadFile (myParserEDM *myParser) {

    FILE *f;
    char *gotOne, tagName[255+1], objName[63+1], val[4095+1], defName[255+1];
    int stat, alpha, activeClass;
    int isCompound;
    tagClass tag;

    tag.initLine();

    // read in file
    printf("read in file <%s>\n", fileName);
    f = this->openAny( this->fileName, "r" );
    if ( !f ) return 0;

    readCommentsAndVersion( f );

    loadHeader(myParser, f );

    myParser->writeStyleSheet(rgb[bgColor].r/256, rgb[bgColor].g/256, rgb[bgColor].b/256);
    myParser->Qt_writeOpenTag("widget", "QWidget", "centralWidget");

    // read file and process each leading keyword
    tag.init();
    tag.loadR( "object", 63, objName );
    tag.loadR( "pvdef", 255, defName );
    tag.loadR( "forceLocalPvs" );

    gotOne = tag.getName( tagName, 255, f );

    while ( gotOne ) {

        //printf("name = [%s]\n", tagName );

        if ( strcmp( tagName, "object" ) == 0 ) {

            memset(emptyStr, ' ', 79); emptyStr[79] = '\0';
            alignment = 0;
            left = 0;

            tag.getValue( val, 4095, f, &isCompound );
            tag.decode( tagName, val, isCompound );

            //printf("objName = [%s]\n", objName );

            if ( strcmp( objName,  "xyGraphClass" )  == 0 ) activeClass = xyGraph;
            else if ( strcmp( objName, "activeLineClass" ) == 0 ) activeClass = activeLine;
            else if ( strcmp( objName,  "activePipClass" )  == 0 ) activeClass = activePip;
            else if ( strcmp( objName,  "activeRectangleClass" )  == 0 ) activeClass = activeRectangle;
            else if ( strcmp( objName, "activeCircleClass" ) == 0 ) activeClass = activeCircle;
            else if ( strcmp( objName, "activeArcClass" ) == 0 ) activeClass = activeArc;
            else if ( strcmp( objName, "activeXTextClass" ) == 0 ) activeClass = activeXText;
            else if ( strcmp( objName, "activeXTextDspClass" ) == 0 ) activeClass = activeXTextDsp;
            else if ( strcmp( objName, "activeXTextDspClass:noedit" ) == 0 ) activeClass = activeXTextDsp;  // Zai
            else if ( strcmp( objName, "TextupdateClass" ) == 0 ) activeClass = TextUpdate; // Zai
            else if ( strcmp( objName, "TextentryClass" ) == 0 ) activeClass = TextEntry; // Zai
            else if ( strcmp( objName, "ByteClass" ) == 0 ) activeClass = Byte; // Zai
            else if ( strcmp( objName, "activeMeterClass" ) == 0 ) activeClass = activeMeter;
            else if ( strcmp( objName, "activeBarClass" ) == 0 ) activeClass = activeBar;
            else if ( strcmp( objName, "activeMessageBoxClass" ) == 0 ) activeClass = activeMessageBox;
            else if ( strcmp( objName, "activeMotifSliderClass" ) == 0 ) activeClass = activeMotifSlider;
            else if ( strcmp( objName, "activeButtonClass" ) == 0 ) activeClass = activeButton;
            else if ( strcmp( objName, "activeMenuButtonClass" ) == 0 ) activeClass = activeMenuButton;
            else if ( strcmp( objName, "activeRadioButtonClass" ) == 0 ) activeClass = activeRadioButton;
            else if ( strcmp( objName, "activeChoiceButtonClass" ) == 0 ) activeClass = activeChoiceButton;
            else if ( strcmp( objName, "activeMessageButtonClass" ) == 0 )  activeClass = activeMessageButton;
            else if ( strcmp( objName, "activeExitButtonClass" ) == 0 ) activeClass = activeExitButton;
            else if ( strcmp( objName, "shellCmdClass") == 0) activeClass = shellCmdButton;
            else if ( strcmp( objName, "relatedDisplayClass" ) == 0 )  activeClass = relatedDisplay;
            else if ( strcmp( objName, "activeGroupClass" ) == 0 )  activeClass = activeGroup;
            else {
                // Discard all content up to "endObjectProperties"

                fprintf( stderr, "Line %-d, Error creating object \"%s\"\n", tag.line(), objName );
                gotOne = tag.getName( tagName, 255, f ); // Zai
                continue;                                // Zai

                tag.init();
                tag.loadR( "endObjectProperties", 63, objName );
                stat = tag.readTags( f, "endObjectProperties" );
                // Start looking for leading keywords again
                tag.init();
                tag.loadR( "object", 63, objName );
                tag.loadR( "pvdef", 255, defName );
                tag.loadR( "forceLocalPvs" );
                activeClass = unknown;
            }


            switch (activeClass) {

            //***************************************************************************************************************
            case xyGraph:

            {
                int n, count;
                tagClass tag;

                tag.init();
                tag.loadR( "beginObjectProperties" );
                tag.loadR( unknownTags );
                tag.loadR( "major", &major );
                tag.loadR( "minor", &minor );
                tag.loadR( "release", &release );

                //tag.loadR( "# Geometry" );
                tag.loadR( "x", &x );
                tag.loadR( "y", &y );
                tag.loadR( "w", &w );
                tag.loadR( "h", &h );



                //tag.loadR( "# Appearance" );
                tag.loadR( "border", &border, &zero );
                tag.loadR( "plotAreaBorder", &plotAreaBorder, &zero );
                tag.loadR( "autoScaleBothDirections", &autoScaleBothDirections, &zero );
                tag.loadR( "autoScaleUpdateMs", &autoScaleTimerMs );
                tag.loadR( "autoScaleThreshPct", &autoScaleThreshPct );
                tag.loadR( "graphTitle", &graphTitle, emptyStr );
                tag.loadR( "xLabel", &xLabel, emptyStr );
                tag.loadR( "yLabel", &yLabel, emptyStr );
                tag.loadR( "y2Label", &y2Label, emptyStr );
                tag.loadR( "fgColor", ci, &fgColor );
                tag.loadR( "bgColor", ci, &bgColor );
                tag.loadR( "gridColor", ci, &gridColor );
                tag.loadR( "font", 63, fontTag );

                //tag.loadR( "# Operating Modes" );
                tag.loadR( "plotMode", 2, plotModeEnumStr, plotModeEnum, &plotMode, &plotModePlotNPtsAndStop );
                tag.loadR( "nPts", &count );
                tag.loadR( "updateTimerMs", &updateTimerValue, &zero );
                tag.loadR( "traceCtlPv", &traceCtlPvExpStr, emptyStr );
                tag.loadR( "triggerPv", &trigPvExpStr, emptyStr );
                tag.loadR( "resetPv", &resetPvExpStr, emptyStr );
                tag.loadR( "resetMode", 2, resetModeEnumStr, resetModeEnum, &resetMode, &resetModeIfNotZero );

                //tag.loadR( "# X axis properties" );
                tag.loadR( "showXAxis", &xAxis, &zero );
                tag.loadR( "xAxisStyle", 4, xAxisStyleEnumStr, xAxisStyleEnum, &xAxisStyle, &xAxisStyleLinear );
                tag.loadR( "xAxisSrc", 3, axisSrcEnumStr, axisSrcEnum, &xAxisSource, &axisSrcFromPv );
                tag.loadR( "xMin", &xMin );
                tag.loadR( "xMax", &xMax );
                tag.loadR( "xAxisTimeFormat", 2, timeFormatEnumStr, timeFormatEnum, &xAxisTimeFormat, &timeFormatSec );
                tag.loadR( "xLabelIntervals", &xNumLabelIntervals );
                tag.loadR( "xMajorsPerLabel", &xNumMajorPerLabel );
                tag.loadR( "xMinorsPerMajor", &xNumMinorPerMajor );
                tag.loadR( "xShowLabelGrid", &xLabelGrid, &zero );
                tag.loadR( "xShowMajorGrid", &xMajorGrid, &zero );
                tag.loadR( "xShowMinorGrid", &xMinorGrid, &zero );
                tag.loadR( "xLableFormat", 2, annoFormatEnumStr, annoFormatEnum, &xAnnotationFormat, &annoFormatF );
                tag.loadR( "xLablePrecision", &xAnnotationPrecision );
                tag.loadR( "xUserSpecScaleDiv", &xGridMode, &zero );
                tag.loadR( "xAxisSmoothing", &xAxisSmoothing, &zero );

                //tag.loadR( "# Y axis properties" );
                tag.loadR( "showYAxis", &y1Axis[0], &zero );
                tag.loadR( "yAxisStyle", 2, yAxisStyleEnumStr, yAxisStyleEnum, &y1AxisStyle[0], &yAxisStyleLinear );
                tag.loadR( "yAxisSrc", 3, axisSrcEnumStr, axisSrcEnum, &y1AxisSource[0], &axisSrcFromPv );
                tag.loadR( "yMin", &y1Min[0] );
                tag.loadR( "yMax", &y1Max[0] );
                tag.loadR( "yLabelIntervals", &y1NumLabelIntervals[0] );
                tag.loadR( "yMajorsPerLabel", &y1NumMajorPerLabel[0] );
                tag.loadR( "yMinorsPerMajor", &y1NumMinorPerMajor[0] );
                tag.loadR( "yShowLabelGrid", &y1LabelGrid[0], &zero );
                tag.loadR( "yShowMajorGrid", &y1MajorGrid[0], &zero );
                tag.loadR( "yShowMinorGrid", &y1MinorGrid[0], &zero );
                tag.loadR( "yAxisFormat", 2, annoFormatEnumStr, annoFormatEnum, &y1AnnotationFormat[0], &annoFormatF );
                tag.loadR( "yAxisPrecision", &y1AnnotationPrecision[0] );
                tag.loadR( "yUserSpecScaleDiv", &y1GridMode[0], &zero );
                tag.loadR( "yAxisSmoothing", &y1AxisSmoothing[0], &zero );

                //tag.loadR( "# Y2 axis properties" );
                tag.loadR( "showY2Axis", &y1Axis[1], &zero );
                tag.loadR( "y2AxisStyle", 2, yAxisStyleEnumStr, yAxisStyleEnum, &y1AxisStyle[1], &yAxisStyleLinear );
                tag.loadR( "y2AxisSrc", 3, axisSrcEnumStr, axisSrcEnum, &y1AxisSource[1], &axisSrcFromPv );
                tag.loadR( "y2Min", &y1Min[1] );
                tag.loadR( "y2Max", &y1Max[1] );
                tag.loadR( "y2LabelIntervals", &y1NumLabelIntervals[1] );
                tag.loadR( "y2MajorsPerLabel", &y1NumMajorPerLabel[1] );
                tag.loadR( "y2MinorsPerMajor", &y1NumMinorPerMajor[1] );
                tag.loadR( "y2ShowLabelGrid", &y1LabelGrid[1], &zero );
                tag.loadR( "y2ShowMajorGrid", &y1MajorGrid[1], &zero );
                tag.loadR( "y2ShowMinorGrid", &y1MinorGrid[1], &zero );
                tag.loadR( "y2AxisFormat", 2, annoFormatEnumStr, annoFormatEnum, &y1AnnotationFormat[1], &annoFormatF );
                tag.loadR( "y2AxisPrecision", &y1AnnotationPrecision[1] );
                tag.loadR( "y2UserSpecScaleDiv", &y1GridMode[1], &zero );
                tag.loadR( "y2AxisSmoothing", &y1AxisSmoothing[1], &zero );

                // trace properties (arrays)
                //tag.loadR( "# Trace Properties" );
                tag.loadR( "numTraces", &numTraces, &zero );
                tag.loadR( "xPv", XYGC_K_MAX_TRACES, xPvExpStr, &n, emptyStr );
                tag.loadR( "yPv", XYGC_K_MAX_TRACES, yPvExpStr, &n, emptyStr );
                tag.loadR( "nPv", XYGC_K_MAX_TRACES, nPvExpStr, &n, emptyStr );
                tag.loadR( "plotStyle", 4, plotStyleEnumStr, plotStyleEnum, XYGC_K_MAX_TRACES, plotStyle, &n, &plotStyleLine );
                tag.loadR( "lineThickness", XYGC_K_MAX_TRACES, xylineThk, &n, &one );
                tag.loadR( "lineStyle", 2, styleEnumStr, styleEnum, XYGC_K_MAX_TRACES, xylineStyle, &n, &styleSolid );
                tag.loadR( "plotUpdateMode", 5, updateModeEnumStr, updateModeEnum, XYGC_K_MAX_TRACES, plotUpdateMode, &n, &updateModexAndY );
                tag.loadR( "plotSymbolType", 4, symbolEnumStr, symbolEnum, XYGC_K_MAX_TRACES, plotSymbolType, &n, &symbolNone );
                tag.loadR( "opMode", 2, opModeEnumStr, opModeEnum, XYGC_K_MAX_TRACES, opMode, &n, &opModeScope );
                tag.loadR( "useY2Axis", XYGC_K_MAX_TRACES, y2Scale, &n, &zero );
                tag.loadR( "xSigned", XYGC_K_MAX_TRACES, xSigned, &n, &zero );
                tag.loadR( "ySigned", XYGC_K_MAX_TRACES, ySigned, &n, &zero );
                tag.loadR( "plotColor", ci, XYGC_K_MAX_TRACES, plotColor, &n );

                tag.loadR( "endObjectProperties" );

                stat = tag.readTags( f, "endObjectProperties" );
                if ( !( stat & 1 ) ) {
                    printf("%s\n", tag.errMsg() );
                }

		adjustGeometry(&x, &y);

                // do we want a stripplot or xyplot ?
                bool stripPlot = false;
                for(int i=0; i<n; i++) {
                    if(opMode[i] == 1) {
                        stripPlot = true;
                    }
                }

                // ----------------- write the properties to the ui file
                if(!stripPlot) {
                    sprintf(widgetName, "caCartesianPlot_%d", widgetNumber++);
                    myParser->Qt_writeOpenTag("widget", "caCartesianPlot", widgetName);
                } else {
                    sprintf(widgetName, "caStripPlot_%d", widgetNumber++);
                    myParser->Qt_writeOpenTag("widget", "caStripPlot", widgetName);
                }
                myParser->writeRectangleDimensions(x, y, w, h);

                myParser->Qt_setColorForeground("", rgb[bgColor].r/256, rgb[bgColor].g/256, rgb[bgColor].b/256, 255);
                myParser->Qt_setColorBackground("", rgb[bgColor].r/256, rgb[bgColor].g/256, rgb[bgColor].b/256, 255); //Background color is not working
                myParser->Qt_setColorGrid("", rgb[gridColor].r/256, rgb[gridColor].g/256, rgb[gridColor].b/256, 255);
                myParser->Qt_setColorScale("", rgb[fgColor].r/256, rgb[fgColor].g/256, rgb[fgColor].b/256, 255);

                myParser-> Qt_handleString("Title", "string", graphTitle.getRaw());
                myParser-> Qt_handleString("TitleX", "string", xLabel.getRaw());
                myParser-> Qt_handleString("TitleY", "string", yLabel.getRaw());

                if(!xAxis) myParser->Qt_handleString("XaxisEnabled", "bool", "false");
                if(!y1Axis[0])  myParser->Qt_handleString("YaxisEnabled", "bool", "false");
                // add everything for stripplot
                if(stripPlot) {
                    char channels[1024] ="\0";
                    for(int i=0; i<min(n,7); i++) {
                        if(opMode[i] == 1) {
                            strcat(channels, yPvExpStr[i].getRaw());
                            strcat(channels, ";");
                        }
                        myParser->Qt_setColorTrace("", rgb[plotColor[i]].r/256, rgb[plotColor[i]].g/256, rgb[plotColor[i]].b/256, 255, i);
                    }
                    if(strlen(channels) > 0) channels[strlen(channels)-1] = '\0';
                    myParser-> Qt_handleString("channels", "string", channels);
                    myParser-> Qt_handleString("XaxisType", "enum", "ValueScale");
                    if(y1AxisStyle[0] == 1) myParser-> Qt_handleString("YaxisType", "enum", "log10");
                    myParser-> Qt_handleString("YAxisScaling", "enum", "autoScale");

                    // add everything for cartesianplot
                } else {
                    char Limits[100];
                    for(int i=0; i<min(n,7); i++) {
                        if(opMode[i] == 0) {
                            char channels[200];
                            sprintf(channels, "%s;%s", xPvExpStr[i].getRaw(), yPvExpStr[i].getRaw());
                            myParser->Qt_ChannelsXY("", channels, i);
                            if(xylineStyle[i] == 0 || xylineStyle[i] == 2) myParser->Qt_setStyleTrace("", "Lines", i);
                            else myParser->Qt_setStyleTrace("", "Dots", i);
                            if(plotSymbolType[i] == 0) myParser->Qt_setSymbolTrace("", "NoSymbol", i);
                            else if(plotSymbolType[i] == 1) myParser->Qt_setSymbolTrace("", "Ellipse", i);
                            else if(plotSymbolType[i] == 2) myParser->Qt_setSymbolTrace("", "Square", i);
                            else myParser->Qt_setSymbolTrace("", "Diamond", i);
                        }
                        myParser->Qt_setColorTrace("", rgb[plotColor[i]].r/256, rgb[plotColor[i]].g/256, rgb[plotColor[i]].b/256, 255, i);
                    }
                    if(y1AxisSource[0] == 0) {
                        myParser->Qt_handleString("YaxisScaling", "enum", "Channel");
                    } else if(y1AxisSource[0] == 1) {
                        myParser->Qt_handleString("YaxisScaling", "enum", "User");
                        sprintf(Limits,"%f;%f", y1Min[0].value(), y1Max[0].value());
                        myParser->Qt_handleString("YaxisLimits", "string", Limits);
                    } else  {
                        myParser->Qt_handleString("YaxisScaling", "enum", "Auto");
                    }

                    if(xAxisSource == 0) {
                        myParser->Qt_handleString("XaxisScaling", "enum", "Channel");
                    } else if(xAxisSource == 1) {
                        myParser->Qt_handleString("XaxisScaling", "enum", "User");
                        sprintf(Limits,"%f;%f", xMin.value(), xMax.value());
                        myParser->Qt_handleString("XaxisLimits", "string", Limits);
                    } else {
                        myParser->Qt_handleString("XaxisScaling", "enum", "Auto");
                    }

                    if(xAxisStyle == 1) myParser-> Qt_handleString("XaxisType", "enum", "log10");
                    if(y1AxisStyle[0] == 1) myParser-> Qt_handleString("YaxisType", "enum", "log10");

                }

                myParser->Qt_writeCloseTag("widget", widgetName, 0);


            }
                break;
                //***************************************************************************************************************
            case activePip :
            {
                int n;
                tagClass tag;
                tag.init();
                tag.loadR( "beginObjectProperties" );
                tag.loadR( unknownTags );
                tag.loadR( "major", &major );
                tag.loadR( "minor", &minor );
                tag.loadR( "release", &release );
                tag.loadR( "x", &x );
                tag.loadR( "y", &y );
                tag.loadR( "w", &w );
                tag.loadR( "h", &h );
                tag.loadR( "fgColor", ci, &fgColor );
                tag.loadR( "bgColor", ci, &bgColor );
                tag.loadR( "topShadowColor", ci, &topShadowColor );
                tag.loadR( "botShadowColor", ci, &botShadowColor );
                tag.loadR( "displaySource", 3, displaySourceEnumStr, displaySourceEnum, &displaySource, &displaySourceFromPV );
                tag.loadR( "filePv", &readPvExpStr, emptyStr );
                tag.loadR( "labelPv", &labelPvExpStr, emptyStr );
                tag.loadR( "file", &fileNameExpStr, emptyStr );
                tag.loadR( "center", &center, &zero );
                tag.loadR( "setSize", &setSize, &zero );
                tag.loadR( "sizeOfs", &sizeOfs, &zero );
                tag.loadR( "numDsps", &numDsps, &zero );
                tag.loadR( "displayFileName", maxDsps, displayFileName, &n, emptyStr );
                tag.loadR( "menuLabel", maxDsps, pipLabel, &n, emptyStr );
                tag.loadR( "symbols", maxDsps, symbolsExpStr, &n, emptyStr );
                tag.loadR( "replaceSymbols", maxDsps, replaceSymbols, &n, &zero );
                tag.loadR( "propagateMacros", maxDsps, propagateMacros, &n, &one );
                tag.loadR( "noScroll", &noScroll, &zero );
                tag.loadR( "ignoreMultiplexors", &ignoreMultiplexors, &zero );
                tag.loadR( "endObjectProperties" );

                stat = tag.readTags( f, "endObjectProperties" );
                if ( !( stat & 1 ) ) {
                    printf("%s\n", tag.errMsg() );
                }

		adjustGeometry(&x, &y);

                // ----------------- write the properties to the ui file
                sprintf(widgetName, "caInclude_%d", widgetNumber++);
                myParser->Qt_writeOpenTag("widget", "caInclude", widgetName);
                myParser->writeRectangleDimensions(x, y, w, h);

                if(numDsps > 0) {
                    myParser->Qt_handleString("filename", "string", displayFileName[0].getRaw());
                    myParser->Qt_handleString("macro", "string", symbolsExpStr[0].getRaw());
                }

                myParser->Qt_writeCloseTag("widget", widgetName, 0);
            }
                break;

                //***************************************************************************************************************
            case activeCircle :

            {
                tagClass tag;
                tag.init();
                tag.loadR( "beginObjectProperties" );
                tag.loadR( unknownTags );
                tag.loadR( "major", &major );
                tag.loadR( "minor", &minor );
                tag.loadR( "release", &release );
                tag.loadR( "x", &x );
                tag.loadR( "y", &y );
                tag.loadR( "w", &w );
                tag.loadR( "h", &h );
                tag.loadR( "lineColor", ci, &lineColor );
                tag.loadR( "lineAlarm", &lineColorMode, &zero );
                tag.loadR( "fill", &fill, &zero );
                tag.loadR( "fillColor", ci, &fillColor );
                tag.loadR( "fillAlarm", &fillColorMode, &zero );
                tag.loadR( "lineWidth", &lineWidth, &one );
                tag.loadR( "lineStyle", 2, styleEnumStr, styleEnum, &lineStyle, &styleSolid );
                tag.loadR( "alarmPv", &alarmPvExpStr, emptyStr );
                tag.loadR( "visPv", &visPvExpStr, emptyStr );
                tag.loadR( "visInvert", &visInverted, &zero );
                tag.loadR( "visMin", 39, minVisString, emptyStr );
                tag.loadR( "visMax", 39, maxVisString, emptyStr );
                tag.loadR( "endObjectProperties" );

                stat = tag.readTags( f, "endObjectProperties" );
                if ( !( stat & 1 ) ) {
                    printf("%s\n", tag.errMsg() );
                }

		adjustGeometry(&x, &y);

                // ----------------- write the properties to the ui file
                sprintf(widgetName, "caGraphics_%d", widgetNumber++);
                myParser->Qt_writeOpenTag("widget", "caGraphics", widgetName);
                myParser->Qt_handleString("form", "enum", "caGraphics::Circle");
                myParser->writeRectangleDimensions(x, y, w, h);
                myParser->Qt_setColorLine("", rgb[lineColor].r/256, rgb[lineColor].g/256, rgb[lineColor].b/256, 255);
                myParser->Qt_setColorForeground("", rgb[fillColor].r/256, rgb[fillColor].g/256, rgb[fillColor].b/256, 255);
                if(lineStyle == 0) {
                    myParser->Qt_handleString("linestyle", "string", "Solid");
                } else {
                    myParser->Qt_handleString("linestyle", "string", "Dash");
                }
                if(fill) myParser->Qt_handleString("fillstyle", "string", "Filled");
                else myParser->Qt_handleString("fillstyle", "string", "Outline");

                addVisibilityCalc(myParser, visPvExpStr.getRaw(), minVisString, maxVisString);

                myParser->Qt_writeCloseTag("widget", widgetName, 0);
            }
                break;

                //***************************************************************************************************************
            case activeArc :

            {
                int startAngle, totalAngle;
                char asc[64];
                tagClass tag;
                tag.init();
                tag.loadR( "beginObjectProperties" );
                tag.loadR( unknownTags );
                tag.loadR( "major", &major );
                tag.loadR( "minor", &minor );
                tag.loadR( "release", &release );
                tag.loadR( "x", &x );
                tag.loadR( "y", &y );
                tag.loadR( "w", &w );
                tag.loadR( "h", &h );
                tag.loadR( "lineColor", ci, &lineColor );
                tag.loadR( "lineAlarm", &lineColorMode, &zero );
                tag.loadR( "fill", &fill, &zero );
                tag.loadR( "fillColor", ci, &fillColor );
                tag.loadR( "fillAlarm", &fillColorMode, &zero );
                tag.loadR( "lineWidth", &lineWidth, &one );
                tag.loadR( "lineStyle", 2, styleEnumStr, styleEnum, &lineStyle, &styleSolid );
                tag.loadR( "alarmPv", &alarmPvExpStr, emptyStr );
                tag.loadR( "visPv", &visPvExpStr, emptyStr );
                tag.loadR( "visInvert", &visInverted, &zero );
                tag.loadR( "visMin", 39, minVisString, emptyStr );
                tag.loadR( "visMax", 39, maxVisString, emptyStr );
                tag.loadR( "startAngle", &efStartAngle );
                tag.loadR( "totalAngle", &efTotalAngle );
                tag.loadR( "fillMode", 2, fillModeEnumStr, fillModeEnum, &fillMode, &fillModeChord );
                tag.loadR( "endObjectProperties" );

                stat = tag.readTags( f, "endObjectProperties" );
                if ( !( stat & 1 ) ) {
                    printf("%s\n", tag.errMsg() );
                }

		adjustGeometry(&x, &y);

                // ----------------- write the properties to the ui file
                sprintf(widgetName, "caGraphics_%d", widgetNumber++);
                myParser->Qt_writeOpenTag("widget", "caGraphics", widgetName);
                myParser->Qt_handleString("form", "enum", "caGraphics::Arc");
                myParser->writeRectangleDimensions(x, y, w, h);
                myParser->Qt_setColorLine("", rgb[lineColor].r/256, rgb[lineColor].g/256, rgb[lineColor].b/256, 255);
                myParser->Qt_setColorForeground("", rgb[fillColor].r/256, rgb[fillColor].g/256, rgb[fillColor].b/256, 255);
                if(lineStyle == 0) {
                    myParser->Qt_handleString("linestyle", "string", "Solid");
                } else {
                    myParser->Qt_handleString("linestyle", "string", "Dash");
                }
                myParser->Qt_handleString("fillstyle", "string", "Filled");

                if ( efStartAngle.isNull() ) startAngle = 0;
                else startAngle = (int) ( efStartAngle.value() * 64.0 +0.5 );

                if ( efTotalAngle.isNull() ) totalAngle = 180 * 64;
                else totalAngle = (int) ( efTotalAngle.value() * 64.0 +0.5 );

                sprintf(asc, "%d", startAngle/64);
                myParser->Qt_handleString("startAngle", "number", asc);
                sprintf(asc, "%d", totalAngle/64);
                myParser->Qt_handleString("spanAngle", "number", asc);

                if(fill) myParser->Qt_handleString("fillstyle", "string", "Filled");
                else myParser->Qt_handleString("fillstyle", "string", "Outline");

                myParser->Qt_handleString("channel", "string", visPvExpStr.getRaw());

                addVisibilityCalc(myParser, visPvExpStr.getRaw(), minVisString, maxVisString);
                myParser->Qt_writeCloseTag("widget", widgetName, 0);

            }
                break;

                //***************************************************************************************************************
            case activeLine :
            {
                int i, *xArray, xSize, *yArray, ySize;
                tagClass tag;
                tag.init();
                tag.loadR( "beginObjectProperties" );
                tag.loadR( unknownTags );
                tag.loadR( "major", &major );
                tag.loadR( "minor", &minor );
                tag.loadR( "release", &release );
                tag.loadR( "x", &x );
                tag.loadR( "y", &y );
                tag.loadR( "w", &w );
                tag.loadR( "h", &h );
                tag.loadR( "lineColor", ci, &lineColor );
                tag.loadR( "lineAlarm", &lineColorMode, &zero );
                tag.loadR( "fill", &fill, &zero );
                tag.loadR( "fillColor", ci, &fillColor );
                tag.loadR( "fillAlarm", &fillColorMode, &zero );
                tag.loadR( "lineWidth", &lineWidth, &one );
                tag.loadR( "lineStyle", 2, styleEnumStr, styleEnum, &lineStyle, &styleSolid );
                tag.loadR( "alarmPv", &alarmPvExpStr, emptyStr );
                tag.loadR( "visPv", &visPvExpStr, emptyStr );
                tag.loadR( "visInvert", &visInverted, &zero );
                tag.loadR( "visMin", 39, minVisString, emptyStr );
                tag.loadR( "visMax", 39, maxVisString, emptyStr );
                tag.loadR( "closePolygon", &closePolygon, &zero );
                tag.loadR( "arrows", 4, arrowsEnumStr, arrowsEnum, &arrows, &arrowsNone );
                tag.loadR( "numPoints", &numPoints, &zero );
                tag.loadR( "xPoints", &xArray, &xSize );
                tag.loadR( "yPoints", &yArray, &ySize );
                tag.loadR( "endObjectProperties" );

                stat = tag.readTags( f, "endObjectProperties" );
                if ( !( stat & 1 ) ) {
                    printf("%s\n", tag.errMsg() );
                }

		adjustGeometry(&x, &y);

                if ( numPoints == 0 ) {
                    if ( xSize < ySize ) numPoints = xSize;
                    else numPoints = ySize;
                }

                char points[LONGSTRING];
                points[0] = '\0';
                for ( i=0; i<numPoints; i++ ) {
                    sprintf(points, "%s%d,%d;", points,  (short) (xArray[i]-x), (short) (yArray[i]-y));
                }

                // ----------------- write the properties to the ui file
                sprintf(widgetName, "caPolyLine_%d", widgetNumber++);
                myParser->Qt_writeOpenTag("widget", "caPolyLine", widgetName);
                myParser->writeRectangleDimensions(x, y, w, h);
                myParser->Qt_setColorLine("", rgb[lineColor].r/256, rgb[lineColor].g/256, rgb[lineColor].b/256, 255);

                if(strlen(points) > 0) myParser->Qt_handleString("xyPairs", "string", points);

                myParser->Qt_handleString("channel", "string", visPvExpStr.getRaw());

                addVisibilityCalc(myParser, visPvExpStr.getRaw(), minVisString, maxVisString);
                myParser->Qt_writeCloseTag("widget", widgetName, 0);

            }

                break;

                //***************************************************************************************************************
            case activeRectangle :
            {
                tagClass tag;
                tag.init();
                tag.loadR( "beginObjectProperties" );
                tag.loadR( unknownTags );
                tag.loadR( "major", &major );
                tag.loadR( "minor", &minor );
                tag.loadR( "release", &release );
                tag.loadR( "x", &x );
                tag.loadR( "y", &y );
                tag.loadR( "w", &w );
                tag.loadR( "h", &h );
                tag.loadR( "lineColor", ci, &lineColor );
                tag.loadR( "lineAlarm", &lineColorMode, &zero );
                tag.loadR( "fill", &fill, &zero );
                tag.loadR( "fillColor", ci, &fillColor );
                tag.loadR( "fillAlarm", &fillColorMode, &zero );
                tag.loadR( "lineWidth", &lineWidth, &one );
                tag.loadR( "lineStyle", 2, styleEnumStr, styleEnum, &lineStyle, &styleSolid );
                tag.loadR( "invisible", &invisible, &zero );
                tag.loadR( "alarmPv", &alarmPvExpStr, emptyStr );
                tag.loadR( "visPv", &visPvExpStr, emptyStr );
                tag.loadR( "visInvert", &visInverted, &zero );
                tag.loadR( "visMin", 39, minVisString, emptyStr );
                tag.loadR( "visMax", 39, maxVisString, emptyStr );
                tag.loadR( "endObjectProperties" );

                stat = tag.readTags( f, "endObjectProperties" );
                if ( !( stat & 1 ) ) {
                    printf("%s\n", tag.errMsg() );
                }

		adjustGeometry(&x, &y);

                // ----------------- write the properties to the ui file
                sprintf(widgetName, "caGraphics_%d", widgetNumber++);
                myParser->Qt_writeOpenTag("widget", "caGraphics", widgetName);
                myParser->Qt_handleString("form", "enum", "caGraphics::Rectangle");
                myParser->writeRectangleDimensions(x, y, w, h);
                myParser->Qt_setColorLine("", rgb[lineColor].r/256, rgb[lineColor].g/256, rgb[lineColor].b/256, 255);
                myParser->Qt_setColorForeground("", rgb[fillColor].r/256, rgb[fillColor].g/256, rgb[fillColor].b/256, 255);
                if(lineStyle == 0) {
                    myParser->Qt_handleString("linestyle", "string", "Solid");
                } else {
                    myParser->Qt_handleString("linestyle", "string", "Dash");
                }
                if(fill) myParser->Qt_handleString("fillstyle", "string", "Filled");
                else myParser->Qt_handleString("fillstyle", "string", "Outline");

                myParser->Qt_handleString("channel", "string", visPvExpStr.getRaw());


                addVisibilityCalc(myParser, visPvExpStr.getRaw(), minVisString, maxVisString);
                myParser->Qt_writeCloseTag("widget", widgetName, 0);

            }
                break;


                //***************************************************************************************************************
            case activeXText :
            {

                tagClass tag;
                tag.init();
                tag.loadR( "beginObjectProperties" );
                tag.loadR( unknownTags );
                tag.loadR( "major", &major );
                tag.loadR( "minor", &minor );
                tag.loadR( "release", &release );
                tag.loadR( "x", &x );
                tag.loadR( "y", &y );
                tag.loadR( "w", &w );
                tag.loadR( "h", &h );
                tag.loadR( "font", 63, fontTag );
                tag.loadR( "fontAlign", 3, alignEnumStr, alignEnum, &alignment, &left );
                tag.loadR( "fgColor", ci, &fgColor);
                tag.loadR( "fgAlarm", &fgColorMode, &zero);
                tag.loadR( "bgColor", ci, &bgColor);
                tag.loadR( "bgAlarm", &bgColorMode, &zero);
                tag.loadR( "useDisplayBg", &useDisplayBg, &zero);
                tag.loadR( "alarmPv", &alarmPvExpStr, emptyStr);
                tag.loadR( "visPv", &visPvExpStr, emptyStr);
                tag.loadR( "visInvert", &visInverted, &zero);
                tag.loadR( "visMin", 39, minVisString, emptyStr );
                tag.loadR( "visMax", 39, maxVisString, emptyStr );
                tag.loadR( "value", &value, emptyStr);
                tag.loadR( "autoSize", &autoSize, &zero );
                tag.loadR( "border", &border, &zero );
                tag.loadR( "lineWidth", &lineThk, &one );

                tag.loadR( "endObjectProperties" );

                stat = tag.readTags( f, "endObjectProperties" );
                if ( !( stat & 1 ) ) {
                    printf("%s\n", tag.errMsg() );
                }

		adjustGeometry(&x, &y);

                // ----------------- write the properties to the ui file
                sprintf(widgetName, "caLabel_%d", widgetNumber++);
                myParser->Qt_writeOpenTag("widget", "caLabel", widgetName);
                myParser->writeRectangleDimensions(x, y, w, h);
                myParser->Qt_handleString("text", "string",  value.getRaw());
                myParser->Qt_handleString("channel", "string", pvExpStr.getRaw());

                setFont(myParser, fontTag); // Zai added
// Zai                myParser->Qt_handleString("fontScaleMode", "enum",  "WidthAndHeight");
                if(alignment==0) {
                    myParser->Qt_handleString("alignment", "set", "Qt::AlignAbsolute|Qt::AlignLeft|Qt::AlignVCenter");
                } else if(alignment==1) {
                    myParser-> Qt_handleString("alignment", "set", "Qt::AlignAbsolute|Qt::AlignHCenter|Qt::AlignVCenter");
                } else if(alignment==2) {
                    myParser->Qt_handleString("alignment", "set", "Qt::AlignAbsolute|Qt::AlignRight|Qt::AlignVCenter");
                }
                if(useDisplayBg) alpha = 0; else alpha=255;
                myParser->Qt_setColorForeground("", rgb[fgColor].r/256, rgb[fgColor].g/256, rgb[fgColor].b/256, 255);
                myParser->Qt_setColorBackground("", rgb[bgColor].r/256, rgb[bgColor].g/256, rgb[bgColor].b/256, alpha);

                addVisibilityCalc(myParser, visPvExpStr.getRaw(), minVisString, maxVisString);
                myParser->Qt_writeCloseTag("widget", widgetName, 0);
            }
                break;

                //***************************************************************************************************************
            case Byte :
                // Zai added
            {
                tagClass tag;

                tag.init();
                tag.loadR( "beginObjectProperties" );
                tag.loadR( unknownTags );
                tag.loadR( "major", &major );
                tag.loadR( "minor", &minor );
                tag.loadR( "release", &release );
                tag.loadR( "x", &x );
                tag.loadR( "y", &y );
                tag.loadR( "w", &w );
                tag.loadR( "h", &h );
                tag.loadR( "lineColor", ci, &lineColor );
                tag.loadR( "onColor", ci, &onColor );
                tag.loadR( "offColor", ci, &offColor );
                tag.loadR( "controlPv", &controlPvExpStr, emptyStr );
                tag.loadR( "numBits", &numBits );
                tag.loadR( "endObjectProperties" );

                stat = tag.readTags( f, "endObjectProperties" );
                if ( !( stat & 1 ) ) {
                    printf("%s\n", tag.errMsg() );
                }

		adjustGeometry(&x, &y);

		if(numBits == 0) //Needed because EDM does not write on edl file if numBits = 16
			numBits = 16;

                // ----------------- write the properties to the ui file
                sprintf(widgetName, "caByte_%d", widgetNumber++);
                myParser->Qt_writeOpenTag("widget", "caByte", widgetName);
                myParser->writeRectangleDimensions(x, y, w, h);
                myParser->Qt_handleString("channel", "string", controlPvExpStr.getRaw());
                myParser->Qt_handleString("startBit", "string", "0");
                myParser->Qt_handleString("endBit", "string", (char*)QString::number(numBits - 1).toStdString().c_str());
                if (w > h){
                    myParser->Qt_handleString("direction", "enum", "Right");
                }
                else{
                    myParser->Qt_handleString("direction", "enum", "Down");
                }
                myParser->Qt_writeCloseTag("widget", widgetName, 0);
            }
                break;
            case activeXTextDsp :
            case TextEntry :    // Zai
            case TextUpdate :   // Zai

            {
                tagClass tag;
                tag.init();
                tag.loadR( "beginObjectProperties" );
                tag.loadR( unknownTags );
                tag.loadR( "major", &major );
                tag.loadR( "minor", &minor );
                tag.loadR( "release", &release );
                tag.loadR( "x", &x );
                tag.loadR( "y", &y );
                tag.loadR( "w", &w );
                tag.loadR( "h", &h );
                tag.loadR( "controlPv", &pvExpStr, emptyStr );
                tag.loadR( "format", 7, formatTypeEnumStr, formatTypeEnum, &formatType, &formatTypeDefault );
                tag.loadR( "font", 63, fontTag );
                tag.loadR( "fontAlign", 3, alignEnumStr, alignEnum, &alignment, &left );
                tag.loadR( "fgColor", ci, &fgColor );
                tag.loadR( "fgAlarm", &colorMode, &zero );
                tag.loadR( "bgColor", ci, &bgColor );
                tag.loadR( "bgAlarm", &bgColorMode, &zero );
                tag.loadR( "fill", &fill, &zero );  // Zai - avoid warnings
                tag.loadR( "useDisplayBg", &useDisplayBg, &zero );
                tag.loadR( "editable", &editable, &zero );
                tag.loadR( "autoHeight", &autoHeight, &zero );
                tag.loadR( "motifWidget", &isWidget, &zero );
                tag.loadR( "limitsFromDb", &limitsFromDb, &zero );
                tag.loadR( "precision", &efPrecision );
                tag.loadR( "fieldLen", 7, fieldLenInfo, emptyStr );
                tag.loadR( "nullPv", &svalPvExpStr, emptyStr );
                tag.loadR( "nullColor", ci, &index );
                tag.loadR( "nullCondition", 3, nullCondEnumStr, nullCondEnum, &nullDetectMode, &nullCondNullEqCtl );
                tag.loadR( "colorPv", &fgPvExpStr, emptyStr );
                tag.loadR( "smartRefresh", &smartRefresh, &zero );
                tag.loadR( "useKp", &useKp, &zero );
                tag.loadR( "changeValOnLoseFocus", &changeValOnLoseFocus, &zero );
                tag.loadR( "fastUpdate", &fastUpdate, &zero );
                tag.loadR( "date", &isDate, &zero );
                tag.loadR( "file", &isFile, &zero );
                tag.loadR( "defDir", &defDir, emptyStr );
                tag.loadR( "pattern", &pattern, emptyStr );
                tag.loadR( "autoSelect", &autoSelect, &zero );
                tag.loadR( "updatePvOnDrop", &updatePvOnDrop, &zero );
                tag.loadR( "useHexPrefix", &useHexPrefix, &zero );
                tag.loadR( "fileComponent", 3, fileCompEnumStr, fileCompEnum, &fileComponent, &fileCompFullPath );
                tag.loadR( "dateAsFileName", &dateAsFileName, &zero );
                tag.loadR( "showUnits", &showUnits, &zero );
                tag.loadR( "useAlarmBorder", &useAlarmBorder, &zero );
                tag.loadR( "newPos", &newPositioning, &zero );
                tag.loadR( "inputFocusUpdates", &inputFocusUpdatesAllowed, &zero );
                tag.loadR( "objType", 4, objTypeEnumStr, objTypeEnum, &objType, &objTypeUnknown );
                tag.loadR( "clipToDspLimits", &clipToDspLimits, &zero );
                tag.loadR( "id", 31, id, emptyStr );
                tag.loadR( "changeCallback", &changeCallbackFlag, &zero );
                tag.loadR( "isPassword", &isPassword, &zero );
                tag.loadR( "characterMode", &characterMode, &zero );
                tag.loadR( "noExecuteClipMask", &noExecuteClipMask, &zero );
                tag.loadR( "endObjectProperties" );

                stat = tag.readTags( f, "endObjectProperties" );
                if ( !( stat & 1 ) ) {
                    printf("%s\n", tag.errMsg() );
                }

		adjustGeometry(&x, &y);

                // ----------------- write the properties to the ui file
                if(activeClass == TextUpdate || (activeClass != TextEntry && /* Zai added */!editable)) {
                    sprintf(widgetName, "caLineEdit_%d", widgetNumber++);
                    myParser->Qt_writeOpenTag("widget", "caLineEdit", widgetName);
                } else {
                    sprintf(widgetName, "caTextEntry_%d", widgetNumber++);
                    myParser->Qt_writeOpenTag("widget", "caTextEntry", widgetName);
                }
                myParser->writeRectangleDimensions(x, y, w, h);
                myParser->Qt_handleString("channel", "string", pvExpStr.getRaw());

                setFont(myParser, fontTag); // Zai added
// Zai                myParser->Qt_handleString("fontScaleMode", "enum",  "WidthAndHeight");
                if(alignment==0) {
                    myParser->Qt_handleString("alignment", "set", "Qt::AlignAbsolute|Qt::AlignLeft|Qt::AlignVCenter");
                } else if(alignment==1) {
                    myParser-> Qt_handleString("alignment", "set", "Qt::AlignAbsolute|Qt::AlignHCenter|Qt::AlignVCenter");
                } else if(alignment==2) {
                    myParser->Qt_handleString("alignment", "set", "Qt::AlignAbsolute|Qt::AlignRight|Qt::AlignVCenter");
                }
                myParser->Qt_setColorForeground("", rgb[fgColor].r/256, rgb[fgColor].g/256, rgb[fgColor].b/256, 255);
                myParser->Qt_setColorBackground("", rgb[bgColor].r/256, rgb[bgColor].g/256, rgb[bgColor].b/256, 255);
                if(showUnits) myParser->Qt_handleString("unitsEnabled", "bool", "false");

                myParser->Qt_writeCloseTag("widget", widgetName, 0);

            }
                break;

                //***************************************************************************************************************
            case activeMeter :

            {

                tagClass tag;
                tag.init();
                tag.loadR( "beginObjectProperties" );
                tag.loadR( unknownTags );
                tag.loadR( "major", &major );
                tag.loadR( "minor", &minor );
                tag.loadR( "release", &release );
                tag.loadR( "x", &x );
                tag.loadR( "y", &y );
                tag.loadR( "w", &w );
                tag.loadR( "h", &h );
                tag.loadR( "caseColor", ci, &meterColor );
                tag.loadR( "caseAlarm", &meterColorMode, &zero );
                tag.loadR( "scaleColor", ci, &scaleColor );
                tag.loadR( "scaleAlarm", &scaleColorMode, &zero );
                tag.loadR( "labelColor", ci, &labelColor );
                tag.loadR( "fgColor", ci, &fgColor );
                tag.loadR( "fgAlarm", &fgColorMode, &zero );
                tag.loadR( "bgColor", ci, &bgColor );
                tag.loadR( "topShadowColor", ci, &tsColor );
                tag.loadR( "botShadowColor", ci, &bsColor );
                tag.loadR( "readPv", &readPvExpStr, emptyStr );
                tag.loadR( "label", 39, literalLabel, emptyStr );
                tag.loadR( "labelType", 3, labelTypeEnumStr, labelTypeEnum, &labelType, &lit );
                tag.loadR( "trackDelta", &trackDelta, &zero );
                tag.loadR( "showScale", &showScale, &zero );
                tag.loadR( "scaleFormat", 15, scaleFormat );
                tag.loadR( "scalePrecision", &scalePrecExpStr );
                tag.loadR( "scaleLimitsFromDb", &scaleLimitsFromDb, &zero );
                tag.loadR( "useDisplayBg", &useDisplayBg, &zero );
                tag.loadR( "labelIntervals", &labIntExpStr );
                tag.loadR( "majorIntervals", &majorIntervals, &zero );
                tag.loadR( "minorIntervals", &minorIntervals, &zero );
                tag.loadR( "complexNeedle", &needleType, &zero );
                tag.loadR( "3d", &shadowMode, &zero );
                tag.loadR( "scaleMin", &scaleMinExpStr );
                tag.loadR( "scaleMax", &scaleMaxExpStr );
                tag.loadR( "labelFontTag", 63, labelFontTag );
                tag.loadR( "scaleFontTag", 63, scaleFontTag );
                tag.loadR( "meterAngle", &meterAngle, &a180 );
                tag.loadR( "endObjectProperties" );

                stat = tag.readTags( f, "endObjectProperties" );
                if ( !( stat & 1 ) ) {
                    printf("%s\n", tag.errMsg() );
                }

		adjustGeometry(&x, &y);

                // ----------------- write the properties to the ui file
                sprintf(widgetName, "caCircularGauge_%d", widgetNumber++);
                myParser->Qt_writeOpenTag("widget", "caCircularGauge", widgetName);
                myParser->writeRectangleDimensions(x, y, w, h);
                myParser->Qt_handleString("channel", "string", readPvExpStr.getRaw());
                if(scaleLimitsFromDb) {
                    myParser->Qt_handleString("displayLimits", "enum", "Channel_Limits");
                } else {
                    myParser->Qt_handleString("displayLimits", "enum", "User_Limits");
                    myParser->Qt_handleString("minValue", "double", scaleMinExpStr.getRaw());
                    myParser->Qt_handleString("maxValue", "double", scaleMaxExpStr.getRaw());
                }
                if(!meterColorMode) {
                    myParser->Qt_handleString("alarmLimits", "enum", "Channel_Alarms");
                } else {
                    myParser->Qt_handleString("alarmLimits", "enum", "User_Alarms");
                    myParser->Qt_handleString("lowError", "double", scaleMinExpStr.getRaw());
                    myParser->Qt_handleString("highError", "double", scaleMaxExpStr.getRaw());
                    myParser->Qt_handleString("lowWarning", "double", scaleMinExpStr.getRaw());
                    myParser->Qt_handleString("highWarning", "double", scaleMaxExpStr.getRaw());
                }
                char precFormat[80];
                strcpy(precFormat, "%.0f");
                if( strlen(scalePrecExpStr.getRaw()) < 1) strcpy(precFormat, "%.0f");
                else if(strlen(scalePrecExpStr.getRaw()) == 1) sprintf(precFormat, "%%.%sf", scalePrecExpStr.getRaw());


                myParser->Qt_handleString("valueFormat", "string",  precFormat);

                myParser->Qt_writeCloseTag("widget", widgetName, 0);
            }
                break;

                //***************************************************************************************************************
            case activeBar :


            {
                tagClass tag;
                tag.init();
                tag.loadR( "beginObjectProperties" );
                tag.loadR( unknownTags );
                tag.loadR( "major", &major );
                tag.loadR( "minor", &minor );
                tag.loadR( "release", &release );
                tag.loadR( "x", &x );
                tag.loadR( "y", &y );
                tag.loadR( "w", &w );
                tag.loadR( "h", &h );
                tag.loadR( "indicatorColor", ci, &barColor );
                tag.loadR( "indicatorAlarm", &barColorMode, &zero );
                tag.loadR( "fgColor", ci, &fgColor );
                tag.loadR( "fgAlarm", &fgColorMode, &zero );
                tag.loadR( "bgColor", ci, &bgColor );
                tag.loadR( "indicatorPv", &readPvExpStr, emptyStr );
                tag.loadR( "nullPv", &nullPvExpStr, emptyStr );
                tag.loadR( "label", &label, emptyStr );
                tag.loadR( "labelType", 2, labelTypeEnumStr, labelTypeEnum,&labelType, &lit );
                tag.loadR( "showScale", &showScale, &zero );
                tag.loadR( "origin", &barOriginValExpStr, emptyStr );
                tag.loadR( "font", 63, fontTag );
                tag.loadR( "labelTicks", &labelTicksExpStr, emptyStr );
                tag.loadR( "majorTicks", &majorTicksExpStr, emptyStr );
                tag.loadR( "minorTicks", &minorTicksExpStr, emptyStr );
                tag.loadR( "border", &border, &zero );
                tag.loadR( "limitsFromDb", &limitsFromDb, &zero );
                tag.loadR( "precision", &precisionExpStr, emptyStr );
                tag.loadR( "min", &readMinExpStr, emptyStr );
                tag.loadR( "max", &readMaxExpStr, emptyStr );
                tag.loadR( "scaleFormat", 15, scaleFormat );
                tag.loadR( "orientation", 2, orienTypeEnumStr, orienTypeEnum, &horizontal, &horz );
                tag.loadR( "endObjectProperties" );

                stat = tag.readTags( f, "endObjectProperties" );
                if ( !( stat & 1 ) ) {
                    printf("%s\n", tag.errMsg() );
                }

		adjustGeometry(&x, &y);

                // ----------------- write the properties to the ui file
                sprintf(widgetName, "caThermoM_%d", widgetNumber++);
                myParser->Qt_writeOpenTag("widget", "caThermo", widgetName);
                myParser->writeRectangleDimensions(x, y, w, h);
                myParser->Qt_handleString("channel", "string", readPvExpStr.getRaw());
                myParser->Qt_setColorForeground("", rgb[barColor].r/256, rgb[barColor].g/256, rgb[barColor].b/256, 255);
                myParser->Qt_setColorBackground("", rgb[bgColor].r/256, rgb[bgColor].g/256, rgb[bgColor].b/256, 255);
                if(horizontal == 0) {
                    myParser->Qt_handleString("direction", "enum", "Up");
                } else {
                    myParser->Qt_handleString("direction", "enum", "Right");
                }
                if(showScale) {
                    if(horizontal == 0) {
                        myParser->Qt_handleString("scalePosition", "enum", "LeftScale");
                    } else {
                        myParser->Qt_handleString("scalePosition", "enum", "BottomScale");
                    }
                    myParser->Qt_handleString("look", "enum", "Outline");
                }
                if(fgColorMode) {
                    myParser->Qt_handleString("colorMode", "enum", "Alarm");
                }

                myParser->Qt_writeCloseTag("widget", widgetName, 0);
            }

                break;

                //***************************************************************************************************************
            case activeMessageBox :
                break;

                //***************************************************************************************************************
            case activeMotifSlider :
            {
                char asc[10];
                tagClass tag;
                tag.init();
                tag.loadR( "beginObjectProperties" );
                tag.loadR( unknownTags );
                tag.loadR( "major", &major );
                tag.loadR( "minor", &minor );
                tag.loadR( "release", &release );
                tag.loadR( "x", &x );
                tag.loadR( "y", &y );
                tag.loadR( "w", &w );
                tag.loadR( "h", &h );
                tag.loadR( "fgColor", ci, &fgColor );
                tag.loadR( "bgColor", ci, &bgColor );
                tag.loadR( "bgAlarm", &bgColorMode, &zero );
                tag.loadR( "2ndBgColor", ci, &shadeColor );
                tag.loadR( "topShadowColor", ci, &topColor );
                tag.loadR( "botShadowColor", ci, &botColor );
                tag.loadR( "increment", &dincrement, &dzero );
                tag.loadR( "controlPv", &controlPvName, emptyStr );
                tag.loadR( "controlLabel", &controlLabelName, emptyStr );
                tag.loadR( "controlLabelType", 3, labelEnumStr, labelEnum,&controlLabelType, &labelTypeLiteral );
                tag.loadR( "font", 63, fontTag );
                tag.loadR( "displayFormat", 3, formatTypeEnumStrM, formatTypeEnumM, &formatType, &formatTypeFfloat );
                tag.loadR( "limitsFromDb", &limitsFromDb, &zero );
                tag.loadR( "precision", &efPrecision );
                tag.loadR( "scaleMin", &efScaleMin );
                tag.loadR( "scaleMax", &efScaleMax );
                tag.loadR( "showLimits", &showLimits, &zero );
                tag.loadR( "showLabel", &showLabel, &zero );
                tag.loadR( "showValue", &showValue, &zero );
                tag.loadR( "orientation", 2, orienTypeEnumStr, orienTypeEnum, &orientation,&horz );
                tag.loadR( "savedValuePv", &savedValuePvName, emptyStr );
                tag.loadR( "showSavedValue", &showSavedValue, &zero );
                tag.loadR( "endObjectProperties" );

                stat = tag.readTags( f, "endObjectProperties" );
                if ( !( stat & 1 ) ) {
                    printf("%s\n", tag.errMsg() );
                }

		adjustGeometry(&x, &y);

                // ----------------- write the properties to the ui file
                sprintf(widgetName, "caSlider_%d", widgetNumber++);
                myParser->Qt_writeOpenTag("widget", "caSlider", widgetName);
                myParser->writeRectangleDimensions(x, y, w, h);
                myParser->Qt_handleString("channel", "string", readPvExpStr.getRaw());
                if(horizontal == 0) {
                    myParser->Qt_handleString("direction", "enum", "Right");
                } else {
                    myParser->Qt_handleString("direction", "enum", "Up");
                }
                if(showScale) {
                    if(horizontal == 0) {
                        myParser->Qt_handleString("scalePosition", "enum", "BottomScale");
                    } else {
                        myParser->Qt_handleString("scalePosition", "enum", "LeftScale");
                    }
                }
                if(fgColorMode) {
                    myParser->Qt_handleString("colorMode", "enum", "Alarm");
                } else {
                    myParser->Qt_handleString("colorMode", "enum", "Default");
                }
                if(dincrement <= 0.0000000001) dincrement = 0.1;
                sprintf(asc, "%f\n", dincrement);
                myParser->Qt_handleString("incrementValue", "double", asc);

                myParser->Qt_writeCloseTag("widget", widgetName, 0);
            }
                break;

                //***************************************************************************************************************
            case shellCmdButton :
            {
                int n;
                char args[1024], files[1024], labels[1024];
                args[0] = files[0] = labels[0] = '\0';
                tagClass tag;

                tag.init();
                tag.loadR( "beginObjectProperties" );
                tag.loadR( unknownTags );
                tag.loadR( "major", &major );
                tag.loadR( "minor", &minor );
                tag.loadR( "release", &release );
                tag.loadR( "x", &x );
                tag.loadR( "y", &y );
                tag.loadR( "w", &w );
                tag.loadR( "h", &h );
                tag.loadR( "fgColor", ci, &fgColor );
                tag.loadR( "bgColor", ci, &bgColor );
                tag.loadR( "topShadowColor", ci, &topShadowColor );
                tag.loadR( "botShadowColor", ci, &botShadowColor );
                tag.loadR( "font", 63, fontTag );
                tag.loadR( "invisible", &invisible, &zero );
                tag.loadR( "closeDisplay", &closeAction, &zero );
                tag.loadR( "buttonLabel", &buttonLabel, emptyStr );
                tag.loadR( "autoExecPeriod", &autoExecInterval, &dzero );
                tag.loadR( "initialDelay", &threadSecondsToDelay, &dzero );
                tag.loadR( "password", 31, pw, emptyStr );
                tag.loadR( "lock", &lock, &zero );
                tag.loadR( "oneShot", &oneShot, &zero );
                tag.loadR( "swapButtons", &swapButtons, &zero );
                tag.loadR( "multipleInstances", &multipleInstancesAllowed, &zero );
                tag.loadR( "requiredHostName", 15, requiredHostName, emptyStr );
                tag.loadR( "numCmds", &numCmds, &zero );
                tag.loadR( "commandLabel", maxCmds, labelS, &n, emptyStr );
                tag.loadR( "command", maxCmds, shellCommand, &n, emptyStr );
                tag.loadR( "includeHelpIcon", &includeHelpIcon, &zero );
                tag.loadR( "execCursor", &execCursor, &zero );
                tag.loadR( "endObjectProperties" );

                stat = tag.readTags( f, "endObjectProperties" );
                if ( !( stat & 1 ) ) {
                    printf("%s\n", tag.errMsg() );
                }

		adjustGeometry(&x, &y);

                // ----------------- write the properties to the ui file
                sprintf(widgetName, "caShellCommand_%d", widgetNumber++);
                myParser->Qt_writeOpenTag("widget", "caShellCommand", widgetName);
                myParser->writeRectangleDimensions(x, y, w, h);
                myParser->Qt_setColorForeground("", rgb[fgColor].r/256, rgb[fgColor].g/256, rgb[fgColor].b/256, 255);
                myParser->Qt_setColorBackground("", rgb[bgColor].r/256, rgb[bgColor].g/256, rgb[bgColor].b/256, 255);

                for(int i=0; i<numCmds; i++) {
                    strcat(labels, labelS[i].getRaw());
                    strcat(labels, ";");
                    strcat(files, shellCommand[i].getRaw());
                    strcat(files, ";");
                }
                if(strlen(files) > 0) files[strlen(files) -1] = '\0';
                if(strlen(labels) > 0) labels[strlen(labels) -1] = '\0';
                myParser->Qt_handleString("files", "string", files);
                myParser->Qt_handleString("labels", "string", labels);
                myParser->Qt_handleString("args", "string", args);
                myParser->Qt_handleString("label", "string", buttonLabel.getRaw());

                myParser->Qt_writeCloseTag("widget", widgetName, 0);

            }
                break;


                //***************************************************************************************************************
            case activeChoiceButton: // Zai added
            case activeRadioButton:
            {
                tagClass tag;

                tag.init();
                tag.loadR( "beginObjectProperties" );
                tag.loadR( unknownTags );
                tag.loadR( "major", &major );
                tag.loadR( "minor", &minor );
                tag.loadR( "release", &release );
                tag.loadR( "x", &x );
                tag.loadR( "y", &y );
                tag.loadR( "w", &w );
                tag.loadR( "h", &h );
                tag.loadR( "fgColor", ci, &fgColor );
                tag.loadR( "fgAlarm", &fgColorMode, &zero );
                tag.loadR( "bgColor", ci, &bgColor );
                tag.loadR( "bgAlarm", &bgColorMode, &zero );
                tag.loadR( "buttonColor", ci, &buttonColor );
                tag.loadR( "selectColor", ci, &selectColor );
                tag.loadR( "topShadowColor", ci, &topShadowColor );
                tag.loadR( "botShadowColor", ci, &botShadowColor );
                tag.loadR( "controlPv", &controlPvExpStr, emptyStr );
                tag.loadR( "font", 63, fontTag );
                tag.loadR( "inconsistentColor", ci, &inconsistentColor );  // Zai added for ChoiceButton
                tag.loadR( "orientation", 2, orienTypeEnumStr, orienTypeEnum, &horizontal, &horz ); // Zai added for ChoiceButton
                tag.loadR( "endObjectProperties" );

                stat = tag.readTags( f, "endObjectProperties" );
                if ( !( stat & 1 ) ) {
                    printf("%s\n", tag.errMsg() );
                }

		adjustGeometry(&x, &y);

                // ----------------- write the properties to the ui file
                sprintf(widgetName, "caChoice_%d", widgetNumber++);
                myParser->Qt_writeOpenTag("widget", "caChoice", widgetName);
                myParser->writeRectangleDimensions(x, y, w, h);
                myParser->Qt_setColorForeground("", rgb[fgColor].r/256, rgb[fgColor].g/256, rgb[fgColor].b/256, 255);
                myParser->Qt_setColorBackground("", rgb[bgColor].r/256, rgb[bgColor].g/256, rgb[bgColor].b/256, 255);
                myParser->Qt_handleString("channel", "string", controlPvExpStr.getRaw());
                setFont(myParser, fontTag); // Zai added
                // Zai added
                if (activeClass == activeChoiceButton){
                    if(horizontal == 0) {
                        myParser->Qt_handleString("stackingMode", "enum", "Row");
                    } else {
                        myParser->Qt_handleString("stackingMode", "enum", "Column");
                    }
                    myParser->Qt_handleString("startBit", "number", "0");
                    myParser->Qt_handleString("endBit", "number", "2");
                } // Zai added end
                myParser->Qt_writeCloseTag("widget", widgetName, 0);
            }
                break;

                //***************************************************************************************************************
            case activeMenuButton :

            {
                tagClass tag;
                tag.init();
                tag.loadR( "beginObjectProperties" );
                tag.loadR( unknownTags );
                tag.loadR( "major", &major );
                tag.loadR( "minor", &minor );
                tag.loadR( "release", &release );
                tag.loadR( "x", &x );
                tag.loadR( "y", &y );
                tag.loadR( "w", &w );
                tag.loadR( "h", &h );
                tag.loadR( "fgColor", ci, &fgColor );
                tag.loadR( "fgAlarm", &fgColorMode, &zero );
                tag.loadR( "bgColor", ci, &bgColor );
                tag.loadR( "bgAlarm", &bgColorMode, &zero );
                tag.loadR( "inconsistentColor", ci, &inconsistentColor );
                tag.loadR( "topShadowColor", ci, &topShadowColor );
                tag.loadR( "botShadowColor", ci, &botShadowColor );
                tag.loadR( "controlPv", &controlPvExpStr, emptyStr );
                tag.loadR( "indicatorPv", &readPvExpStr, emptyStr );
                tag.loadR( "font", 63, fontTag );
                tag.loadR( "visPv", &visPvExpStr, emptyStr );
                tag.loadR( "visInvert", &visInverted, &zero );
                tag.loadR( "visMin", 39, minVisString, emptyStr );
                tag.loadR( "visMax", 39, maxVisString, emptyStr );
                tag.loadR( "colorPv", &colorPvExpStr, emptyStr );
                tag.loadR( "endObjectProperties" );

                stat = tag.readTags( f, "endObjectProperties" );
                if ( !( stat & 1 ) ) {
                    printf("%s\n", tag.errMsg() );
                }

		adjustGeometry(&x, &y);

                // ----------------- write the properties to the ui file
                sprintf(widgetName, "caMenu_%d", widgetNumber++);
                myParser->Qt_writeOpenTag("widget", "caMenu", widgetName);
                myParser->writeRectangleDimensions(x, y, w, h);
                setFont(myParser, fontTag); // Zai added
                myParser->Qt_setColorForeground("", rgb[fgColor].r/256, rgb[fgColor].g/256, rgb[fgColor].b/256, 255);
                myParser->Qt_setColorBackground("", rgb[bgColor].r/256, rgb[bgColor].g/256, rgb[bgColor].b/256, 255);
                myParser->Qt_handleString("channel", "string", controlPvExpStr.getRaw());
                myParser->Qt_handleString("colorMode", "string", "Static");

                addVisibilityCalc(myParser, visPvExpStr.getRaw(), minVisString, maxVisString);
                myParser->Qt_writeCloseTag("widget", widgetName, 0);
            }


                break;

                //***************************************************************************************************************
            case activeMessageButton :
            {
                char label[80];
                tagClass tag;
                tag.init();
                tag.loadR( "beginObjectProperties" );
                tag.loadR( unknownTags );
                tag.loadR( "major", &major );
                tag.loadR( "minor", &minor );
                tag.loadR( "release", &release );
                tag.loadR( "x", &x );
                tag.loadR( "y", &y );
                tag.loadR( "w", &w );
                tag.loadR( "h", &h );
                tag.loadR( "fgColor", ci, &fgColor );
                tag.loadR( "onColor", ci, &onColor );
                tag.loadR( "offColor", ci, &offColor );
                tag.loadR( "topShadowColor", ci, &topShadowColor );
                tag.loadR( "botShadowColor", ci, &botShadowColor );
                tag.loadR( "controlPv", &destPvExpStringM, emptyStr );
                tag.loadR( "pressValue",  &sourcePressPvExpString, emptyStr );
                tag.loadR( "releaseValue",  &sourceReleasePvExpString, emptyStr );
                tag.loadR( "onLabel", &onLabel, emptyStr );
                tag.loadR( "offLabel", &offLabel, emptyStr );
                tag.loadR( "toggle", &toggle, &zero );
                tag.loadR( "closeOnPress", &pressAction, &zero );
                tag.loadR( "closeOnRelease", &releaseAction, &zero );
                tag.loadR( "3d", &_3D, &zero );
                tag.loadR( "invisible", &invisible, &zero );
                tag.loadR( "useEnumNumeric", &useEnumNumeric, &zero );
                tag.loadR( "password", 31, pw, emptyStr );
                tag.loadR( "lock", &lock, &zero );
                tag.loadR( "font", 63, fontTag );
                tag.loadR( "visPv", &visPvExpString, emptyStr );
                tag.loadR( "visInvert", &visInverted, &zero );
                tag.loadR( "visMin", 39, minVisString, emptyStr );
                tag.loadR( "visMax", 39, maxVisString, emptyStr );
                tag.loadR( "colorPv", &colorPvExpString, emptyStr );
                tag.loadR( "endObjectProperties" );

                stat = tag.readTags( f, "endObjectProperties" );
                if ( !( stat & 1 ) ) {
                    printf("%s\n", tag.errMsg() );
                }

		adjustGeometry(&x, &y);

                // ----------------- write the properties to the ui file
                sprintf(widgetName, "caMessageButton_%d", widgetNumber++);
                myParser->Qt_writeOpenTag("widget", "caMessageButton", widgetName);
                myParser->writeRectangleDimensions(x, y, w, h);
                setFont(myParser, fontTag); // Zai added
                myParser->Qt_setColorForeground("", rgb[fgColor].r/256, rgb[fgColor].g/256, rgb[fgColor].b/256, 255);
                myParser->Qt_setColorBackground("", rgb[onColor].r/256, rgb[onColor].g/256, rgb[onColor].b/256, 255);
                myParser->Qt_handleString("channel", "string", destPvExpStringM.getRaw());
                myParser->Qt_handleString("pressMessage", "string", sourcePressPvExpString.getRaw());
                myParser->Qt_handleString("releaseMessage", "string", sourceReleasePvExpString.getRaw());
                sprintf(label, "%s / %s", onLabel.getRaw(), offLabel.getRaw());
                myParser->Qt_handleString("label", "string", label);

                addVisibilityCalc(myParser, visPvExpStr.getRaw(), minVisString, maxVisString);
                myParser->Qt_writeCloseTag("widget", widgetName, 0);

            }
                break;

                //***************************************************************************************************************
            case activeExitButton :
                break;

                //***************************************************************************************************************
            case activeGroup:
            {
		tagClass tag;
                tag.init();
                tag.loadR( "beginObjectProperties" );
                tag.loadR( unknownTags );
                tag.loadR( "major", &major );
                tag.loadR( "minor", &minor );
                tag.loadR( "release", &release );
                tag.loadR( "x", &x );
                tag.loadR( "y", &y );
                tag.loadR( "w", &w );
                tag.loadR( "h", &h );

		tag.loadR( "beginGroup");

                stat = tag.readTags( f, "beginGroup" );
                if ( !( stat & 1 ) ) {
                    printf("%s\n", tag.errMsg() );
                }

		adjustGeometry(&x, &y);

		group g;
		g.major = major;
		g.minor = minor;
		g.release = release;
		g.x = x;
		g.y = y;
		g.w = w;
		g.h = h;

                // ----------------- write the properties to the ui file
                sprintf(g.name, "caFrame_%d", widgetNumber++);

                myParser->Qt_writeOpenTag("widget", "caFrame", g.name);

		groups.push_back(g);
		openGroups ++;

                myParser->writeRectangleDimensions(x, y, w, h);
            }
		break;
            case relatedDisplay :
            {

                char args[1024], files[1024], labels[1024], remove[1024];
                args[0] = files[0] = labels[0] = remove[0] = '\0';
                tagClass tag;
                tag.init();
                tag.loadR( "beginObjectProperties" );
                tag.loadR( unknownTags );
                tag.loadR( "major", &major );
                tag.loadR( "minor", &minor );
                tag.loadR( "release", &release );
                tag.loadR( "x", &x );
                tag.loadR( "y", &y );
                tag.loadR( "w", &w );
                tag.loadR( "h", &h );
                tag.loadR( "fgColor", ci, &fgColor );
                tag.loadR( "bgColor", ci, &bgColor );
                tag.loadR( "topShadowColor", ci, &topShadowColor );
                tag.loadR( "botShadowColor", ci, &botShadowColor );
                tag.loadR( "font", 63, fontTag );
                tag.loadR( "xPosOffset", &ofsX, &zero );
                tag.loadR( "yPosOffset", &ofsY, &zero );
                tag.loadR( "noEdit", &noEdit, &zero );
                tag.loadR( "useFocus", &useFocus, &zero );
                tag.loadR( "button3Popup", &button3Popup, &zero );
                tag.loadR( "invisible", &invisible, &zero );
                tag.loadR( "buttonLabel", &buttonLabel, emptyStr );
                tag.loadR( "numPvs", &numPvs, &zero );
                tag.loadR( "pv", NUMPVS, destPvExpString, &n1, emptyStr );
                tag.loadR( "value", NUMPVS, sourceExpString, &n1, emptyStr );
                tag.loadR( "numDsps", &numDsps, &zero );
                tag.loadR( "displayFileName", maxDsps, displayFileName, &n2, emptyStr );
                tag.loadR( "menuLabel", maxDsps, relLabel, &n2, emptyStr );
                tag.loadR( "closeAction", maxDsps, closeAction, &n2, &zero );
                tag.loadR( "setPosition", 3, setPosEnumStr, setPosEnum, maxDsps, setPostion,  &n2, &setPosOriginal );
                tag.loadR( "allowDups", maxDsps, allowDups, &n2, &zero );
                tag.loadR( "cascade", maxDsps, cascade, &n2, &zero );
                tag.loadR( "symbols", maxDsps, symbolsExpStr, &n2, emptyStr );
                tag.loadR( "replaceSymbols", maxDsps, replaceSymbols, &n2, &zero );
                tag.loadR( "propagateMacros", maxDsps, propagateMacros, &n2, &one );
                tag.loadR( "closeDisplay", maxDsps, closeAction, &n2, &zero );
                tag.loadR( "colorPv", &colorPvExpString, emptyStr );
                tag.loadR( "icon", &icon, &zero );
                tag.loadR( "swapButtons", &swapButtons, &zero );
                tag.loadR( "helpCommand", &helpCommandExpString, emptyStr );
                tag.loadR( "endObjectProperties" );

                stat = tag.readTags( f, "endObjectProperties" );
                if ( !( stat & 1 ) ) {
                    printf("%s\n", tag.errMsg() );
                }

		adjustGeometry(&x, &y);

                // ----------------- write the properties to the ui file
                sprintf(widgetName, "caRelatedDisplay_%d", widgetNumber++);
                myParser->Qt_writeOpenTag("widget", "caRelatedDisplay", widgetName);
                myParser->writeRectangleDimensions(x, y, w, h);
                setFont(myParser, fontTag); // Zai added
                myParser->Qt_setColorForeground("", rgb[fgColor].r/256, rgb[fgColor].g/256, rgb[fgColor].b/256, 255);
                myParser->Qt_setColorBackground("", rgb[bgColor].r/256, rgb[bgColor].g/256, rgb[bgColor].b/256, 255);

                for(int i=0; i<numDsps; i++) {
                    strcat(files, displayFileName[i].getRaw());
                    strcat(files, ";");
                    strcat(labels, relLabel[i].getRaw());
                    strcat(labels, ";");
                    strcat(args, symbolsExpStr[i].getRaw());
                    strcat(args, ";");
                    if(replaceSymbols[i] == 0) strcat(remove, "false"); else strcat(remove, "true");
                    strcat(remove, ";");
                }
                if(strlen(files) > 0) files[strlen(files) -1] = '\0';
                if(strlen(labels) > 0) labels[strlen(labels) -1] = '\0';
                if(strlen(args) > 0) args[strlen(args) -1] = '\0';
                if(strlen(remove) > 0) remove[strlen(remove) -1] = '\0';
                myParser->Qt_handleString("files", "string", files);
                myParser->Qt_handleString("labels", "string", labels);
                myParser->Qt_handleString("args", "string", args);
                myParser->Qt_handleString("removeParent", "string", remove);
                myParser->Qt_handleString("label", "string", buttonLabel.getRaw());
                myParser->Qt_handleString("stackingMode", "enum", "Row");

                myParser->Qt_writeCloseTag("widget", widgetName, 0);
            }
                break;

                //***************************************************************************************************************
            case unknown :
                break;

            default :
                break;
            }

            gotOne = tag.getName( tagName, 255, f );

        }
	else if(strcmp( tagName, "endGroup" ) == 0)
	{
                tag.loadR( "visPv", &visPvExpStr, emptyStr );
                tag.loadR( "visInvert", &visInverted, &zero );
                tag.loadR( "visMin", 39, minVisString, emptyStr );
                tag.loadR( "visMax", 39, maxVisString, emptyStr );

                stat = tag.readTags( f, "endObjectProperties" );

		addVisibilityCalc(myParser, visPvExpStr.getRaw(), minVisString, maxVisString);

                if ( !( stat & 1 ) ) {
                    printf("error message = %s\n", tag.errMsg() );
                }

		myParser->Qt_writeCloseTag("widget", groups[openGroups-1].name, 0);
		openGroups--;
		groups.pop_back();

		gotOne = tag.getName( tagName, 255, f );
	} 

	else {
// Zai            fprintf( stderr, "Unknown tag name: [%s]\n", tagName );
            gotOne = NULL;
            gotOne = tag.getName( tagName, 255, f );    //Zai

        }

    }

    fclose( f );
    return 1;
}
// Zai added
void parserClass::setFont(myParserEDM *myParser, char fontTag[80]){
    QStringList fontTagStringList = QString((char*)fontTag).split("-");
    bool bold = false;
    int fontSize = 10;
    if (fontTagStringList.count() < 4) return;
    if (fontTagStringList.at(1).contains("bold")){
        // set Bold
//        qDebug() << "set bold";
        bold = true;
    }
    if (!fontTagStringList.at(3).isEmpty()){
        // set Font size
        fontSize = QString(fontTagStringList.at(3)).toDouble()/1;
//        qDebug() << "Set  Font Size: " << fontSize;
    }
    myParser->writeFontProperties(fontSize, bold);
    myParser->Qt_handleString("fontScaleMode", "enum",  "None");
}





