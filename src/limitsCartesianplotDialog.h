 #ifndef limitsCartesianplotDialog_H
 #define limitsCartesianplotDialog_H

#include <QDialog>
#include <QDialogButtonBox>
#include <QGridLayout>
#include <QComboBox>
#include <QLineEdit>
#include <QCheckBox>
#include <QPushButton>
#include <QList>
#include <QPair>
#include <QString>
#include <QLabel>
#include "cacartesianplot.h"
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

 class limitsCartesianplotDialog : public QDialog
{
     Q_OBJECT

 public:

     limitsCartesianplotDialog(caCartesianPlot *w, MutexKnobData *data, const QString &title, QWidget *parent);

 public slots:
     void cancelClicked();
     void applyClicked();

 private:
     QComboBox *xComboBox;
     QComboBox *yComboBox;
     QCheckBox *xCheckBox;
     QCheckBox *yCheckBox;
     QLineEdit *xminLineEdit;
     QLineEdit *xmaxLineEdit;
     QLineEdit *yminLineEdit;
     QLineEdit *ymaxLineEdit;
     caCartesianPlot *CartesianPlot;
     MutexKnobData *monData;
 };


 #endif
