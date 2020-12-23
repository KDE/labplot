/***************************************************************************
	File                 : InfoElement.cpp
	Project              : LabPlot
	Description          : Marker which can highlight points of curves and
						   show their values
	--------------------------------------------------------------------
	Copyright            : (C) 2020 Martin Marmsoler (martin.marmsoler@gmail.com)
	Copyright            : (C) 2020 Alexander Semke (alexander.semke@web.de)
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

#include "backend/core/Project.h"
#include "backend/lib/commandtemplates.h"
#include "backend/lib/XmlStreamReader.h"
#include "backend/worksheet/plots/cartesian/CartesianCoordinateSystem.h"
#include "backend/worksheet/plots/cartesian/CartesianPlot.h"
#include "backend/worksheet/plots/cartesian/CustomPoint.h"
#include "backend/worksheet/plots/cartesian/XYCurve.h"
#include "backend/worksheet/InfoElementPrivate.h"
#include "backend/worksheet/TextLabel.h"
#include "backend/worksheet/Worksheet.h"

#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QKeyEvent>
#include <QAction>
#include <QMenu>
#include <QTextEdit>
#include <QDateTime>

InfoElement::InfoElement(const QString& name, CartesianPlot* plot):
	WorksheetElement(name, AspectType::InfoElement),
	d_ptr(new InfoElementPrivate(this, plot)) {
	Q_D(InfoElement);
	init();
	setVisible(false);
	d->retransform();
}

InfoElement::InfoElement(const QString& name, CartesianPlot* plot, const XYCurve* curve, double pos):
	WorksheetElement(name, AspectType::InfoElement),
	// must be at least, because otherwise label ist not a nullptr
	d_ptr(new InfoElementPrivate(this, plot, curve)) {
	Q_D(InfoElement);

	init();

	m_suppressChildPositionChanged = true;

	if (curve) {
		d->connectionLineCurveName = curve->name();
		CustomPoint* custompoint = new CustomPoint(plot, curve->name());
		addChild(custompoint);
		InfoElement::MarkerPoints_T markerpoint(custompoint, custompoint->path(), curve, curve->path());
		markerpoints.append(markerpoint);

		// setpos after label was created
		if (curve->xColumn() && curve->yColumn()) {
			bool valueFound;
			double xpos;
			double y = curve->y(pos,xpos,valueFound);
			if (valueFound) {
				d->xPos = xpos;
				d->position = xpos;
				d->m_index = curve->xColumn()->indexForValue(xpos);
				markerpoints.last().x = xpos;
				markerpoints.last().y = y;
				custompoint->setPosition(QPointF(xpos,y));
				DEBUG("Value found");
			}
		} else {
			d->xPos = 0;
			d->position = 0;
			markerpoints.last().x = 0;
			markerpoints.last().y = 0;
			custompoint->setPosition(d->cSystem->mapSceneToLogical(QPointF(0,0)));
			DEBUG("Value not found");
		}

		connect(curve, QOverload<bool>::of(&XYCurve::visibilityChanged), this, &InfoElement::curveVisibilityChanged);
		custompoint->setVisible(curve->isVisible());

		setVisible(true);
	} else
		setVisible(false);

	TextLabel::TextWrapper text;
	text.allowPlaceholder = true;

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
		text.textPlaceholder = i18n("Please Add Text here");

	m_title->setText(text);

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

	//	removeChild(m_title);

	//	m_suppressChildRemoved = false;
}

void InfoElement::init() {
	Q_D(InfoElement);

	initActions();
	initMenus();

	connect(this, &InfoElement::aspectRemoved, this, &InfoElement::childRemoved);
	connect(this, &InfoElement::aspectAdded, this, &InfoElement::childAdded);

	m_title = new TextLabel(i18n("Label"), d->plot);
	addChild(m_title);
	m_title->setHidden(true);
	m_title->enableCoordBinding(true);
	m_title->setCoordBinding(true);
	TextLabel::TextWrapper text;
	text.allowPlaceholder = true;
	m_title->setText(text); // set placeholder to true

	//use the color for the axis line from the theme also for info element's lines
	KConfig config;
	loadThemeConfig(config);
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

	project()->setSuppressAspectAddedSignal(true);

	if (!custompoint) {
		custompoint = new CustomPoint(d->plot, curve->name());
		addChild(custompoint);

		if (curve->xColumn() && curve->yColumn()) {
			bool valueFound;
			double x_new, y;
			if (markerpoints.isEmpty())
				y = curve->y(d->xPos, x_new, valueFound);
			else
				y = curve->y(markerpoints[0].customPoint->position().x(), x_new, valueFound);
			custompoint->setPosition(QPointF(x_new, y));
		}
	} else
		addChild(custompoint);

	project()->setSuppressAspectAddedSignal(true);

	connect(curve, QOverload<bool>::of(&XYCurve::visibilityChanged), this, &InfoElement::curveVisibilityChanged);
	connect(curve, &XYCurve::moveBegin, this, [this]() { m_curveGetsMoved = true; });
	connect(curve, &XYCurve::moveEnd, this, [this]() { m_curveGetsMoved = false; });
	custompoint->setVisible(curve->isVisible());

	if (d->m_index < 0 && curve->xColumn())
		d->m_index = curve->xColumn()->indexForValue(custompoint->position().x());

	struct MarkerPoints_T markerpoint = {custompoint, custompoint->path(), curve, curve->path()};
	markerpoints.append(markerpoint);

	if (markerpoints.length() == 1) {
		// Do a retransform, because when the first markerpoint
		// was added, after a curve was removed and added, the
		// position of the connection line must be recalculated
		retransform();
	}

	m_title->setVisible(true); //show in the worksheet view
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
		custompoint = new CustomPoint(d->plot, i18n("Symbol"));
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
				connect(curve, QOverload<bool>::of(&XYCurve::visibilityChanged), this, &InfoElement::curveVisibilityChanged);
				markerpoints[i].customPoint->setVisible(curve->isVisible()); // initial visibility
				break;
			}
		}
	}

	// check if all markerpoints have a valid curve
	// otherwise delete customPoint with no valid curve
	for (int i = markerpoints.count()-1; i >= 0; i--) {
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
	if (m_curveGetsMoved)
		return;

	for (int i = 0; i< markerpoints.length(); i++) {
		if (markerpoints[i].curve == curve) {
			disconnect(curve, QOverload<bool>::of(&XYCurve::visibilityChanged), this, &InfoElement::curveVisibilityChanged);
			removeChild(markerpoints[i].customPoint);
		}
	}

	//hide the label if now curves are selected
	if (markerpoints.isEmpty()) {
		m_title->setVisible(false); //hide in the worksheet view
		Q_D(InfoElement);
		d->update(); //redraw to remove all children graphic items belonging to InfoElement
	}
}

/*!
 * Set the z value of the m_title and the custompoints higher than the infoelement
 * @param value
 */
void InfoElement::setZValue(qreal value) {
	graphicsItem()->setZValue(value);

	m_title->setZValue(value+1);

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
	return m_title->gluePointAt(index);
}

int InfoElement::gluePointsCount() {
	return m_title->gluePointCount();
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
 * Called when:
 *  - The position of the infoelement was changed
 *  - a curve was removed
 *  - a curve was added
 * @return Text
 */
TextLabel::TextWrapper InfoElement::createTextLabelText() {

	// TODO: save positions of the variables in extra variables to replace faster, because replace takes long time
	TextLabel::TextWrapper wrapper = m_title->text();
	if (markerPointsCount() < 1) {
		DEBUG(wrapper.text.toStdString());
		DEBUG(wrapper.textPlaceholder.toStdString());
		wrapper.text = wrapper.textPlaceholder;
		return wrapper;
	}

	if (!markerpoints[0].curve->xColumn())
		return wrapper; //no data is set in the curve yet, nothing to do

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

TextLabel* InfoElement::title() {
	return m_title;
}

bool InfoElement::isTextLabel() const {
	return m_title != nullptr;
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
	m_title->setText(createTextLabelText());
	m_setTextLabelText = false;

	Q_D(InfoElement);
	d->retransform();
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
		if (custompoint.curve == curve)
			custompoint.customPoint->setVisible(visible);

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
		//TODO
// 		setConnectionLineVisible(false);
// 		setXPosLineVisible(false);
		m_title->setVisible(false);
	} else {
// 		setConnectionLineVisible(true);
// 		setXPosLineVisible(true);
		m_title->setVisible(true);
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
		// recreate text, because when marker was deleted,
		// the placeholder should not be replaced anymore by a value
		m_title->setText(createTextLabelText());
	}

	// textlabel was deleted
	const TextLabel* textlabel = dynamic_cast<const TextLabel*>(child);
	if (textlabel) {
		Q_ASSERT(m_title == textlabel);
		m_title = nullptr;
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

	const TextLabel* m_titleChild = dynamic_cast<const TextLabel*>(child);
	if (m_titleChild) {
		connect(m_title, &TextLabel::positionChanged, this, &InfoElement::labelPositionChanged);
		connect(m_title, &TextLabel::visibleChanged, this, &InfoElement::labelVisibleChanged);
		connect(m_title, &TextLabel::textWrapperChanged, this, &InfoElement::labelTextWrapperChanged);
		connect(m_title, &TextLabel::borderShapeChanged, this, &InfoElement::labelBorderShapeChanged);
		connect(m_title, &TextLabel::moveBegin, this, &InfoElement::moveElementBegin);
		connect(m_title, &TextLabel::moveEnd, this, &InfoElement::moveElementEnd);
		connect(m_title, &TextLabel::rotationAngleChanged, this, &InfoElement::retransform);

		TextLabel* l = const_cast<TextLabel*>(m_titleChild);
		l->setParentGraphicsItem(graphicsItem());
	}
}

/*!
 * \brief InfoElement::currentValue
 * Calculates the new x position from
 * \param new_x
 * \return
 */
int InfoElement::currentIndex(double x, double* found_x) {
	Q_D(InfoElement);

	for (auto markerpoint : markerpoints) {
		if (markerpoint.curve->name() == connectionLineCurveName()) {
			int index = markerpoint.curve->xColumn()->indexForValue(x);

			if (found_x && index >= 0) {
				auto mode = markerpoint.curve->xColumn()->columnMode();
				switch (mode) {
				case AbstractColumn::ColumnMode::Numeric:
				case AbstractColumn::ColumnMode::Integer:
				case AbstractColumn::ColumnMode::BigInt:
					*found_x = markerpoint.curve->xColumn()->valueAt(index);
					break;
				case AbstractColumn::ColumnMode::Text:
					break;
				case AbstractColumn::ColumnMode::DateTime:
				case AbstractColumn::ColumnMode::Month:
				case AbstractColumn::ColumnMode::Day:
					*found_x = markerpoint.curve->xColumn()->dateTimeAt(index).toMSecsSinceEpoch();
					break;
				}

				return index;
			}
		}
	}

	return -1;
}

double InfoElement::setMarkerpointPosition(double x) {
	// TODO: can be optimized when it will be checked if all markerpoints have the same xColumn, then the index m_index is the same
	Q_D(InfoElement);
	double x_new;
	double x_new_first = 0;
	for (int i=0; i<markerpoints.length(); i++) {
		bool valueFound;
		double y = markerpoints[i].curve->y(x,x_new, valueFound);
		d->xPos = x_new;
		if (i == 0)
			x_new_first = x_new;
		if (valueFound) {
			m_suppressChildPositionChanged = true;
			markerpoints[i].x = x_new;
			markerpoints[i].y = y;
			markerpoints[i].customPoint->graphicsItem()->setFlag(QGraphicsItem::ItemSendsGeometryChanges, false);
			markerpoints[i].customPoint->setPosition(QPointF(x_new,y));
			markerpoints[i].customPoint->graphicsItem()->setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);
			//QPointF position = d->cSystem->mapSceneToLogical(markerpoints[i].customPoint->graphicsItem()->pos());
			m_suppressChildPositionChanged = false;
		}
	}
	return x_new_first;
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

	setPosition(point->position().x());
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
BASIC_SHARED_D_READER_IMPL(InfoElement, double, position, position);
BASIC_SHARED_D_READER_IMPL(InfoElement, int, gluePointIndex, gluePointIndex);
CLASS_SHARED_D_READER_IMPL(InfoElement, QString, connectionLineCurveName, connectionLineCurveName);
CLASS_SHARED_D_READER_IMPL(InfoElement, QPen, verticalLinePen, verticalLinePen)
BASIC_SHARED_D_READER_IMPL(InfoElement, qreal, verticalLineOpacity, verticalLineOpacity)
CLASS_SHARED_D_READER_IMPL(InfoElement, QPen, connectionLinePen, connectionLinePen)
BASIC_SHARED_D_READER_IMPL(InfoElement, qreal, connectionLineOpacity, connectionLineOpacity)

/* ============================ setter methods ================= */
STD_SETTER_CMD_IMPL_F_S(InfoElement, SetPosition, double, position, retransform);
void InfoElement::setPosition(double pos) {
	Q_D(InfoElement);
	double value;
	int index = currentIndex(pos, &value);
	if (index < 0)
		return;

	if (value != d->position) {
		d->m_index = index;
		setMarkerpointPosition(value);
		m_title->setText(createTextLabelText());
		exec(new InfoElementSetPositionCmd(d, pos, ki18n("%1: set position")));
	}
}

STD_SETTER_CMD_IMPL_F_S(InfoElement, SetGluePointIndex, int, gluePointIndex, retransform);
void InfoElement::setGluePointIndex(int value) {
	Q_D(InfoElement);
	if (value != d->gluePointIndex)
		exec(new InfoElementSetGluePointIndexCmd(d, value, ki18n("%1: set gluepoint index")));
}

STD_SETTER_CMD_IMPL_F_S(InfoElement, SetConnectionLineCurveName, QString, connectionLineCurveName, retransform);
void InfoElement::setConnectionLineCurveName(const QString& name) {
	Q_D(InfoElement);
	if (name.compare(d->connectionLineCurveName) != 0)
		exec(new InfoElementSetConnectionLineCurveNameCmd(d, name, ki18n("%1: set connectionline curve name")));
}

STD_SETTER_CMD_IMPL_F_S(InfoElement, SetVisible, bool, visible, visibilityChanged);
void InfoElement::setVisible(bool visible) {
	Q_D(InfoElement);
	if (visible != d->visible)
		exec(new InfoElementSetVisibleCmd(d, visible, ki18n("%1: set visible")));
}

//vertical line
STD_SETTER_CMD_IMPL_F_S(InfoElement, SetVerticalLinePen, QPen, verticalLinePen, updateVerticalLine)
void InfoElement::setVerticalLinePen(const QPen& pen) {
	Q_D(InfoElement);
	if (pen != d->verticalLinePen)
		exec(new InfoElementSetVerticalLinePenCmd(d, pen, ki18n("%1: set vertical line style")));
}

STD_SETTER_CMD_IMPL_F_S(InfoElement, SetVerticalLineOpacity, qreal, verticalLineOpacity, update);
void InfoElement::setVerticalLineOpacity(qreal opacity) {
	Q_D(InfoElement);
	if (opacity != d->verticalLineOpacity)
		exec(new InfoElementSetVerticalLineOpacityCmd(d, opacity, ki18n("%1: set vertical line opacity")));
}

//connection line
STD_SETTER_CMD_IMPL_F_S(InfoElement, SetConnectionLinePen, QPen, connectionLinePen, updateConnectionLine)
void InfoElement::setConnectionLinePen(const QPen& pen) {
	Q_D(InfoElement);
	if (pen != d->connectionLinePen)
		exec(new InfoElementSetConnectionLinePenCmd(d, pen, ki18n("%1: set connection line style")));
}

STD_SETTER_CMD_IMPL_F_S(InfoElement, SetConnectionLineOpacity, qreal, connectionLineOpacity, update);
void InfoElement::setConnectionLineOpacity(qreal opacity) {
	Q_D(InfoElement);
	if (opacity != d->connectionLineOpacity)
		exec(new InfoElementSetConnectionLineOpacityCmd(d, opacity, ki18n("%1: set connection line opacity")));
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

	//TODO
	if(plot)
		cSystem =  dynamic_cast<const CartesianCoordinateSystem*>(plot->coordinateSystem(0));
}

QString InfoElementPrivate::name() const {
	return q->name();
}

/*!
	calculates the position and the bounding box of the InfoElement. Called on geometry changes.
	Or when the label or the point where moved
 */
void InfoElementPrivate::retransform() {
	if (!q->m_title || q->markerpoints.isEmpty())
		return;

	q->m_suppressChildPositionChanged = true;

	q->m_title->retransform();

	for (auto markerpoint : q->markerpoints)
		markerpoint.customPoint->retransform();

	//determine the position to connect the line to
	QPointF pointPos;
	for (int i = 0; i < q->markerPointsCount(); ++i) {
		const auto* curve = q->markerpoints[i].curve;
		if (curve && curve->name() == connectionLineCurveName) {
			pointPos = cSystem->mapLogicalToScene(q->markerpoints[i].customPoint->position(),
												  AbstractCoordinateSystem::MappingFlag::SuppressPageClipping);
			break;
		}
	}

	// use limit function like in the cursor! So the line will be drawn only till the border of the cartesian Plot
	QPointF m_titlePos;
	if (gluePointIndex < 0)
		m_titlePos = q->m_title->findNearestGluePoint(pointPos);
	else
		m_titlePos = q->m_title->gluePointAt(gluePointIndex).point;

	//new bounding rectangle
	const QRectF& rect = plot->dataRect();
	boundingRectangle = mapFromParent(rect).boundingRect();

	//connection line
	const QPointF m_titlePosItemCoords = mapFromParent(m_titlePos); // calculate item coords from scene coords
	const QPointF pointPosItemCoords = mapFromParent(pointPos); // calculate item coords from scene coords
	if (boundingRectangle.contains(m_titlePosItemCoords) && boundingRectangle.contains(pointPosItemCoords))
		connectionLine = QLineF(m_titlePosItemCoords.x(), m_titlePosItemCoords.y(), pointPosItemCoords.x(), pointPosItemCoords.y());
	else
		connectionLine = QLineF();

	//vertical line
	xposLine = QLineF(pointPosItemCoords.x(), boundingRectangle.bottom(),
					  pointPosItemCoords.x(), boundingRectangle.top());

	//new item position
	setFlag(QGraphicsItem::ItemSendsGeometryChanges, false);
	QPointF itemPos(qMin(rect.top(), rect.bottom()),
					qMin(rect.left(), rect.right()));
	setPos(itemPos);
	update(boundingRectangle);
	setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);

	q->m_suppressChildPositionChanged = false;
}

void InfoElementPrivate::updatePosition() {

}

/*!
 * Repainting to update xposLine
 */
void InfoElementPrivate::updateVerticalLine() {
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
	if(q->m_title)
		q->m_title->setVisible(visible);
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
	if (connectionLinePen.style() != Qt::NoPen && q->m_title->isVisible()) {
		painter->setPen(connectionLinePen);
		painter->drawLine(connectionLine);
	}

	// draw vertical line, which connects all points together
	if (verticalLinePen.style() != Qt::NoPen) {
		painter->setPen(verticalLinePen);
		painter->drawLine(xposLine);
	}
}

QVariant InfoElementPrivate::itemChange(GraphicsItemChange change, const QVariant &value) {
	return QGraphicsItem::itemChange(change, value);
}

void InfoElementPrivate::mousePressEvent(QGraphicsSceneMouseEvent* event) {
	if (event->button() == Qt::MouseButton::LeftButton) {

		if (verticalLinePen.style() != Qt::NoPen) {
			const double width = verticalLinePen.widthF();
			if (abs(xposLine.x1()-event->pos().x())< ((width < 3)? 3 : width)) {
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
		double dx12 = connectionLine.x2() - connectionLine.x1();
		double dy12 = connectionLine.y2() - connectionLine.y1();
		double vecLenght = sqrt(pow(dx12,2) + pow(dy12,2));
		QPointF unitvec(dx12/vecLenght, dy12/vecLenght);

		double dx1m = event->pos().x() - connectionLine.x1();
		double dy1m = event->pos().y() - connectionLine.y1();

		double dist_segm = abs(dx1m*unitvec.y() - dy1m*unitvec.x());
		double scalar_product = dx1m*unitvec.x()+dy1m*unitvec.y();
		DEBUG("DIST_SEGMENT   " << dist_segm << "SCALAR_PRODUCT: " << scalar_product << "VEC_LENGTH: " << vecLenght);

		if (scalar_product > 0) {
			const double width = connectionLinePen.widthF();
			if (scalar_product < vecLenght && dist_segm < ((width < 3) ? 3 : width)) {
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

	if (!q->m_title)
		return;
	if (q->markerpoints.isEmpty())
		return;

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
	int xindex = q->markerpoints[activeIndex].curve->xColumn()->indexForValue(x);
	double x_new = q->markerpoints[activeIndex].curve->xColumn()->valueAt(xindex);
	if (abs(x_new - q->markerpoints[activeIndex].x) > 0) {
		oldMousePos = eventPos;
		q->setPosition(x);
	}
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

		double x;
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

		// find markerpoint to which the values matches (curvename is stored in connectionLineCurveName)
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

		q->setPosition(x);
//		xNew = x;
//		for (int i =0; i< q->markerpoints.length(); i++) {
//			q->markerpoints[i].x = x;
//			auto* curve = q->markerpoints[i].curve;
//			if (curve->xColumn()->rowCount() == rowCount) { // if the other columns have the same length it can simply used the index
//				q->markerpoints[i].y = curve->yColumn()->valueAt(m_index);
//				valueFound = true;
//			} else // if the length of the columns of the other curves are different, the y value must be searched
//				q->markerpoints[i].y = curve->y(x, xNew, valueFound);
//			if (valueFound) { // new set by curve->y()
//				pointPosition.setX(xNew);
//				pointPosition.setY(q->markerpoints[i].y);
//				DEBUG("X_old: " << q->markerpoints[i].customPoint->position().x() << "X_new: " << x);
//				q->m_suppressChildPositionChanged = true;
//				q->markerpoints[i].customPoint->setPosition(pointPosition);
//				q->m_suppressChildPositionChanged = false;
//			}
//		}
//		q->m_title->setText(q->createTextLabelText());
//		retransform();

	}
}

//##############################################################################
//##################  Serialization/Deserialization  ###########################
//##############################################################################
void InfoElement::save(QXmlStreamWriter* writer) const {
	Q_D(const InfoElement);

	writer->writeStartElement("infoElement");
	writeBasicAttributes(writer);
	writeCommentElement(writer);

	//general
	writer->writeStartElement("general");
	writer->writeAttribute("position", QString::number(d->position));
	writer->writeAttribute("curve", d->connectionLineCurveName);
	writer->writeAttribute("gluePointIndex", QString::number(d->gluePointIndex));
	writer->writeAttribute("markerIndex", QString::number(d->m_index));
	writer->writeAttribute("visible", QString::number(d->visible));
	writer->writeEndElement();

	//lines
	writer->writeStartElement("verticalLine");
	WRITE_QPEN(d->verticalLinePen);
	writer->writeAttribute("opacity", QString::number(d->verticalLineOpacity));
	writer->writeEndElement();

	writer->writeStartElement("connectionLine");
	WRITE_QPEN(d->connectionLinePen);
	writer->writeAttribute("opacity", QString::number(d->connectionLineOpacity));
	writer->writeEndElement();

	//text label
	m_title->save(writer);

	//custom points
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
		} else if (reader->name() == "general") {
			attribs = reader->attributes();

			str = attribs.value("visible").toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.subs("x").toString());
			else
				setVisible(str.toInt());

			READ_DOUBLE_VALUE("position", position);
			READ_INT_VALUE("gluePointIndex", gluePointIndex, int);
			READ_INT_VALUE("markerIndex", m_index, int);
			READ_STRING_VALUE("curve", connectionLineCurveName);
		} else if (reader->name() == "verticalLine") {
			attribs = reader->attributes();
			READ_QPEN(d->verticalLinePen);
			READ_DOUBLE_VALUE("opacity", verticalLineOpacity);
		} else if (reader->name() == "connectionLine") {
			attribs = reader->attributes();
			READ_QPEN(d->connectionLinePen);
			READ_DOUBLE_VALUE("opacity", connectionLineOpacity);
		} else if (reader->name() == "textLabel") {
			if (!m_title) {
				m_title = new TextLabel(i18n("Label"), d->plot);
				this->addChild(m_title);
			}
			if (!m_title->load(reader, preview))
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

	QPen p;
	p.setStyle((Qt::PenStyle)group.readEntry("LineStyle", (int)Qt::SolidLine));
	p.setWidthF(group.readEntry("LineWidth", Worksheet::convertToSceneUnits(1.0, Worksheet::Unit::Point)));
	p.setColor(group.readEntry("LineColor", QColor(Qt::black)));

	this->setVerticalLinePen(p);
	this->setVerticalLineOpacity(group.readEntry("LineOpacity", 1.0));

	this->setConnectionLinePen(p);
	this->setConnectionLineOpacity(group.readEntry("LineOpacity", 1.0));

	//load the theme for all the children
	for (auto* child : children<WorksheetElement>(ChildIndexFlag::IncludeHidden))
		child->loadThemeConfig(config);
}
