/*
    File             : XYCurveDock.h
    Project          : LabPlot
    Description      : widget for curve properties
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2010-2021 Alexander Semke <alexander.semke@web.de>
    SPDX-FileCopyrightText: 2013 Stefan Gerlach <stefan.gerlach@uni.kn>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef XYCURVEDOCK_H
#define XYCURVEDOCK_H

#include "kdefrontend/dockwidgets/BaseDock.h"
#include "backend/core/AbstractColumn.h"
#include "backend/worksheet/plots/cartesian/XYCurve.h"

#include "ui_xycurvedock.h"
#include "ui_xycurvedockgeneraltab.h"

class AspectTreeModel;
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
	void updatePlotRanges() const override;

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
	QList<XYCurve*> m_curvesList;
	XYCurve* m_curve{nullptr};
	AspectTreeModel* m_aspectTreeModel{nullptr};
	SymbolWidget* symbolWidget{nullptr};

public Q_SLOTS:
	void visibilityChanged(bool);

private Q_SLOTS:
	void init();
	void retranslateUi();

	//SLOTs for changes triggered in XYCurveDock
	void xColumnChanged(const QModelIndex&);
	void yColumnChanged(const QModelIndex&);
	void legendVisibleChanged(bool);

	//Line-Tab
	void lineTypeChanged(int);
	void lineSkipGapsChanged(bool);
	void lineIncreasingXOnlyChanged(bool);
	void lineInterpolationPointsCountChanged(int);
	void lineStyleChanged(int);
	void lineColorChanged(const QColor&);
	void lineWidthChanged(double);
	void lineOpacityChanged(int);

	void dropLineTypeChanged(int);
	void dropLineStyleChanged(int);
	void dropLineColorChanged(const QColor&);
	void dropLineWidthChanged(double);
	void dropLineOpacityChanged(int);

	//Values-Tab
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

	//Filling-tab
	void fillingPositionChanged(int);
	void fillingTypeChanged(int);
	void fillingColorStyleChanged(int);
	void fillingImageStyleChanged(int);
	void fillingBrushStyleChanged(int);
	void fillingFirstColorChanged(const QColor&);
	void fillingSecondColorChanged(const QColor&);
	void selectFile();
	void fileNameChanged();
	void fillingOpacityChanged(int);

	//"Error bars"-Tab
	void xErrorTypeChanged(int) const;
	void yErrorTypeChanged(int) const;
	void xErrorPlusColumnChanged(const QModelIndex&) const;
	void xErrorMinusColumnChanged(const QModelIndex&) const;
	void yErrorPlusColumnChanged(const QModelIndex&) const;
	void yErrorMinusColumnChanged(const QModelIndex&) const;
	void errorBarsTypeChanged(int) const;
	void errorBarsCapSizeChanged(double) const;
	void errorBarsStyleChanged(int) const;
	void errorBarsColorChanged(const QColor&);
	void errorBarsWidthChanged(double) const;
	void errorBarsOpacityChanged(int) const;

	//"Margin Plots"-Tab
	void rugEnabledChanged(int) const;
	void rugOrientationChanged(int) const;
	void rugLengthChanged(double) const;
	void rugWidthChanged(double) const;
	void rugOffsetChanged(double) const;

	//SLOTs for changes triggered in XYCurve
	//General-Tab
	void curveDescriptionChanged(const AbstractAspect*);
	void curveXColumnChanged(const AbstractColumn*);
	void curveYColumnChanged(const AbstractColumn*);
	void curveLegendVisibleChanged(bool);
	void curveVisibilityChanged(bool);

	//Line-Tab
	void curveLineTypeChanged(XYCurve::LineType);
	void curveLineSkipGapsChanged(bool);
	void curveLineIncreasingXOnlyChanged(bool);
	void curveLineInterpolationPointsCountChanged(int);
	void curveLinePenChanged(const QPen&);
	void curveLineOpacityChanged(qreal);
	void curveDropLineTypeChanged(XYCurve::DropLineType);
	void curveDropLinePenChanged(const QPen&);
	void curveDropLineOpacityChanged(qreal);

	//Values-Tab
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

	//Filling-Tab
	void curveFillingPositionChanged(XYCurve::FillingPosition);
	void curveFillingTypeChanged(WorksheetElement::BackgroundType);
	void curveFillingColorStyleChanged(WorksheetElement::BackgroundColorStyle);
	void curveFillingImageStyleChanged(WorksheetElement::BackgroundImageStyle);
	void curveFillingBrushStyleChanged(Qt::BrushStyle);
	void curveFillingFirstColorChanged(QColor&);
	void curveFillingSecondColorChanged(QColor&);
	void curveFillingFileNameChanged(QString&);
	void curveFillingOpacityChanged(float);

	//"Error bars"-Tab
	void curveXErrorTypeChanged(XYCurve::ErrorType);
	void curveXErrorPlusColumnChanged(const AbstractColumn*);
	void curveXErrorMinusColumnChanged(const AbstractColumn*);
	void curveYErrorTypeChanged(XYCurve::ErrorType);
	void curveYErrorPlusColumnChanged(const AbstractColumn*);
	void curveYErrorMinusColumnChanged(const AbstractColumn*);
	void curveErrorBarsCapSizeChanged(qreal);
	void curveErrorBarsTypeChanged(XYCurve::ErrorBarsType);
	void curveErrorBarsPenChanged(const QPen&);
	void curveErrorBarsOpacityChanged(qreal);

	//"Margin Plots"-Tab
	void curveRugEnabledChanged(bool);
	void curveRugOrientationChanged(WorksheetElement::Orientation);
	void curveRugLengthChanged(double);
	void curveRugWidthChanged(double);
	void curveRugOffsetChanged(double);

	//load and save
	void loadConfigFromTemplate(KConfig&);
	void saveConfigAsTemplate(KConfig&);

Q_SIGNALS:
	void info(const QString&);
};

#endif
