/*
	File                 : InfoElement.cpp
	Project              : LabPlot
	Description          : Marker which can highlight points of curves and
						   show their values
	--------------------------------------------------------------------
    SPDX-FileCopyrightText: 2020 Martin Marmsoler <martin.marmsoler@gmail.com>
    SPDX-FileCopyrightText: 2020 Alexander Semke <alexander.semke@web.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "InfoElement.h"

#include "backend/core/AbstractColumn.h"
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
	WorksheetElement(name, new InfoElementPrivate(this), AspectType::InfoElement)
{
	m_plot = plot;

	init();
	setVisible(false);
}

InfoElement::InfoElement(const QString& name, CartesianPlot* p, const XYCurve* curve, double pos):
	WorksheetElement(name, new InfoElementPrivate(this, curve), AspectType::InfoElement)
{
	Q_D(InfoElement);
	m_plot = p;

	init();

	m_suppressChildPositionChanged = true;

	if (curve) {
		d->connectionLineCurveName = curve->name();
		auto* custompoint = new CustomPoint(m_plot, curve->name());
		custompoint->setFixed(true);
		custompoint->setCoordinateBindingEnabled(true);
		addChild(custompoint);
		InfoElement::MarkerPoints_T markerpoint(custompoint, custompoint->path(), curve, curve->path());
		markerpoints.append(markerpoint);

		// setpos after label was created
		if (curve->xColumn() && curve->yColumn()) {
			bool valueFound;
			double xpos;
			double y = curve->y(pos,xpos,valueFound);
			if (valueFound) {
				d->positionLogical = xpos;
				d->m_index = curve->xColumn()->indexForValue(xpos);
				custompoint->setPositionLogical(QPointF(xpos,y));
				DEBUG("Value found")
			}
		} else {
			d->positionLogical = 0;
			custompoint->setPositionLogical(cSystem->mapSceneToLogical(QPointF(0, 0)));
			DEBUG("Value not found")
		}

		TextLabel::TextWrapper text;
		text.allowPlaceholder = true;

		QString textString;
		textString = QString::number(markerpoints[0].customPoint->positionLogical().x()) + ", ";
		textString.append(QString(QString(markerpoints[0].curve->name()+":")));
		textString.append(QString::number(markerpoints[0].customPoint->positionLogical().y()));
		text.text = textString;

		// TODO: Find better solution than using textedit
		QString str = QLatin1String("&(x), ") + markerpoints[0].curve->name()
								+ QLatin1Char(':') + QLatin1String("&(") + markerpoints[0].curve->name() + QLatin1Char(')');
		QTextEdit textedit(str);
		text.textPlaceholder = textedit.toHtml();

		m_setTextLabelText = true;
		m_title->setUndoAware(false);
		m_title->setText(text);
		m_title->setUndoAware(true);
		m_setTextLabelText = false;

		connect(curve, QOverload<bool>::of(&XYCurve::visibilityChanged), this, &InfoElement::curveVisibilityChanged);
		custompoint->setVisible(curve->isVisible());

		setVisible(true);
	} else
		setVisible(false);

	m_suppressChildPositionChanged = false;
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
	cSystem = dynamic_cast<const CartesianCoordinateSystem*>(m_plot->coordinateSystem(m_cSystemIndex));

	initActions();
	initMenus();

	connect(this, &InfoElement::aspectRemoved, this, &InfoElement::childRemoved);
	connect(this, &InfoElement::aspectAdded, this, &InfoElement::childAdded);

	m_title = new TextLabel(i18n("Label"), m_plot);
	m_title->setHidden(true);
	TextLabel::TextWrapper text;
	text.allowPlaceholder = true;
	m_setTextLabelText = true;
	m_title->setUndoAware(false);
	m_title->setText(text);  // set placeholder to true
	m_title->setUndoAware(true);
	m_setTextLabelText = false;
	addChild(m_title);

	//use the color for the axis line from the theme also for info element's lines
	KConfig config;
	InfoElement::loadThemeConfig(config);
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

	for (auto& markerpoint : markerpoints) {
		if (curve == markerpoint.curve)
			return;
	}

	project()->setSuppressAspectAddedSignal(true);

	if (!custompoint) {
		m_suppressChildPositionChanged = true;
		custompoint = new CustomPoint(m_plot, curve->name());
		custompoint->setCoordinateBindingEnabled(true);
		addChild(custompoint);

		if (curve->xColumn() && curve->yColumn()) {
			bool valueFound;
			double x_new, y;
			y = curve->y(d->positionLogical, x_new, valueFound);

			custompoint->setUndoAware(false);
			custompoint->setPositionLogical(QPointF(x_new, y));
			custompoint->setUndoAware(true);
		}
		m_suppressChildPositionChanged = false;
	} else
		addChild(custompoint);

	project()->setSuppressAspectAddedSignal(true);

	connect(curve, QOverload<bool>::of(&XYCurve::visibilityChanged), this, &InfoElement::curveVisibilityChanged);
	connect(curve, &XYCurve::moveBegin, this, [this]() { m_curveGetsMoved = true; });
	connect(curve, &XYCurve::moveEnd, this, [this]() { m_curveGetsMoved = false; });

	custompoint->setUndoAware(false);
	custompoint->setVisible(curve->isVisible());
	custompoint->setUndoAware(true);

	if (d->m_index < 0 && curve->xColumn())
		d->m_index = curve->xColumn()->indexForValue(custompoint->positionLogical().x());

	struct MarkerPoints_T markerpoint = {custompoint, custompoint->path(), curve, curve->path()};
	markerpoints.append(markerpoint);

	m_title->setUndoAware(false);
	m_title->setText(createTextLabelText());

	if (markerpoints.length() == 1) {
		// Do a retransform, because when the first markerpoint
		// was added, after a curve was removed and added, the
		// position of the connection line must be recalculated
		retransform();
	}

	m_title->setVisible(true); //show in the worksheet view
	m_title->setUndoAware(true);
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
	for(auto& markerpoint : markerpoints) {
		if(curvePath == markerpoint.curvePath)
			return;
	}

	if (!custompoint) {
		custompoint = new CustomPoint(m_plot, i18n("Symbol"));
		custompoint->setVisible(false);
		m_suppressChildPositionChanged = true;
		custompoint->setCoordinateBindingEnabled(true);
		m_suppressChildPositionChanged = false;
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
	for (auto& mp : markerpoints) {
		for (auto curve : curves) {
			if(mp.curvePath == curve->path()) {
				mp.curve = curve;
				connect(curve, QOverload<bool>::of(&XYCurve::visibilityChanged), this, &InfoElement::curveVisibilityChanged);
				mp.customPoint->setVisible(curve->isVisible()); // initial visibility
				break;
			}
		}
	}

	// check if all markerpoints have a valid curve
	// otherwise delete customPoint with no valid curve
	for (int i = markerpoints.count() - 1; i >= 0; i--) {
		if (markerpoints[i].curve == nullptr) {
			removeChild(markerpoints[i].customPoint);
			success = false;
		}
	}

	return success;
}

/*!
 * Remove markerpoint from a curve
 * @param curve
 */
void InfoElement::removeCurve(const XYCurve* curve) {
	if (m_curveGetsMoved)
		return;

	for (const auto& mp : markerpoints) {
		if (mp.curve == curve) {
			disconnect(curve, QOverload<bool>::of(&XYCurve::visibilityChanged), this, &InfoElement::curveVisibilityChanged);
			removeChild(mp.customPoint);
		}
	}

	m_title->setUndoAware(false);
	m_title->setText(createTextLabelText());

	//hide the label if now curves are selected
	if (markerpoints.isEmpty()) {
		m_title->setUndoAware(false);
		m_title->setVisible(false); //hide in the worksheet view
		m_title->setUndoAware(true);
		Q_D(InfoElement);
		d->update(); //redraw to remove all children graphic items belonging to InfoElement
	}

	m_title->setUndoAware(true);
}

/*!
 * Set the z value of the m_title and the custompoints higher than the infoelement
 * @param value
 */
void InfoElement::setZValue(qreal value) {
	graphicsItem()->setZValue(value);

	m_title->setZValue(value + 1);

	for (auto& markerpoint : markerpoints)
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
		DEBUG(STDSTRING(wrapper.text))
		DEBUG(STDSTRING(wrapper.textPlaceholder))
		wrapper.text = wrapper.textPlaceholder;
		return wrapper;
	}

	if (!(markerpoints.at(0).curve && markerpoints.at(0).curve->xColumn()))
		return wrapper; //no data is set in the curve yet, nothing to do

	Q_D(const InfoElement);

	QString text = wrapper.textPlaceholder;

	//replace the placeholder for the x-value
	QString xValueStr;
	auto columnMode = markerpoints.at(0).curve->xColumn()->columnMode();
	if (columnMode== AbstractColumn::ColumnMode::Double
		|| columnMode == AbstractColumn::ColumnMode::Integer
		|| columnMode == AbstractColumn::ColumnMode::BigInt)
		xValueStr = QString::number(d->positionLogical);
	else if (columnMode== AbstractColumn::ColumnMode::Day ||
			columnMode == AbstractColumn::ColumnMode::Month ||
			columnMode == AbstractColumn::ColumnMode::DateTime) {
		const auto& dateTime = QDateTime::fromMSecsSinceEpoch(d->positionLogical);
		xValueStr = dateTime.toString(m_plot->xRangeDateTimeFormat());
	}

	if (wrapper.mode == TextLabel::Mode::Text)
		text.replace("&amp;(x)", xValueStr);
	else
		text.replace("&(x)", xValueStr);

	//replace the placeholders for curve's y-values
	for (const auto& markerpoint : qAsConst(markerpoints)) {
		QString replace;
		if (wrapper.mode == TextLabel::Mode::Text)
			replace = QLatin1String("&amp;(");
		else
			replace = QLatin1String("&(");

		replace += markerpoint.curve->name() + QLatin1Char(')');
		text.replace(replace, QString::number(markerpoint.customPoint->positionLogical().y()));
	}

	wrapper.text = text;
	return wrapper;
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
void InfoElement::labelPositionChanged(TextLabel::PositionWrapper /*position*/) {
	if (m_suppressChildPositionChanged)
		return;

	Q_D(InfoElement);
	d->retransform();
}

void InfoElement::labelVisibleChanged(bool /*visible*/) {
	Q_D(InfoElement);
	d->update();
}

void InfoElement::labelTextWrapperChanged(TextLabel::TextWrapper) {
	if (m_setTextLabelText)
		return;

	m_setTextLabelText = true;
	m_title->setUndoAware(false);
	m_title->setText(createTextLabelText());
	m_title->setUndoAware(true);
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
	for (auto& custompoint: markerpoints) {
		if (custompoint.curve == curve)
			custompoint.customPoint->setVisible(visible);

		if (custompoint.customPoint->isVisible())
			oneMarkerpointVisible = true;
	}

	// if curve was set to hidden, set InfoElement to first visible curve
	if (!visible) {
		for (auto& custompoint : markerpoints) {
			if (custompoint.curve->isVisible()) {
				setConnectionLineCurveName(custompoint.curve->name());
				break;
			}
		}
	}

	// if no markerpoints are visible, hide the title label
	m_title->setUndoAware(false);
	if ((!visible && markerpoints.count() == 0) || !oneMarkerpointVisible)
		m_title->setVisible(false);
	else
		m_title->setVisible(true);
	m_title->setUndoAware(true);
}

void InfoElement::labelBorderShapeChanged() {
	Q_D(InfoElement);
	Q_EMIT labelBorderShapeChangedSignal();
	d->retransform();
}

/*!
 * Delete child and remove from markerpoint list if it is a markerpoint. If it is a textlabel delete complete InfoElement
 */
void InfoElement::childRemoved(const AbstractAspect* parent, const AbstractAspect* /*before*/, const AbstractAspect* child) {
	Q_D(InfoElement);

	// when childs are reordered, don't remove them
	// problem: when the order was changed the elements are deleted for a short time and recreated. This function will called then
	if (m_suppressChildRemoved)
		return;

	if (parent != this) // why do I need this?
		return;

	// point removed
	const auto* point = dynamic_cast<const CustomPoint*>(child);
	if (point) {
		for (int i = 0; i < markerpoints.length(); i++) {
			if (point == markerpoints[i].customPoint)
				markerpoints.removeAt(i);
			// no point->remove() needed, because it was already deleted
		}
		// recreate text, because when marker was deleted,
		// the placeholder should not be replaced anymore by a value
		m_title->setUndoAware(false);
		m_title->setText(createTextLabelText());
		m_title->setUndoAware(true);
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
		auto* p = const_cast<CustomPoint*>(point);
		// otherwise Custom point must be patched to handle discrete curve points.
		// This makes it much easier
		p->graphicsItem()->setFlag(QGraphicsItem::ItemIsMovable, false);
		p->setParentGraphicsItem(graphicsItem());
		// Must be done after setCoordinateBindingEnabled, otherwise positionChanged will be called and
		// then the InfoElement position will be set incorrectly
		connect(point, &CustomPoint::positionChanged, this, &InfoElement::pointPositionChanged);
		connect(point, &CustomPoint::moveBegin, this, &InfoElement::moveElementBegin);
		connect(point, &CustomPoint::moveEnd, this, &InfoElement::moveElementEnd);
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
	for (auto& markerpoint : markerpoints) {
		if (markerpoint.curve->name() == connectionLineCurveName()) {
			int index = markerpoint.curve->xColumn()->indexForValue(x);

			if (found_x && index >= 0) {
				auto mode = markerpoint.curve->xColumn()->columnMode();
				switch (mode) {
				case AbstractColumn::ColumnMode::Double:
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
	for (int i = 0; i < markerpoints.length(); i++) {
		bool valueFound;
		double y = markerpoints[i].curve->y(x,x_new, valueFound);
		d->positionLogical = x_new;
		if (i == 0)
			x_new_first = x_new;

		if (valueFound) {
			m_suppressChildPositionChanged = true;
			auto* point = markerpoints[i].customPoint;
			point->graphicsItem()->setFlag(QGraphicsItem::ItemSendsGeometryChanges, false);
			point->setUndoAware(false);
			point->setPositionLogical(QPointF(x_new,y));
			point->setUndoAware(false);
			point->graphicsItem()->setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);
			m_suppressChildPositionChanged = false;
		}
	}
	return x_new_first;
}

/*!
 * Will be called, when the customPoint changes his position
 * @param pos
 */
void InfoElement::pointPositionChanged(const WorksheetElement::PositionWrapper&) {
	if (m_suppressChildPositionChanged)
		return;

	CustomPoint* point = dynamic_cast<CustomPoint*>(QObject::sender());
	if (point == nullptr)
		return;

	setPositionLogical(point->positionLogical().x());
}

void InfoElement::setParentGraphicsItem(QGraphicsItem* item) {
	Q_D(InfoElement);
	d->setParentItem(item);
	d->updatePosition();
}

QGraphicsItem* InfoElement::graphicsItem() const {
	return d_ptr;
}

void InfoElement::retransform() {
	Q_D(InfoElement);
	d->retransform();
}

void InfoElement::handleResize(double /*horizontalRatio*/, double /*verticalRatio*/, bool /*pageResize*/) {
}

//##############################################################################
//######  Getter and setter methods ############################################
//##############################################################################

/* ============================ getter methods ================= */
BASIC_SHARED_D_READER_IMPL(InfoElement, double, positionLogical, positionLogical)
BASIC_SHARED_D_READER_IMPL(InfoElement, int, gluePointIndex, gluePointIndex)
BASIC_SHARED_D_READER_IMPL(InfoElement, QString, connectionLineCurveName, connectionLineCurveName)
BASIC_SHARED_D_READER_IMPL(InfoElement, QPen, verticalLinePen, verticalLinePen)
BASIC_SHARED_D_READER_IMPL(InfoElement, qreal, verticalLineOpacity, verticalLineOpacity)
BASIC_SHARED_D_READER_IMPL(InfoElement, QPen, connectionLinePen, connectionLinePen)
BASIC_SHARED_D_READER_IMPL(InfoElement, qreal, connectionLineOpacity, connectionLineOpacity)

/* ============================ setter methods ================= */
STD_SETTER_CMD_IMPL(InfoElement, SetPositionLogical, double, positionLogical)
void InfoElement::setPositionLogical(double pos) {
	Q_D(InfoElement);
	double value = 0.;
	int index = currentIndex(pos, &value);
	if (index < 0)
		return;

	if (value != d->positionLogical) {
		d->m_index = index;
		exec(new InfoElementSetPositionLogicalCmd(d, pos, ki18n("%1: set position")));
		setMarkerpointPosition(value);
		m_setTextLabelText = true;
		m_title->setUndoAware(false);
		m_title->setText(createTextLabelText());
		m_title->setUndoAware(true);
		m_setTextLabelText = false;
		retransform();
		positionLogicalChanged(d->positionLogical);
	}
}

STD_SETTER_CMD_IMPL_F_S(InfoElement, SetGluePointIndex, int, gluePointIndex, retransform)
void InfoElement::setGluePointIndex(int value) {
	Q_D(InfoElement);
	if (value != d->gluePointIndex)
		exec(new InfoElementSetGluePointIndexCmd(d, value, ki18n("%1: set gluepoint index")));
}

STD_SETTER_CMD_IMPL_F_S(InfoElement, SetConnectionLineCurveName, QString, connectionLineCurveName, retransform)
void InfoElement::setConnectionLineCurveName(const QString& name) {
	Q_D(InfoElement);
	if (name.compare(d->connectionLineCurveName) != 0)
		exec(new InfoElementSetConnectionLineCurveNameCmd(d, name, ki18n("%1: set connectionline curve name")));
}

STD_SETTER_CMD_IMPL_F_S(InfoElement, SetVisible, bool, visible, visibilityChanged)
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

STD_SETTER_CMD_IMPL_F_S(InfoElement, SetVerticalLineOpacity, qreal, verticalLineOpacity, update)
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

STD_SETTER_CMD_IMPL_F_S(InfoElement, SetConnectionLineOpacity, qreal, connectionLineOpacity, update)
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

InfoElementPrivate::InfoElementPrivate(InfoElement* owner):
	WorksheetElementPrivate(owner), q(owner) {
	init();
}

InfoElementPrivate::InfoElementPrivate(InfoElement* owner, const XYCurve*):
	WorksheetElementPrivate(owner), q(owner) {
	init();
}

void InfoElementPrivate::init() {
	setFlag(QGraphicsItem::ItemIsMovable, false);
	setFlag(QGraphicsItem::ItemClipsChildrenToShape, false);
	setFlag(QGraphicsItem::ItemIsSelectable, true);
	setFlag(QGraphicsItem::ItemIsFocusable, true);

	setFlag(QGraphicsItem::ItemSendsGeometryChanges, false);
	setPos(QPointF(0,0));
	setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);
}

/*!
	calculates the position and the bounding box of the InfoElement. Called on geometry changes.
	Or when the label or the point where moved
 */
void InfoElementPrivate::retransform() {
	DEBUG(Q_FUNC_INFO)
	if (!q->m_title || q->markerpoints.isEmpty() || q->isLoading())
		return;

	q->m_suppressChildPositionChanged = true;

	//new bounding rectangle
	const QRectF& rect = parentItem()->mapRectFromScene(q->plot()->rect());
	boundingRectangle = mapFromParent(rect).boundingRect();
	QDEBUG(Q_FUNC_INFO << ", rect = " << rect)
	QDEBUG(Q_FUNC_INFO << ", bounding rect = " << boundingRectangle)

	// TODO: why do I need to retransform the label and the custompoints?
	q->m_title->retransform();
	for (auto& markerpoint : q->markerpoints)
		markerpoint.customPoint->retransform();

	//determine the position to connect the line to
	QPointF pointPos;
	for (int i = 0; i < q->markerPointsCount(); ++i) {
		const auto* curve = q->markerpoints.at(i).curve;
		if (curve && curve->name() == connectionLineCurveName) {
			bool visible;
			pointPos = q->cSystem->mapLogicalToScene(q->markerpoints.at(i).customPoint->positionLogical(),
					visible, AbstractCoordinateSystem::MappingFlag::SuppressPageClipping);
			break;
		}
	}

	// use limit function like in the cursor! So the line will be drawn only till the border of the cartesian Plot
	QPointF m_titlePos;
	if (gluePointIndex < 0)
		m_titlePos = q->m_title->findNearestGluePoint(pointPos);
	else
		m_titlePos = q->m_title->gluePointAt(gluePointIndex).point;

	//connection line
	const QPointF m_titlePosItemCoords = mapFromParent(m_titlePos); // calculate item coords from scene coords
	const QPointF pointPosItemCoords = mapFromParent(mapPlotAreaToParent(pointPos)); // calculate item coords from scene coords
	if (boundingRectangle.contains(m_titlePosItemCoords) && boundingRectangle.contains(pointPosItemCoords))
		connectionLine = QLineF(m_titlePosItemCoords.x(), m_titlePosItemCoords.y(),
					pointPosItemCoords.x(), pointPosItemCoords.y());
	else
		connectionLine = QLineF();
	QDEBUG(Q_FUNC_INFO << ", connection line = " << connectionLine)

	//vertical line
	const QRectF& dataRect = mapFromParent(q->m_plot->dataRect()).boundingRect();
	xposLine = QLineF(pointPosItemCoords.x(), dataRect.bottom(), pointPosItemCoords.x(), dataRect.top());
	QDEBUG(Q_FUNC_INFO << ", vertical line " << xposLine)

	update(boundingRectangle);

	q->m_suppressChildPositionChanged = false;
}

void InfoElementPrivate::updatePosition() {
	//TODO?
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
	for(auto& markerpoint : q->markerpoints)
		markerpoint.customPoint->setVisible(visible);
	if(q->m_title) {
		q->m_title->setUndoAware(false);
		q->m_title->setVisible(visible);
		q->m_title->setUndoAware(true);
	}
	update(boundingRect());
}

//reimplemented from QGraphicsItem
QRectF InfoElementPrivate::boundingRect() const {
	return boundingRectangle;
}

void InfoElementPrivate::paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*) {
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
		//DEBUG("DIST_SEGMENT   " << dist_segm << "SCALAR_PRODUCT: " << scalar_product << "VEC_LENGTH: " << vecLenght);

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
// 	DEBUG("EventPos: " << eventPos.x() << " Y: " << eventPos.y());
	QPointF delta = eventPos - oldMousePos;
	if (delta == QPointF(0,0))
		return;

	QPointF eventLogicPos = q->cSystem->mapSceneToLogical(eventPos, AbstractCoordinateSystem::MappingFlag::SuppressPageClipping);
	QPointF delta_logic =  eventLogicPos - q->cSystem->mapSceneToLogical(oldMousePos);

	if (!q->m_title)
		return;
	if (q->markerpoints.isEmpty())
		return;

	// TODO: find better method to do this. It's inefficient.
	// Finding which curve should be used to find the new values
	double x = positionLogical;
	int activeIndex = 0;
	for (int i = 1; i < q->markerPointsCount(); i++) {
		if (q->markerpoints[i].curve->name().compare(connectionLineCurveName) == 0) {
			// not possible to use index, because when the number of elements in the columns of the curves are not the same there is a problem
			x = q->markerpoints[i].customPoint->positionLogical().x(); //q->markerpoints[i].curve->xColumn()->valueAt(m_index)
			activeIndex = i;
			break;
		}
	}
	x += delta_logic.x();
	auto xColumn = q->markerpoints[activeIndex].curve->xColumn();
	int xindex = xColumn->indexForValue(x);
	double x_new = xColumn->valueAt(xindex);
	auto pointPos = q->markerpoints[activeIndex].customPoint->positionLogical().x();
	if (abs(x_new - pointPos) > 0) {
		if ((xColumn->rowCount() - 1 == xindex && pointPos > x_new) || (xindex == 0 && pointPos < x_new))
		{
			q->setPositionLogical(x_new);
		} else {
			oldMousePos = eventPos;
			q->setPositionLogical(x);
		}
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

		// problem: when curves have different number of samples, the points are anymore aligned
		// with the vertical line
		// find markerpoint to which the values matches (curvename is stored in connectionLineCurveName)
		for (int i = 0; i < q->markerPointsCount(); i++) {
			if (q->markerpoints[i].curve->name().compare(connectionLineCurveName) == 0) {
				auto rowCount = q->markerpoints[i].curve->xColumn()->rowCount();
				m_index += index;
				if (m_index > rowCount - 1)
					m_index = rowCount - 1;
				if (m_index < 0)
					m_index = 0;
				auto x = q->markerpoints[i].curve->xColumn()->valueAt(m_index);
				q->setPositionLogical(x);
				break;
			}
		}
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
	writer->writeAttribute("position", QString::number(d->positionLogical));
	writer->writeAttribute("curve", d->connectionLineCurveName);
	writer->writeAttribute("gluePointIndex", QString::number(d->gluePointIndex));
	writer->writeAttribute("markerIndex", QString::number(d->m_index));
	writer->writeAttribute( "plotRangeIndex", QString::number(m_cSystemIndex) );
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
	if (!markerpoints.isEmpty()) {
		writer->writeStartElement("points");
		for (const auto& custompoint : markerpoints) {
			writer->writeStartElement("point");
			writer->writeAttribute(QLatin1String("curvepath"), custompoint.curve->path());
			custompoint.customPoint->save(writer);
			writer->writeEndElement(); //close "point"
		}
		writer->writeEndElement(); //clost "points"
	}

	writer->writeEndElement(); // close "infoElement"
}

bool InfoElement::load(XmlStreamReader* reader, bool preview) {
	if (!readBasicAttributes(reader))
		return false;

	Q_D(InfoElement);

	QXmlStreamAttributes attribs;
	KLocalizedString attributeWarning = ki18n("Attribute '%1' missing or empty, default value is used");
	QString str;
	QString curvePath;

	while (!reader->atEnd()) {
		reader->readNext();
		if (reader->isEndElement() && reader->name() == "infoElement")
			break;

		if (!reader->isStartElement())
			continue;

		if (!preview && reader->name() == "comment") {
			if (!readCommentElement(reader)) return false;
		} else if (reader->name() == "general") {
			attribs = reader->attributes();

			READ_INT_VALUE_DIRECT("plotRangeIndex", m_cSystemIndex, int);

			str = attribs.value("visible").toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.subs("x").toString());
			else
				setVisible(str.toInt());

			READ_DOUBLE_VALUE("position", positionLogical);
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
				m_title = new TextLabel(i18n("Label"), m_plot);
				m_title->setIsLoading(true);
				this->addChild(m_title);
			}
			if (!m_title->load(reader, preview))
				return false;
		} else if (reader->name() == "customPoint") {
			if (curvePath.isEmpty()) //safety check in case the xml is broken
				continue;

			auto* point = new CustomPoint(m_plot, QString());
			point->setIsLoading(true);
			if (!point->load(reader,preview)) {
				delete  point;
				return false;
			}
			this->addChild(point);
			addCurvePath(curvePath, point);
			curvePath.clear();
		} else if (reader->name() == "point") {
			attribs = reader->attributes();
			curvePath = attribs.value("curvepath").toString();
		}
	}

	return true;
}

//##############################################################################
//#########################  Theme management ##################################
//##############################################################################
void InfoElement::loadThemeConfig(const KConfig& config) {
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
	const auto& children = this->children<WorksheetElement>(ChildIndexFlag::IncludeHidden);
	for (auto* child : children)
		child->loadThemeConfig(config);
}
