/***************************************************************************
	File                 : InfoElement.cpp
	Project              : LabPlot
	Description          : Marker which can highlight points of curves and
						   show their values
	--------------------------------------------------------------------
	Copyright            : (C) 2019 Martin Marmsoler (martin.marmsoler@gmail.com)
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

#include "InfoElement.h"

#include "backend/worksheet/plots/cartesian/CartesianPlot.h"
#include "backend/worksheet/plots/cartesian/XYCurve.h"
#include "backend/worksheet/plots/cartesian/CartesianCoordinateSystem.h"
#include "backend/worksheet/InfoElementPrivate.h"
#include "backend/worksheet/plots/cartesian/CustomPoint.h"
#include "backend/worksheet/TextLabel.h"
#include "backend/lib/commandtemplates.h"
#include "backend/lib/XmlStreamReader.h"

#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QKeyEvent>
#include <QAction>
#include <QMenu>
#include <QTextEdit>
#include <QDateTime>


InfoElement::InfoElement(const QString &name, CartesianPlot *plot):
	WorksheetElement(name, AspectType::InfoElement),
	d_ptr(new InfoElementPrivate(this,plot)) {
	Q_D(InfoElement);
	init();
	setVisible(false);
	d->retransform();
}

InfoElement::InfoElement(const QString &name, CartesianPlot *plot, const XYCurve *curve, double pos):
	WorksheetElement(name, AspectType::InfoElement),
	// must be at least, because otherwise label ist not a nullptr
	d_ptr(new InfoElementPrivate(this,plot,curve)) {
	Q_D(InfoElement);

	init();

	m_suppressChildPositionChanged = true;

	if (curve) {
		CustomPoint* custompoint = new CustomPoint(plot, "Markerpoint");
		addChild(custompoint);
		InfoElement::MarkerPoints_T markerpoint(custompoint, custompoint->path(), curve, curve->path());
		markerpoints.append(markerpoint);
		// setpos after label was created
		bool valueFound;
		double xpos;
		double y = curve->y(pos,xpos,valueFound);
		if (valueFound) {
			d->xPos = xpos;
			d->m_index = curve->xColumn()->indexForValue(xpos);
			markerpoints.last().x = xpos;
			markerpoints.last().y = y;
			custompoint->setPosition(QPointF(xpos,y));
			DEBUG("Value found");
		} else {
			d->xPos = 0;
			markerpoints.last().x = 0;
			markerpoints.last().y = 0;
			custompoint->setPosition(d->cSystem->mapSceneToLogical(QPointF(0,0)));
			DEBUG("Value not found");
		}

		// C++14 enabled:
		connect(curve, QOverload<bool>::of(&XYCurve::visibilityChanged), this, &InfoElement::curveVisibilityChanged);
		custompoint->setVisible(curve->isVisible());

		setVisible(true);
	} else
		setVisible(false);

	TextLabel::TextWrapper text;
	text.placeholder = true;

	if (!markerpoints.empty()) {
		QString textString;
		textString = QString::number(markerpoints[0].x)+ ", ";
		textString.append(QString(QString(markerpoints[0].curve->name()+":")));
		textString.append(QString::number(markerpoints[0].y));
		text.text = textString;
		// TODO: Find better solution than using textedit
		QTextEdit textedit(QString("&(x), ")+ QString(markerpoints[0].curve->name()+":"+"&("+markerpoints[0].curve->name()+")"));
		text.textPlaceholder = textedit.toHtml();
	} else
		text.textPlaceholder = "Please Add Text here";
	label->setText(text);

	m_suppressChildPositionChanged = false;

	d->retransform();
}

InfoElement::~InfoElement() {
	//	m_suppressChildRemoved = true;
	//	// this function is not called, when deleting marker
	//	// don't understand why I have to remove them manually
	//	// I think it is because of the graphicsitem, which exists
	//	for (auto markerpoint : markerpoints) {
	//		removeChild(markerpoint.customPoint);
	//	}

	//	removeChild(label);

	//	m_suppressChildRemoved = false;
}

void InfoElement::init() {
	Q_D(InfoElement);

	initActions();
	initMenus();

	connect(this, &InfoElement::aspectRemoved, this, &InfoElement::childRemoved);
	connect(this, &InfoElement::aspectAdded, this, &InfoElement::childAdded);

	label = new TextLabel("InfoElementLabel", d->plot);
	addChild(label);
	label->enableCoordBinding(true);
	label->setCoordBinding(true);
	TextLabel::TextWrapper text;
	text.placeholder = true;
	label->setText(text); // set placeholder to true

	//use the color for the axis line from the theme also for info element's lines
	KConfig config;
	const KConfigGroup& group = config.group("Axis");
	d->connectionLineColor = group.readEntry("LineColor", QColor(Qt::black));
	d->xposLineColor = d->connectionLineColor;
}

void InfoElement::initActions() {
	visibilityAction = new QAction(i18n("Visible"), this);
	visibilityAction->setCheckable(true);
	connect(visibilityAction, &QAction::triggered, this, &InfoElement::setVisible);
}

void InfoElement::initMenus() {
	m_menusInitialized = true;
}

QMenu* InfoElement::createContextMenu() {
	if (!m_menusInitialized)
		initMenus();

	QMenu* menu = WorksheetElement::createContextMenu();
	QAction* firstAction = menu->actions().at(1);

	visibilityAction->setChecked(isVisible());
	menu->insertAction(firstAction, visibilityAction);

	return menu;
}

/*!
 * @brief InfoElement::addCurve
 * Adds a new markerpoint to the plot which is placed on the curve curve
 * @param curve Curve on which the markerpoints sits
 * @param custompoint Use existing point, if the project was loaded the custompoint can have different settings
 */
void InfoElement::addCurve(const XYCurve* curve, CustomPoint* custompoint) {
	Q_D(InfoElement);

	for (auto markerpoint: markerpoints) {
		if (curve == markerpoint.curve)
			return;
	}

	if (!custompoint) {
		custompoint = new CustomPoint(d->plot, "Markerpoint");
		addChild(custompoint);
		bool valueFound;
		double x_new, y;
		if (markerpoints.isEmpty())
			y = curve->y(d->xPos, x_new, valueFound);
		else
			y = curve->y(markerpoints[0].customPoint->position().x(), x_new, valueFound);
		custompoint->setPosition(QPointF(x_new,y));
	} else
		addChild(custompoint);

	// C++14 enabled:
	//connect(curve, qOverload<bool>(&XYCurve::visibilityChanged), this, &InfoElement::curveVisibilityChanged);
	connect(curve, static_cast<void(XYCurve::*)(bool)>(&XYCurve::visibilityChanged), this, &InfoElement::curveVisibilityChanged);
	custompoint->setVisible(curve->isVisible());

	if (d->m_index < 0)
		d->m_index = curve->xColumn()->indexForValue(custompoint->position().x());
	if (d->m_index < 0)
		d->m_index = 0;

	struct MarkerPoints_T markerpoint = {custompoint, custompoint->path(), curve, curve->path()};
	markerpoints.append(markerpoint);
}

/*!
 * \brief InfoElement::addCurvePath
 * When loading infoelement from xml file, there is no information available, which curves are loaded.
 * So only the path will be stored and after all curves where loaded the curves will be assigned to the InfoElement
 * with the function assignCurve
 * Assumption: if custompoint!=nullptr then the custompoint was already added to the InfoElement previously. Here
 * only new created CustomPoints will be added to the InfoElement
 * @param curvePath path from the curve
 * @param custompoint adding already created custom point
 */
void InfoElement::addCurvePath(QString &curvePath, CustomPoint* custompoint) {
	Q_D(InfoElement);

	for(auto markerpoint: markerpoints) {
		if(curvePath == markerpoint.curvePath)
			return;
	}

	if (!custompoint) {
		custompoint = new CustomPoint(d->plot, "Markerpoint");
		custompoint->setVisible(false);
		addChild(custompoint);
	}
	struct MarkerPoints_T markerpoint = {custompoint, custompoint->path(), nullptr, curvePath};
	markerpoints.append(markerpoint);
}

/*!
 * \brief assignCurve
 * Finds the curve with the path stored in the markerpoints and assigns the pointer to markerpoints
 * @param curves
 * \return true if all markerpoints are assigned with a curve, false if one or more markerpoints don't have a curve assigned
 */
bool InfoElement::assignCurve(const QVector<XYCurve *> &curves) {

	bool success = true;
	for (int i =0; i< markerpoints.length(); i++) {
		for (auto curve: curves) {
			QString curvePath = curve->path();
			if(markerpoints[i].curvePath == curve->path()) {
				markerpoints[i].curve = curve;
				// C++14 enabled:
				//connect(curve, qOverload<bool>(&XYCurve::visibilityChanged), this, &InfoElement::curveVisibilityChanged);
				connect(curve, static_cast<void(XYCurve::*)(bool)>(&XYCurve::visibilityChanged), this, &InfoElement::curveVisibilityChanged);
				markerpoints[i].customPoint->setVisible(curve->isVisible()); // initial visibility
				break;
			}
		}
	}

	// check if all markerpoints have a valid curve
	// otherwise delete customPoint with no valid curve
	for (int i=markerpoints.count()-1; i >= 0; i--) {
		if (markerpoints[i].curve == nullptr) {
			removeChild(markerpoints[i].customPoint);
			success = false;
		}
	}

	retransform();
	return success;
}

/*!
 * Remove markerpoint from a curve
 * @param curve
 */
void InfoElement::removeCurve(const XYCurve* curve) {
	for (int i=0; i< markerpoints.length(); i++) {
		if (markerpoints[i].curve == curve) {
			disconnect(curve, static_cast<void(XYCurve::*)(bool)>(&XYCurve::visibilityChanged), this, &InfoElement::curveVisibilityChanged);
			removeChild(markerpoints[i].customPoint);
		}
	}
}

/*!
 * Set the z value of the label and the custompoints higher than the infoelement
 * @param value
 */
void InfoElement::setZValue(qreal value) {
	graphicsItem()->setZValue(value);

	label->setZValue(value+1);

	for (auto markerpoint: markerpoints)
		markerpoint.customPoint->setZValue(value+1);
}

/*!
 * Returns the amount of markerpoints. Used in the InfoElementDock to fill listWidget.
 */
int InfoElement::markerPointsCount() {
	return markerpoints.length();
}

TextLabel::GluePoint InfoElement::gluePoint(int index) {
	return label->gluePointAt(index);
}

int InfoElement::gluePointsCount() {
	return label->gluePointCount();
}

/*!
 * Returns the Markerpoint at index \p index. Used in the InfoElementDock to fill listWidget
 * @param index
 */
InfoElement::MarkerPoints_T InfoElement::markerPointAt(int index) {
	return markerpoints.at(index);
}

/*!
 * create Text which will be shown in the TextLabel
 * @return Text
 */
TextLabel::TextWrapper InfoElement::createTextLabelText() {

	if (!label || markerPointsCount() == 0)
		return TextLabel::TextWrapper();
	// TODO: save positions of the variables in extra variables to replace faster, because replace takes long time
	TextLabel::TextWrapper wrapper = label->text();

	AbstractColumn::ColumnMode columnMode = markerpoints[0].curve->xColumn()->columnMode();
	QString placeholderText = wrapper.textPlaceholder;
	if (!wrapper.teXUsed) {
		double value = markerpoints[0].x;
		if (columnMode== AbstractColumn::ColumnMode::Numeric ||
				columnMode == AbstractColumn::ColumnMode::Integer)
			placeholderText.replace("&amp;(x)",QString::number(value));
		else if (columnMode== AbstractColumn::ColumnMode::Day ||
				 columnMode == AbstractColumn::ColumnMode::Month ||
				 columnMode == AbstractColumn::ColumnMode::DateTime) {
			QDateTime dateTime;
			dateTime.setTime_t(value);
			QString dateTimeString = dateTime.toString();
			placeholderText.replace("&amp;(x)",dateTimeString);
		}
	} else {
		if (columnMode== AbstractColumn::ColumnMode::Numeric ||
				columnMode == AbstractColumn::ColumnMode::Integer)
			placeholderText.replace("&(x)",QString::number(markerpoints[0].x));
		else if (columnMode== AbstractColumn::ColumnMode::Day ||
				 columnMode == AbstractColumn::ColumnMode::Month ||
				 columnMode == AbstractColumn::ColumnMode::DateTime) {
			QDateTime dateTime;
			dateTime.setTime_t(markerpoints[0].x);
			QString dateTimeString = dateTime.toString();
			placeholderText.replace("&(x)",dateTimeString);
		}
	}

	for (int i=0; i< markerpoints.length(); i++) {
		QString replace;
		if(!wrapper.teXUsed)
			replace = QString("&amp;(");
		else
			replace = QString("&(");

		replace+=  markerpoints[i].curve->name() + QString(")");
		placeholderText.replace(replace, QString::number(markerpoints[i].y));
	}

	wrapper.text = placeholderText;
	return wrapper;
}

/*!
 * Returns plot, where this marker is used. Needed in the infoelement Dock
 * @return
 */
CartesianPlot* InfoElement::plot() {
	Q_D(InfoElement);
	return d->plot;
}

bool InfoElement::isVisible() const {
	Q_D(const InfoElement);
	return d->visible;
}

bool InfoElement::isTextLabel() const {
	return label != nullptr;
}

/*!
 * Will be called, when the label changes his position
 * @param position
 */
void InfoElement::labelPositionChanged(TextLabel::PositionWrapper position) {
	Q_UNUSED(position)
	if (m_suppressChildPositionChanged)
		return;

	Q_D(InfoElement);
	d->retransform();
}

void InfoElement::labelVisibleChanged(bool visible) {
	Q_UNUSED(visible)
	Q_D(InfoElement);
	d->update();
}

void InfoElement::labelTextWrapperChanged(TextLabel::TextWrapper wrapper) {
	Q_UNUSED(wrapper);
	if (m_setTextLabelText)
		return;

	m_setTextLabelText = true;
	label->setText(createTextLabelText());
	m_setTextLabelText = false;
}

/*!
 * \brief InfoElement::moveElementBegin
 * Called, when a child is moved in front or behind another element.
 * Needed, because the child calls child removed, when moving and then
 * everything will be deleted
 */
void InfoElement::moveElementBegin() {
	m_suppressChildRemoved = true;
}

/*!
 * \brief InfoElement::moveElementEnd
 * Called, when a child is moved in front or behind another element.
 * Needed, because the child calls child removed, when moving and then
 * everything will be deleted
 */
void InfoElement::moveElementEnd() {
	m_suppressChildRemoved = false;
}

void InfoElement::curveVisibilityChanged() {
	XYCurve* curve = static_cast<XYCurve*>(QObject::sender());
	bool visible = curve->isVisible();

	bool oneMarkerpointVisible = false;
	for (auto custompoint: markerpoints) {
		if (custompoint.curve == curve) {
			custompoint.customPoint->setVisible(visible);
		}

		if (custompoint.customPoint->isVisible())
			oneMarkerpointVisible = true;
	}

	// if curve was set to hidden, set InfoElement to first visible curve
	if (!visible) {
		for (auto custompoint: markerpoints) {
			if (custompoint.curve->isVisible()) {
				setConnectionLineCurveName(custompoint.curve->name());
				break;
			}
		}
	}

	// if no markerpoints are visible, hide all infoElement elements
	if ((!visible && markerpoints.count() == 0) || !oneMarkerpointVisible) {
		setConnectionLineVisible(false);
		setXPosLineVisible(false);
		label->setVisible(false);
	} else {
		setConnectionLineVisible(true);
		setXPosLineVisible(true);
		label->setVisible(true);
	}
}

void InfoElement::labelBorderShapeChanged() {
	Q_D(InfoElement);
	emit labelBorderShapeChangedSignal();
	d->retransform();
}

/*!
 * Delete child and remove from markerpoint list if it is a markerpoint. If it is a textlabel delete complete InfoElement
 */
void InfoElement::childRemoved(const AbstractAspect* parent, const AbstractAspect* before, const AbstractAspect* child) {
	Q_UNUSED(before)
	Q_D(InfoElement);

	// when childs are reordered, don't remove them
	// problem: when the order was changed the elements are deleted for a short time and recreated. This function will called then
	if (m_suppressChildRemoved)
		return;

	if (parent != this) // why do I need this?
		return;

	// point removed
	const CustomPoint* point = dynamic_cast<const CustomPoint*> (child);
	if (point != nullptr) {
		for (int i =0; i< markerpoints.length(); i++) {
			if (point == markerpoints[i].customPoint)
				markerpoints.removeAt(i);
			// no point->remove() needed, because it was already deleted
		}
	}

	// textlabel was deleted
	const TextLabel* textlabel = dynamic_cast<const TextLabel*>(child);
	if (textlabel) {
		Q_ASSERT(label == textlabel);
		label = nullptr;
		for (int i = 0; i < markerpoints.length(); i++) { // why it's not working without?
			m_suppressChildRemoved = true;
			markerpoints[i].customPoint->remove();
			markerpoints.removeAt(i);
			m_suppressChildRemoved = false;
		}
		remove(); // delete marker if textlabel was deleted, because there is no use case of this
	}

	d->retransform();
}

void InfoElement::childAdded(const AbstractAspect* child) {
	const CustomPoint* point = dynamic_cast<const CustomPoint*>(child);
	if (point) {
		connect(point, &CustomPoint::positionChanged, this, &InfoElement::pointPositionChanged);
		connect(point, &CustomPoint::moveBegin, this, &InfoElement::moveElementBegin);
		connect(point, &CustomPoint::moveEnd, this, &InfoElement::moveElementEnd);

		CustomPoint* p = const_cast<CustomPoint*>(point);
		p->setParentGraphicsItem(graphicsItem());
		// otherwise Custom point must be patched to handle discrete curve points.
		// This makes it much easier
		p->graphicsItem()->setFlag(QGraphicsItem::ItemIsMovable, false);
		return;
	}

	const TextLabel* labelChild = dynamic_cast<const TextLabel*>(child);
	if (labelChild) {
		connect(label, &TextLabel::positionChanged, this, &InfoElement::labelPositionChanged);
		connect(label, &TextLabel::visibleChanged, this, &InfoElement::labelVisibleChanged);
		connect(label, &TextLabel::textWrapperChanged, this, &InfoElement::labelTextWrapperChanged);
		connect(label, &TextLabel::borderShapeChanged, this, &InfoElement::labelBorderShapeChanged);
		connect(label, &TextLabel::moveBegin, this, &InfoElement::moveElementBegin);
		connect(label, &TextLabel::moveEnd, this, &InfoElement::moveElementEnd);
		connect(label, &TextLabel::rotationAngleChanged, this, &InfoElement::retransform);

		TextLabel* l = const_cast<TextLabel*>(labelChild);
		l->setParentGraphicsItem(graphicsItem());
	}
}

/*!
 * Will be called, when the customPoint changes his position
 * @param pos
 */
void InfoElement::pointPositionChanged(QPointF pos) {
	Q_UNUSED(pos)
	Q_D(InfoElement);

	if (m_suppressChildPositionChanged)
		return;

	CustomPoint* point = dynamic_cast<CustomPoint*>(QObject::sender());
	if (point == nullptr)
		return;

	// caĺculate new y value
	double x = point->position().x();
	double x_new;
	for (int i=0; i<markerpoints.length(); i++) {
		bool valueFound;
		double y = markerpoints[i].curve->y(x,x_new, valueFound);
		d->xPos = x_new;
		if (valueFound) {
			m_suppressChildPositionChanged = true;
			markerpoints[i].customPoint->graphicsItem()->setFlag(QGraphicsItem::ItemSendsGeometryChanges, false);
			DEBUG("InfoElement::pointPositionChanged, Set Position: ("<< x_new << "," << y << ")");
			markerpoints[i].customPoint->setPosition(QPointF(x_new,y));
			markerpoints[i].customPoint->graphicsItem()->setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);
            //QPointF position = d->cSystem->mapSceneToLogical(markerpoints[i].customPoint->graphicsItem()->pos());
			m_suppressChildPositionChanged = false;
		}
	}

	label->setText(createTextLabelText());
	d->retransform();
}

void InfoElement::setParentGraphicsItem(QGraphicsItem* item) {
	Q_D(InfoElement);
	d->setParentItem(item);
	d->updatePosition();
}

QGraphicsItem* InfoElement::graphicsItem() const {
	return d_ptr;
}

void InfoElement::setPrinting(bool on) {
	Q_D(InfoElement);
	d->m_printing = on;
}

void InfoElement::retransform() {
	Q_D(InfoElement);
	d->retransform();
}

void InfoElement::handleResize(double horizontalRatio, double verticalRatio, bool pageResize) {
    Q_UNUSED(horizontalRatio)
    Q_UNUSED(verticalRatio)
    Q_UNUSED(pageResize)
}

//##############################################################################
//######  Getter and setter methods ############################################
//##############################################################################

/* ============================ getter methods ================= */
BASIC_SHARED_D_READER_IMPL(InfoElement, bool, xposLineVisible, xposLineVisible);
BASIC_SHARED_D_READER_IMPL(InfoElement, bool, connectionLineVisible, connectionLineVisible);
BASIC_SHARED_D_READER_IMPL(InfoElement, double, xposLineWidth, xposLineWidth);
BASIC_SHARED_D_READER_IMPL(InfoElement, QColor, xposLineColor, xposLineColor);
BASIC_SHARED_D_READER_IMPL(InfoElement, double, connectionLineWidth, connectionLineWidth);
BASIC_SHARED_D_READER_IMPL(InfoElement, QColor, connectionLineColor, connectionLineColor);
BASIC_SHARED_D_READER_IMPL(InfoElement, int, gluePointIndex, gluePointIndex);
BASIC_SHARED_D_READER_IMPL(InfoElement, QString, connectionLineCurveName, connectionLineCurveName);
/* ============================ setter methods ================= */

// Problem: No member named 'Private' in 'InfoElement':
// Solution:
// Define "typedef  InfoElementPrivate Private;" in public section
// of InfoElement

// Problem: InfoElementPrivate has no member named 'name'
// Solution: implement function name()

STD_SETTER_CMD_IMPL_F_S(InfoElement, SetXPosLineVisible, bool, xposLineVisible, updateXPosLine);
void InfoElement::setXPosLineVisible(const bool xposLineVisible) {
	Q_D(InfoElement);
	if (xposLineVisible != d->xposLineVisible)
		exec(new InfoElementSetXPosLineVisibleCmd(d, xposLineVisible, ki18n("%1: set vertical line visible")));
}

STD_SETTER_CMD_IMPL_F_S(InfoElement, SetConnectionLineVisible, bool, connectionLineVisible, updateConnectionLine);
void InfoElement::setConnectionLineVisible(const bool connectionLineVisible) {
	Q_D(InfoElement);
	if (connectionLineVisible != d->connectionLineVisible)
		exec(new InfoElementSetConnectionLineVisibleCmd(d, connectionLineVisible, ki18n("%1: set connection line visible")));
}

STD_SETTER_CMD_IMPL_F_S(InfoElement, SetXPosLineWidth, double, xposLineWidth, updateXPosLine);
void InfoElement::setXPosLineWidth(const double xposLineWidth) {
	Q_D(InfoElement);
	if (xposLineWidth != d->xposLineWidth)
		exec(new InfoElementSetXPosLineWidthCmd(d, xposLineWidth, ki18n("%1: set vertical line width")));
}

STD_SETTER_CMD_IMPL_F_S(InfoElement, SetXPosLineColor, QColor, xposLineColor, updateXPosLine);
void InfoElement::setXPosLineColor(const QColor xposLineColor) {
	Q_D(InfoElement);
	if (xposLineColor != d->xposLineColor)
		exec(new InfoElementSetXPosLineColorCmd(d, xposLineColor, ki18n("%1: set vertical line color")));
}

STD_SETTER_CMD_IMPL_F_S(InfoElement, SetConnectionLineWidth, double, connectionLineWidth, updateConnectionLine);
void InfoElement::setConnectionLineWidth(const double connectionLineWidth) {
	Q_D(InfoElement);
	if (connectionLineWidth != d->connectionLineWidth)
		exec(new InfoElementSetConnectionLineWidthCmd(d, connectionLineWidth, ki18n("%1: set connection line width")));
}

STD_SETTER_CMD_IMPL_F_S(InfoElement, SetConnectionLineColor, QColor, connectionLineColor, updateConnectionLine);
void InfoElement::setConnectionLineColor(const QColor connectionLineColor) {
	Q_D(InfoElement);
	if (connectionLineColor != d->connectionLineColor)
		exec(new InfoElementSetConnectionLineColorCmd(d, connectionLineColor, ki18n("%1: set connection line color")));
}

STD_SETTER_CMD_IMPL_F_S(InfoElement, SetVisible, bool, visible, visibilityChanged);
void InfoElement::setVisible(const bool visible) {
	Q_D(InfoElement);
	if (visible != d->visible)
		exec(new InfoElementSetVisibleCmd(d, visible, ki18n("%1: set visible")));
}

STD_SETTER_CMD_IMPL_F_S(InfoElement, SetGluePointIndex, int, gluePointIndex, retransform);
void InfoElement::setGluePointIndex(const int value) {
	Q_D(InfoElement);
	if (value != d->gluePointIndex)
		exec(new InfoElementSetGluePointIndexCmd(d, value, ki18n("%1: set visible")));
}

STD_SETTER_CMD_IMPL_F_S(InfoElement, SetConnectionLineCurveName, QString, connectionLineCurveName, retransform);
void InfoElement::setConnectionLineCurveName(const QString name) {
	Q_D(InfoElement);
	if (name.compare(d->connectionLineCurveName) != 0)
		exec(new InfoElementSetConnectionLineCurveNameCmd(d, name, ki18n("%1: set visible")));
}

//##############################################################################
//######  SLOTs for changes triggered via QActions in the context menu  ########
//##############################################################################

//##############################################################################
//####################### Private implementation ###############################
//##############################################################################

InfoElementPrivate::InfoElementPrivate(InfoElement* owner,CartesianPlot* plot):
	plot(plot),
	q(owner) {
	init();
}

InfoElementPrivate::InfoElementPrivate(InfoElement* owner, CartesianPlot* plot, const XYCurve* curve):
	plot(plot),
	q(owner) {
	Q_UNUSED(curve)
	init();
}

void InfoElementPrivate::init() {
	setFlag(QGraphicsItem::ItemIsMovable, false);
	setFlag(QGraphicsItem::ItemClipsChildrenToShape, true);
	setFlag(QGraphicsItem::ItemIsSelectable, true);
	setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);
	setFlag(QGraphicsItem::ItemIsFocusable, true);

	if(plot)
		cSystem =  dynamic_cast<const CartesianCoordinateSystem*>(plot->coordinateSystem());
}

QString InfoElementPrivate::name() const {
	return q->name();
}

/*!
	calculates the position and the bounding box of the InfoElement. Called on geometry changes.
	Or when the label or the point where moved
 */
void InfoElementPrivate::retransform() {
	if (!q->label || q->markerpoints.isEmpty())
		return;

	q->m_suppressChildPositionChanged = true;

	q->label->retransform();

	for (auto markerpoint: q->markerpoints)
		markerpoint.customPoint->retransform();

	// line goes to the first pointPos
	QPointF pointPos = cSystem->mapLogicalToScene(q->markerpoints[0].customPoint->position(), AbstractCoordinateSystem::MappingFlag::SuppressPageClipping);
	for (int i=1; i< q->markerPointsCount(); i++) {
		if (q->markerpoints[i].curve->name().compare(connectionLineCurveName) == 0) {
			pointPos = cSystem->mapLogicalToScene(q->markerpoints[i].customPoint->position(), AbstractCoordinateSystem::MappingFlag::SuppressPageClipping);
			break;
		}
	}

	// use limit function like in the cursor! So the line will be drawn only till the border of the cartesian Plot
	QPointF labelPos;
	if (gluePointIndex < 0)
		labelPos = q->label->findNearestGluePoint(pointPos);
	else
		labelPos = q->label->gluePointAt(gluePointIndex).point;

	double x,y;
	QPointF min_scene = cSystem->mapLogicalToScene(QPointF(plot->xMin(),plot->yMin()));
	QPointF max_scene = cSystem->mapLogicalToScene(QPointF(plot->xMax(),plot->yMax()));

	y = abs(max_scene.y() - min_scene.y()) / 2;
	x = abs(max_scene.x() - min_scene.x()) / 2;

	QPointF labelPosItemCoords = mapFromParent(labelPos); // calculate item coords from scene coords
	QPointF pointPosItemCoords = mapFromParent(pointPos); // calculate item coords from scene coords

	setFlag(QGraphicsItem::ItemSendsGeometryChanges, false);

	boundingRectangle.setTopLeft(mapFromParent(plot->plotArea()->graphicsItem()->boundingRect().topLeft()));
	boundingRectangle.setBottomRight(mapFromParent(plot->plotArea()->graphicsItem()->boundingRect().bottomRight()));

	if (boundingRectangle.contains(labelPosItemCoords) && boundingRectangle.contains(pointPosItemCoords))
		connectionLine = QLineF(labelPosItemCoords.x(), labelPosItemCoords.y(), pointPosItemCoords.x(), pointPosItemCoords.y());
	else
		connectionLine = QLineF();

	xposLine = QLineF(pointPosItemCoords.x(), 0, pointPosItemCoords.x(), 2 * y);

	QPointF itemPos;
	//DEBUG("ConnectionLine: P1.x: " << (connectionLine.p1()).x() << "P2.x: " << (connectionLine.p2()).x());
	itemPos.setX(x); // x is always between the labelpos and the point pos
	if (max_scene.y() < min_scene.y())
		itemPos.setY(max_scene.y());
	else
		itemPos.setY(min_scene.y());

	if (max_scene.x() < min_scene.x())
		itemPos.setX(max_scene.x());
	else
		itemPos.setX(min_scene.x());

	setPos(itemPos);

	update(boundingRect());
	setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);

	q->m_suppressChildPositionChanged = false;
}

void InfoElementPrivate::updatePosition() {

}

/*!
 * Repainting to update xposLine
 */
void InfoElementPrivate::updateXPosLine() {
	update(boundingRectangle);
}

/*!
 * Repainting to updateConnectionLine
 */
void InfoElementPrivate::updateConnectionLine() {
	update(boundingRect());
}

void InfoElementPrivate::visibilityChanged() {
	for(auto markerpoint: q->markerpoints)
		markerpoint.customPoint->setVisible(visible);
	if(q->label)
		q->label->setVisible(visible);
	update(boundingRect());
}

//reimplemented from QGraphicsItem
QRectF InfoElementPrivate::boundingRect() const {
	return boundingRectangle;
}

void InfoElementPrivate::paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget* widget) {
	Q_UNUSED(widget)
	if (!visible)
		return;

	if (q->markerpoints.isEmpty())
		return;

	// do not draw connection line when the label is not visible
	if (connectionLineVisible && q->label->isVisible()) {
		QPen pen(connectionLineColor, connectionLineWidth);
		painter->setPen(pen);
		painter->drawLine(connectionLine);
	}

	// draw vertical line, which connects all points together
	if (xposLineVisible) {
		QPen pen(xposLineColor, xposLineWidth);
		painter->setPen(pen);
		painter->drawLine(xposLine);
	}
}

QVariant InfoElementPrivate::itemChange(GraphicsItemChange change, const QVariant &value) {
	return QGraphicsItem::itemChange(change, value);
}

void InfoElementPrivate::mousePressEvent(QGraphicsSceneMouseEvent* event) {
	if (event->button() == Qt::MouseButton::LeftButton) {

		if (xposLineVisible) {
			if (abs(xposLine.x1()-event->pos().x())< ((xposLineWidth < 3)? 3: xposLineWidth)) {
                if (!isSelected())
                    setSelected(true);
				m_suppressKeyPressEvents = false;
				oldMousePos = mapToParent(event->pos());
				event->accept();
				setFocus();
				return;
			}
		}/* else {
			for (int i=0; i< q->markerPointsCount(); i++) {
				InfoElement::MarkerPoints_T markerpoint =  q->markerPointAt(i);
				//if (markerpoint.customPoint->symbolSize())
			}
		}*/

		// https://stackoverflow.com/questions/11604680/point-laying-near-line
		double dx12 = connectionLine.x2()-connectionLine.x1();
		double dy12 = connectionLine.y2()-connectionLine.y1();
		double vecLenght = sqrt(pow(dx12,2)+pow(dy12,2));
		QPointF unitvec(dx12/vecLenght,dy12/vecLenght);

		double dx1m = event->pos().x() - connectionLine.x1();
		double dy1m = event->pos().y() - connectionLine.y1();

		double dist_segm = abs(dx1m*unitvec.y() - dy1m*unitvec.x());
		double scalar_product = dx1m*unitvec.x()+dy1m*unitvec.y();
		DEBUG("DIST_SEGMENT   " << dist_segm << "SCALAR_PRODUCT: " << scalar_product << "VEC_LENGTH: " << vecLenght);

		if (scalar_product > 0) {
			if (scalar_product < vecLenght && dist_segm < ((connectionLineWidth < 3) ? 3: connectionLineWidth)) {
				event->accept();
				if (!isSelected())
					setSelected(true);
				oldMousePos = mapToParent(event->pos());
				m_suppressKeyPressEvents = false;
				event->accept();
				setFocus();
				return;
			}
		}

		m_suppressKeyPressEvents = true;
		event->ignore();
		if (isSelected())
			setSelected(false);
	}
	QGraphicsItem::mousePressEvent(event);
}

void InfoElementPrivate::mouseMoveEvent(QGraphicsSceneMouseEvent* event) {

	QPointF eventPos = mapToParent(event->pos());
	DEBUG("EventPos: " << eventPos.x() << " Y: " << eventPos.y());
	QPointF delta = eventPos - oldMousePos;
	if (delta == QPointF(0,0))
		return;

	QPointF eventLogicPos = cSystem->mapSceneToLogical(eventPos, AbstractCoordinateSystem::MappingFlag::SuppressPageClipping);
	QPointF delta_logic =  eventLogicPos - cSystem->mapSceneToLogical(oldMousePos);

	if (!q->label)
		return;
	if (q->markerpoints.isEmpty())
		return;

	for (auto markerpoint: q->markerpoints)
		markerpoint.customPoint->graphicsItem()->setFlag(QGraphicsItem::ItemSendsGeometryChanges, false);

	bool newMarkerPointPos = false;

	q->label->graphicsItem()->setFlag(QGraphicsItem::ItemSendsGeometryChanges, false);

	// TODO: find better method to do this. It's inefficient.
	// Finding which curve should be used to find the new values
	double x = q->markerpoints[0].x;
	int activeIndex = 0;
	for (int i=1; i< q->markerPointsCount(); i++) {
		if (q->markerpoints[i].curve->name().compare(connectionLineCurveName) == 0) {
			// not possible to use index, because when the number of elements in the columns of the curves are not the same there is a problem
			x = q->markerpoints[i].x; //q->markerpoints[i].curve->xColumn()->valueAt(m_index)
			activeIndex = i;
			break;
		}
	}
	x += delta_logic.x();
	DEBUG("markerpoints[0].x: " << q->markerpoints[0].x << ", markerpoints[0].y: " << q->markerpoints[0].y << ", Scene xpos: " << x);
	for (int i =0; i < q->markerpoints.length(); i++) {
		bool valueFound = false;
		double x_new = NAN;
		int index = -1;

		// find index and y value for a corresponding x value
		double y;
		if (q->markerpoints[i].curve) {
			index = q->markerpoints[i].curve->xColumn()->indexForValue(x);
			x_new = q->markerpoints[i].curve->xColumn()->valueAt(index);
			y = q->markerpoints[i].curve->yColumn()->valueAt(index);
			valueFound = true;
		} else
			y = 0;

		if (valueFound) {
			if (abs(x_new - q->markerpoints[i].x) > 0 && i == activeIndex)
				newMarkerPointPos = true;
			q->markerpoints[i].y = y;
			q->markerpoints[i].x = x_new;
			q->m_suppressChildPositionChanged = true;
			q->markerpoints[i].customPoint->setPosition(QPointF(x_new,y));
			q->m_suppressChildPositionChanged = false;
			m_index = index;
		} else
			DEBUG("No value found for Logicalpoint" << i);
	}

	if (newMarkerPointPos) { // move oldMousePos only when the markerpoints are moved to the next value
		q->label->setText(q->createTextLabelText());
        //double x_label = q->label->position().point.x() + delta.x();
        //double y_label = q->label->position().point.y();
		//q->label->setPosition(QPointF(x_label,y_label)); // don't move label
		oldMousePos = eventPos;
	}

	q->label->graphicsItem()->setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);
	for (auto markerpoint: q->markerpoints)
		markerpoint.customPoint->graphicsItem()->setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);

	retransform();
}

void InfoElementPrivate::keyPressEvent(QKeyEvent * event) {
	if (m_suppressKeyPressEvents) {
		event->ignore();
		return QGraphicsItem::keyPressEvent(event);
	}

	TextLabel::TextWrapper text;
	if (event->key() == Qt::Key_Right || event->key() == Qt::Key_Left) {
		int index;
		if (event->key() == Qt::Key_Right)
			index = 1;
		else
			index = -1;

        double x, xNew;
		bool valueFound;
		QPointF pointPosition;
		int rowCount;

		// problem: when curves have different number of samples, the points are anymore aligned
		// with the vertical line
		QPointF position = q->markerpoints[0].customPoint->position();
		m_index += index;
		auto* column = q->markerpoints[0].curve->xColumn();
		rowCount = column->rowCount();
		if (m_index > rowCount - 1)
			m_index = rowCount - 1;
		if (m_index < 0)
			m_index = 0;

		x = column->valueAt(m_index);
		for (int i=1; i< q->markerPointsCount(); i++) {
			if (q->markerpoints[i].curve->name().compare(connectionLineCurveName) == 0) {
				position = q->markerpoints[i].customPoint->position();
				if (m_index > rowCount - 1)
					m_index = rowCount - 1;
				if (m_index < 0)
					m_index = 0;
				q->markerpoints[i].curve->xColumn()->valueAt(m_index);
				break;
			}
		}

		xNew = x;
		for (int i =0; i< q->markerpoints.length(); i++) {
			q->markerpoints[i].x = x;
			auto* curve = q->markerpoints[i].curve;
			if (curve->xColumn()->rowCount() == rowCount) { // if the other columns have the same length it can simply used the index
				q->markerpoints[i].y = curve->yColumn()->valueAt(m_index);
				valueFound = true;
			} else // if the length of the columns of the other curves are different, the y value must be searched
				q->markerpoints[i].y = curve->y(x, xNew, valueFound);
			if (valueFound) { // new set by curve->y()
				pointPosition.setX(xNew);
				pointPosition.setY(q->markerpoints[i].y);
				DEBUG("X_old: " << q->markerpoints[i].customPoint->position().x() << "X_new: " << x);
				q->m_suppressChildPositionChanged = true;
				q->markerpoints[i].customPoint->setPosition(pointPosition);
				q->m_suppressChildPositionChanged = false;
			}
		}
		q->label->setText(q->createTextLabelText());
		retransform();

	}
}

//##############################################################################
//##################  Serialization/Deserialization  ###########################
//##############################################################################
void InfoElement::save(QXmlStreamWriter* writer) const {
	Q_D(const InfoElement);

	writer->writeStartElement( "infoElement" );
	writeBasicAttributes(writer);
	writeCommentElement(writer);

	//geometry
	writer->writeStartElement( "geometry" );
	writer->writeAttribute( "visible", QString::number(d->visible) );
	writer->writeAttribute("connectionLineWidth", QString::number(connectionLineWidth()));
	writer->writeAttribute("connectionLineColor_r", QString::number(connectionLineColor().red()));
	writer->writeAttribute("connectionLineColor_g", QString::number(connectionLineColor().green()));
	writer->writeAttribute("connectionLineColor_b", QString::number(connectionLineColor().blue()));
	writer->writeAttribute("xposLineWidth", QString::number(xposLineWidth()));
	writer->writeAttribute("xposLineColor_r", QString::number(xposLineColor().red()));
	writer->writeAttribute("xposLineColor_g", QString::number(xposLineColor().green()));
	writer->writeAttribute("xposLineColor_b", QString::number(xposLineColor().blue()));
	writer->writeAttribute("xposLineVisible", QString::number(xposLineVisible()));
	writer->writeAttribute("connectionLineCurveName", connectionLineCurveName());
	writer->writeAttribute("gluePointIndex", QString::number(gluePointIndex()));
	writer->writeEndElement();

	writer->writeStartElement("settings");
	writer->writeAttribute("markerIndex", QString::number(d->m_index));
	writer->writeEndElement();

	label->save(writer);
	QString path;
	for (auto custompoint: markerpoints) {
		custompoint.customPoint->save(writer);
		writer->writeStartElement("markerPoint");
		writer->writeAttribute(QLatin1String("curvepath"), path = custompoint.curve->path());
		writer->writeAttribute(QLatin1String("x"), QString::number(custompoint.x));
		writer->writeAttribute(QLatin1String("y"), QString::number(custompoint.y));
		writer->writeEndElement(); // close "markerPoint
	}
	writer->writeEndElement(); // close "infoElement"
}

bool InfoElement::load(XmlStreamReader* reader, bool preview) {
	if (!readBasicAttributes(reader))
		return false;

	Q_D(InfoElement);

	CustomPoint* markerpoint = nullptr;
	bool markerpointFound = false;
	QXmlStreamAttributes attribs;
	QString str;
	KLocalizedString attributeWarning = ki18n("Attribute '%1' missing or empty, default value is used");

	while (!reader->atEnd()) {
		reader->readNext();
		QStringRef text =  reader->text();
		if (reader->isEndElement() && reader->name() == "infoElement")
			break;

		if (!reader->isStartElement())
			continue;

		if (!preview && reader->name() == "comment") {
			if (!readCommentElement(reader)) return false;
		} else if (reader->name() == "geometry") {
			attribs = reader->attributes();

			str = attribs.value("visible").toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.subs("x").toString());
			else
				setVisible(str.toInt());

			str = attribs.value("connectionLineWidth").toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.subs("x").toString());
			else
				setConnectionLineWidth(str.toDouble());

			str = attribs.value("connectionLineColor_r").toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.subs("x").toString());
			else
				d->connectionLineColor.setRed(str.toInt());

			str = attribs.value("connectionLineColor_g").toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.subs("x").toString());
			else
				d->connectionLineColor.setGreen(str.toInt());

			str = attribs.value("connectionLineColor_b").toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.subs("x").toString());
			else
				d->connectionLineColor.setBlue(str.toInt());

			str = attribs.value("xposLineWidth").toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.subs("x").toString());
			else
				setXPosLineWidth(str.toDouble());

			str = attribs.value("xposLineColor_r").toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.subs("x").toString());
			else
				d->xposLineColor.setRed(str.toInt());

			str = attribs.value("xposLineColor_g").toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.subs("x").toString());
			else
				d->xposLineColor.setGreen(str.toInt());

			str = attribs.value("xposLineColor_b").toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.subs("x").toString());
			else
				d->xposLineColor.setBlue(str.toInt());

			str = attribs.value("xposLineVisible").toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.subs("x").toString());
			else
				setXPosLineVisible(str.toInt());

			str = attribs.value("connectionLineCurveName").toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.subs("x").toString());
			else
				setConnectionLineCurveName(str);

			str = attribs.value("gluePointIndex").toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.subs("x").toString());
			else
				setGluePointIndex(str.toInt());

		} else if (reader->name() == "textLabel") {
			if (!label) {
				label = new TextLabel("InfoElementLabel", d->plot);
				this->addChild(label);
			}
			if (!label->load(reader, preview))
				return false;
		} else if (reader->name() == "customPoint") {
			// Marker must have at least one curve
			if (markerpointFound) { // must be cleared by markerPoint
				delete markerpoint;
				return false;
			}
			markerpoint = new CustomPoint(d->plot, "Marker");
			if (!markerpoint->load(reader,preview)) {
				delete  markerpoint;
				return false;
			}
			this->addChild(markerpoint);
			markerpointFound = true;

		} else if (reader->name() == "markerPoint") {
			markerpointFound = false;
			QString path;
			attribs = reader->attributes();
			path = attribs.value("curvepath").toString();
			addCurvePath(path, markerpoint);
			markerpoints.last().x = attribs.value("x").toDouble();
			markerpoints.last().y = attribs.value("y").toDouble();
		} else if (reader->name() == "settings") {
			attribs = reader->attributes();
			str = attribs.value("markerIndex").toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.subs("x").toString());
			else
				d->m_index = str.toInt();
		}
	}

	if (markerpointFound) {
		// problem, if a markerpoint has no markerPointCurve
		delete markerpoint;
		return false;
	}
	return true;
}

//##############################################################################
//#########################  Theme management ##################################
//##############################################################################
void InfoElement::loadThemeConfig(const KConfig& config) {
	Q_D(InfoElement);

	//use the color for the axis line from the theme also for info element's lines
	const KConfigGroup& group = config.group("Axis");
	d->connectionLineColor = group.readEntry("LineColor", QColor(Qt::black));
	d->xposLineColor = d->connectionLineColor;
}
