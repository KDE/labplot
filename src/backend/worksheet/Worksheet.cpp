/*
	File                 : Worksheet.cpp
	Project              : LabPlot
	Description          : Worksheet
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2009 Tilman Benkert <thzs@gmx.net>
	SPDX-FileCopyrightText: 2011-2022 Alexander Semke <alexander.semke@web.de>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "Worksheet.h"
#include "Background.h"
#include "WorksheetElement.h"
#include "WorksheetPrivate.h"
#include "backend/core/Project.h"
#include "backend/core/Settings.h"
#include "backend/lib/XmlStreamReader.h"
#include "backend/lib/commandtemplates.h"
#include "backend/worksheet/Image.h"
#include "backend/worksheet/Line.h"
#include "backend/worksheet/TextLabel.h"
#include "backend/worksheet/TreeModel.h"
#include "backend/worksheet/plots/cartesian/CartesianPlot.h"
#include "backend/worksheet/plots/cartesian/XYCurve.h"
#include "commonfrontend/worksheet/WorksheetView.h"
#include "kdefrontend/ThemeHandler.h"
#include "kdefrontend/worksheet/ExportWorksheetDialog.h"

#ifndef SDK
#include <QPrintDialog>
#include <QPrintPreviewDialog>
#include <QPrinter>
#endif

#include <QDir>
#include <QGraphicsItem>
#include <QIcon>
#include <QMenu>

#include <KConfig>
#include <KConfigGroup>
#include <KLocalizedString>

#include <backend/worksheet/plots/3d/Surface3DPlotArea.h>

/**
 * \class Worksheet
 * \brief Top-level container for worksheet elements like plot, labels, etc.
 *
 * The worksheet is, besides the data containers \c Spreadsheet and \c Matrix, another central part of the application
 * and provides an area for showing and grouping together different kinds of worksheet objects - plots, labels &etc;
 *
 * * \ingroup worksheet
 */
Worksheet::Worksheet(const QString& name, bool loading)
	: AbstractPart(name, AspectType::Worksheet)
	, d_ptr(new WorksheetPrivate(this)) {
	Q_D(Worksheet);
	d->background = new Background(QStringLiteral("background"));
	addChild(d->background);
	d->background->setHidden(true);
	connect(d->background, &Background::updateRequested, [=] {
		d->update();
	});

	connect(this, &Worksheet::childAspectAdded, this, &Worksheet::handleAspectAdded);
	connect(this, &Worksheet::childAspectAboutToBeRemoved, this, &Worksheet::handleAspectAboutToBeRemoved);
	connect(this, &Worksheet::childAspectRemoved, this, &Worksheet::handleAspectRemoved);
	connect(this, &Worksheet::childAspectMoved, this, &Worksheet::handleAspectMoved);

	if (!loading)
		init();
}

Worksheet::~Worksheet() {
	delete d_ptr;
}

void Worksheet::init() {
	Q_D(Worksheet);
	KConfig config;
	auto group = config.group(QStringLiteral("Worksheet"));

	// size
	d->scaleContent = group.readEntry(QStringLiteral("ScaleContent"), false);
	d->useViewSize = group.readEntry(QStringLiteral("UseViewSize"), false);
	d->pageRect.setX(0);
	d->pageRect.setY(0);
	d->pageRect.setWidth(group.readEntry(QStringLiteral("Width"), 1000));
	d->pageRect.setHeight(group.readEntry(QStringLiteral("Height"), 1000));
	d->m_scene->setSceneRect(d->pageRect);

	// background
	d->background->init(group);

	// layout
	d->layout = (Layout)group.readEntry(QStringLiteral("Layout"), static_cast<int>(Layout::VerticalLayout));
	d->layoutTopMargin = group.readEntry(QStringLiteral("LayoutTopMargin"), convertToSceneUnits(0., Unit::Centimeter));
	d->layoutBottomMargin = group.readEntry(QStringLiteral("LayoutBottomMargin"), convertToSceneUnits(0., Unit::Centimeter));
	d->layoutLeftMargin = group.readEntry(QStringLiteral("LayoutLeftMargin"), convertToSceneUnits(0., Unit::Centimeter));
	d->layoutRightMargin = group.readEntry(QStringLiteral("LayoutRightMargin"), convertToSceneUnits(0., Unit::Centimeter));
	d->layoutVerticalSpacing = group.readEntry(QStringLiteral("LayoutVerticalSpacing"), convertToSceneUnits(0., Unit::Centimeter));
	d->layoutHorizontalSpacing = group.readEntry(QStringLiteral("LayoutHorizontalSpacing"), convertToSceneUnits(0., Unit::Centimeter));
	d->layoutRowCount = group.readEntry(QStringLiteral("LayoutRowCount"), 2);
	d->layoutColumnCount = group.readEntry(QStringLiteral("LayoutColumnCount"), 2);

	// default theme
	auto settings = Settings::group(QStringLiteral("Settings_Worksheet"));
	d->theme = settings.readEntry(QStringLiteral("Theme"), QString());
	loadTheme(d->theme);
}

/*!
	converts from \c unit to the scene units. At the moment, 1 scene unit corresponds to 1/10 mm.
 */
double Worksheet::convertToSceneUnits(const double value, const Worksheet::Unit unit) {
	switch (unit) {
	case Unit::Millimeter:
		return value * 10.0;
	case Unit::Centimeter:
		return value * 100.0;
	case Unit::Inch:
		return value * 25.4 * 10.;
	case Unit::Point:
		return value * 25.4 / 72. * 10.;
	}

	return 0;
}

/*!
	converts from the scene units to \c unit . At the moment, 1 scene unit corresponds to 1/10 mm.
 */
double Worksheet::convertFromSceneUnits(const double value, const Worksheet::Unit unit) {
	switch (unit) {
	case Unit::Millimeter:
		return value / 10.0;
	case Unit::Centimeter:
		return value / 100.0;
	case Unit::Inch:
		return value / 25.4 / 10.;
	case Unit::Point:
		return value / 25.4 / 10. * 72.;
	}

	return 0;
}

QIcon Worksheet::icon() const {
	return QIcon::fromTheme(QStringLiteral("labplot-worksheet"));
}

/**
 * Return a new context menu. The caller takes ownership of the menu.
 */
QMenu* Worksheet::createContextMenu() {
	QMenu* menu = AbstractPart::createContextMenu();
	Q_ASSERT(menu);
	Q_EMIT requestProjectContextMenu(menu);
	return menu;
}

//! Construct a primary view on me.
/**
 * This method may be called multiple times during the life time of an Aspect, or it might not get
 * called at all. Aspects must not depend on the existence of a view for their operation.
 */
QWidget* Worksheet::view() const {
	DEBUG(Q_FUNC_INFO)
	if (!m_partView) {
		m_view = new WorksheetView(const_cast<Worksheet*>(this));
		m_partView = m_view;
		connect(m_view, &WorksheetView::statusInfo, this, &Worksheet::statusInfo);
		connect(m_view, &WorksheetView::propertiesExplorerRequested, this, &Worksheet::propertiesExplorerRequested);
		connect(this, &Worksheet::cartesianPlotMouseModeChanged, m_view, &WorksheetView::cartesianPlotMouseModeChangedSlot);
		connect(this, &Worksheet::childContextMenuRequested, m_view, &WorksheetView::childContextMenuRequested);
		connect(this, &Worksheet::viewAboutToBeDeleted, [this]() {
			m_view = nullptr;
		});
		Q_EMIT const_cast<Worksheet*>(this)->changed();
	}
	return m_partView;
}

/*!
 * returns the list of all parent aspects (folders and sub-folders)
 * together with all the data containers required to plot the data in the worksheet
 */
QVector<AbstractAspect*> Worksheet::dependsOn() const {
	// add all parent aspects (folders and sub-folders)
	auto aspects = AbstractAspect::dependsOn();

	// traverse all plots and add all data containers they depend on
	for (const auto* plot : children<AbstractPlot>())
		aspects << plot->dependsOn();

	return aspects;
}

QVector<AspectType> Worksheet::pasteTypes() const {
	return QVector<AspectType>{AspectType::CartesianPlot, AspectType::TextLabel, AspectType::Image};
}

bool Worksheet::exportView() const {
#ifndef SDK
	setPrinting(true);
	// Retransform all elements with print enabled
	for (auto* child : children<WorksheetElement>())
		child->retransform();
	auto* dlg = new ExportWorksheetDialog(m_view);
	dlg->setProjectFileName(const_cast<Worksheet*>(this)->project()->fileName());
	dlg->setFileName(name());
	bool ret;
	if ((ret = (dlg->exec() == QDialog::Accepted))) {
		QString path = dlg->path();
		const WorksheetView::ExportFormat format = dlg->exportFormat();
		const WorksheetView::ExportArea area = dlg->exportArea();
		const bool background = dlg->exportBackground();
		const int resolution = dlg->exportResolution();

		WAIT_CURSOR;
		m_view->exportToFile(path, format, area, background, resolution);
		RESET_CURSOR;
	}
	delete dlg;
	setPrinting(false);
	return ret;
#else
	return true;
#endif
}

bool Worksheet::exportView(QPixmap& pixmap) const {
	if (!m_view)
		return false;

	m_view->exportToPixmap(pixmap);
	return true;
}

bool Worksheet::printView() {
#ifndef SDK
	setPrinting(true);
	// for print/export, retransform all XYCurves to get better quality with the disabled line optimization
	for (auto* curve : children<XYCurve>())
		curve->retransform();

	QPrinter printer;
	auto* dlg = new QPrintDialog(&printer, m_view);
	dlg->setWindowTitle(i18nc("@title:window", "Print Worksheet"));
	bool ret;
	if ((ret = (dlg->exec() == QDialog::Accepted)))
		m_view->print(&printer);

	delete dlg;
	setPrinting(false);
	return ret;
#else
	return true;
#endif
}

bool Worksheet::printPreview() const {
#ifndef SDK
	setPrinting(true);
	// for print/export, retransform all XYCurves to get better quality with the disabled line optimization
	for (auto* curve : children<XYCurve>())
		curve->retransform();

	auto* dlg = new QPrintPreviewDialog(m_view);
	connect(dlg, &QPrintPreviewDialog::paintRequested, m_view, &WorksheetView::print);
	const auto r = dlg->exec();
	setPrinting(false);
	return r;
#else
	return true;
#endif
}

void Worksheet::handleAspectAdded(const AbstractAspect* aspect) {
	DEBUG(Q_FUNC_INFO)
	Q_D(Worksheet);
	const auto* addedElement = dynamic_cast<const WorksheetElement*>(aspect);
	if (!addedElement)
		return;

	if (aspect->parentAspect() != this)
		return;

	// add the GraphicsItem of the added child to the scene
	DEBUG(Q_FUNC_INFO << ", ADDING child to SCENE")

	if (aspect->type() == AspectType::SurfacePlot) {
		const auto* addedElement = dynamic_cast<const Surface3DPlotArea*>(aspect);
		const Q3DSurface* graph = addedElement->graph();
		if (graph) {
			QWidget* window = graph->window();
			if (window) {
				QGraphicsProxyWidget* proxy = d->m_scene->addWidget(window);
			}
		}
	} else {
		auto* item = addedElement->graphicsItem();
		d->m_scene->addItem(item);
	}
	connect(aspect, &AbstractAspect::contextMenuRequested, this, &Worksheet::childContextMenuRequested);
	connect(addedElement, &WorksheetElement::changed, this, &Worksheet::changed);

	// for containers, connect to visilibity changes and update the layout accordingly
	if (dynamic_cast<const WorksheetElementContainer*>(addedElement))
		connect(addedElement, &WorksheetElement::visibleChanged, this, [=]() {
			if (layout() != Worksheet::Layout::NoLayout)
				updateLayout();
		});

	const auto* plot = dynamic_cast<const CartesianPlot*>(aspect);
	if (plot) {
		connect(plot, &CartesianPlot::axisShiftSignal, this, &Worksheet::cartesianPlotAxisShift);
		connect(plot, &CartesianPlot::wheelEventSignal, this, &Worksheet::cartesianPlotWheelEvent);
		connect(plot, &CartesianPlot::mouseMoveCursorModeSignal, this, &Worksheet::cartesianPlotMouseMoveCursorMode);
		connect(plot, &CartesianPlot::mouseMoveSelectionModeSignal, this, &Worksheet::cartesianPlotMouseMoveSelectionMode);
		connect(plot, &CartesianPlot::mouseMoveZoomSelectionModeSignal, this, &Worksheet::cartesianPlotMouseMoveZoomSelectionMode);
		connect(plot, &CartesianPlot::mousePressCursorModeSignal, this, &Worksheet::cartesianPlotMousePressCursorMode);
		connect(plot, &CartesianPlot::mousePressZoomSelectionModeSignal, this, &Worksheet::cartesianPlotMousePressZoomSelectionMode);
		connect(plot, &CartesianPlot::mouseReleaseZoomSelectionModeSignal, this, &Worksheet::cartesianPlotMouseReleaseZoomSelectionMode);
		connect(plot, &CartesianPlot::mouseHoverZoomSelectionModeSignal, this, &Worksheet::cartesianPlotMouseHoverZoomSelectionMode);
		connect(plot, &CartesianPlot::mouseHoverOutsideDataRectSignal, this, &Worksheet::cartesianPlotMouseHoverOutsideDataRect);
		connect(plot, &CartesianPlot::aspectDescriptionChanged, this, &Worksheet::updateCompleteCursorTreeModel);
		connect(plot, &CartesianPlot::curveNameChanged, this, &Worksheet::updateCompleteCursorTreeModel);
		connect(plot, &CartesianPlot::curveRemoved, this, &Worksheet::curveRemoved);
		connect(plot, &CartesianPlot::curveAdded, this, &Worksheet::curveAdded);
		connect(plot, &CartesianPlot::visibleChanged, this, &Worksheet::updateCompleteCursorTreeModel);
		connect(plot, &CartesianPlot::curveVisibilityChangedSignal, this, &Worksheet::updateCompleteCursorTreeModel);
		connect(plot, &CartesianPlot::curveDataChanged, this, &Worksheet::curveDataChanged);
		connect(plot,
				static_cast<void (CartesianPlot::*)(const QColor&, const QString&)>(&CartesianPlot::plotColorChanged),
				this,
				&Worksheet::updateCurveBackground);
		connect(plot, &CartesianPlot::mouseModeChanged, this, &Worksheet::cartesianPlotMouseModeChangedSlot);
		auto* p = const_cast<CartesianPlot*>(plot);
		p->setInteractive(d->plotsInteractive);

		cursorModelPlotAdded(p->name());
	}
	qreal zVal = 0;
	const auto& children = this->children<WorksheetElement>(ChildIndexFlag::IncludeHidden);
	for (auto* child : children)
		child->graphicsItem()->setZValue(zVal++);

	// if a theme was selected in the worksheet, apply this theme for newly added children
	if (!d->theme.isEmpty() && !isLoading() && !pasted() && !aspect->pasted()) {
		KConfig config(ThemeHandler::themeFilePath(d->theme), KConfig::SimpleConfig);
		const_cast<WorksheetElement*>(addedElement)->loadThemeConfig(config);
	}

	// recalculate the layout if enabled, set the currently added plot resizable otherwise
	if (!isLoading()) {
		if (d->layout != Worksheet::Layout::NoLayout)
			d->updateLayout(false);
		else {
			if (plot) {
				// make other plots non-resizable
				const auto& containers = this->children<WorksheetElementContainer>();
				for (auto* container : containers)
					container->setResizeEnabled(false);

				// make the newly added plot resizable
				const_cast<CartesianPlot*>(plot)->setResizeEnabled(true);
			}
		}
	}
}

void Worksheet::handleAspectAboutToBeRemoved(const AbstractAspect* aspect) {
	Q_D(Worksheet);
	const auto* removedElement = qobject_cast<const WorksheetElement*>(aspect);
	if (removedElement) {
		auto* item = removedElement->graphicsItem();
		// TODO: disabled until Origin project import is fixed
		if (item->scene() == d->m_scene)
			d->m_scene->removeItem(item);
	}
}

void Worksheet::handleAspectRemoved(const AbstractAspect* /*parent*/, const AbstractAspect* /*before*/, const AbstractAspect* child) {
	Q_D(Worksheet);
	if (d->layout != Worksheet::Layout::NoLayout)
		d->updateLayout(false);
	auto* plot = dynamic_cast<const CartesianPlot*>(child);
	if (plot)
		cursorModelPlotRemoved(plot->name());
}

/*!
 * called when one of the children was moved, re-adjusts the Z-values for all children.
 */
void Worksheet::handleAspectMoved() {
	qreal zVal = 0;
	const auto& children = this->children<WorksheetElement>(ChildIndexFlag::IncludeHidden);
	for (auto* child : children)
		child->graphicsItem()->setZValue(zVal++);
}

QGraphicsScene* Worksheet::scene() const {
	Q_D(const Worksheet);
	return d->m_scene;
}

QRectF Worksheet::pageRect() const {
	Q_D(const Worksheet);
	return d->m_scene->sceneRect();
}

double Worksheet::zoomFactor() const {
	return m_view->zoomFactor();
}

/*!
	this slot is called when a worksheet element is selected in the project explorer.
	emits \c itemSelected() which forwards this event to the \c WorksheetView
	in order to select the corresponding \c QGraphicsItem.
 */
void Worksheet::childSelected(const AbstractAspect* aspect) {
	auto* element = qobject_cast<WorksheetElement*>(const_cast<AbstractAspect*>(aspect));
	if (element)
		Q_EMIT itemSelected(element->graphicsItem());
}

/*!
	this slot is called when a worksheet element is deselected in the project explorer.
	emits \c itemDeselected() which forwards this event to \c WorksheetView
	in order to deselect the corresponding \c QGraphicsItem.
 */
void Worksheet::childDeselected(const AbstractAspect* aspect) {
	auto* element = qobject_cast<WorksheetElement*>(const_cast<AbstractAspect*>(aspect));
	if (element)
		Q_EMIT itemDeselected(element->graphicsItem());
}

/*!
 *  Emits the signal to select or to deselect the aspect corresponding to \c QGraphicsItem \c item in the project explorer,
 *  if \c selected=true or \c selected=false, respectively.
 *  The signal is handled in \c AspectTreeModel and forwarded to the tree view in \c ProjectExplorer.
 * This function is called in \c WorksheetView upon selection changes.
 */
void Worksheet::setItemSelectedInView(const QGraphicsItem* item, const bool b) {
	// determine the corresponding aspect
	AbstractAspect* aspect(nullptr);
	for (const auto* child : children<WorksheetElement>(ChildIndexFlag::IncludeHidden)) {
		aspect = this->aspectFromGraphicsItem(child, item);
		if (aspect)
			break;
	}

	if (!aspect)
		return;

	// forward selection/deselection to AbstractTreeModel
	if (b)
		Q_EMIT childAspectSelectedInView(aspect);
	else
		Q_EMIT childAspectDeselectedInView(aspect);

	// handle the resize items on selection changes
	if (layout() == Worksheet::Layout::NoLayout) {
		// only one selected plot can be made resizable
		if (b) {
			const auto& items = m_view->selectedItems();
			if (items.size() == 1) {
				// only one object is selected.
				// make it resiable if its a container
				auto* container = dynamic_cast<WorksheetElementContainer*>(aspect);
				if (container)
					container->setResizeEnabled(true);
			} else if (items.size() > 1) {
				// multiple objects are selected, make all containers non-resizable
				const auto& elements = children<WorksheetElement>();
				for (auto* element : elements) {
					auto* container = dynamic_cast<WorksheetElementContainer*>(element);
					if (container)
						container->setResizeEnabled(false);
				}
			}
		} else {
			auto* container = dynamic_cast<WorksheetElementContainer*>(aspect);
			if (container)
				container->setResizeEnabled(false);
		}
	}
}

/*!
 * helper function:  checks whether \c aspect or one of its children has the \c GraphicsItem \c item
 * Returns a pointer to \c WorksheetElement having this item.
 */
WorksheetElement* Worksheet::aspectFromGraphicsItem(const WorksheetElement* parent, const QGraphicsItem* item) const {
	if (parent->graphicsItem() == item)
		return const_cast<WorksheetElement*>(parent);
	else {
		for (const auto* child : parent->children<WorksheetElement>(AbstractAspect::ChildIndexFlag::IncludeHidden)) {
			WorksheetElement* a = this->aspectFromGraphicsItem(child, item);
			if (a)
				return a;
		}
		return nullptr;
	}
}

/*!
	Selects or deselects the worksheet in the project explorer.
	This function is called in \c WorksheetView.
	The worksheet gets deselected if there are selected items in the view,
	and selected if there are no selected items in the view.
*/
void Worksheet::setSelectedInView(const bool b) {
	if (b)
		Q_EMIT childAspectSelectedInView(this);
	else
		Q_EMIT childAspectDeselectedInView(this);
}

void Worksheet::deleteAspectFromGraphicsItem(const QGraphicsItem* item) {
	Q_ASSERT(item);
	// determine the corresponding aspect
	AbstractAspect* aspect(nullptr);
	for (const auto* child : children<WorksheetElement>(ChildIndexFlag::IncludeHidden)) {
		aspect = this->aspectFromGraphicsItem(child, item);
		if (aspect)
			break;
	}

	if (!aspect)
		return;

	if (aspect->parentAspect())
		aspect->parentAspect()->removeChild(aspect);
	else
		this->removeChild(aspect);
}

void Worksheet::setIsClosing() {
	if (m_view)
		m_view->setIsClosing();
}

void Worksheet::suppressSelectionChangedEvent(bool value) {
	if (m_view)
		m_view->suppressSelectionChangedEvent(value);
}

/*!
 * \brief Worksheet::plotCount
 * \return number of CartesianPlot's in the Worksheet
 */
int Worksheet::plotCount() {
	return children<CartesianPlot>().length();
}

/*!
 * \brief Worksheet::plot
 * \param index Number of plot which should be returned
 * \return Pointer to the CartesianPlot which was searched with index
 */
CartesianPlot* Worksheet::plot(int index) {
	auto plots = children<CartesianPlot>();
	if (plots.length() - 1 >= index)
		return plots.at(index);
	return nullptr;
}

TreeModel* Worksheet::cursorModel() {
	Q_D(const Worksheet);
	return d->cursorData;
}

void Worksheet::update() {
	Q_EMIT requestUpdate();
	Q_EMIT changed();
}

void Worksheet::setSuppressLayoutUpdate(bool value) {
	Q_D(Worksheet);
	d->suppressLayoutUpdate = value;
}

void Worksheet::updateLayout() {
	Q_D(Worksheet);
	d->updateLayout();
}

Worksheet::CartesianPlotActionMode Worksheet::cartesianPlotActionMode() const {
	Q_D(const Worksheet);
	return d->cartesianPlotActionMode;
}

Worksheet::CartesianPlotActionMode Worksheet::cartesianPlotCursorMode() const {
	Q_D(const Worksheet);
	return d->cartesianPlotCursorMode;
}

bool Worksheet::plotsInteractive() const {
	Q_D(const Worksheet);
	return d->plotsInteractive;
}

void Worksheet::setCartesianPlotActionMode(Worksheet::CartesianPlotActionMode mode) {
	Q_D(Worksheet);
	if (d->cartesianPlotActionMode == mode)
		return;

	d->cartesianPlotActionMode = mode;
	setProjectChanged(true);
}

void Worksheet::setCartesianPlotCursorMode(Worksheet::CartesianPlotActionMode mode) {
	Q_D(Worksheet);
	if (d->cartesianPlotCursorMode == mode)
		return;

	d->cartesianPlotCursorMode = mode;

	if (mode == CartesianPlotActionMode::ApplyActionToAll) {
		d->suppressCursorPosChanged = true;
		const auto& plots = children<CartesianPlot>();
		QPointF logicPos;
		if (!plots.isEmpty()) {
			for (int i = 0; i < 2; i++) {
				logicPos = QPointF(plots[0]->cursorPos(i), 0); // y value does not matter
				cartesianPlotMousePressCursorMode(i, logicPos);
			}
		}
		d->suppressCursorPosChanged = false;
	}
	updateCompleteCursorTreeModel();
	setProjectChanged(true);
}

void Worksheet::setInteractive(bool value) {
	if (!m_view)
		view();
	m_view->setInteractive(value);
}

void Worksheet::setPlotsInteractive(bool interactive) {
	Q_D(Worksheet);
	if (d->plotsInteractive == interactive)
		return;

	d->plotsInteractive = interactive;

	for (auto* plot : children<CartesianPlot>())
		plot->setInteractive(interactive);

	setProjectChanged(true);
}

void Worksheet::registerShortcuts() {
	m_view->registerShortcuts();
}

WorksheetElement* Worksheet::currentSelection() {
	if (!m_view) {
		view();
		return nullptr;
	}

	return m_view->selectedElement();
}

void Worksheet::unregisterShortcuts() {
	m_view->unregisterShortcuts();
}

/* =============================== getter methods for general options ==================================== */
BASIC_D_READER_IMPL(Worksheet, bool, scaleContent, scaleContent)
BASIC_D_READER_IMPL(Worksheet, bool, useViewSize, useViewSize)
BASIC_D_READER_IMPL(Worksheet, Worksheet::ZoomFit, zoomFit, zoomFit)

// background
Background* Worksheet::background() const {
	Q_D(const Worksheet);
	return d->background;
}

/* =============================== getter methods for layout options ====================================== */
BASIC_D_READER_IMPL(Worksheet, Worksheet::Layout, layout, layout)
BASIC_D_READER_IMPL(Worksheet, double, layoutTopMargin, layoutTopMargin)
BASIC_D_READER_IMPL(Worksheet, double, layoutBottomMargin, layoutBottomMargin)
BASIC_D_READER_IMPL(Worksheet, double, layoutLeftMargin, layoutLeftMargin)
BASIC_D_READER_IMPL(Worksheet, double, layoutRightMargin, layoutRightMargin)
BASIC_D_READER_IMPL(Worksheet, double, layoutHorizontalSpacing, layoutHorizontalSpacing)
BASIC_D_READER_IMPL(Worksheet, double, layoutVerticalSpacing, layoutVerticalSpacing)
BASIC_D_READER_IMPL(Worksheet, int, layoutRowCount, layoutRowCount)
BASIC_D_READER_IMPL(Worksheet, int, layoutColumnCount, layoutColumnCount)

BASIC_D_READER_IMPL(Worksheet, QString, theme, theme)

/* ============================ setter methods and undo commands for general options  ===================== */
STD_SETTER_CMD_IMPL_S(Worksheet, SetUseViewSize, bool, useViewSize)
void Worksheet::setUseViewSize(bool useViewSize) {
	Q_D(Worksheet);
	if (useViewSize != d->useViewSize)
		exec(new WorksheetSetUseViewSizeCmd(d, useViewSize, ki18n("%1: change size type")));
}

void Worksheet::setZoomFit(ZoomFit zoomFit) {
	Q_D(Worksheet);
	d->zoomFit = zoomFit; // No need to undo
}

STD_SETTER_CMD_IMPL_S(Worksheet, SetScaleContent, bool, scaleContent)
void Worksheet::setScaleContent(bool scaleContent) {
	Q_D(Worksheet);
	if (scaleContent != d->scaleContent)
		exec(new WorksheetSetScaleContentCmd(d, scaleContent, ki18n("%1: change \"rescale the content\" property")));
}

/* ============================ setter methods and undo commands  for layout options  ================= */
STD_SETTER_CMD_IMPL_F_S(Worksheet, SetLayout, Worksheet::Layout, layout, updateLayout)
void Worksheet::setLayout(Worksheet::Layout layout) {
	Q_D(Worksheet);
	if (layout != d->layout) {
		beginMacro(i18n("%1: set layout", name()));
		exec(new WorksheetSetLayoutCmd(d, layout, ki18n("%1: set layout")));
		endMacro();
	}
}

STD_SETTER_CMD_IMPL_M_F_S(Worksheet, SetLayoutTopMargin, double, layoutTopMargin, updateLayout)
void Worksheet::setLayoutTopMargin(double margin) {
	Q_D(Worksheet);
	if (margin != d->layoutTopMargin) {
		beginMacro(i18n("%1: set layout top margin", name()));
		exec(new WorksheetSetLayoutTopMarginCmd(d, margin, ki18n("%1: set layout top margin")));
		endMacro();
	}
}

STD_SETTER_CMD_IMPL_M_F_S(Worksheet, SetLayoutBottomMargin, double, layoutBottomMargin, updateLayout)
void Worksheet::setLayoutBottomMargin(double margin) {
	Q_D(Worksheet);
	if (margin != d->layoutBottomMargin) {
		beginMacro(i18n("%1: set layout bottom margin", name()));
		exec(new WorksheetSetLayoutBottomMarginCmd(d, margin, ki18n("%1: set layout bottom margin")));
		endMacro();
	}
}

STD_SETTER_CMD_IMPL_M_F_S(Worksheet, SetLayoutLeftMargin, double, layoutLeftMargin, updateLayout)
void Worksheet::setLayoutLeftMargin(double margin) {
	Q_D(Worksheet);
	if (margin != d->layoutLeftMargin) {
		beginMacro(i18n("%1: set layout left margin", name()));
		exec(new WorksheetSetLayoutLeftMarginCmd(d, margin, ki18n("%1: set layout left margin")));
		endMacro();
	}
}

STD_SETTER_CMD_IMPL_M_F_S(Worksheet, SetLayoutRightMargin, double, layoutRightMargin, updateLayout)
void Worksheet::setLayoutRightMargin(double margin) {
	Q_D(Worksheet);
	if (margin != d->layoutRightMargin) {
		beginMacro(i18n("%1: set layout right margin", name()));
		exec(new WorksheetSetLayoutRightMarginCmd(d, margin, ki18n("%1: set layout right margin")));
		endMacro();
	}
}

STD_SETTER_CMD_IMPL_M_F_S(Worksheet, SetLayoutVerticalSpacing, double, layoutVerticalSpacing, updateLayout)
void Worksheet::setLayoutVerticalSpacing(double spacing) {
	Q_D(Worksheet);
	if (spacing != d->layoutVerticalSpacing) {
		beginMacro(i18n("%1: set layout vertical spacing", name()));
		exec(new WorksheetSetLayoutVerticalSpacingCmd(d, spacing, ki18n("%1: set layout vertical spacing")));
		endMacro();
	}
}

STD_SETTER_CMD_IMPL_M_F_S(Worksheet, SetLayoutHorizontalSpacing, double, layoutHorizontalSpacing, updateLayout)
void Worksheet::setLayoutHorizontalSpacing(double spacing) {
	Q_D(Worksheet);
	if (spacing != d->layoutHorizontalSpacing) {
		beginMacro(i18n("%1: set layout horizontal spacing", name()));
		exec(new WorksheetSetLayoutHorizontalSpacingCmd(d, spacing, ki18n("%1: set layout horizontal spacing")));
		endMacro();
	}
}

STD_SETTER_CMD_IMPL_M_F_S(Worksheet, SetLayoutRowCount, int, layoutRowCount, updateLayout)
void Worksheet::setLayoutRowCount(int count) {
	Q_D(Worksheet);
	if (count != d->layoutRowCount) {
		beginMacro(i18n("%1: set layout row count", name()));
		exec(new WorksheetSetLayoutRowCountCmd(d, count, ki18n("%1: set layout row count")));
		endMacro();
	}
}

STD_SETTER_CMD_IMPL_M_F_S(Worksheet, SetLayoutColumnCount, int, layoutColumnCount, updateLayout)
void Worksheet::setLayoutColumnCount(int count) {
	Q_D(Worksheet);
	if (count != d->layoutColumnCount) {
		beginMacro(i18n("%1: set layout column count", name()));
		exec(new WorksheetSetLayoutColumnCountCmd(d, count, ki18n("%1: set layout column count")));
		endMacro();
	}
}

class WorksheetSetPageRectCmd : public StandardMacroSetterCmd<Worksheet::Private, QRectF> {
public:
	WorksheetSetPageRectCmd(Worksheet::Private* target, QRectF newValue, const KLocalizedString& description)
		: StandardMacroSetterCmd<Worksheet::Private, QRectF>(target, &Worksheet::Private::pageRect, newValue, description) {
	}
	void finalize() override {
		m_target->updatePageRect();
		Q_EMIT m_target->q->pageRectChanged(m_target->*m_field);
	}
	void finalizeUndo() override {
		m_target->m_scene->setSceneRect(m_target->*m_field);
		Q_EMIT m_target->q->pageRectChanged(m_target->*m_field);
	}
};

void Worksheet::setPageRect(const QRectF& rect) {
	Q_D(Worksheet);
	// don't allow any rectangulars of width/height equal to zero
	if (qFuzzyCompare(rect.width(), 0.) || qFuzzyCompare(rect.height(), 0.)) {
		Q_EMIT pageRectChanged(d->pageRect);
		return;
	}

	if (rect != d->pageRect) {
		if (!d->useViewSize) {
			beginMacro(i18n("%1: set page size", name()));
			exec(new WorksheetSetPageRectCmd(d, rect, ki18n("%1: set page size")));
			endMacro();
		} else {
			d->pageRect = rect;
			d->updatePageRect();
			Q_EMIT pageRectChanged(d->pageRect);
		}
	}
}

void Worksheet::setPrinting(bool on) const {
	const auto& elements = children<WorksheetElement>(AbstractAspect::ChildIndexFlag::Recursive | AbstractAspect::ChildIndexFlag::IncludeHidden);
	for (auto* elem : elements)
		elem->setPrinting(on);
}

STD_SETTER_CMD_IMPL_S(Worksheet, SetTheme, QString, theme)
void Worksheet::setTheme(const QString& theme) {
	Q_D(Worksheet);
	QString info;
	if (!theme.isEmpty())
		info = i18n("%1: load theme %2", name(), theme);
	else
		info = i18n("%1: load default theme", name());
	beginMacro(info);
	exec(new WorksheetSetThemeCmd(d, theme, ki18n("%1: set theme")));
	loadTheme(theme);
	endMacro();
}

void Worksheet::cartesianPlotMousePressZoomSelectionMode(QPointF logicPos) {
	auto senderPlot = static_cast<CartesianPlot*>(QObject::sender());
	auto mouseMode = senderPlot->mouseMode();
	auto actionMode = cartesianPlotActionMode();
	if (actionMode == CartesianPlotActionMode::ApplyActionToAll) {
		const auto& plots = children<CartesianPlot>(AbstractAspect::ChildIndexFlag::Recursive | AbstractAspect::ChildIndexFlag::IncludeHidden);
		for (auto* plot : plots)
			plot->mousePressZoomSelectionMode(logicPos, -1);
	} else if ((actionMode == CartesianPlotActionMode::ApplyActionToAllX && mouseMode != CartesianPlot::MouseMode::ZoomYSelection)
			   || (actionMode == CartesianPlotActionMode::ApplyActionToAllY && mouseMode != CartesianPlot::MouseMode::ZoomXSelection)) {
		const auto& plots = children<CartesianPlot>(AbstractAspect::ChildIndexFlag::Recursive | AbstractAspect::ChildIndexFlag::IncludeHidden);
		for (auto* plot : plots) {
			if (plot != senderPlot) {
				if (actionMode == CartesianPlotActionMode::ApplyActionToAllX)
					plot->setMouseMode(CartesianPlot::MouseMode::ZoomXSelection);
				else if (actionMode == CartesianPlotActionMode::ApplyActionToAllY)
					plot->setMouseMode(CartesianPlot::MouseMode::ZoomYSelection);
			}
			plot->mousePressZoomSelectionMode(logicPos, -1);
		}
	} else {
		int index = CartesianPlot::cSystemIndex(m_view->selectedElement());
		senderPlot->mousePressZoomSelectionMode(logicPos, index);
	}
}

void Worksheet::cartesianPlotMouseReleaseZoomSelectionMode() {
	auto senderPlot = static_cast<CartesianPlot*>(QObject::sender());
	auto mouseMode = senderPlot->mouseMode();
	auto actionMode = cartesianPlotActionMode();
	if (actionMode == CartesianPlotActionMode::ApplyActionToAll
		|| (actionMode == CartesianPlotActionMode::ApplyActionToAllX && mouseMode != CartesianPlot::MouseMode::ZoomYSelection)
		|| (actionMode == CartesianPlotActionMode::ApplyActionToAllY && mouseMode != CartesianPlot::MouseMode::ZoomXSelection)) {
		const auto& plots = children<CartesianPlot>(AbstractAspect::ChildIndexFlag::Recursive | AbstractAspect::ChildIndexFlag::IncludeHidden);
		for (auto* plot : plots) {
			plot->mouseReleaseZoomSelectionMode(-1);
			plot->setMouseMode(mouseMode);
		}
	} else {
		int index = CartesianPlot::cSystemIndex(m_view->selectedElement());
		auto* plot = static_cast<CartesianPlot*>(QObject::sender());
		plot->mouseReleaseZoomSelectionMode(index);
	}
}

void Worksheet::cartesianPlotMousePressCursorMode(int cursorNumber, QPointF logicPos) {
	if (cartesianPlotCursorMode() == CartesianPlotActionMode::ApplyActionToAll) {
		const auto& plots = children<CartesianPlot>(AbstractAspect::ChildIndexFlag::Recursive | AbstractAspect::ChildIndexFlag::IncludeHidden);
		for (auto* plot : plots)
			plot->mousePressCursorMode(cursorNumber, logicPos);
	} else {
		auto* plot = static_cast<CartesianPlot*>(QObject::sender());
		plot->mousePressCursorMode(cursorNumber, logicPos);
	}

	cursorPosChanged(cursorNumber, logicPos.x());
}

void Worksheet::cartesianPlotMouseMoveZoomSelectionMode(QPointF logicPos) {
	auto senderPlot = static_cast<CartesianPlot*>(QObject::sender());
	auto actionMode = cartesianPlotActionMode();
	auto mouseMode = senderPlot->mouseMode();
	if (actionMode == CartesianPlotActionMode::ApplyActionToAll
		|| (actionMode == CartesianPlotActionMode::ApplyActionToAllX && mouseMode != CartesianPlot::MouseMode::ZoomYSelection)
		|| (actionMode == CartesianPlotActionMode::ApplyActionToAllY && mouseMode != CartesianPlot::MouseMode::ZoomXSelection)) {
		const auto& plots = children<CartesianPlot>(AbstractAspect::ChildIndexFlag::Recursive | AbstractAspect::ChildIndexFlag::IncludeHidden);
		for (auto* plot : plots)
			plot->mouseMoveZoomSelectionMode(logicPos, -1);
	} else
		senderPlot->mouseMoveZoomSelectionMode(logicPos, CartesianPlot::cSystemIndex(m_view->selectedElement()));
}

void Worksheet::cartesianPlotMouseMoveSelectionMode(QPointF logicStart, QPointF logicEnd) {
	auto* senderPlot = static_cast<CartesianPlot*>(QObject::sender());
	auto actionMode = cartesianPlotActionMode();
	if (actionMode == CartesianPlotActionMode::ApplyActionToAll) {
		const auto& plots = children<CartesianPlot>(AbstractAspect::ChildIndexFlag::Recursive | AbstractAspect::ChildIndexFlag::IncludeHidden);
		for (auto* plot : plots)
			plot->mouseMoveSelectionMode(logicStart, logicEnd);
	} else if (actionMode == CartesianPlotActionMode::ApplyActionToSelection) {
		senderPlot->mouseMoveSelectionMode(logicStart, logicEnd);
	} else {
		const auto& plots = children<CartesianPlot>(AbstractAspect::ChildIndexFlag::Recursive | AbstractAspect::ChildIndexFlag::IncludeHidden);
		if (actionMode == CartesianPlotActionMode::ApplyActionToAllX) {
			// value does not matter, only difference
			logicStart.setY(0);
			logicEnd.setY(0);
			for (auto* plot : plots)
				plot->mouseMoveSelectionMode(logicStart, logicEnd);
		} else if (actionMode == CartesianPlotActionMode::ApplyActionToAllY) {
			// value does not matter, only difference
			logicStart.setX(0);
			logicEnd.setX(0);
			for (auto* plot : plots)
				plot->mouseMoveSelectionMode(logicStart, logicEnd);
		}
	}
}

void Worksheet::cartesianPlotMouseHoverZoomSelectionMode(QPointF logicPos) {
	auto senderPlot = static_cast<CartesianPlot*>(QObject::sender());
	auto actionMode = cartesianPlotActionMode();
	auto mouseMode = senderPlot->mouseMode();
	if (actionMode == CartesianPlotActionMode::ApplyActionToAll
		|| (actionMode == CartesianPlotActionMode::ApplyActionToAllX && mouseMode != CartesianPlot::MouseMode::ZoomYSelection)
		|| (actionMode == CartesianPlotActionMode::ApplyActionToAllY && mouseMode != CartesianPlot::MouseMode::ZoomXSelection)) {
		const auto& plots = children<CartesianPlot>(AbstractAspect::ChildIndexFlag::Recursive | AbstractAspect::ChildIndexFlag::IncludeHidden);
		for (auto* plot : plots)
			plot->mouseHoverZoomSelectionMode(logicPos, -1);
	} else {
		if (m_view->selectedElement()->parent(AspectType::CartesianPlot) == senderPlot)
			senderPlot->mouseHoverZoomSelectionMode(logicPos, CartesianPlot::cSystemIndex(m_view->selectedElement()));
		else
			senderPlot->mouseHoverZoomSelectionMode(logicPos, -1);
	}
}

void Worksheet::cartesianPlotMouseHoverOutsideDataRect() {
	auto senderPlot = static_cast<CartesianPlot*>(QObject::sender());
	auto mode = cartesianPlotActionMode();
	auto mouseMode = senderPlot->mouseMode();
	if (cartesianPlotActionMode() == CartesianPlotActionMode::ApplyActionToAll
		|| (mode == CartesianPlotActionMode::ApplyActionToAllX && mouseMode != CartesianPlot::MouseMode::ZoomYSelection)
		|| (mode == CartesianPlotActionMode::ApplyActionToAllY && mouseMode != CartesianPlot::MouseMode::ZoomXSelection)) {
		const auto& plots = children<CartesianPlot>(AbstractAspect::ChildIndexFlag::Recursive | AbstractAspect::ChildIndexFlag::IncludeHidden);
		for (auto* plot : plots)
			plot->mouseHoverOutsideDataRect();
	} else
		senderPlot->mouseHoverOutsideDataRect();
}

void Worksheet::cartesianPlotAxisShift(int delta, Dimension dim, int index) {
	const auto& plots = children<CartesianPlot>(AbstractAspect::ChildIndexFlag::Recursive | AbstractAspect::ChildIndexFlag::IncludeHidden);
	const auto cursorMode = cartesianPlotActionMode();
	bool leftOrDown = false;
	if (delta < 0)
		leftOrDown = true;

	switch (cursorMode) {
	case CartesianPlotActionMode::ApplyActionToAll: {
		for (auto* plot : plots)
			plot->shift(-1, dim, leftOrDown);
		break;
	}
	case CartesianPlotActionMode::ApplyActionToAllX: {
		switch (dim) {
		case Dimension::X: {
			for (auto* plot : plots)
				plot->shift(-1, dim, leftOrDown);
			break;
		}
		case Dimension::Y: {
			auto* plot = static_cast<CartesianPlot*>(QObject::sender());
			plot->shift(index, dim, leftOrDown);
			break;
		}
		}
		break;
	}
	case CartesianPlotActionMode::ApplyActionToAllY: {
		switch (dim) {
		case Dimension::X: {
			for (auto* plot : plots)
				plot->shift(index, dim, leftOrDown);
			break;
		}
		case Dimension::Y: {
			auto* plot = static_cast<CartesianPlot*>(QObject::sender());
			plot->shift(-1, dim, leftOrDown);
			break;
		}
		}
		break;
	}
	case CartesianPlotActionMode::ApplyActionToSelection: {
		auto* plot = static_cast<CartesianPlot*>(QObject::sender());
		plot->shift(index, dim, leftOrDown);
		break;
	}
	}
}

void Worksheet::cartesianPlotWheelEvent(const QPointF& sceneRelPos, int delta, int xIndex, int yIndex, bool considerDimension, Dimension dim) {
	const auto& plots = children<CartesianPlot>(AbstractAspect::ChildIndexFlag::Recursive | AbstractAspect::ChildIndexFlag::IncludeHidden);
	const auto cursorMode = cartesianPlotActionMode();
	if (considerDimension) {
		if ((dim == Dimension::X && (cursorMode == CartesianPlotActionMode::ApplyActionToAllX || cursorMode == CartesianPlotActionMode::ApplyActionToAll))
			|| (dim == Dimension::Y && (cursorMode == CartesianPlotActionMode::ApplyActionToAllY || cursorMode == CartesianPlotActionMode::ApplyActionToAll))) {
			for (auto* plot : plots)
				plot->wheelEvent(sceneRelPos, delta, -1, -1, considerDimension, dim);
		} else {
			auto* plot = static_cast<CartesianPlot*>(QObject::sender());
			plot->wheelEvent(sceneRelPos, delta, xIndex, yIndex, considerDimension, dim);
		}

	} else {
		switch (cursorMode) {
		case CartesianPlotActionMode::ApplyActionToAll: {
			for (auto* plot : plots)
				plot->wheelEvent(sceneRelPos, delta, -1, -1, considerDimension, dim);
			break;
		}
		case CartesianPlotActionMode::ApplyActionToAllX: {
			auto* plot = static_cast<CartesianPlot*>(QObject::sender());
			plot->wheelEvent(sceneRelPos, delta, -1, yIndex, considerDimension, dim);
			for (auto* p : plots) {
				if (p != plot) {
					// The yIndex must not be available in the other plots
					// yIndex does not matter, because considerDimension is true
					p->wheelEvent(sceneRelPos, delta, -1, -1, true, Dimension::X);
				}
			}
			break;
		}
		case CartesianPlotActionMode::ApplyActionToAllY: {
			auto* plot = static_cast<CartesianPlot*>(QObject::sender());
			// wheelEvent on all y in that plot
			plot->wheelEvent(sceneRelPos, delta, xIndex, -1, considerDimension, dim);
			for (auto* p : plots) {
				if (p != plot) {
					// The xIndex must not be available in the other plots
					// xIndex does not matter, because considerDimension is true
					p->wheelEvent(sceneRelPos, delta, -1, -1, true, Dimension::Y);
				}
			}
			break;
		}
		case CartesianPlotActionMode::ApplyActionToSelection: {
			auto* plot = static_cast<CartesianPlot*>(QObject::sender());
			plot->wheelEvent(sceneRelPos, delta, xIndex, yIndex, considerDimension, dim);
			break;
		}
		}
	}
}

void Worksheet::cartesianPlotMouseMoveCursorMode(int cursorNumber, QPointF logicPos) {
	if (cartesianPlotCursorMode() == CartesianPlotActionMode::ApplyActionToAll) {
		const auto& plots = children<CartesianPlot>(AbstractAspect::ChildIndexFlag::Recursive | AbstractAspect::ChildIndexFlag::IncludeHidden);
		for (auto* plot : plots)
			plot->mouseMoveCursorMode(cursorNumber, logicPos);
	} else {
		auto* plot = static_cast<CartesianPlot*>(QObject::sender());
		plot->mouseMoveCursorMode(cursorNumber, logicPos);
	}

	cursorPosChanged(cursorNumber, logicPos.x());
}

QString dateTimeDiffToString(const QDateTime& dt0, const QDateTime& dt1) {
	// dt0 to dt1 is positive if dt0 is smaller than dt1
	QString result;
	qint64 diff;
	bool negative = false;
	;
	if (dt0 < dt1)
		diff = dt0.msecsTo(dt1);
	else {
		diff = dt1.msecsTo(dt0);
		negative = true;
	}
	const qint64 dayToMsecs = 24 * 3600 * 1000;
	const qint64 hourToMsecs = 3600 * 1000;
	const qint64 minutesToMsecs = 60 * 1000;

	qint64 days = diff / dayToMsecs;
	diff -= days * dayToMsecs;
	qint64 hours = diff / hourToMsecs;
	diff -= hours * hourToMsecs;
	qint64 minutes = diff / minutesToMsecs;
	diff -= minutes * minutesToMsecs;
	qint64 seconds = diff / 1000;
	diff -= seconds * 1000;
	qint64 msecs = diff;

	if (negative)
		result += QStringLiteral("- ");

	if (days > 0)
		result += QString::number(days) + QStringLiteral(" ") + QObject::tr("days") + QStringLiteral(" ");

	if (hours > 0)
		result += QString::number(hours) + QStringLiteral(":");
	else
		result += QStringLiteral("00:");

	if (minutes > 0)
		result += QString::number(minutes) + QStringLiteral(":");
	else
		result += QStringLiteral("00:");

	if (seconds > 0)
		result += QString::number(seconds) + QStringLiteral(".");
	else
		result += QStringLiteral("00.");

	if (msecs > 0)
		result += QString::number(msecs);
	else
		result += QStringLiteral("000");

	return result;
}

/*!
 * \brief Worksheet::cursorPosChanged
 * Updates the cursor treemodel with the new data
 * \param xPos: new position of the cursor
 * It is assumed, that the plots/curves are in the same order than receiving from
 * the children() function. It's not checked if the names are the same
 */
void Worksheet::cursorPosChanged(int cursorNumber, double xPos) {
	Q_D(const Worksheet);
	if (d->suppressCursorPosChanged)
		return;

	auto* sender = dynamic_cast<CartesianPlot*>(QObject::sender());
	if (!sender)
		return;

	TreeModel* treeModel = cursorModel();

	// if ApplyActionToSelection, each plot has it's own x value
	bool isDatetime = sender->xRangeFormatDefault() == RangeT::Format::DateTime;
	if (cartesianPlotCursorMode() == CartesianPlotActionMode::ApplyActionToAll) {
		// x values
		int rowPlot = 1;
		QModelIndex xName = treeModel->index(0, static_cast<int>(WorksheetPrivate::TreeModelColumn::SIGNALNAME));
		treeModel->setData(xName, QVariant(QStringLiteral("X")));
		double valueCursor[2];
		QDateTime datetime[2];
		for (int i = 0; i < 2; i++) { // need both cursors to calculate diff
			QVariant data;
			valueCursor[i] = sender->cursorPos(i);
			if (isDatetime) {
				datetime[i] = QDateTime::fromMSecsSinceEpoch(valueCursor[i], Qt::UTC);
				data = datetime[i].toString(sender->rangeDateTimeFormat(Dimension::X));
			} else
				data = QVariant(valueCursor[i]);
			treeModel->setTreeData(data, 0, static_cast<int>(WorksheetPrivate::TreeModelColumn::CURSOR0) + i);
		}
		if (isDatetime)
			treeModel->setTreeData(dateTimeDiffToString(datetime[0], datetime[1]), 0, static_cast<int>(WorksheetPrivate::TreeModelColumn::CURSORDIFF));
		else
			treeModel->setTreeData(QVariant(valueCursor[1] - valueCursor[0]), 0, static_cast<int>(WorksheetPrivate::TreeModelColumn::CURSORDIFF));

		// y values
		for (int i = 0; i < plotCount(); i++) { // i=0 is the x Axis

			auto* p = plot(i);
			if (!p || !p->isVisible())
				continue;

			QModelIndex plotIndex = treeModel->index(rowPlot, static_cast<int>(WorksheetPrivate::TreeModelColumn::PLOTNAME));

			// curves
			int rowCurve = 0;
			for (int j = 0; j < p->curveCount(); j++) {
				// assumption: index of signals in model is the same than the index of the signal in the plot
				bool valueFound;

				const XYCurve* curve = p->getCurve(j);
				if (!curve->isVisible())
					continue;

				double value = curve->y(xPos, valueFound);
				if (cursorNumber == 0) {
					treeModel->setTreeData(QVariant(value), rowCurve, static_cast<int>(WorksheetPrivate::TreeModelColumn::CURSOR0), plotIndex);
					double valueCursor1 = treeModel->treeData(rowCurve, static_cast<int>(WorksheetPrivate::TreeModelColumn::CURSOR1), plotIndex).toDouble();
					treeModel->setTreeData(QVariant(valueCursor1 - value),
										   rowCurve,
										   static_cast<int>(WorksheetPrivate::TreeModelColumn::CURSORDIFF),
										   plotIndex);
				} else {
					treeModel->setTreeData(QVariant(value), rowCurve, static_cast<int>(WorksheetPrivate::TreeModelColumn::CURSOR1), plotIndex);
					double valueCursor0 = treeModel->treeData(rowCurve, static_cast<int>(WorksheetPrivate::TreeModelColumn::CURSOR0), plotIndex).toDouble();
					treeModel->setTreeData(QVariant(value - valueCursor0),
										   rowCurve,
										   static_cast<int>(WorksheetPrivate::TreeModelColumn::CURSORDIFF),
										   plotIndex);
				}
				rowCurve++;
			}
			rowPlot++;
		}
	} else { // apply to selection
		// assumption: plot is visible
		int rowCount = treeModel->rowCount();
		for (int i = 0; i < rowCount; i++) {
			QModelIndex plotIndex = treeModel->index(i, static_cast<int>(WorksheetPrivate::TreeModelColumn::PLOTNAME));
			if (plotIndex.data().toString().compare(sender->name()) != 0)
				continue;

			// x values (first row always exist)
			treeModel->setTreeData(QVariant(QStringLiteral("X")), 0, static_cast<int>(WorksheetPrivate::TreeModelColumn::SIGNALNAME), plotIndex);
			double valueCursor[2];
			for (int i = 0; i < 2; i++) { // need both cursors to calculate diff
				valueCursor[i] = sender->cursorPos(i);
				treeModel->setTreeData(QVariant(valueCursor[i]), 0, static_cast<int>(WorksheetPrivate::TreeModelColumn::CURSOR0) + i, plotIndex);
			}
			treeModel->setTreeData(QVariant(valueCursor[1] - valueCursor[0]), 0, static_cast<int>(WorksheetPrivate::TreeModelColumn::CURSORDIFF), plotIndex);

			// y values
			int rowCurve = 1; // first is x value
			for (int j = 0; j < sender->curveCount(); j++) { // j=0 are the x values

				const XYCurve* curve = sender->getCurve(j); // -1 because we start with 1 for the x axis
				if (!curve->isVisible())
					continue;

				// assumption: index of signals in model is the same than the index of the signal in the plot
				bool valueFound;

				double value = curve->y(xPos, valueFound);
				if (cursorNumber == 0) {
					treeModel->setTreeData(QVariant(value), rowCurve, static_cast<int>(WorksheetPrivate::TreeModelColumn::CURSOR0), plotIndex);
					double valueCursor1 = treeModel->treeData(rowCurve, static_cast<int>(WorksheetPrivate::TreeModelColumn::CURSOR1), plotIndex).toDouble();
					treeModel->setTreeData(QVariant(valueCursor1 - value),
										   rowCurve,
										   static_cast<int>(WorksheetPrivate::TreeModelColumn::CURSORDIFF),
										   plotIndex);
				} else {
					treeModel->setTreeData(QVariant(value), rowCurve, static_cast<int>(WorksheetPrivate::TreeModelColumn::CURSOR1), plotIndex);
					double valueCursor0 = treeModel->treeData(rowCurve, static_cast<int>(WorksheetPrivate::TreeModelColumn::CURSOR0), plotIndex).toDouble();
					treeModel->setTreeData(QVariant(value - valueCursor0),
										   rowCurve,
										   static_cast<int>(WorksheetPrivate::TreeModelColumn::CURSORDIFF),
										   plotIndex);
				}
				rowCurve++;
			}
		}
	}
}

void Worksheet::cursorModelPlotAdded(const QString& /*name*/) {
	//	TreeModel* treeModel = cursorModel();
	//	int rowCount = treeModel->rowCount();
	//	// add plot at the end
	//	treeModel->insertRows(rowCount, 1); // add empty rows. Then they become filled
	//	treeModel->setTreeData(QVariant(name), rowCount, WorksheetPrivate::TreeModelColumn::PLOTNAME); // rowCount instead of rowCount -1 because first row
	// is
	// the x value
	updateCompleteCursorTreeModel();
}

void Worksheet::cursorModelPlotRemoved(const QString& name) {
	TreeModel* treeModel = cursorModel();
	int rowCount = treeModel->rowCount();

	// first is x Axis
	for (int i = 1; i < rowCount; i++) {
		QModelIndex plotIndex = treeModel->index(i, static_cast<int>(WorksheetPrivate::TreeModelColumn::PLOTNAME));
		if (plotIndex.data().toString().compare(name) != 0)
			continue;
		treeModel->removeRows(plotIndex.row(), 1);
		return;
	}
}

void Worksheet::cartesianPlotMouseModeChangedSlot(CartesianPlot::MouseMode mode) {
	Q_D(Worksheet);
	if (d->updateCompleteCursorModel) {
		updateCompleteCursorTreeModel();
		d->updateCompleteCursorModel = false;
	}

	Q_EMIT cartesianPlotMouseModeChanged(mode);
}

void Worksheet::curveDataChanged(const XYCurve* curve) {
	auto* plot = dynamic_cast<CartesianPlot*>(QObject::sender());
	if (!plot || !curve)
		return;

	TreeModel* treeModel = cursorModel();
	int rowCount = treeModel->rowCount();

	for (int i = 0; i < rowCount; i++) {
		QModelIndex plotIndex = treeModel->index(i, static_cast<int>(WorksheetPrivate::TreeModelColumn::PLOTNAME));
		if (plotIndex.data().toString().compare(plot->name()) != 0)
			continue;

		for (int j = 0; j < plot->curveCount(); j++) {
			if (plot->getCurve(j)->name().compare(curve->name()) != 0)
				continue;

			treeModel->setTreeData(QVariant(curve->name()), j, static_cast<int>(WorksheetPrivate::TreeModelColumn::SIGNALNAME), plotIndex);

			bool valueFound;
			double valueCursor0 = curve->y(plot->cursorPos(0), valueFound);
			treeModel->setTreeData(QVariant(valueCursor0), j, static_cast<int>(WorksheetPrivate::TreeModelColumn::CURSOR0), plotIndex);

			double valueCursor1 = curve->y(plot->cursorPos(1), valueFound);
			treeModel->setTreeData(QVariant(valueCursor1), j, static_cast<int>(WorksheetPrivate::TreeModelColumn::CURSOR1), plotIndex);

			treeModel->setTreeData(QVariant(valueCursor1 - valueCursor0), j, static_cast<int>(WorksheetPrivate::TreeModelColumn::CURSORDIFF), plotIndex);
			break;
		}
		break;
	}
}

void Worksheet::curveAdded(const XYCurve* curve) {
	Q_D(const Worksheet);
	auto* plot = dynamic_cast<CartesianPlot*>(QObject::sender());
	if (!plot)
		return;

	TreeModel* treeModel = cursorModel();
	int rowCount = treeModel->rowCount();

	int i = 0;
	// first row is the x axis when applied to all plots. Starting at the second row
	if (cartesianPlotCursorMode() == CartesianPlotActionMode::ApplyActionToAll)
		i = 1;

	for (; i < rowCount; i++) {
		QModelIndex plotIndex = treeModel->index(i, static_cast<int>(WorksheetPrivate::TreeModelColumn::PLOTNAME));
		if (plotIndex.data().toString().compare(plot->name()) != 0)
			continue;
		int row = 0;
		for (int j = 0; j < plot->curveCount(); j++) {
			if (plot->getCurve(j)->name().compare(curve->name()) != 0) {
				if (plot->getCurve(j)->isVisible())
					row++;
				continue;
			}

			treeModel->insertRow(row, plotIndex);

			treeModel->setTreeData(QVariant(curve->name()), row, static_cast<int>(WorksheetPrivate::TreeModelColumn::SIGNALNAME), plotIndex);
			QColor curveColor = curve->line()->pen().color();
			curveColor.setAlpha(d->cursorTreeModelCurveBackgroundAlpha);
			treeModel->setTreeData(QVariant(curveColor), row, static_cast<int>(WorksheetPrivate::TreeModelColumn::SIGNALNAME), plotIndex, Qt::BackgroundRole);
			bool valueFound;
			double valueCursor0 = curve->y(plot->cursorPos(0), valueFound);
			treeModel->setTreeData(QVariant(valueCursor0), row, static_cast<int>(WorksheetPrivate::TreeModelColumn::CURSOR0), plotIndex);

			double valueCursor1 = curve->y(plot->cursorPos(1), valueFound);
			treeModel->setTreeData(QVariant(valueCursor1), row, static_cast<int>(WorksheetPrivate::TreeModelColumn::CURSOR1), plotIndex);

			treeModel->setTreeData(QVariant(valueCursor1 - valueCursor0), row, static_cast<int>(WorksheetPrivate::TreeModelColumn::CURSORDIFF), plotIndex);
			break;
		}
		break;
	}
}

void Worksheet::curveRemoved(const XYCurve* curve) {
	auto* plot = dynamic_cast<CartesianPlot*>(QObject::sender());
	if (!plot)
		return;

	TreeModel* treeModel = cursorModel();
	int rowCount = treeModel->rowCount();

	for (int i = 0; i < rowCount; i++) {
		QModelIndex plotIndex = treeModel->index(i, static_cast<int>(WorksheetPrivate::TreeModelColumn::PLOTNAME));
		if (plotIndex.data().toString().compare(plot->name()) != 0)
			continue;

		int curveCount = treeModel->rowCount(plotIndex);
		for (int j = 0; j < curveCount; j++) {
			QModelIndex curveIndex = treeModel->index(j, static_cast<int>(WorksheetPrivate::TreeModelColumn::SIGNALNAME), plotIndex);

			if (curveIndex.data().toString().compare(curve->name()) != 0)
				continue;
			treeModel->removeRow(j, plotIndex);
			break;
		}
		break;
	}
}

/*!
 * Updates the background of the cuves entry in the treeview
 * @param pen Pen of the curve
 * @param curveName Curve name to find in treemodel
 */
void Worksheet::updateCurveBackground(QColor color, const QString& curveName) {
	Q_D(const Worksheet);
	const auto* plot = static_cast<const CartesianPlot*>(QObject::sender());
	auto* treeModel = cursorModel();
	int rowCount = treeModel->rowCount();

	for (int i = 0; i < rowCount; i++) {
		auto plotIndex = treeModel->index(i, static_cast<int>(WorksheetPrivate::TreeModelColumn::PLOTNAME));
		if (plotIndex.data().toString().compare(plot->name()) != 0)
			continue;

		int curveCount = treeModel->rowCount(plotIndex);
		for (int j = 0; j < curveCount; j++) {
			auto curveIndex = treeModel->index(j, static_cast<int>(WorksheetPrivate::TreeModelColumn::SIGNALNAME), plotIndex);

			if (curveIndex.data().toString().compare(curveName) != 0)
				continue;

			color.setAlpha(d->cursorTreeModelCurveBackgroundAlpha);
			treeModel->setTreeData(QVariant(color), j, static_cast<int>(WorksheetPrivate::TreeModelColumn::SIGNALNAME), plotIndex, Qt::BackgroundRole);
			return;
		}
		return;
	}
}

/**
 * @brief Worksheet::updateCompleteCursorTreeModel
 * If the plot or the curve are not available, the plot/curve is not in the treemodel!
 */
void Worksheet::updateCompleteCursorTreeModel() {
	Q_D(const Worksheet);
	if (isLoading())
		return;

	TreeModel* treeModel = cursorModel();

	if (treeModel->rowCount() > 0)
		treeModel->removeRows(0, treeModel->rowCount()); // remove all data

	int pc = plotCount();
	if (pc < 1)
		return;

	if (cartesianPlotCursorMode() == CartesianPlotActionMode::ApplyActionToAll) {
		// 1 because of the X data
		treeModel->insertRows(0, 1); //, treeModel->index(0,0)); // add empty rows. Then they become filled

		// set X data
		QModelIndex xName = treeModel->index(0, static_cast<int>(WorksheetPrivate::TreeModelColumn::SIGNALNAME));
		treeModel->setData(xName, QVariant(QStringLiteral("X")));
		auto* plot0 = plot(0);
		if (plot0) {
			double valueCursor[2];
			for (int i = 0; i < 2; i++) {
				valueCursor[i] = plot0->cursorPos(i);
				QModelIndex cursor = treeModel->index(0, static_cast<int>(WorksheetPrivate::TreeModelColumn::CURSOR0) + i);

				treeModel->setData(cursor, QVariant(valueCursor[i]));
			}
			QModelIndex diff = treeModel->index(0, static_cast<int>(WorksheetPrivate::TreeModelColumn::CURSORDIFF));
			treeModel->setData(diff, QVariant(valueCursor[1] - valueCursor[0]));
		}
	} else {
		// treeModel->insertRows(0, plotCount, treeModel->index(0,0)); // add empty rows. Then they become filled
	}

	// set plot name, y value, background
	for (int i = 0; i < pc; i++) {
		auto* p = plot(i);
		QModelIndex plotName;
		int addOne = 0;

		if (!p || !p->isVisible())
			continue;

		// add new entry for the plot
		treeModel->insertRows(treeModel->rowCount(), 1); //, treeModel->index(0, 0));

		// add plot name and X row if needed
		if (cartesianPlotCursorMode() == CartesianPlotActionMode::ApplyActionToAll) {
			plotName = treeModel->index(i + 1, static_cast<int>(WorksheetPrivate::TreeModelColumn::PLOTNAME)); // plus one because first row are the x values
			treeModel->setData(plotName, QVariant(p->name()));
		} else {
			addOne = 1;
			plotName = treeModel->index(i, static_cast<int>(WorksheetPrivate::TreeModelColumn::PLOTNAME));
			treeModel->setData(plotName, QVariant(p->name()));
			treeModel->insertRows(0, 1, plotName); // one, because the first row are the x values

			QModelIndex xName = treeModel->index(0, static_cast<int>(WorksheetPrivate::TreeModelColumn::SIGNALNAME), plotName);
			treeModel->setData(xName, QVariant(QStringLiteral("X")));
			double valueCursor[2];
			for (int i = 0; i < 2; i++) {
				valueCursor[i] = p->cursorPos(i);
				QModelIndex cursor = treeModel->index(0, static_cast<int>(WorksheetPrivate::TreeModelColumn::CURSOR0) + i, plotName);

				treeModel->setData(cursor, QVariant(valueCursor[i]));
			}
			QModelIndex diff = treeModel->index(0, static_cast<int>(WorksheetPrivate::TreeModelColumn::CURSORDIFF), plotName);
			treeModel->setData(diff, QVariant(valueCursor[1] - valueCursor[0]));
		}

		int rowCurve = addOne;
		for (int j = 0; j < p->curveCount(); j++) {
			double cursorValue[2] = {NAN, NAN};
			const XYCurve* curve = p->getCurve(j);

			if (!curve->isVisible())
				continue;

			for (int k = 0; k < 2; k++) {
				double xPos = p->cursorPos(k);
				bool valueFound;
				cursorValue[k] = curve->y(xPos, valueFound);
			}
			treeModel->insertRows(rowCurve, 1, plotName);
			QColor curveColor = curve->line()->pen().color();
			curveColor.setAlpha(d->cursorTreeModelCurveBackgroundAlpha);
			treeModel->setTreeData(QVariant(curveColor), rowCurve, 0, plotName, Qt::BackgroundRole);
			treeModel->setTreeData(QVariant(curve->name()), rowCurve, static_cast<int>(WorksheetPrivate::TreeModelColumn::SIGNALNAME), plotName);
			treeModel->setTreeData(QVariant(cursorValue[0]), rowCurve, static_cast<int>(WorksheetPrivate::TreeModelColumn::CURSOR0), plotName);
			treeModel->setTreeData(QVariant(cursorValue[1]), rowCurve, static_cast<int>(WorksheetPrivate::TreeModelColumn::CURSOR1), plotName);
			treeModel->setTreeData(QVariant(cursorValue[1] - cursorValue[0]),
								   rowCurve,
								   static_cast<int>(WorksheetPrivate::TreeModelColumn::CURSORDIFF),
								   plotName);
			rowCurve++;
		}
	}
}

// ##############################################################################
// ######################  Private implementation ###############################
// ##############################################################################
WorksheetPrivate::WorksheetPrivate(Worksheet* owner)
	: q(owner)
	, m_scene(new QGraphicsScene()) {
	QStringList headers = {i18n("Curves"), QStringLiteral("V1"), QStringLiteral("V2"), QStringLiteral("V2-V1")};
	cursorData = new TreeModel(headers, nullptr);
}

QString WorksheetPrivate::name() const {
	return q->name();
}

/*!
 * called if the worksheet page (the actual size of worksheet's rectangular) was changed.
 * if a layout is active, it is is updated - this adjusts the sizes of the elements in the layout to the new page size.
 * if no layout is active and the option "scale content" is active, \c handleResize() is called to adjust the properties.
 */
void WorksheetPrivate::updatePageRect() {
	if (q->isLoading())
		return;

	QRectF oldRect = m_scene->sceneRect();
	m_scene->setSceneRect(pageRect);

	if (layout != Worksheet::Layout::NoLayout)
		updateLayout();
	else {
		if (scaleContent) {
			qreal horizontalRatio = pageRect.width() / oldRect.width();
			qreal verticalRatio = pageRect.height() / oldRect.height();
			const auto& children = q->children<WorksheetElement>(AbstractAspect::ChildIndexFlag::IncludeHidden);
			if (useViewSize) {
				// don't make the change of the geometry undoable/redoable if the view size is used.
				for (auto* elem : children) {
					elem->setUndoAware(false);
					elem->handleResize(horizontalRatio, verticalRatio, true);
					elem->setUndoAware(true);
				}
			} else {
				// 				for (auto* child : children)
				// 					child->handleResize(horizontalRatio, verticalRatio, true);
			}
		}
	}
}

void WorksheetPrivate::update() {
	q->update();
}

WorksheetPrivate::~WorksheetPrivate() {
	delete m_scene;
	delete cursorData;
}

void WorksheetPrivate::updateLayout(bool undoable) {
	if (suppressLayoutUpdate)
		return;

	const auto& list = q->children<WorksheetElementContainer>();
	int count = 0;
	for (auto* elem : list)
		if (elem->isVisible())
			++count;

	if (count == 0)
		return;

	// determine the currently selected plot/container and make it
	// resizable or not depending on the layout settings
	bool resizable = (layout == Worksheet::Layout::NoLayout);
	if (q->m_view) {
		const auto& items = q->m_view->selectedItems();
		if (items.size() == 1) {
			const auto& item = items.constFirst();
			const auto& containers = q->children<WorksheetElementContainer>();
			for (auto* container : containers) {
				if (container->graphicsItem() == item) {
					container->setResizeEnabled(resizable);
					break;
				}
			}
		}
	}

	if (layout == Worksheet::Layout::NoLayout) {
		for (auto* elem : list)
			elem->graphicsItem()->setFlag(QGraphicsItem::ItemIsMovable, true);

		return;
	}

	double x = layoutLeftMargin;
	double y = layoutTopMargin;
	double w, h;
	if (layout == Worksheet::Layout::VerticalLayout) {
		w = m_scene->sceneRect().width() - layoutLeftMargin - layoutRightMargin;
		h = (m_scene->sceneRect().height() - layoutTopMargin - layoutBottomMargin - (count - 1) * layoutVerticalSpacing) / count;
		for (auto* elem : list) {
			if (!elem->isVisible())
				continue;
			setContainerRect(elem, x, y, h, w, undoable);
			y += h + layoutVerticalSpacing;
		}
	} else if (layout == Worksheet::Layout::HorizontalLayout) {
		w = (m_scene->sceneRect().width() - layoutLeftMargin - layoutRightMargin - (count - 1) * layoutHorizontalSpacing) / count;
		h = m_scene->sceneRect().height() - layoutTopMargin - layoutBottomMargin;
		for (auto* elem : list) {
			if (!elem->isVisible())
				continue;
			setContainerRect(elem, x, y, h, w, undoable);
			x += w + layoutHorizontalSpacing;
		}
	} else { // GridLayout
		// add new rows, if not sufficient
		if (count > layoutRowCount * layoutColumnCount) {
			layoutRowCount = floor((double)count / layoutColumnCount + 0.5);
			Q_EMIT q->layoutRowCountChanged(layoutRowCount);
		}

		w = (m_scene->sceneRect().width() - layoutLeftMargin - layoutRightMargin - (layoutColumnCount - 1) * layoutHorizontalSpacing) / layoutColumnCount;
		h = (m_scene->sceneRect().height() - layoutTopMargin - layoutBottomMargin - (layoutRowCount - 1) * layoutVerticalSpacing) / layoutRowCount;
		int columnIndex = 0; // counts the columns in a row
		for (auto* elem : list) {
			if (!elem->isVisible())
				continue;
			setContainerRect(elem, x, y, h, w, undoable);
			x += w + layoutHorizontalSpacing;
			columnIndex++;
			if (columnIndex == layoutColumnCount) {
				columnIndex = 0;
				x = layoutLeftMargin;
				y += h + layoutVerticalSpacing;
			}
		}
	}

	Q_EMIT q->changed();
}

void WorksheetPrivate::setContainerRect(WorksheetElementContainer* elem, double x, double y, double h, double w, bool undoable) {
	if (useViewSize) {
		// when using the view size, no need to put rect changes onto the undo-stack
		elem->setUndoAware(false);
		elem->setRect(QRectF(x, y, w, h));
		elem->setUndoAware(true);
	} else {
		// don't put rect changed onto the undo-stack if undoable-flag is set to true,
		// e.g. when new child is added or removed (the layout and the childrend rects will be updated anyway)
		if (!undoable) {
			elem->setUndoAware(false);
			elem->setRect(QRectF(x, y, w, h));
			elem->setUndoAware(true);
		} else
			elem->setRect(QRectF(x, y, w, h));
	}
	elem->graphicsItem()->setFlag(QGraphicsItem::ItemIsMovable, false);
}

// ##############################################################################
// ##################  Serialization/Deserialization  ###########################
// ##############################################################################

//! Save as XML
void Worksheet::save(QXmlStreamWriter* writer) const {
	Q_D(const Worksheet);
	writer->writeStartElement(QStringLiteral("worksheet"));
	writeBasicAttributes(writer);
	writeCommentElement(writer);

	// applied theme
	if (!d->theme.isEmpty()) {
		writer->writeStartElement(QStringLiteral("theme"));
		writer->writeAttribute(QStringLiteral("name"), d->theme);
		writer->writeEndElement();
	}

	// geometry
	writer->writeStartElement(QStringLiteral("geometry"));
	QRectF rect = d->m_scene->sceneRect();
	writer->writeAttribute(QStringLiteral("x"), QString::number(rect.x()));
	writer->writeAttribute(QStringLiteral("y"), QString::number(rect.y()));
	writer->writeAttribute(QStringLiteral("width"), QString::number(rect.width()));
	writer->writeAttribute(QStringLiteral("height"), QString::number(rect.height()));
	writer->writeAttribute(QStringLiteral("useViewSize"), QString::number(d->useViewSize));
	writer->writeAttribute(QStringLiteral("zoomFit"), QString::number((int)d->zoomFit));
	writer->writeEndElement();

	// layout
	writer->writeStartElement(QStringLiteral("layout"));
	writer->writeAttribute(QStringLiteral("layout"), QString::number(static_cast<int>(d->layout)));
	writer->writeAttribute(QStringLiteral("topMargin"), QString::number(d->layoutTopMargin));
	writer->writeAttribute(QStringLiteral("bottomMargin"), QString::number(d->layoutBottomMargin));
	writer->writeAttribute(QStringLiteral("leftMargin"), QString::number(d->layoutLeftMargin));
	writer->writeAttribute(QStringLiteral("rightMargin"), QString::number(d->layoutRightMargin));
	writer->writeAttribute(QStringLiteral("verticalSpacing"), QString::number(d->layoutVerticalSpacing));
	writer->writeAttribute(QStringLiteral("horizontalSpacing"), QString::number(d->layoutHorizontalSpacing));
	writer->writeAttribute(QStringLiteral("columnCount"), QString::number(d->layoutColumnCount));
	writer->writeAttribute(QStringLiteral("rowCount"), QString::number(d->layoutRowCount));
	writer->writeEndElement();

	// background properties
	d->background->save(writer);

	// cartesian properties
	writer->writeStartElement(QStringLiteral("plotProperties"));
	writer->writeAttribute(QStringLiteral("plotInteractive"), QString::number(d->plotsInteractive));
	writer->writeAttribute(QStringLiteral("cartesianPlotActionMode"), QString::number(static_cast<int>(d->cartesianPlotActionMode)));
	writer->writeAttribute(QStringLiteral("cartesianPlotCursorMode"), QString::number(static_cast<int>(d->cartesianPlotCursorMode)));
	writer->writeEndElement();

	// serialize all children
	for (auto* child : children<WorksheetElement>(ChildIndexFlag::IncludeHidden))
		child->save(writer);

	writer->writeEndElement(); // close "worksheet" section
}

//! Load from XML
bool Worksheet::load(XmlStreamReader* reader, bool preview) {
	if (!readBasicAttributes(reader))
		return false;

	Q_D(Worksheet);

	// clear the theme that was potentially set in init() in order to correctly load here the worksheets without any theme used
	d->theme.clear();

	QXmlStreamAttributes attribs;
	QString str;

	while (!reader->atEnd()) {
		reader->readNext();
		if (reader->isEndElement() && reader->name() == QLatin1String("worksheet"))
			break;

		if (!reader->isStartElement())
			continue;

		if (reader->name() == QLatin1String("comment")) {
			if (!readCommentElement(reader))
				return false;
		} else if (!preview && reader->name() == QLatin1String("theme")) {
			attribs = reader->attributes();
			d->theme = attribs.value(QStringLiteral("name")).toString();
		} else if (!preview && reader->name() == QLatin1String("geometry")) {
			attribs = reader->attributes();

			str = attribs.value(QStringLiteral("x")).toString();
			if (str.isEmpty())
				reader->raiseMissingAttributeWarning(QStringLiteral("x"));
			else
				d->pageRect.setX(str.toDouble());

			str = attribs.value(QStringLiteral("y")).toString();
			if (str.isEmpty())
				reader->raiseMissingAttributeWarning(QStringLiteral("y"));
			else
				d->pageRect.setY(str.toDouble());

			str = attribs.value(QStringLiteral("width")).toString();
			if (str.isEmpty())
				reader->raiseMissingAttributeWarning(QStringLiteral("width"));
			else
				d->pageRect.setWidth(str.toDouble());

			str = attribs.value(QStringLiteral("height")).toString();
			if (str.isEmpty())
				reader->raiseMissingAttributeWarning(QStringLiteral("height"));
			else
				d->pageRect.setHeight(str.toDouble());

			READ_INT_VALUE("useViewSize", useViewSize, int);
			READ_INT_VALUE("zoomFit", zoomFit, ZoomFit);
		} else if (!preview && reader->name() == QLatin1String("layout")) {
			attribs = reader->attributes();

			READ_INT_VALUE("layout", layout, Worksheet::Layout);
			READ_DOUBLE_VALUE("topMargin", layoutTopMargin);
			READ_DOUBLE_VALUE("bottomMargin", layoutBottomMargin);
			READ_DOUBLE_VALUE("leftMargin", layoutLeftMargin);
			READ_DOUBLE_VALUE("rightMargin", layoutRightMargin);
			READ_DOUBLE_VALUE("verticalSpacing", layoutVerticalSpacing);
			READ_DOUBLE_VALUE("horizontalSpacing", layoutHorizontalSpacing);
			READ_INT_VALUE("columnCount", layoutColumnCount, int);
			READ_INT_VALUE("rowCount", layoutRowCount, int);
		} else if (!preview && reader->name() == QLatin1String("background"))
			d->background->load(reader, preview);
		else if (!preview && reader->name() == QLatin1String("plotProperties")) {
			attribs = reader->attributes();

			str = attribs.value(QStringLiteral("plotInteractive")).toString();
			if (str.isEmpty()) {
				str = attribs.value(QStringLiteral("plotLocked")).toString();
				if (str.isEmpty())
					reader->raiseMissingAttributeWarning(QStringLiteral("plotLocked"));
				else
					d->plotsInteractive = !static_cast<bool>(str.toInt());
			} else
				d->plotsInteractive = static_cast<bool>(str.toInt());

			READ_INT_VALUE("cartesianPlotActionMode", cartesianPlotActionMode, Worksheet::CartesianPlotActionMode);
			READ_INT_VALUE("cartesianPlotCursorMode", cartesianPlotCursorMode, Worksheet::CartesianPlotActionMode);
		} else if (reader->name() == QLatin1String("cartesianPlot")) {
			auto* plot = new CartesianPlot(QString(), true);
			plot->setIsLoading(true);
			if (!plot->load(reader, preview)) {
				delete plot;
				return false;
			} else
				addChildFast(plot);
		} else if (!preview && reader->name() == QLatin1String("textLabel")) {
			auto* label = new TextLabel(QString());
			label->setIsLoading(true);
			if (!label->load(reader, preview)) {
				delete label;
				return false;
			} else
				addChildFast(label);
		} else if (!preview && reader->name() == QLatin1String("image")) {
			Image* image = new Image(QString());
			image->setIsLoading(true);
			if (!image->load(reader, preview)) {
				delete image;
				return false;
			} else
				addChildFast(image);
		} else { // unknown element
			reader->raiseUnknownElementWarning();
			if (!reader->skipToEndElement())
				return false;
		}
	}

	if (!preview) {
		d->m_scene->setSceneRect(d->pageRect);
		d->updateLayout();
		updateCompleteCursorTreeModel();
	}

	return true;
}

// ##############################################################################
// #########################  Theme management ##################################
// ##############################################################################
void Worksheet::loadTheme(const QString& theme) {
	Q_D(Worksheet);
	KConfigGroup group;
	KConfig* config = nullptr;
	if (!theme.isEmpty()) {
		// load values from the theme config
		config = new KConfig(ThemeHandler::themeFilePath(theme), KConfig::SimpleConfig);

		// apply the same background color for Worksheet as for the CartesianPlot
		group = config->group(QStringLiteral("CartesianPlot"));

		// load the theme for all the children
		const auto& children = this->children<WorksheetElement>(ChildIndexFlag::IncludeHidden);
		for (auto* child : children)
			child->loadThemeConfig(*config);
	} else {
		// load default values
		config = new KConfig();
		group = config->group(QStringLiteral("Worksheet"));
	}

	// load background properties
	d->background->loadThemeConfig(group);

	// load the theme for all the children
	const auto& children = this->children<WorksheetElement>(ChildIndexFlag::IncludeHidden);
	for (auto* child : children)
		child->loadThemeConfig(*config);

	delete config;

	Q_EMIT changed();
}
