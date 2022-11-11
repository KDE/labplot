/*
	File                 : CartesianPlotDock.h
	Project              : LabPlot
	Description          : widget for cartesian plot properties
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2011-2022 Alexander Semke <alexander.semke@web.de>
	SPDX-FileCopyrightText: 2012-2021 Stefan Gerlach <stefan.gerlach@uni.kn>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef CARTESIANPLOTDOCK_H
#define CARTESIANPLOTDOCK_H

#include "backend/worksheet/plots/PlotArea.h"
#include "backend/worksheet/plots/cartesian/CartesianPlot.h"
#include "kdefrontend/dockwidgets/BaseDock.h"
#include "ui_cartesianplotdock.h"

#include <KConfig>

template<class T>
class QList;
class BackgroundWidget;
class LabelWidget;
class LineWidget;
class ThemeHandler;

class CartesianPlotDock : public BaseDock {
	Q_OBJECT

public:
	explicit CartesianPlotDock(QWidget*);
	void setPlots(QList<CartesianPlot*>);
	void activateTitleTab();
	void updateLocale() override;
	void updateUnits() override;
	void updateRangeList(const Dimension dim);
	void updatePlotRangeList();

private:
	Ui::CartesianPlotDock ui;
	BackgroundWidget* backgroundWidget{nullptr};
	QList<CartesianPlot*> m_plotList;
	CartesianPlot* m_plot{nullptr};
	LabelWidget* labelWidget{nullptr};
	LineWidget* cursorLineWidget{nullptr};
	ThemeHandler* m_themeHandler;
	QButtonGroup* m_bgDefaultPlotRange{nullptr};
	bool m_autoScale{false};
	bool m_updateUI{true};

	void autoScaleRange(const Dimension, const int index, bool);
	void loadConfig(KConfig&);

private Q_SLOTS:
	void init();
	void retranslateUi();

	// SLOTs for changes triggered in CartesianPlotDock
	//"General"-tab
	void visibilityChanged(bool);

	void rangeTypeChanged(int);
	void niceExtendChanged(bool checked);
	void rangePointsChanged(const QString&);

	void autoScaleChanged(const Dimension, const int rangeIndex, bool);
	void minDateTimeChanged(const QObject* sender, const Dimension, const QDateTime&);
	void maxDateTimeChanged(const QObject* sender, const Dimension, const QDateTime&);
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
	void borderStyleChanged(int);
	void borderColorChanged(const QColor&);
	void borderWidthChanged(double);
	void borderCornerRadiusChanged(double);
	void borderOpacityChanged(int);

	void exportPlotTemplate();

	// SLOTs for changes triggered in CartesianPlot
	// general

	void plotRangeTypeChanged(CartesianPlot::RangeType);
	void plotRangeFirstValuesChanged(int);
	void plotRangeLastValuesChanged(int);

	void plotAutoScaleChanged(const Dimension, int, bool);
	void plotXMinChanged(int xRangeIndex, double);
	void plotYMinChanged(int yRangeIndex, double);
	void plotXMaxChanged(int xRangeIndex, double);
	void plotYMaxChanged(int yRangeIndex, double);
	void plotRangeChanged(const Dimension, int, Range<double>);
	void plotRangeFormatChanged(const Dimension, int rangeIndex, RangeT::Format format);
	void plotScaleChanged(const Dimension, int xRangeIndex, RangeT::Scale);

	void defaultPlotRangeChanged();

	void plotVisibleChanged(bool);

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
	void plotBorderPenChanged(QPen&);
	void plotBorderCornerRadiusChanged(double);
	void plotBorderOpacityChanged(double);

	// save/load template
	void loadConfigFromTemplate(KConfig&);
	void saveConfigAsTemplate(KConfig&);

	// save/load themes
	void loadTheme(const QString&);
	void saveTheme(KConfig&) const;

	void load();

Q_SIGNALS:
	void info(const QString&);

	friend class RetransformTest;
};

#endif
