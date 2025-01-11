/*
	File                 : HistogramDock.h
	Project              : LabPlot
	Description          : widget for histogram plot properties
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2016 Anu Mittal <anu22mittal@gmail.com>
	SPDX-FileCopyrightText: 2016-2024 Alexander Semke <alexander.semke@web.de>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef HISTOGRAMDOCK_H
#define HISTOGRAMDOCK_H

#include "backend/worksheet/plots/cartesian/Histogram.h"
#include "frontend/dockwidgets/BaseDock.h"
#include "ui_histogramdock.h"

class BackgroundWidget;
class ErrorBarWidget;
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
	TreeViewComboBox* cbDataColumn{nullptr};

	void updateValuesWidgets();
	void updateLocale() override;
	void load();
	void loadConfig(KConfig&);

protected:
	Ui::HistogramDock ui;
	BackgroundWidget* backgroundWidget{nullptr};
	LineWidget* lineWidget{nullptr};
	SymbolWidget* symbolWidget{nullptr};
	ValueWidget* valueWidget{nullptr};
	ErrorBarWidget* errorBarWidget{nullptr};
	QList<Histogram*> m_curvesList;
	Histogram* m_curve{nullptr};

	virtual void setModel();

private Q_SLOTS:
	void init();
	void retranslateUi();

	// SLOTs for changes triggered in HistogramDock

	// General-Tab
	void dataColumnChanged(const QModelIndex&);
	void typeChanged(int);
	void orientationChanged(int);
	void normalizationChanged(int);
	void binningMethodChanged(int);
	void binCountChanged(int);
	void binWidthChanged(double);
	void autoBinRangesChanged(bool);
	void binRangesMinChanged(double);
	void binRangesMaxChanged(double);
	void binRangesMinDateTimeChanged(qint64);
	void binRangesMaxDateTimeChanged(qint64);

	//"Margin Plots"-Tab
	void rugEnabledChanged(bool);
	void rugLengthChanged(double) const;
	void rugWidthChanged(double) const;
	void rugOffsetChanged(double) const;

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
