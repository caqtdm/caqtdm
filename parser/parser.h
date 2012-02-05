#ifndef PARSER_H
#define PARSER_H


#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <sys/types.h>
#include <math.h>
#include <string.h>
#include <limits.h>
#include <sys/types.h>
#include <time.h>

#ifndef MAX
#define MAX(a,b)  ((a)>(b)?(a):(b))
#endif
#ifndef MIN
#define MIN(a,b)  ((a)<(b)?(a):(b))
#endif

#ifndef STRDUP
#define STRDUP(a) (strcpy( (char *)malloc(strlen(a)+1),a))
#endif

#define MAX_TOKEN_LENGTH        256     /* max size of strings in adl    */
#define MAX_PENS                8       /* max # of pens on strip chart  */
#define MAX_TRACES              8       /* max # of traces on cart. plot */
#define MAX_FILE_CHARS          256     /* max # of chars in filename    */
#define MAX_CALC_RECORDS        4       /* max # of records for calc     */
#define MAX_CALC_INPUTS         12      /* max # of inputs for calc      */
#define DL_MAX_COLORS           65      /* max # of colors for display   */
#define DL_COLORS_COLUMN_SIZE   5       /* # of colors in each column    */

#if MAX_CALC_RECORDS != 4
#error Need to make changes (CALC_A_RC, etc.) if MAX_CALC_RECORDS != 4
#endif

/*********************************************************************
 * Resource Types                                                    *
 *********************************************************************/

#define NUM_LABEL_TYPES         5
typedef enum {
    LABEL_NONE     = 2,
    NO_DECORATIONS = 3,
    OUTLINE        = 4,
    LIMITS         = 5,
    CHANNEL        = 6
} LabelType;
const LabelType FIRST_LABEL_TYPE = LABEL_NONE;

#define NUM_COLOR_MODES         3
typedef enum {
    STATIC   = 7,
    ALARM    = 8,
    DISCRETE = 9
} ColorMode;
const ColorMode FIRST_COLOR_MODE = STATIC;

#define NUM_VISIBILITY_MODES    4
typedef enum {
    V_STATIC    = 10,
    IF_NOT_ZERO = 11,
    IF_ZERO     = 12,
    V_CALC      = 13
} VisibilityMode;
const VisibilityMode FIRST_VISIBILITY_MODE = V_STATIC;

#define NUM_DIRECTIONS          4
typedef enum {
    UP    = 14,
    RIGHT = 15,
    DOWN  = 16,
    LEFT  = 17
} Direction;
const Direction FIRST_DIRECTION = UP;

#define NUM_EDGE_STYLES         3
typedef enum {
    SOLID = 18,
    DASH  = 19,
    BIGDASH = 20
} EdgeStyle;
const EdgeStyle FIRST_EDGE_STYLE = SOLID;

#define NUM_FILL_STYLES         2
typedef enum {
    F_SOLID   = 21,
    F_OUTLINE = 22
} FillStyle;
const FillStyle FIRST_FILL_STYLE = F_SOLID;

#define NUM_TEXT_FORMATS        11
typedef enum {
    MEDM_DECIMAL  = 23,
    EXPONENTIAL   = 24,
    ENGR_NOTATION = 25,
    COMPACT       = 26,
    TRUNCATED     = 27,
    HEXADECIMAL   = 28,
    OCTAL         = 29,
    STRING        = 30,
    SEXAGESIMAL   = 31,
    SEXAGESIMAL_HMS= 32,
    SEXAGESIMAL_DMS= 33
} TextFormat;
const TextFormat FIRST_TEXT_FORMAT = MEDM_DECIMAL;

#define NUM_TEXT_ALIGNS         3
typedef enum {
    HORIZ_LEFT   = 34,
    HORIZ_CENTER = 35,
    HORIZ_RIGHT  = 36
} TextAlign;
const TextAlign FIRST_TEXT_ALIGN = HORIZ_LEFT;


#define NUM_STACKINGS           3
typedef enum {
    COLUMN      = 37,
    ROW         = 38,
    ROW_COLUMN  = 39
} Stacking;
const Stacking FIRST_STACKING = COLUMN;

#define NUM_FILL_MODES          2
typedef enum {
    FROM_EDGE   = 40,
    FROM_CENTER = 41
} FillMode;
const FillMode FIRST_FILL_MODE = FROM_EDGE;

#define NUM_TIME_UNITS          3
typedef enum {
    MILLISECONDS = 42,
    SECONDS      = 43,
    MINUTES      = 44
} TimeUnits;
const TimeUnits FIRST_TIME_UNIT = MILLISECONDS;

#define NUM_CARTESIAN_PLOT_STYLES       6
typedef enum {
    POINT_PLOT      = 45,
    LINE_PLOT       = 46,
    LINEBIGMARKS_PLOT = 47,
    LINENOMARKS_PLOT= 48,
    LINETHICK_PLOT   =49,
    FILL_UNDER_PLOT = 50
} CartesianPlotStyle;
const CartesianPlotStyle FIRST_CARTESIAN_PLOT_STYLE = POINT_PLOT;

#define NUM_ERASE_OLDESTS       2
typedef enum {
    ERASE_OLDEST_OFF = 51,
    ERASE_OLDEST_ON  = 52
} EraseOldest;
const EraseOldest FIRST_ERASE_OLDEST = ERASE_OLDEST_OFF;

#define NUM_IMAGE_TYPES 3
typedef enum {
    NO_IMAGE   = 53,
    GIF_IMAGE  = 54,
    TIFF_IMAGE = 55
} ImageType;
const ImageType FIRST_IMAGE_TYPE = NO_IMAGE;

#define NUM_CARTESIAN_PLOT_AXIS_STYLES 3
typedef enum {
    LINEAR_AXIS = 56,
    LOG10_AXIS  = 57,
    TIME_AXIS   = 58
} CartesianPlotAxisStyle;
const CartesianPlotAxisStyle FIRST_CARTESIAN_PLOT_AXIS_STYLE = LINEAR_AXIS;

#define NUM_CARTESIAN_PLOT_RANGE_STYLES 3
typedef enum {
    CHANNEL_RANGE        = 59,
    USER_SPECIFIED_RANGE = 60,
    AUTO_SCALE_RANGE     = 61
} CartesianPlotRangeStyle;
const CartesianPlotRangeStyle FIRST_CARTESIAN_PLOT_RANGE_STYLE = CHANNEL_RANGE;

#define NUM_ERASE_MODES 2
typedef enum {
    ERASE_IF_NOT_ZERO = 62,
    ERASE_IF_ZERO     = 63
} eraseMode_t;
const eraseMode_t FIRST_ERASE_MODE = ERASE_IF_NOT_ZERO;

#define NUM_RD_MODES 2
typedef enum {
    ADD_NEW_DISPLAY = 64,
    REPLACE_DISPLAY = 65
} relatedDisplayMode_t;
const relatedDisplayMode_t FIRST_RD_MODE = ADD_NEW_DISPLAY;

#define NUM_RD_VISUAL 4
typedef enum {
    RD_MENU       = 66,
    RD_ROW_OF_BTN = 67,
    RD_COL_OF_BTN = 68,
    RD_HIDDEN_BTN = 69
} relatedDisplayVisual_t;
const relatedDisplayVisual_t FIRST_RD_VISUAL = RD_MENU;

#define NUM_CP_TIME_FORMAT 7
typedef enum {
    HHMMSS    = 70,
    HHMM      = 71,
    HH00      = 72,
    MMMDDYYYY = 73,
    MMMDD     = 74,
    MMDDHH00  = 75,
    WDHH00    = 76
} CartesianPlotTimeFormat_t;
const CartesianPlotTimeFormat_t FIRST_CP_TIME_FORMAT = HHMMSS;

#define NUM_BOOLEAN 2
typedef enum {
    BOOLEAN_FALSE = 77,
    BOOLEAN_TRUE  = 78
} Boolean_t;
const Boolean_t FIRST_BOOLEAN = BOOLEAN_FALSE;

#define NUM_PV_LIMITS_SRC 4
typedef enum {
    PV_LIMITS_CHANNEL = 79,
    PV_LIMITS_DEFAULT = 80,
    PV_LIMITS_USER    = 81,
    PV_LIMITS_UNUSED  = 82
} PvLimitsSrc_t;
const PvLimitsSrc_t FIRST_PV_LIMITS_SRC = PV_LIMITS_CHANNEL;

#define NUMBER_STRING_VALUES    83    /* PV_LIMITS_UNUSED+1 */

typedef struct {
    int x, y;
    unsigned int width, height;
} DlObject;

typedef struct {
   int frameX, frameY;
   int frameXprv, frameYprv; 
} FrameOffset;

typedef struct {
    int r, g, b;
    int inten;
} DlColormapEntry;

typedef struct {
    char name[MAX_TOKEN_LENGTH];
    int versionNumber;
} DlFile;

typedef struct {
    DlObject object;
    int clr, bclr;
    char cmap[MAX_TOKEN_LENGTH];
} DlDisplay;

typedef struct {
    int ncolors;
    DlColormapEntry dl_color[DL_MAX_COLORS];
} DlColormap;

#define X_AXIS_ELEMENT  0
#define Y1_AXIS_ELEMENT 1
#define Y2_AXIS_ELEMENT 2

/* Name-value table (for macro substitutions in display files) */
typedef struct {
    char *name;
    char *value;
} NameValueTable;

typedef struct _DisplayInfo {
    FILE *filePtr;
    DlFile *dlFile;
    int versionNumber;
    int drawingAreaBackgroundColor;
    int drawingAreaForegroundColor;
    DlColormap  *dlColormap;
    NameValueTable *nameValueTable;
    int numNameValues;
} DisplayInfo;

typedef enum
{ T_WORD, T_EQUAL, T_QUOTE, T_LEFT_BRACE, T_RIGHT_BRACE, T_EOF} TOKEN;


#define MAX_RELATED_DISPLAYS    16
#define MAX_SHELL_COMMANDS      16
#define SC_DEFAULT_UNITS    SECONDS

#define True 1==1
#define False !True
#define Boolean unsigned int

int parsingCompositeFile;

#ifdef __cplusplus
}
#endif

#endif
