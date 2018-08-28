/***************************************************************************
    File                 : PlotDataDialog.h
    Project              : LabPlot
    Description          : Dialog for generating plots for the spreadsheet data
    --------------------------------------------------------------------
    Copyright            : (C) 2017 by Alexander Semke (alexander.semke@web.de)

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
#ifndef PLOTDATADIALOG_H
#define PLOTDATADIALOG_H

namespace Ui {
	class PlotDataWidget;
}

#include <QDialog>

class QComboBox;
class AspectTreeModel;
class CartesianPlot;
class Column;
class Spreadsheet;
class TreeViewComboBox;
class Worksheet;

class PlotDataDialog : public QDialog {
	Q_OBJECT

public:
	enum AnalysisAction {DataReduction,
		Differentiation, Integration, Interpolation, Smoothing,
		FitLinear, FitPower, FitExp1, FitExp2, FitInvExp, FitGauss, FitCauchyLorentz, FitTan, FitTanh, FitErrFunc, FitCustom,
		FourierFilter};

	explicit PlotDataDialog(Spreadsheet*, QWidget* parent = nullptr);
	~PlotDataDialog() override;

	void setAnalysisAction(AnalysisAction);

private:
	Ui::PlotDataWidget* ui;
	QPushButton* m_okButton;
	Spreadsheet* m_spreadsheet;
	TreeViewComboBox* cbExistingPlots;
	TreeViewComboBox* cbExistingWorksheets;
	QVector<Column*> m_columns;
	QVector<QComboBox*> m_columnComboBoxes;
	AspectTreeModel* m_plotsModel;
	AspectTreeModel* m_worksheetsModel;
	AnalysisAction m_analysisAction;
	bool m_analysisMode;

	void processColumns();
	void addCurvesToPlot(CartesianPlot*) const;
	void addCurvesToPlots(Worksheet*) const;
	void addCurve(const QString& name, Column* xColumn, Column* yColumn, CartesianPlot*) const;
	Column* columnFromName(const QString&) const;

protected  slots:
	virtual void checkOkButton();

private slots:
	void plot();
	void curvePlacementChanged();
	void plotPlacementChanged();
	void loadSettings();
};

#endif
