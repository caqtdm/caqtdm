// System
#include <stdio.h>

// Storage
#include "IndexFile.h"
#include "DataReader.h"

int main(int argc, char *argv[])
{
    stdString index_name = argv[1];
    stdString channel_name = argv[2];
    stdString t, s, v;

    if (argc != 3)
    {
        fprintf(stderr, "USAGE: ReadTest <index> <channel>\n");
        return 1;
    }
    
    IndexFile index(50);
    if (index.open(index_name))
    {
        RawDataReader reader(index);
        const RawValue::Data *value = reader.find(channel_name, 0);
        while (value)
        {
            epicsTime2string(RawValue::getTime(value), t);
            RawValue::getStatus(value, s);
            RawValue::getValueString(
                v, reader.getType(), reader.getCount(), value);
            printf("%s\t%s\t%s\n",
                   t.c_str(), v.c_str(), s.c_str());
            value = reader.next();
        }
        index.close();
    }
    else
    {
        fprintf(stderr, "Cannot open index '%s'\n",
                index_name.c_str());
        return 2;
    }
    
    return 0;
}
