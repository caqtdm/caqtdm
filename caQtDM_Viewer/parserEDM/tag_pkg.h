#ifndef __tag_pkg_h
#define __tag_pkg_h 1

// class for reading info from files by tag/value pairs
#if defined(_MSC_VER)
   #define _CRT_SECURE_NO_WARNINGS
   #include <windows.h>
   #define strdup _strdup
   #define strtok_r strtok_s
#endif

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#include <list>

#include "expString.h"
#include "utility.h"

class efDouble {

private:

    double val;
    int null;

public:

    efDouble ( void ) {
        null = 1;
        val = 0.0;
    }

    ~efDouble ( void ) { }
    double value ( void ) { return val; }
    void setValue ( double v ) { val = v; null = 0; }
    int isNull ( void ) { return null; }
    void setNull ( int n ) { null = n; }

    int write ( FILE *f ) {
        int stat;
        double v, n;

        v = value();
        n = isNull();
        stat = fprintf( f, "%-g %-g\n", v, n );
        if ( stat < 0 ) return 0;
        else            return 1;
    }

    int read ( FILE *f ) {
        int stat;
        double v, n;

        stat = fscanf( f, "%lg %lg\n", &v, &n );
        if ( stat < 0 ) return 0;
        setValue( v );
        setNull( (int) n );
        return 1;

    }
};


class efInt {

private:

    int val;
    int null;

public:

    efInt ( void ) {
        null = 1;
        val = 0;
    }

    ~efInt ( void ) { }

    int value ( void ) { return val; }
    void setValue ( int v ) { val = v; null = 0; }
    int isNull ( void ) { return null; }
    void setNull ( int n ) { null = n; }

    int write ( FILE *f ) {
        int stat, v, n;

        v = value();
        n = isNull();
        stat = fprintf( f, "%-d %-d\n", v, n );
        if ( stat < 0) return 0;
        else           return 1;
    }

    int read ( FILE *f ) {
        int stat, v, n;

        stat = fscanf( f, "%d %d\n", &v, &n );
        if ( stat < 0 ) return 0;
        setValue( v );
        setNull( n );
        return 1;
    }

};


#include "tag_pkg.str"

// prefix for foreign tags - edm remembers foreign tags and
// writes them back during a save operation
#define UNKNOWN_PREFIX '_'

using namespace std;

struct unknownTag {
    char *tag;
    char *val;
    bool isCompound;

    unknownTag( char *tag, char *val, bool isCompound );
    unknownTag( const unknownTag &other );
    unknownTag &operator=( const unknownTag &other );
    ~unknownTag();
};
typedef list < unknownTag > unknownTagList;

class tagClass {

    unknownTagList *unknownTags;

public:

    tagClass ( void );
    int genDoc ( void );
    void setGenDoc (int flag );
    void initLine ( void );
    void incLine ( void );

    int line ( void );
    void setLine (int _line );

    const char *filename ( void );
    static void setFileName (const char *curFileName );
    static void pushLevel ( void );
    static void popLevel ( void );

    int init ( void );

    // load for read functions
    int loadR (char *tag);
    int loadR (char *tag, int *destination);
    int loadR (char *tag, int *destination,int *oneDefault);
    int loadR (char *tag, unsigned int *destination);
    int loadR (char *tag, unsigned int *destination, unsigned int *oneDefault);
    int loadR (char *tag, int numValues, char **strValues, int *intValues, int *destination );                 // enumerated, translate string into integer destination
    int loadR (char *tag, int numValues, char **strValues, int *intValues, int *destination, int *oneDefault); // enumerated, translate string into integer destination
    int loadR (char *tag, int numValues, char **strValues, int *intValues, int maxLen, int *destination, int *numElements); // enumerated array, translate integer source
    int loadR (char *tag, int numValues, char **strValues, int *intValues, int maxLen, int *destination, int *numElements,int *oneDefault);  // enumerated array, translate integer source into string value
    int loadR (char *tag, double *destination);
    int loadR (char *tag, double *destination, double *oneDefault);
    int loadR (char *tag, efDouble *destination);
    int loadR (char *tag, int maxLen, efDouble *destination, int *numElements);
    int loadR (char *tag, efDouble *destination, double *oneDefault);
    int loadR (char *tag, int maxLen, efDouble *destination, int *numElements, double *oneDefault);
    int loadR (char *tag, efInt *destination);
    int loadR (char *tag, int maxLen, efInt *destination, int *numElements);
    int loadR (char *tag, efInt *destination, int *oneDefault);
    int loadR (char *tag, int maxLen, efInt *destination, int *numElements, int *oneDefault);
    int loadR (char *tag, int maxLen, char *destination);
    int loadR (char *tag, int maxLen, char *destination, char *oneDefault);
    int loadR (char *tag, int maxLen, int strSize, char *destination, int *numElements); // fixed length string array
    int loadR (char *tag, int maxLen, int strSize, char *destination, int *numElements, char *oneDefault); // fixed length string array
    int loadR (char *tag, char **destination); // dynamic string
    int loadR (char *tag, expStringClass *destination);
    int loadR (char *tag, expStringClass *destination, char *oneDefault);
    int loadR (char *tag, int maxLen, expStringClass *source, int *numElements);
    int loadR (char *tag, int maxLen, expStringClass *source, int *numElements, char *oneDefault);
    int loadR (char *tag, void *_ci, int maxLen, int *destination, int *numElements);
    int loadR (char *tag, int maxLen, int *destination, int *numElements);
    int loadR (char *tag, int maxLen, int *destination, int *numElements, int *oneDefault);
    int loadR (char *tag, int maxLen, unsigned int *destination, int *numElements);
    int loadR (char *tag, int maxLen,unsigned int *destination, int *numElements, unsigned int *oneDefault);
    int loadR (char *tag, int **destination,int *numElements); // dynamic integer array
    int loadR (char *tag, int maxLen, double *destination,int *numElements);
    int loadR (char *tag, int maxLen, double *destination,int *numElements,double *oneDefault);
    int loadR (char *tag, double **destination, int *numElements); // dynamic double array
    int loadR (char *tag, void *_ci, int *destination);

    int loadR (
            unknownTagList &unknownTags
            );

    char *getName (
            char *tag,
            int maxLen,
            FILE *f
            );

    char *getCompoundValue (
            char *val,
            int maxLen,
            FILE *f
            );

    char *getValue (
            char *val,
            int maxLen,
            FILE *f,
            int *valueIsCompound
            );

    char *getCompoundValue (
            char **valp,
            int *maxLen,
            FILE *f,
            int *overFlow
            );

    char *getValue (
            char **valp,
            int *maxLen,
            FILE *f,
            int *valueIsCompound,
            int *overFlow
            );

    void writeMultiLineString (
            FILE *f,
            char *s
            );

    void writeString (
            FILE *f,
            char *s
            );

    int decode (
            char *tag,
            char *val,
            int valueIsCompound
            );

    int readTags (
            FILE *f,
            char *endingTag
            );

    int writeTags (
            FILE *f
            );

    char *errMsg ( void );

private:

    static const int MAX = 1000;
    static const int MAXBUF = 10000;

    static const int LABEL = 1;
    static const int COLOR = 2;
    static const int INTEGER = 3;
    static const int REAL = 4;
    static const int STRING = 5;
    static const int COMPLEX_STRING = 6;
    static const int PV_COLOR = 7;
    static const int EXP_STRING = 8;
    static const int COMPLEX_EXP_STRING = 9;
    static const int EF_INT = 10;
    static const int EF_DOUBLE = 11;
    static const int EF_STRING = 12;
    static const int BOOLEAN = 13;
    static const int ENUMERATED = 14;
    static const int INTEGER_ARRAY = 15;
    static const int REAL_ARRAY = 16;
    static const int EF_INT_ARRAY = 17;
    static const int EF_DOUBLE_ARRAY = 18;
    static const int EF_STRING_ARRAY = 19;
    static const int DYNAMIC_STRING = 20;
    static const int DYNAMIC_INTEGER_ARRAY = 21;
    static const int DYNAMIC_REAL_ARRAY = 22;
    static const int EXP_STRING_ARRAY = 23;
    static const int HEX_INTEGER = 24;
    static const int HEX_INTEGER_ARRAY = 25;
    static const int UNSIGNED_INTEGER = 26;
    static const int UNSIGNED_INTEGER_ARRAY = 27;
    static const int ENUMERATED_ARRAY = 28;
    static const int COLOR_ARRAY = 29;
    static const int PV_COLOR_ARRAY = 30;
    static const int STRING_ARRAY = 31;

    static const int UNKNOWN = 32;

    static const int TAG_E_SUCCESS = 1;
    static const int TAG_E_NOTAGS = 100;

    char buf[MAXBUF+1];
    char msg[79+1];

    int first, last, len;
    int numTags;
    char *tagName[MAX];
    void *tagDestination[MAX];     // will use this for source also
    int tagDestMaxSize[MAX];       // for strings and arrays
    int *tagDestNumElements[MAX];  // for arrays
    char tagDestType[MAX];         // c=color, i=integer, s=string, d=double
    char isCompound[MAX];          // for compound values
    void *tagDefault[MAX];
    int enumNumChoices[MAX];
    int *enumIntArray[MAX];
    char **enumStrArray[MAX];
    //colorInfoClass *ci;

};

#endif
