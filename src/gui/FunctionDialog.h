#ifndef FUNCTIONDIALOG_H
#define FUNCTIONDIALOG_H

#include "kdialog.h"
#include "../plottype.h"

class MainWin;
class FunctionWidget;
class Function;

class FunctionDialog: public KDialog{
	Q_OBJECT

public:
	FunctionDialog(MainWin *mw, const PlotType& type=PLOT2D);

public slots:
	void setFunction( Function* );
// 	void saveFunction( Function* ) const;

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
	FunctionWidget* functionWidget;

private slots :
	void save() const;
	void apply() const;
};
#endif //FUNCTIONDIALOG_H
