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
class FunctionWidget;
class LabelWidget;
class PlotStyleWidgetInterface;
class ValuesWidget;
class TreeViewComboBox;
class Set;

class FunctionPlotDialog: public KDialog{
	Q_OBJECT

public:
	FunctionPlotDialog(MainWin *mw, const Plot::PlotType& type=Plot::PLOT2D);
	void setModel(QAbstractItemModel * model);
	void setSet(Set*);
	void saveSet(Set*) const;
	void setCurrentIndex(const QModelIndex&);
	QModelIndex currentIndex() const;

private:
	QTabWidget* tabWidget;
	FunctionWidget* functionWidget;
	LabelWidget* labelWidget;
	PlotStyleWidgetInterface* plotStyleWidget;
	ValuesWidget* valuesWidget;
	QFrame* frameAddTo;
	TreeViewComboBox* cbAddTo;

	Set* set;
	bool editMode;
	MainWin* mainWin;
	Plot::PlotType plotType;

private slots :
 	void save();
	void apply() const;
};
#endif
