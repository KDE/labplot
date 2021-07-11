/***************************************************************************
    File                 : CartesianPlotDock.h
    Project              : LabPlot
    Description          : widget for cartesian plot properties
    --------------------------------------------------------------------
    Copyright            : (C) 2011-2020 Alexander Semke (alexander.semke@web.de)
    Copyright            : (C) 2012-2021 Stefan Gerlach (stefan.gerlach@uni.kn)

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

#ifndef CARTESIANPLOTDOCK_H
#define CARTESIANPLOTDOCK_H

#include "backend/worksheet/Worksheet.h"
#include "backend/worksheet/plots/cartesian/CartesianPlot.h"
#include "ui_cartesianplotdock.h"
#include "kdefrontend/dockwidgets/BaseDock.h"

#include <KConfig>

template <class T> class QList;
class LabelWidget;
class ThemeHandler;
class KLocalizedString;

class CartesianPlotDock : public BaseDock {
	Q_OBJECT

public:
	explicit CartesianPlotDock(QWidget*);
	void setPlots(QList<CartesianPlot*>);
	void activateTitleTab();
	void updateLocale() override;
	void updateUnits() override;
	void updateXRangeList();
	void updateYRangeList();
	void updatePlotRangeList();

private:
	Ui::CartesianPlotDock ui;
	QList<CartesianPlot*> m_plotList;
	CartesianPlot* m_plot{nullptr};
	LabelWidget* labelWidget{nullptr};
	ThemeHandler* m_themeHandler;
	QButtonGroup* m_bgDefaultPlotRange{nullptr};
	bool m_autoScale{false};

	void autoScaleXRange(int rangeIndex, bool);
	void autoScaleYRange(int rangeIndex, bool);
	void loadConfig(KConfig&);

private slots:
	void init();
	void retranslateUi();

	//SLOTs for changes triggered in CartesianPlotDock
	//"General"-tab
	void visibilityChanged(bool);
	void geometryChanged();
	void layoutChanged(Worksheet::Layout);

	void rangeTypeChanged();
	void rangeFirstChanged(const QString&);
	void rangeLastChanged(const QString&);

	void autoScaleXChanged(int);
	void xMinChanged(const QString&);
	void xMaxChanged(const QString&);
	void xRangeChanged(const Range<double>&);
	void xMinDateTimeChanged(const QDateTime&);
	void xMaxDateTimeChanged(const QDateTime&);
	//void xRangeDateTimeChanged(const Range<quint64>&);
	void xRangeFormatChanged(int);
	void xScaleChanged(int);
	void addXRange();
	void addYRange();
	void removeXRange();
	void removeYRange();
	void addPlotRange();
	void removePlotRange();
	void PlotRangeXChanged(const int index);
	void PlotRangeYChanged(const int index);

	void autoScaleYChanged(int);
	void yMinChanged(const QString&);
	void yMaxChanged(const QString&);
	void yRangeChanged(const Range<double>&);
	void yMinDateTimeChanged(const QDateTime&);
	void yMaxDateTimeChanged(const QDateTime&);
	//void yRangeDateTimeChanged(const Range<quint64>&);
	void yRangeFormatChanged(int);
	void yScaleChanged(int);

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
	void backgroundTypeChanged(int);
	void backgroundColorStyleChanged(int);
	void backgroundImageStyleChanged(int);
	void backgroundBrushStyleChanged(int);
	void backgroundFirstColorChanged(const QColor&);
	void backgroundSecondColorChanged(const QColor&);
	void selectFile();
	void fileNameChanged();
	void backgroundOpacityChanged(int);
	void borderTypeChanged();
	void borderStyleChanged(int);
	void borderColorChanged(const QColor&);
	void borderWidthChanged(double);
	void borderCornerRadiusChanged(double);
	void borderOpacityChanged(int);
	void symmetricPaddingChanged(bool);
	void horizontalPaddingChanged(double);
	void rightPaddingChanged(double);
	void verticalPaddingChanged(double);
	void bottomPaddingChanged(double);

	// "Cursor"-tab
	void cursorLineWidthChanged(int);
	void cursorLineColorChanged(const QColor&);
	void cursorLineStyleChanged(int);

	//SLOTs for changes triggered in CartesianPlot
	//general
	void plotRectChanged(QRectF&);
	void plotRangeTypeChanged(CartesianPlot::RangeType);
	void plotRangeFirstValuesChanged(int);
	void plotRangeLastValuesChanged(int);

	void plotXAutoScaleChanged(int xRangeIndex, bool);
	void plotYAutoScaleChanged(int yRangeIndex, bool);
	void plotXMinChanged(int xRangeIndex, double);
	void plotYMinChanged(int yRangeIndex, double);
	void plotXMaxChanged(int xRangeIndex, double);
	void plotYMaxChanged(int yRangeIndex, double);
	void plotXRangeChanged(int xRangeIndex, Range<double>);
	void plotYRangeChanged(int yRangeIndex, Range<double>);
	void plotXRangeFormatChanged(int xRangeIndex, RangeT::Format);
	void plotYRangeFormatChanged(int yRangeIndex, RangeT::Format);
	void plotXScaleChanged(int xRangeIndex, RangeT::Scale);
	void plotYScaleChanged(int yRangeIndex, RangeT::Scale);

	void defaultPlotRangeChanged();

	void plotVisibleChanged(bool);

	//range breaks
	void plotXRangeBreakingEnabledChanged(bool);
	void plotXRangeBreaksChanged(const CartesianPlot::RangeBreaks&);
	void plotYRangeBreakingEnabledChanged(bool);
	void plotYRangeBreaksChanged(const CartesianPlot::RangeBreaks&);

	//background
	void plotBackgroundTypeChanged(PlotArea::BackgroundType);
	void plotBackgroundColorStyleChanged(PlotArea::BackgroundColorStyle);
	void plotBackgroundImageStyleChanged(PlotArea::BackgroundImageStyle);
	void plotBackgroundBrushStyleChanged(Qt::BrushStyle);
	void plotBackgroundFirstColorChanged(QColor&);
	void plotBackgroundSecondColorChanged(QColor&);
	void plotBackgroundFileNameChanged(QString&);
	void plotBackgroundOpacityChanged(float);
	void plotBorderTypeChanged(PlotArea::BorderType);
	void plotBorderPenChanged(QPen&);
	void plotBorderCornerRadiusChanged(float);
	void plotBorderOpacityChanged(float);
	void plotHorizontalPaddingChanged(float);
	void plotVerticalPaddingChanged(float);
	void plotRightPaddingChanged(double);
	void plotBottomPaddingChanged(double);
	void plotSymmetricPaddingChanged(bool);

	// Cursor
	void plotCursorPenChanged(const QPen&);

	//save/load template
	void loadConfigFromTemplate(KConfig&);
	void saveConfigAsTemplate(KConfig&);

	//save/load themes
	void loadTheme(const QString&);
	void saveTheme(KConfig&) const;

	void load();

signals:
	void info(const QString&);
};

#endif
