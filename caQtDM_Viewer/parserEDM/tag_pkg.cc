#include "tag_pkg.h"

#define UNUSED(x) (void)(x)

static int g_genDocFlag = 0;

#define MAXLEVEL 5
static int fileLineNumber[MAXLEVEL] = { 1, 1, 1, 1, 1 };
static char fileName[MAXLEVEL][127+1];
static int level = -1;

unknownTag::unknownTag( char *tag, char *val, bool isCompound ) {
    this->tag = strdup( tag );
    this->val = strdup( val );
    this->isCompound = isCompound;
}

unknownTag::unknownTag( const unknownTag &other ) {
    this->tag = strdup( other.tag );
    this->val = strdup( other.val );
    this->isCompound = other.isCompound;
}

unknownTag &unknownTag::operator=( const unknownTag &other ) {
    if ( this->tag ) {
        free( this->tag );
    }
    if ( this->val ) {
        free( this->val );
    }
    this->tag = strdup( other.tag );
    this->val = strdup( other.val );
    this->isCompound = other.isCompound;
    return *this;
}

unknownTag::~unknownTag() {
    if ( this->tag ) {
        free( this->tag );
    }
    if ( this->val ) {
        free( this->val );
    }
}

tagClass::tagClass ( void ) : unknownTags(NULL) {

    char *envPtr;
    int i;

    numTags = 0;
    //ci = NULL;
    first = last = len = 0;

    envPtr = getenv( "EDMGENDOC" );
    if ( envPtr ) {
        g_genDocFlag = 1;
    }
    else {
        g_genDocFlag = 0;
    }

    if ( level == -1 ) {
        level = 0;
        for ( i=0; i<MAXLEVEL; i++ ) {
            fileLineNumber[level] = 1;
            strcpy( fileName[level], "" );
        }
    }

}

int tagClass::genDoc ( void ) {

    return g_genDocFlag;

}

void tagClass::setGenDoc (
        int flag ) {

    g_genDocFlag = flag;

}

void tagClass::initLine ( void ) {

    fileLineNumber[level] = 1;

}

void tagClass::incLine ( void ) {

    fileLineNumber[level]++;

}

int tagClass::line ( void ) {

    return fileLineNumber[level];

}

void tagClass::setLine (
        int _line ) {

    if ( ( level > -1 ) && ( level < (MAXLEVEL-2) ) ) {
        fileLineNumber[level] = _line;
    }
    else {
        fileLineNumber[level] = 0;
    }

}

const char *tagClass::filename ( void ) {

    return (const char *) fileName[level];

}

void tagClass::setFileName (
        const char *curFileName ) {

    if ( ( level > -1 ) && ( level < (MAXLEVEL-2) ) ) {

        strcpy( fileName[level], "" );
        if ( curFileName ) {
            strncat( fileName[level], "(", 1 );
            strncat( fileName[level], curFileName, 125 );
            strncat( fileName[level], ")", 1 );
            fileName[level][127] = 0;
        }

    }
    else {

        strcpy( fileName[level], "Unknown (overflow)" );

    }

}

void tagClass::pushLevel ( void ) {

    if ( level < (MAXLEVEL-1) ) level++;

}

void tagClass::popLevel ( void ) {

    if ( level > 0 ) level--;

}

int tagClass::init ( void ) {

    numTags = 0;
    //ci = NULL;
    first = last = len = 0;
    unknownTags = NULL;

    return 1;

}

char *tagClass::getName (char *tag,int maxLen, FILE *f)
{

    // get next non-blank and non-comment line
    int i, ii, ignore;
    char *gotOne;

    strcpy( tag, "" );

    do {

        gotOne = fgets( buf, MAXBUF, f );
        if ( !gotOne ) return NULL;

        incLine();

        len = strlen(buf);

        // find 1st non-white-space char
        ignore = 1;
        for ( i=0; i<len; i++ ) {
            if ( !isspace( buf[i] ) ) {
                first = i;
                if ( buf[i] == '#' ) {
                    break;
                }
                else {
                    ignore = 0;
                    break;
                }
            }
        }

        if ( !ignore ) { // got something significant

            last = first;
            for ( i=first+1; i<len; i++ ) {
                if ( !isspace( buf[i] ) ) {
                    last = i;
                }
                else {
                    break;
                }
            }

        }

    } while ( ignore );

    // copy string to tag

    for ( i=first, ii=0; i<=last && ii<maxLen; i++, ii++ ) {
        tag[ii] = buf[i];
    }
    tag[ii] = 0;

    first = last + 1;
    last = first;

    return tag;

}

char *tagClass::getCompoundValue (
        char *val,
        int maxLen,
        FILE *f
        ) {

    int i, ii, firstNonWS, firstChar, lastChar, more, foundQuote, escape = 0;
    char *gotOne;

    //fprintf( stderr, "getCompoundValue\n" );

    // read and append file contents until "}"

    more = 1;
    ii = 0;

    do {

        gotOne = fgets( buf, MAXBUF, f );
        if ( !gotOne ) return NULL;

        incLine();

        lastChar = strlen(buf) - 1;

        // find 1st non-white-space char
        firstNonWS = -1;
        for ( i=0; i<=lastChar; i++ ) {
            if ( !isspace( buf[i] ) ) {
                firstNonWS = i;
                break;
            }
        }
        if ( firstNonWS == -1 ) {
            return NULL;
        }

        firstChar = firstNonWS;

        if ( lastChar > 0 ) {

            if ( buf[lastChar] == '\"' ) {
                lastChar = lastChar - 1;
            }
            else if ( buf[lastChar] == '\n' ) {
                if ( buf[lastChar-1] == '\"' ) {
                    buf[lastChar-1] = '\n';
                    buf[lastChar] = 0;
                    lastChar--;
                }
            }

        }

        //fprintf( stderr, "string is [%s]\n", buf );
        //fprintf( stderr, "firstChar = %-d\n", firstChar );
        //fprintf( stderr, "lastChar = %-d\n", lastChar );

        foundQuote = escape = 0;
        for ( i=firstChar; i<=lastChar; i++ ) {

            if ( !escape ) {

                if ( buf[i] == '}' ) {

                    more = 0;
                    break;

                }
                else if ( buf[i] == '\\' ) {

                    escape = 1;

                }
                else if ( ( buf[i] == '\"' ) && !foundQuote ) {

                    foundQuote = 1;

                }
                else {

                    if ( ii < maxLen-1 ) {
                        val[ii++] = buf[i];
                    }

                }

            }
            else { // escape: next char is data

                escape = 0;
                if ( ii < maxLen-1 ) {
                    val[ii++] = buf[i];
                }

            }

        }

    } while ( more );

    val[ii] = 0;

    return val;

}

char *tagClass::getCompoundValue (
        char **valp,  // valp initially points to a stack variable, on overflow we allocate a new buffer
        int *maxLen,
        FILE *f,
        int *overFlow // set if an overflow occurs so caller may delete the allocated buffer
        ) {

    int i, ii, firstNonWS, firstChar, lastChar, more, foundQuote, escape = 0, maxLenNew;
    char *gotOne, *val, *valNew;

    //fprintf( stderr, "getCompoundValue\n" );

    // read and append file contents until "}"

    val = *valp;

    more = 1;
    ii = 0;
    *overFlow = 0;

    do {

        gotOne = fgets( buf, MAXBUF, f );
        if ( !gotOne ) return NULL;

        incLine();

        lastChar = strlen(buf) - 1;

        // find 1st non-white-space char
        firstNonWS = -1;
        for ( i=0; i<=lastChar; i++ ) {
            if ( !isspace( buf[i] ) ) {
                firstNonWS = i;
                break;
            }
        }
        if ( firstNonWS == -1 ) {
            return NULL;
        }

        firstChar = firstNonWS;

        if ( lastChar > 0 ) {

            if ( buf[lastChar] == '\"' ) {
                lastChar = lastChar - 1;
            }
            else if ( buf[lastChar] == '\n' ) {
                if ( buf[lastChar-1] == '\"' ) {
                    buf[lastChar-1] = '\n';
                    buf[lastChar] = 0;
                    lastChar--;
                }
            }

        }

        //fprintf( stderr, "string is [%s]\n", buf );
        //fprintf( stderr, "firstChar = %-d\n", firstChar );
        //fprintf( stderr, "lastChar = %-d\n", lastChar );

        foundQuote = escape = 0;
        for ( i=firstChar; i<=lastChar; i++ ) {

            if ( !escape ) {

                if ( buf[i] == '}' ) {

                    more = 0;
                    break;

                }
                else if ( buf[i] == '\\' ) {

                    escape = 1;

                }
                else if ( ( buf[i] == '\"' ) && !foundQuote ) {

                    foundQuote = 1;

                }
                else {

                    if ( ii < *maxLen-1 ) {
                        val[ii++] = buf[i];
                    }
                    else {
                        if ( *overFlow ) { // if this is set we overflowed more than once
                            maxLenNew = (*maxLen) * 2;
                            valNew = new char[maxLenNew+1];
                            memcpy( valNew, val, *maxLen );
                            if ( val ) delete[] val;
                            *valp = valNew;
                            val = valNew;
                            *maxLen = maxLenNew;
                        }
                        else { // else valp is a stack variable
                            maxLenNew = (*maxLen) * 2;
                            valNew = new char[maxLenNew+1];
                            memcpy( valNew, val, *maxLen );
                            *valp = valNew;
                            val = valNew;
                            *maxLen = maxLenNew;
                        }
                        *overFlow = 1;
                        val[ii++] = buf[i];
                    }

                }

            }
            else { // escape: next char is data

                escape = 0;
                if ( ii < *maxLen-1 ) {
                    val[ii++] = buf[i];
                }
                else {
                    if ( *overFlow ) { // if this is set we overflowed more than once
                        maxLenNew = (*maxLen) * 2;
                        valNew = new char[maxLenNew+1];
                        memcpy( valNew, val, *maxLen );
                        if ( val ) delete[] val;
                        *valp = valNew;
                        val = valNew;
                        *maxLen = maxLenNew;
                    }
                    else { // else valp is a stack variable
                        maxLenNew = (*maxLen) * 2;
                        valNew = new char[maxLenNew+1];
                        memcpy( valNew, val, *maxLen );
                        *valp = valNew;
                        val = valNew;
                        *maxLen = maxLenNew;
                    }
                    *overFlow = 1;
                    val[ii++] = buf[i];
                }

            }

        }

    } while ( more );

    val[ii] = 0;

    return val;

}

char *tagClass::getValue (
        char *val,
        int maxLen,
        FILE *f,
        int *valueIsCompound
        ) {

    int i, ii, firstNonWS;
    char *tk, *context, *gotData;
    int firstChar, lastChar, escape;

    //fprintf( stderr, "first = %-d\n", first );
    //fprintf( stderr, "last = %-d\n", last );
    //fprintf( stderr, "len = %-d\n", len );

    *valueIsCompound = 0;
    strcpy( val, "" );

    // find 1st non-white-space char
    firstNonWS = -1;
    for ( i=first; i<len; i++ ) {
        if ( !isspace( buf[i] ) ) {
            firstNonWS = i;
            break;
        }
    }
    if ( firstNonWS == -1 ) {
        return NULL;
    }

    // we may have a compound entry, e.g.
    //
    // <keyword name> {
    //   12
    //   55
    //   3
    //   .
    //   .
    //   .
    // }

    if ( buf[firstNonWS] == '{' ) {
        *valueIsCompound = 1;
    }
    else {
        *valueIsCompound = 0;
    }

    if ( *valueIsCompound ) {

        if ( len > firstNonWS+1 ) {
            context = NULL;
            tk = strtok_r( &buf[firstNonWS+1], " \t\n", &context );
            if ( tk ) {
                fprintf( stderr, tagClass_str1, line(), filename() );
                return NULL;
            }
        }

        gotData = getCompoundValue( val, maxLen, f );
        if ( !gotData ) {
            fprintf( stderr, tagClass_str2, line(), filename() );
            return NULL;
        }

    }
    else {  // simple entry contained on one line

        tk = &buf[firstNonWS];

        //fprintf( stderr, "string is [%s]\n", tk );

        firstChar = 0;
        if ( tk[0] == '\"' ) firstChar = 1;

        lastChar = strlen(tk) - 1;
        if ( lastChar > 0 ) {

            if ( tk[lastChar] == '\"' ) {
                lastChar = lastChar - 1;
            }
            else if ( tk[lastChar] == '\n' ) {
                if ( tk[lastChar-1] == '\"' ) {
                    tk[lastChar-1] = '\n';
                    tk[lastChar] = 0;
                }
            }

        }

        // copy string to tag

        escape = 0;

        if ( tk ) {

            for ( i=firstChar, ii=0; (i<=(int)lastChar) && (ii<maxLen); i++ ) {

                if ( !escape ) {

                    if ( tk[i] == '\\' ) {
                        escape = 1;
                    }
                    else {
                        val[ii++] = tk[i];
                    }

                }
                else {

                    escape = 0;
                    val[ii++] = tk[i];

                }

            }

            val[ii++] = 0;

        }

        else {

            return NULL;

        }

    }

    first = last = len = 0;

    return val;

}

char *tagClass::getValue (
        char **valp,
        int *maxLen,
        FILE *f,
        int *valueIsCompound,
        int *overFlow
        ) {

    int i, ii, firstNonWS;
    char *tk, *context, *gotData, *val;
    int firstChar, lastChar, escape;

    //fprintf( stderr, "first = %-d\n", first );
    //fprintf( stderr, "last = %-d\n", last );
    //fprintf( stderr, "len = %-d\n", len );

    *valueIsCompound = 0;
    *overFlow = 0;
    val = *valp;
    strcpy( val, "" );

    // find 1st non-white-space char
    firstNonWS = -1;
    for ( i=first; i<len; i++ ) {
        if ( !isspace( buf[i] ) ) {
            firstNonWS = i;
            break;
        }
    }
    if ( firstNonWS == -1 ) {
        return NULL;
    }

    // we may have a compound entry, e.g.
    //
    // <keyword name> {
    //   12
    //   55
    //   3
    //   .
    //   .
    //   .
    // }

    if ( buf[firstNonWS] == '{' ) {
        *valueIsCompound = 1;
    }
    else {
        *valueIsCompound = 0;
    }

    if ( *valueIsCompound ) {

        if ( len > firstNonWS+1 ) {
            context = NULL;
            tk = strtok_r( &buf[firstNonWS+1], " \t\n", &context );
            if ( tk ) {
                fprintf( stderr, tagClass_str1, line(), filename() );
                return NULL;
            }
        }

        gotData = getCompoundValue( valp, maxLen, f, overFlow );
        if ( !gotData ) {
            fprintf( stderr, tagClass_str2, line(), filename() );
            return NULL;
        }

    }
    else {  // simple entry contained on one line

        tk = &buf[firstNonWS];

        //fprintf( stderr, "string is [%s]\n", tk );

        firstChar = 0;
        if ( tk[0] == '\"' ) firstChar = 1;

        lastChar = strlen(tk) - 1;
        if ( lastChar > 0 ) {

            if ( tk[lastChar] == '\"' ) {
                lastChar = lastChar - 1;
            }
            else if ( tk[lastChar] == '\n' ) {
                if ( tk[lastChar-1] == '\"' ) {
                    tk[lastChar-1] = '\n';
                    tk[lastChar] = 0;
                }
            }

        }

        // copy string to tag

        escape = 0;

        if ( tk ) {

            for ( i=firstChar, ii=0; (i<=(int)lastChar) && (ii<*maxLen); i++ ) {

                if ( !escape ) {

                    if ( tk[i] == '\\' ) {
                        escape = 1;
                    }
                    else {
                        val[ii++] = tk[i];
                    }

                }
                else {

                    escape = 0;
                    val[ii++] = tk[i];

                }

            }

            val[ii++] = 0;

        }

        else {

            return NULL;

        }

    }

    first = last = len = 0;

    return val;

}

int tagClass::loadR (
        char *tag
        ) {

    tagName[numTags] = tag;
    tagDestination[numTags] = (void *) NULL;
    tagDestType[numTags] = tagClass::LABEL;

    if ( numTags < MAX ) numTags++;

    return 1;

}

int tagClass::loadR (
        char *tag,
        int *destination
        ) {
    tagName[numTags] = tag;
    tagDestination[numTags] = (void *) destination;
    tagDestType[numTags] = tagClass::INTEGER;

    if ( numTags < MAX ) numTags++;

    return 1;

}

int tagClass::loadR (
        char *tag,
        int *destination,
        int *oneDefault
        ) {

    *destination = *oneDefault;

    tagName[numTags] = tag;
    tagDestination[numTags] = (void *) destination;
    tagDestType[numTags] = tagClass::INTEGER;

    if ( numTags < MAX ) numTags++;

    return 1;

}

int tagClass::loadR (
        char *tag,
        unsigned int *destination
        ) {

    tagName[numTags] = tag;
    tagDestination[numTags] = (void *) destination;
    tagDestType[numTags] = tagClass::UNSIGNED_INTEGER;

    if ( numTags < MAX ) numTags++;

    return 1;

}

int tagClass::loadR (
        char *tag,
        unsigned int *destination,
        unsigned int *oneDefault
        ) {

    *destination = *oneDefault;

    tagName[numTags] = tag;
    tagDestination[numTags] = (void *) destination;
    tagDestType[numTags] = tagClass::UNSIGNED_INTEGER;

    if ( numTags < MAX ) numTags++;

    return 1;

}

int tagClass::loadR ( // enumerated, translate string into integer destination
                      char *tag,
                      int numValues,
                      char **strValues,
                      int *intValues,
                      int *destination
                      ) {

    tagName[numTags] = tag;
    tagDestination[numTags] = (void *) destination;
    tagDestType[numTags] = tagClass::ENUMERATED;
    enumNumChoices[numTags] = numValues;
    enumStrArray[numTags] = strValues;
    enumIntArray[numTags] = intValues;

    if ( numTags < MAX ) numTags++;

    return 1;

}

int tagClass::loadR ( // enumerated, translate string into integer destination
                      char *tag,
                      int numValues,
                      char **strValues,
                      int *intValues,
                      int *destination,
                      int *oneDefault
                      ) {

    *destination = *oneDefault;

    tagName[numTags] = tag;
    tagDestination[numTags] = (void *) destination;
    tagDestType[numTags] = tagClass::ENUMERATED;
    enumNumChoices[numTags] = numValues;
    enumStrArray[numTags] = strValues;
    enumIntArray[numTags] = intValues;

    if ( numTags < MAX ) numTags++;

    return 1;

}

int tagClass::loadR ( // enumerated array, translate string into integer destination
                      char *tag,
                      int numValues,
                      char **strValues,
                      int *intValues,
                      int maxLen,
                      int *destination,
                      int *numElements
                      ) {

    tagName[numTags] = tag;
    tagDestination[numTags] = (void *) destination;
    tagDestMaxSize[numTags] = maxLen;
    tagDestType[numTags] = tagClass::ENUMERATED_ARRAY;
    *numElements = 0;
    tagDestNumElements[numTags] = numElements;
    enumNumChoices[numTags] = numValues;
    enumStrArray[numTags] = strValues;
    enumIntArray[numTags] = intValues;

    if ( numTags < MAX ) numTags++;

    return 1;

}

int tagClass::loadR ( // enumerated array, translate string into integer destination
                      char *tag,
                      int numValues,
                      char **strValues,
                      int *intValues,
                      int maxLen,
                      int *destination,
                      int *numElements,
                      int *oneDefault
                      ) {

    int i;

    for ( i=0; i<maxLen; i++ ) {
        destination[i] = *oneDefault;
    }

    tagName[numTags] = tag;
    tagDestination[numTags] = (void *) destination;
    tagDestMaxSize[numTags] = maxLen;
    tagDestType[numTags] = tagClass::ENUMERATED_ARRAY;
    *numElements = 0;
    tagDestNumElements[numTags] = numElements;
    enumNumChoices[numTags] = numValues;
    enumStrArray[numTags] = strValues;
    enumIntArray[numTags] = intValues;

    if ( numTags < MAX ) numTags++;

    return 1;

}

int tagClass::loadR (
        char *tag,
        double *destination
        ) {

    tagName[numTags] = tag;
    tagDestination[numTags] = (void *) destination;
    tagDestType[numTags] = tagClass::REAL;

    if ( numTags < MAX ) numTags++;

    return 1;

}

int tagClass::loadR (
        char *tag,
        double *destination,
        double *oneDefault
        ) {

    *destination = *oneDefault;

    tagName[numTags] = tag;
    tagDestination[numTags] = (void *) destination;
    tagDestType[numTags] = tagClass::REAL;

    if ( numTags < MAX ) numTags++;

    return 1;

}

int tagClass::loadR (
        char *tag,
        efDouble *destination
        ) {

    tagName[numTags] = tag;
    tagDestination[numTags] = (void *) destination;
    tagDestType[numTags] = tagClass::EF_DOUBLE;

    if ( numTags < MAX ) numTags++;

    return 1;

}

int tagClass::loadR (
        char *tag,
        int maxLen,
        efDouble *destination,
        int *numElements
        ) {

    tagName[numTags] = tag;
    tagDestination[numTags] = (void *) destination;
    tagDestMaxSize[numTags] = maxLen;
    tagDestType[numTags] = tagClass::EF_DOUBLE_ARRAY;
    *numElements = 0;
    tagDestNumElements[numTags] = numElements;

    if ( numTags < MAX ) numTags++;

    return 1;

}

int tagClass::loadR (
        char *tag,
        efDouble *destination,
        double *oneDefault
        ) {

    if ( oneDefault ) {
        destination->setValue( *oneDefault );
        destination->setNull( 0 );
    }
    else {
        destination->setValue( 0.0 );
        destination->setNull( 1 );
    }

    tagName[numTags] = tag;
    tagDestination[numTags] = (void *) destination;
    tagDestType[numTags] = tagClass::EF_DOUBLE;

    if ( numTags < MAX ) numTags++;

    return 1;

}

int tagClass::loadR (
        char *tag,
        int maxLen,
        efDouble *destination,
        int *numElements,
        double *oneDefault
        ) {

    int i;

    if ( oneDefault ) {
        for ( i=0; i<maxLen; i++ ) {
            destination[i].setValue( *oneDefault );
            destination->setNull( 0 );
        }
    }
    else {
        for ( i=0; i<maxLen; i++ ) {
            destination[i].setValue( 0.0 );
            destination[i].setNull( 1 );
        }
    }

    tagName[numTags] = tag;
    tagDestination[numTags] = (void *) destination;
    tagDestMaxSize[numTags] = maxLen;
    tagDestType[numTags] = tagClass::EF_DOUBLE_ARRAY;
    *numElements = 0;
    tagDestNumElements[numTags] = numElements;

    if ( numTags < MAX ) numTags++;

    return 1;

}

int tagClass::loadR (
        char *tag,
        efInt *destination
        ) {

    tagName[numTags] = tag;
    tagDestination[numTags] = (void *) destination;
    tagDestType[numTags] = tagClass::EF_INT;

    if ( numTags < MAX ) numTags++;

    return 1;

}

int tagClass::loadR (
        char *tag,
        int maxLen,
        efInt *destination,
        int *numElements
        ) {

    tagName[numTags] = tag;
    tagDestination[numTags] = (void *) destination;
    tagDestMaxSize[numTags] = maxLen;
    tagDestType[numTags] = tagClass::EF_INT_ARRAY;
    *numElements = 0;
    tagDestNumElements[numTags] = numElements;

    if ( numTags < MAX ) numTags++;

    return 1;

}

int tagClass::loadR (
        char *tag,
        efInt *destination,
        int *oneDefault
        ) {

    if ( oneDefault ) {
        destination->setValue( *oneDefault );
        destination->setNull( 0 );
    }
    else {
        destination->setValue( 0 );
        destination->setNull( 1 );
    }

    tagName[numTags] = tag;
    tagDestination[numTags] = (void *) destination;
    tagDestType[numTags] = tagClass::EF_INT;

    if ( numTags < MAX ) numTags++;

    return 1;

}

int tagClass::loadR (
        char *tag,
        int maxLen,
        efInt *destination,
        int *numElements,
        int *oneDefault
        ) {

    int i;

    if ( oneDefault ) {
        for ( i=0; i<maxLen; i++ ) {
            destination[i].setValue( *oneDefault );
            destination->setNull( 0 );
        }
    }
    else {
        for ( i=0; i<maxLen; i++ ) {
            destination[i].setValue( 0 );
            destination[i].setNull( 1 );
        }
    }

    tagName[numTags] = tag;
    tagDestination[numTags] = (void *) destination;
    tagDestMaxSize[numTags] = maxLen;
    tagDestType[numTags] = tagClass::EF_INT_ARRAY;
    *numElements = 0;
    tagDestNumElements[numTags] = numElements;

    if ( numTags < MAX ) numTags++;

    return 1;

}

int tagClass::loadR (
        char *tag,
        void *_ci,
        int *destination
        ) {
    UNUSED(_ci);
    tagName[numTags] = tag;
    tagDestination[numTags] = (void *) destination;
    tagDestType[numTags] = tagClass::COLOR;
    if ( numTags < MAX ) numTags++;

    return 1;

}
/*
int tagClass::loadR (
  char *tag,
  colorInfoClass *_ci,
  pvColorClass *destination
) {

  tagName[numTags] = tag;
  tagDestination[numTags] = (void *) destination;
  if ( !ci ) ci = _ci;
  tagDestType[numTags] = tagClass::PV_COLOR;

  if ( numTags < MAX ) numTags++;

  return 1;

}
*/
int tagClass::loadR (
  char *tag,
  void *_ci,
  int maxLen,
  int *destination,
  int *numElements
) {
    UNUSED(_ci);
  tagName[numTags] = tag;
  tagDestination[numTags] = (void *) destination;
  tagDestMaxSize[numTags] = maxLen;
  tagDestType[numTags] = tagClass::COLOR_ARRAY;
  *numElements = 0;
  tagDestNumElements[numTags] = numElements;

  if ( numTags < MAX ) numTags++;

  return 1;

}
/*
int tagClass::loadR (
  char *tag,
  colorInfoClass *_ci,
  int maxLen,
  pvColorClass *destination,
  int *numElements
) {

  tagName[numTags] = tag;
  tagDestination[numTags] = (void *) destination;
  tagDestMaxSize[numTags] = maxLen;
  //if ( !ci ) ci = _ci;
  tagDestType[numTags] = tagClass::PV_COLOR_ARRAY;
  *numElements = 0;
  tagDestNumElements[numTags] = numElements;

  if ( numTags < MAX ) numTags++;

  return 1;

}
*/
int tagClass::loadR (
        char *tag,
        int maxLen,
        char *destination
        ) {

    tagName[numTags] = tag;
    tagDestination[numTags] = (void *) destination;
    tagDestMaxSize[numTags] = maxLen;
    tagDestType[numTags] = tagClass::STRING;

    if ( numTags < MAX ) numTags++;

    return 1;

}

int tagClass::loadR (
        char *tag,
        int maxLen,
        char *destination,
        char *oneDefault
        ) {

    strncpy( destination, oneDefault, maxLen );
    destination[maxLen] = 0;

    tagName[numTags] = tag;
    tagDestination[numTags] = (void *) destination;
    tagDestMaxSize[numTags] = maxLen;
    tagDestType[numTags] = tagClass::STRING;

    if ( numTags < MAX ) numTags++;

    return 1;

}

int tagClass::loadR ( // fixed length string array
                      char *tag,
                      int maxLen,
                      int strSize,
                      char *destination,
                      int *numElements
                      ) {

    tagName[numTags] = tag;
    tagDestination[numTags] = (void *) destination;
    tagDestMaxSize[numTags] = maxLen;
    tagDestType[numTags] = tagClass::STRING_ARRAY;
    enumNumChoices[numTags] = strSize;
    *numElements = 0;
    tagDestNumElements[numTags] = numElements;

    if ( numTags < MAX ) numTags++;

    return 1;

}

int tagClass::loadR ( // fixed length string array
                      char *tag,
                      int maxLen,
                      int strSize,
                      char *destination,
                      int *numElements,
                      char *oneDefault
                      ) {

    int i;
    char *s = destination;

    for ( i=0; i<maxLen; i++ ) {
        strncpy( s, oneDefault, strSize-1 );
        s[strSize-1] = 0;
        s += strSize;
    }

    tagName[numTags] = tag;
    tagDestination[numTags] = (void *) destination;
    tagDestMaxSize[numTags] = maxLen;
    tagDestType[numTags] = tagClass::STRING_ARRAY;
    enumNumChoices[numTags] = strSize;
    *numElements = 0;
    tagDestNumElements[numTags] = numElements;

    if ( numTags < MAX ) numTags++;

    return 1;

}

int tagClass::loadR ( // dynamic string
                      char *tag,
                      char **destination
                      ) {

    tagName[numTags] = tag;
    tagDestination[numTags] = (void *) destination;
    tagDestMaxSize[numTags] = 0;
    tagDestType[numTags] = tagClass::DYNAMIC_STRING;

    if ( numTags < MAX ) numTags++;

    return 1;

}

int tagClass::loadR (
        char *tag,
        expStringClass *destination
        ) {

    tagName[numTags] = tag;
    tagDestination[numTags] = (void *) destination;
    tagDestType[numTags] = tagClass::EXP_STRING;

    if ( numTags < MAX ) numTags++;

    return 1;

}

int tagClass::loadR (
        char *tag,
        expStringClass *destination,
        char *oneDefault
        ) {

    destination->setRaw( oneDefault );

    tagName[numTags] = tag;
    tagDestination[numTags] = (void *) destination;
    tagDestType[numTags] = tagClass::EXP_STRING;

    if ( numTags < MAX ) numTags++;

    return 1;

}

int tagClass::loadR (
        char *tag,
        int maxLen,
        expStringClass *destination,
        int *numElements
        ) {

    tagName[numTags] = tag;
    tagDestination[numTags] = (void *) destination;
    tagDestMaxSize[numTags] = maxLen;
    tagDestType[numTags] = tagClass::EXP_STRING_ARRAY;
    *numElements = 0;
    tagDestNumElements[numTags] = numElements;

    if ( numTags < MAX ) numTags++;

    return 1;

}

int tagClass::loadR (
        char *tag,
        int maxLen,
        expStringClass *destination,
        int *numElements,
        char *oneDefault
        ) {

    int i;

    for ( i=0; i<maxLen; i++ ) {
        destination[i].setRaw( oneDefault );
    }

    tagName[numTags] = tag;
    tagDestination[numTags] = (void *) destination;
    tagDestMaxSize[numTags] = maxLen;
    tagDestType[numTags] = tagClass::EXP_STRING_ARRAY;
    *numElements = 0;
    tagDestNumElements[numTags] = numElements;

    if ( numTags < MAX ) numTags++;

    return 1;

}

int tagClass::loadR (
        char *tag,
        int maxLen,
        int *destination,
        int *numElements
        ) {

    tagName[numTags] = tag;
    tagDestination[numTags] = (void *) destination;
    tagDestMaxSize[numTags] = maxLen;
    tagDestType[numTags] = tagClass::INTEGER_ARRAY;
    *numElements = 0;
    tagDestNumElements[numTags] = numElements;

    if ( numTags < MAX ) numTags++;

    return 1;

}

int tagClass::loadR (
        char *tag,
        int maxLen,
        int *destination,
        int *numElements,
        int *oneDefault
        ) {

    int i;

    for ( i=0; i<maxLen; i++ ) {
        destination[i] = *oneDefault;
    }

    tagName[numTags] = tag;
    tagDestination[numTags] = (void *) destination;
    tagDestMaxSize[numTags] = maxLen;
    tagDestType[numTags] = tagClass::INTEGER_ARRAY;
    *numElements = 0;
    tagDestNumElements[numTags] = numElements;

    if ( numTags < MAX ) numTags++;

    return 1;

}

int tagClass::loadR (
        char *tag,
        int maxLen,
        unsigned int *destination,
        int *numElements
        ) {

    tagName[numTags] = tag;
    tagDestination[numTags] = (void *) destination;
    tagDestMaxSize[numTags] = maxLen;
    tagDestType[numTags] = tagClass::UNSIGNED_INTEGER_ARRAY;
    *numElements = 0;
    tagDestNumElements[numTags] = numElements;

    if ( numTags < MAX ) numTags++;

    return 1;

}

int tagClass::loadR (
        char *tag,
        int maxLen,
        unsigned int *destination,
        int *numElements,
        unsigned int *oneDefault
        ) {

    int i;

    for ( i=0; i<maxLen; i++ ) {
        destination[i] = *oneDefault;
    }

    tagName[numTags] = tag;
    tagDestination[numTags] = (void *) destination;
    tagDestMaxSize[numTags] = maxLen;
    tagDestType[numTags] = tagClass::UNSIGNED_INTEGER_ARRAY;
    *numElements = 0;
    tagDestNumElements[numTags] = numElements;

    if ( numTags < MAX ) numTags++;

    return 1;

}

int tagClass::loadR ( // dynamic integer array
                      char *tag,
                      int **destination,
                      int *numElements
                      ) {

    tagName[numTags] = tag;
    tagDestination[numTags] = (void *) destination;
    tagDestMaxSize[numTags] = 0;
    tagDestType[numTags] = tagClass::DYNAMIC_INTEGER_ARRAY;
    *numElements = 0;
    tagDestNumElements[numTags] = numElements;

    if ( numTags < MAX ) numTags++;

    return 1;

}

int tagClass::loadR (
        char *tag,
        int maxLen,
        double *destination,
        int *numElements
        ) {

    tagName[numTags] = tag;
    tagDestination[numTags] = (void *) destination;
    tagDestMaxSize[numTags] = maxLen;
    tagDestType[numTags] = tagClass::REAL_ARRAY;
    *numElements = 0;
    tagDestNumElements[numTags] = numElements;

    if ( numTags < MAX ) numTags++;

    return 1;

}

int tagClass::loadR (
        char *tag,
        int maxLen,
        double *destination,
        int *numElements,
        double *oneDefault
        ) {

    int i;

    for ( i=0; i<maxLen; i++ ) {
        destination[i] = *oneDefault;
    }

    tagName[numTags] = tag;
    tagDestination[numTags] = (void *) destination;
    tagDestMaxSize[numTags] = maxLen;
    tagDestType[numTags] = tagClass::REAL_ARRAY;
    *numElements = 0;
    tagDestNumElements[numTags] = numElements;

    if ( numTags < MAX ) numTags++;

    return 1;

}

int tagClass::loadR ( // dynamic double array
                      char *tag,
                      double **destination,
                      int *numElements
                      ) {

    tagName[numTags] = tag;
    tagDestination[numTags] = (void *) destination;
    tagDestMaxSize[numTags] = 0;
    tagDestType[numTags] = tagClass::DYNAMIC_REAL_ARRAY;
    *numElements = 0;
    tagDestNumElements[numTags] = numElements;

    if ( numTags < MAX ) numTags++;

    return 1;

}

int tagClass::loadR (
        unknownTagList &unknownTags
        ) {

    this->unknownTags = &unknownTags;

    return 1;

}

void tagClass::writeMultiLineString (
        FILE *f,
        char *s
        ) {

    char *p = s;

    //fprintf( f, "%s\n", s );
    //return;

    while ( *p ) {

        fprintf( f, "  \"" );

        while ( *p && ( *p != '\n' ) ) {

            if ( *p == '\\' ) {
                fprintf( f, "\\\\" );
            }
            else if ( *p == '}' ) {
                fprintf( f, "\\}" );
            }
            else if ( *p == '{' ) {
                fprintf( f, "\\{" );
            }
            else if ( *p == '\"' ) {
                fprintf( f, "\\\"" );
            }
            else {
                fprintf( f, "%c", *p );
            }
            p++;
        }

        fprintf( f, "\"\n" );

        if ( *p == '\n' ) {
            p++;
        }

    }

}

void tagClass::writeString (
        FILE *f,
        char *s
        ) {

    char *p = s;

    fprintf( f, "\"" );

    while ( *p && ( *p != '\n' ) ) {

        if ( *p == '\\' ) {
            fprintf( f, "\\\\" );
        }
        else if ( *p == '}' ) {
            fprintf( f, "\\}" );
        }
        else if ( *p == '{' ) {
            fprintf( f, "\\{" );
        }
        else if ( *p == '\"' ) {
            fprintf( f, "\\\"" );
        }
        else {
            fprintf( f, "%c", *p );
        }

        p++;

    }

    fprintf( f, "\"\n" );

}

int tagClass::decode (
        char *tag,
        char *val,
        int valueIsCompound
        ) {

    int i, ii, n, index, r=0, g=0, b=0, l, colorIndex, *intArray, oneIndex,
            foundValue, max, ofs;
    unsigned int *uintArray;
    double *doubleArray;
    char tmp[MAXBUF+1], *tk, *context, *s;
    expStringClass *expStr;
    //pvColorClass *color;
    efDouble *efD;
    efInt *efI;
    int smallIntArray[100];
    double smallDoubleArray[100];

    index = -1;
    for ( i=0; i<numTags; i++ ) {
        if ( strcmp( tag, tagName[i] ) == 0 ) {
            index = i;
            break;
        }
    }

    if ( index == -1 ) {

        if ( tag[0] != UNKNOWN_PREFIX ) {
            fprintf( stderr, tagClass_str3, tag, line(), filename() );
            return 3;
        }

        if ( unknownTags ) {
            unknownTags->push_back( unknownTag( tag, val, valueIsCompound ) );
            return 1;
        }
        else {
            fprintf( stderr, tagClass_str3, tag, line(), filename() );
            return 3;
        }

    }

    switch ( tagDestType[index] ) {

    case tagClass::LABEL: // section header label - no-op

        break;

    case tagClass::COLOR: // color index

        if ( valueIsCompound ) {
            fprintf( stderr, tagClass_str4, tag );
            return 0;
        }

        // we might have the following forms in value:
        //
        //   o index <integer> (value = this color index)
        //
        //   o rbg <integer> <integer> <integer> (value = corresponding
        //                                        color index)

        context = NULL;
        tk = strtok_r( val, " \t\n", &context );

        if ( strcmp( tk, "index" ) == 0 ) { // color index

            tk = strtok_r( NULL, " \t\n", &context );
            *( (int *) tagDestination[index] ) = strtol( tk, NULL, 0 );

            //ci->warnIfBadIndex( *( (int *) tagDestination[index] ), line() );

        }
        else if ( strcmp( tk, "rgb" ) == 0 ) { // color

            tk = strtok_r( NULL, " \t\n", &context );
            r = strtol( tk, NULL, 0 );

            tk = strtok_r( NULL, " \t\n", &context );
            g = strtol( tk, NULL, 0 );

            tk = strtok_r( NULL, " \t\n", &context );
            b = strtol( tk, NULL, 0 );

            //ci->setRGB( r, g, b, &pixel );
            //*( (int *) tagDestination[index] ) = ci->pixIndex( pixel );

        }
        else {

            fprintf( stderr, tagClass_str5 );
            return 0;

        }

        break;

    case tagClass::COLOR_ARRAY: // color index array

        max = -1;

        if ( !valueIsCompound ) {
            fprintf( stderr, tagClass_str6, tag );
            return 0;
        }

        // we might have the following forms in value:
        //
        //   o <integer> index <integer> (value = this color index)
        //
        //   o <integer> rbg <integer> <integer> <integer> (value = corresponding
        //                                                  color index)

        i = 0;

        intArray = (int *) tagDestination[index];

        context = NULL;
        tk = strtok_r( val, " \t\n", &context );

        while ( tk && ( i < tagDestMaxSize[index] ) ) {

            oneIndex = strtol( tk, NULL, 0 );
            if ( ( oneIndex < 0 ) || ( oneIndex >= tagDestMaxSize[index] ) ) {
                fprintf( stderr, tagClass_str14, tagName[index], line(), filename() );
                return 0;
            }
            if ( oneIndex > max ) max = oneIndex;

            tk = strtok_r( NULL, " \t\n", &context );
            if ( tk ) {

                if ( strcmp( tk, "index" ) == 0 ) { // color index

                    tk = strtok_r( NULL, " \t\n", &context );
                    if ( tk ) {
                        intArray[oneIndex] = strtol( tk, NULL, 0 );
                        //ci->warnIfBadIndex( intArray[oneIndex], line() );
                    }
                    else {
                        fprintf( stderr, tagClass_str24, tagName[index], line(), filename() );
                    }

                }
                else if ( strcmp( tk, "rgb" ) == 0 ) { // color

                    tk = strtok_r( NULL, " \t\n", &context );
                    if ( tk ) {
                        r = strtol( tk, NULL, 0 );
                    }
                    else {
                        fprintf( stderr, tagClass_str24, tagName[index], line(), filename() );
                    }

                    tk = strtok_r( NULL, " \t\n", &context );
                    if ( tk ) {
                        g = strtol( tk, NULL, 0 );
                    }
                    else {
                        fprintf( stderr, tagClass_str24, tagName[index], line(), filename() );
                    }

                    tk = strtok_r( NULL, " \t\n", &context );
                    if ( tk ) {
                        b = strtol( tk, NULL, 0 );
                    }
                    else {
                        fprintf( stderr, tagClass_str24, tagName[index], line(), filename() );
                    }

                    //ci->setRGB( r, g, b, &pixel );
                    //intArray[oneIndex] = ci->pixIndex( pixel );

                }
                else {

                    fprintf( stderr, tagClass_str7, tagName[index], line(), filename() );
                    return 0;

                }

            }
            else {

                fprintf( stderr, tagClass_str24, tagName[index], line(), filename() );

            }

            i++;

            tk = strtok_r( NULL, " \t\n", &context );

        }

        *tagDestNumElements[index] = max+1;

        break;

    case tagClass::PV_COLOR: // pvColorClass color

        if ( valueIsCompound ) {
            fprintf( stderr, tagClass_str4, tag );
            return 0;
        }

        // we might have the following forms in value:
        //
        //   o index <integer> (value = this color index)
        //
        //   o rbg <integer> <integer> <integer> (value = corresponding
        //                                        color index)

        //color = (pvColorClass *) tagDestination[index];

        context = NULL;
        tk = strtok_r( val, " \t\n", &context );

        if ( strcmp( tk, "index" ) == 0 ) { // color index

            tk = strtok_r( NULL, " \t\n", &context );
            colorIndex = strtol( tk, NULL, 0 );
            //ci->warnIfBadIndex( colorIndex, line() );
            //color->setColorIndex( colorIndex, ci );

        }
        else if ( strcmp( tk, "rgb" ) == 0 ) { // color

            tk = strtok_r( NULL, " \t\n", &context );
            r = strtol( tk, NULL, 0 );

            tk = strtok_r( NULL, " \t\n", &context );
            g = strtol( tk, NULL, 0 );

            tk = strtok_r( NULL, " \t\n", &context );
            b = strtol( tk, NULL, 0 );

            //ci->setRGB( r, g, b, &pixel );
            //color->setColorIndex( ci->pixIndex(pixel), ci );

        }
        else {

            fprintf( stderr, tagClass_str5 );
            return 0;

        }

        break;

    case tagClass::PV_COLOR_ARRAY: // pvColorClass array color

        max = -1;

        if ( !valueIsCompound ) {
            fprintf( stderr, tagClass_str6, tag );
            return 0;
        }

        // we might have the following forms in value:
        //
        //   o <integer> index <integer> (value = this color index)
        //
        //   o <integer> rbg <integer> <integer> <integer> (value = corresponding
        //                                        color index)

        i = 0;

        //color = (pvColorClass *) tagDestination[index];

        context = NULL;
        tk = strtok_r( val, " \t\n", &context );

        while ( tk && ( i < tagDestMaxSize[index] ) ) {

            oneIndex = strtol( tk, NULL, 0 );
            if ( ( oneIndex < 0 ) || ( oneIndex >= tagDestMaxSize[index] ) ) {
                fprintf( stderr, tagClass_str14, tagName[index], line(), filename() );
                return 0;
            }
            if ( oneIndex > max ) max = oneIndex;

            tk = strtok_r( NULL, " \t\n", &context );
            if ( tk ) {

                if ( strcmp( tk, "index" ) == 0 ) { // color index

                    tk = strtok_r( NULL, " \t\n", &context );
                    if ( tk ) {
                        colorIndex = strtol( tk, NULL, 0 );
                        //ci->warnIfBadIndex( colorIndex, line() );
                        //color[oneIndex].setColorIndex( colorIndex, ci );
                    }
                    else {
                        fprintf( stderr, tagClass_str24, tagName[index], line(), filename() );
                    }

                }
                else if ( strcmp( tk, "rgb" ) == 0 ) { // color

                    tk = strtok_r( NULL, " \t\n", &context );
                    if ( tk ) {
                        r = strtol( tk, NULL, 0 );
                    }
                    else {
                        fprintf( stderr, tagClass_str24, tagName[index], line(), filename() );
                    }

                    tk = strtok_r( NULL, " \t\n", &context );
                    if ( tk ) {
                        g = strtol( tk, NULL, 0 );
                    }
                    else {
                        fprintf( stderr, tagClass_str24, tagName[index], line(), filename() );
                    }

                    tk = strtok_r( NULL, " \t\n", &context );
                    if ( tk ) {
                        b = strtol( tk, NULL, 0 );
                    }
                    else {
                        fprintf( stderr, tagClass_str24, tagName[index], line(), filename() );
                    }

                    //ci->setRGB( r, g, b, &pixel );
                    //color[oneIndex].setColorIndex( ci->pixIndex(pixel), ci );

                }
                else {

                    fprintf( stderr, tagClass_str7, tagName[index], line(), filename() );
                    return 0;

                }

            }
            else {

                fprintf( stderr, tagClass_str24, tagName[index], line(), filename() );

            }

            i++;

            tk = strtok_r( NULL, " \t\n", &context );

        }

        *tagDestNumElements[index] = max+1;

        break;

    case tagClass::INTEGER:

        // we might have the following forms in value:
        //
        //   o <NULL> (value = 1, this means we saw the tag)
        //
        //   o <integer> (value = this integer)
        //

        if ( valueIsCompound ) {
            fprintf( stderr, tagClass_str8, tag );
            return 0;
        }

        context = NULL;
        tk = strtok_r( val, " \t\n", &context );

        if ( !tk ) {

            *( (int *) tagDestination[index] ) = 1;

        }
        else {

            *( (int *) tagDestination[index] ) = strtol( tk, NULL, 0 );

        }

        break;

    case tagClass::UNSIGNED_INTEGER:

        // we might have the following forms in value:
        //
        //   o <NULL> (value = 1, this means we saw the tag)
        //
        //   o <integer> (value = this integer)
        //

        if ( valueIsCompound ) {
            fprintf( stderr, tagClass_str9, tag );
            return 0;
        }

        context = NULL;
        tk = strtok_r( val, " \t\n", &context );

        if ( !tk ) {

            *( (unsigned int *) tagDestination[index] ) = 1;

        }
        else {

            *( (unsigned int *) tagDestination[index] ) = strtoul( tk, NULL, 0 );

        }

        break;

    case tagClass::ENUMERATED:

        if ( valueIsCompound ) {
            fprintf( stderr, tagClass_str10, tag );
            return 0;
        }

        *( (int *) tagDestination[index] ) = -1;
        context = NULL;
        tk = strtok_r( val, " \t\n", &context );

        if ( tk ) {

            for ( i=0; i<enumNumChoices[index]; i++ ) {
                if ( strcmp( tk, enumStrArray[index][i] ) == 0 ) {
                    *( (int *) tagDestination[index] ) = enumIntArray[index][i];
                    break;
                }
            }

            if ( *( (int *) tagDestination[index] ) == -1 ) {
                fprintf( stderr, tagClass_str11, tagName[index], line(), filename() );
                return 0;
            }

        }
        else {

            fprintf( stderr, tagClass_str12, tagName[index], line(), filename() );
            return 0;

        }

        break;

    case tagClass::ENUMERATED_ARRAY:

        max = -1;

        if ( !valueIsCompound ) {
            fprintf( stderr, tagClass_str13, tag );
            return 0;
        }

        intArray = (int *) tagDestination[index];

        i = 0;

        context = NULL;
        tk = strtok_r( val, " \t\n", &context );
        if ( tk ) {

            oneIndex = strtol( tk, NULL, 0 );

            if ( ( oneIndex < 0 ) || ( oneIndex >= tagDestMaxSize[index] ) ) {
                fprintf( stderr, tagClass_str14, tagName[index], line(), filename() );
                return 0;
            }

            if ( oneIndex > max ) max = oneIndex;

            tk = strtok_r( NULL, "\n", &context );

        }

        while ( tk && ( i < tagDestMaxSize[index] ) ) {

            foundValue = 0;
            for ( ii=0; ii<enumNumChoices[index]; ii++ ) {
                if ( strcmp( tk, enumStrArray[index][ii] ) == 0 ) {
                    intArray[oneIndex] = enumIntArray[index][ii];
                    foundValue = 1;
                    break;
                }
            }

            if ( !foundValue ) {
                fprintf( stderr, tagClass_str11, tagName[index], line(), filename() );
                intArray[oneIndex] = 0;
            }

            i++;

            tk = strtok_r( NULL, " \t\n", &context );
            if ( tk ) {

                oneIndex = strtol( tk, NULL, 0 );

                if ( ( oneIndex < 0 ) || ( oneIndex >= tagDestMaxSize[index] ) ) {
                    fprintf( stderr, tagClass_str14, tagName[index], line(), filename() );
                    return 0;
                }

                if ( oneIndex > max ) max = oneIndex;

                tk = strtok_r( NULL, "\n", &context );

            }

        }

        *tagDestNumElements[index] = max+1;

        break;

    case tagClass::REAL:

        // we might have the following forms in value:
        //
        //   o <NULL> (value = 1.0, this means we saw the tag)
        //
        //   o <double> (value = this double)
        //

        if ( valueIsCompound ) {
            fprintf( stderr, tagClass_str15, tag );
            return 0;
        }

        context = NULL;
        tk = strtok_r( val, " \t\n", &context );

        if ( !tk ) {

            *( (double *) tagDestination[index] ) = 1.0;

        }
        else {

            *( (double *) tagDestination[index] ) = strtod( tk, NULL );

        }

        break;

    case tagClass::EF_DOUBLE:

        // If we have the tag then the value is not NULL

        if ( valueIsCompound ) {
            fprintf( stderr, tagClass_str15, tag );
            return 0;
        }

        context = NULL;
        tk = strtok_r( val, " \t\n", &context );

        if ( tk ) {

            efD = (efDouble *) tagDestination[index];
            efD->setValue( strtod( tk, NULL ) );
            efD->setNull( 0 ); // is not null

        }
        else {

            fprintf( stderr, tagClass_str12, tagName[index], line(), filename() );
            return 0;

        }

        break;

    case tagClass::EF_DOUBLE_ARRAY:

        // If we have the tag then the value is not NULL

        max = -1;

        if ( !valueIsCompound ) {
            fprintf( stderr, tagClass_str20, tag );
            return 0;
        }

        efD = (efDouble *) tagDestination[index];

        i = 0;

        context = NULL;
        tk = strtok_r( val, " \t\n", &context );
        if ( tk ) {

            oneIndex = strtol( tk, NULL, 0 );

            if ( ( oneIndex < 0 ) || ( oneIndex >= tagDestMaxSize[index] ) ) {
                fprintf( stderr, tagClass_str14, tagName[index], line(), filename() );
                return 0;
            }

            if ( oneIndex > max ) max = oneIndex;

            tk = strtok_r( NULL, " \t\n", &context );

        }

        while ( tk && ( i < tagDestMaxSize[index] ) ) {

            efD[oneIndex].setValue( strtod( tk, NULL ) );

            tk = strtok_r( NULL, " \t\n", &context );
            if ( tk ) {
                efD[oneIndex].setNull( strtol( tk, NULL, 0 ) );
            }

            i++;

            tk = strtok_r( NULL, " \t\n", &context );
            if ( tk ) {

                oneIndex = strtol( tk, NULL, 0 );

                if ( ( oneIndex < 0 ) || ( oneIndex >= tagDestMaxSize[index] ) ) {
                    fprintf( stderr, tagClass_str14, tagName[index], line(), filename() );
                    return 0;
                }

                if ( oneIndex > max ) max = oneIndex;

                tk = strtok_r( NULL, " \t\n", &context );

            }

        }

        *tagDestNumElements[index] = max+1;

        break;

    case tagClass::EF_INT:

        // If we have the tag then the value is not NULL

        if ( valueIsCompound ) {
            fprintf( stderr, tagClass_str8, tag );
            return 0;
        }

        context = NULL;
        tk = strtok_r( val, " \t\n", &context );

        if ( tk ) {

            efI = (efInt *) tagDestination[index];
            efI->setValue( strtol( tk, NULL, 0 ) );
            efI->setNull( 0 ); // is not null

        }
        else {

            fprintf( stderr, tagClass_str12, tagName[index], line(), filename() );
            return 0;

        }

        break;

    case tagClass::EF_INT_ARRAY:

        // If we have the tag then the value is not NULL

        max = -1;

        if ( !valueIsCompound ) {
            fprintf( stderr, tagClass_str18, tag );
            return 0;
        }

        efI = (efInt *) tagDestination[index];

        i = 0;

        context = NULL;
        tk = strtok_r( val, " \t\n", &context );
        if ( tk ) {

            oneIndex = strtol( tk, NULL, 0 );

            if ( ( oneIndex < 0 ) || ( oneIndex >= tagDestMaxSize[index] ) ) {
                fprintf( stderr, tagClass_str14, tagName[index], line(), filename() );
                return 0;
            }

            if ( oneIndex > max ) max = oneIndex;

            tk = strtok_r( NULL, " \t\n", &context );

        }

        while ( tk && ( i < tagDestMaxSize[index] ) ) {

            efI[oneIndex].setValue( strtol( tk, NULL, 0 ) );

            efI[oneIndex].setNull( 0 ); // is not null

            //tk = strtok_r( NULL, " \t\n", &context );
            //if ( tk ) {
            //  efI[oneIndex].setNull( 0 ); // is not null
            //}

            i++;

            tk = strtok_r( NULL, " \t\n", &context );
            if ( tk ) {

                oneIndex = strtol( tk, NULL, 0 );

                if ( ( oneIndex < 0 ) || ( oneIndex >= tagDestMaxSize[index] ) ) {
                    fprintf( stderr, tagClass_str14, tagName[index], line(), filename() );
                    return 0;
                }

                if ( oneIndex > max ) max = oneIndex;

                tk = strtok_r( NULL, " \t\n", &context );

            }

        }

        *tagDestNumElements[index] = max+1;

        break;

    case tagClass::STRING: // string

        s = (char *) tagDestination[index];

        strncpy( s, val, tagDestMaxSize[index] );
        s[tagDestMaxSize[index]] = 0;

        l = strlen( s );
        if ( l ) {
            if ( s[l-1] == '\n' ) {
                s[l-1] = 0;
            }
        }

        break;

    case tagClass::STRING_ARRAY:

        // enumNumChoices[index] contains the length of the string

        max = -1;

        if ( !valueIsCompound ) {
            fprintf( stderr, tagClass_str16, tag );
            return 0;
        }

        s = (char *) tagDestination[index];

        i = 0;

        context = NULL;
        tk = strtok_r( val, " \t\n", &context );
        if ( tk ) {

            oneIndex = strtol( tk, NULL, 0 );

            if ( ( oneIndex < 0 ) || ( oneIndex >= tagDestMaxSize[index] ) ) {
                fprintf( stderr, tagClass_str14, tagName[index], line(), filename() );
                return 0;
            }

            if ( oneIndex > max ) max = oneIndex;

            tk = strtok_r( NULL, "\n", &context );

        }

        while ( tk && ( i < tagDestMaxSize[index] ) ) {

            ofs = oneIndex * enumNumChoices[index];

            if ( strcmp( tk, "\'\'" ) == 0 ) {
                strcpy( (char *) ( s + ofs ), "" );
            }
            else {
                strncpy( (char *) ( s + ofs ), tk, enumNumChoices[index]-1 );
                ((char *) ( s + ofs ))[enumNumChoices[index]] = 0;
            }

            i++;

            tk = strtok_r( NULL, " \t\n", &context );
            if ( tk ) {

                oneIndex = strtol( tk, NULL, 0 );

                if ( ( oneIndex < 0 ) || ( oneIndex >= tagDestMaxSize[index] ) ) {
                    fprintf( stderr, tagClass_str14, tagName[index], line(), filename() );
                    return 0;
                }

                if ( oneIndex > max ) max = oneIndex;

                tk = strtok_r( NULL, "\n", &context );

            }

        }

        *tagDestNumElements[index] = max+1;

        break;

    case tagClass::DYNAMIC_STRING:

        l = strlen( val );
        s = new char[l+1];
        //fprintf( stderr, "allocate string, l = %-d\n", l+1 );
        if ( !s ) {
            fprintf( stderr, tagClass_str17, __LINE__, __FILE__ );
            return 0;
        }

        strcpy( s, val );

        if ( l ) {
            if ( s[l-1] == '\n' ) {
                s[l-1] = 0;
            }
        }

        *( (char **) tagDestination[index] ) = s;

        break;

    case tagClass::EXP_STRING: // expStringClass

        expStr = (expStringClass *) tagDestination[index];

        l = strlen( val );
        if ( l ) {
            if ( val[l-1] == '\n' ) {
                val[l-1] = 0;
            }
        }

        expStr->setRaw( val );

        break;

    case tagClass::EXP_STRING_ARRAY:

        max = -1;

        if ( !valueIsCompound ) {
            fprintf( stderr, tagClass_str16, tag );
            return 0;
        }

        expStr = (expStringClass *) tagDestination[index];

        i = 0;

        context = NULL;
        tk = strtok_r( val, " \t\n", &context );
        if ( tk ) {

            oneIndex = strtol( tk, NULL, 0 );

            if ( ( oneIndex < 0 ) || ( oneIndex >= tagDestMaxSize[index] ) ) {
                fprintf( stderr, tagClass_str14, tagName[index], line(), filename() );
                return 0;
            }

            if ( oneIndex > max ) max = oneIndex;

            tk = strtok_r( NULL, "\n", &context );

        }

        while ( tk && ( i < tagDestMaxSize[index] ) ) {

            if ( strcmp( tk, "\'\'" ) == 0 ) {
                expStr[oneIndex].setRaw( "" );
            }
            else {
                expStr[oneIndex].setRaw( tk );
            }

            i++;

            tk = strtok_r( NULL, " \t\n", &context );
            if ( tk ) {

                oneIndex = strtol( tk, NULL, 0 );

                if ( ( oneIndex < 0 ) || ( oneIndex >= tagDestMaxSize[index] ) ) {
                    fprintf( stderr, tagClass_str14, tagName[index], line(), filename() );
                    return 0;
                }

                if ( oneIndex > max ) max = oneIndex;

                tk = strtok_r( NULL, "\n", &context );

            }

        }

        *tagDestNumElements[index] = max+1;

        break;

    case tagClass::INTEGER_ARRAY:

        max = -1;

        if ( !valueIsCompound ) {
            fprintf( stderr, tagClass_str18, tag );
            return 0;
        }

        intArray = (int *) tagDestination[index];

        i = 0;

        context = NULL;
        tk = strtok_r( val, " \t\n", &context );
        if ( tk ) {

            oneIndex = strtol( tk, NULL, 0 );

            if ( ( oneIndex < 0 ) || ( oneIndex >= tagDestMaxSize[index] ) ) {
                fprintf( stderr, tagClass_str14, tagName[index], line(), filename() );
                return 0;
            }

            if ( oneIndex > max ) max = oneIndex;

            tk = strtok_r( NULL, " \t\n", &context );

        }

        while ( tk && ( i < tagDestMaxSize[index] ) ) {

            intArray[oneIndex] = strtol( tk, NULL, 0 );

            i++;

            tk = strtok_r( NULL, " \t\n", &context );
            if ( tk ) {

                oneIndex = strtol( tk, NULL, 0 );

                if ( ( oneIndex < 0 ) || ( oneIndex >= tagDestMaxSize[index] ) ) {
                    fprintf( stderr, tagClass_str14, tagName[index], line(), filename() );
                    return 0;
                }

                if ( oneIndex > max ) max = oneIndex;

                tk = strtok_r( NULL, " \t\n", &context );

            }

        }

        *tagDestNumElements[index] = max+1;

        break;

    case tagClass::UNSIGNED_INTEGER_ARRAY:

        max = -1;

        if ( !valueIsCompound ) {
            fprintf( stderr, tagClass_str19, tag );
            return 0;
        }

        uintArray = (unsigned int *) tagDestination[index];

        i = 0;

        context = NULL;
        tk = strtok_r( val, " \t\n", &context );
        if ( tk ) {

            oneIndex = strtol( tk, NULL, 0 );

            if ( ( oneIndex < 0 ) || ( oneIndex >= tagDestMaxSize[index] ) ) {
                fprintf( stderr, tagClass_str14, tagName[index], line(), filename() );
                return 0;
            }

            if ( oneIndex > max ) max = oneIndex;

            tk = strtok_r( NULL, " \t\n", &context );

        }

        while ( tk && ( i < tagDestMaxSize[index] ) ) {

            uintArray[oneIndex] = (unsigned int) strtoul( tk, NULL, 0 );

            i++;

            tk = strtok_r( NULL, " \t\n", &context );
            if ( tk ) {

                oneIndex = strtol( tk, NULL, 0 );

                if ( ( oneIndex < 0 ) || ( oneIndex >= tagDestMaxSize[index] ) ) {
                    fprintf( stderr, tagClass_str14, tagName[index], line(), filename() );
                    return 0;
                }

                if ( oneIndex > max ) max = oneIndex;

                tk = strtok_r( NULL, " \t\n", &context );

            }

        }

        *tagDestNumElements[index] = max+1;

        break;

    case tagClass::REAL_ARRAY:

        max = -1;

        if ( !valueIsCompound ) {
            fprintf( stderr, tagClass_str20, tag );
            return 0;
        }

        doubleArray = (double *) tagDestination[index];

        i = 0;

        context = NULL;
        tk = strtok_r( val, " \t\n", &context );
        if ( tk ) {

            oneIndex = strtol( tk, NULL, 0 );

            if ( ( oneIndex < 0 ) || ( oneIndex >= tagDestMaxSize[index] ) ) {
                fprintf( stderr, tagClass_str14, tagName[index], line(), filename() );
                return 0;
            }

            if ( oneIndex > max ) max = oneIndex;

            tk = strtok_r( NULL, " \t\n", &context );

        }

        while ( tk && ( i < tagDestMaxSize[index] ) ) {

            doubleArray[oneIndex] = strtod( tk, NULL );

            i++;

            tk = strtok_r( NULL, " \t\n", &context );
            if ( tk ) {

                oneIndex = strtol( tk, NULL, 0 );

                if ( ( oneIndex < 0 ) || ( oneIndex >= tagDestMaxSize[index] ) ) {
                    fprintf( stderr, tagClass_str14, tagName[index], line(), filename() );
                    return 0;
                }

                if ( oneIndex > max ) max = oneIndex;

                tk = strtok_r( NULL, " \t\n", &context );

            }

        }

        *tagDestNumElements[index] = max+1;

        break;

    case tagClass::DYNAMIC_INTEGER_ARRAY:

        if ( !valueIsCompound ) {
            fprintf( stderr, tagClass_str18, tag );
            return 0;
        }

        strncpy( tmp, val, MAXBUF );
        tmp[MAXBUF] = 0;

        // first, count tokens and allocate array (two tokens per value)

        n = 0;

        context = NULL;
        tk = strtok_r( tmp, " \t\n", &context ); // index

        if ( tk ) {
            tk = strtok_r( NULL, " \t\n", &context ); // value
        }

        while ( tk ) {

            if ( n < 100 ) smallIntArray[n] = strtol( tk, NULL, 0 );

            n++;

            tk = strtok_r( NULL, " \t\n", &context ); // index

            if ( tk ) {
                tk = strtok_r( NULL, " \t\n", &context ); // value
            }

        }

        *tagDestNumElements[index] = n;

        intArray = new int[n+1];

        if ( n <= 100 ) {

            for ( i=0; i<n; i++ ) {
                intArray[i] = smallIntArray[i];
            }

        }
        else {

            // not a small array, so copy val into tmp again and extract values

            strncpy( tmp, val, MAXBUF );
            tmp[MAXBUF] = 0;

            n = 0;

            context = NULL;
            tk = strtok_r( tmp, " \t\n", &context ); // index

            if ( tk ) {
                tk = strtok_r( NULL, " \t\n", &context ); // value
            }

            while ( tk ) {

                intArray[n++] = strtol( tk, NULL, 0 );

                tk = strtok_r( NULL, " \t\n", &context ); // index

                if ( tk ) {
                    tk = strtok_r( NULL, " \t\n", &context ); // value
                }

            }

        }

        *( (int **) tagDestination[index] ) = intArray;

        break;

    case tagClass::DYNAMIC_REAL_ARRAY:

        if ( !valueIsCompound ) {
            fprintf( stderr, tagClass_str20, tag );
            return 0;
        }

        strncpy( tmp, val, MAXBUF );
        tmp[MAXBUF] = 0;

        // first, count tokens and allocate array (two tokens per value)

        n = 0;

        context = NULL;
        tk = strtok_r( tmp, " \t\n", &context ); // index

        if ( tk ) {
            tk = strtok_r( NULL, " \t\n", &context ); // value
        }

        while ( tk ) {

            if ( n < 100 ) smallDoubleArray[n] = strtod( tk, NULL );

            n++;

            tk = strtok_r( NULL, " \t\n", &context ); // index

            if ( tk ) {
                tk = strtok_r( NULL, " \t\n", &context ); // value
            }

        }

        *tagDestNumElements[index] = n;

        doubleArray = new double[n+1];

        if ( n <= 100 ) {

            for ( i=0; i<n; i++ ) {
                doubleArray[i] = smallDoubleArray[i];
            }

        }
        else {

            // not a small array, so copy val into tmp again and extract values

            strncpy( tmp, val, MAXBUF );
            tmp[MAXBUF] = 0;

            n = 0;

            context = NULL;
            tk = strtok_r( tmp, " \t\n", &context ); // index

            if ( tk ) {
                tk = strtok_r( NULL, " \t\n", &context ); // value
            }

            while ( tk ) {

                doubleArray[n++] = strtod( tk, NULL );

                tk = strtok_r( NULL, " \t\n", &context ); // index

                if ( tk ) {
                    tk = strtok_r( NULL, " \t\n", &context ); // value
                }

            }

        }

        *( (double **) tagDestination[index] ) = doubleArray;

        break;

    }

    return 1;

}


int tagClass::readTags (FILE *f, char *endingTag)
{

    char *gotOne, tagName[255+1], val[10000+1], *valp;
    int isCompound, overFlow, maxLen;

    gotOne = getName( tagName, 255, f );
    if ( !gotOne ) {
        strcpy( msg, tagClass_str21 );
        return TAG_E_NOTAGS;
    }

    while ( gotOne ) {

        //fprintf( stderr, "tagName = [%s]\n", tagName );

        if ( strcmp( tagName, endingTag ) ) {

            maxLen = 10000;
            valp = val;
            getValue( &valp, &maxLen, f, &isCompound, &overFlow );

            decode( tagName, valp, isCompound );

            if ( overFlow ) {
                if ( valp ) delete[] valp;
                valp = NULL;
            }

            gotOne = getName( tagName, 255, f );

        }
        else {

            gotOne = NULL;

        }

    }

    return 1;

}

int tagClass::writeTags (
        FILE *f
        ) {

    int i, ii, index, colorIndex, *colorIndexArray, doWrite, tagDefaultValue;
    int *iArray;
    unsigned int *uiArray;
    double *dArray;
    char *s, *sDef, *enumString;
    expStringClass *expStr;
    //pvColorClass *color;
    efDouble *efD=NULL;
    efInt *efI;

    for ( index=0; index<numTags; index++ ) {

        switch ( tagDestType[index] ) {

        case tagClass::LABEL: // section header label

            fprintf( f, "%s\n", tagName[index] );

            if ( tagClass::genDoc() ) {
                fprintf( stderr, "%s\n", tagName[index] );
            }

            break;

        case tagClass::COLOR: // color

            // use writeColorIndex in color_pkg.cc
            //
            // Two forms are possible:
            //
            //   o index <integer> (value = this color index)
            //
            //   o rbg <integer> <integer> <integer> (value = corresponding
            //                                        color index)

            colorIndex = *( (int *) tagDestination[index] );
            //ci->writeColorIndex( f, tagName[index], colorIndex );

            if ( tagClass::genDoc() ) {
                fprintf( stderr, "%s (index <int> | rgb <int> <int> <int>)\n",
                         tagName[index] );
            }

            break;

        case tagClass::PV_COLOR: // pvColorClass color

            // use writeColorIndex in color_pkg.cc
            //
            // Two forms are possible:
            //
            //   o index <integer> (value = this color index)
            //
            //   o rbg <integer> <integer> <integer> (value = corresponding
            //                                        color index)

            //color = (pvColorClass *) tagDestination[index];
            //colorIndex = color->pixelIndex();
            //ci->writeColorIndex( f, tagName[index], colorIndex );

            if ( tagClass::genDoc() ) {
                fprintf( stderr, "%s (index <int> | rgb <int> <int> <int>)\n",
                         tagName[index] );
            }

            break;

        case tagClass::COLOR_ARRAY: // color

            // use writeColorIndex in color_pkg.cc
            //
            // Two forms are possible:
            //
            //   o index <integer> (value = this color index)
            //
            //   o rbg <integer> <integer> <integer> (value = corresponding
            //                                        color index)

            if ( tagDestination[index] ) {

                fprintf( f, "%s {\n", tagName[index] );

                colorIndexArray = (int *) tagDestination[index];

                for ( i=0; i<tagDestMaxSize[index]; i++ ) {
                    fprintf( f, "  " );
                    //ci->writeColorArrayIndex( f, i, colorIndexArray[i] );
                }

                fprintf( f, "}\n" );

            }

            if ( tagClass::genDoc() ) {
                fprintf( stderr, "%s {\n", tagName[index] );
                fprintf( stderr, "  <int element> (index <int> | rgb <int> <int> <int>)\n" );
                fprintf( stderr, "  <int element> (index <int> | rgb <int> <int> <int>)\n" );
                fprintf( stderr, "                .\n" );
                fprintf( stderr, "                .\n" );
                fprintf( stderr, "}\n" );
            }

            break;

        case tagClass::PV_COLOR_ARRAY: // pvColorClass color

            // use writeColorIndex in color_pkg.cc
            //
            // Two forms are possible:
            //
            //   o index <integer> (value = this color index)
            //
            //   o rbg <integer> <integer> <integer> (value = corresponding
            //                                        color index)

            if ( tagDestination[index] ) {

                fprintf( f, "%s {\n", tagName[index] );

                //color = (pvColorClass *) tagDestination[index];

                for ( i=0; i<tagDestMaxSize[index]; i++ ) {
                    fprintf( f, "  " );
                    //colorIndex = color[i].pixelIndex();
                    //ci->writeColorArrayIndex( f, i, colorIndex );
                }

                fprintf( f, "}\n" );

            }

            if ( tagClass::genDoc() ) {
                fprintf( stderr, "%s {\n", tagName[index] );
                fprintf( stderr, "  <int element> (index <int> | rgb <int> <int> <int>)\n" );
                fprintf( stderr, "  <int element> (index <int> | rgb <int> <int> <int>)\n" );
                fprintf( stderr, "                .\n" );
                fprintf( stderr, "                .\n" );
                fprintf( stderr, "}\n" );
            }

            break;

        case tagClass::INTEGER:

            if ( tagDestination[index] ) {

                if ( tagDefault[index] ) {

                    if ( *( (int *) tagDestination[index] ) !=
                         *( (int *) tagDefault[index] ) ) {

                        fprintf( f, "%s %-d\n", tagName[index],
                                 *( (int *) tagDestination[index] ) );

                    }

                }
                else {

                    fprintf( f, "%s %-d\n", tagName[index],
                             *( (int *) tagDestination[index] ) );

                }

            }
            else {

                fprintf( f, "%-s\n", tagName[index] );

            }

            if ( tagClass::genDoc() ) {
                if ( tagDefault[index] ) {
                    fprintf( stderr, "[%s <int>]   /* default = %-d */\n",
                             tagName[index], *( (int *) tagDefault[index] ) );
                }
                else {
                    fprintf( stderr, "%s <int>\n", tagName[index] );
                }
            }

            break;

        case tagClass::BOOLEAN:

            if ( tagDestination[index] ) {

                if ( tagDefault[index] ) {

                    if ( *( (int *) tagDestination[index] ) !=
                         *( (int *) tagDefault[index] ) ) {

                        if ( *( (int *) tagDestination[index] ) ) {
                            fprintf( f, "%s\n", tagName[index] );
                        }

                    }

                }
                else {

                    if ( *( (int *) tagDestination[index] ) ) {
                        fprintf( f, "%s\n", tagName[index] );
                    }

                }

            }
            else {

                fprintf( f, "%-s\n", tagName[index] );

            }

            if ( tagClass::genDoc() ) {
                if ( tagDefault[index] ) {
                    fprintf( stderr, "[%s [(0|1)]]   /* present with no value = 1, absent = 0 */\n",
                             tagName[index] );
                }
                else {
                    fprintf( stderr, "[%s [(0|1)]]   /* present with no value = 1, absent = 0 */\n",
                             tagName[index] );
                }
            }

            break;

        case tagClass::ENUMERATED:

            if ( tagDestination[index] ) {

                enumString = NULL;
                for ( i=0; i<enumNumChoices[index]; i++ ) {
                    if ( *( (int *) tagDestination[index] ) == enumIntArray[index][i] ) {
                        enumString = enumStrArray[index][i];
                        break;
                    }
                }

                if ( !enumString ) {
                    fprintf( stderr, tagClass_str22, *( (int *) tagDestination[index] ) );
                    for ( i=0; i<enumNumChoices[index]; i++ ) {
                        fprintf( stderr, "  %-d : [%s]\n", enumIntArray[index][i],
                                 enumStrArray[index][i] );
                    }
                    enumString = enumStrArray[index][0];
                }

                if ( tagDefault[index] ) {

                    if ( *( (int *) tagDestination[index] ) !=
                         *( (int *) tagDefault[index] ) ) {

                        fprintf( f, "%s ", tagName[index] );
                        writeString( f, enumString );

                    }

                }
                else {

                    fprintf( f, "%s ", tagName[index] );
                    writeString( f, enumString );

                }

            }
            else {

                fprintf( stderr, tagClass_str23, tagName[index] );
                return 0;

            }

            if ( tagClass::genDoc() ) {

                if ( tagDefault[index] ) {

                    tagDefaultValue = -1;
                    for ( i=0; i<enumNumChoices[index]; i++ ) {
                        if ( *( (int *) tagDefault[index] ) == enumIntArray[index][i] ) {
                            tagDefaultValue = i;
                            break;
                        }
                    }

                    fprintf( stderr, "[%s (", tagName[index] );
                    fprintf( stderr, "\"%s\"", enumStrArray[index][0] );
                    for ( i=1; i<enumNumChoices[index]; i++ ) {
                        fprintf( stderr, "|\"%s\"", enumStrArray[index][i] );
                    }
                    if ( tagDefaultValue == -1 ) {
                        fprintf( stderr, ")]   /* default = \"UNKNOWN\" */\n" );
                    }
                    else {
                        fprintf( stderr, ")]   /* default = \"%s\" */\n",
                                 enumStrArray[index][tagDefaultValue] );
                    }

                }
                else {

                    fprintf( stderr, "%s (", tagName[index] );
                    fprintf( stderr, "\"%s\"", enumStrArray[index][0] );
                    for ( i=1; i<enumNumChoices[index]; i++ ) {
                        fprintf( stderr, "|\"%s\"", enumStrArray[index][i] );
                    }
                    fprintf( stderr, ")\n" );

                }

            }

            break;

        case tagClass::ENUMERATED_ARRAY:

            if ( tagDestination[index] ) {

                iArray = (int *) tagDestination[index];

                doWrite = 0;
                if ( tagDefault[index] ) {

                    // see if anything to write
                    for ( ii=0; ii<tagDestMaxSize[index]; ii++ ) {
                        if ( iArray[ii] != *( (int *) tagDefault[index] ) ) {
                            doWrite = 1;
                        }
                    }

                }
                else {

                    doWrite = 1;

                }

                if ( doWrite ) {

                    fprintf( f, "%s {\n", tagName[index] );

                    for ( ii=0; ii<tagDestMaxSize[index]; ii++ ) {

                        enumString = NULL;
                        for ( i=0; i<enumNumChoices[index]; i++ ) {
                            if ( iArray[ii] == enumIntArray[index][i] ) {
                                enumString = enumStrArray[index][i];
                                break;
                            }
                        }

                        if ( !enumString ) {
                            fprintf( stderr, tagClass_str22, iArray[ii] );
                            for ( i=0; i<enumNumChoices[index]; i++ ) {
                                fprintf( stderr, "  %-d : [%s]\n", enumIntArray[index][i],
                                         enumStrArray[index][i] );
                            }
                            enumString = enumStrArray[index][0];
                        }

                        if ( tagDefault[index] ) {

                            if ( iArray[ii] != *( (int *) tagDefault[index] ) ) {
                                fprintf( f, "  %-d ", ii );
                                writeString( f, enumString );
                            }

                        }
                        else {

                            fprintf( f, "  %-d ", ii );
                            writeString( f, enumString );

                        }

                    }

                    fprintf( f, "}\n" );

                }

            }
            else {

                fprintf( stderr, tagClass_str23, tagName[index] );
                return 0;

            }

            if ( tagClass::genDoc() ) {

                if ( tagDefault[index] ) {

                    tagDefaultValue = -1;
                    for ( i=0; i<enumNumChoices[index]; i++ ) {
                        if ( *( (int *) tagDefault[index] ) == enumIntArray[index][i] ) {
                            tagDefaultValue = i;
                            break;
                        }
                    }

                    fprintf( stderr, "[%s {\n", tagName[index] );

                    fprintf( stderr, "  [<int element> (" );
                    fprintf( stderr, "\"%s\"", enumStrArray[index][0] );
                    for ( i=1; i<enumNumChoices[index]; i++ ) {
                        fprintf( stderr, "|\"%s\"", enumStrArray[index][i] );
                    }
                    if ( tagDefaultValue == -1 ) {
                        fprintf( stderr, ")]   /* default = \"UNKNOWN\" */\n" );
                    }
                    else {
                        fprintf( stderr, ")]   /* default = \"%s\" */\n",
                                 enumStrArray[index][tagDefaultValue] );
                    }

                    fprintf( stderr, "  [<int element> (" );
                    fprintf( stderr, "\"%s\"", enumStrArray[index][0] );
                    for ( i=1; i<enumNumChoices[index]; i++ ) {
                        fprintf( stderr, "|\"%s\"", enumStrArray[index][i] );
                    }
                    if ( tagDefaultValue == -1 ) {
                        fprintf( stderr, ")]   /* default = \"UNKNOWN\" */\n" );
                    }
                    else {
                        fprintf( stderr, ")]   /* default = \"%s\" */\n",
                                 enumStrArray[index][tagDefaultValue] );
                    }

                    fprintf( stderr, "                .\n" );
                    fprintf( stderr, "                .\n" );

                    fprintf( stderr, "}]\n" );

                }
                else {

                    fprintf( stderr, "%s {\n", tagName[index] );

                    fprintf( stderr, "  <int element> (" );
                    fprintf( stderr, "\"%s\"", enumStrArray[index][0] );
                    for ( i=1; i<enumNumChoices[index]; i++ ) {
                        fprintf( stderr, "|\"%s\"", enumStrArray[index][i] );
                    }
                    fprintf( stderr, ")\n" );

                    fprintf( stderr, "  <int element> (" );
                    fprintf( stderr, "\"%s\"", enumStrArray[index][0] );
                    for ( i=1; i<enumNumChoices[index]; i++ ) {
                        fprintf( stderr, "|\"%s\"", enumStrArray[index][i] );
                    }
                    fprintf( stderr, ")\n" );

                    fprintf( stderr, "                .\n" );
                    fprintf( stderr, "                .\n" );

                    fprintf( stderr, "}\n" );

                }

            }

            break;

        case tagClass::REAL:

            if ( tagDestination[index] ) {

                if ( tagDefault[index] ) {

                    if ( *( (double *) tagDestination[index] ) !=
                         *( (double *) tagDefault[index] ) ) {

                        fprintf( f, "%s %-g\n", tagName[index],
                                 *( (double *) tagDestination[index] ) );

                    }

                }
                else {

                    fprintf( f, "%s %-g\n", tagName[index],
                             *( (double *) tagDestination[index] ) );

                }

            }
            else {

                fprintf( stderr, tagClass_str23, tagName[index] );
                return 0;

            }

            if ( tagClass::genDoc() ) {
                if ( tagDefault[index] ) {
                    fprintf( stderr, "[%s <real>]   /* default = %-g */\n",
                             tagName[index], *( (double *) tagDefault[index] ) );
                }
                else {
                    fprintf( stderr, "%s <real>\n", tagName[index] );
                }
            }

            break;

        case tagClass::EF_DOUBLE:

            if ( tagDestination[index] ) {

                efD = (efDouble *) tagDestination[index];
                if ( !efD->isNull() ) {

                    fprintf( f, "%s %-g\n", tagName[index], efD->value() );

                }

            }
            else {

                fprintf( stderr, tagClass_str23, tagName[index] );
                return 0;

            }

            if ( tagClass::genDoc() ) {
                fprintf( stderr, "[%s <real>]   /* default = 0.0 */\n",
                         tagName[index] );
            }

            break;

        case tagClass::EF_DOUBLE_ARRAY:

            if ( tagDestination[index] ) {

                efD = (efDouble *) tagDestination[index];

                doWrite = 0;
                // see if anything to write
                for ( ii=0; ii<tagDestMaxSize[index]; ii++ ) {
                    if ( !efD[ii].isNull() ) {
                        doWrite = 1;
                    }
                }

                if ( doWrite ) {

                    fprintf( f, "%s {\n", tagName[index] );

                    for ( i=0; i<tagDestMaxSize[index]; i++ ) {

                        if ( !efD[i].isNull() ) {
                            fprintf( f, "  %-d %-g 0\n", i, efD[i].value() ); // !null
                        }

                    }

                    fprintf( f, "}\n" );

                }

            }
            else {

                fprintf( stderr, tagClass_str23, tagName[index] );
                return 0;

            }

            if ( tagClass::genDoc() ) {
                fprintf( stderr, "[%s {\n", tagName[index] );
                fprintf( stderr, "[<int element> <real>]   /* default = 0.0 */\n" );
                fprintf( stderr, "[<int element> <real>]   /* default = 0.0 */\n" );
                fprintf( stderr, "                .\n" );
                fprintf( stderr, "                .\n" );
                fprintf( stderr, "}]\n" );
            }

            break;

        case tagClass::EF_INT:

            if ( tagDestination[index] ) {

                efI = (efInt *) tagDestination[index];
                if ( !efI->isNull() ) {

                    fprintf( f, "%s %-d\n", tagName[index], efI->value() );

                }

            }
            else {

                fprintf( stderr, tagClass_str23, tagName[index] );
                return 0;

            }

            if ( tagClass::genDoc() ) {
                fprintf( stderr, "[%s <int>]   /* default = 0 */\n",
                         tagName[index] );
            }

            break;

        case tagClass::EF_INT_ARRAY:

            if ( tagDestination[index] ) {

                efI = (efInt *) tagDestination[index];

                doWrite = 0;
                // see if anything to write
                for ( ii=0; ii<tagDestMaxSize[index]; ii++ ) {
                    if ( !efD[ii].isNull() ) {
                        doWrite = 1;
                    }
                }

                if ( doWrite ) {

                    fprintf( f, "%s {\n", tagName[index] );

                    for ( i=0; i<tagDestMaxSize[index]; i++ ) {

                        if ( !efI[i].isNull() ) {
                            fprintf( f, "  %-d %-d\n", i, efI[i].value() ); // !null
                        }

                    }

                    fprintf( f, "}\n" );

                }

            }
            else {

                fprintf( stderr, tagClass_str23, tagName[index] );
                return 0;

            }

            if ( tagClass::genDoc() ) {
                fprintf( stderr, "[%s {\n", tagName[index] );
                fprintf( stderr, "[<int element> <int>]   /* default = 0 */\n" );
                fprintf( stderr, "[<int element> <int>]   /* default = 0 */\n" );
                fprintf( stderr, "                .\n" );
                fprintf( stderr, "                .\n" );
                fprintf( stderr, "}]\n" );
            }

            break;

        case tagClass::COMPLEX_STRING: // string that may contain new lines and
            // other special characters

            s = (char *) tagDestination[index];
            sDef = (char *) tagDefault[index];

            if ( s ) {

                if ( sDef ) {

                    if ( strcmp( s, sDef ) != 0 ) {

                        fprintf( f, "%s {\n", tagName[index] );
                        writeMultiLineString( f, s );
                        //fprintf( f, "%s\n", s );
                        fprintf( f, "}\n" );

                    }

                }
                else {

                    fprintf( f, "%s {\n", tagName[index] );
                    writeMultiLineString( f, s );
                    //fprintf( f, "%s\n", s );
                    fprintf( f, "}\n" );

                }

            }
            else {

                fprintf( stderr, tagClass_str23, tagName[index] );
                return 0;

            }

            if ( tagClass::genDoc() ) {
                if ( sDef ) {
                    fprintf( stderr, "[%s {\n", tagName[index] );
                    fprintf( stderr, "  <string> /* line 1 of n */\n" );
                    fprintf( stderr, "  <string> /* line 2 of n */\n" );
                    fprintf( stderr, "      .\n" );
                    fprintf( stderr, "      .\n" );
                    fprintf( stderr, "}]   /* default = \"%s\" */\n", sDef );
                }
                else {
                    fprintf( stderr, "%s {\n", tagName[index] );
                    fprintf( stderr, "  <string> /* line 1 of n */\n" );
                    fprintf( stderr, "  <string> /* line 2 of n */\n" );
                    fprintf( stderr, "      .\n" );
                    fprintf( stderr, "      .\n" );
                    fprintf( stderr, "}\n" );
                }
            }

            break;

        case tagClass::STRING: // string

            s = (char *) tagDestination[index];
            sDef = (char *) tagDefault[index];

            if ( s ) {

                if ( sDef ) {

                    if ( strcmp( s, sDef ) != 0 ) {

                        fprintf( f, "%s ", tagName[index] );
                        writeString( f, s );

                    }

                }
                else {

                    fprintf( f, "%s ", tagName[index] );
                    writeString( f, s );

                }

            }
            else {

                fprintf( stderr, tagClass_str23, tagName[index] );
                return 0;

            }

            if ( tagClass::genDoc() ) {
                if ( sDef ) {
                    fprintf( stderr, "[%s <string>] /* default = \"%s\" */\n",
                             tagName[index], sDef );
                }
                else {
                    fprintf( stderr, "%s <string>\n", tagName[index] );
                }
            }

            break;

        case tagClass::STRING_ARRAY: // string 2d fixed size array

            // use enumNumChoices[index] as string size

            s = (char *) tagDestination[index];

            if ( s ) {

                doWrite = 0;
                if ( tagDefault[index] ) {

                    // see if anything to write
                    for ( ii=0; ii<tagDestMaxSize[index]; ii++ ) {
                        if ( strcmp( s, (char *) tagDefault[index] ) != 0 ) {
                            s += enumNumChoices[index];
                            doWrite = 1;
                        }
                    }

                }
                else {

                    doWrite = 1;

                }

                if ( doWrite ) {

                    s = (char *) tagDestination[index];

                    fprintf( f, "%s {\n", tagName[index] );

                    for ( i=0; i<tagDestMaxSize[index]; i++ ) {

                        if ( tagDefault[index] ) {

                            if ( strcmp( s, (char *) tagDefault[index] ) != 0 ) {
                                if ( blank(s) ) strcpy( s, "\'\'" );
                                fprintf( f, "  %-d ", i );
                                writeString( f, s );
                            }

                        }
                        else {

                            if ( blank(s) ) strcpy( s, "\'\'" );
                            fprintf( f, "  %-d ", i );
                            writeString( f, s );

                        }

                        s += enumNumChoices[index];

                    }

                    fprintf( f, "}\n" );

                }

            }
            else {

                fprintf( stderr, tagClass_str23, tagName[index] );
                return 0;

            }

            if ( tagClass::genDoc() ) {

                if ( tagDefault[index] ) {

                    fprintf( stderr, "[%s {\n", tagName[index] );
                    fprintf( stderr, "  [<int element> <string>]   /* default = \"%s\" */\n",
                             (char *) tagDefault[index] );
                    fprintf( stderr, "  [<int element> <string>]   /* default = \"%s\" */\n",
                             (char *) tagDefault[index] );
                    fprintf( stderr, "      .\n" );
                    fprintf( stderr, "      .\n" );
                    fprintf( stderr, "}]\n" );

                }
                else {

                    fprintf( stderr, "%s {\n", tagName[index] );
                    fprintf( stderr, "  <int element> <string>\n" );
                    fprintf( stderr, "  <int element> <string>\n" );
                    fprintf( stderr, "      .\n" );
                    fprintf( stderr, "      .\n" );
                    fprintf( stderr, "}\n" );

                }

            }

            break;

        case tagClass::EXP_STRING: // expStringClass

            expStr = (expStringClass *) tagDestination[index];
            sDef = (char *) tagDefault[index];

            if ( expStr ) {

                if ( sDef ) {

                    if ( strcmp( expStr->getRaw(), sDef ) != 0 ) {

                        fprintf( f, "%s ", tagName[index] );
                        writeString( f, expStr->getRaw() );

                    }

                }
                else {

                    fprintf( f, "%s ", tagName[index] );
                    writeString( f, expStr->getRaw() );

                }

            }
            else {

                fprintf( stderr, tagClass_str23, tagName[index] );
                return 0;

            }

            if ( tagClass::genDoc() ) {
                if ( sDef ) {
                    fprintf( stderr, "[%s <expandable string>] /* default = \"%s\" */\n",
                             tagName[index], sDef );
                }
                else {
                    fprintf( stderr, "%s <expandable string>\n", tagName[index] );
                }
            }

            break;

        case tagClass::COMPLEX_EXP_STRING: // expandable string that may contain
            // new lines and other special
            // characters

            expStr = (expStringClass *) tagDestination[index];
            sDef = (char *) tagDefault[index];

            if ( expStr ) {

                if ( sDef ) {

                    if ( strcmp( expStr->getRaw(), sDef ) != 0 ) {

                        fprintf( f, "%s {\n", tagName[index] );
                        writeMultiLineString( f, expStr->getRaw() );
                        //fprintf( f, "%s\n", expStr->getRaw() );
                        fprintf( f, "}\n" );

                    }

                }
                else {

                    fprintf( f, "%s {\n", tagName[index] );
                    writeMultiLineString( f, expStr->getRaw() );
                    //fprintf( f, "%s\n", expStr->getRaw() );
                    fprintf( f, "}\n" );

                }

            }
            else {

                fprintf( stderr, tagClass_str23, tagName[index] );
                return 0;

            }

            if ( tagClass::genDoc() ) {
                if ( sDef ) {
                    fprintf( stderr, "[%s {\n", tagName[index] );
                    fprintf( stderr, "  <expandable string> /* line 1 of n */\n" );
                    fprintf( stderr, "  <expandable string> /* line 2 of n */\n" );
                    fprintf( stderr, "      .\n" );
                    fprintf( stderr, "      .\n" );
                    fprintf( stderr, "}]   /* default = \"%s\" */\n", sDef );
                }
                else {
                    fprintf( stderr, "%s {\n", tagName[index] );
                    fprintf( stderr, "  <expandable string> /* line 1 of n */\n" );
                    fprintf( stderr, "  <expandable string> /* line 2 of n */\n" );
                    fprintf( stderr, "      .\n" );
                    fprintf( stderr, "      .\n" );
                    fprintf( stderr, "}\n" );
                }
            }

            break;

        case tagClass::EXP_STRING_ARRAY:   // expandable string that may not
            // contain new lines and other special
            // characters

            expStr = (expStringClass *) tagDestination[index];

            if ( expStr ) {

                doWrite = 0;
                if ( tagDefault[index] ) {

                    // see if anything to write
                    for ( ii=0; ii<tagDestMaxSize[index]; ii++ ) {
                        if ( strcmp( expStr[ii].getRaw(), (char *) tagDefault[index] )
                             != 0 ) {
                            doWrite = 1;
                        }
                    }

                }
                else {

                    doWrite = 1;

                }

                if ( doWrite ) {

                    fprintf( f, "%s {\n", tagName[index] );

                    for ( i=0; i<tagDestMaxSize[index]; i++ ) {

                        if ( tagDefault[index] ) {

                            if ( strcmp( expStr[i].getRaw(), (char *) tagDefault[index] )
                                 != 0 ) {

                                if ( blank( expStr[i].getRaw() ) ) {
                                    fprintf( f, "  %-d \'\'\n", i );
                                }
                                else {
                                    fprintf( f, "  %-d ", i );
                                    writeString( f, expStr[i].getRaw() );
                                }

                            }

                        }
                        else {

                            if ( blank( expStr[i].getRaw() ) ) {
                                fprintf( f, "  %-d \'\'\n", i );
                            }
                            else {
                                fprintf( f, "  %-d ", i );
                                writeString( f, expStr[i].getRaw() );
                            }

                        }

                    }

                    fprintf( f, "}\n" );

                }

            }
            else {

                fprintf( stderr, tagClass_str23, tagName[index] );
                return 0;

            }

            if ( tagClass::genDoc() ) {

                if ( tagDefault[index] ) {

                    fprintf( stderr, "[%s {\n", tagName[index] );
                    fprintf( stderr, "  [<int element> <expandable string>]   /* default = \"%s\" */\n",
                             (char *) tagDefault[index] );
                    fprintf( stderr, "  [<int element> <expandable string>]   /* default = \"%s\" */\n",
                             (char *) tagDefault[index] );
                    fprintf( stderr, "      .\n" );
                    fprintf( stderr, "      .\n" );
                    fprintf( stderr, "}]\n" );

                }
                else {

                    fprintf( stderr, "%s {\n", tagName[index] );
                    fprintf( stderr, "  <int element> <expandable string>\n" );
                    fprintf( stderr, "  <int element> <expandable string>\n" );
                    fprintf( stderr, "      .\n" );
                    fprintf( stderr, "      .\n" );
                    fprintf( stderr, "}\n" );

                }

            }

            break;

        case tagClass::INTEGER_ARRAY:

            iArray = (int *) tagDestination[index];

            if ( iArray ) {

                doWrite = 0;
                if ( tagDefault[index] ) {

                    // see if anything to write
                    for ( ii=0; ii<tagDestMaxSize[index]; ii++ ) {
                        if ( iArray[ii] != *( (int *) tagDefault[index] ) ) {
                            doWrite = 1;
                        }
                    }

                }
                else {

                    doWrite = 1;

                }

                if ( doWrite ) {

                    fprintf( f, "%s {\n", tagName[index] );

                    for ( i=0; i<tagDestMaxSize[index]; i++ ) {

                        if ( tagDefault[index] ) {

                            if ( iArray[i] != *( (int *) tagDefault[index] ) ) {
                                fprintf( f, "  %-d %-d\n", i, iArray[i] );
                            }

                        }
                        else {

                            fprintf( f, "  %-d %-d\n", i, iArray[i] );

                        }

                    }

                    fprintf( f, "}\n" );

                }

            }
            else {

                fprintf( stderr, tagClass_str23, tagName[index] );
                return 0;

            }

            if ( tagClass::genDoc() ) {

                if ( tagDefault[index] ) {
                    fprintf( stderr, "[%s {\n", tagName[index] );
                    fprintf( stderr, "  [<int element> <int>]   /* default = %-d */\n",
                             *( (int *) tagDefault[index] ) );
                    fprintf( stderr, "  [<int element> <int>]   /* default = %-d */\n",
                             *( (int *) tagDefault[index] ) );
                    fprintf( stderr, "      .\n" );
                    fprintf( stderr, "      .\n" );
                    fprintf( stderr, "}]\n" );
                }
                else {
                    fprintf( stderr, "%s {\n", tagName[index] );
                    fprintf( stderr, "  <int element> <int>\n" );
                    fprintf( stderr, "  <int element> <int>\n" );
                    fprintf( stderr, "      .\n" );
                    fprintf( stderr, "      .\n" );
                    fprintf( stderr, "}\n" );
                }

            }

            break;

        case tagClass::UNSIGNED_INTEGER_ARRAY:

            uiArray = (unsigned int *) tagDestination[index];

            if ( uiArray ) {

                doWrite = 0;
                if ( tagDefault[index] ) {

                    // see if anything to write
                    for ( ii=0; ii<tagDestMaxSize[index]; ii++ ) {
                        if ( uiArray[ii] != *( (unsigned int *) tagDefault[index] ) ) {
                            doWrite = 1;
                        }
                    }

                }
                else {

                    doWrite = 1;

                }

                if ( doWrite ) {

                    fprintf( f, "%s {\n", tagName[index] );

                    for ( i=0; i<tagDestMaxSize[index]; i++ ) {

                        if ( tagDefault[index] ) {

                            if ( uiArray[i] != *( (unsigned int *) tagDefault[index] ) ) {
                                fprintf( f, " %-d %-u\n", i, uiArray[i] );
                            }

                        }
                        else {

                            fprintf( f, "  %-d %-u\n", i, uiArray[i] );

                        }

                    }

                    fprintf( f, "}\n" );

                }

            }
            else {

                fprintf( stderr, tagClass_str23, tagName[index] );
                return 0;

            }

            if ( tagClass::genDoc() ) {

                if ( tagDefault[index] ) {
                    fprintf( stderr, "[%s {\n", tagName[index] );
                    fprintf( stderr, "  [<int element> <unsigned int>]   /* default = %-d */\n",
                             *( (int *) tagDefault[index] ) );
                    fprintf( stderr, "  [<int element> <unsigned int>]   /* default = %-d */\n",
                             *( (int *) tagDefault[index] ) );
                    fprintf( stderr, "      .\n" );
                    fprintf( stderr, "      .\n" );
                    fprintf( stderr, "}]\n" );
                }
                else {
                    fprintf( stderr, "%s {\n", tagName[index] );
                    fprintf( stderr, "  <int element> <unsigned int>\n" );
                    fprintf( stderr, "  <int element> <unsigned int>\n" );
                    fprintf( stderr, "      .\n" );
                    fprintf( stderr, "      .\n" );
                    fprintf( stderr, "}\n" );
                }

            }

            break;

        case tagClass::HEX_INTEGER_ARRAY:

            iArray = (int *) tagDestination[index];

            if ( iArray ) {

                doWrite = 0;
                if ( tagDefault[index] ) {

                    // see if anything to write
                    for ( ii=0; ii<tagDestMaxSize[index]; ii++ ) {
                        if ( iArray[ii] != *( (int *) tagDefault[index] ) ) {
                            doWrite = 1;
                        }
                    }

                }
                else {

                    doWrite = 1;

                }

                if ( doWrite ) {

                    fprintf( f, "%s {\n", tagName[index] );

                    for ( i=0; i<tagDestMaxSize[index]; i++ ) {

                        if ( tagDefault[index] ) {

                            if ( iArray[i] != *( (int *) tagDefault[index] ) ) {
                                fprintf( f, "  %-d 0x%-x\n", i, iArray[i] );
                            }

                        }
                        else {

                            fprintf( f, "  %-d 0x%-x\n", i, iArray[i] );

                        }

                    }

                    fprintf( f, "}\n" );

                }

            }
            else {

                fprintf( stderr, tagClass_str23, tagName[index] );
                return 0;

            }

            if ( tagClass::genDoc() ) {

                if ( tagDefault[index] ) {
                    fprintf( stderr, "[%s {\n", tagName[index] );
                    fprintf( stderr, "  [<int element> <hex int>]   /* default = %-d */\n",
                             *( (int *) tagDefault[index] ) );
                    fprintf( stderr, "  [<int element> <hex int>]   /* default = %-d */\n",
                             *( (int *) tagDefault[index] ) );
                    fprintf( stderr, "      .\n" );
                    fprintf( stderr, "      .\n" );
                    fprintf( stderr, "}]\n" );
                }
                else {
                    fprintf( stderr, "%s {\n", tagName[index] );
                    fprintf( stderr, "  <int element> <hex int>\n" );
                    fprintf( stderr, "  <int element> <hex int>\n" );
                    fprintf( stderr, "      .\n" );
                    fprintf( stderr, "      .\n" );
                    fprintf( stderr, "}\n" );
                }

            }

            break;

        case tagClass::REAL_ARRAY:

            dArray = (double *) tagDestination[index];

            if ( dArray ) {

                doWrite = 0;
                if ( tagDefault[index] ) {

                    // see if anything to write
                    for ( ii=0; ii<tagDestMaxSize[index]; ii++ ) {
                        if ( dArray[ii] != *( (double *) tagDefault[index] ) ) {
                            doWrite = 1;
                        }
                    }

                }
                else {

                    doWrite = 1;

                }

                if ( doWrite ) {

                    fprintf( f, "%s {\n", tagName[index] );

                    for ( i=0; i<tagDestMaxSize[index]; i++ ) {

                        if ( tagDefault[index] ) {

                            if ( dArray[i] != *( (double *) tagDefault[index] ) ) {
                                fprintf( f, "  %-d %-g\n", i, dArray[i] );
                            }

                        }
                        else {

                            fprintf( f, "  %-d %-g\n", i, dArray[i] );

                        }

                    }

                    fprintf( f, "}\n" );

                }

            }
            else {

                fprintf( stderr, tagClass_str23, tagName[index] );
                return 0;

            }

            if ( tagClass::genDoc() ) {

                if ( tagDefault[index] ) {
                    fprintf( stderr, "[%s {\n", tagName[index] );
                    fprintf( stderr, "  [<int element> <real>]   /* default = %-d */\n",
                             *( (int *) tagDefault[index] ) );
                    fprintf( stderr, "  [<int element> <real>]   /* default = %-d */\n",
                             *( (int *) tagDefault[index] ) );
                    fprintf( stderr, "      .\n" );
                    fprintf( stderr, "      .\n" );
                    fprintf( stderr, "}]\n" );
                }
                else {
                    fprintf( stderr, "%s {\n", tagName[index] );
                    fprintf( stderr, "  <int element> <real>\n" );
                    fprintf( stderr, "  <int element> <real>\n" );
                    fprintf( stderr, "      .\n" );
                    fprintf( stderr, "      .\n" );
                    fprintf( stderr, "}\n" );
                }

            }

            break;

        case tagClass::UNKNOWN:

            if( isCompound[index] ) {
                fprintf( f, "%s {\n", tagName[index] );
                char *val = (char *) tagDestination[index];
                while ( char *end = strchr(val,'\n') ) {
                    *end = 0;
                    fprintf( f, "  %s\n", val );
                    *end = '\n';
                    val = end+1;
                }
                fprintf( f, "}\n" );
            }
            else {
                fprintf( f, "%s %s", tagName[index], (char *) tagDestination[index] );
            }

            break;

        }

    }

    return 1;

}

char *tagClass::errMsg ( void ) {

    return msg;

}
