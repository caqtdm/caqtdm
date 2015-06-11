#ifndef mdaReader_H
#define mdaReader_H

#include <QString>

void mdaReader_RegisterPV(QString pvName);
int mdaReader_gimmeYerData(QString QS_dataFile, QString QS_pvName, float *data, int nx, int ny, int y_cpt);
#endif
