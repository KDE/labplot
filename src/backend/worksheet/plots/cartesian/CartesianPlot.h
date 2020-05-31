
/***************************************************************************
    File                 : CartesianPlot.h
    Project              : LabPlot
    Description          : Cartesian plot
    --------------------------------------------------------------------
    Copyright            : (C) 2011-2020 Alexander Semke (alexander.semke@web.de)
    Copyright            : (C) 2012-2019 Stefan Gerlach (stefan.gerlach@uni.kn)

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

#ifndef CARTESIANPLOT_H
#define CARTESIANPLOT_H

#include "backend/worksheet/plots/AbstractPlot.h"
#include "backend/worksheet/plots/cartesian/Histogram.h"

#include <cmath>

class QDropEvent;
class QToolBar;
class CartesianPlotPrivate;
class CartesianPlotLegend;
class AbstractColumn;
class XYCurve;
class XYEquationCurve;
class XYDataReductionCurve;
class XYDifferentiationCurve;
class XYIntegrationCurve;
class XYInterpolationCurve;
class XYSmoothCurve;
class XYFitCurve;
class XYFourierFilterCurve;
class XYFourierTransformCurve;
class XYConvolutionCurve;
class XYCorrelationCurve;
class KConfig;

class CartesianPlot : public AbstractPlot {
	Q_OBJECT

public:
	explicit CartesianPlot(const QString &name);
	~CartesianPlot() override;

	enum class Scale {Linear, Log10, Log2, Ln, Log10Abs, Log2Abs, LnAbs, Sqrt, X2};
	enum Type {FourAxes, TwoAxes, TwoAxesCentered, TwoAxesCenteredZero};
	enum RangeFormat {Numeric, DateTime};
	enum class RangeType {Free, Last, First};
	enum class RangeBreakStyle {Simple, Vertical, Sloped};
	enum class MouseMode {Selection, ZoomSelection, ZoomXSelection, ZoomYSelection, Cursor};
	enum NavigationOperation {ScaleAuto, ScaleAutoX, ScaleAutoY, ZoomIn, ZoomOut, ZoomInX, ZoomOutX,
	                          ZoomInY, ZoomOutY, ShiftLeftX, ShiftRightX, ShiftUpY, ShiftDownY
	                         };

	struct RangeBreak {
		RangeBreak() : start(NAN), end(NAN), position(0.5), style(RangeBreakStyle::Sloped) {}
		bool isValid() const {
			return (!std::isnan(start) && !std::isnan(end));
		}
		double start;
		double end;
		double position;
		RangeBreakStyle style;
	};

	//simple wrapper for QList<RangeBreaking> in order to get our macros working
	struct RangeBreaks {
		RangeBreaks() : lastChanged(-1) {
			RangeBreak b;
			list << b;
		};
		QList<RangeBreak> list;
		int lastChanged;
	};

	void initDefault(Type = FourAxes);
	QIcon icon() const override;
	QMenu* createContextMenu() override;
	QMenu* analysisMenu();
	QVector<AbstractAspect*> dependsOn() const override;
	void setRect(const QRectF&) override;
	QRectF dataRect() const;
	void setMouseMode(const MouseMode);
	void setLocked(const bool);
	MouseMode mouseMode() const;
	void navigate(NavigationOperation);
	void setSuppressDataChangedSignal(bool);
	const QList<QColor>& themeColorPalette() const;
	void processDropEvent(QDropEvent*) override;
	bool isPanningActive() const;
	bool isHovered() const;
	bool isPrinted() const;
	bool isSelected() const;
	void addLegend(CartesianPlotLegend*);
	int curveCount();
	const XYCurve* getCurve(int index);
	double cursorPos(int cursorNumber);

	void save(QXmlStreamWriter*) const override;
	bool load(XmlStreamReader*, bool preview) override;
	void loadThemeConfig(const KConfig&) override;
	void saveTheme(KConfig& config);
	void mousePressZoomSelectionMode(QPointF logicPos);
	void mousePressCursorMode(int cursorNumber, QPointF logicPos);
	void mouseMoveZoomSelectionMode(QPointF logicPos);
	void mouseMoveCursorMode(int cursorNumber, QPointF logicPos);
	void mouseReleaseZoomSelectionMode();
	void mouseHoverZoomSelectionMode(QPointF logicPos);
	void mouseHoverOutsideDataRect();

	BASIC_D_ACCESSOR_DECL(CartesianPlot::RangeFormat, xRangeFormat, XRangeFormat)
	BASIC_D_ACCESSOR_DECL(CartesianPlot::RangeFormat, yRangeFormat, YRangeFormat)
	const QString& xRangeDateTimeFormat() const;
	const QString& yRangeDateTimeFormat() const;
	BASIC_D_ACCESSOR_DECL(CartesianPlot::RangeType, rangeType, RangeType)
	BASIC_D_ACCESSOR_DECL(int, rangeLastValues, RangeLastValues)
	BASIC_D_ACCESSOR_DECL(int, rangeFirstValues, RangeFirstValues)
	BASIC_D_ACCESSOR_DECL(bool, autoScaleX, AutoScaleX)
	BASIC_D_ACCESSOR_DECL(bool, autoScaleY, AutoScaleY)
	BASIC_D_ACCESSOR_DECL(double, xMin, XMin)
	BASIC_D_ACCESSOR_DECL(double, xMax, XMax)
	BASIC_D_ACCESSOR_DECL(double, yMin, YMin)
	BASIC_D_ACCESSOR_DECL(double, yMax, YMax)
	BASIC_D_ACCESSOR_DECL(CartesianPlot::Scale, xScale, XScale)
	BASIC_D_ACCESSOR_DECL(CartesianPlot::Scale, yScale, YScale)
	BASIC_D_ACCESSOR_DECL(bool, xRangeBreakingEnabled, XRangeBreakingEnabled)
	BASIC_D_ACCESSOR_DECL(bool, yRangeBreakingEnabled, YRangeBreakingEnabled)
	CLASS_D_ACCESSOR_DECL(RangeBreaks, xRangeBreaks, XRangeBreaks)
	CLASS_D_ACCESSOR_DECL(RangeBreaks, yRangeBreaks, YRangeBreaks)
	CLASS_D_ACCESSOR_DECL(QPen, cursorPen, CursorPen);
	CLASS_D_ACCESSOR_DECL(bool, cursor0Enable, Cursor0Enable);
	CLASS_D_ACCESSOR_DECL(bool, cursor1Enable, Cursor1Enable);

	QString theme() const;

	typedef CartesianPlotPrivate Private;

public slots:
	void setTheme(const QString&);

private:
	void init();
	void initActions();
	void initMenus();
	void setColorPalette(const KConfig&);
	const XYCurve* currentCurve() const;
	void shift(bool x, bool leftOrDown);
	void zoom(bool x, bool in);

	void calculateCurvesXMinMax(bool completeRange = true);
	void calculateCurvesYMinMax(bool completeRange = true);

	CartesianPlotLegend* m_legend{nullptr};
	double m_zoomFactor{1.2};
	QList<QColor> m_themeColorPalette;
	bool m_menusInitialized{false};

	QAction* visibilityAction;

	//"add new" actions
	QAction* addCurveAction;
	QAction* addEquationCurveAction;
	QAction* addHistogramAction;
	QAction* addDataReductionCurveAction;
	QAction* addDifferentiationCurveAction;
	QAction* addIntegrationCurveAction;
	QAction* addInterpolationCurveAction;
	QAction* addSmoothCurveAction;
	QAction* addFitCurveAction;
	QAction* addFourierFilterCurveAction;
	QAction* addFourierTransformCurveAction;
	QAction* addConvolutionCurveAction;
	QAction* addCorrelationCurveAction;

	QAction* addHorizontalAxisAction;
	QAction* addVerticalAxisAction;
	QAction* addLegendAction;
	QAction* addTextLabelAction;
	QAction* addImageAction;
	QAction* addCustomPointAction;
	QAction* addReferenceLineAction;

	//scaling, zooming, navigation actions
	QAction* scaleAutoXAction;
	QAction* scaleAutoYAction;
	QAction* scaleAutoAction;
	QAction* zoomInAction;
	QAction* zoomOutAction;
	QAction* zoomInXAction;
	QAction* zoomOutXAction;
	QAction* zoomInYAction;
	QAction* zoomOutYAction;
	QAction* shiftLeftXAction;
	QAction* shiftRightXAction;
	QAction* shiftUpYAction;
	QAction* shiftDownYAction;

	//analysis menu actions
	QAction* addDataOperationAction;
	QAction* addDataReductionAction;
	QAction* addDifferentiationAction;
	QAction* addIntegrationAction;
	QAction* addInterpolationAction;
	QAction* addSmoothAction;
	QVector <QAction*> addFitAction;
	QAction* addFourierFilterAction;
	QAction* addFourierTransformAction;
	QAction* addConvolutionAction;
	QAction* addCorrelationAction;

	QMenu* addNewMenu{nullptr};
	QMenu* addNewAnalysisMenu{nullptr};
	QMenu* zoomMenu{nullptr};
	QMenu* dataAnalysisMenu{nullptr};
	QMenu* themeMenu{nullptr};

	Q_DECLARE_PRIVATE(CartesianPlot)

public slots:
	void addHorizontalAxis();
	void addVerticalAxis();
	void addCurve();
	void addHistogram();
	void addEquationCurve();
	void addDataReductionCurve();
	void addDifferentiationCurve();
	void addIntegrationCurve();
	void addInterpolationCurve();
	void addSmoothCurve();
	void addFitCurve();
	void addFourierFilterCurve();
	void addFourierTransformCurve();
	void addConvolutionCurve();
	void addCorrelationCurve();

	void addLegend();
	void addTextLabel();
	void addImage();
	void addCustomPoint();
	void addReferenceLine();

	void scaleAutoTriggered();
	bool scaleAuto();
	bool scaleAutoX();
	bool scaleAutoY();

	void zoomIn();
	void zoomOut();
	void zoomInX();
	void zoomOutX();
	void zoomInY();
	void zoomOutY();

	void shiftLeftX();
	void shiftRightX();
	void shiftUpY();
	void shiftDownY();

	void cursor();

	void dataChanged();
	void curveLinePenChanged(QPen);

private slots:
	void updateLegend();
	void childAdded(const AbstractAspect*);
	void childRemoved(const AbstractAspect* parent, const AbstractAspect* before, const AbstractAspect* child);
	void childHovered();

	void xDataChanged();
	void yDataChanged();
	void curveVisibilityChanged();

	//SLOTs for changes triggered via QActions in the context menu
	void visibilityChanged();
	void loadTheme(const QString&);

protected:
	CartesianPlot(const QString &name, CartesianPlotPrivate *dd);

signals:
	void rangeTypeChanged(CartesianPlot::RangeType);
	void xRangeFormatChanged(CartesianPlot::RangeFormat);
	void yRangeFormatChanged(CartesianPlot::RangeFormat);
	void rangeLastValuesChanged(int);
	void rangeFirstValuesChanged(int);
	void rectChanged(QRectF&);
	void xAutoScaleChanged(bool);
	void xMinChanged(double);
	void xMaxChanged(double);
	void xScaleChanged(CartesianPlot::Scale);
	void yAutoScaleChanged(bool);
	void yMinChanged(double);
	void yMaxChanged(double);
	void yScaleChanged(CartesianPlot::Scale);
	void xRangeBreakingEnabledChanged(bool);
	void xRangeBreaksChanged(const CartesianPlot::RangeBreaks&);
	void yRangeBreakingEnabledChanged(bool);
	void yRangeBreaksChanged(const CartesianPlot::RangeBreaks&);
	void themeChanged(const QString&);
	void mousePressZoomSelectionModeSignal(QPointF logicPos);
	void mousePressCursorModeSignal(int cursorNumber, QPointF logicPos);
	void mouseMoveZoomSelectionModeSignal(QPointF logicPos);
	void mouseMoveCursorModeSignal(int cursorNumber, QPointF logicPos);
	void mouseReleaseCursorModeSignal();
	void mouseReleaseZoomSelectionModeSignal();
	void mouseHoverZoomSelectionModeSignal(QPointF logicalPoint);
	void mouseHoverOutsideDataRectSignal();
	void curveNameChanged(const AbstractAspect* curve);
	void cursorPosChanged(int cursorNumber, double xPos);
	void curveAdded(const XYCurve*);
	void curveRemoved(const XYCurve*);
	void curveLinePenChanged(QPen, QString curveName);
	void cursorPenChanged(QPen);
	void curveDataChanged(const XYCurve*);
	void curveVisibilityChangedSignal();
	void mouseModeChanged(CartesianPlot::MouseMode);
	void cursor0EnableChanged(bool enable);
	void cursor1EnableChanged(bool enable);
};

#endif
