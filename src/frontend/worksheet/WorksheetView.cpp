/*
	File                 : WorksheetView.cpp
	Project              : LabPlot
	Description          : Worksheet view
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2009-2025 Alexander Semke <alexander.semke@web.de>
	SPDX-FileCopyrightText: 2016-2018 Stefan-Gerlach <stefan.gerlach@uni.kn>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "WorksheetView.h"
#include "backend/core/AbstractColumn.h"
#include "backend/core/Project.h"
#include "backend/core/Settings.h"
#include "backend/lib/XmlStreamReader.h"
#include "backend/lib/trace.h"
#include "backend/worksheet/Background.h"
#include "backend/worksheet/Image.h"
#include "backend/worksheet/TextLabel.h"
#include "backend/worksheet/plots/cartesian/AxisPrivate.h" // TODO: redesign, don't depend on the private class
#include "backend/worksheet/plots/cartesian/BoxPlot.h" //TODO: needed for the icon only, remove later once we have a breeze icon
#include "backend/worksheet/plots/cartesian/ReferenceLine.h"
#include "backend/worksheet/plots/cartesian/ReferenceRange.h"
#include "frontend/widgets/toggleactionmenu.h"
#ifndef SDK
#include "frontend/PlotTemplateDialog.h"
#include "frontend/core/ContentDockWidget.h"
#include "frontend/widgets/ThemesWidget.h"
#include "frontend/widgets/toggleactionmenu.h"
#include "frontend/worksheet/GridDialog.h"
#include "frontend/worksheet/PresenterWidget.h"
#endif
#include <frontend/GuiTools.h>
#ifdef Q_OS_MAC
#include "3rdparty/kdmactouchbar/src/kdmactouchbar.h"
#endif

#include <KColorScheme>
#include <KConfigGroup>
#include <KLocalizedString>
#include <KMessageBox>

#include <QActionGroup>
#include <QApplication>
#include <QClipboard>
#include <QGraphicsOpacityEffect>
#include <QGraphicsPixmapItem>
#include <QImage>
#include <QMenu>
#include <QMimeData>
#include <QPrinter>
#include <QScreen>
#include <QTimeLine>
#include <QToolBar>
#include <QToolButton>
#include <QWheelEvent>
#include <QWidgetAction>
#ifdef HAVE_QTSVG
#include <QSvgGenerator>
#endif

#include <gsl/gsl_const_cgs.h>

/**
 * \class WorksheetView
 * \brief Worksheet view
 */

/*!
  Constructor of the class.
  Creates a view for the Worksheet \c worksheet and initializes the internal model.
*/
WorksheetView::WorksheetView(Worksheet* worksheet)
	: QGraphicsView()
	, m_worksheet(worksheet) {
	setScene(m_worksheet->scene());

	for (Plot3DArea* child : worksheet->children<Plot3DArea>())
		child->init();

	setRenderHint(QPainter::Antialiasing);
	setRubberBandSelectionMode(Qt::ContainsItemBoundingRect);
	setTransformationAnchor(QGraphicsView::AnchorViewCenter);
	setResizeAnchor(QGraphicsView::AnchorViewCenter);
	setMinimumSize(16, 16);
	setFocusPolicy(Qt::StrongFocus);

	updateScrollBarPolicy();

	viewport()->setAttribute(Qt::WA_OpaquePaintEvent);
	viewport()->setAttribute(Qt::WA_NoSystemBackground);
	setAcceptDrops(true);
	setCacheMode(QGraphicsView::CacheBackground);

	m_gridSettings.style = GridStyle::NoGrid;

	// signal/slot connections
	connect(m_worksheet, &Worksheet::requestProjectContextMenu, this, &WorksheetView::createContextMenu);
	connect(m_worksheet, &Worksheet::itemSelected, this, &WorksheetView::selectItem);
	connect(m_worksheet, &Worksheet::itemDeselected, this, &WorksheetView::deselectItem);
	connect(m_worksheet, &Worksheet::requestUpdate, this, &WorksheetView::updateBackground);
	connect(m_worksheet, &Worksheet::childAspectAboutToBeRemoved, this, &WorksheetView::aspectAboutToBeRemoved);
	connect(m_worksheet, &Worksheet::useViewSizeChanged, this, &WorksheetView::useViewSizeChanged);
	connect(m_worksheet, &Worksheet::layoutChanged, this, &WorksheetView::layoutChanged);
	connect(m_worksheet, &Worksheet::changed, this, [=] {
		if (m_magnificationWindow && m_magnificationWindow->isVisible())
			updateMagnificationWindow(mapToScene(mapFromGlobal(QCursor::pos())));
	});
	connect(scene(), &QGraphicsScene::selectionChanged, this, &WorksheetView::selectionChanged);

	// Resize the view to make the complete scene visible.
	const auto* win = window();
	if (win) {
		const auto* handle = win->windowHandle();
		if (handle) {
			const auto* screen = handle->screen();
			if (!m_worksheet->isLoading()) {
				float w = Worksheet::convertFromSceneUnits(sceneRect().width(), Worksheet::Unit::Inch);
				float h = Worksheet::convertFromSceneUnits(sceneRect().height(), Worksheet::Unit::Inch);
				auto dpi = GuiTools::dpi(this);
				w *= dpi.first;
				h *= dpi.second;
				resize(w * 1.1, h * 1.1);
			}
			if (screen) {
				// Rescale to the original size
				static const qreal hscale = screen->physicalDotsPerInchX() / (Worksheet::convertToSceneUnits(1, Worksheet::Unit::Inch));
				static const qreal vscale = screen->physicalDotsPerInchY() / (Worksheet::convertToSceneUnits(1, Worksheet::Unit::Inch));
				setTransform(QTransform::fromScale(hscale, vscale));
			}
		} else
			qWarning() << "Window handle is null.";

	} else
		qWarning() << "Window is null.";

	// rescale to the original size
	static const qreal hscale = GuiTools::dpi(this).first / (Worksheet::convertToSceneUnits(1, Worksheet::Unit::Inch));
	static const qreal vscale = GuiTools::dpi(this).second / (Worksheet::convertToSceneUnits(1, Worksheet::Unit::Inch));
	setTransform(QTransform::fromScale(hscale, vscale));

	initBasicActions();
	installEventFilter(this);
}

/*!
 * initializes couple of actions that have shortcuts assigned in the constructor as opposed
 * to other actions in initAction() that are create on demand only if the context menu is requested
 */
void WorksheetView::initBasicActions() {
	selectAllAction = new QAction(QIcon::fromTheme(QStringLiteral("edit-select-all")), i18n("Select All"), this);
	this->addAction(selectAllAction);
	connect(selectAllAction, &QAction::triggered, this, &WorksheetView::selectAllElements);

	deleteAction = new QAction(QIcon::fromTheme(QStringLiteral("edit-delete")), i18n("Delete"), this);
	this->addAction(deleteAction);
	connect(deleteAction, &QAction::triggered, this, &WorksheetView::deleteElement);

	backspaceAction = new QAction(this);
	this->addAction(backspaceAction);
	connect(backspaceAction, &QAction::triggered, this, &WorksheetView::deleteElement);

	// Zoom actions
	zoomActionGroup = new QActionGroup(this);
	zoomInViewAction = new QAction(QIcon::fromTheme(QStringLiteral("zoom-in")), i18n("Zoom In"), zoomActionGroup);
	zoomInViewAction->setData(static_cast<int>(ZoomMode::ZoomIn));

	zoomOutViewAction = new QAction(QIcon::fromTheme(QStringLiteral("zoom-out")), i18n("Zoom Out"), zoomActionGroup);
	zoomOutViewAction->setData(static_cast<int>(ZoomMode::ZoomOut));

	zoomOriginAction = new QAction(QIcon::fromTheme(QStringLiteral("zoom-original")), i18n("Original Size"), zoomActionGroup);
	zoomOriginAction->setData(static_cast<int>(ZoomMode::ZoomOrigin));
}

void WorksheetView::initActions() {
	auto* addNewActionGroup = new QActionGroup(this);
	auto* mouseModeActionGroup = new QActionGroup(this);
	auto* layoutActionGroup = new QActionGroup(this);
	auto* gridActionGroup = new QActionGroup(this);
	gridActionGroup->setExclusive(true);
	magnificationActionGroup = new QActionGroup(this);

	auto* fitActionGroup = new QActionGroup(zoomActionGroup);
	fitActionGroup->setExclusive(true);
	zoomFitNoneAction = new QAction(QIcon::fromTheme(QStringLiteral("zoom-fit-none")), i18nc("Zoom", "No fit"), fitActionGroup);
	zoomFitNoneAction->setCheckable(true);
	zoomFitNoneAction->setChecked(true);
	zoomFitNoneAction->setData((int)Worksheet::ZoomFit::None);
	zoomFitPageHeightAction = new QAction(QIcon::fromTheme(QStringLiteral("zoom-fit-height")), i18nc("Zoom", "Fit to Height"), fitActionGroup);
	zoomFitPageHeightAction->setCheckable(true);
	zoomFitPageHeightAction->setData((int)Worksheet::ZoomFit::FitToHeight);
	zoomFitPageWidthAction = new QAction(QIcon::fromTheme(QStringLiteral("zoom-fit-width")), i18nc("Zoom", "Fit to Width"), fitActionGroup);
	zoomFitPageWidthAction->setCheckable(true);
	zoomFitPageWidthAction->setData((int)Worksheet::ZoomFit::FitToWidth);
	zoomFitSelectionAction = new QAction(QIcon::fromTheme(QStringLiteral("zoom-fit-selection")), i18nc("Zoom", "Fit to Selection"), fitActionGroup);
	zoomFitSelectionAction->setCheckable(true);
	zoomFitSelectionAction->setData((int)Worksheet::ZoomFit::FitToSelection);
	zoomFitAction = new QAction(QIcon::fromTheme(QStringLiteral("zoom-fit")), i18nc("Zoom", "Fit"), fitActionGroup);
	zoomFitAction->setCheckable(true);
	zoomFitAction->setData((int)Worksheet::ZoomFit::Fit);

	// Mouse mode actions
	selectionModeAction = new QAction(QIcon::fromTheme(QStringLiteral("labplot-cursor-arrow")), i18n("Select and Edit"), mouseModeActionGroup);
	selectionModeAction->setData(static_cast<int>(MouseMode::Selection));
	selectionModeAction->setCheckable(true);
	selectionModeAction->setChecked(true);

	navigationModeAction = new QAction(QIcon::fromTheme(QStringLiteral("input-mouse")), i18n("Navigate"), mouseModeActionGroup);
	navigationModeAction->setData(static_cast<int>(MouseMode::Navigation));
	navigationModeAction->setCheckable(true);

	zoomSelectionModeAction = new QAction(QIcon::fromTheme(QStringLiteral("page-zoom")), i18n("Select and Zoom"), mouseModeActionGroup);
	zoomSelectionModeAction->setData(static_cast<int>(MouseMode::ZoomSelection));
	zoomSelectionModeAction->setCheckable(true);

	// Magnification actions
	auto* action = new QAction(QIcon::fromTheme(QStringLiteral("labplot-1x-zoom")), i18n("No Magnification"), magnificationActionGroup);
	action->setData(1);
	action->setCheckable(true);
	action->setChecked(true);

	action = new QAction(QIcon::fromTheme(QStringLiteral("labplot-2x-zoom")), i18n("2x Magnification"), magnificationActionGroup);
	action->setData(2);
	action->setCheckable(true);

	action = new QAction(QIcon::fromTheme(QStringLiteral("labplot-3x-zoom")), i18n("3x Magnification"), magnificationActionGroup);
	action->setData(3);
	action->setCheckable(true);

	action = new QAction(QIcon::fromTheme(QStringLiteral("labplot-4x-zoom")), i18n("4x Magnification"), magnificationActionGroup);
	action->setData(4);
	action->setCheckable(true);

	action = new QAction(QIcon::fromTheme(QStringLiteral("labplot-5x-zoom")), i18n("5x Magnification"), magnificationActionGroup);
	action->setData(5);
	action->setCheckable(true);

	// "Add new" related actions
	addCartesianPlot1Action = new QAction(QIcon::fromTheme(QStringLiteral("labplot-xy-plot-four-axes")), i18n("Four Axes"), addNewActionGroup);
	addCartesianPlot1Action->setData(static_cast<int>(AddNewMode::PlotAreaFourAxes));

	addCartesianPlot2Action = new QAction(QIcon::fromTheme(QStringLiteral("labplot-xy-plot-two-axes")), i18n("Two Axes"), addNewActionGroup);
	addCartesianPlot2Action->setData(static_cast<int>(AddNewMode::PlotAreaTwoAxes));

	addCartesianPlot3Action = new QAction(QIcon::fromTheme(QStringLiteral("labplot-xy-plot-two-axes-centered")), i18n("Two Axes, Centered"), addNewActionGroup);
	addCartesianPlot3Action->setData(static_cast<int>(AddNewMode::PlotAreaTwoAxesCentered));

	addCartesianPlot4Action =
		new QAction(QIcon::fromTheme(QStringLiteral("labplot-xy-plot-two-axes-centered-origin")), i18n("Two Axes, Crossing at Origin"), addNewActionGroup);
	addCartesianPlot4Action->setData(static_cast<int>(AddNewMode::PlotAreaTwoAxesCenteredZero));

	addCartesianPlotTemplateAction = new QAction(QIcon::fromTheme(QStringLiteral("document-new-from-template")), i18n("Load from Template"), addNewActionGroup);
	addCartesianPlotTemplateAction->setData(static_cast<int>(AddNewMode::PlotAreaFromTemplate));

	add3DPlotAction = new QAction(QIcon::fromTheme(QStringLiteral("office-chart-line")), i18n("3D plot"), addNewActionGroup);
	add3DPlotAction->setData(static_cast<int>(AddNewMode::Plot3D));

	addTextLabelAction = new QAction(QIcon::fromTheme(QStringLiteral("draw-text")), i18n("Text"), addNewActionGroup);
	addTextLabelAction->setData(static_cast<int>(AddNewMode::TextLabel));

	addImageAction = new QAction(QIcon::fromTheme(QStringLiteral("viewimage")), i18n("Image"), addNewActionGroup);
	addImageAction->setData(static_cast<int>(AddNewMode::Image));

	// Layout actions
	// TODO: the icons labplot-editvlayout and labplot-edithlayout are confusing for the user.
	// the orientation is visualized as a horizontal or vertical line on the icon, but the user
	// perceives the two objects (resembles plots on the worksheet) separated by this line much stronger than the line itself.
	// with this, the two objects separated by a vertical line are perceived to be laid out in a _horizontal_ order and the
	// same for the vertical line. Because of this we change the icons here. We can rename the icons later in the breeze icon set.
	verticalLayoutAction = new QAction(QIcon::fromTheme(QStringLiteral("labplot-edithlayout")), i18n("Vertical Layout"), layoutActionGroup);
	verticalLayoutAction->setData(static_cast<int>(Worksheet::Layout::VerticalLayout));
	verticalLayoutAction->setCheckable(true);

	horizontalLayoutAction = new QAction(QIcon::fromTheme(QStringLiteral("labplot-editvlayout")), i18n("Horizontal Layout"), layoutActionGroup);
	horizontalLayoutAction->setData(static_cast<int>(Worksheet::Layout::HorizontalLayout));
	horizontalLayoutAction->setCheckable(true);

	gridLayoutAction = new QAction(QIcon::fromTheme(QStringLiteral("labplot-editgrid")), i18n("Grid Layout"), layoutActionGroup);
	gridLayoutAction->setData(static_cast<int>(Worksheet::Layout::GridLayout));
	gridLayoutAction->setCheckable(true);

	breakLayoutAction = new QAction(QIcon::fromTheme(QStringLiteral("labplot-editbreaklayout")), i18n("No Layout"), layoutActionGroup);
	breakLayoutAction->setData(static_cast<int>(Worksheet::Layout::NoLayout));
	breakLayoutAction->setEnabled(false);

	// Grid actions
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

	showPresenterMode = new QAction(QIcon::fromTheme(QStringLiteral("view-fullscreen")), i18n("Presenter Mode"), this);
	showPresenterMode->setShortcut(Qt::Key_F);

	// check the action corresponding to the currently active layout in worksheet
	this->layoutChanged(m_worksheet->layout());

	connect(addNewActionGroup, &QActionGroup::triggered, this, &WorksheetView::addNew);
	connect(mouseModeActionGroup, &QActionGroup::triggered, this, &WorksheetView::changeMouseMode);
	connect(fitActionGroup, &QActionGroup::triggered, this, &WorksheetView::changeZoomFit);
	connect(zoomActionGroup, &QActionGroup::triggered, this, &WorksheetView::changeZoom);
	connect(magnificationActionGroup, &QActionGroup::triggered, this, &WorksheetView::changeMagnification);
	connect(layoutActionGroup, &QActionGroup::triggered, this, &WorksheetView::changeLayout);
	connect(gridActionGroup, &QActionGroup::triggered, this, &WorksheetView::changeGrid);
	connect(snapToGridAction, &QAction::triggered, this, &WorksheetView::changeSnapToGrid);
	connect(showPresenterMode, &QAction::triggered, this, &WorksheetView::presenterMode);

	// worksheet control actions
	plotsInteractiveAction = new QAction(QIcon::fromTheme(QStringLiteral("hidemouse")), i18n("Interactive Plots"), this);
	plotsInteractiveAction->setToolTip(i18n("If not activated, plots on the worksheet don't react on drag and mouse wheel events."));
	plotsInteractiveAction->setCheckable(true);
	plotsInteractiveAction->setChecked(m_worksheet->plotsInteractive());
	connect(plotsInteractiveAction, &QAction::triggered, this, &WorksheetView::plotsInteractiveActionChanged);

	// actions for cartesian plots
	auto* cartesianPlotActionModeActionGroup = new QActionGroup(this);
	cartesianPlotActionModeActionGroup->setExclusive(true);
	plotApplyToSelectionAction = new QAction(i18n("Selected Plot Areas"), cartesianPlotActionModeActionGroup);
	plotApplyToSelectionAction->setCheckable(true);
	plotApplyToAllAction = new QAction(i18n("All Plot Areas"), cartesianPlotActionModeActionGroup);
	plotApplyToAllAction->setCheckable(true);
	plotApplyToAllXAction = new QAction(i18n("All Plot Areas X"), cartesianPlotActionModeActionGroup);
	plotApplyToAllXAction->setCheckable(true);
	plotApplyToAllYAction = new QAction(i18n("All Plot Areas Y"), cartesianPlotActionModeActionGroup);
	plotApplyToAllYAction->setCheckable(true);
	setCartesianPlotActionMode(m_worksheet->cartesianPlotActionMode());
	connect(cartesianPlotActionModeActionGroup, &QActionGroup::triggered, this, &WorksheetView::cartesianPlotActionModeChanged);

	// cursor apply to all/selected
	auto* plotActionCursorGroup = new QActionGroup(this);
	plotActionCursorGroup->setExclusive(true);
	plotApplyToSelectionCursor = new QAction(i18n("Selected Plot Areas"), plotActionCursorGroup);
	plotApplyToSelectionCursor->setCheckable(true);
	plotApplyToAllCursor = new QAction(i18n("All Plot Areas"), plotActionCursorGroup);
	plotApplyToAllCursor->setCheckable(true);
	setCartesianPlotCursorMode(m_worksheet->cartesianPlotCursorMode());
	connect(plotActionCursorGroup, &QActionGroup::triggered, this, &WorksheetView::cartesianPlotCursorModeChanged);

	initPlotNavigationActions();
}

/*!
 * initialized plot area related actions for the mouse mode (select, select and zoom, etc.)
 * and for the navigation (scale, zoom, shift).
 */
void WorksheetView::initPlotNavigationActions() {
	// mouse mode actions
	m_plotMouseModeActionGroup = new QActionGroup(this);
	m_plotMouseModeActionGroup->setExclusive(true);
	plotSelectionModeAction = new QAction(QIcon::fromTheme(QStringLiteral("labplot-cursor-arrow")), i18n("Select and Edit"), m_plotMouseModeActionGroup);
	plotSelectionModeAction->setData(static_cast<int>(CartesianPlot::MouseMode::Selection));
	plotSelectionModeAction->setCheckable(true);
	plotSelectionModeAction->setChecked(true);

	plotCrosshairModeAction = new QAction(QIcon::fromTheme(QStringLiteral("crosshairs")), i18n("Crosshair"), m_plotMouseModeActionGroup);
	plotCrosshairModeAction->setData(static_cast<int>(CartesianPlot::MouseMode::Crosshair));
	plotCrosshairModeAction->setCheckable(true);

	plotZoomSelectionModeAction =
		new QAction(QIcon::fromTheme(QStringLiteral("labplot-zoom-select")), i18n("Select Region and Zoom In"), m_plotMouseModeActionGroup);
	plotZoomSelectionModeAction->setData(static_cast<int>(CartesianPlot::MouseMode::ZoomSelection));
	plotZoomSelectionModeAction->setCheckable(true);

	plotZoomXSelectionModeAction =
		new QAction(QIcon::fromTheme(QStringLiteral("labplot-zoom-select-x")), i18n("Select X-Region and Zoom In"), m_plotMouseModeActionGroup);
	plotZoomXSelectionModeAction->setData(static_cast<int>(CartesianPlot::MouseMode::ZoomXSelection));
	plotZoomXSelectionModeAction->setCheckable(true);

	plotZoomYSelectionModeAction =
		new QAction(QIcon::fromTheme(QStringLiteral("labplot-zoom-select-y")), i18n("Select Y-Region and Zoom In"), m_plotMouseModeActionGroup);
	plotZoomYSelectionModeAction->setData(static_cast<int>(CartesianPlot::MouseMode::ZoomYSelection));
	plotZoomYSelectionModeAction->setCheckable(true);

	// TODO: change ICON
	plotCursorModeAction = new QAction(QIcon::fromTheme(QStringLiteral("debug-execute-from-cursor")), i18n("Cursor"), m_plotMouseModeActionGroup);
	plotCursorModeAction->setData(static_cast<int>(CartesianPlot::MouseMode::Cursor));
	plotCursorModeAction->setCheckable(true);

	connect(m_plotMouseModeActionGroup, &QActionGroup::triggered, this, &WorksheetView::changePlotMouseMode);

	// navigation actions
	m_plotNavigationActionGroup = new QActionGroup(this);
	plotScaleAutoAction = new QAction(QIcon::fromTheme(QStringLiteral("labplot-auto-scale-all")), i18n("Auto Scale"), m_plotNavigationActionGroup);
	plotScaleAutoAction->setData(static_cast<int>(CartesianPlot::NavigationOperation::ScaleAuto));
	plotScaleAutoAction->setShortcut(Qt::Key_1);

	plotScaleAutoXAction = new QAction(QIcon::fromTheme(QStringLiteral("labplot-auto-scale-x")), i18n("Auto Scale X"), m_plotNavigationActionGroup);
	plotScaleAutoXAction->setData(static_cast<int>(CartesianPlot::NavigationOperation::ScaleAutoX));
	plotScaleAutoXAction->setShortcut(Qt::CTRL | Qt::SHIFT | Qt::Key_X);

	plotScaleAutoYAction = new QAction(QIcon::fromTheme(QStringLiteral("labplot-auto-scale-y")), i18n("Auto Scale Y"), m_plotNavigationActionGroup);
	plotScaleAutoYAction->setData(static_cast<int>(CartesianPlot::NavigationOperation::ScaleAutoY));
	plotScaleAutoYAction->setShortcut(Qt::CTRL | Qt::SHIFT | Qt::Key_Y);

	plotZoomInAction = new QAction(QIcon::fromTheme(QStringLiteral("zoom-in")), i18n("Zoom In"), m_plotNavigationActionGroup);
	plotZoomInAction->setData(static_cast<int>(CartesianPlot::NavigationOperation::ZoomIn));
	plotZoomInAction->setShortcut(Qt::Key_Plus);

	plotZoomOutAction = new QAction(QIcon::fromTheme(QStringLiteral("zoom-out")), i18n("Zoom Out"), m_plotNavigationActionGroup);
	plotZoomOutAction->setData(static_cast<int>(CartesianPlot::NavigationOperation::ZoomOut));
	plotZoomOutAction->setShortcut(Qt::Key_Minus);

	plotZoomInXAction = new QAction(QIcon::fromTheme(QStringLiteral("labplot-zoom-in-x")), i18n("Zoom In X"), m_plotNavigationActionGroup);
	plotZoomInXAction->setData(static_cast<int>(CartesianPlot::NavigationOperation::ZoomInX));
	plotZoomInXAction->setShortcut(Qt::Key_X);

	plotZoomOutXAction = new QAction(QIcon::fromTheme(QStringLiteral("labplot-zoom-out-x")), i18n("Zoom Out X"), m_plotNavigationActionGroup);
	plotZoomOutXAction->setData(static_cast<int>(CartesianPlot::NavigationOperation::ZoomOutX));
	plotZoomOutXAction->setShortcut(Qt::SHIFT | Qt::Key_X);

	plotZoomInYAction = new QAction(QIcon::fromTheme(QStringLiteral("labplot-zoom-in-y")), i18n("Zoom In Y"), m_plotNavigationActionGroup);
	plotZoomInYAction->setData(static_cast<int>(CartesianPlot::NavigationOperation::ZoomInY));
	plotZoomInYAction->setShortcut(Qt::Key_Y);

	plotZoomOutYAction = new QAction(QIcon::fromTheme(QStringLiteral("labplot-zoom-out-y")), i18n("Zoom Out Y"), m_plotNavigationActionGroup);
	plotZoomOutYAction->setData(static_cast<int>(CartesianPlot::NavigationOperation::ZoomOutY));
	plotZoomOutYAction->setShortcut(Qt::SHIFT | Qt::Key_Y);

	plotShiftLeftXAction = new QAction(QIcon::fromTheme(QStringLiteral("labplot-shift-left-x")), i18n("Shift Left X"), m_plotNavigationActionGroup);
	plotShiftLeftXAction->setData(static_cast<int>(CartesianPlot::NavigationOperation::ShiftLeftX));
	plotShiftRightXAction = new QAction(QIcon::fromTheme(QStringLiteral("labplot-shift-right-x")), i18n("Shift Right X"), m_plotNavigationActionGroup);
	plotShiftRightXAction->setData(static_cast<int>(CartesianPlot::NavigationOperation::ShiftRightX));
	plotShiftUpYAction = new QAction(QIcon::fromTheme(QStringLiteral("labplot-shift-up-y")), i18n("Shift Up Y"), m_plotNavigationActionGroup);
	plotShiftUpYAction->setData(static_cast<int>(CartesianPlot::NavigationOperation::ShiftUpY));
	plotShiftDownYAction = new QAction(QIcon::fromTheme(QStringLiteral("labplot-shift-down-y")), i18n("Shift Down Y"), m_plotNavigationActionGroup);
	plotShiftDownYAction->setData(static_cast<int>(CartesianPlot::NavigationOperation::ShiftDownY));

	connect(m_plotNavigationActionGroup, &QActionGroup::triggered, this, &WorksheetView::changePlotNavigation);
}

void WorksheetView::initMenus() {
	if (!m_actionsInitialized)
		initActions();

	m_addNewPlotMenu = new QMenu(i18n("Plot Area"), this);
	m_addNewPlotMenu->addAction(addCartesianPlot1Action);
	m_addNewPlotMenu->addAction(addCartesianPlot2Action);
	m_addNewPlotMenu->addAction(addCartesianPlot3Action);
	m_addNewPlotMenu->addAction(addCartesianPlot4Action);
	m_addNewPlotMenu->addSeparator();
	m_addNewPlotMenu->addAction(addCartesianPlotTemplateAction);
	m_addNewPlotMenu->addSeparator();
	m_addNewPlotMenu->addAction(add3DPlotAction);

	m_addNewMenu = new QMenu(i18n("Add New"), this);
	m_addNewMenu->setIcon(QIcon::fromTheme(QStringLiteral("list-add")));
	m_addNewMenu->addMenu(m_addNewPlotMenu)->setIcon(QIcon::fromTheme(QStringLiteral("office-chart-line")));
	m_addNewMenu->addSeparator();
	m_addNewMenu->addAction(addTextLabelAction);
	m_addNewMenu->addAction(addImageAction);

	m_viewMouseModeMenu = new QMenu(i18n("Mouse Mode"), this);
	m_viewMouseModeMenu->setIcon(QIcon::fromTheme(QStringLiteral("input-mouse")));
	m_viewMouseModeMenu->addAction(selectionModeAction);
	m_viewMouseModeMenu->addAction(navigationModeAction);
	m_viewMouseModeMenu->addAction(zoomSelectionModeAction);

	m_zoomMenu = new QMenu(i18n("Zoom"), this);
	m_zoomMenu->setIcon(QIcon::fromTheme(QStringLiteral("zoom-draw")));
	m_zoomMenu->addAction(zoomInViewAction);
	m_zoomMenu->addAction(zoomOutViewAction);
	m_zoomMenu->addAction(zoomOriginAction);
	m_zoomMenu->addSeparator();
	m_zoomMenu->addAction(zoomFitNoneAction);
	m_zoomMenu->addAction(zoomFitAction);
	m_zoomMenu->addAction(zoomFitPageHeightAction);
	m_zoomMenu->addAction(zoomFitPageWidthAction);
	m_zoomMenu->addAction(zoomFitSelectionAction);

	m_magnificationMenu = new QMenu(i18n("Magnification"), this);
	m_magnificationMenu->setIcon(QIcon::fromTheme(QStringLiteral("zoom-in")));
	for (auto* action : magnificationActionGroup->actions())
		m_magnificationMenu->addAction(action);

	m_layoutMenu = new QMenu(i18n("Layout"), this);
	m_layoutMenu->setIcon(QIcon::fromTheme(QStringLiteral("labplot-editbreaklayout")));
	m_layoutMenu->addAction(verticalLayoutAction);
	m_layoutMenu->addAction(horizontalLayoutAction);
	m_layoutMenu->addAction(gridLayoutAction);
	m_layoutMenu->addSeparator();
	m_layoutMenu->addAction(breakLayoutAction);

	m_gridMenu = new QMenu(i18n("Grid"), this);
	m_gridMenu->setIcon(QIcon::fromTheme(QStringLiteral("view-grid")));
	m_gridMenu->addAction(noGridAction);
	m_gridMenu->addSeparator();
	m_gridMenu->addAction(sparseLineGridAction);
	m_gridMenu->addAction(denseLineGridAction);
	m_gridMenu->addSeparator();
	m_gridMenu->addAction(sparseDotGridAction);
	m_gridMenu->addAction(denseDotGridAction);
	m_gridMenu->addSeparator();
	m_gridMenu->addAction(customGridAction);
	// TODO: implement "snap to grid" and activate this action
	// 	m_gridMenu->addSeparator();
	// 	m_gridMenu->addAction(snapToGridAction);

	m_plotMenu = new QMenu(i18n("Plot Area"), this);
	m_plotMenu->setIcon(QIcon::fromTheme(QStringLiteral("office-chart-line")));

	m_plotActionModeMenu = new QMenu(i18n("Apply Actions to"), this);
	m_plotActionModeMenu->setIcon(QIcon::fromTheme(QStringLiteral("dialog-ok-apply")));
	m_plotActionModeMenu->addAction(plotApplyToSelectionAction);
	m_plotActionModeMenu->addAction(plotApplyToAllAction);
	m_plotActionModeMenu->addAction(plotApplyToAllXAction);
	m_plotActionModeMenu->addAction(plotApplyToAllYAction);

	m_plotCursorModeMenu = new QMenu(i18n("Apply Cursor to"), this);
	m_plotCursorModeMenu->setIcon(QIcon::fromTheme(QStringLiteral("debug-execute-from-cursor")));
	m_plotCursorModeMenu->addAction(plotApplyToSelectionCursor);
	m_plotCursorModeMenu->addAction(plotApplyToAllCursor);

	m_plotMenu->addMenu(m_plotActionModeMenu);
	m_plotMenu->addMenu(m_plotCursorModeMenu);

	// menus with the actions for mouse mode and for scale/zoom/shift that is added to the context menu of a plot,
	// we need to keep the actions in WorksheetView so we can apply them to all plots on the worksheet if needed.
	m_plotMouseModeMenu = new QMenu(i18n("Mouse Mode"), this);
	m_plotMouseModeMenu->setIcon(QIcon::fromTheme(QStringLiteral("input-mouse")));
	m_plotMouseModeMenu->addAction(plotSelectionModeAction);
	m_plotMouseModeMenu->addAction(plotZoomSelectionModeAction);
	m_plotMouseModeMenu->addAction(plotZoomXSelectionModeAction);
	m_plotMouseModeMenu->addAction(plotZoomYSelectionModeAction);
	m_plotMouseModeMenu->addSeparator();
	m_plotMouseModeMenu->addAction(plotCursorModeAction);

	m_plotZoomMenu = new QMenu(i18n("Zoom/Navigate"), this);
	m_plotZoomMenu->setIcon(QIcon::fromTheme(QStringLiteral("zoom-draw")));
	m_plotZoomMenu->addAction(plotScaleAutoAction);
	m_plotZoomMenu->addAction(plotScaleAutoXAction);
	m_plotZoomMenu->addAction(plotScaleAutoYAction);
	m_plotZoomMenu->addSeparator();
	m_plotZoomMenu->addAction(plotZoomInAction);
	m_plotZoomMenu->addAction(plotZoomOutAction);
	m_plotZoomMenu->addSeparator();
	m_plotZoomMenu->addAction(plotZoomInXAction);
	m_plotZoomMenu->addAction(plotZoomOutXAction);
	m_plotZoomMenu->addSeparator();
	m_plotZoomMenu->addAction(plotZoomInYAction);
	m_plotZoomMenu->addAction(plotZoomOutYAction);
	m_plotZoomMenu->addSeparator();
	m_plotZoomMenu->addAction(plotShiftLeftXAction);
	m_plotZoomMenu->addAction(plotShiftRightXAction);
	m_plotZoomMenu->addSeparator();
	m_plotZoomMenu->addAction(plotShiftUpYAction);
	m_plotZoomMenu->addAction(plotShiftDownYAction);

	// themes menu
	m_themeMenu = new QMenu(i18n("Theme"), this);
	m_themeMenu->setIcon(QIcon::fromTheme(QStringLiteral("color-management")));
#ifndef SDK
	connect(m_themeMenu, &QMenu::aboutToShow, this, [=]() {
		if (!m_themeMenu->isEmpty())
			return;
		auto* themeWidget = new ThemesWidget(nullptr);
		themeWidget->setFixedMode();
		connect(themeWidget, &ThemesWidget::themeSelected, m_worksheet, &Worksheet::setTheme);
		connect(themeWidget, &ThemesWidget::themeSelected, m_themeMenu, &QMenu::close);

		auto* widgetAction = new QWidgetAction(this);
		widgetAction->setDefaultWidget(themeWidget);
		m_themeMenu->addAction(widgetAction);
	});
#endif
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
	// there're already actions available there. Skip the first title-action
	// and insert the action at the beginning of the menu.
	if (menu->actions().size() > 1)
		firstAction = menu->actions().at(1);

	menu->insertMenu(firstAction, m_addNewMenu);
	menu->insertSeparator(firstAction);
	menu->insertMenu(firstAction, m_viewMouseModeMenu);
	menu->insertMenu(firstAction, m_zoomMenu);
	menu->insertMenu(firstAction, m_magnificationMenu);
	menu->insertSeparator(firstAction);
	menu->insertMenu(firstAction, m_layoutMenu);
	menu->insertMenu(firstAction, m_gridMenu);
	menu->insertSeparator(firstAction);
	menu->insertMenu(firstAction, m_themeMenu);
	menu->insertSeparator(firstAction);
	menu->insertAction(firstAction, plotsInteractiveAction);
	menu->insertSeparator(firstAction);
	menu->insertMenu(firstAction, m_plotMenu);
	menu->insertSeparator(firstAction);
	menu->insertAction(firstAction, showPresenterMode);
	menu->insertSeparator(firstAction);
}

#ifdef HAVE_TOUCHBAR
void WorksheetView::fillTouchBar(KDMacTouchBar* touchBar) {
	touchBar->addAction(zoomInViewAction);
	touchBar->addAction(zoomOutViewAction);
	touchBar->addAction(showPresenterMode);
}
#endif

void WorksheetView::fillAddNewPlotMenu(ToggleActionMenu* menu) const {
	menu->addAction(addCartesianPlot1Action);
	menu->addAction(addCartesianPlot2Action);
	menu->addAction(addCartesianPlot3Action);
	menu->addAction(addCartesianPlot4Action);
	menu->addSeparator();
	menu->addAction(addCartesianPlotTemplateAction);
}

void WorksheetView::fillZoomMenu(ToggleActionMenu* menu) const {
	menu->addAction(zoomInViewAction);
	menu->addAction(zoomOutViewAction);
	menu->addAction(zoomOriginAction);
	menu->addSeparator();
	menu->addAction(zoomFitNoneAction);
	menu->addAction(zoomFitAction);
	menu->addAction(zoomFitPageHeightAction);
	menu->addAction(zoomFitPageWidthAction);
	menu->addAction(zoomFitSelectionAction);
}

void WorksheetView::fillMagnificationMenu(ToggleActionMenu* menu) const {
	for (auto* action : magnificationActionGroup->actions())
		menu->addAction(action);
}

QMenu* WorksheetView::plotAddNewMenu() const {
	for (auto* item : m_selectedItems) {
		auto* w = static_cast<WorksheetElementPrivate*>(item)->q;
		if (w->type() == AspectType::CartesianPlot) {
			auto* menu = static_cast<CartesianPlot*>(w)->addNewMenu();
			return menu;
		}
	}

	return nullptr;
}

/*!
 * adds the navigation related actions to the toolbar \c toolbar,
 * used in the Presenter Widget to populate its own navigation bar.
 */
void WorksheetView::fillCartesianPlotNavigationToolBar(QToolBar* toolBar) {
	if (!plotSelectionModeAction)
		initPlotNavigationActions();
	toolBar->addAction(plotSelectionModeAction);
	toolBar->addAction(plotCrosshairModeAction);
	toolBar->addAction(plotZoomSelectionModeAction);
	toolBar->addAction(plotZoomXSelectionModeAction);
	toolBar->addAction(plotZoomYSelectionModeAction);
	toolBar->addSeparator();
	toolBar->addAction(plotScaleAutoAction);
	toolBar->addAction(plotScaleAutoXAction);
	toolBar->addAction(plotScaleAutoYAction);
	toolBar->addAction(plotZoomInAction);
	toolBar->addAction(plotZoomOutAction);
	toolBar->addAction(plotZoomInXAction);
	toolBar->addAction(plotZoomOutXAction);
	toolBar->addAction(plotZoomInYAction);
	toolBar->addAction(plotZoomOutYAction);
	toolBar->addAction(plotShiftLeftXAction);
	toolBar->addAction(plotShiftRightXAction);
	toolBar->addAction(plotShiftUpYAction);
	toolBar->addAction(plotShiftDownYAction);
	updateCartesianPlotActions();
}

void WorksheetView::setScene(QGraphicsScene* scene) {
	QGraphicsView::setScene(scene);
}

void WorksheetView::setIsClosing() {
	m_isClosing = true;
}

void WorksheetView::setCartesianPlotActionMode(Worksheet::CartesianPlotActionMode mode) {
	if (mode == Worksheet::CartesianPlotActionMode::ApplyActionToAll)
		plotApplyToAllAction->setChecked(true);
	else if (mode == Worksheet::CartesianPlotActionMode::ApplyActionToAllX)
		plotApplyToAllXAction->setChecked(true);
	else if (mode == Worksheet::CartesianPlotActionMode::ApplyActionToAllY)
		plotApplyToAllYAction->setChecked(true);
	else
		plotApplyToSelectionAction->setChecked(true);
}

void WorksheetView::setCartesianPlotCursorMode(Worksheet::CartesianPlotActionMode mode) {
	if (mode == Worksheet::CartesianPlotActionMode::ApplyActionToAll)
		plotApplyToAllCursor->setChecked(true);
	else
		plotApplyToSelectionCursor->setChecked(true);
}

void WorksheetView::setPlotInteractive(bool interactive) {
	plotsInteractiveAction->setChecked(interactive);
}

void WorksheetView::drawForeground(QPainter* painter, const QRectF& rect) {
	// QDEBUG(Q_FUNC_INFO << ", painter = " << painter << ", rect = " << rect)
	if (m_mouseMode == MouseMode::ZoomSelection && m_selectionBandIsShown) {
		painter->save();
		const QRectF& selRect = mapToScene(QRect(m_selectionStart, m_selectionEnd).normalized()).boundingRect();
		// TODO: don't hardcode for black here, use a a different color depending on the theme of the worksheet/plot under the mouse cursor?
		painter->setPen(QPen(Qt::black, 5 / transform().m11()));
		painter->drawRect(selRect);
		painter->setBrush(QApplication::palette().color(QPalette::Highlight));
		painter->setOpacity(0.2);
		painter->drawRect(selRect);
		painter->restore();
	}
	//	DEBUG(Q_FUNC_INFO << ", CALLING QGraphicsView::drawForeground. items = " << QGraphicsView::items().size()
	//		<< ", scene items = " << scene()->items().count() )
	QGraphicsView::drawForeground(painter, rect);
}

void WorksheetView::drawBackgroundItems(QPainter* painter, const QRectF& scene_rect) {
	// canvas
	m_worksheet->background()->draw(painter, scene_rect);

	// grid
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

			// horizontal lines
			y = top + m_gridSettings.verticalSpacing;
			while (y < bottom) {
				line.setLine(left, y, right, y);
				painter->drawLine(line);
				y += m_gridSettings.verticalSpacing;
			}

			// vertical lines
			x = left + m_gridSettings.horizontalSpacing;
			while (x < right) {
				line.setLine(x, top, x, bottom);
				painter->drawLine(line);
				x += m_gridSettings.horizontalSpacing;
			}
		} else { // DotGrid
			y = top + m_gridSettings.verticalSpacing;
			while (y < bottom) {
				x = left; // + m_gridSettings.horizontalSpacing;
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

	// painter->setRenderHint(QPainter::Antialiasing);
	QRectF scene_rect = sceneRect();

	if (!m_worksheet->useViewSize()) {
		// background
		KColorScheme scheme(QPalette::Active, KColorScheme::Window);
		const QColor color = scheme.background().color();
		if (!scene_rect.contains(rect))
			painter->fillRect(rect, color);

		// shadow
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

CartesianPlot* WorksheetView::plotAt(QPoint pos) const {
	QGraphicsItem* item = itemAt(pos);
	if (!item)
		return nullptr;

	QGraphicsItem* plotItem = nullptr;
	if (item->data(0).toInt() == static_cast<int>(AspectType::CartesianPlot))
		plotItem = item;
	else {
		if (item->parentItem() && item->parentItem()->data(0).toInt() == static_cast<int>(AspectType::CartesianPlot))
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

// ##############################################################################
// ####################################  Events   ###############################
// ##############################################################################
void WorksheetView::resizeEvent(QResizeEvent* event) {
	if (m_isClosing)
		return;

	if (m_worksheet->useViewSize())
		this->processResize();
	else
		updateFit();

	QGraphicsView::resizeEvent(event);
}

void WorksheetView::wheelEvent(QWheelEvent* event) {
	if (isInteractive() && (m_mouseMode == MouseMode::ZoomSelection || (QApplication::keyboardModifiers() & Qt::ControlModifier))) {
		if (!zoomFitNoneAction)
			initActions();
		zoomFitNoneAction->setChecked(true);
		m_worksheet->setZoomFit(Worksheet::ZoomFit::None);
		updateScrollBarPolicy();

		// https://wiki.qt.io/Smooth_Zoom_In_QGraphicsView
		QPoint numDegrees = event->angleDelta() / 8;
		int numSteps = numDegrees.y() / 15; // see QWheelEvent documentation
		zoom(numSteps);
	} else
		QGraphicsView::wheelEvent(event);

	if (m_magnificationWindow && m_magnificationWindow->isVisible())
		updateMagnificationWindow(mapToScene(event->position().toPoint()));
}

void WorksheetView::zoom(int numSteps) {
	m_numScheduledScalings += numSteps;
	if (m_numScheduledScalings * numSteps < 0) // if user moved the wheel in another direction, we reset previously scheduled scalings
		m_numScheduledScalings = numSteps;

	if (!m_zoomTimeLine) {
		m_zoomTimeLine = new QTimeLine(350, this);
		m_zoomTimeLine->setUpdateInterval(20);
		connect(m_zoomTimeLine, &QTimeLine::valueChanged, this, &WorksheetView::scalingTime);
		connect(m_zoomTimeLine, &QTimeLine::finished, this, &WorksheetView::animFinished);
	}

	if (m_zoomTimeLine->state() == QTimeLine::Running)
		m_zoomTimeLine->stop();

	m_zoomTimeLine->start();
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

	updateLabelsZoom();
}

void WorksheetView::mousePressEvent(QMouseEvent* event) {
	// prevent the deselection of items when context menu event
	// was triggered (right button click)
	if (event->button() == Qt::RightButton) {
		event->accept();
		return;
	}

	if (event->button() == Qt::LeftButton && m_mouseMode == MouseMode::ZoomSelection) {
		m_selectionStart = event->pos();
		m_selectionEnd = m_selectionStart; // select&zoom'g starts -> reset the end point to the start point
		m_selectionBandIsShown = true;
		QGraphicsView::mousePressEvent(event);
		return;
	}

	// select the worksheet in the project explorer if the view was clicked
	// and there is no selection currently. We need this for the case when
	// there is a single worksheet in the project and we change from the project-node
	// in the project explorer to the worksheet-node by clicking the view.
	if (scene()->selectedItems().isEmpty())
		m_worksheet->setSelectedInView(true);

	QGraphicsView::mousePressEvent(event);
}

void WorksheetView::mouseReleaseEvent(QMouseEvent* event) {
	if (event->button() == Qt::LeftButton && m_mouseMode == MouseMode::ZoomSelection) {
		m_selectionBandIsShown = false;
		viewport()->repaint(QRect(m_selectionStart, m_selectionEnd).normalized());

		// don't zoom if very small region was selected, avoid occasional/unwanted zooming
		m_selectionEnd = event->pos();
		if (abs(m_selectionEnd.x() - m_selectionStart.x()) > 20 && abs(m_selectionEnd.y() - m_selectionStart.y()) > 20)
			fitInView(mapToScene(QRect(m_selectionStart, m_selectionEnd).normalized()).boundingRect(), Qt::KeepAspectRatio);
	}

	QGraphicsView::mouseReleaseEvent(event);
}

void WorksheetView::mouseDoubleClickEvent(QMouseEvent*) {
	Q_EMIT propertiesExplorerRequested();
}

void WorksheetView::mouseMoveEvent(QMouseEvent* event) {
	if (m_suppressSelectionChangedEvent)
		return QGraphicsView::mouseMoveEvent(event);
	if (m_mouseMode == MouseMode::Selection && m_cartesianPlotMouseMode != CartesianPlot::MouseMode::Selection) {
		// check whether there is a cartesian plot under the cursor
		// and set the cursor appearance according to the current mouse mode for the cartesian plots
		if (plotAt(event->pos())) {
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
		qreal penWidth = 5 / transform().m11();
		rect.setX(rect.x() - penWidth);
		rect.setY(rect.y() - penWidth);
		rect.setHeight(rect.height() + 2 * penWidth);
		rect.setWidth(rect.width() + 2 * penWidth);
		viewport()->repaint(rect);
	}

	// show the magnification window
	if (m_magnificationFactor > 1 /*&& m_mouseMode == SelectAndEditMode*/) {
		if (!m_magnificationWindow) {
			m_magnificationWindow = new QGraphicsPixmapItem(nullptr);
			m_magnificationWindow->setZValue(std::numeric_limits<int>::max());
			scene()->addItem(m_magnificationWindow);
		}

		updateMagnificationWindow(mapToScene(event->pos()));
	} else if (m_magnificationWindow)
		m_magnificationWindow->setVisible(false);

	QGraphicsView::mouseMoveEvent(event);
}

/*!
 * Updates the magnified content of the scene under the cursor that is shown in the magnification window.
 * \pos is the current position of the cursor in scene coordinates which defines the middle of the magnification window.
 */
void WorksheetView::updateMagnificationWindow(const QPointF& pos) {
	m_magnificationWindow->setVisible(false);

	// copy the part of the view to be shown magnified
	const int size = Worksheet::convertToSceneUnits(2.0, Worksheet::Unit::Centimeter) / transform().m11();
	const QRectF copyRect(pos.x() - size / (2 * m_magnificationFactor),
						  pos.y() - size / (2 * m_magnificationFactor),
						  size / m_magnificationFactor,
						  size / m_magnificationFactor);
	QPixmap px = grab(mapFromScene(copyRect).boundingRect());
	px = px.scaled(size, size, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

	// draw the bounding rect
	QPainter painter(&px);
	const QPen pen = QPen(Qt::darkGray, 2 / transform().m11());
	painter.setPen(pen);
	QRect rect = px.rect();
	rect.setWidth(rect.width() - pen.widthF() / 2);
	rect.setHeight(rect.height() - pen.widthF() / 2);
	painter.drawRect(rect);

	// set the pixmap and show it again
	m_magnificationWindow->setPixmap(px);
	m_magnificationWindow->setPos(pos.x() - px.width() / 2, pos.y() - px.height() / 2);
	m_magnificationWindow->setVisible(true);
}

void WorksheetView::contextMenuEvent(QContextMenuEvent* e) {
	if ((m_magnificationWindow && m_magnificationWindow->isVisible() && items(e->pos()).size() == 1) || !itemAt(e->pos())) {
		// no item or only the magnification window under the cursor -> show the context menu for the worksheet
		m_cursorPos = mapToScene(e->pos());
		m_calledFromContextMenu = true;
		m_worksheet->createContextMenu()->exec(QCursor::pos());
	} else {
		// propagate the event to the scene and graphics items
		QGraphicsView::contextMenuEvent(e);
	}
}

void WorksheetView::keyPressEvent(QKeyEvent* event) {
	// handle delete
	if (event->matches(QKeySequence::Delete)) {
		deleteElement();
		QGraphicsView::keyPressEvent(event);
		return;
	}

	// handle copy/paste/duplicate

	// determine the currently selected aspect
	AbstractAspect* aspect = nullptr;
	if (m_selectedItems.count() == 1) {
		// at the moment we allow to copy/paste/duplicate one single selected object only
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

	if (!aspect) {
		QGraphicsView::keyPressEvent(event);
		return;
	}

	if (event->matches(QKeySequence::Copy)) {
		exportToClipboard(); // export the image to the clipboard
		if (aspect != m_worksheet)
			aspect->copy(); // copy the selected object itself
	} else if (event->matches(QKeySequence::Paste)) {
		// paste
		QString name;
		auto t = AbstractAspect::clipboardAspectType(name);
		if (t != AspectType::AbstractAspect && aspect->pasteTypes().indexOf(t) != -1)
			aspect->paste();
	} else if ((event->modifiers() & Qt::ControlModifier) && (event->key() == Qt::Key_D) && aspect != m_worksheet) {
		// duplicate
		aspect->copy();
		aspect->parentAspect()->paste(true);

		/* zooming related key events, handle them here so we can also use them in PresenterWidget without registering shortcuts */
	} else if ((event->modifiers() & Qt::ControlModifier) && (event->key() == Qt::Key_Plus)) {
		changeZoom(zoomInViewAction);
	} else if ((event->modifiers() & Qt::ControlModifier) && (event->key() == Qt::Key_Minus)) {
		changeZoom(zoomOutViewAction);
	} else if ((event->modifiers() & Qt::ControlModifier) && (event->key() == Qt::Key_1)) {
		changeZoom(zoomOriginAction);
	} else if (event->key() == 32) {
		// space key - hide/show the current object
		auto* we = dynamic_cast<WorksheetElement*>(aspect);
		if (we)
			we->setVisible(!we->isVisible());
	} else if (aspect->type() == AspectType::CartesianPlot && m_worksheet->layout() != Worksheet::Layout::NoLayout) {
		// use the arrow keys to navigate only if a layout is active in the worksheet.
		// without any layout the arrow keys are used to move the plot within the worksheet
		if (event->key() == Qt::Key_Left)
			changePlotNavigation(plotShiftRightXAction);
		else if (event->key() == Qt::Key_Right)
			changePlotNavigation(plotShiftLeftXAction);
		else if (event->key() == Qt::Key_Up)
			changePlotNavigation(plotShiftDownYAction);
		else if (event->key() == Qt::Key_Down)
			changePlotNavigation(plotShiftUpYAction);
	}

	QGraphicsView::keyPressEvent(event);
}

void WorksheetView::keyReleaseEvent(QKeyEvent* event) {
	QGraphicsView::keyReleaseEvent(event);
}

void WorksheetView::dragEnterEvent(QDragEnterEvent* event) {
#ifndef SDK
	// ignore events not related to internal drags of columns etc., e.g. dropping of external files onto LabPlot
	const auto* mimeData = event->mimeData();
	if (!mimeData) {
		event->ignore();
		return;
	}

	if (mimeData->formats().at(0) != QLatin1String("labplot-dnd")) {
		event->ignore();
		return;
	}

	event->setAccepted(true);
#else
	Q_UNUSED(event)
#endif
}

void WorksheetView::dragMoveEvent(QDragMoveEvent* event) {
	// only accept drop events if we have a plot under the cursor where we can drop columns onto
	bool plot = (plotAt(event->position().toPoint()) != nullptr);
	event->setAccepted(plot);
}

void WorksheetView::dropEvent(QDropEvent* event) {
	auto* plot = plotAt(event->position().toPoint());
	if (!plot)
		return;

	const auto* mimeData = event->mimeData();
	plot->processDropEvent(plot->project()->droppedAspects(mimeData));

	// select the worksheet in the project explorer and bring the view to the foreground
	m_worksheet->setSelectedInView(true); // FIXME: doesn't work
}

// ##############################################################################
// ####################################  SLOTs   ################################
// ##############################################################################
void WorksheetView::useViewSizeChanged(bool useViewSize) {
	if (!m_actionsInitialized)
		initActions();

	updateScrollBarPolicy();

	if (useViewSize) {
		zoomFitPageHeightAction->setVisible(false);
		zoomFitPageWidthAction->setVisible(false);
		this->processResize(); // determine and set the current view size
	} else {
		zoomFitPageHeightAction->setVisible(true);
		zoomFitPageWidthAction->setVisible(true);
	}
}

void WorksheetView::processResize() {
	const auto* win = window();
	if (win) {
		const auto* handle = win->windowHandle();
		if (handle) {
			const auto* screen = handle->screen();
			if (screen) {
				if (size() != sceneRect().size()) {
					static const float hscale = screen->physicalDotsPerInchX() / (Worksheet::convertToSceneUnits(1, Worksheet::Unit::Inch));
					static const float vscale = screen->physicalDotsPerInchY() / (Worksheet::convertToSceneUnits(1, Worksheet::Unit::Inch));

					m_worksheet->setUndoAware(false);
					m_worksheet->setPageRect(QRectF(0.0, 0.0, width() / hscale, height() / vscale));
					m_worksheet->setUndoAware(true);
				}
			} else
				qWarning() << "Screen is null.";

		} else
			qWarning() << "Window handle is null.";

	} else
		qWarning() << "Window is null.";
}

void WorksheetView::changeZoom(QAction* action) {
	zoomFitNoneAction->setChecked(true);
	m_worksheet->setZoomFit(Worksheet::ZoomFit::None);

	m_zoomMode = static_cast<ZoomMode>(action->data().toInt());
	switch (m_zoomMode) {
	case ZoomMode::ZoomIn:
		zoom(1);
		break;
	case ZoomMode::ZoomOut:
		zoom(-1);
		break;
	case ZoomMode::ZoomOrigin: {
		static const float hscale = GuiTools::dpi(this).first / (Worksheet::convertToSceneUnits(1, Worksheet::Unit::Inch));
		static const float vscale = GuiTools::dpi(this).second / (Worksheet::convertToSceneUnits(1, Worksheet::Unit::Inch));
		setTransform(QTransform::fromScale(hscale, vscale));
	}
	}

	updateLabelsZoom();
}

WorksheetView::ZoomMode WorksheetView::zoomMode() const {
	return m_zoomMode;
}

void WorksheetView::updateFit() {
	switch (m_worksheet->zoomFit()) {
	case Worksheet::ZoomFit::None:
		break;
	case Worksheet::ZoomFit::FitToWidth: {
		const float scaleFactor = viewport()->width() / scene()->sceneRect().width();
		setTransform(QTransform::fromScale(scaleFactor, scaleFactor));
		break;
	}
	case Worksheet::ZoomFit::FitToHeight: {
		const float scaleFactor = viewport()->height() / scene()->sceneRect().height();
		setTransform(QTransform::fromScale(scaleFactor, scaleFactor));
		break;
	}
	case Worksheet::ZoomFit::FitToSelection: {
		fitInView(scene()->selectionArea().boundingRect(), Qt::KeepAspectRatio);
		break;
	}
	case Worksheet::ZoomFit::Fit: {
		const float scaleFactorVertical = viewport()->height() / scene()->sceneRect().height();
		const float scaleFactorHorizontal = viewport()->width() / scene()->sceneRect().width();
		if (scaleFactorVertical * scene()->sceneRect().width() < viewport()->width())
			setTransform(QTransform::fromScale(scaleFactorVertical, scaleFactorVertical));
		else
			setTransform(QTransform::fromScale(scaleFactorHorizontal, scaleFactorHorizontal));
		break;
	}
	}
}

void WorksheetView::updateScrollBarPolicy() {
	if (m_worksheet->useViewSize() || m_worksheet->zoomFit() != Worksheet::ZoomFit::None) {
		setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
		setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	} else {
		setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
		setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
	}
}

void WorksheetView::changeZoomFit(QAction* action) {
	m_worksheet->setZoomFit(static_cast<Worksheet::ZoomFit>(action->data().toInt()));
	updateScrollBarPolicy();
	updateFit();
}

double WorksheetView::zoomFactor() const {
	double scale = transform().m11();
	const auto* win = window();
	if (win) {
		const auto* handle = win->windowHandle();
		if (handle) {
			const auto* screen = handle->screen();
			if (screen) {
				scale *= Worksheet::convertToSceneUnits(1, Worksheet::Unit::Inch) / screen->physicalDotsPerInchX();
			} else
				qWarning() << "Screen is null.";
		} else
			qWarning() << "Window handle is null.";

	} else
		qWarning() << "Window is null.";

	return scale; // Return the computed scale
}

void WorksheetView::updateLabelsZoom() const {
	const double zoom = zoomFactor();
	const auto& labels = m_worksheet->children<TextLabel>(AbstractAspect::ChildIndexFlag::Recursive | AbstractAspect::ChildIndexFlag::IncludeHidden);
	for (auto* label : labels)
		label->setZoomFactor(zoom);
}

void WorksheetView::changeMagnification(QAction* action) {
	m_magnificationFactor = action->data().toInt();
	if (m_magnificationFactor == 1 && m_magnificationWindow)
		m_magnificationWindow->setVisible(false);
}

int WorksheetView::magnification() const {
	return m_magnificationFactor;
}

void WorksheetView::changeMouseMode(QAction* action) {
	m_mouseMode = static_cast<MouseMode>(action->data().toInt());

	switch (m_mouseMode) {
	case MouseMode::Selection:
		setInteractive(true);
		setDragMode(QGraphicsView::NoDrag);
		break;
	case MouseMode::Navigation:
		setInteractive(false);
		setDragMode(QGraphicsView::ScrollHandDrag);
		break;
	case MouseMode::ZoomSelection:
		setInteractive(false);
		setDragMode(QGraphicsView::NoDrag);
		break;
	}
}

WorksheetView::MouseMode WorksheetView::mouseMode() const {
	return m_mouseMode;
}

//"Add new" related slots
void WorksheetView::addNew(QAction* action) {
	m_addNewMode = static_cast<AddNewMode>(action->data().toInt());
	bool restorePointers = false;
	WorksheetElement* aspect = nullptr;
	switch (m_addNewMode) {
	case AddNewMode::PlotAreaFourAxes:{
		auto* plot = new CartesianPlot(i18n("Plot Area"));
		plot->setType(CartesianPlot::Type::FourAxes);
		plot->setMouseMode(m_cartesianPlotMouseMode);
		aspect = plot;
		if (tbNewPlot)
			tbNewPlot->setDefaultAction(addCartesianPlot1Action);
		break;
	}
	case AddNewMode::PlotAreaTwoAxes: {
		auto* plot = new CartesianPlot(i18n("Plot Area"));
		plot->setType(CartesianPlot::Type::TwoAxes);
		plot->setMouseMode(m_cartesianPlotMouseMode);
		aspect = plot;
		if (tbNewPlot)
			tbNewPlot->setDefaultAction(addCartesianPlot2Action);
		break;
	}
	case AddNewMode::PlotAreaTwoAxesCentered: {
		auto* plot = new CartesianPlot(i18n("Plot Area"));
		plot->setType(CartesianPlot::Type::TwoAxesCentered);
		plot->setMouseMode(m_cartesianPlotMouseMode);
		aspect = plot;
		if (tbNewPlot)
			tbNewPlot->setDefaultAction(addCartesianPlot3Action);
		break;
	}
	case AddNewMode::PlotAreaTwoAxesCenteredZero: {
		auto* plot = new CartesianPlot(i18n("Plot Area"));
		plot->setType(CartesianPlot::Type::TwoAxesCenteredZero);
		plot->setMouseMode(m_cartesianPlotMouseMode);
		aspect = plot;
		if (tbNewPlot)
			tbNewPlot->setDefaultAction(addCartesianPlot4Action);
		break;
	}
	case AddNewMode::PlotAreaFromTemplate: {
#ifndef SDK
		// open dialog
		PlotTemplateDialog d;
		if (d.exec() != QDialog::Accepted)
			return;

		auto* plot = d.generatePlot();
		if (!plot)
			return;

		restorePointers = true;
		aspect = plot;
		if (tbNewPlot)
			tbNewPlot->setDefaultAction(addCartesianPlotTemplateAction);
#endif
		break;
	}
	case AddNewMode::TextLabel: {
		auto* l = new TextLabel(i18n("Text Label"));
		l->setText(i18n("Text Label"));
		aspect = l;
		break;
	}
	case AddNewMode::Image: {
		Image* image = new Image(i18n("Image"));
		aspect = image;
		break;
	}
	case AddNewMode::Plot3D: {
		auto* plot = new Plot3DArea(i18n("Plot 3D Area"));
		plot->init();
		aspect = plot;
		break;
	}
	}

	if (!aspect)
		return;

	m_worksheet->addChild(aspect);

	if (restorePointers) {
		m_worksheet->project()->restorePointers(m_worksheet);
		m_worksheet->project()->retransformElements(m_worksheet);
	}

	// labels and images with their initial positions need to be retransformed
	// after they have gotten a parent
	if (aspect->type() == AspectType::TextLabel || aspect->type() == AspectType::Image) {
		if (m_calledFromContextMenu) {
			// must be done after add Child, because otherwise the parentData rect is not available
			// and therefore aligning will not work
			auto position = aspect->position();
			position.point = aspect->parentPosToRelativePos(m_cursorPos, position);
			position.point =
				aspect->align(position.point, aspect->graphicsItem()->boundingRect(), aspect->horizontalAlignment(), aspect->verticalAlignment(), false);
			aspect->setPosition(position);
			m_calledFromContextMenu = false;
		} else
			aspect->retransform();
	} else if (aspect->type() == AspectType::CartesianPlot)
		static_cast<CartesianPlot*>(aspect)->retransform();

	updateCartesianPlotActions();

	// fade-in the newly added element.
	// TODO: don't do any fade-in for text labels - when a text label is added
	// after new curves were created via PlotDataDialog, the undo or delete steps for this label
	// lead to a crash in the handling of graphics effects in Qt. The root cause for the crash
	// is not completely clear (maybe related ot its child ScaledTextItem) so we deactivate
	// this effect completely for text labels, s.a. BUG: 455096.
	if (aspect->type() == AspectType::TextLabel)
		return;

	if (!m_fadeInTimeLine) {
		m_fadeInTimeLine = new QTimeLine(1000, this);
		m_fadeInTimeLine->setFrameRange(0, 100);
		connect(m_fadeInTimeLine, &QTimeLine::valueChanged, this, &WorksheetView::fadeIn);
	}

	// if there is already an element fading in, stop the time line and show the element with the full opacity.
	if (m_fadeInTimeLine->state() == QTimeLine::Running) {
		m_fadeInTimeLine->stop();
		auto* effect = new QGraphicsOpacityEffect(this);
		effect->setOpacity(1);
		lastAddedWorksheetElement->graphicsItem()->setGraphicsEffect(effect);
	}

	// create the opacity effect and start the actual fade-in
	lastAddedWorksheetElement = aspect;
	auto* effect = new QGraphicsOpacityEffect(this);
	effect->setOpacity(0);
	lastAddedWorksheetElement->graphicsItem()->setGraphicsEffect(effect);
	m_fadeInTimeLine->start();
}

WorksheetView::AddNewMode WorksheetView::addNewMode() const {
	return m_addNewMode;
}

/*!
 * select all top-level items
 */
void WorksheetView::selectAllElements() {
	// deselect all previously selected items since there can be some non top-level items belong them
	m_suppressSelectionChangedEvent = true;
	for (auto* item : m_selectedItems)
		m_worksheet->setItemSelectedInView(item, false);

	// select top-level items
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

	auto status = KMessageBox::warningTwoActions(
		this,
		i18np("Do you really want to delete the selected object?", "Do you really want to delete the selected %1 objects?", m_selectedItems.size()),
		i18np("Delete selected object", "Delete selected objects", m_selectedItems.size()),
		KStandardGuiItem::del(),
		KStandardGuiItem::cancel());
	if (status == KMessageBox::SecondaryAction)
		return;

	m_suppressSelectionChangedEvent = true;
	m_worksheet->beginMacro(i18n("%1: Remove selected worksheet elements.", m_worksheet->name()));
	for (auto* item : m_selectedItems)
		m_worksheet->deleteAspectFromGraphicsItem(item);
	m_selectedElement = nullptr;
	m_worksheet->endMacro();
	m_suppressSelectionChangedEvent = false;
}

void WorksheetView::aspectAboutToBeRemoved(const AbstractAspect* /* aspect */) {
	/*
		lastAddedWorksheetElement = dynamic_cast<WorksheetElement*>(const_cast<AbstractAspect*>(aspect));
		if (!lastAddedWorksheetElement)
			return;
	*/
	// FIXME: fading-out doesn't work
	// also, the following code collides with undo/redo of the deletion
	// of a worksheet element (after redoing the element is not shown with the full opacity
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
	auto* effect = static_cast<QGraphicsOpacityEffect*>(lastAddedWorksheetElement->graphicsItem()->graphicsEffect());
	effect->setOpacity(value);
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
void WorksheetView::changeLayout(QAction* action) const {
	const auto layout = static_cast<Worksheet::Layout>(action->data().toInt());
	m_worksheet->setLayout(layout);
}

Worksheet::Layout WorksheetView::layout() const {
	return m_worksheet->layout();
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
#ifndef SDK
		auto* dlg = new GridDialog(this);
		if (dlg->exec() == QDialog::Accepted)
			dlg->save(m_gridSettings);
		else
#endif
		return;
	}

	if (m_gridSettings.style == GridStyle::NoGrid)
		snapToGridAction->setEnabled(false);
	else
		snapToGridAction->setEnabled(true);

	invalidateScene(sceneRect(), QGraphicsScene::BackgroundLayer);
}

// TODO
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
	m_selectedItems << item;
	updateCartesianPlotActions();
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
	updateCartesianPlotActions();
	m_suppressSelectionChangedEvent = false;
}

/*!
 *  Called on selection changes in the view.
 *   Determines which items were selected and deselected
 *  and forwards these changes to \c Worksheet
 */
void WorksheetView::selectionChanged() {
	// if the project is being closed, the scene items are being removed and the selection can change.
	// don't react on these changes since this can lead crashes (worksheet object is already in the destructor).
	if (m_isClosing)
		return;

	if (m_suppressSelectionChangedEvent)
		return;

	const auto& items = scene()->selectedItems();

	// check, whether the previously selected items were deselected now.
	// Forward the deselection prior to the selection of new items
	// in order to avoid the unwanted multiple selection in project explorer
	for (auto* item : m_selectedItems) {
		if (items.indexOf(item) == -1)
			m_worksheet->setItemSelectedInView(item, false);
	}

	// select new items
	if (items.isEmpty()) {
		// no items selected -> select the worksheet again.
		m_worksheet->setSelectedInView(true);

		// if one of the "zoom&select" plot mouse modes was selected before, activate the default "selection mode" again
		// since no plots are selected now.
		if (m_mouseMode == MouseMode::Selection && m_cartesianPlotMouseMode != CartesianPlot::MouseMode::Selection) {
			plotSelectionModeAction->setChecked(true);
			changePlotMouseMode(plotSelectionModeAction);
		}
	} else {
		for (const auto* item : items)
			m_worksheet->setItemSelectedInView(item, true);

		// items selected -> deselect the worksheet in the project explorer
		// prevents unwanted multiple selection with worksheet (if it was selected before)
		m_worksheet->setSelectedInView(false);
	}

	m_selectedItems = std::move(items);
	updateCartesianPlotActions();
}

void WorksheetView::handleCartesianPlotSelected(const CartesianPlot* plot, const QActionGroup* mouseModeActionGroup, const QActionGroup* navigationActionGroup) {
	/* Action to All: action is applied to all ranges in all dimensions
	 *	- Applied to all plots and all ranges
	 * Action to X: action is applied to all x ranges
	 *	- x zoom selection: zooming into all x ranges of all plots (Normally all plots will have the same x ranges so it makes sense
	 *  - y zoom selection: makes no sense. disable
	 * Action to Y: action is applied to all y ranges
	 *  - x zoom selection: makes no sense. disable
	 *  - y zoom selection: zooming into all y ranges of all plots
	 * Action to Selection
	 * - x zoom selection: makes no sense, because the range is unknown, disable
	 * - y zoom selection: makes no sense, because the range is unknown, disable
	 *		- What happens when only one range is available?
	 */

	switch (m_worksheet->cartesianPlotActionMode()) {
	case Worksheet::CartesianPlotActionMode::ApplyActionToAll:
		// mouse mode actions
		for (auto* action : mouseModeActionGroup->actions()) {
			const auto mode = static_cast<CartesianPlot::MouseMode>(action->data().toInt());
			if (mode == CartesianPlot::MouseMode::ZoomSelection || mode == CartesianPlot::MouseMode::ZoomXSelection
				|| mode == CartesianPlot::MouseMode::ZoomYSelection)
				action->setEnabled(true);
		}

		// navigation actions
		for (auto* action : navigationActionGroup->actions())
			action->setEnabled(true);
		break;
	case Worksheet::CartesianPlotActionMode::ApplyActionToSelection: {
		// mouse mode actions, enable only when only one range available
		bool enableX = plot->rangeCount(Dimension::X) == 1;
		bool enableY = plot->rangeCount(Dimension::Y) == 1;
		for (auto* action : mouseModeActionGroup->actions()) {
			const auto mode = static_cast<CartesianPlot::MouseMode>(action->data().toInt());
			if (mode == CartesianPlot::MouseMode::ZoomSelection)
				action->setEnabled(enableX && enableY);
			else if (mode == CartesianPlot::MouseMode::ZoomXSelection)
				action->setEnabled(enableX);
			else if (mode == CartesianPlot::MouseMode::ZoomYSelection)
				action->setEnabled(enableY);
		}

		// navigation actions
		for (auto* action : navigationActionGroup->actions())
			action->setEnabled(true);
		break;
	}
	case Worksheet::CartesianPlotActionMode::ApplyActionToAllX:
		// mouse mode actions
		for (auto* action : mouseModeActionGroup->actions()) {
			const auto mode = static_cast<CartesianPlot::MouseMode>(action->data().toInt());
			if (mode == CartesianPlot::MouseMode::ZoomSelection)
				action->setEnabled(false);
			else if (mode == CartesianPlot::MouseMode::ZoomXSelection || mode == CartesianPlot::MouseMode::ZoomYSelection)
				action->setEnabled(true);
		}

		// navigation actions
		for (auto* action : navigationActionGroup->actions()) {
			const auto op = static_cast<CartesianPlot::NavigationOperation>(action->data().toInt());
			const bool x = (op == CartesianPlot::NavigationOperation::ZoomInX ||op == CartesianPlot::NavigationOperation::ZoomOutX
				||  op == CartesianPlot::NavigationOperation::ShiftLeftX ||  op == CartesianPlot::NavigationOperation::ShiftRightX
				||  op == CartesianPlot::NavigationOperation::ScaleAutoX);
			action->setEnabled(x);
		}
		break;
	case Worksheet::CartesianPlotActionMode::ApplyActionToAllY:
		// mouse mode actions
		for (auto* action : mouseModeActionGroup->actions()) {
			const auto mode = static_cast<CartesianPlot::MouseMode>(action->data().toInt());
			if (mode == CartesianPlot::MouseMode::ZoomSelection)
				action->setEnabled(false);
			else if (mode == CartesianPlot::MouseMode::ZoomXSelection || mode == CartesianPlot::MouseMode::ZoomYSelection)
				action->setEnabled(true);
		}

		// navigation actions
		for (auto* action : navigationActionGroup->actions()) {
			const auto op = static_cast<CartesianPlot::NavigationOperation>(action->data().toInt());
			const bool y = (op == CartesianPlot::NavigationOperation::ZoomInY ||op == CartesianPlot::NavigationOperation::ZoomOutY
				||  op == CartesianPlot::NavigationOperation::ShiftUpY ||  op == CartesianPlot::NavigationOperation::ShiftDownY
				||  op == CartesianPlot::NavigationOperation::ScaleAutoY);
			action->setEnabled(y);
		}
		break;
	}
}

void WorksheetView::handleReferences(WorksheetElement::Orientation orientation, const QActionGroup* mouseModeActionGroup, const QActionGroup* navigationActionGroup) {
	/*
	 *  Action to All: action is applied to all ranges
	 *	- x zoom selection: if vertical:Zooming into all ranges of all plots (mostly the x ranges are the same for all plots --> usecase)
	 *  - y zoom selection: if !vertical: Zooming into all ranges of all plots
	 * Action to X: action is applied to all x ranges
	 *	- x zoom selection: if vertical:Zooming into all ranges of all plots (mostly the x ranges are the same for all plots --> usecase)
	 *  - y zoom selection: if !vertical:zoom only into the range from the reference line
	 * Action to Y: action is applied to all y ranges
	 *  - x zoom selection: if vertical: zoom only into the range from the reference line
	 *  - y zoom selection: if !vertical: Zooming into all ranges of all plots
	 * Action to Selection
	 * - x zoom selection: if vertical: zoom only into the range from the reference line
	 * - y zoom selection: if !vertical:zoom only into the range from the reference line
	 */

	plotZoomInAction->setEnabled(false);
	plotZoomOutAction->setEnabled(false);
	plotZoomSelectionModeAction->setEnabled(false);
	plotScaleAutoAction->setEnabled(false);

	const bool vertical = (orientation == WorksheetElement::Orientation::Vertical);

	// mouse mode actions
	for (auto* action : mouseModeActionGroup->actions()) {
		const auto mode = static_cast<CartesianPlot::MouseMode>(action->data().toInt());
		if (mode == CartesianPlot::MouseMode::ZoomXSelection)
			action->setEnabled(vertical);
		else if (mode == CartesianPlot::MouseMode::ZoomYSelection)
			action->setEnabled(!vertical);
		else if (mode == CartesianPlot::MouseMode::ZoomSelection)
			action->setEnabled(false);
	}

	// navigation actions
	for (auto* action : navigationActionGroup->actions()) {
		const auto op = static_cast<CartesianPlot::NavigationOperation>(action->data().toInt());
		const bool x = (op == CartesianPlot::NavigationOperation::ZoomInX ||op == CartesianPlot::NavigationOperation::ZoomOutX
						||  op == CartesianPlot::NavigationOperation::ShiftLeftX ||  op == CartesianPlot::NavigationOperation::ShiftRightX
						||  op == CartesianPlot::NavigationOperation::ScaleAutoX);
		const bool y = (op == CartesianPlot::NavigationOperation::ZoomInY ||op == CartesianPlot::NavigationOperation::ZoomOutY
						||  op == CartesianPlot::NavigationOperation::ShiftDownY ||  op == CartesianPlot::NavigationOperation::ShiftUpY
						||  op == CartesianPlot::NavigationOperation::ScaleAutoY);
		action->setEnabled((x && vertical) || (y && !vertical));
	}
}

void WorksheetView::handlePlotSelected(const QActionGroup* mouseModeActionGroup, const QActionGroup* navigationActionGroup) {
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

	switch (m_worksheet->cartesianPlotActionMode()) {
	case Worksheet::CartesianPlotActionMode::ApplyActionToAll:
		// mouse mode actions
		for (auto* action : mouseModeActionGroup->actions()) {
			const auto mode = static_cast<CartesianPlot::MouseMode>(action->data().toInt());
			if (mode == CartesianPlot::MouseMode::ZoomSelection || mode == CartesianPlot::MouseMode::ZoomXSelection
				|| mode == CartesianPlot::MouseMode::ZoomYSelection)
				action->setEnabled(false);
		}

		// navigation actions
		for (auto* action : navigationActionGroup->actions()) {
			const auto op = static_cast<CartesianPlot::NavigationOperation>(action->data().toInt());
			const bool scaleAuto = (op == CartesianPlot::NavigationOperation::ScaleAuto || op == CartesianPlot::NavigationOperation::ScaleAutoX
									|| op == CartesianPlot::NavigationOperation::ScaleAutoY);
			action->setEnabled(scaleAuto);
		}
		break;
	case Worksheet::CartesianPlotActionMode::ApplyActionToSelection:
		// mouse mode actions
		for (auto* action : mouseModeActionGroup->actions()) {
			const auto mode = static_cast<CartesianPlot::MouseMode>(action->data().toInt());
			if (mode == CartesianPlot::MouseMode::ZoomSelection || mode == CartesianPlot::MouseMode::ZoomXSelection
				|| mode == CartesianPlot::MouseMode::ZoomYSelection)
				action->setEnabled(true);
		}

		// navigation actions
		for (auto* action : navigationActionGroup->actions())
			action->setEnabled(true);
		break;
	case Worksheet::CartesianPlotActionMode::ApplyActionToAllX:
		// mouse mode actions
		for (auto* action : mouseModeActionGroup->actions()) {
			const auto mode = static_cast<CartesianPlot::MouseMode>(action->data().toInt());
			if (mode == CartesianPlot::MouseMode::ZoomSelection)
				action->setEnabled(false);
			else if (mode == CartesianPlot::MouseMode::ZoomXSelection || mode == CartesianPlot::MouseMode::ZoomYSelection)
				action->setEnabled(true);
		}

		// navigation actions
		for (auto* action : navigationActionGroup->actions()) {
			const auto op = static_cast<CartesianPlot::NavigationOperation>(action->data().toInt());
			const bool zoomInOut = (op == CartesianPlot::NavigationOperation::ZoomIn || op == CartesianPlot::NavigationOperation::ZoomOut);
			action->setEnabled(!zoomInOut);
		}
		break;
	case Worksheet::CartesianPlotActionMode::ApplyActionToAllY:
		// mouse mode actions
		for (auto* action : mouseModeActionGroup->actions()) {
			const auto mode = static_cast<CartesianPlot::MouseMode>(action->data().toInt());
			if (mode == CartesianPlot::MouseMode::ZoomSelection)
				action->setEnabled(false);
			else if (mode == CartesianPlot::MouseMode::ZoomXSelection || mode == CartesianPlot::MouseMode::ZoomYSelection)
				action->setEnabled(true);
		}

		// navigation actions
		// TODO: same as for ApplyActionToAllX above?
		for (auto* action : navigationActionGroup->actions()) {
			const auto op = static_cast<CartesianPlot::NavigationOperation>(action->data().toInt());
			const bool zoomInOut = (op == CartesianPlot::NavigationOperation::ZoomIn || op == CartesianPlot::NavigationOperation::ZoomOut);
			action->setEnabled(!zoomInOut);
		}
		break;
	}
}

void WorksheetView::handleAxisSelected(const Axis* a, const QActionGroup* mouseModeActionGroup, const QActionGroup* navigationActionGroup) {
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

		// mouse mode actions
		for (auto* action : mouseModeActionGroup->actions()) {
			const auto mode = static_cast<CartesianPlot::MouseMode>(action->data().toInt());
			if (mode == CartesianPlot::MouseMode::ZoomXSelection)
				action->setEnabled(true);
			else if (mode == CartesianPlot::MouseMode::ZoomSelection || mode == CartesianPlot::MouseMode::ZoomYSelection)
				action->setEnabled(false);
		}

		// navigation actions
		for (auto* action : navigationActionGroup->actions()) {
			const auto op = static_cast<CartesianPlot::NavigationOperation>(action->data().toInt());
			const bool x = (op == CartesianPlot::NavigationOperation::ZoomInX ||op == CartesianPlot::NavigationOperation::ZoomOutX
				||  op == CartesianPlot::NavigationOperation::ShiftLeftX ||  op == CartesianPlot::NavigationOperation::ShiftRightX
				||  op == CartesianPlot::NavigationOperation::ScaleAutoX);
			action->setEnabled(x);
		}
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

		// mouse mode actions
		for (auto* action : mouseModeActionGroup->actions()) {
			const auto mode = static_cast<CartesianPlot::MouseMode>(action->data().toInt());
			if (mode == CartesianPlot::MouseMode::ZoomYSelection)
				action->setEnabled(true);
			else if (mode == CartesianPlot::MouseMode::ZoomSelection || mode == CartesianPlot::MouseMode::ZoomXSelection)
				action->setEnabled(false);
		}

		// navigation actions
		for (auto* action : navigationActionGroup->actions()) {
			const auto op = static_cast<CartesianPlot::NavigationOperation>(action->data().toInt());
			const bool y = (op == CartesianPlot::NavigationOperation::ZoomInY ||op == CartesianPlot::NavigationOperation::ZoomOutY
				||  op == CartesianPlot::NavigationOperation::ShiftUpY ||  op == CartesianPlot::NavigationOperation::ShiftDownY
				||  op == CartesianPlot::NavigationOperation::ScaleAutoY);
			action->setEnabled(y);
		}
	}
}

/*!
 * register external action groups for plot related mouse mode and navigation actions to update them
 * on selection changes in the view, used for the actions shown in the main window and managed by \c ActionManager.
 */
void WorksheetView::registerCartesianPlotActions(QActionGroup* mouseModeActionGroup, QActionGroup* navigationActionGroup) {
	m_plotMouseModeActionGroupExternal = mouseModeActionGroup;
	m_plotNavigationActionGroupExternal = navigationActionGroup;
	updateCartesianPlotActions(m_plotMouseModeActionGroupExternal, m_plotNavigationActionGroupExternal);
}

/*!
* updates the state of the plot related actions, called on selection changes in the view and when the mouse mode is changed.
* The actions are enabled/disabled depending on the selected items (plots, axes, reference lines/ranges).
*/
void WorksheetView::updateCartesianPlotActions() {
	// update the internal actions shown in the context menu
	if (m_plotMouseModeActionGroup && m_plotNavigationActionGroup)
		updateCartesianPlotActions(m_plotMouseModeActionGroup, m_plotNavigationActionGroup);

	// update the external actions, if available, shown in the main window toolbar and managed by ActionManager
	if (m_plotMouseModeActionGroupExternal && m_plotNavigationActionGroupExternal)
		updateCartesianPlotActions(m_plotMouseModeActionGroupExternal, m_plotNavigationActionGroupExternal);
}

void WorksheetView::updateCartesianPlotActions(const QActionGroup* mouseModeActionGroup, const QActionGroup* navigationActionGroup) {
	if (m_mouseMode != MouseMode::Selection)
		return; // Do not change selection when the mousemode is not selection!

	m_selectedElement = nullptr;

	bool handled = false;
	for (auto* item : m_selectedItems) {
		// TODO: or if a children of a plot is selected
		auto* w = static_cast<WorksheetElementPrivate*>(item)->q;
		if (w->type() == AspectType::CartesianPlot) {
			handled = true;
			m_selectedElement = w;
			handleCartesianPlotSelected(static_cast<CartesianPlot*>(m_selectedElement), mouseModeActionGroup, navigationActionGroup);
			break;
		} else if (w->type() == AspectType::ReferenceLine) {
			handled = true;
			m_selectedElement = w;
			const auto orientation = static_cast<ReferenceLine*>(w)->orientation();
			handleReferences(orientation, mouseModeActionGroup, navigationActionGroup);
		} else if (w->type() == AspectType::ReferenceRange) {
			handled = true;
			m_selectedElement = w;
			const auto orientation = static_cast<ReferenceRange*>(w)->orientation();
			handleReferences(orientation, mouseModeActionGroup, navigationActionGroup);
		} else if (w->type() == AspectType::Axis) {
			handled = true;
			m_selectedElement = w;
			handleAxisSelected(static_cast<Axis*>(m_selectedElement), mouseModeActionGroup, navigationActionGroup);
			break;
		} else if (dynamic_cast<Plot*>(w) || w->coordinateBindingEnabled()) {
			// Plot and other WorksheetElements like custompoint, infoelement, textlabel
			handled = true;
			m_selectedElement = w;
			handlePlotSelected(mouseModeActionGroup, navigationActionGroup);
			break;
		}
	}

	// activate additionally Selection, Crossshair and Cursor actions.
	// if no elements are selected where navigation makes sense and that were handled above,
	// disable all navigation actions
	if (handled) {
		// mouse mode actions
		for (auto* action : mouseModeActionGroup->actions()) {
			const auto mode = static_cast<CartesianPlot::MouseMode>(action->data().toInt());
			if (mode == CartesianPlot::MouseMode::Selection || mode == CartesianPlot::MouseMode::Crosshair)
				action->setEnabled(true);
			else if (mode == CartesianPlot::MouseMode::Cursor)
				action->setEnabled(m_selectedElement->type() == AspectType::CartesianPlot);
		}
	} else {
		// mouse mode actions
		for (auto* action : mouseModeActionGroup->actions()) {
			action->setEnabled(false);
			action->setChecked(false);
		}
		for (auto* p : m_worksheet->children<CartesianPlot>())
			p->setMouseMode(CartesianPlot::MouseMode::Selection);

		// navigation actions
		for (auto* action : navigationActionGroup->actions())
			action->setEnabled(false);
	}
}

bool WorksheetView::exportToFile(const QString& path,
								 const Worksheet::ExportFormat format,
								 const Worksheet::ExportArea area,
								 const bool background,
								 const int resolution) {
	PERFTRACE(QLatin1String(Q_FUNC_INFO));
	bool rc = false;
	QRectF sourceRect;

	if (area == Worksheet::ExportArea::BoundingBox) {
		sourceRect = scene()->itemsBoundingRect();
		sourceRect = QRect(0, 0, sourceRect.width() + sourceRect.x(), sourceRect.height());
	} else if (area == Worksheet::ExportArea::Selection) {
		if (!m_selectedItems.isEmpty()) {
			// Union the bounding rectangles of selected items
			for (const auto* item : m_selectedItems) {
				QRectF itemRect = item->mapToScene(item->boundingRect()).boundingRect();
				sourceRect = sourceRect.united(itemRect);
			}
		} else
			sourceRect = scene()->sceneRect();
	} else
		sourceRect = scene()->sceneRect();
	switch (format) {
	case Worksheet::ExportFormat::PDF: {
		QPrinter printer;
		printer.setOutputFormat(QPrinter::PdfFormat);
		printer.setOutputFileName(path);
		int w = Worksheet::convertFromSceneUnits(sourceRect.width(), Worksheet::Unit::Millimeter);
		int h = Worksheet::convertFromSceneUnits(sourceRect.height(), Worksheet::Unit::Millimeter);
		printer.setPageSize(QPageSize(QSizeF(w, h), QPageSize::Millimeter));
		printer.setPageMargins(QMarginsF(0, 0, 0, 0), QPageLayout::Millimeter);
		printer.setPrintRange(QPrinter::PageRange);
		printer.setCreator(QStringLiteral("LabPlot ") + QLatin1String(LVERSION));

		QPainter painter;
		rc = painter.begin(&printer);
		if (!rc)
			return false;
		painter.setRenderHint(QPainter::Antialiasing);
		QRectF targetRect(0, 0, w, h);
		exportPaint(&painter, targetRect, sourceRect, background);
		painter.end();
		break;
	}
	case Worksheet::ExportFormat::SVG: {
#ifdef HAVE_QTSVG
		QSvgGenerator generator;
		generator.setFileName(path);
		int w = Worksheet::convertFromSceneUnits(sourceRect.width(), Worksheet::Unit::Millimeter);
		int h = Worksheet::convertFromSceneUnits(sourceRect.height(), Worksheet::Unit::Millimeter);
		// Adjust for DPI conversion
		w = w * GuiTools::dpi(this).first / (GSL_CONST_CGS_INCH * Worksheet::convertToSceneUnits(1, Worksheet::Unit::Millimeter));
		h = h * GuiTools::dpi(this).second / (GSL_CONST_CGS_INCH * Worksheet::convertToSceneUnits(1, Worksheet::Unit::Millimeter));

		generator.setSize(QSize(w, h));
		QRectF targetRect(0, 0, w, h);
		generator.setViewBox(targetRect);

		QPainter painter;
		rc = painter.begin(&generator);
		if (!rc)
			return false;
		exportPaint(&painter, targetRect, sourceRect, background);
		painter.end();
#endif
		break;
	}
	case Worksheet::ExportFormat::PNG:
	case Worksheet::ExportFormat::JPG:
	case Worksheet::ExportFormat::BMP:
	case Worksheet::ExportFormat::PPM:
	case Worksheet::ExportFormat::XBM:
	case Worksheet::ExportFormat::XPM: {
		int w = Worksheet::convertFromSceneUnits(sourceRect.width(), Worksheet::Unit::Millimeter);
		int h = Worksheet::convertFromSceneUnits(sourceRect.height(), Worksheet::Unit::Millimeter);
		w = w * resolution / (GSL_CONST_CGS_INCH * Worksheet::convertToSceneUnits(1, Worksheet::Unit::Millimeter));
		h = h * resolution / (GSL_CONST_CGS_INCH * Worksheet::convertToSceneUnits(1, Worksheet::Unit::Millimeter));
		QImage image(QSize(w, h), QImage::Format_ARGB32_Premultiplied);
		image.fill(Qt::transparent);
		QRectF targetRect(0, 0, w, h);

		QPainter painter;
		rc = painter.begin(&image);
		if (!rc)
			return false;
		painter.setRenderHint(QPainter::Antialiasing);
		painter.save();
		exportPaint(&painter, targetRect, sourceRect, background);
		painter.restore();
		painter.end();

		if (!path.isEmpty()) {
			switch (format) {
			case Worksheet::ExportFormat::PNG:
				rc = image.save(path, "PNG");
				break;
			case Worksheet::ExportFormat::JPG:
				rc = image.save(path, "JPG");
				break;
			case Worksheet::ExportFormat::BMP:
				rc = image.save(path, "BMP");
				break;
			case Worksheet::ExportFormat::PPM:
				rc = image.save(path, "PPM");
				break;
			case Worksheet::ExportFormat::XBM:
				rc = image.save(path, "XBM");
				break;
			case Worksheet::ExportFormat::XPM:
				rc = image.save(path, "XPM");
				break;
			case Worksheet::ExportFormat::PDF: // SVG and PDF handled earlier
			case Worksheet::ExportFormat::SVG:
				break;
			}
		} else {
			QApplication::clipboard()->setImage(image, QClipboard::Clipboard);
			rc = true;
		}
		break;
	}
	}

#ifndef SDK
	if (!rc) {
		RESET_CURSOR;
		QMessageBox::critical(nullptr, i18n("Failed to export"), i18n("Failed to write to '%1'. Please check the path.", path));
	}
#endif

	return rc;
}

void WorksheetView::exportToPixmap(QPixmap& pixmap) {
	const auto& sourceRect = scene()->sceneRect();

	int w = Worksheet::convertFromSceneUnits(sourceRect.width(), Worksheet::Unit::Millimeter);
	int h = Worksheet::convertFromSceneUnits(sourceRect.height(), Worksheet::Unit::Millimeter);
	w = w * GuiTools::dpi(this).first / (GSL_CONST_CGS_INCH * Worksheet::convertToSceneUnits(1, Worksheet::Unit::Millimeter));
	h = h * GuiTools::dpi(this).second / (GSL_CONST_CGS_INCH * Worksheet::convertToSceneUnits(1, Worksheet::Unit::Millimeter));

	pixmap = QPixmap(w, h);
	pixmap.fill(Qt::transparent);

	QRectF targetRect(0, 0, w, h);
	QPainter painter;
	painter.begin(&pixmap);
	painter.setRenderHint(QPainter::Antialiasing);

	exportPaint(&painter, targetRect, sourceRect, true /* export background */, true /* export selection */);

	painter.end();
}

bool WorksheetView::eventFilter(QObject* /*watched*/, QEvent* event) {
	if (event->type() == QEvent::KeyPress && m_actionsInitialized) {
		auto* keyEvent = static_cast<QKeyEvent*>(event);
		int key = keyEvent->key();
		switch (key) {
		case Qt::Key_S:
			if (plotSelectionModeAction->isEnabled())
				plotSelectionModeAction->trigger();
			return true;
		case Qt::Key_Z:
			if (plotZoomSelectionModeAction->isEnabled())
				plotZoomSelectionModeAction->trigger();
			return true;
		case Qt::Key_C:
			if (plotCursorModeAction->isEnabled())
				plotCursorModeAction->trigger();
			return true;
		case Qt::Key_Escape:
			if (plotSelectionModeAction->isEnabled())
				plotSelectionModeAction->trigger();
			return false; // so the plot can handle the event too
		default:
			return false;
		}
	}
	return false;
}

void WorksheetView::exportToClipboard() {
	QRectF sourceRect;

	if (m_selectedItems.size() == 0) {
		sourceRect = scene()->itemsBoundingRect();
		sourceRect = QRect(0, 0, sourceRect.width() + sourceRect.x(), sourceRect.height());
	} else {
		// export selection
		// Union the bounding rectangles of selected items
		for (const auto* item : m_selectedItems) {
			QRectF itemRect = item->mapToScene(item->boundingRect()).boundingRect();
			sourceRect = sourceRect.united(itemRect);
		}
	}
	int w = Worksheet::convertFromSceneUnits(sourceRect.width(), Worksheet::Unit::Millimeter);
	int h = Worksheet::convertFromSceneUnits(sourceRect.height(), Worksheet::Unit::Millimeter);
	// Calculate the width and height in pixels
	w = w * GuiTools::dpi(this).first / (GSL_CONST_CGS_INCH * Worksheet::convertToSceneUnits(1, Worksheet::Unit::Millimeter));
	h = h * GuiTools::dpi(this).second / (GSL_CONST_CGS_INCH * Worksheet::convertToSceneUnits(1, Worksheet::Unit::Millimeter));

	// Create an image with the calculated size
	QImage image(QSize(w, h), QImage::Format_ARGB32_Premultiplied);
	image.fill(Qt::transparent);
	QRectF targetRect(0, 0, w, h);

	// Use a QPainter to draw onto the image
	QPainter painter;
	painter.begin(&image);
	painter.setRenderHint(QPainter::Antialiasing);
	exportPaint(&painter, targetRect, sourceRect, true); // Export with background
	painter.end();
	// Set the image to the clipboard
	QApplication::clipboard()->setImage(image, QClipboard::Clipboard);
}

void WorksheetView::exportPaint(QPainter* painter, const QRectF& targetRect, const QRectF& sourceRect, const bool background, const bool selection) {
	// hide the magnification window, shouldn't be exported
	bool magnificationActive = false;
	if (m_magnificationWindow && m_magnificationWindow->isVisible()) {
		magnificationActive = true;
		m_magnificationWindow->setVisible(false);
	}

	// draw the background
	m_isPrinting = true;
	if (background) {
		painter->save();
		const qreal scaleX = targetRect.width() / sourceRect.width();
		const qreal scaleY = targetRect.height() / sourceRect.height();
		painter->scale(scaleX, scaleY);
		drawBackground(painter, targetRect);
		painter->restore();
	}

	// draw the scene items
	if (!selection) // if no selection effects have to be exported, set the printing flag to suppress it in the paint()'s of the children
		m_worksheet->setPrinting(true);
	scene()->render(painter, QRectF(), sourceRect, Qt::IgnoreAspectRatio);
	if (!selection)
		m_worksheet->setPrinting(false);
	m_isPrinting = false;

	// show the magnification window if it was active before
	if (magnificationActive)
		m_magnificationWindow->setVisible(true);
}

void WorksheetView::print(QPrinter* printer) {
	m_isPrinting = true;
	m_worksheet->setPrinting(true);
	bool magnificationActive = false;
	if (m_magnificationWindow && m_magnificationWindow->isVisible()) {
		magnificationActive = true;
		m_magnificationWindow->setVisible(false);
	}

	QPainter painter(printer);
	painter.setRenderHint(QPainter::Antialiasing);

	// draw background
	const auto& page_rect = printer->pageLayout().paintRectPixels(printer->resolution());
	const auto& scene_rect = scene()->sceneRect();
	float scale = std::max(scene_rect.width() / page_rect.width(), scene_rect.height() / page_rect.height());
	drawBackgroundItems(&painter, QRectF(0, 0, scene_rect.width() / scale, scene_rect.height() / scale));

	// draw scene
	scene()->render(&painter);
	m_worksheet->setPrinting(false);
	m_isPrinting = false;

	if (magnificationActive)
		m_magnificationWindow->setVisible(true);
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
	selectAllAction->setShortcut(Qt::CTRL | Qt::Key_A);
	deleteAction->setShortcut(Qt::Key_Delete);
	backspaceAction->setShortcut(Qt::Key_Backspace);
	zoomInViewAction->setShortcut(Qt::CTRL | Qt::Key_Plus);
	zoomOutViewAction->setShortcut(Qt::CTRL | Qt::Key_Minus);
	zoomOriginAction->setShortcut(Qt::CTRL | Qt::Key_1);
}

void WorksheetView::unregisterShortcuts() {
	selectAllAction->setShortcut(QKeySequence());
	deleteAction->setShortcut(QKeySequence());
	backspaceAction->setShortcut(QKeySequence());
	zoomInViewAction->setShortcut(QKeySequence());
	zoomOutViewAction->setShortcut(QKeySequence());
	zoomOriginAction->setShortcut(QKeySequence());
}

// ##############################################################################
// ########################  SLOTs for cartesian plots   ########################
// ##############################################################################
void WorksheetView::cartesianPlotActionModeChanged(QAction* action) {
	if (action == plotApplyToSelectionAction)
		m_worksheet->setCartesianPlotActionMode(Worksheet::CartesianPlotActionMode::ApplyActionToSelection);
	else if (action == plotApplyToAllXAction)
		m_worksheet->setCartesianPlotActionMode(Worksheet::CartesianPlotActionMode::ApplyActionToAllX);
	else if (action == plotApplyToAllYAction)
		m_worksheet->setCartesianPlotActionMode(Worksheet::CartesianPlotActionMode::ApplyActionToAllY);
	else
		m_worksheet->setCartesianPlotActionMode(Worksheet::CartesianPlotActionMode::ApplyActionToAll);

	updateCartesianPlotActions();
}

void WorksheetView::cartesianPlotCursorModeChanged(QAction* action) {
	if (action == plotApplyToSelectionCursor)
		m_worksheet->setCartesianPlotCursorMode(Worksheet::CartesianPlotActionMode::ApplyActionToSelection);
	else
		m_worksheet->setCartesianPlotCursorMode(Worksheet::CartesianPlotActionMode::ApplyActionToAll);

	updateCartesianPlotActions();
}

void WorksheetView::plotsInteractiveActionChanged(bool checked) {
	m_worksheet->setPlotsInteractive(checked);
}

void WorksheetView::changePlotMouseMode(QAction* action) {
	if (m_suppressMouseModeChange)
		return;

	m_cartesianPlotMouseMode = static_cast<CartesianPlot::MouseMode>(action->data().toInt());
	// TODO: find out, which element is selected. So the corresponding range can be modified

	for (auto* plot : m_worksheet->children<CartesianPlot>())
		plot->setMouseMode(m_cartesianPlotMouseMode);
}

void WorksheetView::cartesianPlotMouseModeChangedSlot(CartesianPlot::MouseMode mouseMode) {
	if (!m_menusInitialized)
		return;

	m_suppressMouseModeChange = true;
	if (mouseMode == CartesianPlot::MouseMode::Selection)
		plotSelectionModeAction->setChecked(true);
	else if (mouseMode == CartesianPlot::MouseMode::ZoomSelection)
		plotZoomSelectionModeAction->setChecked(true);
	else if (mouseMode == CartesianPlot::MouseMode::ZoomXSelection)
		plotZoomXSelectionModeAction->setChecked(true);
	else if (mouseMode == CartesianPlot::MouseMode::ZoomYSelection)
		plotZoomYSelectionModeAction->setChecked(true);
	else if (mouseMode == CartesianPlot::MouseMode::Cursor)
		plotCursorModeAction->setChecked(true);
	m_suppressMouseModeChange = false;
}

void WorksheetView::childContextMenuRequested(AspectType t, QMenu* menu) {
	if (!menu)
		return;
	if (t == AspectType::CartesianPlot) {
		// actions.at(0) is the menu title
		// actions.at(1) is the "new" menu
		// actions.at(2) is the separator
		menu->insertMenu(menu->actions().at(3), m_plotMouseModeMenu);
		menu->insertMenu(menu->actions().at(4), m_plotZoomMenu);
		menu->insertSeparator(menu->actions().at(5));
	}
	menu->exec(QCursor::pos());
}

void WorksheetView::changePlotNavigation(QAction* action) {
	// TODO: find out, which element was selected to find out which range should be changed
	// Project().projectExplorer().currentAspect()

	auto op = (CartesianPlot::NavigationOperation)action->data().toInt();
	auto plotActionMode = m_worksheet->cartesianPlotActionMode();
	if (plotActionMode == Worksheet::CartesianPlotActionMode::ApplyActionToSelection) {
		int cSystemIndex = CartesianPlot::cSystemIndex(m_selectedElement);
		const auto& plots = m_worksheet->children<CartesianPlot>();
		for (auto* plot : plots) {
			if (m_selectedItems.indexOf(plot->graphicsItem()) != -1)
				plot->navigate(cSystemIndex, op);
			else {
				// check if one of the plots children is selected. Do the operation there, too.
				for (auto* child : plot->children<WorksheetElement>()) {
					if (m_selectedItems.indexOf(child->graphicsItem()) != -1) {
						plot->navigate(cSystemIndex, op);
						break;
					}
				}
			}
		}
	} else if ((plotActionMode == Worksheet::CartesianPlotActionMode::ApplyActionToAllY
				&& (op == CartesianPlot::NavigationOperation::ScaleAutoX || op == CartesianPlot::NavigationOperation::ShiftLeftX
					|| op == CartesianPlot::NavigationOperation::ShiftRightX || op == CartesianPlot::NavigationOperation::ZoomInX
					|| op == CartesianPlot::NavigationOperation::ZoomOutX))
			   || (plotActionMode == Worksheet::CartesianPlotActionMode::ApplyActionToAllX
				   && (op == CartesianPlot::NavigationOperation::ScaleAutoY || op == CartesianPlot::NavigationOperation::ShiftUpY
					   || op == CartesianPlot::NavigationOperation::ShiftDownY || op == CartesianPlot::NavigationOperation::ZoomInY
					   || op == CartesianPlot::NavigationOperation::ZoomOutY))) {
		int cSystemIndex = CartesianPlot::cSystemIndex(m_selectedElement);
		if (m_selectedElement->type() == AspectType::CartesianPlot)
			static_cast<CartesianPlot*>(m_selectedElement)->navigate(-1, op);
		else {
			auto parentPlot = static_cast<CartesianPlot*>(m_selectedElement->parent(AspectType::CartesianPlot));
			if (parentPlot) // really needed?
				parentPlot->navigate(cSystemIndex, op);
		}
	} else {
		const auto& plots = m_worksheet->children<CartesianPlot>();
		for (auto* plot : plots)
			plot->navigate(-1, op);
	}
}

Worksheet::CartesianPlotActionMode WorksheetView::getCartesianPlotActionMode() const {
	return m_worksheet->cartesianPlotActionMode();
}

void WorksheetView::presenterMode() {
#ifndef SDK
	const auto& group = Settings::group(QStringLiteral("Settings_Worksheet"));
	const bool interactive = group.readEntry("PresenterModeInteractive", false);
	auto* presenterWidget = new PresenterWidget(m_worksheet, screen(), interactive);
	presenterWidget->showFullScreen();
#endif
}
