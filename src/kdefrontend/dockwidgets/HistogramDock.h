/*
	File                 : HistogramDock.h
	Project              : LabPlot
	Description          : widget for histogram plot properties
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2016 Anu Mittal <anu22mittal@gmail.com>
	SPDX-FileCopyrightText: 2016-2022 Alexander Semke <alexander.semke@web.de>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef HISTOGRAMDOCK_H
#define HISTOGRAMDOCK_H

#include "backend/worksheet/plots/cartesian/Histogram.h"
#include "kdefrontend/dockwidgets/BaseDock.h"
#include "ui_histogramdock.h"

class AspectTreeModel;
class Column;
class Histogram;
class BackgroundWidget;
class SymbolWidget;
class TreeViewComboBox;

class HistogramDock : public BaseDock {
	Q_OBJECT

public:
	explicit HistogramDock(QWidget*);
	~HistogramDock() override;

	void setCurves(QList<Histogram*>);

private:
	QStringList dateStrings;
	QStringList timeStrings;

	TreeViewComboBox* cbDataColumn;
	TreeViewComboBox* cbValuesColumn;
	TreeViewComboBox* cbErrorPlusColumn;
	TreeViewComboBox* cbErrorMinusColumn;

	void updateValuesWidgets();
	void updatePlotRanges() override;
	void loadConfig(KConfig&);

protected:
	Ui::HistogramDock ui;
	BackgroundWidget* backgroundWidget{nullptr};
	SymbolWidget* symbolWidget{nullptr};

	QList<Histogram*> m_curvesList;
	Histogram* m_curve{nullptr};
	AspectTreeModel* m_aspectTreeModel{nullptr};

	virtual void setModel();

private Q_SLOTS:
	void init();
	void retranslateUi();

	// SLOTs for changes triggered in HistogramDock

	// General-Tab
	void dataColumnChanged(const QModelIndex&);
	void visibilityChanged(bool);
	void typeChanged(int);
	void orientationChanged(int);
	void normalizationChanged(int);
	void binningMethodChanged(int);
	void binCountChanged(int);
	void binWidthChanged();
	void autoBinRangesChanged(bool);
	void binRangesMinChanged(const QString&);
	void binRangesMaxChanged(const QString&);
	void binRangesMinDateTimeChanged(const QDateTime&);
	void binRangesMaxDateTimeChanged(const QDateTime&);
	void plotRangeChanged(int);

	// Lines-Tab
	void lineTypeChanged(int);
	void lineStyleChanged(int);
	void lineColorChanged(const QColor&);
	void lineWidthChanged(double);
	void lineOpacityChanged(int);

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
	void errorTypeChanged(int) const;
	void errorPlusColumnChanged(const QModelIndex&) const;
	void errorMinusColumnChanged(const QModelIndex&) const;
	void errorBarsTypeChanged(int) const;
	void errorBarsCapSizeChanged(double) const;
	void errorBarsStyleChanged(int) const;
	void errorBarsColorChanged(const QColor&);
	void errorBarsWidthChanged(double) const;
	void errorBarsOpacityChanged(int) const;

	//"Margin Plots"-Tab
	void rugEnabledChanged(bool) const;
	void rugLengthChanged(double) const;
	void rugWidthChanged(double) const;
	void rugOffsetChanged(double) const;

	// SLOTs for changes triggered in Histogram
	// General-Tab
	void curveDataColumnChanged(const AbstractColumn*);
	void curveTypeChanged(Histogram::HistogramType);
	void curveOrientationChanged(Histogram::HistogramOrientation);
	void curveNormalizationChanged(Histogram::HistogramNormalization);
	void curveBinningMethodChanged(Histogram::BinningMethod);
	void curveBinCountChanged(int);
	void curveBinWidthChanged(double);
	void curveAutoBinRangesChanged(bool);
	void curveBinRangesMinChanged(double);
	void curveBinRangesMaxChanged(double);
	void curveVisibilityChanged(bool);

	// Line-tab
	void curveLineTypeChanged(Histogram::LineType);
	void curveLinePenChanged(const QPen&);
	void curveLineOpacityChanged(qreal);

	// Values-Tab
	void curveValuesTypeChanged(Histogram::ValuesType);
	void curveValuesColumnChanged(const AbstractColumn*);
	void curveValuesPositionChanged(Histogram::ValuesPosition);
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
	void curveErrorTypeChanged(Histogram::ErrorType);
	void curveErrorPlusColumnChanged(const AbstractColumn*);
	void curveErrorMinusColumnChanged(const AbstractColumn*);
	void curveErrorBarsTypeChanged(XYCurve::ErrorBarsType);
	void curveErrorBarsPenChanged(const QPen&);
	void curveErrorBarsCapSizeChanged(qreal);
	void curveErrorBarsOpacityChanged(qreal);

	//"Margin Plots"-Tab
	void curveRugEnabledChanged(bool);
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
