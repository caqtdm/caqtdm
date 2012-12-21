 #ifndef limitsStripplotDialog_H
 #define limitsStripplotDialog_H

 #include <QDialog>
#include <QComboBox>
#include <QLineEdit>
 #include <QList>
 #include <QPair>
 #include <QString>
#include "castripplot.h"
#include "dbrString.h"
#include "knobDefines.h"
#include "knobData.h"
#include "mutexKnobData.h"

 class QCheckBox;
 class QDialogButtonBox;
 class QLabel;
 class QLineEdit;
 class QTableWidget;
 class QTextEdit;
 class QWidget;

 class limitsStripplotDialog : public QDialog
{
     Q_OBJECT

 public:

     limitsStripplotDialog(caStripPlot *w, MutexKnobData *data, const QString &title, QWidget *parent);

 public slots:
     void cancelClicked();
     void applyClicked();

 private:
     QComboBox *minComboBox[MAXCURVES];
     QComboBox *maxComboBox[MAXCURVES];
     QLineEdit *minLineEdit[MAXCURVES];
     QLineEdit *maxLineEdit[MAXCURVES];
     QStringList vars;
     caStripPlot *StripPlot;
     MutexKnobData *monData;
 };


 #endif
