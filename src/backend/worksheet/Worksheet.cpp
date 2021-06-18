/***************************************************************************
    File                 : Worksheet.cpp
    Project              : LabPlot
    Description          : Worksheet
    --------------------------------------------------------------------
    Copyright            : (C) 2009 Tilman Benkert (thzs@gmx.net)
    Copyright            : (C) 2011-2020 by Alexander Semke (alexander.semke@web.de)
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

#include "Worksheet.h"
#include "WorksheetPrivate.h"
#include "WorksheetElement.h"
#include "commonfrontend/worksheet/WorksheetView.h"
#include "backend/core/Project.h"
#include "backend/worksheet/plots/cartesian/CartesianPlot.h"
#include "backend/worksheet/Image.h"
#include "backend/worksheet/TreeModel.h"
#include "backend/worksheet/TextLabel.h"
#include "backend/lib/commandtemplates.h"
#include "backend/lib/XmlStreamReader.h"
#include "kdefrontend/worksheet/ExportWorksheetDialog.h"
#include "kdefrontend/ThemeHandler.h"

#include <QPrinter>
#include <QPrintDialog>
#include <QPrintPreviewDialog>
#include <QDir>
#include <QGraphicsItem>
#include <QIcon>

#include <KConfig>
#include <KConfigGroup>
#include <KLocalizedString>
#include <KSharedConfig>

/**
 * \class Worksheet
 * \brief Top-level container for worksheet elements like plot, labels, etc.
 *
 * The worksheet is, besides the data containers \c Spreadsheet and \c Matrix, another central part of the application
 * and provides an area for showing and grouping together different kinds of worksheet objects - plots, labels &etc;
 *
 * * \ingroup worksheet
 */
Worksheet::Worksheet(const QString& name, bool loading) : AbstractPart(name, AspectType::Worksheet), d(new WorksheetPrivate(this)) {
	connect(this, &Worksheet::aspectAdded, this, &Worksheet::handleAspectAdded);
	connect(this, &Worksheet::aspectAboutToBeRemoved, this, &Worksheet::handleAspectAboutToBeRemoved);
	connect(this, &Worksheet::aspectRemoved, this, &Worksheet::handleAspectRemoved);

	if (!loading)
		init();
}

Worksheet::~Worksheet() {
	delete d;
}

void Worksheet::init() {
	KConfig config;
	KConfigGroup group = config.group("Worksheet");

	//size
	d->scaleContent = group.readEntry("ScaleContent", false);
	d->useViewSize = group.readEntry("UseViewSize", false);
	d->pageRect.setX(0);
	d->pageRect.setY(0);
	d->pageRect.setWidth(group.readEntry("Width", 1000));
	d->pageRect.setHeight(group.readEntry("Height", 1000));
	d->m_scene->setSceneRect(d->pageRect);

	//background
	d->backgroundFileName = group.readEntry("BackgroundFileName", QString());

	//layout
	d->layout = (Layout) group.readEntry("Layout", static_cast<int>(Layout::VerticalLayout));
	d->layoutTopMargin =  group.readEntry("LayoutTopMargin", convertToSceneUnits(0.5, Unit::Centimeter));
	d->layoutBottomMargin = group.readEntry("LayoutBottomMargin", convertToSceneUnits(0.5, Unit::Centimeter));
	d->layoutLeftMargin = group.readEntry("LayoutLeftMargin", convertToSceneUnits(0.5, Unit::Centimeter));
	d->layoutRightMargin = group.readEntry("LayoutRightMargin", convertToSceneUnits(0.5, Unit::Centimeter));
	d->layoutVerticalSpacing = group.readEntry("LayoutVerticalSpacing", convertToSceneUnits(0.5, Unit::Centimeter));
	d->layoutHorizontalSpacing = group.readEntry("LayoutHorizontalSpacing", convertToSceneUnits(0.5, Unit::Centimeter));
	d->layoutRowCount = group.readEntry("LayoutRowCount", 2);
	d->layoutColumnCount = group.readEntry("LayoutColumnCount", 2);

	//default theme
	KConfigGroup settings = KSharedConfig::openConfig()->group(QLatin1String("Settings_Worksheet"));
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
		return value * 25.4/72. * 10.;
	}

	return 0;
}

/*!
	converts from the scene units to \c unit . At the moment, 1 scene unit corresponds to 1/10 mm.
 */
double Worksheet::convertFromSceneUnits(const double value, const Worksheet::Unit unit) {
	switch (unit) {
	case Unit::Millimeter:
		return value/10.0;
	case Unit::Centimeter:
		return value/100.0;
	case Unit::Inch:
		return value/25.4/10.;
	case Unit::Point:
		return value/25.4/10.*72.;
	}

	return 0;
}

QIcon Worksheet::icon() const {
	return QIcon::fromTheme("labplot-worksheet");
}

/**
 * Return a new context menu. The caller takes ownership of the menu.
 */
QMenu* Worksheet::createContextMenu() {
	QMenu* menu = AbstractPart::createContextMenu();
	Q_ASSERT(menu);
	emit requestProjectContextMenu(menu);
	return menu;
}

//! Construct a primary view on me.
/**
 * This method may be called multiple times during the life time of an Aspect, or it might not get
 * called at all. Aspects must not depend on the existence of a view for their operation.
 */
QWidget* Worksheet::view() const {
	if (!m_partView) {
		m_view = new WorksheetView(const_cast<Worksheet*>(this));
		m_partView = m_view;
		connect(m_view, &WorksheetView::statusInfo, this, &Worksheet::statusInfo);
		connect(m_view, &WorksheetView::propertiesExplorerRequested, this, &Worksheet::propertiesExplorerRequested);
		connect(this, &Worksheet::cartesianPlotMouseModeChanged, m_view, &WorksheetView::cartesianPlotMouseModeChangedSlot);
	}
	return m_partView;
}

/*!
 * returns the list of all parent aspects (folders and sub-folders)
 * together with all the data containers required to plot the data in the worksheet
 */
QVector<AbstractAspect*> Worksheet::dependsOn() const {
	//add all parent aspects (folders and sub-folders)
	QVector<AbstractAspect*> aspects = AbstractAspect::dependsOn();

	//traverse all plots and add all data containers they depend on
	for (const auto* plot : children<AbstractPlot>())
		aspects << plot->dependsOn();

	return aspects;
}

QVector<AspectType> Worksheet::pasteTypes() const {
	return QVector<AspectType> {AspectType::CartesianPlot, AspectType::TextLabel, AspectType::Image};
}

bool Worksheet::exportView() const {
	auto* dlg = new ExportWorksheetDialog(m_view);
	dlg->setFileName(name());
	bool ret;
	if ( (ret = (dlg->exec() == QDialog::Accepted)) ) {
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
	return ret;
}

bool Worksheet::printView() {
	QPrinter printer;
	auto* dlg = new QPrintDialog(&printer, m_view);
	dlg->setWindowTitle(i18nc("@title:window", "Print Worksheet"));
	bool ret;
	if ( (ret = (dlg->exec() == QDialog::Accepted)) )
		m_view->print(&printer);

	delete dlg;
	return ret;
}

bool Worksheet::printPreview() const {
	auto* dlg = new QPrintPreviewDialog(m_view);
	connect(dlg, &QPrintPreviewDialog::paintRequested, m_view, &WorksheetView::print);
	return dlg->exec();
}

void Worksheet::handleAspectAdded(const AbstractAspect* aspect) {
	const auto* addedElement = qobject_cast<const WorksheetElement*>(aspect);
	if (!addedElement)
		return;

	if (aspect->parentAspect() != this)
		return;

	//add the GraphicsItem of the added child to the scene
	QGraphicsItem* item = addedElement->graphicsItem();
	d->m_scene->addItem(item);

	const CartesianPlot* plot = dynamic_cast<const CartesianPlot*>(aspect);
	if (plot) {
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
		connect(plot, static_cast<void (CartesianPlot::*)(QPen, QString)>(&CartesianPlot::curveLinePenChanged), this, &Worksheet::updateCurveBackground);
		connect(plot, &CartesianPlot::mouseModeChanged, this, &Worksheet::cartesianPlotMouseModeChangedSlot);
		auto* p = const_cast<CartesianPlot*>(plot);
		p->setLocked(d->plotsLocked);

		cursorModelPlotAdded(p->name());
	}
	qreal zVal = 0;
	for (auto* child : children<WorksheetElement>(ChildIndexFlag::IncludeHidden))
		child->graphicsItem()->setZValue(zVal++);

	//if a theme was selected in the worksheet, apply this theme for newly added children
	if (!d->theme.isEmpty() && !isLoading()) {
		KConfig config(ThemeHandler::themeFilePath(d->theme), KConfig::SimpleConfig);
		const_cast<WorksheetElement*>(addedElement)->loadThemeConfig(config);
	}

	//recalculated the layout
	if (!isLoading()) {
		if (d->layout != Worksheet::Layout::NoLayout)
			d->updateLayout(false);
	}
}

void Worksheet::handleAspectAboutToBeRemoved(const AbstractAspect* aspect) {
	const auto* removedElement = qobject_cast<const WorksheetElement*>(aspect);
	if (removedElement) {
		QGraphicsItem* item = removedElement->graphicsItem();
		d->m_scene->removeItem(item);
	}
}

void Worksheet::handleAspectRemoved(const AbstractAspect* parent, const AbstractAspect* before, const AbstractAspect* child) {
	Q_UNUSED(parent);
	Q_UNUSED(before);

	if (d->layout != Worksheet::Layout::NoLayout)
		d->updateLayout(false);
	auto* plot = dynamic_cast<const CartesianPlot*>(child);
	if (plot)
		cursorModelPlotRemoved(plot->name());
}

QGraphicsScene* Worksheet::scene() const {
	return d->m_scene;
}

QRectF Worksheet::pageRect() const {
	return d->m_scene->sceneRect();
}

/*!
	this slot is called when a worksheet element is selected in the project explorer.
	emits \c itemSelected() which forwards this event to the \c WorksheetView
	in order to select the corresponding \c QGraphicsItem.
 */
void Worksheet::childSelected(const AbstractAspect* aspect) {
	auto* element = qobject_cast<WorksheetElement*>(const_cast<AbstractAspect*>(aspect));
	if (element)
		emit itemSelected(element->graphicsItem());
}

/*!
	this slot is called when a worksheet element is deselected in the project explorer.
	emits \c itemDeselected() which forwards this event to \c WorksheetView
	in order to deselect the corresponding \c QGraphicsItem.
 */
void Worksheet::childDeselected(const AbstractAspect* aspect) {
	auto* element = qobject_cast<WorksheetElement*>(const_cast<AbstractAspect*>(aspect));
	if (element)
		emit itemDeselected(element->graphicsItem());
}

/*!
 *  Emits the signal to select or to deselect the aspect corresponding to \c QGraphicsItem \c item in the project explorer,
 *  if \c selected=true or \c selected=false, respectively.
 *  The signal is handled in \c AspectTreeModel and forwarded to the tree view in \c ProjectExplorer.
 * This function is called in \c WorksheetView upon selection changes.
 */
void Worksheet::setItemSelectedInView(const QGraphicsItem* item, const bool b) {
	//determine the corresponding aspect
	const AbstractAspect* aspect(nullptr);
	for (const auto* child : children<WorksheetElement>(ChildIndexFlag::IncludeHidden) ) {
		aspect = this->aspectFromGraphicsItem(child, item);
		if (aspect)
			break;
	}

	if (!aspect)
		return;

	//forward selection/deselection to AbstractTreeModel
	if (b)
		emit childAspectSelectedInView(aspect);
	else
		emit childAspectDeselectedInView(aspect);
}

/*!
 * helper function:  checks whether \c aspect or one of its children has the \c GraphicsItem \c item
 * Returns a pointer to \c WorksheetElement having this item.
 */
WorksheetElement* Worksheet::aspectFromGraphicsItem(const WorksheetElement* parent, const QGraphicsItem* item) const {
	if ( parent->graphicsItem() == item )
		return const_cast<WorksheetElement*>(parent);
	else {
		for (const auto* child : parent->children<WorksheetElement>(AbstractAspect::ChildIndexFlag::IncludeHidden) ) {
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
		emit childAspectSelectedInView(this);
	else
		emit childAspectDeselectedInView(this);
}

void Worksheet::deleteAspectFromGraphicsItem(const QGraphicsItem* item) {
	Q_ASSERT(item);
	//determine the corresponding aspect
	AbstractAspect* aspect(nullptr);
	for (const auto* child : children<WorksheetElement>(ChildIndexFlag::IncludeHidden) ) {
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
	return d->cursorData;
}

void Worksheet::update() {
	emit requestUpdate();
}

void Worksheet::setSuppressLayoutUpdate(bool value) {
	d->suppressLayoutUpdate = value;
}

void Worksheet::updateLayout() {
	d->updateLayout();
}

Worksheet::CartesianPlotActionMode Worksheet::cartesianPlotActionMode() {
	return d->cartesianPlotActionMode;
}

Worksheet::CartesianPlotActionMode Worksheet::cartesianPlotCursorMode() {
	return d->cartesianPlotCursorMode;
}

bool Worksheet::plotsLocked() {
	return d->plotsLocked;
}

void Worksheet::setCartesianPlotActionMode(Worksheet::CartesianPlotActionMode mode) {
	if (d->cartesianPlotActionMode == mode)
		return;

	d->cartesianPlotActionMode = mode;
	project()->setChanged(true);
}

void Worksheet::setCartesianPlotCursorMode(Worksheet::CartesianPlotActionMode mode) {
	if (d->cartesianPlotCursorMode == mode)
		return;

	d->cartesianPlotCursorMode = mode;

	if (mode == CartesianPlotActionMode::ApplyActionToAll) {
		d->suppressCursorPosChanged = true;
		QVector<CartesianPlot*> plots = children<CartesianPlot>();
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
	project()->setChanged(true);
}

void Worksheet::setInteractive(bool value) {
	if (!m_view)
		view();
	m_view->setInteractive(value);
}

void Worksheet::setPlotsLocked(bool lock) {
	if (d->plotsLocked == lock)
		return;

	d->plotsLocked = lock;

	for (auto* plot: children<CartesianPlot>())
		plot->setLocked(lock);

	project()->setChanged(true);
}

void Worksheet::registerShortcuts() {
	m_view->registerShortcuts();
}

WorksheetElement* Worksheet::currentSelection()
{
	if (!m_view)
		view();
	return m_view->selectedElement();
}

void Worksheet::unregisterShortcuts() {
	m_view->unregisterShortcuts();
}

/* =============================== getter methods for general options ==================================== */
BASIC_D_READER_IMPL(Worksheet, bool, scaleContent, scaleContent)
BASIC_D_READER_IMPL(Worksheet, bool, useViewSize, useViewSize)

/* =============================== getter methods for background options ================================= */
BASIC_D_READER_IMPL(Worksheet, PlotArea::BackgroundType, backgroundType, backgroundType)
BASIC_D_READER_IMPL(Worksheet, PlotArea::BackgroundColorStyle, backgroundColorStyle, backgroundColorStyle)
BASIC_D_READER_IMPL(Worksheet, PlotArea::BackgroundImageStyle, backgroundImageStyle, backgroundImageStyle)
BASIC_D_READER_IMPL(Worksheet, Qt::BrushStyle, backgroundBrushStyle, backgroundBrushStyle)
BASIC_D_READER_IMPL(Worksheet, QColor, backgroundFirstColor, backgroundFirstColor)
BASIC_D_READER_IMPL(Worksheet, QColor, backgroundSecondColor, backgroundSecondColor)
BASIC_D_READER_IMPL(Worksheet, QString, backgroundFileName, backgroundFileName)
BASIC_D_READER_IMPL(Worksheet, double, backgroundOpacity, backgroundOpacity)

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
void Worksheet::setUseViewSize(bool useViewSize) {
	if (useViewSize != d->useViewSize) {
		d->useViewSize = useViewSize;
		emit useViewSizeRequested();
	}
}

STD_SETTER_CMD_IMPL_S(Worksheet, SetScaleContent, bool, scaleContent)
void Worksheet::setScaleContent(bool scaleContent) {
	if (scaleContent != d->scaleContent)
		exec(new WorksheetSetScaleContentCmd(d, scaleContent, ki18n("%1: change \"rescale the content\" property")));
}

/* ============================ setter methods and undo commands  for background options  ================= */
STD_SETTER_CMD_IMPL_F_S(Worksheet, SetBackgroundType, PlotArea::BackgroundType, backgroundType, update)
void Worksheet::setBackgroundType(PlotArea::BackgroundType type) {
	if (type != d->backgroundType)
		exec(new WorksheetSetBackgroundTypeCmd(d, type, ki18n("%1: background type changed")));
}

STD_SETTER_CMD_IMPL_F_S(Worksheet, SetBackgroundColorStyle, PlotArea::BackgroundColorStyle, backgroundColorStyle, update)
void Worksheet::setBackgroundColorStyle(PlotArea::BackgroundColorStyle style) {
	if (style != d->backgroundColorStyle)
		exec(new WorksheetSetBackgroundColorStyleCmd(d, style, ki18n("%1: background color style changed")));
}

STD_SETTER_CMD_IMPL_F_S(Worksheet, SetBackgroundImageStyle, PlotArea::BackgroundImageStyle, backgroundImageStyle, update)
void Worksheet::setBackgroundImageStyle(PlotArea::BackgroundImageStyle style) {
	if (style != d->backgroundImageStyle)
		exec(new WorksheetSetBackgroundImageStyleCmd(d, style, ki18n("%1: background image style changed")));
}

STD_SETTER_CMD_IMPL_F_S(Worksheet, SetBackgroundBrushStyle, Qt::BrushStyle, backgroundBrushStyle, update)
void Worksheet::setBackgroundBrushStyle(Qt::BrushStyle style) {
	if (style != d->backgroundBrushStyle)
		exec(new WorksheetSetBackgroundBrushStyleCmd(d, style, ki18n("%1: background brush style changed")));
}

STD_SETTER_CMD_IMPL_F_S(Worksheet, SetBackgroundFirstColor, QColor, backgroundFirstColor, update)
void Worksheet::setBackgroundFirstColor(const QColor &color) {
	if (color!= d->backgroundFirstColor)
		exec(new WorksheetSetBackgroundFirstColorCmd(d, color, ki18n("%1: set background first color")));
}

STD_SETTER_CMD_IMPL_F_S(Worksheet, SetBackgroundSecondColor, QColor, backgroundSecondColor, update)
void Worksheet::setBackgroundSecondColor(const QColor &color) {
	if (color!= d->backgroundSecondColor)
		exec(new WorksheetSetBackgroundSecondColorCmd(d, color, ki18n("%1: set background second color")));
}

STD_SETTER_CMD_IMPL_F_S(Worksheet, SetBackgroundFileName, QString, backgroundFileName, update)
void Worksheet::setBackgroundFileName(const QString& fileName) {
	if (fileName!= d->backgroundFileName)
		exec(new WorksheetSetBackgroundFileNameCmd(d, fileName, ki18n("%1: set background image")));
}

STD_SETTER_CMD_IMPL_F_S(Worksheet, SetBackgroundOpacity, double, backgroundOpacity, update)
void Worksheet::setBackgroundOpacity(double opacity) {
	if (opacity != d->backgroundOpacity)
		exec(new WorksheetSetBackgroundOpacityCmd(d, opacity, ki18n("%1: set opacity")));
}

/* ============================ setter methods and undo commands  for layout options  ================= */
STD_SETTER_CMD_IMPL_F_S(Worksheet, SetLayout, Worksheet::Layout, layout, updateLayout)
void Worksheet::setLayout(Worksheet::Layout layout) {
	if (layout != d->layout) {
		beginMacro(i18n("%1: set layout", name()));
		exec(new WorksheetSetLayoutCmd(d, layout, ki18n("%1: set layout")));
		endMacro();
	}
}

STD_SETTER_CMD_IMPL_M_F_S(Worksheet, SetLayoutTopMargin, double, layoutTopMargin, updateLayout)
void Worksheet::setLayoutTopMargin(double margin) {
	if (margin != d->layoutTopMargin) {
		beginMacro(i18n("%1: set layout top margin", name()));
		exec(new WorksheetSetLayoutTopMarginCmd(d, margin, ki18n("%1: set layout top margin")));
		endMacro();
	}
}

STD_SETTER_CMD_IMPL_M_F_S(Worksheet, SetLayoutBottomMargin, double, layoutBottomMargin, updateLayout)
void Worksheet::setLayoutBottomMargin(double margin) {
	if (margin != d->layoutBottomMargin) {
		beginMacro(i18n("%1: set layout bottom margin", name()));
		exec(new WorksheetSetLayoutBottomMarginCmd(d, margin, ki18n("%1: set layout bottom margin")));
		endMacro();
	}
}

STD_SETTER_CMD_IMPL_M_F_S(Worksheet, SetLayoutLeftMargin, double, layoutLeftMargin, updateLayout)
void Worksheet::setLayoutLeftMargin(double margin) {
	if (margin != d->layoutLeftMargin) {
		beginMacro(i18n("%1: set layout left margin", name()));
		exec(new WorksheetSetLayoutLeftMarginCmd(d, margin, ki18n("%1: set layout left margin")));
		endMacro();
	}
}

STD_SETTER_CMD_IMPL_M_F_S(Worksheet, SetLayoutRightMargin, double, layoutRightMargin, updateLayout)
void Worksheet::setLayoutRightMargin(double margin) {
	if (margin != d->layoutRightMargin) {
		beginMacro(i18n("%1: set layout right margin", name()));
		exec(new WorksheetSetLayoutRightMarginCmd(d, margin, ki18n("%1: set layout right margin")));
		endMacro();
	}
}

STD_SETTER_CMD_IMPL_M_F_S(Worksheet, SetLayoutVerticalSpacing, double, layoutVerticalSpacing, updateLayout)
void Worksheet::setLayoutVerticalSpacing(double spacing) {
	if (spacing != d->layoutVerticalSpacing) {
		beginMacro(i18n("%1: set layout vertical spacing", name()));
		exec(new WorksheetSetLayoutVerticalSpacingCmd(d, spacing, ki18n("%1: set layout vertical spacing")));
		endMacro();
	}
}

STD_SETTER_CMD_IMPL_M_F_S(Worksheet, SetLayoutHorizontalSpacing, double, layoutHorizontalSpacing, updateLayout)
void Worksheet::setLayoutHorizontalSpacing(double spacing) {
	if (spacing != d->layoutHorizontalSpacing) {
		beginMacro(i18n("%1: set layout horizontal spacing", name()));
		exec(new WorksheetSetLayoutHorizontalSpacingCmd(d, spacing, ki18n("%1: set layout horizontal spacing")));
		endMacro();
	}
}

STD_SETTER_CMD_IMPL_M_F_S(Worksheet, SetLayoutRowCount, int, layoutRowCount, updateLayout)
void Worksheet::setLayoutRowCount(int count) {
	if (count != d->layoutRowCount) {
		beginMacro(i18n("%1: set layout row count", name()));
		exec(new WorksheetSetLayoutRowCountCmd(d, count, ki18n("%1: set layout row count")));
		endMacro();
	}
}

STD_SETTER_CMD_IMPL_M_F_S(Worksheet, SetLayoutColumnCount, int, layoutColumnCount, updateLayout)
void Worksheet::setLayoutColumnCount(int count) {
	if (count != d->layoutColumnCount) {
		beginMacro(i18n("%1: set layout column count", name()));
		exec(new WorksheetSetLayoutColumnCountCmd(d, count, ki18n("%1: set layout column count")));
		endMacro();
	}
}

class WorksheetSetPageRectCmd : public StandardMacroSetterCmd<Worksheet::Private, QRectF> {
public:
	WorksheetSetPageRectCmd(Worksheet::Private* target, QRectF newValue, const KLocalizedString& description)
		: StandardMacroSetterCmd<Worksheet::Private, QRectF>(target, &Worksheet::Private::pageRect, newValue, description) {}
	void finalize() override {
		m_target->updatePageRect();
		emit m_target->q->pageRectChanged(m_target->*m_field);
	}
	void finalizeUndo() override {
		m_target->m_scene->setSceneRect(m_target->*m_field);
		emit m_target->q->pageRectChanged(m_target->*m_field);
	}
};

void Worksheet::setPageRect(const QRectF& rect) {
	//don't allow any rectangulars of width/height equal to zero
	if (qFuzzyCompare(rect.width(), 0.) || qFuzzyCompare(rect.height(), 0.)) {
		emit pageRectChanged(d->pageRect);
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
			emit pageRectChanged(d->pageRect);
		}
	}
}

void Worksheet::setPrinting(bool on) const {
	QVector<WorksheetElement*> childElements = children<WorksheetElement>(AbstractAspect::ChildIndexFlag::Recursive | AbstractAspect::ChildIndexFlag::IncludeHidden);
	for (auto* child : childElements)
		child->setPrinting(on);
}

STD_SETTER_CMD_IMPL_S(Worksheet, SetTheme, QString, theme)
void Worksheet::setTheme(const QString& theme) {
	if (theme != d->theme) {
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
}

int Worksheet::cSystemIndex(WorksheetElement* e)
{
	if (!e) // TODO: really needed?
		return -1;

	auto type = e->type();

	if (type == AspectType::CartesianPlot)
		return -1;
	else if (((int)type & (int)AspectType::XYCurve) == (int)AspectType::XYCurve || // All subclasses of xycurve
			 type == AspectType::Axis)
		return e->coordinateSystemIndex();
	return -1;
}

void Worksheet::cartesianPlotMousePressZoomSelectionMode(QPointF logicPos) {
	auto senderPlot = static_cast<CartesianPlot*>(QObject::sender());
	auto mouseMode = senderPlot->mouseMode();
	auto actionMode = cartesianPlotActionMode();
	if (actionMode == CartesianPlotActionMode::ApplyActionToAll) {
		auto plots = children<CartesianPlot>(AbstractAspect::ChildIndexFlag::Recursive | AbstractAspect::ChildIndexFlag::IncludeHidden);
		for (auto* plot : plots)
			plot->mousePressZoomSelectionMode(logicPos, -1);
	} else if ((actionMode == CartesianPlotActionMode::ApplyActionToAllX && mouseMode != CartesianPlot::MouseMode::ZoomYSelection) ||
	           (actionMode == CartesianPlotActionMode::ApplyActionToAllY && mouseMode != CartesianPlot::MouseMode::ZoomXSelection)) {
		auto plots = children<CartesianPlot>(AbstractAspect::ChildIndexFlag::Recursive | AbstractAspect::ChildIndexFlag::IncludeHidden);
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
		int index = cSystemIndex(m_view->selectedElement());
		senderPlot->mousePressZoomSelectionMode(logicPos, index);
	}
}

void Worksheet::cartesianPlotMouseReleaseZoomSelectionMode() {
	auto senderPlot = static_cast<CartesianPlot*>(QObject::sender());
	auto mouseMode = senderPlot->mouseMode();
	auto actionMode = cartesianPlotActionMode();
	if (actionMode == CartesianPlotActionMode::ApplyActionToAll ||
	        (actionMode == CartesianPlotActionMode::ApplyActionToAllX && mouseMode != CartesianPlot::MouseMode::ZoomYSelection) ||
	        (actionMode == CartesianPlotActionMode::ApplyActionToAllY && mouseMode != CartesianPlot::MouseMode::ZoomXSelection )) {
		auto plots = children<CartesianPlot>(AbstractAspect::ChildIndexFlag::Recursive | AbstractAspect::ChildIndexFlag::IncludeHidden);
		for (auto* plot : plots) {
			plot->mouseReleaseZoomSelectionMode(-1);
			plot->setMouseMode(mouseMode);
		}
	} else {
		int index = cSystemIndex(m_view->selectedElement());
		CartesianPlot* plot = static_cast<CartesianPlot*>(QObject::sender());
		plot->mouseReleaseZoomSelectionMode(index);
	}
}

void Worksheet::cartesianPlotMousePressCursorMode(int cursorNumber, QPointF logicPos) {
	if (cartesianPlotCursorMode() == CartesianPlotActionMode::ApplyActionToAll) {
		auto plots = children<CartesianPlot>(AbstractAspect::ChildIndexFlag::Recursive | AbstractAspect::ChildIndexFlag::IncludeHidden);
		for (auto* plot : plots)
			plot->mousePressCursorMode(cursorNumber, logicPos);
	} else {
		CartesianPlot* plot = static_cast<CartesianPlot*>(QObject::sender());
		plot->mousePressCursorMode(cursorNumber, logicPos);
	}

	cursorPosChanged(cursorNumber, logicPos.x());
}

void Worksheet::cartesianPlotMouseMoveZoomSelectionMode(QPointF logicPos) {
	auto senderPlot = static_cast<CartesianPlot*>(QObject::sender());
	auto actionMode = cartesianPlotActionMode();
	auto mouseMode = senderPlot->mouseMode();
	if (actionMode == CartesianPlotActionMode::ApplyActionToAll ||
	        (actionMode == CartesianPlotActionMode::ApplyActionToAllX && mouseMode != CartesianPlot::MouseMode::ZoomYSelection) ||
	        (actionMode == CartesianPlotActionMode::ApplyActionToAllY && mouseMode != CartesianPlot::MouseMode::ZoomXSelection )) {
		QVector<CartesianPlot*> plots = children<CartesianPlot>(AbstractAspect::ChildIndexFlag::Recursive | AbstractAspect::ChildIndexFlag::IncludeHidden);
		for (auto* plot : plots)
			plot->mouseMoveZoomSelectionMode(logicPos, -1);
	}  else
		senderPlot->mouseMoveZoomSelectionMode(logicPos, cSystemIndex(m_view->selectedElement()));
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
	if (actionMode == CartesianPlotActionMode::ApplyActionToAll ||
	        (actionMode == CartesianPlotActionMode::ApplyActionToAllX && mouseMode != CartesianPlot::MouseMode::ZoomYSelection) ||
	        (actionMode == CartesianPlotActionMode::ApplyActionToAllY && mouseMode != CartesianPlot::MouseMode::ZoomXSelection )) {
		auto plots = children<CartesianPlot>(AbstractAspect::ChildIndexFlag::Recursive | AbstractAspect::ChildIndexFlag::IncludeHidden);
		for (auto* plot : plots)
			plot->mouseHoverZoomSelectionMode(logicPos, -1);
	} else
		senderPlot->mouseHoverZoomSelectionMode(logicPos, cSystemIndex(m_view->selectedElement()));
}

void Worksheet::cartesianPlotMouseHoverOutsideDataRect() {
	auto senderPlot = static_cast<CartesianPlot*>(QObject::sender());
	auto mode = cartesianPlotActionMode();
	auto mouseMode = senderPlot->mouseMode();
	if (cartesianPlotActionMode() == CartesianPlotActionMode::ApplyActionToAll ||
	        (mode == CartesianPlotActionMode::ApplyActionToAllX && mouseMode != CartesianPlot::MouseMode::ZoomYSelection) ||
	        (mode == CartesianPlotActionMode::ApplyActionToAllY && mouseMode != CartesianPlot::MouseMode::ZoomXSelection )) {
		auto plots = children<CartesianPlot>(AbstractAspect::ChildIndexFlag::Recursive | AbstractAspect::ChildIndexFlag::IncludeHidden);
		for (auto* plot : plots)
			plot->mouseHoverOutsideDataRect();
	} else
		senderPlot->mouseHoverOutsideDataRect();
}

void Worksheet::cartesianPlotMouseMoveCursorMode(int cursorNumber, QPointF logicPos) {
	if (cartesianPlotCursorMode() == CartesianPlotActionMode::ApplyActionToAll) {
		auto plots = children<CartesianPlot>(AbstractAspect::ChildIndexFlag::Recursive | AbstractAspect::ChildIndexFlag::IncludeHidden);
		for (auto* plot : plots)
			plot->mouseMoveCursorMode(cursorNumber, logicPos);
	} else {
		CartesianPlot* plot = static_cast<CartesianPlot*>(QObject::sender());
		plot->mouseMoveCursorMode(cursorNumber, logicPos);
	}

	cursorPosChanged(cursorNumber, logicPos.x());
}

/*!
 * \brief Worksheet::cursorPosChanged
 * Updates the cursor treemodel with the new data
 * \param xPos: new position of the cursor
 * It is assumed, that the plots/curves are in the same order than receiving from
 * the children() function. It's not checked if the names are the same
 */
void Worksheet::cursorPosChanged(int cursorNumber, double xPos) {
	if (d->suppressCursorPosChanged)
		return;

	auto* sender = dynamic_cast<CartesianPlot*>(QObject::sender());
	if (!sender)
		return;

	TreeModel* treeModel = cursorModel();

	// if ApplyActionToSelection, each plot has it's own x value
	if (cartesianPlotCursorMode() == CartesianPlotActionMode::ApplyActionToAll) {
		// x values
		int rowPlot = 1;
		QModelIndex xName = treeModel->index(0, static_cast<int>(WorksheetPrivate::TreeModelColumn::SIGNALNAME));
		treeModel->setData(xName, QVariant("X"));
		double valueCursor[2];
		for (int i = 0; i < 2; i++) { // need both cursors to calculate diff
			valueCursor[i] = sender->cursorPos(i);
			treeModel->setTreeData(QVariant(valueCursor[i]), 0, static_cast<int>(WorksheetPrivate::TreeModelColumn::CURSOR0) + i);

		}
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
					treeModel->setTreeData(QVariant(valueCursor1 - value), rowCurve, static_cast<int>(WorksheetPrivate::TreeModelColumn::CURSORDIFF), plotIndex);
				} else {
					treeModel->setTreeData(QVariant(value), rowCurve, static_cast<int>(WorksheetPrivate::TreeModelColumn::CURSOR1), plotIndex);
					double valueCursor0 = treeModel->treeData(rowCurve, static_cast<int>(WorksheetPrivate::TreeModelColumn::CURSOR0), plotIndex).toDouble();
					treeModel->setTreeData(QVariant(value - valueCursor0), rowCurve, static_cast<int>(WorksheetPrivate::TreeModelColumn::CURSORDIFF), plotIndex);
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
			treeModel->setTreeData(QVariant("X"), 0, static_cast<int>(WorksheetPrivate::TreeModelColumn::SIGNALNAME), plotIndex);
			double valueCursor[2];
			for (int i = 0; i < 2; i++) { // need both cursors to calculate diff
				valueCursor[i] = sender->cursorPos(i);
				treeModel->setTreeData(QVariant(valueCursor[i]), 0, static_cast<int>(WorksheetPrivate::TreeModelColumn::CURSOR0) + i, plotIndex);
			}
			treeModel->setTreeData(QVariant(valueCursor[1]-valueCursor[0]), 0, static_cast<int>(WorksheetPrivate::TreeModelColumn::CURSORDIFF), plotIndex);

			// y values
			int rowCurve = 1; // first is x value
			for (int j = 0; j< sender->curveCount(); j++) { // j=0 are the x values

				const XYCurve* curve = sender->getCurve(j); // -1 because we start with 1 for the x axis
				if (!curve->isVisible())
					continue;

				// assumption: index of signals in model is the same than the index of the signal in the plot
				bool valueFound;

				double value = curve->y(xPos, valueFound);
				if (cursorNumber == 0) {
					treeModel->setTreeData(QVariant(value), rowCurve, static_cast<int>(WorksheetPrivate::TreeModelColumn::CURSOR0), plotIndex);
					double valueCursor1 = treeModel->treeData(rowCurve, static_cast<int>(WorksheetPrivate::TreeModelColumn::CURSOR1), plotIndex).toDouble();
					treeModel->setTreeData(QVariant(valueCursor1 - value), rowCurve, static_cast<int>(WorksheetPrivate::TreeModelColumn::CURSORDIFF), plotIndex);
				} else {
					treeModel->setTreeData(QVariant(value), rowCurve, static_cast<int>(WorksheetPrivate::TreeModelColumn::CURSOR1), plotIndex);
					double valueCursor0 = treeModel->treeData(rowCurve, static_cast<int>(WorksheetPrivate::TreeModelColumn::CURSOR0), plotIndex).toDouble();
					treeModel->setTreeData(QVariant(value - valueCursor0), rowCurve, static_cast<int>(WorksheetPrivate::TreeModelColumn::CURSORDIFF), plotIndex);
				}
				rowCurve++;
			}
		}
	}
}

void Worksheet::cursorModelPlotAdded(const QString& name) {
	Q_UNUSED(name);
//	TreeModel* treeModel = cursorModel();
//	int rowCount = treeModel->rowCount();
//	// add plot at the end
//	treeModel->insertRows(rowCount, 1); // add empty rows. Then they become filled
//	treeModel->setTreeData(QVariant(name), rowCount, WorksheetPrivate::TreeModelColumn::PLOTNAME); // rowCount instead of rowCount -1 because first row is the x value
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
	if (d->updateCompleteCursorModel) {
		updateCompleteCursorTreeModel();
		d->updateCompleteCursorModel = false;
	}

	emit cartesianPlotMouseModeChanged(mode);
}

void Worksheet::curveDataChanged(const XYCurve* curve) {
	auto* plot = dynamic_cast<CartesianPlot*>(QObject::sender());
	if (!plot)
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

			treeModel->setTreeData(QVariant(valueCursor1-valueCursor0), j, static_cast<int>(WorksheetPrivate::TreeModelColumn::CURSORDIFF), plotIndex);
			break;
		}
		break;
	}
}

void Worksheet::curveAdded(const XYCurve* curve) {
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
					row ++;
				continue;
			}

			treeModel->insertRow(row, plotIndex);

			treeModel->setTreeData(QVariant(curve->name()), row, static_cast<int>(WorksheetPrivate::TreeModelColumn::SIGNALNAME), plotIndex);
			QColor curveColor = curve->linePen().color();
			curveColor.setAlpha(50);
			treeModel->setTreeData(QVariant(curveColor),row, static_cast<int>(WorksheetPrivate::TreeModelColumn::SIGNALNAME), plotIndex, Qt::BackgroundRole);
			bool valueFound;
			double valueCursor0 = curve->y(plot->cursorPos(0), valueFound);
			treeModel->setTreeData(QVariant(valueCursor0), row, static_cast<int>(WorksheetPrivate::TreeModelColumn::CURSOR0), plotIndex);

			double valueCursor1 = curve->y(plot->cursorPos(1), valueFound);
			treeModel->setTreeData(QVariant(valueCursor1), row, static_cast<int>(WorksheetPrivate::TreeModelColumn::CURSOR1), plotIndex);

			treeModel->setTreeData(QVariant(valueCursor1-valueCursor0), row, static_cast<int>(WorksheetPrivate::TreeModelColumn::CURSORDIFF), plotIndex);
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
void Worksheet::updateCurveBackground(const QPen& pen, const QString& curveName) {
	const CartesianPlot* plot = static_cast<const CartesianPlot*>(QObject::sender());
	TreeModel* treeModel = cursorModel();
	int rowCount = treeModel->rowCount();

	for (int i = 0; i < rowCount; i++) {
		QModelIndex plotIndex = treeModel->index(i, static_cast<int>(WorksheetPrivate::TreeModelColumn::PLOTNAME));
		if (plotIndex.data().toString().compare(plot->name()) != 0)
			continue;

		int curveCount = treeModel->rowCount(plotIndex);
		for (int j = 0; j < curveCount; j++) {
			QModelIndex curveIndex = treeModel->index(j, static_cast<int>(WorksheetPrivate::TreeModelColumn::SIGNALNAME),
			                         plotIndex);

			if (curveIndex.data().toString().compare(curveName) != 0)
				continue;

			QColor curveColor = pen.color();
			curveColor.setAlpha(50);
			treeModel->setTreeData(QVariant(curveColor), j,
			                       static_cast<int>(WorksheetPrivate::TreeModelColumn::SIGNALNAME),
			                       plotIndex, Qt::BackgroundRole);
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
		treeModel->setData(xName, QVariant("X"));
		auto* plot0 = plot(0);
		if (plot0) {
			double valueCursor[2];
			for (int i = 0; i < 2; i++) {
				valueCursor[i] = plot0->cursorPos(i);
				QModelIndex cursor = treeModel->index(0, static_cast<int>(WorksheetPrivate::TreeModelColumn::CURSOR0) + i);

				treeModel->setData(cursor, QVariant(valueCursor[i]));
			}
			QModelIndex diff = treeModel->index(0, static_cast<int>(WorksheetPrivate::TreeModelColumn::CURSORDIFF));
			treeModel->setData(diff, QVariant(valueCursor[1]-valueCursor[0]));
		}
	} else {
		//treeModel->insertRows(0, plotCount, treeModel->index(0,0)); // add empty rows. Then they become filled
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
			treeModel->setData(xName, QVariant("X"));
			double valueCursor[2];
			for (int i = 0; i < 2; i++) {
				valueCursor[i] = p->cursorPos(i);
				QModelIndex cursor = treeModel->index(0, static_cast<int>(WorksheetPrivate::TreeModelColumn::CURSOR0) + i, plotName);

				treeModel->setData(cursor, QVariant(valueCursor[i]));
			}
			QModelIndex diff = treeModel->index(0, static_cast<int>(WorksheetPrivate::TreeModelColumn::CURSORDIFF), plotName);
			treeModel->setData(diff, QVariant(valueCursor[1]-valueCursor[0]));
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
				cursorValue[k] = curve->y(xPos,valueFound);
			}
			treeModel->insertRows(rowCurve, 1, plotName);
			QColor curveColor = curve->linePen().color();
			curveColor.setAlpha(50);
			treeModel->setTreeData(QVariant(curveColor), rowCurve, 0, plotName, Qt::BackgroundRole);
			treeModel->setTreeData(QVariant(curve->name()), rowCurve, static_cast<int>(WorksheetPrivate::TreeModelColumn::SIGNALNAME), plotName);
			treeModel->setTreeData(QVariant(cursorValue[0]), rowCurve, static_cast<int>(WorksheetPrivate::TreeModelColumn::CURSOR0), plotName);
			treeModel->setTreeData(QVariant(cursorValue[1]), rowCurve, static_cast<int>(WorksheetPrivate::TreeModelColumn::CURSOR1), plotName);
			treeModel->setTreeData(QVariant(cursorValue[1] - cursorValue[0]), rowCurve, static_cast<int>(WorksheetPrivate::TreeModelColumn::CURSORDIFF), plotName);
			rowCurve++;
		}
	}
}

//##############################################################################
//######################  Private implementation ###############################
//##############################################################################
WorksheetPrivate::WorksheetPrivate(Worksheet* owner) : q(owner), m_scene(new QGraphicsScene()) {
	QStringList headers = {i18n("Curves"), "V1", "V2", "V2-V1"};
	cursorData = new TreeModel(headers, nullptr);
}

QString WorksheetPrivate::name() const {
	return q->name();
}

/*!
 * called if the worksheet page (the actual size of worksheet's rectangular) was changed.
 * if a layout is active, it is is updated - this adjusts the sizes of the elements in the layout to the new page size.
 * if no layout is active and the option "scale content" is active, \c handleResize() is called to adjust zhe properties.
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
			QVector<WorksheetElement*> childElements = q->children<WorksheetElement>(AbstractAspect::ChildIndexFlag::IncludeHidden);
			if (useViewSize) {
				//don't make the change of the geometry undoable/redoable if the view size is used.
				for (auto* elem : childElements) {
					elem->setUndoAware(false);
					elem->handleResize(horizontalRatio, verticalRatio, true);
					elem->setUndoAware(true);
				}
			} else {
// 				for (auto* child : childElements)
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
}

void WorksheetPrivate::updateLayout(bool undoable) {
	if (suppressLayoutUpdate)
		return;

	auto list = q->children<WorksheetElementContainer>();
	int count = list.count();
	if (count == 0)
		return;

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
		h = (m_scene->sceneRect().height()-layoutTopMargin-layoutBottomMargin- (count-1)*layoutVerticalSpacing)/count;
		for (auto* elem : list) {
			setContainerRect(elem, x, y, h, w, undoable);
			y += h + layoutVerticalSpacing;
		}
	} else if (layout == Worksheet::Layout::HorizontalLayout) {
		w = (m_scene->sceneRect().width()-layoutLeftMargin-layoutRightMargin- (count-1)*layoutHorizontalSpacing)/count;
		h = m_scene->sceneRect().height() - layoutTopMargin-layoutBottomMargin;
		for (auto* elem : list) {
			setContainerRect(elem, x, y, h, w, undoable);
			x += w + layoutHorizontalSpacing;
		}
	} else { //GridLayout
		//add new rows, if not sufficient
		if (count > layoutRowCount*layoutColumnCount) {
			layoutRowCount = floor( (double)count/layoutColumnCount + 0.5);
			emit q->layoutRowCountChanged(layoutRowCount);
		}

		w = (m_scene->sceneRect().width()-layoutLeftMargin-layoutRightMargin- (layoutColumnCount-1)*layoutHorizontalSpacing)/layoutColumnCount;
		h = (m_scene->sceneRect().height()-layoutTopMargin-layoutBottomMargin- (layoutRowCount-1)*layoutVerticalSpacing)/layoutRowCount;
		int columnIndex = 0; //counts the columns in a row
		for (auto* elem : list) {
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
}

void WorksheetPrivate::setContainerRect(WorksheetElementContainer* elem, double x, double y, double h, double w, bool undoable) {
	if (useViewSize) {
		//when using the view size, no need to put rect changes onto the undo-stack
		elem->setUndoAware(false);
		elem->setRect(QRectF(x, y, w, h));
		elem->setUndoAware(true);
	} else {
		//don't put rect changed onto the undo-stack if undoable-flag is set to true,
		//e.g. when new child is added or removed (the layout and the childrend rects will be updated anyway)
		if (!undoable) {
			elem->setUndoAware(false);
			elem->setRect(QRectF(x, y, w, h));
			elem->setUndoAware(true);
		} else
			elem->setRect(QRectF(x, y, w, h));
	}
	elem->graphicsItem()->setFlag(QGraphicsItem::ItemIsMovable, false);
}

//##############################################################################
//##################  Serialization/Deserialization  ###########################
//##############################################################################

//! Save as XML
void Worksheet::save(QXmlStreamWriter* writer) const {
	writer->writeStartElement( "worksheet" );
	writeBasicAttributes(writer);
	writeCommentElement(writer);

	//applied theme
	if (!d->theme.isEmpty()) {
		writer->writeStartElement( "theme" );
		writer->writeAttribute("name", d->theme);
		writer->writeEndElement();
	}

	//geometry
	writer->writeStartElement( "geometry" );
	QRectF rect = d->m_scene->sceneRect();
	writer->writeAttribute( "x", QString::number(rect.x()) );
	writer->writeAttribute( "y", QString::number(rect.y()) );
	writer->writeAttribute( "width", QString::number(rect.width()) );
	writer->writeAttribute( "height", QString::number(rect.height()) );
	writer->writeAttribute( "useViewSize", QString::number(d->useViewSize) );
	writer->writeEndElement();

	//layout
	writer->writeStartElement( "layout" );
	writer->writeAttribute( "layout", QString::number(static_cast<int>(d->layout)) );
	writer->writeAttribute( "topMargin", QString::number(d->layoutTopMargin) );
	writer->writeAttribute( "bottomMargin", QString::number(d->layoutBottomMargin) );
	writer->writeAttribute( "leftMargin",  QString::number(d->layoutLeftMargin) );
	writer->writeAttribute( "rightMargin", QString::number(d->layoutRightMargin) );
	writer->writeAttribute( "verticalSpacing", QString::number(d->layoutVerticalSpacing) );
	writer->writeAttribute( "horizontalSpacing", QString::number(d->layoutHorizontalSpacing) );
	writer->writeAttribute( "columnCount", QString::number(d->layoutColumnCount) );
	writer->writeAttribute( "rowCount", QString::number(d->layoutRowCount) );
	writer->writeEndElement();

	//background properties
	writer->writeStartElement( "background" );
	writer->writeAttribute( "type", QString::number(static_cast<int>(d->backgroundType)) );
	writer->writeAttribute( "colorStyle", QString::number(static_cast<int>(d->backgroundColorStyle)) );
	writer->writeAttribute( "imageStyle", QString::number(static_cast<int>(d->backgroundImageStyle)) );
	writer->writeAttribute( "brushStyle", QString::number(d->backgroundBrushStyle) );
	writer->writeAttribute( "firstColor_r", QString::number(d->backgroundFirstColor.red()) );
	writer->writeAttribute( "firstColor_g", QString::number(d->backgroundFirstColor.green()) );
	writer->writeAttribute( "firstColor_b", QString::number(d->backgroundFirstColor.blue()) );
	writer->writeAttribute( "secondColor_r", QString::number(d->backgroundSecondColor.red()) );
	writer->writeAttribute( "secondColor_g", QString::number(d->backgroundSecondColor.green()) );
	writer->writeAttribute( "secondColor_b", QString::number(d->backgroundSecondColor.blue()) );
	writer->writeAttribute( "fileName", d->backgroundFileName );
	writer->writeAttribute( "opacity", QString::number(d->backgroundOpacity) );
	writer->writeEndElement();

	// cartesian properties
	writer->writeStartElement( "plotProperties" );
	writer->writeAttribute( "plotsLocked", QString::number(d->plotsLocked) );
	writer->writeAttribute( "cartesianPlotActionMode", QString::number(static_cast<int>(d->cartesianPlotActionMode)));
	writer->writeAttribute( "cartesianPlotCursorMode", QString::number(static_cast<int>(d->cartesianPlotCursorMode)));
	writer->writeEndElement();

	//serialize all children
	for (auto* child : children<WorksheetElement>(ChildIndexFlag::IncludeHidden))
		child->save(writer);

	writer->writeEndElement(); // close "worksheet" section
}

//! Load from XML
bool Worksheet::load(XmlStreamReader* reader, bool preview) {
	if (!readBasicAttributes(reader))
		return false;

	//clear the theme that was potentially set in init() in order to correctly load here the worksheets without any theme used
	d->theme.clear();

	KLocalizedString attributeWarning = ki18n("Attribute '%1' missing or empty, default value is used");
	QXmlStreamAttributes attribs;
	QString str;
	QRectF rect;

	while (!reader->atEnd()) {
		reader->readNext();
		if (reader->isEndElement() && reader->name() == "worksheet")
			break;

		if (!reader->isStartElement())
			continue;

		if (reader->name() == "comment") {
			if (!readCommentElement(reader))
				return false;
		} else if (!preview && reader->name() == "theme") {
			attribs = reader->attributes();
			d->theme = attribs.value("name").toString();
		} else if (!preview && reader->name() == "geometry") {
			attribs = reader->attributes();

			str = attribs.value("x").toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.subs("x").toString());
			else
				rect.setX(str.toDouble());

			str = attribs.value("y").toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.subs("y").toString());
			else
				rect.setY(str.toDouble());

			str = attribs.value("width").toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.subs("width").toString());
			else
				rect.setWidth(str.toDouble());

			str = attribs.value("height").toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.subs("height").toString());
			else
				rect.setHeight(str.toDouble());

			READ_INT_VALUE("useViewSize", useViewSize, int);
		} else if (!preview && reader->name() == "layout") {
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
		} else if (!preview && reader->name() == "background") {
			attribs = reader->attributes();

			READ_INT_VALUE("type", backgroundType, PlotArea::BackgroundType);
			READ_INT_VALUE("colorStyle", backgroundColorStyle, PlotArea::BackgroundColorStyle);
			READ_INT_VALUE("imageStyle", backgroundImageStyle, PlotArea::BackgroundImageStyle);
			READ_INT_VALUE("brushStyle", backgroundBrushStyle, Qt::BrushStyle);

			str = attribs.value("firstColor_r").toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.subs("firstColor_r").toString());
			else
				d->backgroundFirstColor.setRed(str.toInt());

			str = attribs.value("firstColor_g").toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.subs("firstColor_g").toString());
			else
				d->backgroundFirstColor.setGreen(str.toInt());

			str = attribs.value("firstColor_b").toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.subs("firstColor_b").toString());
			else
				d->backgroundFirstColor.setBlue(str.toInt());

			str = attribs.value("secondColor_r").toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.subs("secondColor_r").toString());
			else
				d->backgroundSecondColor.setRed(str.toInt());

			str = attribs.value("secondColor_g").toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.subs("secondColor_g").toString());
			else
				d->backgroundSecondColor.setGreen(str.toInt());

			str = attribs.value("secondColor_b").toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.subs("secondColor_b").toString());
			else
				d->backgroundSecondColor.setBlue(str.toInt());

			str = attribs.value("fileName").toString();
			d->backgroundFileName = str;

			READ_DOUBLE_VALUE("opacity", backgroundOpacity);
		} else if(!preview && reader->name() == "plotProperties") {
			attribs = reader->attributes();

			READ_INT_VALUE("plotsLocked", plotsLocked, bool);
			READ_INT_VALUE("cartesianPlotActionMode", cartesianPlotActionMode, Worksheet::CartesianPlotActionMode);
			READ_INT_VALUE("cartesianPlotCursorMode", cartesianPlotCursorMode, Worksheet::CartesianPlotActionMode);
		} else if (reader->name() == "cartesianPlot") {
			CartesianPlot* plot = new CartesianPlot(QString());
			plot->setIsLoading(true);
			if (!plot->load(reader, preview)) {
				delete plot;
				return false;
			} else
				addChildFast(plot);
		} else if (!preview && reader->name() == "textLabel") {
			TextLabel* label = new TextLabel(QString());
			if (!label->load(reader, preview)) {
				delete label;
				return false;
			} else
				addChildFast(label);
		} else if (!preview && reader->name() == "image") {
			Image* image = new Image(QString());
			if (!image->load(reader, preview)) {
				delete image;
				return false;
			} else
				addChildFast(image);
		} else { // unknown element
			if (!preview)
				reader->raiseWarning(i18n("unknown element '%1'", reader->name().toString()));
			if (!reader->skipToEndElement()) return false;
		}
	}

	if (!preview) {
		d->m_scene->setSceneRect(rect);
		d->updateLayout();
		updateCompleteCursorTreeModel();
	}

	return true;
}

//##############################################################################
//#########################  Theme management ##################################
//##############################################################################
void Worksheet::loadTheme(const QString& theme) {
	KConfigGroup group;
	KConfig* config = nullptr;
	if (!theme.isEmpty()) {
		//load values from the theme config
		config = new KConfig(ThemeHandler::themeFilePath(theme), KConfig::SimpleConfig);

		//apply the same background color for Worksheet as for the CartesianPlot
		group = config->group("CartesianPlot");

		//load the theme for all the children
		const auto& children = this->children<WorksheetElement>(ChildIndexFlag::IncludeHidden);
		for (auto* child : children)
			child->loadThemeConfig(*config);
	} else {
		//load default values
		config = new KConfig();
		group = config->group("Worksheet");
	}

	this->setBackgroundType((PlotArea::BackgroundType) group.readEntry("BackgroundType", static_cast<int>(PlotArea::BackgroundType::Color)));
	this->setBackgroundColorStyle((PlotArea::BackgroundColorStyle) group.readEntry("BackgroundColorStyle", static_cast<int>(PlotArea::BackgroundColorStyle::SingleColor)));
	this->setBackgroundImageStyle((PlotArea::BackgroundImageStyle) group.readEntry("BackgroundImageStyle", static_cast<int>(PlotArea::BackgroundImageStyle::Scaled)));
	this->setBackgroundBrushStyle((Qt::BrushStyle) group.readEntry("BackgroundBrushStyle", static_cast<int>(Qt::SolidPattern)));
	this->setBackgroundFirstColor(group.readEntry("BackgroundFirstColor", QColor(Qt::white)));
	this->setBackgroundSecondColor(group.readEntry("BackgroundSecondColor", QColor(Qt::black)));
	this->setBackgroundOpacity(group.readEntry("BackgroundOpacity", 1.0));

	//load the theme for all the children
	const auto& children = this->children<WorksheetElement>(ChildIndexFlag::IncludeHidden);
	for (auto* child : children)
		child->loadThemeConfig(*config);

	delete config;
}
