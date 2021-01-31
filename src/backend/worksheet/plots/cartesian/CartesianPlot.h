
/***************************************************************************
    File                 : CartesianPlot.h
    Project              : LabPlot
    Description          : Cartesian plot
    --------------------------------------------------------------------
    Copyright            : (C) 2011-2021 Alexander Semke (alexander.semke@web.de)
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

#ifndef CARTESIANPLOT_H
#define CARTESIANPLOT_H

#include "Axis.h"
#include "Histogram.h"
#include "../AbstractPlot.h"
#include "backend/lib/Range.h"

class CartesianPlotPrivate;
class CartesianPlotLegend;
class CartesianCoordinateSystem;
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
class InfoElementDialog;
class KConfig;

class CartesianPlot : public AbstractPlot {
	Q_OBJECT

public:
	explicit CartesianPlot(const QString &name);
	~CartesianPlot() override;

	enum class Type {FourAxes, TwoAxes, TwoAxesCentered, TwoAxesCenteredZero};
	enum class MouseMode {Selection, ZoomSelection, ZoomXSelection, ZoomYSelection, Cursor, Crosshair};
	enum class NavigationOperation {ScaleAuto, ScaleAutoX, ScaleAutoY, ZoomIn, ZoomOut, ZoomInX, ZoomOutX,
				  ZoomInY, ZoomOutY, ShiftLeftX, ShiftRightX, ShiftUpY, ShiftDownY};
	enum class RangeType {Free, Last, First};
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
	QRectF dataRect() const;
	void setMouseMode(MouseMode);
	MouseMode mouseMode() const;
	BASIC_D_ACCESSOR_DECL(bool, isLocked, Locked)
	void navigate(NavigationOperation);
	void setSuppressDataChangedSignal(bool);
	const QList<QColor>& themeColorPalette() const;
	void processDropEvent(const QVector<quintptr>&) override;
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

	const QString xRangeDateTimeFormat() const;
	const QString xRangeDateTimeFormat(int index) const;
	const QString yRangeDateTimeFormat() const;
	const QString yRangeDateTimeFormat(int index) const;
	BASIC_D_ACCESSOR_DECL(CartesianPlot::RangeType, rangeType, RangeType)
	BASIC_D_ACCESSOR_DECL(int, rangeLastValues, RangeLastValues)
	BASIC_D_ACCESSOR_DECL(int, rangeFirstValues, RangeFirstValues)

	BASIC_D_ACCESSOR_DECL(bool, autoScaleX, AutoScaleX)	// auto scale x range of default csystem
	void setAutoScaleX(int index, bool);	// auto scale x axis index
	BASIC_D_ACCESSOR_DECL(bool, autoScaleY, AutoScaleY)	// auto scale y range of default csystem
	void setAutoScaleY(int index, bool);	// auto scale y axis index

	int xRangeCount() const;
	int yRangeCount() const;
	const Range<double>& xRange() const;		// get x range of default plot range
	const Range<double>& yRange() const;		// get y range of default plot range
	void setXRange(Range<double>);		// set x range of default plot range
	void setYRange(Range<double>);		// set y range of default plot range
	BASIC_D_INDEX_ACCESSOR_DECL(const Range<double>, xRange, XRange) // x range index
	BASIC_D_INDEX_ACCESSOR_DECL(const Range<double>, yRange, YRange) // y range index
	void addXRange();				// add new x range
	void addYRange();				// add new y range
	void addXRange(Range<double>);			// add x range
	void addYRange(Range<double>);			// add y range
	void removeXRange(int index);			// remove selected x range
	void removeYRange(int index);			// remove selected y range
	// convenience methods
	void setXMin(int index, double value);	// set x min of x range index
	void setXMax(int index, double value);	// set x max of x range index
	void setYMin(int index, double value);	// set y min of y range index
	void setYMax(int index, double value);	// set y max of y range index
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
	CLASS_D_ACCESSOR_DECL(QPen, cursorPen, CursorPen);
	CLASS_D_ACCESSOR_DECL(bool, cursor0Enable, Cursor0Enable);
	CLASS_D_ACCESSOR_DECL(bool, cursor1Enable, Cursor1Enable);

	int coordinateSystemCount() const;	// get number of coordinate systems
	CartesianCoordinateSystem* coordinateSystem(int) const;	// get coordinate system index
	CartesianCoordinateSystem* defaultCoordinateSystem() const;	// return default coordinate system
	void addCoordinateSystem();			// add a new coordinate system
	void addCoordinateSystem(CartesianCoordinateSystem* cSystem);	// add a coordinate system
	void removeCoordinateSystem(int index);		// remove coordinate system index
	BASIC_D_ACCESSOR_DECL(int, defaultCoordinateSystemIndex, DefaultCoordinateSystemIndex)

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
	void checkAxisFormat(const AbstractColumn*, Axis::Orientation);
	void calculateCurvesXMinMax(int index, bool completeRange = true);
	void calculateCurvesYMinMax(int index, bool completeRange = true);

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

public slots:
	void addHorizontalAxis();
	void addVerticalAxis();
	void addCurve();
	void addHistogram();
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
	void addConvolutionCurve();
	void addCorrelationCurve();

	void addLegend();
	void addTextLabel();
	void addImage();
	void addCustomPoint();
	void addReferenceLine();
	void addInfoElement();

	void scaleAutoTriggered();
	bool scaleAuto(int xIndex = -1, int yIndex = -1, bool fullRange = true);
	bool scaleAutoX(int index = -1, bool fullRange = true);
	bool scaleAutoY(int index = -1, bool fullRange = true);

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

	bool autoScale(bool fullRange = true);
	void dataChanged();

private slots:
	void updateLegend();
	void childAdded(const AbstractAspect*);
	void childRemoved(const AbstractAspect* parent, const AbstractAspect* before, const AbstractAspect* child);
	void childHovered();

	void xDataChanged();
	void yDataChanged();
	void curveLinePenChanged(QPen);
	void curveVisibilityChanged();

	//SLOTs for changes triggered via QActions in the context menu
	void visibilityChanged();
	void loadTheme(const QString&);

protected:
	CartesianPlot(const QString &name, CartesianPlotPrivate *dd);

signals:
	void rangeTypeChanged(CartesianPlot::RangeType);
	void xRangeFormatChanged(RangeT::Format);
	void yRangeFormatChanged(RangeT::Format);
	void rangeLastValuesChanged(int);
	void rangeFirstValuesChanged(int);
	void rectChanged(QRectF&);
	void xAutoScaleChanged(bool);
	void yAutoScaleChanged(bool);
	void xRangeChanged(Range<double>);
	void yRangeChanged(Range<double>);
	void xMinChanged(double);
	void xMaxChanged(double);
	void yMinChanged(double);
	void yMaxChanged(double);
	void xScaleChanged(RangeT::Scale);
	void yScaleChanged(RangeT::Scale);
	void defaultCoordinateSystemIndexChanged(int);
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
