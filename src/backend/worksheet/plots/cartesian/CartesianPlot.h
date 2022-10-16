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
#include "backend/worksheet/plots/cartesian/CartesianCoordinateSystem.h"

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

using Dimension = CartesianCoordinateSystem::Dimension;

#ifdef SDK
#include "labplot_export.h"
class LABPLOT_EXPORT CartesianPlot : public AbstractPlot {
#else
class CartesianPlot : public AbstractPlot {
#endif
	Q_OBJECT

public:
	explicit CartesianPlot(const QString& name);
	~CartesianPlot() override;

	enum class Type { FourAxes, TwoAxes, TwoAxesCentered, TwoAxesCenteredZero };
	enum class MouseMode { Selection, ZoomSelection, ZoomXSelection, ZoomYSelection, Cursor, Crosshair };
	enum class NavigationOperation {
		ScaleAuto,
		ScaleAutoX,
		ScaleAutoY,
		ZoomIn,
		ZoomOut,
		ZoomInX,
		ZoomOutX,
		ZoomInY,
		ZoomOutY,
		ShiftLeftX,
		ShiftRightX,
		ShiftUpY,
		ShiftDownY
	};
	enum class RangeType { Free, Last, First };
	Q_ENUM(RangeType)
	enum class RangeBreakStyle { Simple, Vertical, Sloped };

	struct RangeBreak {
		RangeBreak()
			: range(qQNaN(), qQNaN())
			, position(0.5)
			, style(RangeBreakStyle::Sloped) {
		}
		bool isValid() const {
			return range.valid();
		}
		Range<double> range;
		double position;
		RangeBreakStyle style;
	};

	// simple wrapper for QList<RangeBreaking> in order to get our macros working
	// TODO: same for xRanges, etc.?
	struct RangeBreaks {
		RangeBreaks()
			: lastChanged(-1) {
			RangeBreak b;
			list << b;
		}
		QList<RangeBreak> list;
		int lastChanged;
	};

	void setType(Type type);
	Type type() const;

	QIcon icon() const override;
	QMenu* createContextMenu();
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
	void finalizeLoad();
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

	const QString rangeDateTimeFormat(const Dimension) const;
	const QString rangeDateTimeFormat(const Dimension, const int index) const;
	BASIC_D_ACCESSOR_DECL(CartesianPlot::RangeType, rangeType, RangeType)
	BASIC_D_ACCESSOR_DECL(bool, niceExtend, NiceExtend)
	BASIC_D_ACCESSOR_DECL(int, rangeLastValues, RangeLastValues)
	BASIC_D_ACCESSOR_DECL(int, rangeFirstValues, RangeFirstValues)

	bool autoScale(const Dimension, int index = -1) const;
	void enableAutoScale(const Dimension, int index, bool enable, bool fullRange = false);

	int rangeCount(const Dimension) const;
	const Range<double>& range(const Dimension, int index = -1) const; // get range of (default) plot range
	void setRange(const Dimension, const Range<double>); // set range of default plot range
	void setRange(const Dimension, const int index, const Range<double>& range);
	void setXRange(int index, const Range<double>&);
	void setYRange(int index, const Range<double>&);
	const Range<double>& dataRange(const Dimension, int index = -1);
	bool rangeDirty(const Dimension, int index) const;
	void setRangeDirty(const Dimension, int index, bool dirty);
	void addXRange(); // add new x range
	void addYRange(); // add new y range
	void addXRange(const Range<double>&); // add x range
	void addYRange(const Range<double>&); // add y range
	void removeRange(const Dimension, int index); // remove selected range
	// convenience methods

	void setMin(const Dimension, int index, double); // set x min of range index
	void setMax(const Dimension, int index, double); // set x max of range index
	void setRangeFormat(const Dimension, const RangeT::Format);
	void setRangeFormat(const Dimension, const int, const RangeT::Format);
	RangeT::Format rangeFormat(const Dimension, const int) const;
	RangeT::Scale rangeScale(const Dimension, const int index) const;
	BASIC_D_ACCESSOR_DECL(RangeT::Format, xRangeFormat, XRangeFormat) // x range format of default cSystem
	BASIC_D_INDEX_ACCESSOR_DECL(RangeT::Format, xRangeFormat, XRangeFormat) // range format of x range index
	BASIC_D_ACCESSOR_DECL(RangeT::Format, yRangeFormat, YRangeFormat) // y range format of default cSystem
	BASIC_D_INDEX_ACCESSOR_DECL(RangeT::Format, yRangeFormat, YRangeFormat) // range format of x range index
	void setRangeScale(const Dimension, const int index, const RangeT::Scale scale);
	BASIC_D_ACCESSOR_DECL(RangeT::Scale, xRangeScale, XRangeScale) // x range scale of default cSystem
	BASIC_D_INDEX_ACCESSOR_DECL(RangeT::Scale, xRangeScale, XRangeScale) // range scale of x range index
	BASIC_D_ACCESSOR_DECL(RangeT::Scale, yRangeScale, YRangeScale) // y range scale of default cSystem
	BASIC_D_INDEX_ACCESSOR_DECL(RangeT::Scale, yRangeScale, YRangeScale) // range scale of x range index

	BASIC_D_ACCESSOR_DECL(bool, xRangeBreakingEnabled, XRangeBreakingEnabled)
	BASIC_D_ACCESSOR_DECL(bool, yRangeBreakingEnabled, YRangeBreakingEnabled)
	CLASS_D_ACCESSOR_DECL(RangeBreaks, xRangeBreaks, XRangeBreaks)
	CLASS_D_ACCESSOR_DECL(RangeBreaks, yRangeBreaks, YRangeBreaks)
	CLASS_D_ACCESSOR_DECL(QPen, cursorPen, CursorPen)
	CLASS_D_ACCESSOR_DECL(bool, cursor0Enable, Cursor0Enable)
	CLASS_D_ACCESSOR_DECL(bool, cursor1Enable, Cursor1Enable)

	int coordinateSystemCount() const; // get number of coordinate systems
	CartesianCoordinateSystem* coordinateSystem(int) const; // get coordinate system index
	CartesianCoordinateSystem* defaultCoordinateSystem() const; // return default coordinate system
	void addCoordinateSystem(); // add a new coordinate system
	void addCoordinateSystem(CartesianCoordinateSystem* cSystem); // add a coordinate system
	void removeCoordinateSystem(int index); // remove coordinate system index
	BASIC_D_ACCESSOR_DECL(int, defaultCoordinateSystemIndex, DefaultCoordinateSystemIndex)

	void retransformScales();
	void retransformScale(Dimension, int index);

	QString theme() const;

	typedef CartesianPlotPrivate Private;

public Q_SLOTS:
	void setTheme(const QString&);
	virtual void retransform() override;

private:
	void init();
	void initActions();
	void initMenus();
	void setColorPalette(const KConfig&);
	const XYCurve* currentCurve() const;
	void shift(int index, const Dimension, bool leftOrDown);
	void zoom(int index, const Dimension, bool in);
	void checkAxisFormat(const AbstractColumn*, Axis::Orientation);
	void calculateDataRange(const Dimension, const int index, bool completeRange = true);
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
	QAction* addBarPlotAction;
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

	// analysis menu actions
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
	void addBarPlot();
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

	bool scaleAuto(int xIndex = -1, int yIndex = -1, bool fullRange = true, bool suppressRetransformScale = false);
	bool scaleAuto(const Dimension, int index = -1, bool fullRange = true, bool suppressRetransformScale = false);

	void zoomIn(int xIndex = -1, int yIndex = -1);
	void zoomOut(int xIndex = -1, int yIndex = -1);
	void zoomInX(int index = -1);
	void zoomOutX(int index = -1);
	void zoomInY(int index = -1);
	void zoomOutY(int index = -1);
	void zoomInOut(const int index, const Dimension dim, const bool zoomIn);

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

	void dataChanged(WorksheetElement*);
	void dataChanged(XYCurve*, const Dimension);
	void curveLinePenChanged(QPen);
	void curveVisibilityChanged();
	void boxPlotOrientationChanged(WorksheetElement::Orientation);

	// SLOTs for changes triggered via QActions in the context menu
	void loadTheme(const QString&);

protected:
	CartesianPlot(const QString& name, CartesianPlotPrivate* dd);

Q_SIGNALS:
	void rangeTypeChanged(CartesianPlot::RangeType);
	void niceExtendChanged(bool);
	void rangeFormatChanged(const Dimension, int rangeIndex, RangeT::Format);
	void rangeLastValuesChanged(int);
	void rangeFirstValuesChanged(int);
	void rectChanged(QRectF&);
	void autoScaleChanged(const Dimension, int xRangeIndex, bool);
	void rangeChanged(const Dimension, int, Range<double>);
	void yRangeChanged(int yRangeIndex, Range<double>);
	void xMinChanged(int xRangeIndex, double);
	void xMaxChanged(int xRangeIndex, double);
	void yMinChanged(int yRangeIndex, double);
	void yMaxChanged(int yRangeIndex, double);
	void scaleChanged(const Dimension, int rangeIndex, RangeT::Scale);
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

Q_SIGNALS:
	void scaleRetransformed(const CartesianPlot* plot, const Dimension dim, int index);

	friend CartesianPlotDock;
	friend class CartesianPlotTest;
};

#endif
