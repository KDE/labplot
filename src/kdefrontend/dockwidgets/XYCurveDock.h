/*
	File             : XYCurveDock.h
	Project          : LabPlot
	Description      : widget for curve properties
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2010-2022 Alexander Semke <alexander.semke@web.de>
	SPDX-FileCopyrightText: 2013 Stefan Gerlach <stefan.gerlach@uni.kn>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef XYCURVEDOCK_H
#define XYCURVEDOCK_H

#include "backend/core/AbstractColumn.h"
#include "backend/worksheet/plots/cartesian/XYCurve.h"
#include "kdefrontend/dockwidgets/BaseDock.h"

#include "ui_xycurvedock.h"
#include "ui_xycurvedockgeneraltab.h"

class AspectTreeModel;
class BackgroundWidget;
class LineWidget;
class Column;
class SymbolWidget;
class TreeViewComboBox;

class XYCurveDock : public BaseDock {
	Q_OBJECT

public:
	explicit XYCurveDock(QWidget*);
	~XYCurveDock() override;

	void setCurves(QList<XYCurve*>);
	virtual void setupGeneral();
	void updateLocale() override;

private:
	virtual void initGeneralTab();
	void updateValuesWidgets();
	void updatePlotRanges() override;

	void load();
	void loadConfig(KConfig&);

	Ui::XYCurveDockGeneralTab uiGeneralTab;

	TreeViewComboBox* cbXColumn{nullptr};
	TreeViewComboBox* cbYColumn{nullptr};
	TreeViewComboBox* cbValuesColumn;
	TreeViewComboBox* cbXErrorPlusColumn;
	TreeViewComboBox* cbXErrorMinusColumn;
	TreeViewComboBox* cbYErrorPlusColumn;
	TreeViewComboBox* cbYErrorMinusColumn;

protected:
	void initTabs();
	virtual void setModel();
	void setSymbols(QList<XYCurve*>);

	Ui::XYCurveDock ui;
	LineWidget* lineWidget{nullptr};
	LineWidget* dropLineWidget{nullptr};
	BackgroundWidget* backgroundWidget{nullptr};
	SymbolWidget* symbolWidget{nullptr};
	LineWidget* errorBarsLineWidget{nullptr};
	QList<XYCurve*> m_curvesList;
	XYCurve* m_curve{nullptr};
	AspectTreeModel* m_aspectTreeModel{nullptr};

public Q_SLOTS:
	void visibilityChanged(bool);

private Q_SLOTS:
	void init();
	void retranslateUi();

	// SLOTs for changes triggered in XYCurveDock
	void xColumnChanged(const QModelIndex&);
	void yColumnChanged(const QModelIndex&);
	void legendVisibleChanged(bool);

	// Line-Tab
	void lineTypeChanged(int);
	void lineSkipGapsChanged(bool);
	void lineIncreasingXOnlyChanged(bool);
	void lineInterpolationPointsCountChanged(int);

	// Values-Tab
	void valuesTypeChanged(int);
	void valuesColumnChanged(const QModelIndex&);
	void valuesPositionChanged(int);
	void valuesDistanceChanged(double);
	void valuesRotationChanged(int);
	void valuesOpacityChanged(int);
	void valuesNumericFormatChanged(int);
	void valuesPrecisionChanged(int);
	void valuesDateTimeFormatChanged(const QString&);
	void valuesPrefixChanged();
	void valuesSuffixChanged();
	void valuesFontChanged(const QFont&);
	void valuesColorChanged(const QColor&);

	//"Error bars"-Tab
	void xErrorTypeChanged(int);
	void yErrorTypeChanged(int);
	void xErrorPlusColumnChanged(const QModelIndex&);
	void xErrorMinusColumnChanged(const QModelIndex&);
	void yErrorPlusColumnChanged(const QModelIndex&);
	void yErrorMinusColumnChanged(const QModelIndex&);

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
	void curveLegendVisibleChanged(bool);
	void curveVisibilityChanged(bool);

	// Line-Tab
	void curveLineTypeChanged(XYCurve::LineType);
	void curveLineSkipGapsChanged(bool);
	void curveLineIncreasingXOnlyChanged(bool);
	void curveLineInterpolationPointsCountChanged(int);

	// Values-Tab
	void curveValuesTypeChanged(XYCurve::ValuesType);
	void curveValuesColumnChanged(const AbstractColumn*);
	void curveValuesPositionChanged(XYCurve::ValuesPosition);
	void curveValuesDistanceChanged(qreal);
	void curveValuesOpacityChanged(qreal);
	void curveValuesRotationAngleChanged(qreal);
	void curveValuesNumericFormatChanged(char);
	void curveValuesPrecisionChanged(int);
	void curveValuesDateTimeFormatChanged(const QString&);
	void curveValuesPrefixChanged(const QString&);
	void curveValuesSuffixChanged(const QString&);
	void curveValuesFontChanged(QFont);
	void curveValuesColorChanged(QColor);

	//"Error bars"-Tab
	void curveXErrorTypeChanged(XYCurve::ErrorType);
	void curveXErrorPlusColumnChanged(const AbstractColumn*);
	void curveXErrorMinusColumnChanged(const AbstractColumn*);
	void curveYErrorTypeChanged(XYCurve::ErrorType);
	void curveYErrorPlusColumnChanged(const AbstractColumn*);
	void curveYErrorMinusColumnChanged(const AbstractColumn*);

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
};

#endif
