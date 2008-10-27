/***************************************************************************
    File                 : FunctionPlotDialog.h
    Project              : LabPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2008 by Stefan Gerlach, Alexander Semke
    Email (use @ for *)  : stefan.gerlach*uni-konstanz.de, alexander.semke*web.de
    Description          : dialog for plotting functions
                           
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *  This program is free software; you can redistribute it and/or modify   *
 *  it under the terms of the GNU General Public License as published by   *
 *  the Free Software Foundation; either version 2 of the License, or      *
 *  (at your option) any later version.                                    *
 *                                                                         *
 *  This program is distributed in the hope that it will be useful,        *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *  GNU General Public License for more details.                           *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the Free Software           *
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor,                    *
 *   Boston, MA  02110-1301  USA                                           *
 *                                                                         *
 ***************************************************************************/
#ifndef FUNCTIONPLOTDIALOG_H
#define FUNCTIONPLOTDIALOG_H

#include <QtGui>
#include "kdialog.h"
#include "plots/Plot.h"

class MainWin;
class FunctionPlotWidget;
class Set;

class FunctionPlotDialog: public KDialog{
	Q_OBJECT

public:
	FunctionPlotDialog(MainWin *mw, const Plot::PlotType& type=Plot::PLOT2D);
	void setSet(Set*);
	void saveSet(Set*) const;
	int currentSheetIndex() const;
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
	FunctionPlotWidget* functionPlotWidget;
	QFrame* frameAddTo;
	QComboBox* cbAddTo;
	Set* set;
	bool editMode;
	MainWin* mainWin;
	Plot::PlotType plotType;

private slots :
// 	void save() const;
	void apply() const;
};
#endif
