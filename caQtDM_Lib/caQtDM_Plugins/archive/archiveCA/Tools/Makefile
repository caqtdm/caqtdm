OBJ_SRC = ArchiveException.o
OBJ_SRC += ArgParser.o
OBJ_SRC += ASCIIParser.o
OBJ_SRC += BenchTimer.o
OBJ_SRC += BinIO.o
OBJ_SRC += Bitset.o
OBJ_SRC += CGIDemangler.o
OBJ_SRC += ConcurrentList.o
OBJ_SRC += epicsTimeHelper.o
OBJ_SRC += Filename.o
OBJ_SRC += Guard.o
OBJ_SRC += GenericException.o
OBJ_SRC += IndexConfig.o
OBJ_SRC += Lockfile.o
OBJ_SRC += MsgLogger.o
OBJ_SRC += NetTools.o
OBJ_SRC += OrderedMutex.o
OBJ_SRC += RegularExpression.o
OBJ_SRC += stdString.o
OBJ_SRC += Throttle.o
OBJ_SRC += ThrottledMsgLogger.o
OBJ_SRC += UnitTestLib.o
OBJ_SRC += FUX.o


CXXFLAGS += -I. -I$(EPICSINCLUDE) -I$(EPICSINCLUDE)/os/Linux -I../xerces-c-3.1.4/src/ -fPIC
CC = gcc -fPIC 

EPICS_LIBS = -L$${EPICSLIB} -Wl,-rpath,$${EPICSLIB} -lca -lCom

LBITS := $(shell getconf LONG_BIT)
ifeq ($(LBITS),64)
TARGET = libTools_64.so
TARGET1 = libTools_64.a
LIBS = ../xerces-c-3.1.4/libxerces-c_64.a
else 
TARGET = libTools_32.so
TARGET1 = libTools_32.a
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


