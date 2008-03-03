//LabPlot : FunctionDialog.h

#ifndef FUNCTIONDIALOG_H
#define FUNCTIONDIALOG_H

#include <KComboBox>
#include <KLineEdit>
#include <KIntNumInput>
#include "Dialog.h"
#include "Label.h"

class FunctionDialog: public Dialog
{
	Q_OBJECT
public:
	FunctionDialog(MainWin *mw);
public slots:
/*	int addFunction();	
	void updateLabel() { rtw->getLabel()->setTitle(funle->text()); rtw->update(); }

	void setFunction(QString fun) { funle->setText(fun); }			//!< set function to create 
	void recreate(bool b=true) { reread->setChecked(b); }		//!< create function new ?
	void setLabel(QString label=0) {					//!< set label of function
		if(label!=0) funle->setText(label);
		updateLabel();
	}
	void setRange(double a, double b) { xmin->setText(QString::number(a)); xmax->setText(QString::number(b)); }
														//!< set X range of function
	void setYRange(double a, double b) { ymin->setText(QString::number(a)); ymax->setText(QString::number(b)); }
														//!< set Y range of function
	void setPoints(int nr) { nx->setValue(nr); }				//!< set number of X points for function
	void setYPoints(int nr) { ny->setValue(nr); }				//!< set number of Y points for function
	
	int Apply() { return apply_clicked(); }
*/
private:
	PlotType type;
// TODO	Set *set;	-> for changing set
	KComboBox *constantscb, *functionscb;
	KLineEdit *functionle;
	KLineEdit *minle, *maxle;
	KIntNumInput *nxi;
	KComboBox *sheetcb;
	void setupGUI();
/*
	void findPlot();
	KLineEdit *ymin, *ymax;
	KIntNumInput *ny;
	QCheckBox *reread;
	ListDialog *l;
	int item;
	PType type;
	Graph *graph;			// Graph
	RichTextWidget *rtw;		// label widget
*/
private slots :
	void saveSettings();
//	void toggleOptions();
	void Apply();
	void insertFunction(QString f);
	void insertConstant(QString c);
};
#endif //FUNCTIONDIALOG_H
