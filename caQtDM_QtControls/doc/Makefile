TARGET = caQtDM.qhc

OBJ = caQtDM.html caQtDM.qhp

all: $(TARGET)

clean:
	rm -f  *~ *.a *.qch *.qhc
	
$(TARGET): $(OBJ)
	qcollectiongenerator caQtDM.qhcp -o caQtDM.qhc
  