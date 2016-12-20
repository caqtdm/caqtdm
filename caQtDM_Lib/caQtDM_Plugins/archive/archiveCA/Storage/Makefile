
CXXFLAGS += -I. -I$(EPICSINCLUDE) -I$(EPICSINCLUDE)/os/Linux -I../xerces-c-3.1.4/src -I../Tools -fPIC 
CC = gcc  -fPIC 

# Old
OBJ_SRC = HashTable.o
OBJ_SRC += OldDirectoryFile.o
OBJ_SRC += OldDataWriter.o
OBJ_SRC += OldDataReader.o
# Current
OBJ_SRC += CtrlInfo.o
OBJ_SRC += RawValue.o
OBJ_SRC += DataFile.o
OBJ_SRC += FileAllocator.o
OBJ_SRC += NameHash.o
OBJ_SRC += RTree.o
OBJ_SRC += IndexFile.o
OBJ_SRC += ListIndex.o
OBJ_SRC += AutoIndex.o
OBJ_SRC += DataWriter.o
OBJ_SRC += DataReader.o
OBJ_SRC += RawDataReader.o
OBJ_SRC += ReaderFactory.o
OBJ_SRC += AverageReader.o
OBJ_SRC += LinearReader.o
OBJ_SRC += PlotReader.o
OBJ_SRC += SpreadsheetReader.o

LBITS := $(shell getconf LONG_BIT)
ifeq ($(LBITS),64)
	TARGET = libStorage_64.so
	TARGET1 = libStorage_64.a
	LIBS = ../xerces-c-3.1.4/libxerces-c_64.a
else 
	TARGET = libStorage_32.so
	TARGET1 = libStorage_32.a
	LIBS = ../xerces-c-3.1.4/libxerces-c_32.a
endif

all: $(TARGET)

clean:
	rm -f *.ln *.BAK *.bak *.o core errs ,* *~ 

$(TARGET): $(OBJ_SRC)
	$(CC) -shared -o $(TARGET) $(OBJ_SRC) $(LIBS)
	rm -f $(TARGET1)
	ar cr $(TARGET1) $(OBJ_SRC)
	ranlib $(TARGET1)
