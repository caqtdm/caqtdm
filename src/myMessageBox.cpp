#include <myMessageBox.h>


#include <QVBoxLayout>
#include <QPushButton>
#include <QDialogButtonBox>

myMessageBox::myMessageBox(QWidget *parent) : QDialog(parent)
{
    thisText = new QTextEdit(this);
    thisText->setReadOnly(true);
    thisText->setTextInteractionFlags(Qt::TextSelectableByMouse);
    thisText->setLineWrapMode(QTextEdit::NoWrap);

    QPushButton *cancelButton = new QPushButton(tr("Close"));

    QDialogButtonBox *buttonBox = new QDialogButtonBox(Qt::Horizontal);
    buttonBox->addButton(cancelButton, QDialogButtonBox::RejectRole);

    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

    QVBoxLayout *lt = new QVBoxLayout;
    lt->addWidget(thisText);
    lt->addWidget(buttonBox);

    setMinimumSize(300,500);

    setLayout(lt);
}

void myMessageBox::setText(QString strng) const
{
   thisText->setText(strng);
}

