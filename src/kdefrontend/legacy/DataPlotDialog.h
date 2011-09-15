/***************************************************************************
    File                 : DataPlotDialog.h
    Project              : LabPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2008 by Stefan Gerlach, Alexander Semke
    Email (use @ for *)  : stefan.gerlach*uni-konstanz.de, alexander.semke*web.de
    Description          : dialog for importing data to a Worksheet or Spreadsheet.

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
#ifndef DATAPLOTDIALOG_H
#define DATAPLOTDIALOG_H

#include <QtGui>
#include "kdialog.h"
#include "plots/Plot.h"

class MainWin;
class ImportWidget;
class LabelWidget;
class PlotStyleWidgetInterface;
class ValuesWidget;
class TreeViewComboBox;
class Set;

class DataPlotDialog: public KDialog{
	Q_OBJECT

public:
	DataPlotDialog(MainWin *mw, const Plot::PlotType& type=Plot::PLOT2D);
	void setModel(QAbstractItemModel * model);
	void saveSet(Set*) const;
	QModelIndex currentIndex() const;
	void setCurrentIndex(const QModelIndex&);

private:
	QTabWidget* tabWidget;
	ImportWidget* importWidget;
	LabelWidget* labelWidget;
	PlotStyleWidgetInterface* plotStyleWidget;
	ValuesWidget* valuesWidget;
	QFrame* frameAddTo;
	TreeViewComboBox* cbAddTo;
	MainWin* mainWin;
};
#endif
