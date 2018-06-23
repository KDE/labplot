/***************************************************************************
    File                 : Worksheet.cpp
    Project              : LabPlot
    Description          : Worksheet
    --------------------------------------------------------------------
    Copyright            : (C) 2009 Tilman Benkert (thzs@gmx.net)
	Copyright            : (C) 2011-2016 by Alexander Semke (alexander.semke@web.de)
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
#include "backend/worksheet/plots/cartesian/CartesianPlot.h"
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
#include <cmath>

/**
 * \class Worksheet
 * \brief Top-level container for worksheet elements like plot, labels, etc.
 *
 * The worksheet is, besides the data containers \c Spreadsheet and \c Matrix, another central part of the application
 * and provides an area for showing and grouping together different kinds of worksheet objects - plots, labels &etc;
 *
 * * \ingroup worksheet
 */
Worksheet::Worksheet(AbstractScriptingEngine* engine, const QString& name, bool loading)
	: AbstractPart(name), scripted(engine), d(new WorksheetPrivate(this)), m_view(nullptr) {

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
	KConfigGroup group = config.group( "Worksheet" );

	//size
	d->scaleContent = group.readEntry("ScaleContent", false);
	d->useViewSize = group.readEntry("UseViewSize", false);
	d->pageRect.setX(0);
	d->pageRect.setY(0);
	d->pageRect.setWidth(group.readEntry("Width", 1500));
	d->pageRect.setHeight(group.readEntry("Height",1500));
	d->m_scene->setSceneRect(d->pageRect);

	//background
	d->backgroundType = (PlotArea::BackgroundType) group.readEntry("BackgroundType", (int) PlotArea::Color);
	d->backgroundColorStyle = (PlotArea::BackgroundColorStyle) group.readEntry("BackgroundColorStyle", (int) PlotArea::SingleColor);
	d->backgroundImageStyle = (PlotArea::BackgroundImageStyle) group.readEntry("BackgroundImageStyle", (int) PlotArea::Scaled);
	d->backgroundBrushStyle = (Qt::BrushStyle) group.readEntry("BackgroundBrushStyle", (int) Qt::SolidPattern);
	d->backgroundFileName = group.readEntry("BackgroundFileName", QString());
	d->backgroundFirstColor = group.readEntry("BackgroundFirstColor", QColor(Qt::white));
	d->backgroundSecondColor = group.readEntry("BackgroundSecondColor", QColor(Qt::black));
	d->backgroundOpacity = group.readEntry("BackgroundOpacity", 1.0);

	//layout
	d->layout = (Worksheet::Layout) group.readEntry("Layout", (int) Worksheet::VerticalLayout);
	d->layoutTopMargin =  group.readEntry("LayoutTopMargin", convertToSceneUnits(1, Centimeter));
	d->layoutBottomMargin = group.readEntry("LayoutBottomMargin", convertToSceneUnits(1, Centimeter));
	d->layoutLeftMargin = group.readEntry("LayoutLeftMargin", convertToSceneUnits(1, Centimeter));
	d->layoutRightMargin = group.readEntry("LayoutRightMargin", convertToSceneUnits(1, Centimeter));
	d->layoutVerticalSpacing = group.readEntry("LayoutVerticalSpacing", convertToSceneUnits(1, Centimeter));
	d->layoutHorizontalSpacing = group.readEntry("LayoutHorizontalSpacing", convertToSceneUnits(1, Centimeter));
	d->layoutRowCount = group.readEntry("LayoutRowCount", 2);
	d->layoutColumnCount = group.readEntry("LayoutColumnCount", 2);

	//default theme
	KConfigGroup settings = KSharedConfig::openConfig()->group(QLatin1String("Settings_Worksheet"));
	d->theme = settings.readEntry(QLatin1String("Theme"), "");
	if (!d->theme.isEmpty())
		loadTheme(d->theme);
}

/*!
	converts from \c unit to the scene units. At the moment, 1 scene unit corresponds to 1/10 mm.
 */
float Worksheet::convertToSceneUnits(const float value, const Worksheet::Unit unit) {
	switch (unit) {
	case Worksheet::Millimeter:
		return value*10.0;
	case Worksheet::Centimeter:
		return value*100.0;
	case Worksheet::Inch:
		return value*25.4*10.;
	case Worksheet::Point:
		return value*25.4/72.*10.;
	}

	return 0;
}

/*!
	converts from the scene units to \c unit . At the moment, 1 scene unit corresponds to 1/10 mm.
 */
float Worksheet::convertFromSceneUnits(const float value, const Worksheet::Unit unit) {
	switch (unit) {
	case Worksheet::Millimeter:
		return value/10.0;
	case Worksheet::Centimeter:
		return value/100.0;
	case Worksheet::Inch:
		return value/25.4/10.;
	case Worksheet::Point:
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

bool Worksheet::exportView() const {
	ExportWorksheetDialog* dlg = new ExportWorksheetDialog(m_view);
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
	QPrintDialog* dlg = new QPrintDialog(&printer, m_view);
	dlg->setWindowTitle(i18n("Print Worksheet"));
	bool ret;
	if ( (ret = (dlg->exec() == QDialog::Accepted)) )
		m_view->print(&printer);

	delete dlg;
	return ret;
}

bool Worksheet::printPreview() const {
	QPrintPreviewDialog* dlg = new QPrintPreviewDialog(m_view);
	connect(dlg, &QPrintPreviewDialog::paintRequested, m_view, &WorksheetView::print);
	return dlg->exec();
}

void Worksheet::handleAspectAdded(const AbstractAspect* aspect) {
	const WorksheetElement* addedElement = qobject_cast<const WorksheetElement*>(aspect);
	if (!addedElement)
		return;

	if (aspect->parentAspect() != this)
		return;

	//add the GraphicsItem of the added child to the scene
	QGraphicsItem* item = addedElement->graphicsItem();
	d->m_scene->addItem(item);

	qreal zVal = 0;
	for (auto* child : children<WorksheetElement>(IncludeHidden))
		child->graphicsItem()->setZValue(zVal++);

	//if a theme was selected in the worksheet, apply this theme for newly added children
	if (!d->theme.isEmpty() && !isLoading()) {
		KConfig config(ThemeHandler::themeFilePath(d->theme), KConfig::SimpleConfig);
		const_cast<WorksheetElement*>(addedElement)->loadThemeConfig(config);
	}

	//recalculated the layout
	if (!isLoading()) {
		if (d->layout != Worksheet::NoLayout)
			d->updateLayout(false);
	}
}

void Worksheet::handleAspectAboutToBeRemoved(const AbstractAspect* aspect) {
	const WorksheetElement* removedElement = qobject_cast<const WorksheetElement*>(aspect);
	if (removedElement) {
		QGraphicsItem* item = removedElement->graphicsItem();
		d->m_scene->removeItem(item);
	}
}

void Worksheet::handleAspectRemoved(const AbstractAspect* parent, const AbstractAspect* before, const AbstractAspect* child) {
	Q_UNUSED(parent);
	Q_UNUSED(before);
	Q_UNUSED(child);

	if (d->layout != Worksheet::NoLayout)
		d->updateLayout(false);
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
	WorksheetElement* element=qobject_cast<WorksheetElement*>(const_cast<AbstractAspect*>(aspect));
	if (element)
		emit itemSelected(element->graphicsItem());
}

/*!
	this slot is called when a worksheet element is deselected in the project explorer.
	emits \c itemDeselected() which forwards this event to \c WorksheetView
	in order to deselect the corresponding \c QGraphicsItem.
 */
void Worksheet::childDeselected(const AbstractAspect* aspect) {
	WorksheetElement* element=qobject_cast<WorksheetElement*>(const_cast<AbstractAspect*>(aspect));
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
	for (const auto* child : children<WorksheetElement>(IncludeHidden) ) {
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
WorksheetElement* Worksheet::aspectFromGraphicsItem(const WorksheetElement* aspect, const QGraphicsItem* item) const {
	if ( aspect->graphicsItem() == item )
		return const_cast<WorksheetElement*>(aspect);
	else {
		for (const auto* child : aspect->children<WorksheetElement>(AbstractAspect::IncludeHidden) ) {
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
	for (const auto* child : children<WorksheetElement>(IncludeHidden) ) {
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

void Worksheet::update() {
	emit requestUpdate();
}

void Worksheet::setSuppressLayoutUpdate(bool value) {
	d->suppressLayoutUpdate = value;
}

void Worksheet::updateLayout() {
	d->updateLayout();
}

/* =============================== getter methods for general options ==================================== */
BASIC_D_READER_IMPL(Worksheet, bool, scaleContent, scaleContent)
BASIC_D_READER_IMPL(Worksheet, bool, useViewSize, useViewSize)

/* =============================== getter methods for background options ================================= */
BASIC_D_READER_IMPL(Worksheet, PlotArea::BackgroundType, backgroundType, backgroundType)
BASIC_D_READER_IMPL(Worksheet, PlotArea::BackgroundColorStyle, backgroundColorStyle, backgroundColorStyle)
BASIC_D_READER_IMPL(Worksheet, PlotArea::BackgroundImageStyle, backgroundImageStyle, backgroundImageStyle)
BASIC_D_READER_IMPL(Worksheet, Qt::BrushStyle, backgroundBrushStyle, backgroundBrushStyle)
CLASS_D_READER_IMPL(Worksheet, QColor, backgroundFirstColor, backgroundFirstColor)
CLASS_D_READER_IMPL(Worksheet, QColor, backgroundSecondColor, backgroundSecondColor)
CLASS_D_READER_IMPL(Worksheet, QString, backgroundFileName, backgroundFileName)
BASIC_D_READER_IMPL(Worksheet, float, backgroundOpacity, backgroundOpacity)

/* =============================== getter methods for layout options ====================================== */
BASIC_D_READER_IMPL(Worksheet, Worksheet::Layout, layout, layout)
BASIC_D_READER_IMPL(Worksheet, float, layoutTopMargin, layoutTopMargin)
BASIC_D_READER_IMPL(Worksheet, float, layoutBottomMargin, layoutBottomMargin)
BASIC_D_READER_IMPL(Worksheet, float, layoutLeftMargin, layoutLeftMargin)
BASIC_D_READER_IMPL(Worksheet, float, layoutRightMargin, layoutRightMargin)
BASIC_D_READER_IMPL(Worksheet, float, layoutHorizontalSpacing, layoutHorizontalSpacing)
BASIC_D_READER_IMPL(Worksheet, float, layoutVerticalSpacing, layoutVerticalSpacing)
BASIC_D_READER_IMPL(Worksheet, int, layoutRowCount, layoutRowCount)
BASIC_D_READER_IMPL(Worksheet, int, layoutColumnCount, layoutColumnCount)

CLASS_D_READER_IMPL(Worksheet, QString, theme, theme)

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

STD_SETTER_CMD_IMPL_F_S(Worksheet, SetBackgroundOpacity, float, backgroundOpacity, update)
void Worksheet::setBackgroundOpacity(float opacity) {
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

STD_SETTER_CMD_IMPL_M_F_S(Worksheet, SetLayoutTopMargin, float, layoutTopMargin, updateLayout)
void Worksheet::setLayoutTopMargin(float margin) {
	if (margin != d->layoutTopMargin) {
		beginMacro(i18n("%1: set layout top margin", name()));
		exec(new WorksheetSetLayoutTopMarginCmd(d, margin, ki18n("%1: set layout top margin")));
		endMacro();
	}
}

STD_SETTER_CMD_IMPL_M_F_S(Worksheet, SetLayoutBottomMargin, float, layoutBottomMargin, updateLayout)
void Worksheet::setLayoutBottomMargin(float margin) {
	if (margin != d->layoutBottomMargin) {
		beginMacro(i18n("%1: set layout bottom margin", name()));
		exec(new WorksheetSetLayoutBottomMarginCmd(d, margin, ki18n("%1: set layout bottom margin")));
		endMacro();
	}
}

STD_SETTER_CMD_IMPL_M_F_S(Worksheet, SetLayoutLeftMargin, float, layoutLeftMargin, updateLayout)
void Worksheet::setLayoutLeftMargin(float margin) {
	if (margin != d->layoutLeftMargin) {
		beginMacro(i18n("%1: set layout left margin", name()));
		exec(new WorksheetSetLayoutLeftMarginCmd(d, margin, ki18n("%1: set layout left margin")));
		endMacro();
	}
}

STD_SETTER_CMD_IMPL_M_F_S(Worksheet, SetLayoutRightMargin, float, layoutRightMargin, updateLayout)
void Worksheet::setLayoutRightMargin(float margin) {
	if (margin != d->layoutRightMargin) {
		beginMacro(i18n("%1: set layout right margin", name()));
		exec(new WorksheetSetLayoutRightMarginCmd(d, margin, ki18n("%1: set layout right margin")));
		endMacro();
	}
}

STD_SETTER_CMD_IMPL_M_F_S(Worksheet, SetLayoutVerticalSpacing, float, layoutVerticalSpacing, updateLayout)
void Worksheet::setLayoutVerticalSpacing(float spacing) {
	if (spacing != d->layoutVerticalSpacing) {
		beginMacro(i18n("%1: set layout vertical spacing", name()));
		exec(new WorksheetSetLayoutVerticalSpacingCmd(d, spacing, ki18n("%1: set layout vertical spacing")));
		endMacro();
	}
}

STD_SETTER_CMD_IMPL_M_F_S(Worksheet, SetLayoutHorizontalSpacing, float, layoutHorizontalSpacing, updateLayout)
void Worksheet::setLayoutHorizontalSpacing(float spacing) {
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
	QVector<WorksheetElement*> childElements = children<WorksheetElement>(AbstractAspect::Recursive | AbstractAspect::IncludeHidden);
	for (auto* child : childElements)
		child->setPrinting(on);
}

STD_SETTER_CMD_IMPL_S(Worksheet, SetTheme, QString, theme)
void Worksheet::setTheme(const QString& theme) {
	if (theme != d->theme) {
		if (!theme.isEmpty()) {
			beginMacro( i18n("%1: load theme %2", name(), theme) );
			exec(new WorksheetSetThemeCmd(d, theme, ki18n("%1: set theme")));
			loadTheme(theme);
			endMacro();
		} else {
			exec(new WorksheetSetThemeCmd(d, theme, ki18n("%1: disable theming")));
		}
	}
}

//##############################################################################
//######################  Private implementation ###############################
//##############################################################################
WorksheetPrivate::WorksheetPrivate(Worksheet* owner):q(owner),
	m_scene(new QGraphicsScene()),
	scaleContent(false),
	suppressLayoutUpdate(false) {
}

QString WorksheetPrivate::name() const {
	return q->name();
}

void WorksheetPrivate::updatePageRect() {
	QRectF oldRect = m_scene->sceneRect();
	m_scene->setSceneRect(pageRect);

	if (layout != Worksheet::NoLayout)
		updateLayout();
	else {
		if (scaleContent) {
			qreal horizontalRatio = pageRect.width() / oldRect.width();
			qreal verticalRatio = pageRect.height() / oldRect.height();
			QVector<WorksheetElement*> childElements = q->children<WorksheetElement>(AbstractAspect::IncludeHidden);
			if (useViewSize) {
				//don't make the change of the geometry undoable/redoable if the view size is used.
				for (auto* elem : childElements) {
					elem->setUndoAware(false);
					elem->handleResize(horizontalRatio, verticalRatio, true);
					elem->setUndoAware(true);
				}
			} else {
				for (auto* child : childElements)
					child->handleResize(horizontalRatio, verticalRatio, true);
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

	QVector<WorksheetElementContainer*> list = q->children<WorksheetElementContainer>();
	if (layout==Worksheet::NoLayout) {
		for(auto* elem : list)
			elem->graphicsItem()->setFlag(QGraphicsItem::ItemIsMovable, true);

		return;
	}

	float x=layoutLeftMargin;
	float y=layoutTopMargin;
	float w, h;
	int count=list.count();
	if (layout == Worksheet::VerticalLayout) {
		w= m_scene->sceneRect().width() - layoutLeftMargin - layoutRightMargin;
		h=(m_scene->sceneRect().height()-layoutTopMargin-layoutBottomMargin- (count-1)*layoutVerticalSpacing)/count;
		for (auto* elem : list) {
			setContainerRect(elem, x, y, h, w, undoable);
			y+=h + layoutVerticalSpacing;
		}
	} else if (layout == Worksheet::HorizontalLayout) {
		w=(m_scene->sceneRect().width()-layoutLeftMargin-layoutRightMargin- (count-1)*layoutHorizontalSpacing)/count;
		h= m_scene->sceneRect().height() - layoutTopMargin-layoutBottomMargin;
		for (auto* elem : list) {
			setContainerRect(elem, x, y, h, w, undoable);
			x+=w + layoutHorizontalSpacing;
		}
	} else { //GridLayout
		//add new rows, if not sufficient
		if (count>layoutRowCount*layoutColumnCount) {
			layoutRowCount = floor( (float)count/layoutColumnCount + 0.5);
			emit q->layoutRowCountChanged(layoutRowCount);
		}

		w=(m_scene->sceneRect().width()-layoutLeftMargin-layoutRightMargin- (layoutColumnCount-1)*layoutHorizontalSpacing)/layoutColumnCount;
		h=(m_scene->sceneRect().height()-layoutTopMargin-layoutBottomMargin- (layoutRowCount-1)*layoutVerticalSpacing)/layoutRowCount;
		int columnIndex=0; //counts the columns in a row
		for (auto* elem : list) {
			setContainerRect(elem, x, y, h, w, undoable);
			x+=w + layoutHorizontalSpacing;
			columnIndex++;
			if (columnIndex==layoutColumnCount) {
				columnIndex=0;
				x=layoutLeftMargin;
				y+=h + layoutVerticalSpacing;
			}
		}
	}
}

void WorksheetPrivate::setContainerRect(WorksheetElementContainer* elem, float x, float y, float h, float w, bool undoable) {
	if (useViewSize) {
		//when using the view size, no need to put rect changes onto the undo-stack
		elem->setUndoAware(false);
		elem->setRect(QRectF(x,y,w,h));
		elem->setUndoAware(true);
	} else {
		//don't put rect changed onto the undo-stack if undoable-flag is set to true,
		//e.g. when new child is added or removed (the layout and the childrend rects will be updated anyway)
		if (!undoable) {
			elem->setUndoAware(false);
			elem->setRect(QRectF(x,y,w,h));
			elem->setUndoAware(true);
		} else
			elem->setRect(QRectF(x,y,w,h));
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
	if (!d->theme.isEmpty()){
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
	writer->writeAttribute( "layout", QString::number(d->layout) );
	writer->writeAttribute( "topMargin", QString::number(d->layoutTopMargin) );
	writer->writeAttribute( "bottomMargin", QString::number(d->layoutBottomMargin) );
	writer->writeAttribute( "leftMargin", QString::number(d->layoutLeftMargin) );
	writer->writeAttribute( "rightMargin", QString::number(d->layoutRightMargin) );
	writer->writeAttribute( "verticalSpacing", QString::number(d->layoutVerticalSpacing) );
	writer->writeAttribute( "horizontalSpacing", QString::number(d->layoutHorizontalSpacing) );
	writer->writeAttribute( "columnCount", QString::number(d->layoutColumnCount) );
	writer->writeAttribute( "rowCount", QString::number(d->layoutRowCount) );
	writer->writeEndElement();

	//background properties
	writer->writeStartElement( "background" );
	writer->writeAttribute( "type", QString::number(d->backgroundType) );
	writer->writeAttribute( "colorStyle", QString::number(d->backgroundColorStyle) );
	writer->writeAttribute( "imageStyle", QString::number(d->backgroundImageStyle) );
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

	//serialize all children
	for (auto* child : children<WorksheetElement>(IncludeHidden))
		child->save(writer);

	writer->writeEndElement(); // close "worksheet" section
}

//! Load from XML
bool Worksheet::load(XmlStreamReader* reader, bool preview) {
	if(!reader->isStartElement() || reader->name() != "worksheet") {
		reader->raiseError(i18n("no worksheet element found"));
		return false;
	}

	if (!readBasicAttributes(reader))
		return false;

	//clear the theme that was potentially set in init() in order to correctly load here the worksheets without any theme used
	d->theme = "";

	QString attributeWarning = i18n("Attribute '%1' missing or empty, default value is used");
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
			if(str.isEmpty())
				reader->raiseWarning(attributeWarning.arg("'x'"));
			else
				rect.setX(str.toDouble());

			str = attribs.value("y").toString();
			if(str.isEmpty())
				reader->raiseWarning(attributeWarning.arg("'y'"));
			else
				rect.setY(str.toDouble());

			str = attribs.value("width").toString();
			if(str.isEmpty())
				reader->raiseWarning(attributeWarning.arg("'width'"));
			else
				rect.setWidth(str.toDouble());

			str = attribs.value("height").toString();
			if(str.isEmpty())
				reader->raiseWarning(attributeWarning.arg("'height'"));
			else
				rect.setHeight(str.toDouble());

			str = attribs.value("useViewSize").toString();
			if(str.isEmpty())
				reader->raiseWarning(attributeWarning.arg("'useViewSize'"));
			else
				d->useViewSize = str.toInt();
		} else if (!preview && reader->name() == "layout") {
			attribs = reader->attributes();

			str = attribs.value("layout").toString();
			if(str.isEmpty())
				reader->raiseWarning(attributeWarning.arg("layout"));
			else
				d->layout = Worksheet::Layout(str.toInt());

			str = attribs.value("topMargin").toString();
			if(str.isEmpty())
				reader->raiseWarning(attributeWarning.arg("topMargin"));
			else
				d->layoutTopMargin = str.toDouble();

			str = attribs.value("bottomMargin").toString();
			if(str.isEmpty())
				reader->raiseWarning(attributeWarning.arg("bottomMargin"));
			else
				d->layoutBottomMargin = str.toDouble();

			str = attribs.value("leftMargin").toString();
			if(str.isEmpty())
				reader->raiseWarning(attributeWarning.arg("leftMargin"));
			else
				d->layoutLeftMargin = str.toDouble();

			str = attribs.value("rightMargin").toString();
			if(str.isEmpty())
				reader->raiseWarning(attributeWarning.arg("rightMargin"));
			else
				d->layoutRightMargin = str.toDouble();

			str = attribs.value("verticalSpacing").toString();
			if(str.isEmpty())
				reader->raiseWarning(attributeWarning.arg("verticalSpacing"));
			else
				d->layoutVerticalSpacing = str.toDouble();

			str = attribs.value("horizontalSpacing").toString();
			if(str.isEmpty())
				reader->raiseWarning(attributeWarning.arg("horizontalSpacing"));
			else
				d->layoutHorizontalSpacing = str.toDouble();

			str = attribs.value("columnCount").toString();
			if(str.isEmpty())
				reader->raiseWarning(attributeWarning.arg("columnCount"));
			else
				d->layoutColumnCount = str.toInt();

			str = attribs.value("rowCount").toString();
			if(str.isEmpty())
				reader->raiseWarning(attributeWarning.arg("rowCount"));
			else
				d->layoutRowCount = str.toInt();
		} else if (!preview && reader->name() == "background") {
			attribs = reader->attributes();

			str = attribs.value("type").toString();
			if(str.isEmpty())
				reader->raiseWarning(attributeWarning.arg("type"));
			else
				d->backgroundType = PlotArea::BackgroundType(str.toInt());

			str = attribs.value("colorStyle").toString();
			if(str.isEmpty())
				reader->raiseWarning(attributeWarning.arg("colorStyle"));
			else
				d->backgroundColorStyle = PlotArea::BackgroundColorStyle(str.toInt());

			str = attribs.value("imageStyle").toString();
			if(str.isEmpty())
				reader->raiseWarning(attributeWarning.arg("imageStyle"));
			else
				d->backgroundImageStyle = PlotArea::BackgroundImageStyle(str.toInt());

			str = attribs.value("brushStyle").toString();
			if(str.isEmpty())
				reader->raiseWarning(attributeWarning.arg("brushStyle"));
			else
				d->backgroundBrushStyle = Qt::BrushStyle(str.toInt());

			str = attribs.value("firstColor_r").toString();
			if(str.isEmpty())
				reader->raiseWarning(attributeWarning.arg("firstColor_r"));
			else
				d->backgroundFirstColor.setRed(str.toInt());

			str = attribs.value("firstColor_g").toString();
			if(str.isEmpty())
				reader->raiseWarning(attributeWarning.arg("firstColor_g"));
			else
				d->backgroundFirstColor.setGreen(str.toInt());

			str = attribs.value("firstColor_b").toString();
			if(str.isEmpty())
				reader->raiseWarning(attributeWarning.arg("firstColor_b"));
			else
				d->backgroundFirstColor.setBlue(str.toInt());

			str = attribs.value("secondColor_r").toString();
			if(str.isEmpty())
				reader->raiseWarning(attributeWarning.arg("secondColor_r"));
			else
				d->backgroundSecondColor.setRed(str.toInt());

			str = attribs.value("secondColor_g").toString();
			if(str.isEmpty())
				reader->raiseWarning(attributeWarning.arg("secondColor_g"));
			else
				d->backgroundSecondColor.setGreen(str.toInt());

			str = attribs.value("secondColor_b").toString();
			if(str.isEmpty())
				reader->raiseWarning(attributeWarning.arg("secondColor_b"));
			else
				d->backgroundSecondColor.setBlue(str.toInt());

			str = attribs.value("fileName").toString();
			d->backgroundFileName = str;

			str = attribs.value("opacity").toString();
			if(str.isEmpty())
				reader->raiseWarning(attributeWarning.arg("opacity"));
			else
				d->backgroundOpacity = str.toDouble();
		} else if(reader->name() == "cartesianPlot") {
			CartesianPlot* plot = new CartesianPlot("");
			plot->setIsLoading(true);
			if (!plot->load(reader, preview)) {
				delete plot;
				return false;
			} else
				addChildFast(plot);
		} else if(reader->name() == "textLabel") {
			TextLabel* label = new TextLabel("");
			if (!label->load(reader, preview)) {
				delete label;
				return false;
			} else
				addChildFast(label);
		} else { // unknown element
			reader->raiseWarning(i18n("unknown element '%1'", reader->name().toString()));
			if (!reader->skipToEndElement()) return false;
		}
	}

	if (!preview) {
		d->m_scene->setSceneRect(rect);
		d->updateLayout();
	}

	return true;
}

//##############################################################################
//#########################  Theme management ##################################
//##############################################################################
void Worksheet::loadTheme(const QString& theme) {
	KConfig config(ThemeHandler::themeFilePath(theme), KConfig::SimpleConfig);

	//apply the same background color for Worksheet as for the CartesianPlot
	const KConfigGroup group = config.group("CartesianPlot");
	this->setBackgroundBrushStyle((Qt::BrushStyle)group.readEntry("BackgroundBrushStyle",(int) this->backgroundBrushStyle()));
	this->setBackgroundColorStyle((PlotArea::BackgroundColorStyle)(group.readEntry("BackgroundColorStyle",(int) this->backgroundColorStyle())));
	this->setBackgroundFirstColor(group.readEntry("BackgroundFirstColor",(QColor) this->backgroundFirstColor()));
	this->setBackgroundImageStyle((PlotArea::BackgroundImageStyle)group.readEntry("BackgroundImageStyle",(int) this->backgroundImageStyle()));
	this->setBackgroundOpacity(group.readEntry("BackgroundOpacity", this->backgroundOpacity()));
	this->setBackgroundSecondColor(group.readEntry("BackgroundSecondColor",(QColor) this->backgroundSecondColor()));
	this->setBackgroundType((PlotArea::BackgroundType)(group.readEntry("BackgroundType",(int) this->backgroundType())));

	//load the theme for all the children
	const QVector<WorksheetElement*>& childElements = children<WorksheetElement>(AbstractAspect::IncludeHidden);
	for (auto* child : childElements)
		child->loadThemeConfig(config);
}
