/*
	File                 : CartesianPlotDock.h
	Project              : LabPlot
	Description          : widget for cartesian plot properties
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2011-2025 Alexander Semke <alexander.semke@web.de>
	SPDX-FileCopyrightText: 2012-2021 Stefan Gerlach <stefan.gerlach@uni.kn>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef CARTESIANPLOTDOCK_H
#define CARTESIANPLOTDOCK_H

#include "backend/worksheet/plots/PlotArea.h"
#include "backend/worksheet/plots/cartesian/CartesianPlot.h"
#include "frontend/dockwidgets/BaseDock.h"
#include "ui_cartesianplotdock.h"

template<class T>
class QList;
class BackgroundWidget;
class LabelWidget;
class LineWidget;
class ThemeHandler;
class KConfig;

class CartesianPlotDock : public BaseDock {
	Q_OBJECT

public:
	explicit CartesianPlotDock(QWidget*);
	void setPlots(QList<CartesianPlot*>);
	void activateTitleTab();

	void updateLocale() override;
	void retranslateUi() override;
	void updateUnits() override;

	void updateRangeList(const Dimension);
	void updatePlotRangeList() override;
	void updatePlotRangeListValues(const Dimension dim, int rangeIndex);

private:
	Ui::CartesianPlotDock ui;
	BackgroundWidget* backgroundWidget{nullptr};
	QList<CartesianPlot*> m_plotList;
	CartesianPlot* m_plot{nullptr};
	LabelWidget* labelWidget{nullptr};
	LineWidget* borderLineWidget{nullptr};
	LineWidget* cursorLineWidget{nullptr};
	ThemeHandler* m_themeHandler;
	QButtonGroup* m_bgDefaultPlotRange{nullptr};
	bool m_autoScale{false};
	bool m_updateUI{true};

	void autoScaleRange(const Dimension, const int index, bool);
	void load();
	void loadConfig(KConfig&);

private Q_SLOTS:
	void init();

	// SLOTs for changes triggered in CartesianPlotDock
	//"General"-tab
	void plotColorModeChanged(int);
	void plotColorMapChanged(const QString&);
	void selectColorMap();
	void rangeTypeChanged(int);
	void niceExtendChanged(bool);
	void rangePointsChanged(const QString&);

	void autoScaleChanged(const Dimension, const int rangeIndex, bool);
	void minDateTimeChanged(const QObject* sender, const Dimension, qint64);
	void maxDateTimeChanged(const QObject* sender, const Dimension, qint64);
	// void xRangeDateTimeChanged(const Range<quint64>&);
	void rangeFormatChanged(const QObject* sender, const Dimension, int index);
	void scaleChanged(const QObject* sender, const Dimension, int);
	void addXRange();
	void addYRange();
	void removeRange(const Dimension dim);
	void removeXRange();
	void removeYRange();
	void addPlotRange();
	void removePlotRange();
	void PlotRangeChanged(const int cSystemIndex, const Dimension, const int index);
	void PlotRangeXChanged(const int index);
	void PlotRangeYChanged(const int index);

	void minChanged(const Dimension dim, const int index, double min);
	void maxChanged(const Dimension dim, const int index, double max);
	// void yRangeDateTimeChanged(const Range<quint64>&);

	// "Layout"-tab
	void geometryChanged();
	void layoutChanged(Worksheet::Layout);
	void symmetricPaddingChanged(bool);
	void horizontalPaddingChanged(double);
	void rightPaddingChanged(double);
	void verticalPaddingChanged(double);
	void bottomPaddingChanged(double);

	//"Range Breaks"-tab
	void toggleXBreak(bool);
	void addXBreak();
	void removeXBreak();
	void currentXBreakChanged(int);
	void xBreakStartChanged();
	void xBreakEndChanged();
	void xBreakPositionChanged(int);
	void xBreakStyleChanged(int);

	void toggleYBreak(bool);
	void addYBreak();
	void removeYBreak();
	void currentYBreakChanged(int);
	void yBreakStartChanged();
	void yBreakEndChanged();
	void yBreakPositionChanged(int);
	void yBreakStyleChanged(int);

	//"Plot area"-tab
	void borderTypeChanged();
	void borderCornerRadiusChanged(double);

	void exportPlotTemplate();

	// SLOTs for changes triggered in CartesianPlot
	// general
	void plotPlotColorModeChanged(CartesianPlot::PlotColorMode);
	void plotPlotColorMapChanged(const QString&);
	void plotRangeTypeChanged(CartesianPlot::RangeType);
	void plotRangeFirstValuesChanged(int);
	void plotRangeLastValuesChanged(int);

	void plotAutoScaleChanged(const Dimension, int, bool);
	void plotMinChanged(const Dimension, int rangeIndex, double);
	void plotMaxChanged(const Dimension, int rangeIndex, double);
	void plotRangeChanged(const Dimension, int, Range<double>);
	void plotRangeFormatChanged(const Dimension, int rangeIndex, RangeT::Format);
	void plotScaleChanged(const Dimension, int rangeIndex, RangeT::Scale);

	void defaultPlotRangeChanged();

	// layout
	void plotRectChanged(QRectF&);
	void plotHorizontalPaddingChanged(double);
	void plotVerticalPaddingChanged(double);
	void plotRightPaddingChanged(double);
	void plotBottomPaddingChanged(double);
	void plotSymmetricPaddingChanged(bool);

	// range breaks
	void plotXRangeBreakingEnabledChanged(bool);
	void plotXRangeBreaksChanged(const CartesianPlot::RangeBreaks&);
	void plotYRangeBreakingEnabledChanged(bool);
	void plotYRangeBreaksChanged(const CartesianPlot::RangeBreaks&);

	// background
	void plotBorderTypeChanged(PlotArea::BorderType);
	void plotBorderCornerRadiusChanged(double);

	// save/load template
	void loadConfigFromTemplate(KConfig&);
	void saveConfigAsTemplate(KConfig&);

	// save/load themes
	void loadTheme(const QString&);
	void saveTheme(KConfig&) const;

Q_SIGNALS:
	void info(const QString&);

	friend class RetransformTest;
	friend class SpinBoxTest;
};

#endif
