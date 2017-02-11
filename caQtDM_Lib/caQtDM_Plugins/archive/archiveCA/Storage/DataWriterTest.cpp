// Tools
#include <UnitTest.h>
#include <AutoPtr.h>
// Storage
#include <DataWriter.h>
#include <RawDataReader.h>
#include <IndexFile.h>
#include <DataFile.h>

static const char *index_name = "test/data_writer.index";
static const char *channel_name = "fred";
static const size_t samples = 10000;

TEST_CASE data_writer_test()
{
    TEST_DELETE_FILE(index_name);
    TEST_DELETE_FILE("test/data_writer.data");
    try
    {
        IndexFile index(50);
        index.open(index_name, false);
    
        CtrlInfo info;
        info.setNumeric (2, "socks",
                         0.0, 10.0,
                         0.0, 1.0, 9.0, 10.0);
    
        DbrType dbr_type = DBR_TIME_DOUBLE;
        DbrCount dbr_count = 1;
        DataWriter::file_size_limit = 10*1024*1024;
        DataWriter::data_file_name_base = "data_writer.data";
        AutoPtr<DataWriter> writer(new DataWriter(index,
                                                  channel_name, info,
                                                  dbr_type, dbr_count, 2.0,
                                                  samples));
        RawValueAutoPtr data(RawValue::allocate(dbr_type, dbr_count, 1));
        RawValue::setStatus(data, 0, 0);
        size_t i;
        for (i=0; i<samples; ++i)
        {
            data->value = (double) i;
            RawValue::setTime(data, epicsTime::getCurrent());
            if (!writer->add(data))
            {
                fprintf(stderr, "Write error with value %zu/%zu\n",
                        i, samples);
                break;
            }   
        }
        writer = 0;
        DataFile::close_all();
    }
    catch (GenericException &e)
    {
        printf("Exception:\n%s\n", e.what());
        FAIL("DataWriter test failed");
    }
    TEST_MSG(1, "DataWriter test passed");
    TEST_OK;
}

TEST_CASE data_writer_readback()
{
    size_t samples_read = 0;
    try
    {
        IndexFile index(50);
        index.open(index_name, true);
        AutoPtr<DataReader> reader(new RawDataReader(index));
        const RawValue::Data *data =
            reader->find(channel_name, 0);
        while (data)
        {
            if (samples_read != ((const dbr_time_double *)data)->value)
                 FAIL("Value read back doesn't contain expected number");
            data = reader->next();    
            ++samples_read;
        }
        reader = 0;
        DataFile::close_all();
        index.close();
    }
    catch (GenericException &e)
    {
        printf("Exception:\n%s\n", e.what());
        FAIL("DataWriter test failed");
    }
    TEST(samples_read == samples);
    TEST_OK;

}


