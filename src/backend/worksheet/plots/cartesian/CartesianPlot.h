/*
    File                 : CartesianPlot.h
    Project              : LabPlot
    Description          : Cartesian plot
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2011-2021 Alexander Semke <alexander.semke@web.de>
    SPDX-FileCopyrightText: 2012-2021 Stefan Gerlach <stefan.gerlach@uni.kn>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef CARTESIANPLOT_H
#define CARTESIANPLOT_H

#include "backend/lib/Range.h"
#include "backend/worksheet/plots/AbstractPlot.h"
#include "backend/worksheet/plots/cartesian/Axis.h"

extern "C" {
#include "backend/nsl/nsl_sf_stats.h"
}

class AbstractColumn;
class CartesianPlotPrivate;
class CartesianPlotLegend;
class CartesianCoordinateSystem;
class CartesianPlotDock;
class Histogram;
class InfoElementDialog;
class XYCurve;
class KConfig;

#ifdef SDK
#include "labplot_export.h"
class LABPLOT_EXPORT CartesianPlot : public AbstractPlot {
#else
class CartesianPlot : public AbstractPlot {
#endif
	Q_OBJECT

public:
	explicit CartesianPlot(const QString &name);
	~CartesianPlot() override;

	enum class Type {FourAxes, TwoAxes, TwoAxesCentered, TwoAxesCenteredZero};
	enum class MouseMode {Selection, ZoomSelection, ZoomXSelection, ZoomYSelection, Cursor, Crosshair};
	enum class NavigationOperation {ScaleAuto, ScaleAutoX, ScaleAutoY, ZoomIn, ZoomOut, ZoomInX, ZoomOutX,
				  ZoomInY, ZoomOutY, ShiftLeftX, ShiftRightX, ShiftUpY, ShiftDownY};
	enum class RangeType {Free, Last, First};
	Q_ENUM(RangeType)
	enum class RangeBreakStyle {Simple, Vertical, Sloped};

	struct RangeBreak {
		RangeBreak() : range(qQNaN(), qQNaN()), position(0.5), style(RangeBreakStyle::Sloped) {}
		bool isValid() const {
			return range.valid();
		}
		Range<double> range;
		double position;
		RangeBreakStyle style;
	};

	//simple wrapper for QList<RangeBreaking> in order to get our macros working
	//TODO: same for xRanges, etc.?
	struct RangeBreaks {
		RangeBreaks() : lastChanged(-1) {
			RangeBreak b;
			list << b;
		}
		QList<RangeBreak> list;
		int lastChanged;
	};

	void setType(Type type);
	Type type() const;

	QIcon icon() const override;
	QMenu* createContextMenu() override;
	QMenu* analysisMenu();
	QVector<AbstractAspect*> dependsOn() const override;
	QVector<AspectType> pasteTypes() const override;

	void setRect(const QRectF&) override;
	void setPrevRect(const QRectF&) override;
	QRectF dataRect() const;
	void setMouseMode(MouseMode);
	MouseMode mouseMode() const;
	BASIC_D_ACCESSOR_DECL(bool, isLocked, Locked)
	void navigate(int cSystemIndex, NavigationOperation);
	void setSuppressRetransform(bool);
	const QList<QColor>& themeColorPalette() const;
	const QColor themeColorPalette(int index) const;
	void processDropEvent(const QVector<quintptr>&) override;
	bool isPanningActive() const;
	bool isHovered() const;
	bool isPrinted() const;
	bool isSelected() const;

	void addLegend(CartesianPlotLegend*);
	int curveCount();
	const XYCurve* getCurve(int index);
	double cursorPos(int cursorNumber);
	int curveChildIndex(const WorksheetElement*) const;

	void save(QXmlStreamWriter*) const override;
	bool load(XmlStreamReader*, bool preview) override;
	void loadThemeConfig(const KConfig&) override;
	void saveTheme(KConfig& config);
	void mousePressZoomSelectionMode(QPointF logicPos, int cSystemIndex);
	void mousePressCursorMode(int cursorNumber, QPointF logicPos);
	void mouseMoveZoomSelectionMode(QPointF logicPos, int cSystemIndex);
	void mouseMoveSelectionMode(QPointF logicStart, QPointF logicEnd);
	void mouseMoveCursorMode(int cursorNumber, QPointF logicPos);
	void mouseReleaseZoomSelectionMode(int cSystemIndex);
	void mouseHoverZoomSelectionMode(QPointF logicPos, int cSystemIndex);
	void mouseHoverOutsideDataRect();

	const QString xRangeDateTimeFormat() const;
	const QString xRangeDateTimeFormat(int index) const;
	const QString yRangeDateTimeFormat() const;
	const QString yRangeDateTimeFormat(int index) const;
	BASIC_D_ACCESSOR_DECL(CartesianPlot::RangeType, rangeType, RangeType)
	BASIC_D_ACCESSOR_DECL(bool, niceExtend, NiceExtend)
	BASIC_D_ACCESSOR_DECL(int, rangeLastValues, RangeLastValues)
	BASIC_D_ACCESSOR_DECL(int, rangeFirstValues, RangeFirstValues)

	bool autoScaleX(int index = -1) const;	// is x range index auto scaled?
	void setAutoScaleX(int index, bool autoscale, bool suppressRetransform);	// auto scale x range index
	bool autoScaleY(int index = -1) const;	// is y range index auto scaled?
	void setAutoScaleY(int index, bool autoscale, bool suppressRetransform);	// auto scale y range index

	int xRangeCount() const;
	int yRangeCount() const;
	const Range<double>& xRange(int index = -1) const;	// get x range of (default) plot range
	const Range<double>& yRange(int index = -1) const;	// get y range of (default) plot range
	void setXRange(const Range<double>);		// set x range of default plot range
	void setYRange(const Range<double>);		// set y range of default plot range
	void setXRange(int index, const Range<double>&);
	void setYRange(int index, const Range<double>&);
	const Range<double>& dataXRange(int index = -1);
	const Range<double>& dataYRange(int index = -1);
	bool xRangeDirty(int index);
	bool yRangeDirty(int index);
	void setXRangeDirty(int index, bool dirty);
	void setYRangeDirty(int index, bool dirty);
	void addXRange();				// add new x range
	void addYRange();				// add new y range
	void addXRange(const Range<double>&);		// add x range
	void addYRange(const Range<double>&);		// add y range
	void removeXRange(int index);			// remove selected x range
	void removeYRange(int index);			// remove selected y range
	// convenience methods
	void setXMin(int index, double);	// set x min of x range index
	void setXMax(int index, double);	// set x max of x range index
	void setYMin(int index, double);	// set y min of y range index
	void setYMax(int index, double);	// set y max of y range index
	BASIC_D_ACCESSOR_DECL(RangeT::Format, xRangeFormat, XRangeFormat)	// x range format of default cSystem
	BASIC_D_INDEX_ACCESSOR_DECL(RangeT::Format, xRangeFormat, XRangeFormat) // range format of x range index
	BASIC_D_ACCESSOR_DECL(RangeT::Format, yRangeFormat, YRangeFormat)	// y range format of default cSystem
	BASIC_D_INDEX_ACCESSOR_DECL(RangeT::Format, yRangeFormat, YRangeFormat) // range format of x range index
	BASIC_D_ACCESSOR_DECL(RangeT::Scale, xRangeScale, XRangeScale)	// x range scale of default cSystem
	BASIC_D_INDEX_ACCESSOR_DECL(RangeT::Scale, xRangeScale, XRangeScale) // range scale of x range index
	BASIC_D_ACCESSOR_DECL(RangeT::Scale, yRangeScale, YRangeScale)	// y range scale of default cSystem
	BASIC_D_INDEX_ACCESSOR_DECL(RangeT::Scale, yRangeScale, YRangeScale) // range scale of x range index

	BASIC_D_ACCESSOR_DECL(bool, xRangeBreakingEnabled, XRangeBreakingEnabled)
	BASIC_D_ACCESSOR_DECL(bool, yRangeBreakingEnabled, YRangeBreakingEnabled)
	CLASS_D_ACCESSOR_DECL(RangeBreaks, xRangeBreaks, XRangeBreaks)
	CLASS_D_ACCESSOR_DECL(RangeBreaks, yRangeBreaks, YRangeBreaks)
	CLASS_D_ACCESSOR_DECL(QPen, cursorPen, CursorPen)
	CLASS_D_ACCESSOR_DECL(bool, cursor0Enable, Cursor0Enable)
	CLASS_D_ACCESSOR_DECL(bool, cursor1Enable, Cursor1Enable)

	int coordinateSystemCount() const;	// get number of coordinate systems
	CartesianCoordinateSystem* coordinateSystem(int) const;	// get coordinate system index
	CartesianCoordinateSystem* defaultCoordinateSystem() const;	// return default coordinate system
	void addCoordinateSystem();			// add a new coordinate system
	void addCoordinateSystem(CartesianCoordinateSystem* cSystem);	// add a coordinate system
	void removeCoordinateSystem(int index);		// remove coordinate system index
	BASIC_D_ACCESSOR_DECL(int, defaultCoordinateSystemIndex, DefaultCoordinateSystemIndex)

	QString theme() const;

	typedef CartesianPlotPrivate Private;

public Q_SLOTS:
	void setTheme(const QString&);
	void retransformAll();

private:
	void init();
	void initActions();
	void initMenus();
	void setColorPalette(const KConfig&);
	const XYCurve* currentCurve() const;
	void shift(int index, bool x, bool leftOrDown);
	void zoom(int index, bool x, bool in);
	void checkAxisFormat(const AbstractColumn*, Axis::Orientation);
	void calculateDataXRange(int index, bool completeRange = true);
	void calculateDataYRange(int index, bool completeRange = true);
	void retransformScales();
	int curveTotalCount() const;

	CartesianPlotLegend* m_legend{nullptr};
	double m_zoomFactor{1.2};
	QList<QColor> m_themeColorPalette;
	bool m_menusInitialized{false};

	QAction* visibilityAction;

	//"add new" actions
	QAction* addCurveAction;
	QAction* addEquationCurveAction;
	QAction* addHistogramAction;
	QAction* addBoxPlotAction;
	QAction* addDataReductionCurveAction;
	QAction* addDifferentiationCurveAction;
	QAction* addIntegrationCurveAction;
	QAction* addInterpolationCurveAction;
	QAction* addSmoothCurveAction;
	QAction* addFitCurveAction;
	QAction* addFourierFilterCurveAction;
	QAction* addFourierTransformCurveAction;
	QAction* addHilbertTransformCurveAction;
	QAction* addConvolutionCurveAction;
	QAction* addCorrelationCurveAction;

	QAction* addHorizontalAxisAction;
	QAction* addVerticalAxisAction;
	QAction* addLegendAction;
	QAction* addTextLabelAction;
	QAction* addImageAction;
	QAction* addInfoElementAction;
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
	QVector<QAction*> addFitActions;
	QAction* addFourierFilterAction;
	QAction* addFourierTransformAction;
	QAction* addHilbertTransformAction;
	QAction* addConvolutionAction;
	QAction* addCorrelationAction;

	QMenu* addNewMenu{nullptr};
	QMenu* addNewAnalysisMenu{nullptr};
	QMenu* zoomMenu{nullptr};
	QMenu* dataAnalysisMenu{nullptr};
	QMenu* themeMenu{nullptr};

	// storing the pointer, because then it can be implemented also interactive clicking on a curve
	// otherwise I have to do QDialog::exec and everything is blocked
	// When saving, it is possible to use show
	InfoElementDialog* m_infoElementDialog{nullptr};

	Q_DECLARE_PRIVATE(CartesianPlot)

public Q_SLOTS:
	void addHorizontalAxis();
	void addVerticalAxis();
	void addCurve();
	void addHistogram();
	void addHistogramFit(Histogram*, nsl_sf_stats_distribution);
	void addBoxPlot();
	void addEquationCurve();
	void addDataReductionCurve();
	void addDifferentiationCurve();
	void addIntegrationCurve();
	void addInterpolationCurve();
	void addSmoothCurve();
	void addFitCurve();
	void addFourierFilterCurve();
	void addFourierTransformCurve();
	void addHilbertTransformCurve();
	void addConvolutionCurve();
	void addCorrelationCurve();

	void addLegend();
	void addTextLabel();
	void addImage();
	void addCustomPoint();
	void addReferenceLine();
	void addInfoElement();

	void scaleAutoTriggered();
	bool scaleAuto(int xIndex, int yIndex, bool fullRange = true);
	bool scaleAutoX(int index, bool fullRange = true);
	bool scaleAutoY(int index, bool fullRange = true);

	void zoomIn(int xIndex = -1, int yIndex = -1);
	void zoomOut(int xIndex = -1, int yIndex = -1);
	void zoomInX(int index = -1);
	void zoomOutX(int index = -1);
	void zoomInY(int index = -1);
	void zoomOutY(int index = -1);

	void shiftLeftX(int index = -1);
	void shiftRightX(int index = -1);
	void shiftUpY(int index = -1);
	void shiftDownY(int index = -1);

	void cursor();

	void dataChanged(int xIndex = -1, int yIndex = -1, WorksheetElement* sender = nullptr);

private Q_SLOTS:
	void updateLegend();
	void childAdded(const AbstractAspect*);
	void childRemoved(const AbstractAspect* parent, const AbstractAspect* before, const AbstractAspect* child);
	void childHovered();

	void xDataChanged(XYCurve*);
	void yDataChanged(XYCurve*);
	void curveLinePenChanged(QPen);
	void curveVisibilityChanged();
	void boxPlotOrientationChanged(WorksheetElement::Orientation);

	//SLOTs for changes triggered via QActions in the context menu
	void loadTheme(const QString&);

protected:
	CartesianPlot(const QString &name, CartesianPlotPrivate *dd);

Q_SIGNALS:
	void rangeTypeChanged(CartesianPlot::RangeType);
	void niceExtendChanged(bool);
	void xRangeFormatChanged(int xRangeIndex, RangeT::Format);
	void yRangeFormatChanged(int yRangeIndex, RangeT::Format);
	void rangeLastValuesChanged(int);
	void rangeFirstValuesChanged(int);
	void rectChanged(QRectF&);
	void xAutoScaleChanged(int xRangeIndex, bool);
	void yAutoScaleChanged(int yRangeIndex, bool);
	void xRangeChanged(int xRangeIndex, Range<double>);
	void yRangeChanged(int yRangeIndex, Range<double>);
	void xMinChanged(int xRangeIndex, double);
	void xMaxChanged(int xRangeIndex, double);
	void yMinChanged(int yRangeIndex, double);
	void yMaxChanged(int yRangeIndex, double);
	void xScaleChanged(int xRangeIndex, RangeT::Scale);
	void yScaleChanged(int yRangeIndex, RangeT::Scale);
	void defaultCoordinateSystemIndexChanged(int);
	void xRangeBreakingEnabledChanged(bool);
	void xRangeBreaksChanged(const CartesianPlot::RangeBreaks&);
	void yRangeBreakingEnabledChanged(bool);
	void yRangeBreaksChanged(const CartesianPlot::RangeBreaks&);
	void themeChanged(const QString&);
	void mousePressZoomSelectionModeSignal(QPointF logicPos);
	void mousePressCursorModeSignal(int cursorNumber, QPointF logicPos);
	void mouseMoveSelectionModeSignal(QPointF logicalStart, QPointF logicalEnd);
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

	friend CartesianPlotDock;
};

#endif
