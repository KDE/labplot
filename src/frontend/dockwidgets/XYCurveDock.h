/*
	File             : XYCurveDock.h
	Project          : LabPlot
	Description      : widget for curve properties
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2010-2026 Alexander Semke <alexander.semke@web.de>
	SPDX-FileCopyrightText: 2013 Stefan Gerlach <stefan.gerlach@uni.kn>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef XYCURVEDOCK_H
#define XYCURVEDOCK_H

#include "backend/core/AbstractColumn.h"
#include "backend/worksheet/plots/cartesian/XYCurve.h"
#include "frontend/dockwidgets/BaseDock.h"

#include "ui_xycurvedock.h"
#include "ui_xycurvedockgeneraltab.h"

class BackgroundWidget;
class ErrorBarWidget;
class LineWidget;
class Column;
class SymbolWidget;
class ValueWidget;
class TreeViewComboBox;

class XYCurveDock : public BaseDock {
	Q_OBJECT

public:
	explicit XYCurveDock(QWidget*);
	~XYCurveDock() override;

	void setCurves(QList<XYCurve*>);
	virtual void setupGeneral();
	void updateLocale() override;
	void retranslateUi() override;

private:
	virtual void initGeneralTab();

	void load();
	void loadConfig(KConfig&);

	Ui::XYCurveDockGeneralTab uiGeneralTab;

	TreeViewComboBox* cbXColumn{nullptr};
	TreeViewComboBox* cbYColumn{nullptr};

protected:
	void initTabs();
	static QList<AspectType> defaultColumnTopLevelClasses();
	void setModel();
	void setSymbols(const QList<XYCurve*>&);

	Ui::XYCurveDock ui;
	LineWidget* lineWidget{nullptr};
	LineWidget* dropLineWidget{nullptr};
	BackgroundWidget* backgroundWidget{nullptr};
	SymbolWidget* symbolWidget{nullptr};
	ErrorBarWidget* errorBarWidget{nullptr};
	ValueWidget* valueWidget{nullptr};
	QList<XYCurve*> m_curvesList;
	XYCurve* m_curve{nullptr};

private Q_SLOTS:
	void init();

	// SLOTs for changes triggered in XYCurveDock
	void xColumnChanged(const QModelIndex&);
	void yColumnChanged(const QModelIndex&);

	// Line-Tab
	void lineTypeChanged(int);
	void lineSkipGapsChanged(bool);
	void lineIncreasingXOnlyChanged(bool);
	void lineInterpolationPointsCountChanged(int);

	//"Margin Plots"-Tab
	void rugEnabledChanged(bool);
	void rugOrientationChanged(int);
	void rugLengthChanged(double);
	void rugWidthChanged(double);
	void rugOffsetChanged(double);

	// SLOTs for changes triggered in XYCurve
	// General-Tab
	void curveDescriptionChanged(const AbstractAspect*);
	void curveXColumnChanged(const AbstractColumn*);
	void curveYColumnChanged(const AbstractColumn*);

	// Line-Tab
	void curveLineTypeChanged(XYCurve::LineType);
	void curveLineSkipGapsChanged(bool);
	void curveLineIncreasingXOnlyChanged(bool);
	void curveLineInterpolationPointsCountChanged(int);

	//"Margin Plots"-Tab
	void curveRugEnabledChanged(bool);
	void curveRugOrientationChanged(WorksheetElement::Orientation);
	void curveRugLengthChanged(double);
	void curveRugWidthChanged(double);
	void curveRugOffsetChanged(double);

	// load and save
	void loadConfigFromTemplate(KConfig&);
	void saveConfigAsTemplate(KConfig&);

Q_SIGNALS:
	void info(const QString&);

	friend class MultiRangeTest;
};

#endif
