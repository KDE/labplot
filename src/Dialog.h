//LabPlot : Dialog.h

#ifndef DIALOG_H
#define DIALOG_H

// for all dialogs
#include <KDebug>
#include <KLocale>
#include <QLabel>

#include <QGridLayout>
#include <KDialog>
#include <KNumInput>

#include "MainWin.h"
#include "elements/Label.h"

class Dialog: public KDialog
{
	Q_OBJECT
public:
	Dialog(MainWin *mw);
protected:
	MainWin *mw;
	QGridLayout *layout;
	KDoubleNumInput *xni, *yni;	// label widget
	QTextEdit *labelte;
	void labelWidget(QWidget *parent, Label *label);	//!< widget for label settings
	void setupLabel(Label *label);				//!< apply settings from label widget
private slots:
	void setLabelFont(QFont font);
	void setLabelSize(QString size);
	void insertSymbol(QString c);
};

#endif //DIALOG_H
