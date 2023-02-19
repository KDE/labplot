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
class BackgroundWidget;
class LineWidget;
class SymbolWidget;
class ValueWidget;
class TreeViewComboBox;

class HistogramDock : public BaseDock {
	Q_OBJECT

public:
	explicit HistogramDock(QWidget*);
	~HistogramDock() override;

	void setCurves(QList<Histogram*>);

private:
	TreeViewComboBox* cbDataColumn;
	TreeViewComboBox* cbErrorPlusColumn;
	TreeViewComboBox* cbErrorMinusColumn;

	void updateValuesWidgets();
	void updatePlotRanges() override;
	void updateLocale() override;
	void loadConfig(KConfig&);

protected:
	Ui::HistogramDock ui;
	BackgroundWidget* backgroundWidget{nullptr};
	LineWidget* lineWidget{nullptr};
	SymbolWidget* symbolWidget{nullptr};
	ValueWidget* valueWidget{nullptr};
	LineWidget* errorBarsLineWidget{nullptr};

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
	void binRangesMinDateTimeChanged(qint64);
	void binRangesMaxDateTimeChanged(qint64);

	//"Error bars"-Tab
	void errorTypeChanged(int);
	void errorPlusColumnChanged(const QModelIndex&);
	void errorMinusColumnChanged(const QModelIndex&);

	//"Margin Plots"-Tab
	void rugEnabledChanged(bool);
	void rugLengthChanged(Common::ExpressionValue) const;
	void rugWidthChanged(Common::ExpressionValue) const;
	void rugOffsetChanged(Common::ExpressionValue) const;

	// SLOTs for changes triggered in Histogram
	// General-Tab
	void curveDataColumnChanged(const AbstractColumn*);
	void curveTypeChanged(Histogram::Type);
	void curveOrientationChanged(Histogram::Orientation);
	void curveNormalizationChanged(Histogram::Normalization);
	void curveBinningMethodChanged(Histogram::BinningMethod);
	void curveBinCountChanged(int);
	void curveBinWidthChanged(double);
	void curveAutoBinRangesChanged(bool);
	void curveBinRangesMinChanged(double);
	void curveBinRangesMaxChanged(double);
	void curveVisibilityChanged(bool);

	//"Error bars"-Tab
	void curveErrorTypeChanged(Histogram::ErrorType);
	void curveErrorPlusColumnChanged(const AbstractColumn*);
	void curveErrorMinusColumnChanged(const AbstractColumn*);

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
