/***************************************************************************
    File                 : WorksheetView.cpp
    Project              : LabPlot
    Description          : Worksheet view
    --------------------------------------------------------------------
    Copyright            : (C) 2009-2019 Alexander Semke (alexander.semke@web.de)
    Copyright            : (C) 2016-2018 Stefan-Gerlach (stefan.gerlach@uni.kn)

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
#include "commonfrontend/worksheet/WorksheetView.h"
#include "backend/core/AbstractColumn.h"
#include "backend/core/Project.h"
#include "backend/worksheet/plots/cartesian/Axis.h"
#include "backend/worksheet/plots/cartesian/AxisPrivate.h"
#include "backend/worksheet/plots/cartesian/XYCurve.h"
#include "backend/worksheet/plots/cartesian/XYCurvePrivate.h"
#include "backend/worksheet/WorksheetElementContainerPrivate.h"
#include "backend/worksheet/Image.h"
#include "backend/worksheet/TextLabel.h"
#include "commonfrontend/core/PartMdiView.h"
#include "kdefrontend/widgets/ThemesWidget.h"
#include "kdefrontend/worksheet/GridDialog.h"
#include "kdefrontend/worksheet/PresenterWidget.h"
#include "kdefrontend/worksheet/DynamicPresenterWidget.h"
#include "backend/lib/trace.h"

#include <QApplication>
#include <QDesktopWidget>
#include <QMdiArea>
#include <QMenu>
#include <QToolBar>
#include <QScreen>
#include <QWheelEvent>
#include <QPrinter>
#include <QSvgGenerator>
#include <QImage>
#include <QToolButton>
#include <QGraphicsOpacityEffect>
#include <QTimeLine>
#include <QClipboard>
#include <QMimeData>
#include <QWidgetAction>

#include <KColorScheme>
#include <KLocalizedString>
#include <KMessageBox>
#include <KConfigGroup>

#ifdef Q_OS_MAC
#include "3rdparty/kdmactouchbar/src/kdmactouchbar.h"
#endif

#include <limits>

/**
 * \class WorksheetView
 * \brief Worksheet view
 */

/*!
  Constructor of the class.
  Creates a view for the Worksheet \c worksheet and initializes the internal model.
*/
WorksheetView::WorksheetView(Worksheet* worksheet) : QGraphicsView(), m_worksheet(worksheet) {
	setScene(m_worksheet->scene());

	setRenderHint(QPainter::Antialiasing);
	setRubberBandSelectionMode(Qt::ContainsItemBoundingRect);
	setTransformationAnchor(QGraphicsView::AnchorViewCenter);
	setResizeAnchor(QGraphicsView::AnchorViewCenter);
	setMinimumSize(16, 16);
	setFocusPolicy(Qt::StrongFocus);

	if (m_worksheet->useViewSize()) {
		setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
		setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	}

	viewport()->setAttribute( Qt::WA_OpaquePaintEvent );
	viewport()->setAttribute( Qt::WA_NoSystemBackground );
	setAcceptDrops(true);
	setCacheMode(QGraphicsView::CacheBackground);

	m_gridSettings.style = GridStyle::NoGrid;

	//signal/slot connections
	connect(m_worksheet, &Worksheet::requestProjectContextMenu, this, &WorksheetView::createContextMenu);
	connect(m_worksheet, &Worksheet::itemSelected, this, &WorksheetView::selectItem);
	connect(m_worksheet, &Worksheet::itemDeselected, this, &WorksheetView::deselectItem);
	connect(m_worksheet, &Worksheet::requestUpdate, this, &WorksheetView::updateBackground);
	connect(m_worksheet, &Worksheet::aspectAboutToBeRemoved, this, &WorksheetView::aspectAboutToBeRemoved);
	connect(m_worksheet, &Worksheet::useViewSizeRequested, this, &WorksheetView::useViewSizeRequested);
	connect(m_worksheet, &Worksheet::layoutChanged, this, &WorksheetView::layoutChanged);
	connect(scene(), &QGraphicsScene::selectionChanged, this, &WorksheetView::selectionChanged);

	//resize the view to make the complete scene visible.
	//no need to resize the view when the project is being opened,
	//all views will be resized to the stored values at the end
	if (!m_worksheet->isLoading()) {
		float w = Worksheet::convertFromSceneUnits(sceneRect().width(), Worksheet::Unit::Inch);
		float h = Worksheet::convertFromSceneUnits(sceneRect().height(), Worksheet::Unit::Inch);
		w *= QApplication::desktop()->physicalDpiX();
		h *= QApplication::desktop()->physicalDpiY();
		resize(w*1.1, h*1.1);
	}

	//rescale to the original size
	static const qreal hscale = QApplication::desktop()->physicalDpiX()/(Worksheet::convertToSceneUnits(1, Worksheet::Unit::Inch));
	static const qreal vscale = QApplication::desktop()->physicalDpiY()/(Worksheet::convertToSceneUnits(1, Worksheet::Unit::Inch));
	setTransform(QTransform::fromScale(hscale, vscale));

	initBasicActions();
	installEventFilter(this);
}

/*!
 * initializes couple of actions that have shortcuts assigned in the constructor as opposed
 * to other actions in initAction() that are create on demand only if the context menu is requested
 */
void WorksheetView::initBasicActions() {
	selectAllAction = new QAction(QIcon::fromTheme("edit-select-all"), i18n("Select All"), this);
	this->addAction(selectAllAction);
	connect(selectAllAction, &QAction::triggered, this, &WorksheetView::selectAllElements);

	deleteAction = new QAction(QIcon::fromTheme("edit-delete"), i18n("Delete"), this);
	this->addAction(deleteAction);
	connect(deleteAction, &QAction::triggered, this, &WorksheetView::deleteElement);

	backspaceAction = new QAction(this);
	this->addAction(backspaceAction);
	connect(backspaceAction, &QAction::triggered, this, &WorksheetView::deleteElement);

	//Zoom actions
	zoomInViewAction = new QAction(QIcon::fromTheme("zoom-in"), i18n("Zoom In"), this);
	zoomOutViewAction = new QAction(QIcon::fromTheme("zoom-out"), i18n("Zoom Out"), this);
	zoomOriginAction = new QAction(QIcon::fromTheme("zoom-original"), i18n("Original Size"), this);
}

void WorksheetView::initActions() {
	auto* addNewActionGroup = new QActionGroup(this);
	auto* zoomActionGroup = new QActionGroup(this);
	auto* mouseModeActionGroup = new QActionGroup(this);
	auto* layoutActionGroup = new QActionGroup(this);
	auto* gridActionGroup = new QActionGroup(this);
	gridActionGroup->setExclusive(true);
	auto* magnificationActionGroup = new QActionGroup(this);

	zoomActionGroup->addAction(zoomInViewAction);
	zoomActionGroup->addAction(zoomOutViewAction);
	zoomActionGroup->addAction(zoomOriginAction);

	zoomFitPageHeightAction = new QAction(QIcon::fromTheme("zoom-fit-height"), i18n("Fit to Height"), zoomActionGroup);
	zoomFitPageWidthAction = new QAction(QIcon::fromTheme("zoom-fit-width"), i18n("Fit to Width"), zoomActionGroup);
	zoomFitSelectionAction = new QAction(i18n("Fit to Selection"), zoomActionGroup);

	// Mouse mode actions
	selectionModeAction = new QAction(QIcon::fromTheme("labplot-cursor-arrow"), i18n("Select and Edit"), mouseModeActionGroup);
	selectionModeAction->setCheckable(true);

	navigationModeAction = new QAction(QIcon::fromTheme("input-mouse"), i18n("Navigate"), mouseModeActionGroup);
	navigationModeAction->setCheckable(true);

	zoomSelectionModeAction = new QAction(QIcon::fromTheme("page-zoom"), i18n("Select and Zoom"), mouseModeActionGroup);
	zoomSelectionModeAction->setCheckable(true);

	//Magnification actions
	noMagnificationAction = new QAction(QIcon::fromTheme("labplot-1x-zoom"), i18n("No Magnification"), magnificationActionGroup);
	noMagnificationAction->setCheckable(true);
	noMagnificationAction->setChecked(true);

	twoTimesMagnificationAction = new QAction(QIcon::fromTheme("labplot-2x-zoom"), i18n("2x Magnification"), magnificationActionGroup);
	twoTimesMagnificationAction->setCheckable(true);

	threeTimesMagnificationAction = new QAction(QIcon::fromTheme("labplot-3x-zoom"), i18n("3x Magnification"), magnificationActionGroup);
	threeTimesMagnificationAction->setCheckable(true);

	fourTimesMagnificationAction = new QAction(QIcon::fromTheme("labplot-4x-zoom"), i18n("4x Magnification"), magnificationActionGroup);
	fourTimesMagnificationAction->setCheckable(true);

	fiveTimesMagnificationAction = new QAction(QIcon::fromTheme("labplot-5x-zoom"), i18n("5x Magnification"), magnificationActionGroup);
	fiveTimesMagnificationAction->setCheckable(true);

	//TODO implement later "group selection action" where multiple objects can be selected by drawing a rectangular
// 	selectionModeAction = new QAction(QIcon::fromTheme("select-rectangular"), i18n("Selection"), mouseModeActionGroup);
// 	selectionModeAction->setCheckable(true);

	//"Add new" related actions
	addCartesianPlot1Action = new QAction(QIcon::fromTheme("labplot-xy-plot-four-axes"), i18n("Box Plot, Four Axes"), addNewActionGroup);
	addCartesianPlot2Action = new QAction(QIcon::fromTheme("labplot-xy-plot-two-axes"), i18n("Box Plot, Two Axes"), addNewActionGroup);
	addCartesianPlot3Action = new QAction(QIcon::fromTheme("labplot-xy-plot-two-axes-centered"), i18n("Two Axes, Centered"), addNewActionGroup);
	addCartesianPlot4Action = new QAction(QIcon::fromTheme("labplot-xy-plot-two-axes-centered-origin"), i18n("Two Axes, Crossing at Origin"), addNewActionGroup);
	addTextLabelAction = new QAction(QIcon::fromTheme("draw-text"), i18n("Text"), addNewActionGroup);
	addImageAction = new QAction(QIcon::fromTheme("viewimage"), i18n("Image"), addNewActionGroup);

	//Layout actions
	//TODO: the icons labplot-editvlayout and labplot-edithlayout are confusing for the user.
	//the orientation is visualized as a horizontal or vertical line on the icon, but the user
	//percieves the two objects (resembles plots on the worksheet) separated by this line much stronger than the line itself.
	//with this, the two objects separated by a vertical line are perceived to be layed out in a _horizontal_ order and the
	//same for the vertical line. Because of this we change the icons here. We can rename the icons later in the breeze icon set.
	verticalLayoutAction = new QAction(QIcon::fromTheme("labplot-edithlayout"), i18n("Vertical Layout"), layoutActionGroup);
	verticalLayoutAction->setCheckable(true);

	horizontalLayoutAction = new QAction(QIcon::fromTheme("labplot-editvlayout"), i18n("Horizontal Layout"), layoutActionGroup);
	horizontalLayoutAction->setCheckable(true);

	gridLayoutAction = new QAction(QIcon::fromTheme("labplot-editgrid"), i18n("Grid Layout"), layoutActionGroup);
	gridLayoutAction->setCheckable(true);

	breakLayoutAction = new QAction(QIcon::fromTheme("labplot-editbreaklayout"), i18n("No Layout"), layoutActionGroup);
	breakLayoutAction->setEnabled(false);

	//Grid actions
	noGridAction = new QAction(i18n("No Grid"), gridActionGroup);
	noGridAction->setCheckable(true);
	noGridAction->setChecked(true);
	noGridAction->setData(static_cast<int>(GridStyle::NoGrid));

	denseLineGridAction = new QAction(i18n("Dense Line Grid"), gridActionGroup);
	denseLineGridAction->setCheckable(true);

	sparseLineGridAction = new QAction(i18n("Sparse Line Grid"), gridActionGroup);
	sparseLineGridAction->setCheckable(true);

	denseDotGridAction = new QAction(i18n("Dense Dot Grid"), gridActionGroup);
	denseDotGridAction->setCheckable(true);

	sparseDotGridAction = new QAction(i18n("Sparse Dot Grid"), gridActionGroup);
	sparseDotGridAction->setCheckable(true);

	customGridAction = new QAction(i18n("Custom Grid"), gridActionGroup);
	customGridAction->setCheckable(true);

	snapToGridAction = new QAction(i18n("Snap to Grid"), this);
	snapToGridAction->setCheckable(true);

	showPresenterMode = new QAction(QIcon::fromTheme("view-fullscreen"), i18n("Show in Presenter Mode"), this);

	//check the action corresponding to the currently active layout in worksheet
	this->layoutChanged(m_worksheet->layout());

	connect(addNewActionGroup, &QActionGroup::triggered, this, &WorksheetView::addNew);
	connect(mouseModeActionGroup, &QActionGroup::triggered, this, &WorksheetView::mouseModeChanged);
	connect(zoomActionGroup, &QActionGroup::triggered, this, &WorksheetView::changeZoom);
	connect(magnificationActionGroup, &QActionGroup::triggered, this, &WorksheetView::magnificationChanged);
	connect(layoutActionGroup, &QActionGroup::triggered, this, &WorksheetView::changeLayout);
	connect(gridActionGroup, &QActionGroup::triggered, this, &WorksheetView::changeGrid);
	connect(snapToGridAction, &QAction::triggered, this, &WorksheetView::changeSnapToGrid);
	connect(showPresenterMode, &QAction::triggered, this, &WorksheetView::presenterMode);

	//worksheet control actions
	plotsLockedAction = new QAction(i18n("Non-interactive Plots"), this);
	plotsLockedAction->setToolTip(i18n("If activated, plots on the worksheet don't react on drag and mouse wheel events."));
	plotsLockedAction->setCheckable(true);
	plotsLockedAction->setChecked(m_worksheet->plotsLocked());
	connect(plotsLockedAction, &QAction::triggered, this, &WorksheetView::plotsLockedActionChanged);

	//action for cartesian plots
	auto* cartesianPlotActionModeActionGroup = new QActionGroup(this);
	cartesianPlotActionModeActionGroup->setExclusive(true);
	cartesianPlotApplyToSelectionAction = new QAction(i18n("Selected Plots"), cartesianPlotActionModeActionGroup);
	cartesianPlotApplyToSelectionAction->setCheckable(true);
	cartesianPlotApplyToAllAction = new QAction(i18n("All Plots"), cartesianPlotActionModeActionGroup);
	cartesianPlotApplyToAllAction->setCheckable(true);
	cartesianPlotApplyToAllXAction = new QAction(i18n("All Plots X"), cartesianPlotActionModeActionGroup);
	cartesianPlotApplyToAllXAction->setCheckable(true);
	cartesianPlotApplyToAllYAction = new QAction(i18n("All Plots Y"), cartesianPlotActionModeActionGroup);
	cartesianPlotApplyToAllYAction->setCheckable(true);
	setCartesianPlotActionMode(m_worksheet->cartesianPlotActionMode());
	connect(cartesianPlotActionModeActionGroup, &QActionGroup::triggered, this, &WorksheetView::cartesianPlotActionModeChanged);

	// cursor apply to all/selected
	auto* plotActionCursorGroup = new QActionGroup(this);
	plotActionCursorGroup->setExclusive(true);
	cartesianPlotApplyToSelectionCursor = new QAction(i18n("Selected Plots"), plotActionCursorGroup);
	cartesianPlotApplyToSelectionCursor->setCheckable(true);
	cartesianPlotApplyToAllCursor = new QAction(i18n("All Plots"), plotActionCursorGroup);
	cartesianPlotApplyToAllCursor->setCheckable(true);
	setCartesianPlotCursorMode(m_worksheet->cartesianPlotCursorMode());
	connect(plotActionCursorGroup, &QActionGroup::triggered, this, &WorksheetView::cartesianPlotCursorModeChanged);

	auto* plotMouseModeActionGroup = new QActionGroup(this);
	plotMouseModeActionGroup->setExclusive(true);
	cartesianPlotSelectionModeAction = new QAction(QIcon::fromTheme("labplot-cursor-arrow"), i18n("Select and Edit"), plotMouseModeActionGroup);
	cartesianPlotSelectionModeAction->setData(static_cast<int>(CartesianPlot::MouseMode::Selection));
	cartesianPlotSelectionModeAction->setCheckable(true);
	cartesianPlotSelectionModeAction->setChecked(true);

	cartesianPlotCrosshairModeAction = new QAction(QIcon::fromTheme("crosshairs"), i18n("Crosshair"), plotMouseModeActionGroup);
	cartesianPlotCrosshairModeAction->setData(static_cast<int>(CartesianPlot::MouseMode::Crosshair));
	cartesianPlotCrosshairModeAction->setCheckable(true);

	cartesianPlotZoomSelectionModeAction = new QAction(QIcon::fromTheme("labplot-zoom-select"), i18n("Select Region and Zoom In"), plotMouseModeActionGroup);
	cartesianPlotZoomSelectionModeAction->setData(static_cast<int>(CartesianPlot::MouseMode::ZoomSelection));
	cartesianPlotZoomSelectionModeAction->setCheckable(true);

	cartesianPlotZoomXSelectionModeAction = new QAction(QIcon::fromTheme("labplot-zoom-select-x"), i18n("Select x-Region and Zoom In"), plotMouseModeActionGroup);
	cartesianPlotZoomXSelectionModeAction->setData(static_cast<int>(CartesianPlot::MouseMode::ZoomXSelection));
	cartesianPlotZoomXSelectionModeAction->setCheckable(true);

	cartesianPlotZoomYSelectionModeAction = new QAction(QIcon::fromTheme("labplot-zoom-select-y"), i18n("Select y-Region and Zoom In"), plotMouseModeActionGroup);
	cartesianPlotZoomYSelectionModeAction->setData(static_cast<int>(CartesianPlot::MouseMode::ZoomYSelection));
	cartesianPlotZoomYSelectionModeAction->setCheckable(true);

	// TODO: change ICON
	cartesianPlotCursorModeAction = new QAction(QIcon::fromTheme("debug-execute-from-cursor"), i18n("Cursor"), plotMouseModeActionGroup);
	cartesianPlotCursorModeAction->setData(static_cast<int>(CartesianPlot::MouseMode::Cursor));
	cartesianPlotCursorModeAction->setCheckable(true);

	connect(plotMouseModeActionGroup, &QActionGroup::triggered, this, &WorksheetView::cartesianPlotMouseModeChanged);

	auto* cartesianPlotAddNewActionGroup = new QActionGroup(this);
	addCurveAction = new QAction(QIcon::fromTheme("labplot-xy-curve"), i18n("xy-Curve"), cartesianPlotAddNewActionGroup);
	addHistogramAction = new QAction(QIcon::fromTheme("view-object-histogram-linear"), i18n("Histogram"), cartesianPlotAddNewActionGroup);
	addEquationCurveAction = new QAction(QIcon::fromTheme("labplot-xy-equation-curve"), i18n("xy-Curve from a Formula"), cartesianPlotAddNewActionGroup);
	// TODO: no own icons yet
	addDataOperationCurveAction = new QAction(QIcon::fromTheme("labplot-xy-curve"), i18n("Data Operation"), cartesianPlotAddNewActionGroup);
//	addDataOperationCurveAction = new QAction(QIcon::fromTheme("labplot-xy-data-operation-curve"), i18n("Data Operation"), cartesianPlotAddNewActionGroup);
	addDataReductionCurveAction = new QAction(QIcon::fromTheme("labplot-xy-curve"), i18n("Data Reduction"), cartesianPlotAddNewActionGroup);
//	addDataReductionCurveAction = new QAction(QIcon::fromTheme("labplot-xy-data-reduction-curve"), i18n("Data Reduction"), cartesianPlotAddNewActionGroup);
	addDifferentiationCurveAction = new QAction(QIcon::fromTheme("labplot-xy-curve"), i18n("Differentiation"), cartesianPlotAddNewActionGroup);
//	addDifferentiationCurveAction = new QAction(QIcon::fromTheme("labplot-xy-differentiation-curve"), i18n("Differentiation"), cartesianPlotAddNewActionGroup);
	addIntegrationCurveAction = new QAction(QIcon::fromTheme("labplot-xy-curve"), i18n("Integration"), cartesianPlotAddNewActionGroup);
//	addIntegrationCurveAction = new QAction(QIcon::fromTheme("labplot-xy-integration-curve"), i18n("Integration"), cartesianPlotAddNewActionGroup);
	addConvolutionCurveAction = new QAction(QIcon::fromTheme("labplot-xy-curve"), i18n("(De-)Convolution"), cartesianPlotAddNewActionGroup);
//	addConvolutionCurveAction = new QAction(QIcon::fromTheme("labplot-xy-convolution-curve"), i18n("(De-)Convolution"), cartesianPlotAddNewActionGroup);
	addCorrelationCurveAction = new QAction(QIcon::fromTheme("labplot-xy-curve"), i18n("Auto-/Cross-Correlation"), cartesianPlotAddNewActionGroup);
//	addCorrelationCurveAction = new QAction(QIcon::fromTheme("labplot-xy-convolution-curve"), i18n("Auto-/Cross-Correlation"), cartesianPlotAddNewActionGroup);
	addInterpolationCurveAction = new QAction(QIcon::fromTheme("labplot-xy-interpolation-curve"), i18n("Interpolation"), cartesianPlotAddNewActionGroup);
	addSmoothCurveAction = new QAction(QIcon::fromTheme("labplot-xy-smoothing-curve"), i18n("Smooth"), cartesianPlotAddNewActionGroup);
	addFitCurveAction = new QAction(QIcon::fromTheme("labplot-xy-fit-curve"), i18n("Fit"), cartesianPlotAddNewActionGroup);
	addFourierFilterCurveAction = new QAction(QIcon::fromTheme("labplot-xy-fourier-filter-curve"), i18n("Fourier Filter"), cartesianPlotAddNewActionGroup);
	addFourierTransformCurveAction = new QAction(QIcon::fromTheme("labplot-xy-fourier-transform-curve"), i18n("Fourier Transform"), cartesianPlotAddNewActionGroup);
	addLegendAction = new QAction(QIcon::fromTheme("text-field"), i18n("Legend"), cartesianPlotAddNewActionGroup);
	addHorizontalAxisAction = new QAction(QIcon::fromTheme("labplot-axis-horizontal"), i18n("Horizontal Axis"), cartesianPlotAddNewActionGroup);
	addVerticalAxisAction = new QAction(QIcon::fromTheme("labplot-axis-vertical"), i18n("Vertical Axis"), cartesianPlotAddNewActionGroup);
	addPlotTextLabelAction = new QAction(QIcon::fromTheme("draw-text"), i18n("Text"), cartesianPlotAddNewActionGroup);
	addPlotImageAction = new QAction(QIcon::fromTheme("viewimage"), i18n("Image"), cartesianPlotAddNewActionGroup);
	addCustomPointAction = new QAction(QIcon::fromTheme("draw-cross"), i18n("Custom Point"), cartesianPlotAddNewActionGroup);

	// Analysis menu
	// TODO: no own icons yet
	addDataOperationAction = new QAction(QIcon::fromTheme("labplot-xy-curve"), i18n("Data Operation"), cartesianPlotAddNewActionGroup);
//	addDataOperationAction = new QAction(QIcon::fromTheme("labplot-xy-data-operation-curve"), i18n("Data Operation"), cartesianPlotAddNewActionGroup);
	addDataReductionAction = new QAction(QIcon::fromTheme("labplot-xy-curve"), i18n("Data Reduction"), cartesianPlotAddNewActionGroup);
//	addDataReductionAction = new QAction(QIcon::fromTheme("labplot-xy-data-reduction-curve"), i18n("Data Reduction"), cartesianPlotAddNewActionGroup);
	addDifferentiationAction = new QAction(QIcon::fromTheme("labplot-xy-curve"), i18n("Differentiation"), cartesianPlotAddNewActionGroup);
//	addDifferentiationAction = new QAction(QIcon::fromTheme("labplot-xy-differentiation-curve"), i18n("Differentiation"), cartesianPlotAddNewActionGroup);
	addIntegrationAction = new QAction(QIcon::fromTheme("labplot-xy-curve"), i18n("Integration"), cartesianPlotAddNewActionGroup);
//	addIntegrationAction = new QAction(QIcon::fromTheme("labplot-xy-integration-curve"), i18n("Integration"), cartesianPlotAddNewActionGroup);
	addConvolutionAction = new QAction(QIcon::fromTheme("labplot-xy-curve"), i18n("Convolution/Deconvolution"), cartesianPlotAddNewActionGroup);
//	addConvolutionAction = new QAction(QIcon::fromTheme("labplot-xy-convolution-curve"), i18n("Convolution/Deconvolution"), cartesianPlotAddNewActionGroup);
	addCorrelationAction = new QAction(QIcon::fromTheme("labplot-xy-curve"), i18n("Auto-/Cross-Correlation"), cartesianPlotAddNewActionGroup);
//	addCorrelationAction = new QAction(QIcon::fromTheme("labplot-xy-correlation-curve"), i18n("Auto-/Cross-Correlation"), cartesianPlotAddNewActionGroup);
	addHilbertTransformAction = new QAction(QIcon::fromTheme("labplot-xy-curve"), i18n("Hilbert-Transform"), cartesianPlotAddNewActionGroup);
//	addHilbertTransformAction = new QAction(QIcon::fromTheme("labplot-xy-hilbert-curve"), i18n("Hilbert-Transform"), cartesianPlotAddNewActionGroup);

	addInterpolationAction = new QAction(QIcon::fromTheme("labplot-xy-interpolation-curve"), i18n("Interpolation"), cartesianPlotAddNewActionGroup);
	addSmoothAction = new QAction(QIcon::fromTheme("labplot-xy-smoothing-curve"), i18n("Smooth"), cartesianPlotAddNewActionGroup);
	addFitAction = new QAction(QIcon::fromTheme("labplot-xy-fit-curve"), i18n("Fit"), cartesianPlotAddNewActionGroup);
	addFourierFilterAction = new QAction(QIcon::fromTheme("labplot-xy-fourier-filter-curve"), i18n("Fourier Filter"), cartesianPlotAddNewActionGroup);
	addFourierTransformAction = new QAction(QIcon::fromTheme("labplot-xy-fourier-transform-curve"), i18n("Fourier Transform"), cartesianPlotAddNewActionGroup);

	connect(cartesianPlotAddNewActionGroup, &QActionGroup::triggered, this, &WorksheetView::cartesianPlotAddNew);

	auto* cartesianPlotNavigationGroup = new QActionGroup(this);
	scaleAutoAction = new QAction(QIcon::fromTheme("labplot-auto-scale-all"), i18n("Auto Scale"), cartesianPlotNavigationGroup);
	scaleAutoAction->setData(static_cast<int>(CartesianPlot::NavigationOperation::ScaleAuto));
	scaleAutoXAction = new QAction(QIcon::fromTheme("labplot-auto-scale-x"), i18n("Auto Scale X"), cartesianPlotNavigationGroup);
	scaleAutoXAction->setData(static_cast<int>(CartesianPlot::NavigationOperation::ScaleAutoX));
	scaleAutoYAction = new QAction(QIcon::fromTheme("labplot-auto-scale-y"), i18n("Auto Scale Y"), cartesianPlotNavigationGroup);
	scaleAutoYAction->setData(static_cast<int>(CartesianPlot::NavigationOperation::ScaleAutoY));
	zoomInAction = new QAction(QIcon::fromTheme("zoom-in"), i18n("Zoom In"), cartesianPlotNavigationGroup);
	zoomInAction->setData(static_cast<int>(CartesianPlot::NavigationOperation::ZoomIn));
	zoomOutAction = new QAction(QIcon::fromTheme("zoom-out"), i18n("Zoom Out"), cartesianPlotNavigationGroup);
	zoomOutAction->setData(static_cast<int>(CartesianPlot::NavigationOperation::ZoomOut));
	zoomInXAction = new QAction(QIcon::fromTheme("labplot-zoom-in-x"), i18n("Zoom In X"), cartesianPlotNavigationGroup);
	zoomInXAction->setData(static_cast<int>(CartesianPlot::NavigationOperation::ZoomInX));
	zoomOutXAction = new QAction(QIcon::fromTheme("labplot-zoom-out-x"), i18n("Zoom Out X"), cartesianPlotNavigationGroup);
	zoomOutXAction->setData(static_cast<int>(CartesianPlot::NavigationOperation::ZoomOutX));
	zoomInYAction = new QAction(QIcon::fromTheme("labplot-zoom-in-y"), i18n("Zoom In Y"), cartesianPlotNavigationGroup);
	zoomInYAction->setData(static_cast<int>(CartesianPlot::NavigationOperation::ZoomInY));
	zoomOutYAction = new QAction(QIcon::fromTheme("labplot-zoom-out-y"), i18n("Zoom Out Y"), cartesianPlotNavigationGroup);
	zoomOutYAction->setData(static_cast<int>(CartesianPlot::NavigationOperation::ZoomOutY));
	shiftLeftXAction = new QAction(QIcon::fromTheme("labplot-shift-left-x"), i18n("Shift Left X"), cartesianPlotNavigationGroup);
	shiftLeftXAction->setData(static_cast<int>(CartesianPlot::NavigationOperation::ShiftLeftX));
	shiftRightXAction = new QAction(QIcon::fromTheme("labplot-shift-right-x"), i18n("Shift Right X"), cartesianPlotNavigationGroup);
	shiftRightXAction->setData(static_cast<int>(CartesianPlot::NavigationOperation::ShiftRightX));
	shiftUpYAction = new QAction(QIcon::fromTheme("labplot-shift-up-y"), i18n("Shift Up Y"), cartesianPlotNavigationGroup);
	shiftUpYAction->setData(static_cast<int>(CartesianPlot::NavigationOperation::ShiftUpY));
	shiftDownYAction = new QAction(QIcon::fromTheme("labplot-shift-down-y"), i18n("Shift Down Y"), cartesianPlotNavigationGroup);
	shiftDownYAction->setData(static_cast<int>(CartesianPlot::NavigationOperation::ShiftDownY));

	connect(cartesianPlotNavigationGroup, &QActionGroup::triggered, this, &WorksheetView::cartesianPlotNavigationChanged);

	//set some default values
	selectionModeAction->setChecked(true);
	handleCartesianPlotActions();
	currentZoomAction = zoomInViewAction;
	currentMagnificationAction = noMagnificationAction;

	m_actionsInitialized = true;
}

void WorksheetView::initMenus() {
	if (!m_actionsInitialized)
		initActions();

	m_addNewCartesianPlotMenu = new QMenu(i18n("xy-plot"), this);
	m_addNewCartesianPlotMenu->addAction(addCartesianPlot1Action);
	m_addNewCartesianPlotMenu->addAction(addCartesianPlot2Action);
	m_addNewCartesianPlotMenu->addAction(addCartesianPlot3Action);
	m_addNewCartesianPlotMenu->addAction(addCartesianPlot4Action);

	m_addNewMenu = new QMenu(i18n("Add New"), this);
	m_addNewMenu->setIcon(QIcon::fromTheme("list-add"));
	m_addNewMenu->addMenu(m_addNewCartesianPlotMenu)->setIcon(QIcon::fromTheme("office-chart-line"));
	m_addNewMenu->addSeparator();
	m_addNewMenu->addAction(addTextLabelAction);
	m_addNewMenu->addAction(addImageAction);
	m_addNewMenu->addAction(addGlobalInfoElementAction);

	m_viewMouseModeMenu = new QMenu(i18n("Mouse Mode"), this);
	m_viewMouseModeMenu->setIcon(QIcon::fromTheme("input-mouse"));
	m_viewMouseModeMenu->addAction(selectionModeAction);
	m_viewMouseModeMenu->addAction(navigationModeAction);
	m_viewMouseModeMenu->addAction(zoomSelectionModeAction);

	m_zoomMenu = new QMenu(i18n("Zoom"), this);
	m_zoomMenu->setIcon(QIcon::fromTheme("zoom-draw"));
	m_zoomMenu->addAction(zoomInViewAction);
	m_zoomMenu->addAction(zoomOutViewAction);
	m_zoomMenu->addAction(zoomOriginAction);
	m_zoomMenu->addAction(zoomFitPageHeightAction);
	m_zoomMenu->addAction(zoomFitPageWidthAction);
	m_zoomMenu->addAction(zoomFitSelectionAction);

	m_magnificationMenu = new QMenu(i18n("Magnification"), this);
	m_magnificationMenu->setIcon(QIcon::fromTheme("zoom-in"));
	m_magnificationMenu->addAction(noMagnificationAction);
	m_magnificationMenu->addAction(twoTimesMagnificationAction);
	m_magnificationMenu->addAction(threeTimesMagnificationAction);
	m_magnificationMenu->addAction(fourTimesMagnificationAction);
	m_magnificationMenu->addAction(fiveTimesMagnificationAction);

	m_layoutMenu = new QMenu(i18n("Layout"), this);
	m_layoutMenu->setIcon(QIcon::fromTheme("labplot-editbreaklayout"));
	m_layoutMenu->addAction(verticalLayoutAction);
	m_layoutMenu->addAction(horizontalLayoutAction);
	m_layoutMenu->addAction(gridLayoutAction);
	m_layoutMenu->addSeparator();
	m_layoutMenu->addAction(breakLayoutAction);

	m_gridMenu = new QMenu(i18n("Grid"), this);
	m_gridMenu->setIcon(QIcon::fromTheme("view-grid"));
	m_gridMenu->addAction(noGridAction);
	m_gridMenu->addSeparator();
	m_gridMenu->addAction(sparseLineGridAction);
	m_gridMenu->addAction(denseLineGridAction);
	m_gridMenu->addSeparator();
	m_gridMenu->addAction(sparseDotGridAction);
	m_gridMenu->addAction(denseDotGridAction);
	m_gridMenu->addSeparator();
	m_gridMenu->addAction(customGridAction);
	//TODO: implement "snap to grid" and activate this action
// 	m_gridMenu->addSeparator();
// 	m_gridMenu->addAction(snapToGridAction);

	m_cartesianPlotMenu = new QMenu(i18n("Cartesian Plot"), this);
	m_cartesianPlotMenu->setIcon(QIcon::fromTheme("office-chart-line"));

	m_cartesianPlotMouseModeMenu = new QMenu(i18n("Mouse Mode"), this);
	m_cartesianPlotMouseModeMenu->setIcon(QIcon::fromTheme("input-mouse"));
	m_cartesianPlotMouseModeMenu->addAction(cartesianPlotSelectionModeAction);
	m_cartesianPlotMouseModeMenu->addAction(cartesianPlotZoomSelectionModeAction);
	m_cartesianPlotMouseModeMenu->addAction(cartesianPlotZoomXSelectionModeAction);
	m_cartesianPlotMouseModeMenu->addAction(cartesianPlotZoomYSelectionModeAction);
	m_cartesianPlotMouseModeMenu->addSeparator();
	m_cartesianPlotMouseModeMenu->addAction(cartesianPlotCursorModeAction);
	m_cartesianPlotMouseModeMenu->addSeparator();

	m_cartesianPlotAddNewMenu = new QMenu(i18n("Add New"), this);
	m_cartesianPlotAddNewMenu->setIcon(QIcon::fromTheme("list-add"));
	m_cartesianPlotAddNewMenu->addAction(addCurveAction);
	m_cartesianPlotAddNewMenu->addAction(addHistogramAction);
	m_cartesianPlotAddNewMenu->addAction(addEquationCurveAction);
	m_cartesianPlotAddNewMenu->addSeparator();

	m_cartesianPlotAddNewAnalysisMenu = new QMenu(i18n("Analysis Curve"));
	m_cartesianPlotAddNewAnalysisMenu->addAction(addFitCurveAction);
	m_cartesianPlotAddNewAnalysisMenu->addSeparator();
	m_cartesianPlotAddNewAnalysisMenu->addAction(addDifferentiationCurveAction);
	m_cartesianPlotAddNewAnalysisMenu->addAction(addIntegrationCurveAction);
	m_cartesianPlotAddNewAnalysisMenu->addSeparator();
	m_cartesianPlotAddNewAnalysisMenu->addAction(addInterpolationCurveAction);
	m_cartesianPlotAddNewAnalysisMenu->addAction(addSmoothCurveAction);
	m_cartesianPlotAddNewAnalysisMenu->addSeparator();
	m_cartesianPlotAddNewAnalysisMenu->addAction(addFourierFilterCurveAction);
	m_cartesianPlotAddNewAnalysisMenu->addAction(addFourierTransformCurveAction);
	m_cartesianPlotAddNewAnalysisMenu->addSeparator();
	m_cartesianPlotAddNewAnalysisMenu->addAction(addConvolutionCurveAction);
	m_cartesianPlotAddNewAnalysisMenu->addAction(addCorrelationCurveAction);
	m_cartesianPlotAddNewAnalysisMenu->addSeparator();
// 	m_cartesianPlotAddNewAnalysisMenu->addAction(addDataOperationCurveAction);
	m_cartesianPlotAddNewAnalysisMenu->addAction(addDataReductionCurveAction);
	m_cartesianPlotAddNewMenu->addMenu(m_cartesianPlotAddNewAnalysisMenu);

	m_cartesianPlotAddNewMenu->addSeparator();
	m_cartesianPlotAddNewMenu->addAction(addLegendAction);
	m_cartesianPlotAddNewMenu->addSeparator();
	m_cartesianPlotAddNewMenu->addAction(addHorizontalAxisAction);
	m_cartesianPlotAddNewMenu->addAction(addVerticalAxisAction);
	m_cartesianPlotAddNewMenu->addSeparator();
	m_cartesianPlotAddNewMenu->addAction(addPlotTextLabelAction);
	m_cartesianPlotAddNewMenu->addAction(addPlotImageAction);
	m_cartesianPlotAddNewMenu->addSeparator();
	m_cartesianPlotAddNewMenu->addAction(addCustomPointAction);

	m_cartesianPlotZoomMenu = new QMenu(i18n("Zoom/Navigate"), this);
	m_cartesianPlotZoomMenu->setIcon(QIcon::fromTheme("zoom-draw"));
	m_cartesianPlotZoomMenu->addAction(scaleAutoAction);
	m_cartesianPlotZoomMenu->addAction(scaleAutoXAction);
	m_cartesianPlotZoomMenu->addAction(scaleAutoYAction);
	m_cartesianPlotZoomMenu->addSeparator();
	m_cartesianPlotZoomMenu->addAction(zoomInAction);
	m_cartesianPlotZoomMenu->addAction(zoomOutAction);
	m_cartesianPlotZoomMenu->addSeparator();
	m_cartesianPlotZoomMenu->addAction(zoomInXAction);
	m_cartesianPlotZoomMenu->addAction(zoomOutXAction);
	m_cartesianPlotZoomMenu->addSeparator();
	m_cartesianPlotZoomMenu->addAction(zoomInYAction);
	m_cartesianPlotZoomMenu->addAction(zoomOutYAction);
	m_cartesianPlotZoomMenu->addSeparator();
	m_cartesianPlotZoomMenu->addAction(shiftLeftXAction);
	m_cartesianPlotZoomMenu->addAction(shiftRightXAction);
	m_cartesianPlotZoomMenu->addSeparator();
	m_cartesianPlotZoomMenu->addAction(shiftUpYAction);
	m_cartesianPlotZoomMenu->addAction(shiftDownYAction);

	m_cartesianPlotActionModeMenu = new QMenu(i18n("Apply Actions to"), this);
	m_cartesianPlotActionModeMenu->setIcon(QIcon::fromTheme("dialog-ok-apply"));
	m_cartesianPlotActionModeMenu->addAction(cartesianPlotApplyToSelectionAction);
	m_cartesianPlotActionModeMenu->addAction(cartesianPlotApplyToAllAction);
	m_cartesianPlotActionModeMenu->addAction(cartesianPlotApplyToAllXAction);
	m_cartesianPlotActionModeMenu->addAction(cartesianPlotApplyToAllYAction);

	m_cartesianPlotCursorModeMenu = new QMenu(i18n("Apply Cursor to"), this);
	m_cartesianPlotCursorModeMenu->addAction(cartesianPlotApplyToSelectionCursor);
	m_cartesianPlotCursorModeMenu->addAction(cartesianPlotApplyToAllCursor);

	m_cartesianPlotMenu->addMenu(m_cartesianPlotAddNewMenu);
	m_cartesianPlotMenu->addSeparator();
	m_cartesianPlotMenu->addMenu(m_cartesianPlotMouseModeMenu);
	m_cartesianPlotMenu->addMenu(m_cartesianPlotZoomMenu);
	m_cartesianPlotMenu->addSeparator();
	m_cartesianPlotMenu->addMenu(m_cartesianPlotActionModeMenu);
	m_cartesianPlotMenu->addMenu(m_cartesianPlotCursorModeMenu);
	m_cartesianPlotMenu->addSeparator();
	m_cartesianPlotMenu->addAction(plotsLockedAction);

	// Data manipulation menu
	m_dataManipulationMenu = new QMenu(i18n("Data Manipulation"),this);
	m_dataManipulationMenu->setIcon(QIcon::fromTheme("zoom-draw"));
	m_dataManipulationMenu->addAction(addDataOperationAction);
	m_dataManipulationMenu->addAction(addDataReductionAction);

	//themes menu
	m_themeMenu = new QMenu(i18n("Apply Theme"), this);
	m_themeMenu->setIcon(QIcon::fromTheme("color-management"));
	auto* themeWidget = new ThemesWidget(nullptr);
	themeWidget->setFixedMode();
	connect(themeWidget, &ThemesWidget::themeSelected, m_worksheet, &Worksheet::setTheme);
	connect(themeWidget, &ThemesWidget::themeSelected, m_themeMenu, &QMenu::close);

	auto* widgetAction = new QWidgetAction(this);
	widgetAction->setDefaultWidget(themeWidget);
	m_themeMenu->addAction(widgetAction);

	m_menusInitialized = true;
}

/*!
 * Populates the menu \c menu with the worksheet and worksheet view relevant actions.
 * The menu is used
 *   - as the context menu in WorksheetView
 *   - as the "worksheet menu" in the main menu-bar (called form MainWin)
 *   - as a part of the worksheet context menu in project explorer
 */
void WorksheetView::createContextMenu(QMenu* menu) {
	Q_ASSERT(menu != nullptr);

	if (!m_menusInitialized)
		initMenus();

	QAction* firstAction = nullptr;
	// if we're populating the context menu for the project explorer, then
	//there're already actions available there. Skip the first title-action
	//and insert the action at the beginning of the menu.
	if (menu->actions().size() > 1)
		firstAction = menu->actions().at(1);

	menu->insertMenu(firstAction, m_addNewMenu);
	menu->insertSeparator(firstAction);
	menu->insertMenu(firstAction, m_viewMouseModeMenu);
	menu->insertMenu(firstAction, m_zoomMenu);
	menu->insertMenu(firstAction, m_magnificationMenu);
	menu->insertMenu(firstAction, m_layoutMenu);
	menu->insertMenu(firstAction, m_gridMenu);
	menu->insertMenu(firstAction, m_themeMenu);
	menu->insertSeparator(firstAction);
	menu->insertAction(firstAction, plotsLockedAction);
	menu->insertSeparator(firstAction);
	menu->insertMenu(firstAction, m_cartesianPlotMenu);
	menu->insertSeparator(firstAction);
	menu->insertAction(firstAction, showPresenterMode);
	menu->insertSeparator(firstAction);
}

void WorksheetView::createAnalysisMenu(QMenu* menu) {
	Q_ASSERT(menu != nullptr);

	if (!m_menusInitialized)
		initMenus();

	// Data manipulation menu
// 	menu->insertMenu(nullptr, m_dataManipulationMenu);

	menu->addAction(addFitAction);
	menu->addSeparator();
	menu->addAction(addDifferentiationAction);
	menu->addAction(addIntegrationAction);
	menu->addSeparator();
	menu->addAction(addInterpolationAction);
	menu->addAction(addSmoothAction);
	menu->addSeparator();
	menu->addAction(addFourierFilterAction);
	menu->addAction(addFourierTransformAction);
	menu->addAction(addHilbertTransformAction);
	menu->addSeparator();
	menu->addAction(addConvolutionAction);
	menu->addAction(addCorrelationAction);
	menu->addSeparator();
	menu->addAction(addDataReductionAction);
}

void WorksheetView::fillToolBar(QToolBar* toolBar) {
	toolBar->addSeparator();
	tbNewCartesianPlot = new QToolButton(toolBar);
	tbNewCartesianPlot->setPopupMode(QToolButton::MenuButtonPopup);
	tbNewCartesianPlot->setMenu(m_addNewCartesianPlotMenu);
	tbNewCartesianPlot->setDefaultAction(addCartesianPlot1Action);
	toolBar->addWidget(tbNewCartesianPlot);
	toolBar->addAction(addTextLabelAction);
	toolBar->addAction(addImageAction);

	toolBar->addSeparator();
	toolBar->addAction(verticalLayoutAction);
	toolBar->addAction(horizontalLayoutAction);
	toolBar->addAction(gridLayoutAction);
	toolBar->addAction(breakLayoutAction);

	toolBar->addSeparator();
	toolBar->addAction(selectionModeAction);
	toolBar->addAction(navigationModeAction);
	toolBar->addAction(zoomSelectionModeAction);
	toolBar->addSeparator();
	tbZoom = new QToolButton(toolBar);
	tbZoom->setPopupMode(QToolButton::MenuButtonPopup);
	tbZoom->setMenu(m_zoomMenu);
	tbZoom->setDefaultAction(currentZoomAction);
	toolBar->addWidget(tbZoom);

	tbMagnification = new QToolButton(toolBar);
	tbMagnification->setPopupMode(QToolButton::MenuButtonPopup);
	tbMagnification->setMenu(m_magnificationMenu);
	tbMagnification->setDefaultAction(currentMagnificationAction);
	toolBar->addWidget(tbMagnification);
}

#ifdef Q_OS_MAC
void WorksheetView::fillTouchBar(KDMacTouchBar* touchBar) {
	//touchBar->addAction(addCartesianPlot1Action);
	touchBar->addAction(zoomInViewAction);
	touchBar->addAction(zoomOutViewAction);
	touchBar->addAction(showPresenterMode);
}
#endif

void WorksheetView::fillCartesianPlotToolBar(QToolBar* toolBar) {
	toolBar->addAction(cartesianPlotSelectionModeAction);
	toolBar->addAction(cartesianPlotCrosshairModeAction);
	toolBar->addAction(cartesianPlotZoomSelectionModeAction);
	toolBar->addAction(cartesianPlotZoomXSelectionModeAction);
	toolBar->addAction(cartesianPlotZoomYSelectionModeAction);
	toolBar->addAction(cartesianPlotCursorModeAction);
	toolBar->addSeparator();
	toolBar->addAction(addCurveAction);
	toolBar->addAction(addHistogramAction);
	toolBar->addAction(addEquationCurveAction);
// don't over-populate the tool bar
//	toolBar->addAction(addDifferentiationCurveAction);
//	toolBar->addAction(addIntegrationCurveAction);
//	toolBar->addAction(addDataOperationCurveAction);
//	toolBar->addAction(addDataReductionCurveAction);
//	toolBar->addAction(addInterpolationCurveAction);
//	toolBar->addAction(addSmoothCurveAction);
//	toolBar->addAction(addFitCurveAction);
//	toolBar->addAction(addFourierFilterCurveAction);
//	toolBar->addAction(addFourierTransformCurveAction);
//	toolBar->addAction(addConvolutionCurveAction);
//	toolBar->addAction(addCorrelationCurveAction);
	toolBar->addSeparator();
	toolBar->addAction(addLegendAction);
	toolBar->addSeparator();
	toolBar->addAction(addHorizontalAxisAction);
	toolBar->addAction(addVerticalAxisAction);
	toolBar->addSeparator();
	toolBar->addAction(addPlotTextLabelAction);
	toolBar->addAction(addPlotImageAction);
	toolBar->addSeparator();
	toolBar->addAction(scaleAutoAction);
	toolBar->addAction(scaleAutoXAction);
	toolBar->addAction(scaleAutoYAction);
	toolBar->addAction(zoomInAction);
	toolBar->addAction(zoomOutAction);
	toolBar->addAction(zoomInXAction);
	toolBar->addAction(zoomOutXAction);
	toolBar->addAction(zoomInYAction);
	toolBar->addAction(zoomOutYAction);
	toolBar->addAction(shiftLeftXAction);
	toolBar->addAction(shiftRightXAction);
	toolBar->addAction(shiftUpYAction);
	toolBar->addAction(shiftDownYAction);
	toolBar->addSeparator();

	handleCartesianPlotActions();
}

void WorksheetView::setScene(QGraphicsScene* scene) {
	QGraphicsView::setScene(scene);
}

void WorksheetView::setIsClosing() {
	m_isClosing = true;
}

void WorksheetView::setCartesianPlotActionMode(Worksheet::CartesianPlotActionMode mode) {
	if (mode == Worksheet::CartesianPlotActionMode::ApplyActionToAll)
		cartesianPlotApplyToAllAction->setChecked(true);
	else if (mode == Worksheet::CartesianPlotActionMode::ApplyActionToAllX)
		cartesianPlotApplyToAllXAction->setChecked(true);
	else if (mode == Worksheet::CartesianPlotActionMode::ApplyActionToAllY)
		cartesianPlotApplyToAllYAction->setChecked(true);
	else
		cartesianPlotApplyToSelectionAction->setChecked(true);
}

void WorksheetView::setCartesianPlotCursorMode(Worksheet::CartesianPlotActionMode mode) {
	if (mode == Worksheet::CartesianPlotActionMode::ApplyActionToAll)
		cartesianPlotApplyToAllCursor->setChecked(true);
	else
		cartesianPlotApplyToSelectionCursor->setChecked(true);
}

void WorksheetView::setPlotLock(bool lock) {
	plotsLockedAction->setChecked(lock);
}

void WorksheetView::drawForeground(QPainter* painter, const QRectF& rect) {
	if (m_mouseMode == MouseMode::ZoomSelection && m_selectionBandIsShown) {
		painter->save();
		const QRectF& selRect = mapToScene(QRect(m_selectionStart, m_selectionEnd).normalized()).boundingRect();
		//TODO: don't hardcode for black here, use a a different color depending on the theme of the worksheet/plot under the mouse cursor?
		painter->setPen(QPen(Qt::black, 5/transform().m11()));
		painter->drawRect(selRect);
		painter->setBrush(QApplication::palette().color(QPalette::Highlight));
		painter->setOpacity(0.2);
		painter->drawRect(selRect);
		painter->restore();
	}
	QGraphicsView::drawForeground(painter, rect);
}

void WorksheetView::drawBackgroundItems(QPainter* painter, const QRectF& scene_rect) {
	// canvas
	painter->setOpacity(m_worksheet->backgroundOpacity());
	if (m_worksheet->backgroundType() == PlotArea::BackgroundType::Color) {
		switch (m_worksheet->backgroundColorStyle()) {
		case PlotArea::BackgroundColorStyle::SingleColor: {
				painter->setBrush(QBrush(m_worksheet->backgroundFirstColor()));
				break;
			}
		case PlotArea::BackgroundColorStyle::HorizontalLinearGradient: {
				QLinearGradient linearGrad(scene_rect.topLeft(), scene_rect.topRight());
				linearGrad.setColorAt(0, m_worksheet->backgroundFirstColor());
				linearGrad.setColorAt(1, m_worksheet->backgroundSecondColor());
				painter->setBrush(QBrush(linearGrad));
				break;
			}
		case PlotArea::BackgroundColorStyle::VerticalLinearGradient: {
				QLinearGradient linearGrad(scene_rect.topLeft(), scene_rect.bottomLeft());
				linearGrad.setColorAt(0, m_worksheet->backgroundFirstColor());
				linearGrad.setColorAt(1, m_worksheet->backgroundSecondColor());
				painter->setBrush(QBrush(linearGrad));
				break;
			}
		case PlotArea::BackgroundColorStyle::TopLeftDiagonalLinearGradient: {
				QLinearGradient linearGrad(scene_rect.topLeft(), scene_rect.bottomRight());
				linearGrad.setColorAt(0, m_worksheet->backgroundFirstColor());
				linearGrad.setColorAt(1, m_worksheet->backgroundSecondColor());
				painter->setBrush(QBrush(linearGrad));
				break;
			}
		case PlotArea::BackgroundColorStyle::BottomLeftDiagonalLinearGradient: {
				QLinearGradient linearGrad(scene_rect.bottomLeft(), scene_rect.topRight());
				linearGrad.setColorAt(0, m_worksheet->backgroundFirstColor());
				linearGrad.setColorAt(1, m_worksheet->backgroundSecondColor());
				painter->setBrush(QBrush(linearGrad));
				break;
			}
		case PlotArea::BackgroundColorStyle::RadialGradient: {
				QRadialGradient radialGrad(scene_rect.center(), scene_rect.width()/2);
				radialGrad.setColorAt(0, m_worksheet->backgroundFirstColor());
				radialGrad.setColorAt(1, m_worksheet->backgroundSecondColor());
				painter->setBrush(QBrush(radialGrad));
				break;
			}
			//default:
			//	painter->setBrush(QBrush(m_worksheet->backgroundFirstColor()));
		}
		painter->drawRect(scene_rect);
	} else if (m_worksheet->backgroundType() == PlotArea::BackgroundType::Image) {	// background image
		const QString& backgroundFileName = m_worksheet->backgroundFileName().trimmed();
		if ( !backgroundFileName.isEmpty() ) {
			QPixmap pix(backgroundFileName);
			switch (m_worksheet->backgroundImageStyle()) {
			case PlotArea::BackgroundImageStyle::ScaledCropped:
				pix = pix.scaled(scene_rect.size().toSize(),Qt::KeepAspectRatioByExpanding,Qt::SmoothTransformation);
				painter->drawPixmap(scene_rect.topLeft(),pix);
				break;
			case PlotArea::BackgroundImageStyle::Scaled:
				pix = pix.scaled(scene_rect.size().toSize(),Qt::IgnoreAspectRatio,Qt::SmoothTransformation);
				painter->drawPixmap(scene_rect.topLeft(),pix);
				break;
			case PlotArea::BackgroundImageStyle::ScaledAspectRatio:
				pix = pix.scaled(scene_rect.size().toSize(),Qt::KeepAspectRatio,Qt::SmoothTransformation);
				painter->drawPixmap(scene_rect.topLeft(),pix);
				break;
			case PlotArea::BackgroundImageStyle::Centered:
				painter->drawPixmap(QPointF(scene_rect.center().x()-pix.size().width()/2,scene_rect.center().y()-pix.size().height()/2),pix);
				break;
			case PlotArea::BackgroundImageStyle::Tiled:
				painter->drawTiledPixmap(scene_rect,pix);
				break;
			case PlotArea::BackgroundImageStyle::CenterTiled:
				painter->drawTiledPixmap(scene_rect,pix,QPoint(scene_rect.size().width()/2,scene_rect.size().height()/2));
				break;
				//default:
				//	painter->drawPixmap(scene_rect.topLeft(),pix);
			}
		}
	} else if (m_worksheet->backgroundType() == PlotArea::BackgroundType::Pattern) {	// background pattern
		painter->setBrush(QBrush(m_worksheet->backgroundFirstColor(),m_worksheet->backgroundBrushStyle()));
		painter->drawRect(scene_rect);
	}

	//grid
	if (m_gridSettings.style != GridStyle::NoGrid && !m_isPrinting) {
		QColor c = m_gridSettings.color;
		c.setAlphaF(m_gridSettings.opacity);
		painter->setPen(c);

		qreal x, y;
		qreal left = scene_rect.left();
		qreal right = scene_rect.right();
		qreal top = scene_rect.top();
		qreal bottom = scene_rect.bottom();

		if (m_gridSettings.style == GridStyle::Line) {
			QLineF line;

			//horizontal lines
			y = top + m_gridSettings.verticalSpacing;
			while (y < bottom) {
				line.setLine( left, y,  right, y );
				painter->drawLine(line);
				y += m_gridSettings.verticalSpacing;
			}

			//vertical lines
			x = left + m_gridSettings.horizontalSpacing;
			while (x < right) {
				line.setLine( x, top,  x, bottom );
				painter->drawLine(line);
				x += m_gridSettings.horizontalSpacing;
			}
		} else { //DotGrid
			y = top + m_gridSettings.verticalSpacing;
			while (y < bottom) {
				x = left;// + m_gridSettings.horizontalSpacing;
				while (x < right) {
					x += m_gridSettings.horizontalSpacing;
					painter->drawPoint(x, y);
				}
				y += m_gridSettings.verticalSpacing;
			}
		}
	}
}

void WorksheetView::drawBackground(QPainter* painter, const QRectF& rect) {
	painter->save();

	//painter->setRenderHint(QPainter::Antialiasing);
	QRectF scene_rect = sceneRect();

	if (!m_worksheet->useViewSize()) {
		// background
		KColorScheme scheme(QPalette::Active, KColorScheme::Window);
		const QColor& color = scheme.background().color();
		if (!scene_rect.contains(rect))
			painter->fillRect(rect, color);

		//shadow
// 		int shadowSize = scene_rect.width()*0.02;
// 		QRectF rightShadowRect(scene_rect.right(), scene_rect.top() + shadowSize, shadowSize, scene_rect.height());
// 		QRectF bottomShadowRect(scene_rect.left() + shadowSize, scene_rect.bottom(), scene_rect.width(), shadowSize);
//
// 		const QColor& shadeColor = scheme.shade(color, KColorScheme::MidShade);
// 		painter->fillRect(rightShadowRect.intersected(rect), shadeColor);
// 		painter->fillRect(bottomShadowRect.intersected(rect), shadeColor);
	}

	drawBackgroundItems(painter, scene_rect);

	invalidateScene(rect, QGraphicsScene::BackgroundLayer);
	painter->restore();
}

bool WorksheetView::isPlotAtPos(QPoint pos) const {
	bool plot = false;
	QGraphicsItem* item = itemAt(pos);
	if (item) {
		plot = item->data(0).toInt() == static_cast<int>(WorksheetElement::WorksheetElementName::NameCartesianPlot);
		if (!plot && item->parentItem())
			plot = item->parentItem()->data(0).toInt() == static_cast<int>(WorksheetElement::WorksheetElementName::NameCartesianPlot);
	}

	return plot;
}

CartesianPlot* WorksheetView::plotAt(QPoint pos) const {
	QGraphicsItem* item = itemAt(pos);
	if (!item)
		return nullptr;

	QGraphicsItem* plotItem = nullptr;
	if (item->data(0).toInt() == static_cast<int>(WorksheetElement::WorksheetElementName::NameCartesianPlot))
		plotItem = item;
	else {
		if (item->parentItem() && item->parentItem()->data(0).toInt() == static_cast<int>(WorksheetElement::WorksheetElementName::NameCartesianPlot))
			plotItem = item->parentItem();
	}

	if (plotItem == nullptr)
		return nullptr;

	CartesianPlot* plot = nullptr;
	for (auto* p : m_worksheet->children<CartesianPlot>()) {
		if (p->graphicsItem() == plotItem) {
			plot = p;
			break;
		}
	}

	return plot;
}

//##############################################################################
//####################################  Events   ###############################
//##############################################################################
void WorksheetView::resizeEvent(QResizeEvent* event) {
	if (m_isClosing)
		return;

	if (m_worksheet->useViewSize())
		this->processResize();

	QGraphicsView::resizeEvent(event);
}

void WorksheetView::wheelEvent(QWheelEvent* event) {
	//https://wiki.qt.io/Smooth_Zoom_In_QGraphicsView
	if (m_mouseMode == MouseMode::ZoomSelection || (QApplication::keyboardModifiers() & Qt::ControlModifier)) {
		int numDegrees = event->delta() / 8;
		int numSteps = numDegrees / 15; // see QWheelEvent documentation
		zoom(numSteps);
	} else
		QGraphicsView::wheelEvent(event);
}

void WorksheetView::zoom(int numSteps) {
	m_numScheduledScalings += numSteps;
	if (m_numScheduledScalings * numSteps < 0) // if user moved the wheel in another direction, we reset previously scheduled scalings
		m_numScheduledScalings = numSteps;

	auto* anim = new QTimeLine(350, this);
	anim->setUpdateInterval(20);

	connect(anim, &QTimeLine::valueChanged, this, &WorksheetView::scalingTime);
	connect(anim, &QTimeLine::finished, this, &WorksheetView::animFinished);
	anim->start();
}

void WorksheetView::scalingTime() {
	qreal factor = 1.0 + qreal(m_numScheduledScalings) / 300.0;
	scale(factor, factor);
}

void WorksheetView::animFinished() {
	if (m_numScheduledScalings > 0)
		m_numScheduledScalings--;
	else
		m_numScheduledScalings++;
	sender()->~QObject();
}

void WorksheetView::mousePressEvent(QMouseEvent* event) {
	//prevent the deselection of items when context menu event
	//was triggered (right button click)
	if (event->button() == Qt::RightButton) {
		event->accept();
		return;
	}

	if (event->button() == Qt::LeftButton && m_mouseMode == MouseMode::ZoomSelection) {
		m_selectionStart = event->pos();
		m_selectionEnd = m_selectionStart; //select&zoom'g starts -> reset the end point to the start point
		m_selectionBandIsShown = true;
		QGraphicsView::mousePressEvent(event);
		return;
	}

	// select the worksheet in the project explorer if the view was clicked
	// and there is no selection currently. We need this for the case when
	// there is a single worksheet in the project and we change from the project-node
	// in the project explorer to the worksheet-node by clicking the view.
	if ( scene()->selectedItems().isEmpty() )
		m_worksheet->setSelectedInView(true);

	QGraphicsView::mousePressEvent(event);
}

void WorksheetView::mouseReleaseEvent(QMouseEvent* event) {
	if (event->button() == Qt::LeftButton && m_mouseMode == MouseMode::ZoomSelection) {
		m_selectionBandIsShown = false;
		viewport()->repaint(QRect(m_selectionStart, m_selectionEnd).normalized());

		//don't zoom if very small region was selected, avoid occasional/unwanted zooming
		m_selectionEnd = event->pos();
		if ( abs(m_selectionEnd.x() - m_selectionStart.x()) > 20 && abs(m_selectionEnd.y() - m_selectionStart.y()) > 20 )
			fitInView(mapToScene(QRect(m_selectionStart, m_selectionEnd).normalized()).boundingRect(), Qt::KeepAspectRatio);
	}

	QGraphicsView::mouseReleaseEvent(event);
}

void WorksheetView::mouseDoubleClickEvent(QMouseEvent*) {
	emit propertiesExplorerRequested();
}

void WorksheetView::mouseMoveEvent(QMouseEvent* event) {
	if (m_suppressSelectionChangedEvent)
		return QGraphicsView::mouseMoveEvent(event);
	if (m_mouseMode == MouseMode::Selection && m_cartesianPlotMouseMode != CartesianPlot::MouseMode::Selection ) {
		//check whether there is a cartesian plot under the cursor
		//and set the cursor appearance according to the current mouse mode for the cartesian plots
		if ( isPlotAtPos(event->pos()) ) {
			if (m_cartesianPlotMouseMode == CartesianPlot::MouseMode::ZoomSelection)
				setCursor(Qt::CrossCursor);
			else if (m_cartesianPlotMouseMode == CartesianPlot::MouseMode::ZoomXSelection)
				setCursor(Qt::SizeHorCursor);
			else if (m_cartesianPlotMouseMode == CartesianPlot::MouseMode::ZoomYSelection)
				setCursor(Qt::SizeVerCursor);
		} else
			setCursor(Qt::ArrowCursor);
	} else if (m_mouseMode == MouseMode::Selection && m_cartesianPlotMouseMode == CartesianPlot::MouseMode::Selection)
		setCursor(Qt::ArrowCursor);
	else if (m_selectionBandIsShown) {
		QRect rect = QRect(m_selectionStart, m_selectionEnd).normalized();
		m_selectionEnd = event->pos();
		rect = rect.united(QRect(m_selectionStart, m_selectionEnd).normalized());
		qreal penWidth = 5/transform().m11();
		rect.setX(rect.x()-penWidth);
		rect.setY(rect.y()-penWidth);
		rect.setHeight(rect.height()+2*penWidth);
		rect.setWidth(rect.width()+2*penWidth);
		viewport()->repaint(rect);
	}

	//show the magnification window
	if (magnificationFactor /*&& m_mouseMode == SelectAndEditMode*/) {
		if (!m_magnificationWindow) {
			m_magnificationWindow = new QGraphicsPixmapItem(nullptr);
			m_magnificationWindow->setZValue(std::numeric_limits<int>::max());
			scene()->addItem(m_magnificationWindow);
		}

		m_magnificationWindow->setVisible(false);

		//copy the part of the view to be shown magnified
		QPointF pos = mapToScene(event->pos());
		const int size = Worksheet::convertToSceneUnits(2.0, Worksheet::Unit::Centimeter)/transform().m11();

		const QRectF copyRect(pos.x() - size/(2*magnificationFactor), pos.y() - size/(2*magnificationFactor), size/magnificationFactor, size/magnificationFactor);
		QPixmap px = grab(mapFromScene(copyRect).boundingRect());
		px = px.scaled(size, size, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

		//draw the bounding rect
		QPainter painter(&px);
		const QPen pen = QPen(Qt::lightGray, 2/transform().m11());
		painter.setPen(pen);
		QRect rect = px.rect();
		rect.setWidth(rect.width()-pen.widthF()/2);
		rect.setHeight(rect.height()-pen.widthF()/2);
		painter.drawRect(rect);

		//set the pixmap
		m_magnificationWindow->setPixmap(px);
		m_magnificationWindow->setPos(pos.x()- px.width()/2, pos.y()- px.height()/2);

		m_magnificationWindow->setVisible(true);
	} else if (m_magnificationWindow)
		m_magnificationWindow->setVisible(false);

	QGraphicsView::mouseMoveEvent(event);
}

void WorksheetView::contextMenuEvent(QContextMenuEvent* e) {
	if ( (m_magnificationWindow && m_magnificationWindow->isVisible() && items(e->pos()).size() == 1) || !itemAt(e->pos()) ) {
		//no item or only the magnification window under the cursor -> show the context menu for the worksheet
		QMenu* menu = m_worksheet->createContextMenu();
		m_cursorPos = mapToScene(e->pos());
		menu->exec(QCursor::pos());
	} else {
		//propagate the event to the scene and graphics items
		QGraphicsView::contextMenuEvent(e);
	}
}

void WorksheetView::keyPressEvent(QKeyEvent* event) {
	//handle delete
	if (event->matches(QKeySequence::Delete)) {
		deleteElement();
		QGraphicsView::keyPressEvent(event);
		return;
	}

	//handle copy/paste/duplicate

	//determine the currently selected aspect
	AbstractAspect* aspect = nullptr;
	if (m_selectedItems.count() == 1) {
		//at the moment we allow to copy/paste/duplicate one single selcted object only
		const auto children = m_worksheet->children<WorksheetElement>(AbstractAspect::ChildIndexFlag::Recursive);
		const auto* item = m_selectedItems.constFirst();
		for (auto* child : children) {
			if (child->graphicsItem() == item) {
				aspect = child;
				break;
			}
		}
	} else
		aspect = m_worksheet;

	if (event->matches(QKeySequence::Copy)) {
		exportToClipboard(); //export the image to the clipboard
		if (aspect != m_worksheet)
			aspect->copy(); //copy the selected object itself
	} else if (event->matches(QKeySequence::Paste)) {
		//paste
		QString name;
		auto t = AbstractAspect::clipboardAspectType(name);
		if (t != AspectType::AbstractAspect && aspect->pasteTypes().indexOf(t) != -1)
			aspect->paste();
	} else if ( (event->modifiers() & Qt::ControlModifier) && (event->key() == Qt::Key_D) && aspect != m_worksheet) {
		//duplicate
		aspect->copy();
		aspect->parentAspect()->paste(true);
	} else if (event->key() == 32) {
		//space key - hide/show the current object
		auto* we = dynamic_cast<WorksheetElement*>(aspect);
		if (we)
			we->setVisible(!we->isVisible());
	}

	QGraphicsView::keyPressEvent(event);
}

void WorksheetView::keyReleaseEvent(QKeyEvent* event) {
	QGraphicsView::keyReleaseEvent(event);
}

void WorksheetView::dragEnterEvent(QDragEnterEvent* event) {
	//ignore events not related to internal drags of columns etc., e.g. dropping of external files onto LabPlot
	const QMimeData* mimeData = event->mimeData();
	if (!mimeData) {
		event->ignore();
		return;
	}

	if (mimeData->formats().at(0) != QLatin1String("labplot-dnd")) {
		event->ignore();
		return;
	}

	//select the worksheet in the project explorer and bring the view to the foreground
	m_worksheet->setSelectedInView(true);
	m_worksheet->mdiSubWindow()->mdiArea()->setActiveSubWindow(m_worksheet->mdiSubWindow());

	event->setAccepted(true);
}

void WorksheetView::dragMoveEvent(QDragMoveEvent* event) {
	// only accept drop events if we have a plot under the cursor where we can drop columns onto
	bool plot = isPlotAtPos(event->pos());
	event->setAccepted(plot);
}

void WorksheetView::dropEvent(QDropEvent* event) {
	CartesianPlot* plot = plotAt(event->pos());
	if (!plot)
		return;

	const QMimeData* mimeData = event->mimeData();
	plot->processDropEvent(plot->project()->droppedAspects(mimeData));
}

//##############################################################################
//####################################  SLOTs   ################################
//##############################################################################
void WorksheetView::useViewSizeRequested() {
	if (!m_actionsInitialized)
		initActions();

	if (m_worksheet->useViewSize()) {
		setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
		setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
		zoomFitPageHeightAction->setVisible(false);
		zoomFitPageWidthAction->setVisible(false);
		currentZoomAction = zoomInViewAction;
		if (tbZoom)
			tbZoom->setDefaultAction(zoomInViewAction);

		//determine and set the current view size
		this->processResize();
	} else {
		setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
		setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
		zoomFitPageHeightAction->setVisible(true);
		zoomFitPageWidthAction->setVisible(true);
	}
}

void WorksheetView::processResize() {
	if (size() != sceneRect().size()) {
		static const float hscale = QApplication::desktop()->physicalDpiX()/(Worksheet::convertToSceneUnits(1, Worksheet::Unit::Inch));
		static const float vscale = QApplication::desktop()->physicalDpiY()/(Worksheet::convertToSceneUnits(1, Worksheet::Unit::Inch));
		m_worksheet->setUndoAware(false);
		m_worksheet->setPageRect(QRectF(0.0, 0.0, width()/hscale, height()/vscale));
		m_worksheet->setUndoAware(true);
	}
}

void WorksheetView::changeZoom(QAction* action) {
	if (action == zoomInViewAction)
		zoom(1);
	else if (action == zoomOutViewAction)
		zoom(-1);
	else if (action == zoomOriginAction) {
		static const float hscale = QApplication::desktop()->physicalDpiX()/(Worksheet::convertToSceneUnits(1, Worksheet::Unit::Inch));
		static const float vscale = QApplication::desktop()->physicalDpiY()/(Worksheet::convertToSceneUnits(1, Worksheet::Unit::Inch));
		setTransform(QTransform::fromScale(hscale, vscale));
	} else if (action == zoomFitPageWidthAction) {
		float scaleFactor = viewport()->width()/scene()->sceneRect().width();
		setTransform(QTransform::fromScale(scaleFactor, scaleFactor));
	} else if (action == zoomFitPageHeightAction) {
		float scaleFactor = viewport()->height()/scene()->sceneRect().height();
		setTransform(QTransform::fromScale(scaleFactor, scaleFactor));
	} else if (action == zoomFitSelectionAction)
		fitInView(scene()->selectionArea().boundingRect(),Qt::KeepAspectRatio);
	currentZoomAction = action;
	if (tbZoom)
		tbZoom->setDefaultAction(action);
}

void WorksheetView::magnificationChanged(QAction* action) {
	if (action == noMagnificationAction)
		magnificationFactor = 0;
	else if (action == twoTimesMagnificationAction)
		magnificationFactor = 2;
	else if (action == threeTimesMagnificationAction)
		magnificationFactor = 3;
	else if (action == fourTimesMagnificationAction)
		magnificationFactor = 4;
	else if (action == fiveTimesMagnificationAction)
		magnificationFactor = 5;

	currentMagnificationAction = action;
	if (tbMagnification)
		tbMagnification->setDefaultAction(action);
}

void WorksheetView::mouseModeChanged(QAction* action) {
	if (action == selectionModeAction) {
		m_mouseMode = MouseMode::Selection;
		setInteractive(true);
		setDragMode(QGraphicsView::NoDrag);
	} else if (action == navigationModeAction) {
		m_mouseMode = MouseMode::Navigation;
		setInteractive(false);
		setDragMode(QGraphicsView::ScrollHandDrag);
	} else {
		m_mouseMode = MouseMode::ZoomSelection;
		setInteractive(false);
		setDragMode(QGraphicsView::NoDrag);
	}
}

//"Add new" related slots
void WorksheetView::addNew(QAction* action) {
	WorksheetElement* aspect = nullptr;
	if (action == addCartesianPlot1Action) {
		CartesianPlot* plot = new CartesianPlot(i18n("xy-plot"));
		plot->setType(CartesianPlot::Type::FourAxes);
		plot->setMouseMode(m_cartesianPlotMouseMode);
		aspect = plot;
		if (tbNewCartesianPlot)
			tbNewCartesianPlot->setDefaultAction(addCartesianPlot1Action);
	} else if (action == addCartesianPlot2Action) {
		CartesianPlot* plot = new CartesianPlot(i18n("xy-plot"));
		plot->setType(CartesianPlot::Type::TwoAxes);
		plot->setMouseMode(m_cartesianPlotMouseMode);
		aspect = plot;
		if (tbNewCartesianPlot)
			tbNewCartesianPlot->setDefaultAction(addCartesianPlot2Action);
	} else if (action == addCartesianPlot3Action) {
		CartesianPlot* plot = new CartesianPlot(i18n("xy-plot"));
		plot->setType(CartesianPlot::Type::TwoAxesCentered);
		plot->setMouseMode(m_cartesianPlotMouseMode);
		aspect = plot;
		if (tbNewCartesianPlot)
			tbNewCartesianPlot->setDefaultAction(addCartesianPlot3Action);
	} else if (action == addCartesianPlot4Action) {
		CartesianPlot* plot = new CartesianPlot(i18n("xy-plot"));
		plot->setType(CartesianPlot::Type::TwoAxesCenteredZero);
		plot->setMouseMode(m_cartesianPlotMouseMode);
		aspect = plot;
		if (tbNewCartesianPlot)
			tbNewCartesianPlot->setDefaultAction(addCartesianPlot4Action);
	} else if (action == addTextLabelAction) {
		TextLabel* l = new TextLabel(i18n("Text Label"));
		l->setText(i18n("Text Label"));

		//position the label at the point where the context menu was called
		auto position = l->position();
		position.point = l->parentPosToRelativePos(m_cursorPos,
												   m_worksheet->pageRect(),
												   l->graphicsItem()->boundingRect(),
												   position,
												   l->horizontalAlignment(),
												   l->verticalAlignment()
		);
		l->setPosition(position);
		aspect = l;
	} else if (action == addImageAction) {
		Image* image = new Image(i18n("Image"));

		//position the image at the point where the context menu was called
		auto position = image->position();
		position.point = image->parentPosToRelativePos(m_cursorPos,
												   m_worksheet->pageRect(),
												   image->graphicsItem()->boundingRect(),
												   position,
												   image->horizontalAlignment(),
												   image->verticalAlignment()
		);
		image->setPosition(position);

		aspect = image;
	}
	if (!aspect)
		return;

	m_worksheet->addChild(aspect);

	//labels and images with their initial positions need to be retransformed
	//after they have gotten a parent
	if (aspect->type() == AspectType::TextLabel || aspect->type() == AspectType::Image)
		aspect->retransform();

	handleCartesianPlotActions();

	if (!m_fadeInTimeLine) {
		m_fadeInTimeLine = new QTimeLine(1000, this);
		m_fadeInTimeLine->setFrameRange(0, 100);
		connect(m_fadeInTimeLine, &QTimeLine::valueChanged, this, &WorksheetView::fadeIn);
	}

	//if there is already an element fading in, stop the time line and show the element with the full opacity.
	if (m_fadeInTimeLine->state() == QTimeLine::Running) {
		m_fadeInTimeLine->stop();
		auto* effect = new QGraphicsOpacityEffect();
		effect->setOpacity(1);
		lastAddedWorksheetElement->graphicsItem()->setGraphicsEffect(effect);
	}

	//fade-in the newly added element
	lastAddedWorksheetElement = aspect;
	auto* effect = new QGraphicsOpacityEffect();
	effect->setOpacity(0);
	lastAddedWorksheetElement->graphicsItem()->setGraphicsEffect(effect);
	m_fadeInTimeLine->start();
}

/*!
 * select all top-level items
 */
void WorksheetView::selectAllElements() {
	//deselect all previously selected items since there can be some non top-level items belong them
	m_suppressSelectionChangedEvent = true;
	for (auto* item : m_selectedItems)
		m_worksheet->setItemSelectedInView(item, false);

	//select top-level items
	for (auto* item : scene()->items()) {
		if (!item->parentItem())
			item->setSelected(true);
	}
	m_suppressSelectionChangedEvent = false;
	this->selectionChanged();
}

/*!
 * deletes selected worksheet elements
 */
void WorksheetView::deleteElement() {
	if (m_selectedItems.isEmpty())
		return;

	int rc = KMessageBox::warningYesNo( this, i18np("Do you really want to delete the selected object?", "Do you really want to delete the selected %1 objects?", m_selectedItems.size()),
	                                    i18np("Delete selected object", "Delete selected objects", m_selectedItems.size()));

	if (rc == KMessageBox::No)
		return;

	m_suppressSelectionChangedEvent = true;
	m_worksheet->beginMacro(i18n("%1: Remove selected worksheet elements.", m_worksheet->name()));
	for (auto* item : m_selectedItems)
		m_worksheet->deleteAspectFromGraphicsItem(item);
	m_selectedElement = nullptr;
	m_worksheet->endMacro();
	m_suppressSelectionChangedEvent = false;
}

void WorksheetView::aspectAboutToBeRemoved(const AbstractAspect* aspect) {
	lastAddedWorksheetElement = dynamic_cast<WorksheetElement*>(const_cast<AbstractAspect*>(aspect));
	if (!lastAddedWorksheetElement)
		return;

	//FIXME: fading-out doesn't work
	//also, the following code collides with undo/redo of the deletion
	//of a worksheet element (after redoing the element is not shown with the full opacity
	/*
		if (!m_fadeOutTimeLine) {
			m_fadeOutTimeLine = new QTimeLine(1000, this);
			m_fadeOutTimeLine->setFrameRange(0, 100);
			connect(m_fadeOutTimeLine, SIGNAL(valueChanged(qreal)), this, SLOT(fadeOut(qreal)));
		}

		//if there is already an element fading out, stop the time line
		if (m_fadeOutTimeLine->state() == QTimeLine::Running)
			m_fadeOutTimeLine->stop();

		m_fadeOutTimeLine->start();
	*/
}

void WorksheetView::fadeIn(qreal value) {
	auto* effect = new QGraphicsOpacityEffect();
	effect->setOpacity(value);
	lastAddedWorksheetElement->graphicsItem()->setGraphicsEffect(effect);
}

void WorksheetView::fadeOut(qreal value) {
	auto* effect = new QGraphicsOpacityEffect();
	effect->setOpacity(1 - value);
	lastAddedWorksheetElement->graphicsItem()->setGraphicsEffect(effect);
}

/*!
 * called when one of the layout-actions in WorkseetView was triggered.
 * sets the layout in Worksheet and enables/disables the layout actions.
 */
void WorksheetView::changeLayout(QAction* action) {
	if (action == breakLayoutAction) {
		verticalLayoutAction->setEnabled(true);
		verticalLayoutAction->setChecked(false);

		horizontalLayoutAction->setEnabled(true);
		horizontalLayoutAction->setChecked(false);

		gridLayoutAction->setEnabled(true);
		gridLayoutAction->setChecked(false);

		breakLayoutAction->setEnabled(false);

		m_worksheet->setLayout(Worksheet::Layout::NoLayout);
	} else {
		verticalLayoutAction->setEnabled(false);
		horizontalLayoutAction->setEnabled(false);
		gridLayoutAction->setEnabled(false);
		breakLayoutAction->setEnabled(true);

		if (action == verticalLayoutAction) {
			verticalLayoutAction->setChecked(true);
			m_worksheet->setLayout(Worksheet::Layout::VerticalLayout);
		} else if (action == horizontalLayoutAction) {
			horizontalLayoutAction->setChecked(true);
			m_worksheet->setLayout(Worksheet::Layout::HorizontalLayout);
		} else {
			gridLayoutAction->setChecked(true);
			m_worksheet->setLayout(Worksheet::Layout::GridLayout);
		}
	}
}

void WorksheetView::changeGrid(QAction* action) {
	if (action == noGridAction) {
		m_gridSettings.style = GridStyle::NoGrid;
		snapToGridAction->setEnabled(false);
	} else if (action == sparseLineGridAction) {
		m_gridSettings.style = GridStyle::Line;
		m_gridSettings.color = Qt::gray;
		m_gridSettings.opacity = 0.7;
		m_gridSettings.horizontalSpacing = 15;
		m_gridSettings.verticalSpacing = 15;
	} else if (action == denseLineGridAction) {
		m_gridSettings.style = GridStyle::Line;
		m_gridSettings.color = Qt::gray;
		m_gridSettings.opacity = 0.7;
		m_gridSettings.horizontalSpacing = 5;
		m_gridSettings.verticalSpacing = 5;
	} else if (action == denseDotGridAction) {
		m_gridSettings.style = GridStyle::Dot;
		m_gridSettings.color = Qt::black;
		m_gridSettings.opacity = 0.7;
		m_gridSettings.horizontalSpacing = 5;
		m_gridSettings.verticalSpacing = 5;
	} else if (action == sparseDotGridAction) {
		m_gridSettings.style = GridStyle::Dot;
		m_gridSettings.color = Qt::black;
		m_gridSettings.opacity = 0.7;
		m_gridSettings.horizontalSpacing = 15;
		m_gridSettings.verticalSpacing = 15;
	} else if (action == customGridAction) {
		auto* dlg = new GridDialog(this);
		if (dlg->exec() == QDialog::Accepted)
			dlg->save(m_gridSettings);
		else
			return;
	}

	if (m_gridSettings.style == GridStyle::NoGrid)
		snapToGridAction->setEnabled(false);
	else
		snapToGridAction->setEnabled(true);

	invalidateScene(sceneRect(), QGraphicsScene::BackgroundLayer);
}

//TODO
void WorksheetView::changeSnapToGrid() {

}

/*!
 *  Selects the QGraphicsItem \c item in \c WorksheetView.
 * 	The selection in \c ProjectExplorer is forwarded to  \c Worksheet
 *  and is finally handled here.
 */
void WorksheetView::selectItem(QGraphicsItem* item) {
	m_suppressSelectionChangedEvent = true;
	item->setSelected(true);
	m_selectedItems<<item;
	handleCartesianPlotActions();
	m_suppressSelectionChangedEvent = false;
}

/*!
 *  Deselects the \c QGraphicsItem \c item in \c WorksheetView.
 * 	The deselection in \c ProjectExplorer is forwarded to \c Worksheet
 *  and is finally handled here.
 */
void WorksheetView::deselectItem(QGraphicsItem* item) {
	m_suppressSelectionChangedEvent = true;
	item->setSelected(false);
	m_selectedItems.removeOne(item);
	handleCartesianPlotActions();
	m_suppressSelectionChangedEvent = false;
}

/*!
 *  Called on selection changes in the view.
 *   Determines which items were selected and deselected
 *  and forwards these changes to \c Worksheet
 */
void WorksheetView::selectionChanged() {
	//if the project is being closed, the scene items are being removed and the selection can change.
	//don't react on these changes since this can lead crashes (worksheet object is already in the destructor).
	if (m_isClosing)
		return;

	if (m_suppressSelectionChangedEvent)
		return;

	QList<QGraphicsItem*> items = scene()->selectedItems();

	//check, whether the previously selected items were deselected now.
	//Forward the deselection prior to the selection of new items
	//in order to avoid the unwanted multiple selection in project explorer
	for (auto* item : m_selectedItems ) {
		if ( items.indexOf(item) == -1 )
			m_worksheet->setItemSelectedInView(item, false);
	}

	//select new items
	if (items.isEmpty()) {
		//no items selected -> select the worksheet again.
		m_worksheet->setSelectedInView(true);

		//if one of the "zoom&select" plot mouse modes was selected before, activate the default "selection mode" again
		//since no plots are selected now.
		if (m_mouseMode == MouseMode::Selection && m_cartesianPlotMouseMode != CartesianPlot::MouseMode::Selection) {
			cartesianPlotSelectionModeAction->setChecked(true);
			cartesianPlotMouseModeChanged(cartesianPlotSelectionModeAction);
		}
	} else {
		for (const auto* item : items)
			m_worksheet->setItemSelectedInView(item, true);

		//items selected -> deselect the worksheet in the project explorer
		//prevents unwanted multiple selection with worksheet (if it was selected before)
		m_worksheet->setSelectedInView(false);
	}

	m_selectedItems = items;
	handleCartesianPlotActions();
}

void WorksheetView::handleCartesianPlotSelected(const CartesianPlot* plot)
{
	/* Action to All: action is applied to all ranges
	 *	- Applied to all plots and all ranges
	 * Action to X: action is applied to all x ranges
	 *	- x zoom selection: zooming into all x ranges of all plots (Normaly all plots will have the same x ranges so it makes sense
	 *  - y zoom selection: makes no sense. disable
	 * Action to Y: action is applied to all y ranges
	 *  - x zoom selection: makes no sense. disable
	 *  - y zoom selection: zooming into all y ranges of all plots
	 * Action to Selection
	 * - x zoom selection: makes no sense, because the range is unknown, disable
	 * - y zoom selection: makes no sense, because the range is unknown, disable
	 *		- What happens when only one range is available?
	*/

	switch(m_worksheet->cartesianPlotActionMode()) {
	case Worksheet::CartesianPlotActionMode::ApplyActionToAll: // Is there a usecase for this?
		cartesianPlotZoomSelectionModeAction->setEnabled(true);
		cartesianPlotZoomXSelectionModeAction->setEnabled(true);
		cartesianPlotZoomYSelectionModeAction->setEnabled(true);
		zoomInAction->setEnabled(true);
		zoomOutAction->setEnabled(true);
		zoomInXAction->setEnabled(true);
		zoomOutXAction->setEnabled(true);
		zoomInYAction->setEnabled(true);
		zoomOutYAction->setEnabled(true);
		shiftLeftXAction->setEnabled(true);
		shiftRightXAction->setEnabled(true);
		shiftUpYAction->setEnabled(true);
		shiftDownYAction->setEnabled(true);
		scaleAutoAction->setEnabled(true);
		scaleAutoXAction->setEnabled(true);
		scaleAutoYAction->setEnabled(true);
		break;
	case Worksheet::CartesianPlotActionMode::ApplyActionToSelection: {

		bool enableX = plot->xRangeCount() == 1;
		bool enableY = plot->yRangeCount() == 1;
		// only when only one range available
		cartesianPlotZoomSelectionModeAction->setEnabled(enableX && enableY);
		cartesianPlotZoomXSelectionModeAction->setEnabled(enableX);
		cartesianPlotZoomYSelectionModeAction->setEnabled(enableY);
		zoomInAction->setEnabled(true);
		zoomOutAction->setEnabled(true);
		zoomInXAction->setEnabled(true);
		zoomOutXAction->setEnabled(true);
		zoomInYAction->setEnabled(true);
		zoomOutYAction->setEnabled(true);
		shiftLeftXAction->setEnabled(true);
		shiftRightXAction->setEnabled(true);
		shiftUpYAction->setEnabled(true);
		shiftDownYAction->setEnabled(true);
		scaleAutoAction->setEnabled(true);
		scaleAutoXAction->setEnabled(true);
		scaleAutoYAction->setEnabled(true);
		break;
	} case Worksheet::CartesianPlotActionMode::ApplyActionToAllX:
		cartesianPlotZoomSelectionModeAction->setEnabled(false);
		cartesianPlotZoomXSelectionModeAction->setEnabled(true);
		cartesianPlotZoomYSelectionModeAction->setEnabled(true);
		zoomInAction->setEnabled(false);
		zoomOutAction->setEnabled(false);
		zoomInXAction->setEnabled(true);
		zoomOutXAction->setEnabled(true);
		zoomInYAction->setEnabled(false);
		zoomOutYAction->setEnabled(false);
		shiftLeftXAction->setEnabled(true);
		shiftRightXAction->setEnabled(true);
		shiftUpYAction->setEnabled(false);
		shiftDownYAction->setEnabled(false);
		scaleAutoAction->setEnabled(false);
		scaleAutoXAction->setEnabled(true);
		scaleAutoYAction->setEnabled(true);
		break;
	case Worksheet::CartesianPlotActionMode::ApplyActionToAllY:
		cartesianPlotZoomSelectionModeAction->setEnabled(false);
		cartesianPlotZoomXSelectionModeAction->setEnabled(true);
		cartesianPlotZoomYSelectionModeAction->setEnabled(true);
		zoomInAction->setEnabled(false);
		zoomOutAction->setEnabled(false);
		zoomInXAction->setEnabled(false);
		zoomOutXAction->setEnabled(false);
		zoomInYAction->setEnabled(true);
		zoomOutYAction->setEnabled(true);
		shiftLeftXAction->setEnabled(false);
		shiftRightXAction->setEnabled(false);
		shiftUpYAction->setEnabled(true);
		shiftDownYAction->setEnabled(true);
		scaleAutoAction->setEnabled(false);
		scaleAutoXAction->setEnabled(false);
		scaleAutoYAction->setEnabled(true);
		break;
	default:
		break;
	}

	cartesianPlotSelectionModeAction->setEnabled(true);
	cartesianPlotCursorModeAction->setEnabled(true);
}

void WorksheetView::handleXYCurveSelected()
{
	/* Action to All: action is applied to all ranges
	 *	- Disable
	 * Action to X: action is applied to all x ranges
	 *	- x zoom selection: Zooming into all ranges of all plots (mostly the x ranges are the same for all plots --> usecase)
	 *  - y zoom selection: zoom only into the range from the curve
	 * Action to Y: action is applied to all y ranges
	 *  - x zoom selection: zoom only into the range from the curve
	 *  - y zoom selection: Zooming into all ranges of all plots
	 * Action to Selection
	 * - x zoom selection: zoom only into the range from the curve
	 * - y zoom selection: zoom only into the range from the curve
	*/

	switch(m_worksheet->cartesianPlotActionMode()) {
	case Worksheet::CartesianPlotActionMode::ApplyActionToAll:
		cartesianPlotZoomSelectionModeAction->setEnabled(false);
		cartesianPlotZoomXSelectionModeAction->setEnabled(false);
		cartesianPlotZoomYSelectionModeAction->setEnabled(false);
		zoomInAction->setEnabled(false);
		zoomOutAction->setEnabled(false);
		zoomInXAction->setEnabled(false);
		zoomOutXAction->setEnabled(false);
		zoomInYAction->setEnabled(false);
		zoomOutYAction->setEnabled(false);
		shiftLeftXAction->setEnabled(false);
		shiftRightXAction->setEnabled(false);
		shiftUpYAction->setEnabled(false);
		shiftDownYAction->setEnabled(false);
		scaleAutoAction->setEnabled(true);
		scaleAutoXAction->setEnabled(true);
		scaleAutoYAction->setEnabled(true);
		break;
	case Worksheet::CartesianPlotActionMode::ApplyActionToSelection:
		cartesianPlotZoomSelectionModeAction->setEnabled(true);
		cartesianPlotZoomXSelectionModeAction->setEnabled(true);
		cartesianPlotZoomYSelectionModeAction->setEnabled(true);
		zoomInAction->setEnabled(true);
		zoomOutAction->setEnabled(true);
		zoomInXAction->setEnabled(true);
		zoomOutXAction->setEnabled(true);
		zoomInYAction->setEnabled(true);
		zoomOutYAction->setEnabled(true);
		shiftLeftXAction->setEnabled(true);
		shiftRightXAction->setEnabled(true);
		shiftUpYAction->setEnabled(true);
		shiftDownYAction->setEnabled(true);
		scaleAutoAction->setEnabled(true);
		scaleAutoXAction->setEnabled(true);
		scaleAutoYAction->setEnabled(true);
		break;
	case Worksheet::CartesianPlotActionMode::ApplyActionToAllX:
		cartesianPlotZoomSelectionModeAction->setEnabled(false);
		cartesianPlotZoomXSelectionModeAction->setEnabled(true);
		cartesianPlotZoomYSelectionModeAction->setEnabled(true);
		zoomInAction->setEnabled(false);
		zoomOutAction->setEnabled(false);
		zoomInXAction->setEnabled(true);
		zoomOutXAction->setEnabled(true);
		zoomInYAction->setEnabled(true);
		zoomOutYAction->setEnabled(true);
		shiftLeftXAction->setEnabled(true);
		shiftRightXAction->setEnabled(true);
		shiftUpYAction->setEnabled(true);
		shiftDownYAction->setEnabled(true);
		scaleAutoAction->setEnabled(true);
		scaleAutoXAction->setEnabled(true);
		scaleAutoYAction->setEnabled(true);
		break;
	case Worksheet::CartesianPlotActionMode::ApplyActionToAllY:
		cartesianPlotZoomSelectionModeAction->setEnabled(false);
		cartesianPlotZoomXSelectionModeAction->setEnabled(true);
		cartesianPlotZoomYSelectionModeAction->setEnabled(true);
		zoomInAction->setEnabled(false);
		zoomOutAction->setEnabled(false);
		zoomInXAction->setEnabled(true);
		zoomOutXAction->setEnabled(true);
		zoomInYAction->setEnabled(true);
		zoomOutYAction->setEnabled(true);
		shiftLeftXAction->setEnabled(true);
		shiftRightXAction->setEnabled(true);
		shiftUpYAction->setEnabled(true);
		shiftDownYAction->setEnabled(true);
		scaleAutoAction->setEnabled(true);
		scaleAutoXAction->setEnabled(true);
		scaleAutoYAction->setEnabled(true);
		break;
	default:
		break;
	}
	cartesianPlotSelectionModeAction->setEnabled(true);
	cartesianPlotCursorModeAction->setEnabled(false);

}

void WorksheetView::handleAxisSelected(const Axis* a)
{
	if (a->orientation() == Axis::Orientation::Horizontal) {
		/* HORIZONTAL:
		 * Action to All: action is applied to all ranges
		 *	- Disable
		 * Action to X: action is applied to all x ranges
		 *	- x zoom selection: Zooming
		 *  - y zoom selection: makes no sense. disable
		 * Action to Y: action is applied to all y ranges
		 *  - x zoom selection: zooming into the range of the axis
		 *  - y zoom selection: makes no sense. disable
		 * Action to Selection
		 * - x zoom selection: apply to range assigned to the axis, but only for the plot where the axis is child
		 * - y zoom selection: makes no sense. disable
		*/
		cartesianPlotZoomYSelectionModeAction->setEnabled(false);
		cartesianPlotZoomSelectionModeAction->setEnabled(false);
		cartesianPlotZoomXSelectionModeAction->setEnabled(true);
		zoomInAction->setEnabled(false);
		zoomOutAction->setEnabled(false);
		zoomInXAction->setEnabled(true);
		zoomOutXAction->setEnabled(true);
		zoomInYAction->setEnabled(false);
		zoomOutYAction->setEnabled(false);
		shiftLeftXAction->setEnabled(true);
		shiftRightXAction->setEnabled(true);
		shiftUpYAction->setEnabled(false);
		shiftDownYAction->setEnabled(false);
		scaleAutoAction->setEnabled(false);
		scaleAutoXAction->setEnabled(true);
		scaleAutoYAction->setEnabled(false);
	} else {
		/* VERTICAL:
		 * Action to All: action is applied to all ranges
		 *	- Disable
		 * Action to Y: action is applied to all y ranges
		 *	- y zoom selection: Zooming
		 *  - x zoom selection: makes no sense. disable
		 * Action to X: action is applied to all x ranges
		 *  - x zoom selection: makes no sense. disable
		 *  - y zoom selection: makes no sense. disable
		 * Action to Selection
		 * - x zoom selection: apply to range assigned to the axis, but only for the plot where the axis is child
		 * - y zoom selection: makes no sense. disable
		*/
		cartesianPlotZoomYSelectionModeAction->setEnabled(true);
		cartesianPlotZoomSelectionModeAction->setEnabled(false);
		cartesianPlotZoomXSelectionModeAction->setEnabled(false);
		zoomInAction->setEnabled(false);
		zoomOutAction->setEnabled(false);
		zoomInXAction->setEnabled(false);
		zoomOutXAction->setEnabled(false);
		zoomInYAction->setEnabled(true);
		zoomOutYAction->setEnabled(true);
		shiftLeftXAction->setEnabled(false);
		shiftRightXAction->setEnabled(false);
		shiftUpYAction->setEnabled(true);
		shiftDownYAction->setEnabled(true);
		scaleAutoAction->setEnabled(false);
		scaleAutoXAction->setEnabled(false);
		scaleAutoYAction->setEnabled(true);
	}

	cartesianPlotSelectionModeAction->setEnabled(true);
	cartesianPlotCursorModeAction->setEnabled(false);
}

//check whether we have cartesian plots selected and activate/deactivate
void WorksheetView::handleCartesianPlotActions() {
	if (!m_menusInitialized)
		return;

	if (m_mouseMode != MouseMode::Selection || m_cartesianPlotMouseMode != CartesianPlot::MouseMode::Selection)
		return; // Do not change selection when the mousemode is not selection!

	m_selectedElement = nullptr;

	bool handled = false, plot = false;
	for (auto* item : m_selectedItems) {
		//TODO: or if a children of a plot is selected
		int key = item->data(0).toInt();
		if (key == static_cast<int>(WorksheetElement::WorksheetElementName::NameCartesianPlot)) {
			handled = true;
			plot = true;
			m_selectedElement = static_cast<WorksheetElementContainerPrivate*>(item)->q;
			handleCartesianPlotSelected(static_cast<CartesianPlot*>(m_selectedElement));
			break;
		} else if (key == static_cast<int>(WorksheetElement::WorksheetElementName::XYCurve)) {
			handled = true;
			m_selectedElement = static_cast<XYCurvePrivate*>(item)->q;
			// XYCurvePrivate does not depend yet on WorksheetElementContainerPrivate
			//m_selectedElement = static_cast<WorksheetElementContainerPrivate*>(item)->q;
			handleXYCurveSelected();
			break;
		} else if (key == static_cast<int>(WorksheetElement::WorksheetElementName::Axis)) {
			handled = true;
			m_selectedElement = static_cast<AxisPrivate*>(item)->q;
			// AxisPrivate does not depend yet on WorksheetElementContainerPrivate
			//m_selectedElement = static_cast<WorksheetElementContainerPrivate*>(item)->q;
			handleAxisSelected(static_cast<Axis*>(m_selectedElement));
			break;
		}
	}

	if (!handled) {
		cartesianPlotZoomYSelectionModeAction->setEnabled(false);
		cartesianPlotZoomSelectionModeAction->setEnabled(false);
		cartesianPlotZoomXSelectionModeAction->setEnabled(false);
		zoomInAction->setEnabled(false);
		zoomOutAction->setEnabled(false);
		zoomInXAction->setEnabled(false);
		zoomOutXAction->setEnabled(false);
		zoomInYAction->setEnabled(false);
		zoomOutYAction->setEnabled(false);
		shiftLeftXAction->setEnabled(false);
		shiftRightXAction->setEnabled(false);
		shiftUpYAction->setEnabled(false);
		shiftDownYAction->setEnabled(false);
		scaleAutoAction->setEnabled(false);
		scaleAutoXAction->setEnabled(false);
		scaleAutoYAction->setEnabled(false);
	}

	m_cartesianPlotAddNewMenu->setEnabled(plot);
	m_cartesianPlotZoomMenu->setEnabled(plot);
	m_cartesianPlotMouseModeMenu->setEnabled(plot);

	// analysis menu
	//TODO: enable also if children of plots are selected
// 	m_dataManipulationMenu->setEnabled(plot);
// 	addDataOperationAction->setEnabled(false);
	addDataReductionAction->setEnabled(false);
	addDifferentiationAction->setEnabled(plot);
	addIntegrationAction->setEnabled(plot);
	addInterpolationAction->setEnabled(plot);
	addSmoothAction->setEnabled(plot);
	addFitAction->setEnabled(plot);
	addFourierFilterAction->setEnabled(plot);
	addFourierTransformAction->setEnabled(plot);
	addHilbertTransformAction->setEnabled(plot);
	addConvolutionAction->setEnabled(plot);
	addCorrelationAction->setEnabled(plot);
}

void WorksheetView::exportToFile(const QString& path, const ExportFormat format, const ExportArea area, const bool background, const int resolution) {
	QRectF sourceRect;

	//determine the rectangular to print
	if (area == ExportArea::BoundingBox)
		sourceRect = scene()->itemsBoundingRect();
	else if (area == ExportArea::Selection) {
		if (!m_selectedItems.isEmpty()) {
			//TODO doesn't work: rect = scene()->selectionArea().boundingRect();
			for (const auto* item : m_selectedItems)
				sourceRect = sourceRect.united( item->mapToScene(item->boundingRect()).boundingRect() );
		} else
			sourceRect = scene()->sceneRect(); //export everything if nothing is selected
	} else
		sourceRect = scene()->sceneRect();

	//save
	switch (format) {
	case ExportFormat::PDF: {
		QPrinter printer(QPrinter::HighResolution);
		printer.setOutputFormat(QPrinter::PdfFormat);

		printer.setOutputFileName(path);
		int w = Worksheet::convertFromSceneUnits(sourceRect.width(), Worksheet::Unit::Millimeter);
		int h = Worksheet::convertFromSceneUnits(sourceRect.height(), Worksheet::Unit::Millimeter);
		printer.setPaperSize( QSizeF(w, h), QPrinter::Millimeter);
		printer.setPageMargins(0,0,0,0, QPrinter::Millimeter);
		printer.setPrintRange(QPrinter::PageRange);
		printer.setCreator(QLatin1String("LabPlot ") + LVERSION);

		QPainter painter(&printer);
		painter.setRenderHint(QPainter::Antialiasing);
		QRectF targetRect(0, 0, painter.device()->width(),painter.device()->height());
		painter.begin(&printer);
		exportPaint(&painter, targetRect, sourceRect, background);
		painter.end();
		break;
		}
	case ExportFormat::SVG: {
		QSvgGenerator generator;
		generator.setFileName(path);
// 		if (!generator.isValid()) {
// 			RESET_CURSOR;
// 			QMessageBox::critical(nullptr, i18n("Failed to export"), i18n("Failed to write to '%1'. Please check the path.", path));
// 		}
		int w = Worksheet::convertFromSceneUnits(sourceRect.width(), Worksheet::Unit::Millimeter);
		int h = Worksheet::convertFromSceneUnits(sourceRect.height(), Worksheet::Unit::Millimeter);
		w = w*QApplication::desktop()->physicalDpiX()/25.4;
		h = h*QApplication::desktop()->physicalDpiY()/25.4;

		generator.setSize(QSize(w, h));
		QRectF targetRect(0, 0, w, h);
		generator.setViewBox(targetRect);

		QPainter painter;
		painter.begin(&generator);
		exportPaint(&painter, targetRect, sourceRect, background);
		painter.end();
		break;
		}
	case  ExportFormat::PNG:
	case  ExportFormat::JPG:
	case  ExportFormat::BMP:
	case  ExportFormat::PPM:
	case  ExportFormat::XBM:
	case  ExportFormat::XPM: {
		int w = Worksheet::convertFromSceneUnits(sourceRect.width(), Worksheet::Unit::Millimeter);
		int h = Worksheet::convertFromSceneUnits(sourceRect.height(), Worksheet::Unit::Millimeter);
		w = w*resolution/25.4;
		h = h*resolution/25.4;
		QImage image(QSize(w, h), QImage::Format_ARGB32_Premultiplied);
		image.fill(Qt::transparent);
		QRectF targetRect(0, 0, w, h);

		QPainter painter;
		painter.begin(&image);
		painter.setRenderHint(QPainter::Antialiasing);
		exportPaint(&painter, targetRect, sourceRect, background);
		painter.end();

		if (!path.isEmpty()) {
			bool rc{false};
			switch (format) {
			case  ExportFormat::PNG:
				rc = image.save(path, "PNG");
				break;
			case  ExportFormat::JPG:
				rc = image.save(path, "JPG");
				break;
			case  ExportFormat::BMP:
				rc = image.save(path, "BMP");
				break;
			case  ExportFormat::PPM:
				rc = image.save(path, "PPM");
				break;
			case  ExportFormat::XBM:
				rc = image.save(path, "XBM");
				break;
			case  ExportFormat::XPM:
				rc = image.save(path, "XPM");
				break;
			case ExportFormat::PDF:
			case ExportFormat::SVG:
				break;
			}
			if (!rc) {
				RESET_CURSOR;
				QMessageBox::critical(nullptr, i18n("Failed to export"), i18n("Failed to write to '%1'. Please check the path.", path));
			}
		} else
			QApplication::clipboard()->setImage(image, QClipboard::Clipboard);
		}
	}
}

bool WorksheetView::eventFilter(QObject *watched, QEvent *event) {
	Q_UNUSED(watched)
	if (event->type() == QEvent::KeyPress) {
		QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
		int key = keyEvent->key();
		switch(key) {
		case Qt::Key_S:
			if (cartesianPlotSelectionModeAction->isEnabled())
				cartesianPlotSelectionModeAction->trigger();
			return true;
		case Qt::Key_X:
			if (cartesianPlotZoomXSelectionModeAction->isEnabled())
				cartesianPlotZoomXSelectionModeAction->trigger();
			return true;
		case Qt::Key_Y:
			if (cartesianPlotZoomYSelectionModeAction->isEnabled())
				cartesianPlotZoomYSelectionModeAction->trigger();
			return true;
		case Qt::Key_Z:
			if (cartesianPlotZoomSelectionModeAction->isEnabled())
			cartesianPlotZoomSelectionModeAction->trigger();
			return true;
		case Qt::Key_C:
			if (cartesianPlotCursorModeAction->isEnabled())
				cartesianPlotCursorModeAction->trigger();
		case Qt::Key_Escape:
			if (cartesianPlotSelectionModeAction->isEnabled())
				cartesianPlotSelectionModeAction->trigger();
			return false; // so the plot can handle the event too
		default:
			return false;
		}
	}
	return false;
}

void WorksheetView::exportToClipboard() {
	QRectF sourceRect;

	if (m_selectedItems.size() == 0)
		sourceRect = scene()->itemsBoundingRect();
	else {
		//export selection
		for (const auto* item : m_selectedItems)
			sourceRect = sourceRect.united( item->mapToScene(item->boundingRect()).boundingRect() );
	}

	int w = Worksheet::convertFromSceneUnits(sourceRect.width(), Worksheet::Unit::Millimeter);
	int h = Worksheet::convertFromSceneUnits(sourceRect.height(), Worksheet::Unit::Millimeter);
	w = w*QApplication::desktop()->physicalDpiX()/25.4;
	h = h*QApplication::desktop()->physicalDpiY()/25.4;
	QImage image(QSize(w, h), QImage::Format_ARGB32_Premultiplied);
	image.fill(Qt::transparent);
	QRectF targetRect(0, 0, w, h);

	QPainter painter;
	painter.begin(&image);
	painter.setRenderHint(QPainter::Antialiasing);
	exportPaint(&painter, targetRect, sourceRect, true);
	painter.end();

	QApplication::clipboard()->setImage(image, QClipboard::Clipboard);
}

void WorksheetView::exportPaint(QPainter* painter, const QRectF& targetRect, const QRectF& sourceRect, const bool background) {
	//draw the background
	m_isPrinting = true;
	if (background) {
		painter->save();
		painter->scale(targetRect.width()/sourceRect.width(), targetRect.height()/sourceRect.height());
		drawBackground(painter, sourceRect);
		painter->restore();
	}

	//draw the scene items
	m_worksheet->setPrinting(true);
	scene()->render(painter, QRectF(), sourceRect);
	m_worksheet->setPrinting(false);
	m_isPrinting = false;
}

void WorksheetView::print(QPrinter* printer) {
	m_isPrinting = true;
	m_worksheet->setPrinting(true);
	QPainter painter(printer);
	painter.setRenderHint(QPainter::Antialiasing);

	// draw background
	QRectF page_rect = printer->pageRect();
	QRectF scene_rect = scene()->sceneRect();
	float scale = qMax(scene_rect.width()/page_rect.width(),scene_rect.height()/page_rect.height());
	drawBackgroundItems(&painter, QRectF(0,0,scene_rect.width()/scale,scene_rect.height()/scale));

	// draw scene
	scene()->render(&painter);
	m_worksheet->setPrinting(false);
	m_isPrinting = false;
}

void WorksheetView::updateBackground() {
	invalidateScene(sceneRect(), QGraphicsScene::BackgroundLayer);
}

/*!
 * called when the layout was changed in Worksheet,
 * enables the corresponding action
 */
void WorksheetView::layoutChanged(Worksheet::Layout layout) {
	if (layout == Worksheet::Layout::NoLayout) {
		verticalLayoutAction->setEnabled(true);
		verticalLayoutAction->setChecked(false);

		horizontalLayoutAction->setEnabled(true);
		horizontalLayoutAction->setChecked(false);

		gridLayoutAction->setEnabled(true);
		gridLayoutAction->setChecked(false);

		breakLayoutAction->setEnabled(false);
	} else {
		verticalLayoutAction->setEnabled(false);
		horizontalLayoutAction->setEnabled(false);
		gridLayoutAction->setEnabled(false);
		breakLayoutAction->setEnabled(true);

		if (layout == Worksheet::Layout::VerticalLayout)
			verticalLayoutAction->setChecked(true);
		else if (layout == Worksheet::Layout::HorizontalLayout)
			horizontalLayoutAction->setChecked(true);
		else
			gridLayoutAction->setChecked(true);
	}
}

void WorksheetView::suppressSelectionChangedEvent(bool value) {
	m_suppressSelectionChangedEvent = value;
}

WorksheetElement* WorksheetView::selectedElement() const {
	return m_selectedElement;
}
QList<QGraphicsItem*> WorksheetView::selectedItems() const {
	return m_selectedItems;
}

void WorksheetView::registerShortcuts() {
	selectAllAction->setShortcut(Qt::CTRL+Qt::Key_A);
	deleteAction->setShortcut(Qt::Key_Delete);
	backspaceAction->setShortcut(Qt::Key_Backspace);
	zoomInViewAction->setShortcut(Qt::CTRL+Qt::Key_Plus);
	zoomOutViewAction->setShortcut(Qt::CTRL+Qt::Key_Minus);
	zoomOriginAction->setShortcut(Qt::CTRL+Qt::Key_1);
}

void WorksheetView::unregisterShortcuts() {
	selectAllAction->setShortcut(QKeySequence());
	deleteAction->setShortcut(QKeySequence());
	backspaceAction->setShortcut(QKeySequence());
	zoomInViewAction->setShortcut(QKeySequence());
	zoomOutViewAction->setShortcut(QKeySequence());
	zoomOriginAction->setShortcut(QKeySequence());
}

//##############################################################################
//########################  SLOTs for cartesian plots   ########################
//##############################################################################
void WorksheetView::cartesianPlotActionModeChanged(QAction* action) {
	if (action == cartesianPlotApplyToSelectionAction)
		m_worksheet->setCartesianPlotActionMode(Worksheet::CartesianPlotActionMode::ApplyActionToSelection);
	else if (action == cartesianPlotApplyToAllXAction)
		m_worksheet->setCartesianPlotActionMode(Worksheet::CartesianPlotActionMode::ApplyActionToAllX);
	else if (action == cartesianPlotApplyToAllYAction)
		m_worksheet->setCartesianPlotActionMode(Worksheet::CartesianPlotActionMode::ApplyActionToAllY);
	else
		m_worksheet->setCartesianPlotActionMode(Worksheet::CartesianPlotActionMode::ApplyActionToAll);

	handleCartesianPlotActions();
}

void WorksheetView::cartesianPlotCursorModeChanged(QAction* action) {
	if (action == cartesianPlotApplyToSelectionCursor)
		m_worksheet->setCartesianPlotCursorMode(Worksheet::CartesianPlotActionMode::ApplyActionToSelection);
	else
		m_worksheet->setCartesianPlotCursorMode(Worksheet::CartesianPlotActionMode::ApplyActionToAll);

	handleCartesianPlotActions();
}

void WorksheetView::plotsLockedActionChanged(bool checked) {
	m_worksheet->setPlotsLocked(checked);
}

void WorksheetView::cartesianPlotMouseModeChanged(QAction* action) {
	if (m_suppressMouseModeChange)
		return;

	auto oldMouseMode = m_cartesianPlotMouseMode;
	m_cartesianPlotMouseMode = static_cast<CartesianPlot::MouseMode>(action->data().toInt());


	if (m_cartesianPlotMouseMode == CartesianPlot::MouseMode::Selection &&
			(oldMouseMode == CartesianPlot::MouseMode::ZoomSelection ||
			oldMouseMode == CartesianPlot::MouseMode::ZoomXSelection ||
			oldMouseMode == CartesianPlot::MouseMode::ZoomYSelection)) {
		// restore selection after zoom selection
		auto item = m_selectedElement->graphicsItem();
		for (auto i: m_selectedItems)
			i->setSelected(false);
		item->setSelected(true);
		//selectItem(m_selectedElement->graphicsItem());
		//m_worksheet->setItemSelectedInView(m_selectedElement->graphicsItem(), true);
	}
	// TODO: find out, which element is selected. So the corresponding range can be modified

	for (auto* plot : m_worksheet->children<CartesianPlot>() )
		plot->setMouseMode(m_cartesianPlotMouseMode);
}

void WorksheetView::cartesianPlotMouseModeChangedSlot(CartesianPlot::MouseMode mouseMode) {
	if (!m_menusInitialized)
		return;

	m_suppressMouseModeChange = true;
	if (mouseMode == CartesianPlot::MouseMode::Selection)
		cartesianPlotSelectionModeAction->setChecked(true);
	else if (mouseMode == CartesianPlot::MouseMode::ZoomSelection)
		cartesianPlotZoomSelectionModeAction->setChecked(true);
	else if (mouseMode == CartesianPlot::MouseMode::ZoomXSelection)
		cartesianPlotZoomXSelectionModeAction->setChecked(true);
	else if (mouseMode == CartesianPlot::MouseMode::ZoomYSelection)
		cartesianPlotZoomYSelectionModeAction->setChecked(true);
	else if (mouseMode == CartesianPlot::MouseMode::Cursor)
		cartesianPlotCursorModeAction->setChecked(true);
	m_suppressMouseModeChange = false;
}

void WorksheetView::cartesianPlotAddNew(QAction* action) {
	QVector<CartesianPlot*> plots = m_worksheet->children<CartesianPlot>();
	if (m_worksheet->cartesianPlotActionMode() == Worksheet::CartesianPlotActionMode::ApplyActionToSelection) {
		int selectedPlots = 0;
		for (auto* plot : plots) {
			if (m_selectedItems.indexOf(plot->graphicsItem()) != -1)
				++selectedPlots;
			else {
				//current plot is not selected, check if one of its children is selected
				auto children = plot->children<WorksheetElement>();
				for (auto* child : children) {
					if (m_selectedItems.indexOf(child->graphicsItem()) != -1) {
						++selectedPlots;
						break;
					}
				}
			}
		}

		if  (selectedPlots > 1)
			m_worksheet->beginMacro(i18n("%1: Add curve to %2 plots", m_worksheet->name(), selectedPlots));

		for (auto* plot : plots) {
			if (m_selectedItems.indexOf(plot->graphicsItem()) != -1)
				this->cartesianPlotAdd(plot, action);
			else {
				//current plot is not selected, check if one of its children is selected
				auto children = plot->children<WorksheetElement>();
				for (auto* child : children) {
					if (m_selectedItems.indexOf(child->graphicsItem()) != -1) {
						this->cartesianPlotAdd(plot, action);
						break;
					}
				}
			}
		}

		if (selectedPlots > 1)
			m_worksheet->endMacro();
	} else {
		if  (plots.size() > 1)
			m_worksheet->beginMacro(i18n("%1: Add curve to %2 plots", m_worksheet->name(), plots.size()));

		for (auto* plot : plots)
			this->cartesianPlotAdd(plot, action);

		if  (plots.size() > 1)
			m_worksheet->endMacro();
	}
}

void WorksheetView::cartesianPlotAdd(CartesianPlot* plot, QAction* action) {
	DEBUG("WorksheetView::cartesianPlotAdd()");
	if (action == addCurveAction)
		plot->addCurve();
	else if (action == addHistogramAction)
		plot->addHistogram();
	else if (action == addEquationCurveAction)
		plot->addEquationCurve();
	else if (action == addDataReductionCurveAction)
		plot->addDataReductionCurve();
	else if (action == addDifferentiationCurveAction)
		plot->addDifferentiationCurve();
	else if (action == addIntegrationCurveAction)
		plot->addIntegrationCurve();
	else if (action == addInterpolationCurveAction)
		plot->addInterpolationCurve();
	else if (action == addSmoothCurveAction)
		plot->addSmoothCurve();
	else if (action == addFitCurveAction)
		plot->addFitCurve();
	else if (action == addFourierFilterCurveAction)
		plot->addFourierFilterCurve();
	else if (action == addFourierTransformCurveAction)
		plot->addFourierTransformCurve();
	else if (action == addConvolutionCurveAction)
		plot->addConvolutionCurve();
	else if (action == addCorrelationCurveAction)
		plot->addCorrelationCurve();
	else if (action == addLegendAction)
		plot->addLegend();
	else if (action == addHorizontalAxisAction)
		plot->addHorizontalAxis();
	else if (action == addVerticalAxisAction)
		plot->addVerticalAxis();
	else if (action == addPlotTextLabelAction)
		plot->addTextLabel();
	else if (action == addPlotImageAction)
		plot->addImage();
	else if (action == addCustomPointAction)
		plot->addCustomPoint();
// analysis actions
	else if (action == addDataReductionAction)
		plot->addDataReductionCurve();
	else if (action == addDifferentiationAction)
		plot->addDifferentiationCurve();
	else if (action == addIntegrationAction)
		plot->addIntegrationCurve();
	else if (action == addInterpolationAction)
		plot->addInterpolationCurve();
	else if (action == addSmoothAction)
		plot->addSmoothCurve();
	else if (action == addFitAction)
		plot->addFitCurve();
	else if (action == addFourierFilterAction)
		plot->addFourierFilterCurve();
	else if (action == addFourierTransformAction)
		plot->addFourierTransformCurve();
	else if (action == addHilbertTransformAction)
		plot->addHilbertTransformCurve();
	else if (action == addConvolutionAction)
		plot->addConvolutionCurve();
	else if (action == addCorrelationAction)
		plot->addCorrelationCurve();
}

void WorksheetView::cartesianPlotNavigationChanged(QAction* action) {
	// TODO: find out, which element was selected to find out which range should be changed
	//Project().projectExplorer().currentAspect()

	CartesianPlot::NavigationOperation op = (CartesianPlot::NavigationOperation)action->data().toInt();
	auto plotActionMode = m_worksheet->cartesianPlotActionMode();
	if (plotActionMode == Worksheet::CartesianPlotActionMode::ApplyActionToSelection) {
		int cSystemIndex = Worksheet::cSystemIndex(m_selectedElement);
		for (auto* plot : m_worksheet->children<CartesianPlot>() ) {
			if (m_selectedItems.indexOf(plot->graphicsItem()) != -1)
				plot->navigate(cSystemIndex, op);
			else {
				// check if one of the plots childrend is selected. Do the operation there too.
				for (auto* child : plot->children<WorksheetElement>()) {
					if (m_selectedItems.indexOf(child->graphicsItem()) != -1) {
						plot->navigate(cSystemIndex, op);
						break;
					}
				}
			}
		}
	} else if ((plotActionMode == Worksheet::CartesianPlotActionMode::ApplyActionToAllY &&
				(op == CartesianPlot::NavigationOperation::ScaleAutoX ||
				op == CartesianPlot::NavigationOperation::ShiftLeftX||
			   op == CartesianPlot::NavigationOperation::ShiftRightX ||
				op == CartesianPlot::NavigationOperation::ZoomInX ||
				op == CartesianPlot::NavigationOperation::ZoomOutX))||
			   (plotActionMode == Worksheet::CartesianPlotActionMode::ApplyActionToAllX &&
			   (op == CartesianPlot::NavigationOperation::ScaleAutoY ||
				op == CartesianPlot::NavigationOperation::ShiftUpY||
				op == CartesianPlot::NavigationOperation::ShiftDownY ||
				op == CartesianPlot::NavigationOperation::ZoomInY ||
				op == CartesianPlot::NavigationOperation::ZoomOutY))) {
		int cSystemIndex = Worksheet::cSystemIndex(m_selectedElement);
		if (m_selectedElement->type() == AspectType::CartesianPlot)
			static_cast<CartesianPlot*>(m_selectedElement)->navigate(-1, op);
		else {
			auto parentPlot = static_cast<CartesianPlot*>(m_selectedElement->parent(AspectType::CartesianPlot));
			if (parentPlot) // really needed?
				parentPlot->navigate(cSystemIndex, op);
		}
	} else {
		for (auto* plot : m_worksheet->children<CartesianPlot>() )
			plot->navigate(-1, op);
	}
}

Worksheet::CartesianPlotActionMode WorksheetView::getCartesianPlotActionMode() {
	return m_worksheet->cartesianPlotActionMode();
}

void WorksheetView::presenterMode() {
	KConfigGroup group = KSharedConfig::openConfig()->group("Settings_Worksheet");

	//show dynamic presenter widget, if enabled
	if (group.readEntry("PresenterModeInteractive", false)) {
		auto* dynamicPresenterWidget = new DynamicPresenterWidget(m_worksheet);
		dynamicPresenterWidget->showFullScreen();
		return;
	}

	//show static presenter widget (default)
	QRectF sourceRect(scene()->sceneRect());

	int w = Worksheet::convertFromSceneUnits(sourceRect.width(), Worksheet::Unit::Millimeter);
	int h = Worksheet::convertFromSceneUnits(sourceRect.height(), Worksheet::Unit::Millimeter);
	w *= QApplication::desktop()->physicalDpiX()/25.4;
	h *= QApplication::desktop()->physicalDpiY()/25.4;

	QRectF targetRect(0, 0, w, h);
	const QRectF& screenSize = QGuiApplication::primaryScreen()->availableGeometry();

	if (targetRect.width() > screenSize.width() || ((targetRect.height() > screenSize.height()))) {
		const double ratio = qMin(screenSize.width() / targetRect.width(), screenSize.height() / targetRect.height());
		targetRect.setWidth(targetRect.width()* ratio);
		targetRect.setHeight(targetRect.height() * ratio);
	}

	QImage image(QSize(targetRect.width(), targetRect.height()), QImage::Format_ARGB32_Premultiplied);
	image.fill(Qt::transparent);
	QPainter painter;
	painter.begin(&image);
	painter.setRenderHint(QPainter::Antialiasing);
	exportPaint(&painter, targetRect, sourceRect, true);
	painter.end();

	auto* presenterWidget = new PresenterWidget(QPixmap::fromImage(image), m_worksheet->name());
	presenterWidget->showFullScreen();
}
