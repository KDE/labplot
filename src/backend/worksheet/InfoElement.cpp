/*
	File                 : InfoElement.cpp
	Project              : LabPlot
	Description          : Marker which can highlight points of curves and
						   show their values
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2020 Martin Marmsoler <martin.marmsoler@gmail.com>
	SPDX-FileCopyrightText: 2020-2023 Alexander Semke <alexander.semke@web.de>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "InfoElement.h"

#include "backend/core/AbstractColumn.h"
#include "backend/core/Project.h"
#include "backend/lib/XmlStreamReader.h"
#include "backend/lib/commandtemplates.h"
#include "backend/worksheet/InfoElementPrivate.h"
#include "backend/worksheet/Line.h"
#include "backend/worksheet/Worksheet.h"
#include "backend/worksheet/plots/cartesian/CartesianCoordinateSystem.h"
#include "backend/worksheet/plots/cartesian/CartesianPlot.h"
#include "backend/worksheet/plots/cartesian/CustomPoint.h"
#include "backend/worksheet/plots/cartesian/XYCurve.h"

#include <KConfig>
#include <KConfigGroup>

#include <QAction>
#include <QDateTime>
#include <QGraphicsSceneMouseEvent>
#include <QKeyEvent>
#include <QMenu>
#include <QPainter>
#include <QTextEdit>
#include <qglobal.h>

InfoElement::MarkerPoints_T::MarkerPoints_T(CustomPoint* custompoint, const XYCurve* curve, QString curvePath)
	: customPoint(custompoint)
	, curve(curve)
	, curvePath(curvePath) {
	if (customPoint)
		visible = custompoint->isVisible();
}

/**
 * \class InfoElement
 * \brief Marker which can highlight points of curves and show their values.
 * \ingroup CartesianPlotArea
 */
InfoElement::InfoElement(const QString& name, CartesianPlot* plot)
	: WorksheetElement(name, new InfoElementPrivate(this), AspectType::InfoElement) {
	Q_D(InfoElement);
	d->m_plot = plot;

	init();
	setVisible(false);
}

InfoElement::InfoElement(const QString& name, CartesianPlot* p, const XYCurve* curve, double logicalPos)
	: WorksheetElement(name, new InfoElementPrivate(this, curve), AspectType::InfoElement) {
	Q_D(InfoElement);
	d->m_plot = p;

	init();

	m_suppressChildPositionChanged = true;

	if (curve) {
		d->connectionLineCurveName = curve->name();
		auto* custompoint = new CustomPoint(d->m_plot, curve->name());
		custompoint->setFixed(true);
		custompoint->setCoordinateBindingEnabled(true);
		custompoint->setCoordinateSystemIndex(curve->coordinateSystemIndex());
		addChild(custompoint);
		InfoElement::MarkerPoints_T markerpoint(custompoint, curve, curve->path());
		markerpoints.append(markerpoint);

		// setpos after label was created
		if (curve->xColumn() && curve->yColumn()) {
			bool valueFound;
			double xpos;
			double y = curve->y(logicalPos, xpos, valueFound);
			if (valueFound) {
				d->positionLogical = xpos;
				d->m_index = curve->xColumn()->indexForValue(xpos);
				custompoint->setPositionLogical(QPointF(xpos, y));
				DEBUG("Value found")
			}
		} else {
			d->positionLogical = 0;
			custompoint->setPositionLogical(cSystem->mapSceneToLogical(QPointF(0, 0)));
			DEBUG("Value not found")
		}

		TextLabel::TextWrapper text;
		text.allowPlaceholder = true;

		QString textString = QString::number(markerpoints[0].customPoint->positionLogical().x()) + QStringLiteral(", ");
		textString.append(QString(QString(markerpoints[0].curve->name() + QStringLiteral(":"))));
		textString.append(QString::number(markerpoints[0].customPoint->positionLogical().y()));
		text.text = textString;

		// TODO: Find better solution than using textedit
		QString str = QStringLiteral("&(x), ") + markerpoints[0].curve->name() + QLatin1Char(':') + QLatin1String("&(") + markerpoints[0].curve->name()
			+ QLatin1Char(')');
		QTextEdit textedit(str);
		text.textPlaceholder = textedit.toHtml();

		m_setTextLabelText = true;
		m_title->setUndoAware(false);
		m_title->setText(text);
		m_title->setUndoAware(true);
		m_setTextLabelText = false;

		initCurveConnections(curve);
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
	Q_D(InfoElement);
	cSystem = dynamic_cast<const CartesianCoordinateSystem*>(d->m_plot->coordinateSystem(m_cSystemIndex));

	initActions();
	initMenus();

	connect(this, &InfoElement::childAspectRemoved, this, &InfoElement::childRemoved);
	connect(this, &InfoElement::childAspectAdded, this, &InfoElement::childAdded);

	m_title = new TextLabel(i18n("Label"), d->m_plot);
	m_title->setHidden(true);
	TextLabel::TextWrapper text;
	text.allowPlaceholder = true;
	m_setTextLabelText = true;
	m_title->setUndoAware(false);
	m_title->setText(text); // set placeholder to true
	m_title->setUndoAware(true);
	m_setTextLabelText = false;
	addChild(m_title);

	// use the line properties of axis line also for the info element lines
	KConfig config;
	const auto& group = config.group(QStringLiteral("Axis"));

	// lines
	d->verticalLine = new Line(QString());
	d->verticalLine->setHidden(true);
	d->verticalLine->setPrefix(QStringLiteral("VerticalLine"));
	addChild(d->verticalLine);
	d->verticalLine->init(group);
	connect(d->verticalLine, &Line::updatePixmapRequested, [=] {
		d->update();
	});
	connect(d->verticalLine, &Line::updateRequested, [=] {
		d->updateVerticalLine();
	});

	d->connectionLine = new Line(QString());
	d->connectionLine->setHidden(true);
	d->connectionLine->setPrefix(QStringLiteral("ConnectionLine"));
	addChild(d->connectionLine);
	d->connectionLine->init(group);
	connect(d->connectionLine, &Line::updatePixmapRequested, [=] {
		d->update();
	});
	connect(d->connectionLine, &Line::updateRequested, [=] {
		d->updateConnectionLine();
	});
}

void InfoElement::initActions() {
}

void InfoElement::initMenus() {
	m_menusInitialized = true;
}

void InfoElement::initCurveConnections(const XYCurve* curve) {
	connect(curve, &XYCurve::visibleChanged, this, &InfoElement::curveVisibilityChanged);
	connect(curve, &XYCurve::coordinateSystemIndexChanged, this, &InfoElement::curveCoordinateSystemIndexChanged);
	connect(curve, &XYCurve::dataChanged, this, &InfoElement::curveDataChanged);
	connect(curve, &XYCurve::xDataChanged, this, &InfoElement::curveDataChanged);
	connect(curve, &XYCurve::yDataChanged, this, &InfoElement::curveDataChanged);
	connect(curve, &XYCurve::aspectAboutToBeRemoved, this, &InfoElement::curveDeleted);
}

void InfoElement::pointVisibleChanged(bool visible) {
	const auto* point = QObject::sender();

	if (m_suppressVisibleChange)
		return;
	for (auto& c : markerpoints) {
		if (c.customPoint == point)
			c.visible = visible;
	}
}

QMenu* InfoElement::createContextMenu() {
	if (!m_menusInitialized)
		initMenus();

	return WorksheetElement::createContextMenu();
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
		custompoint = new CustomPoint(d->m_plot, curve->name());
		custompoint->setCoordinateBindingEnabled(true);
		custompoint->setCoordinateSystemIndex(curve->coordinateSystemIndex());
		setUndoAware(false);
		addChild(custompoint);
		setUndoAware(true);

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

	initCurveConnections(curve);

	custompoint->setUndoAware(false);
	custompoint->setVisible(curve->isVisible());
	custompoint->setUndoAware(true);

	if (d->m_index < 0 && curve->xColumn())
		d->m_index = curve->xColumn()->indexForValue(custompoint->positionLogical().x());

	struct MarkerPoints_T markerpoint = {custompoint, curve, curve->path()};
	markerpoints.append(markerpoint);

	if (markerpoints.count() == 1) // first point
		setConnectionLineCurveName(curve->name());

	m_title->setUndoAware(false);
	m_title->setText(createTextLabelText());

	if (markerpoints.length() == 1) {
		// Do a retransform, because when the first markerpoint
		// was added, after a curve was removed and added, the
		// position of the connection line must be recalculated
		retransform();
	}

	m_title->setVisible(true); // show in the worksheet view
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
void InfoElement::addCurvePath(const QString& curvePath, CustomPoint* custompoint) {
	for (auto& markerpoint : markerpoints) {
		if (curvePath == markerpoint.curvePath)
			return;
	}

	Q_D(const InfoElement);
	if (!custompoint) {
		custompoint = new CustomPoint(d->m_plot, i18n("Symbol"));
		custompoint->setVisible(false);
		m_suppressChildPositionChanged = true;
		custompoint->setCoordinateBindingEnabled(true);
		m_suppressChildPositionChanged = false;
		addChild(custompoint);
	}

	struct MarkerPoints_T markerpoint = {custompoint, nullptr, curvePath};
	markerpoints.append(markerpoint);
}

/*!
 * \brief assignCurve
 * Finds the curve with the path stored in the markerpoints and assigns the pointer to markerpoints
 * @param curves
 * \return true if all markerpoints are assigned with a curve, false if one or more markerpoints don't have a curve assigned
 */
bool InfoElement::assignCurve(const QVector<XYCurve*>& curves) {
	bool success = true;
	for (auto& mp : markerpoints) {
		for (auto curve : curves) {
			if (mp.curvePath == curve->path()) {
				mp.curve = curve;
				initCurveConnections(curve);
				mp.customPoint->setCoordinateSystemIndex(curve->coordinateSystemIndex());
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

void InfoElement::curveDeleted(const AbstractAspect* aspect) {
	auto curve = dynamic_cast<const XYCurve*>(aspect);
	if (!curve)
		return;

	for (auto& mp : markerpoints) {
		if (mp.curve == curve) {
			disconnect(curve, nullptr, this, nullptr);
			// No remove of the custompoint

			Lock lock(m_suppressVisibleChange);
			assert(mp.curvePath == curve->path());
			mp.curve = nullptr;
			mp.customPoint->setVisible(false);
		}
	}

	updateValid();

	if (curve->name() == connectionLineCurveName())
		setConnectionLineNextValidCurve();
}

void InfoElement::setConnectionLineNextValidCurve() {
	for (const auto& mp : markerpoints) {
		if (mp.curve) {
			setConnectionLineCurveName(mp.curve->name());
			return;
		}
	}
	setConnectionLineCurveName(QLatin1String());
}

/*!
 * Remove markerpoint from a curve
 * @param curve
 */
void InfoElement::removeCurve(const XYCurve* curve) {
	for (const auto& mp : markerpoints) {
		if (mp.curve == curve) {
			disconnect(curve, nullptr, this, nullptr);
			setUndoAware(false);
			removeChild(mp.customPoint);
			setUndoAware(true);
		}
	}

	setUndoAware(false);
	if (curve->name() == connectionLineCurveName())
		setConnectionLineNextValidCurve();
	setUndoAware(true);

	m_title->setUndoAware(false);
	m_title->setText(createTextLabelText());

	// hide the label if now curves are selected
	if (markerpoints.isEmpty()) {
		m_title->setVisible(false); // hide in the worksheet view
		Q_D(InfoElement);
		d->update(); // redraw to remove all children graphic items belonging to InfoElement
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
		markerpoint.customPoint->setZValue(value + 1);
}

/*!
 * Returns the amount of markerpoints. Used in the InfoElementDock to fill listWidget.
 */
int InfoElement::markerPointsCount() const {
	return markerpoints.length();
}

TextLabel::GluePoint InfoElement::gluePoint(int index) const {
	return m_title->gluePointAt(index);
}

int InfoElement::gluePointsCount() const {
	return m_title->gluePointCount();
}

/*!
 * Returns the Markerpoint at index \p index. Used in the InfoElementDock to fill listWidget
 * @param index
 */
InfoElement::MarkerPoints_T InfoElement::markerPointAt(int index) const {
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
	auto wrapper = m_title->text();
	if (markerPointsCount() < 1) {
		DEBUG(STDSTRING(wrapper.text))
		DEBUG(STDSTRING(wrapper.textPlaceholder))
		wrapper.text = wrapper.textPlaceholder;
		return wrapper;
	}

	if (!(markerpoints.at(0).curve && markerpoints.at(0).curve->xColumn()))
		return wrapper; // no data is set in the curve yet, nothing to do

	Q_D(const InfoElement);

	// replace the placeholder for the x-value
	QString xValueStr;
	const auto columnMode = markerpoints.at(0).curve->xColumn()->columnMode();
	if (columnMode == AbstractColumn::ColumnMode::Double || columnMode == AbstractColumn::ColumnMode::Integer
		|| columnMode == AbstractColumn::ColumnMode::BigInt)
		xValueStr = QString::number(d->positionLogical);
	else if (columnMode == AbstractColumn::ColumnMode::Day || columnMode == AbstractColumn::ColumnMode::Month
			 || columnMode == AbstractColumn::ColumnMode::DateTime) {
		const auto& dateTime = QDateTime::fromMSecsSinceEpoch(d->positionLogical, QTimeZone::UTC);
		xValueStr = dateTime.toString(d->m_plot->rangeDateTimeFormat(Dimension::X));
	}

	QString text = wrapper.textPlaceholder;
	if (wrapper.mode == TextLabel::Mode::Text)
		text.replace(QStringLiteral("&amp;(x)"), xValueStr);
	else
		text.replace(QStringLiteral("&(x)"), xValueStr);

	// replace the placeholders for curve's y-values
	for (const auto& markerpoint : std::as_const(markerpoints)) {
		QString replace;
		if (wrapper.mode == TextLabel::Mode::Text)
			replace = QStringLiteral("&amp;(");
		else
			replace = QStringLiteral("&(");

		replace += markerpoint.curve->name() + QLatin1Char(')');
		text.replace(replace, QString::number(markerpoint.customPoint->positionLogical().y()));
	}

	wrapper.text = text;
	return wrapper;
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

void InfoElement::curveCoordinateSystemIndexChanged(int /*index*/) {
	auto* curve = static_cast<XYCurve*>(QObject::sender());
	auto cSystemIndex = curve->coordinateSystemIndex();

	for (auto& custompoint : markerpoints) {
		if (custompoint.curve == curve) {
			custompoint.customPoint->setCoordinateSystemIndex(cSystemIndex);
			break;
		}
	}

	retransform();
}

void InfoElement::curveVisibilityChanged() {
	XYCurve* curve = static_cast<XYCurve*>(QObject::sender());
	bool visible = curve->isVisible();

	bool oneMarkerpointVisible = false;
	for (auto& custompoint : markerpoints) {
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

void InfoElement::curveDataChanged() {
	Q_D(InfoElement);
	setMarkerpointPosition(d->positionLogical);
	m_setTextLabelText = true;
	m_title->setUndoAware(false);
	m_title->setText(createTextLabelText());
	m_title->setUndoAware(true);
	m_setTextLabelText = false;
	retransform();
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
	const auto* textlabel = dynamic_cast<const TextLabel*>(child);
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
	const auto* point = dynamic_cast<const CustomPoint*>(child);
	if (point) {
		auto* p = const_cast<CustomPoint*>(point);
		// otherwise Custom point must be patched to handle discrete curve points.
		// This makes it much easier
		p->graphicsItem()->setFlag(QGraphicsItem::ItemIsMovable, false);
		p->setParentGraphicsItem(graphicsItem());
		// Must be done after setCoordinateBindingEnabled, otherwise positionChanged will be called and
		// then the InfoElement position will be set incorrectly
		connect(point, &CustomPoint::positionChanged, this, &InfoElement::pointPositionChanged);
		connect(point, &CustomPoint::visibleChanged, this, &InfoElement::pointVisibleChanged);
		return;
	}

	const auto* m_titleChild = dynamic_cast<const TextLabel*>(child);
	if (m_titleChild) {
		connect(m_title, &TextLabel::positionChanged, this, &InfoElement::labelPositionChanged);
		connect(m_title, &TextLabel::visibleChanged, this, &InfoElement::labelVisibleChanged);
		connect(m_title, &TextLabel::textWrapperChanged, this, &InfoElement::labelTextWrapperChanged);
		connect(m_title, &TextLabel::borderShapeChanged, this, &InfoElement::labelBorderShapeChanged);
		connect(m_title, &TextLabel::rotationAngleChanged, this, &InfoElement::retransform);

		auto* l = const_cast<TextLabel*>(m_titleChild);
		l->setParentGraphicsItem(graphicsItem());
	}
}

/*!
 * \brief InfoElement::currentValue
 * Calculates the new x position from
 * \param new_x
 * \return
 */
int InfoElement::currentIndex(double x, double* found_x) const {
	if (!isValid())
		return -1;
	for (auto& markerpoint : markerpoints) {
		if (markerpoint.curve->name() == connectionLineCurveName()) {
			if (!markerpoint.curve->xColumn())
				return -1;
			const int index = markerpoint.curve->xColumn()->indexForValue(x);

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
	updateValid();
	double x_new;
	double x_new_first = 0;
	for (int i = 0; i < markerpoints.length(); i++) {
		auto* point = markerpoints[i].customPoint;
		bool valueFound;
		double y = markerpoints[i].curve->y(x, x_new, valueFound);
		m_suppressVisibleChange = true;
		if (y == std::nan("0")) {
			point->setVisible(false);
			m_title->setVisible(false);
		} else {
			point->setVisible(markerpoints[i].visible);
			m_title->setVisible(true);
		}
		m_suppressVisibleChange = false;
		d->positionLogical = x_new;
		if (i == 0)
			x_new_first = x_new;

		if (valueFound) {
			m_suppressChildPositionChanged = true;
			point->graphicsItem()->setFlag(QGraphicsItem::ItemSendsGeometryChanges, false);
			point->setUndoAware(false);
			point->setPositionLogical(QPointF(x_new, y));
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

	const auto* point = dynamic_cast<CustomPoint*>(QObject::sender());
	if (!point)
		return;

	setPositionLogical(point->positionLogical().x());
}

void InfoElement::setParentGraphicsItem(QGraphicsItem* item) {
	Q_D(InfoElement);
	d->setParentItem(item);
	d->updatePosition();
}

void InfoElement::retransform() {
	Q_D(InfoElement);
	d->retransform();
}

void InfoElement::handleResize(double /*horizontalRatio*/, double /*verticalRatio*/, bool /*pageResize*/) {
}

void InfoElement::handleAspectUpdated(const QString& path, const AbstractAspect* aspect) {
	const auto* curve = dynamic_cast<const XYCurve*>(aspect);
	if (!curve)
		return;

	// The curve name changed and now it matches the correct path
	// Add the curve again
	for (auto& p : markerpoints) {
		if (!p.curve && p.curvePath.compare(path) == 0) {
			p.curve = curve;
			updateValid();
			retransform();
			break;
		}
	}
}

// ##############################################################################
// ######  Getter and setter methods ############################################
// ##############################################################################

/* ============================ getter methods ================= */
BASIC_SHARED_D_READER_IMPL(InfoElement, double, positionLogical, positionLogical)
BASIC_SHARED_D_READER_IMPL(InfoElement, int, gluePointIndex, gluePointIndex)
BASIC_SHARED_D_READER_IMPL(InfoElement, QString, connectionLineCurveName, connectionLineCurveName)

Line* InfoElement::verticalLine() const {
	Q_D(const InfoElement);
	return d->verticalLine;
}

Line* InfoElement::connectionLine() const {
	Q_D(const InfoElement);
	return d->connectionLine;
}

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

STD_SWAP_METHOD_SETTER_CMD_IMPL(InfoElement, SetVisible, bool, changeVisibility)
void InfoElement::setVisible(bool on) {
	Q_D(InfoElement);
	if (on != isVisible())
		exec(new InfoElementSetVisibleCmd(d, on, on ? ki18n("%1: set visible") : ki18n("%1: set invisible")));
}

bool InfoElement::isValid() const {
	Q_D(const InfoElement);
	return d->valid;
}

void InfoElement::updateValid() {
	Q_D(InfoElement);
	bool valid = false;
	for (const auto& mp : markerpoints) {
		if (mp.curve && mp.curve->xColumn() && mp.curve->yColumn())
			valid = true; // at least one valid curve
	}

	d->valid = valid;

	Lock lock(m_suppressVisibleChange);
	m_title->setUndoAware(false);
	m_title->setVisible(valid);
	m_title->setUndoAware(true);

	if (valid) {
		for (auto& mp : markerpoints) {
			if (mp.curve && mp.curve->xColumn() && mp.curve->yColumn()) {
				mp.customPoint->setUndoAware(false);
				mp.customPoint->setVisible(mp.visible);
				mp.customPoint->setUndoAware(true);
			}
		}
	} else {
		for (auto& mp : markerpoints) {
			mp.customPoint->setUndoAware(false);
			mp.customPoint->setVisible(false);
			mp.customPoint->setUndoAware(true);
		}
	}
}

// ##############################################################################
// ######  SLOTs for changes triggered via QActions in the context menu  ########
// ##############################################################################

// ##############################################################################
// ####################### Private implementation ###############################
// ##############################################################################

InfoElementPrivate::InfoElementPrivate(InfoElement* owner)
	: WorksheetElementPrivate(owner)
	, q(owner) {
	init();
}

InfoElementPrivate::InfoElementPrivate(InfoElement* owner, const XYCurve*)
	: WorksheetElementPrivate(owner)
	, q(owner) {
	init();
}

void InfoElementPrivate::init() {
	setFlag(QGraphicsItem::ItemIsMovable, false);
	setFlag(QGraphicsItem::ItemClipsChildrenToShape, false);
	setFlag(QGraphicsItem::ItemIsSelectable, true);
	setFlag(QGraphicsItem::ItemIsFocusable, true);

	setFlag(QGraphicsItem::ItemSendsGeometryChanges, false);
	setPos(QPointF(0, 0));
	setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);
}

/*!
	calculates the position and the bounding box of the InfoElement. Called on geometry changes.
	Or when the label or the point where moved
 */
void InfoElementPrivate::retransform() {
	DEBUG(Q_FUNC_INFO)
	q->updateValid();
	if (!q->m_title || q->markerpoints.isEmpty() || q->isLoading() || !parentItem() || !valid) {
		update(m_boundingRectangle);
		return;
	}

	Lock lock(q->m_suppressChildPositionChanged);
	xposLine = QLineF();
	m_connectionLine = QLineF();

	// new bounding rectangle
	const QRectF& rect = parentItem()->mapRectFromScene(q->plot()->rect());
	const QRectF& newBoundingRect = mapFromParent(rect).boundingRect();
	QDEBUG(Q_FUNC_INFO << ", rect = " << rect)
	QDEBUG(Q_FUNC_INFO << ", bounding rect = " << newBoundingRect)

	// TODO: why do I need to retransform the label and the custompoints?
	q->m_title->retransform();
	for (auto& markerpoint : q->markerpoints)
		markerpoint.customPoint->retransform();

	// determine the position to connect the line to
	QPointF pointPos;
	bool validPos = false;
	for (int i = 0; i < q->markerPointsCount(); ++i) {
		const auto* curve = q->markerpoints.at(i).curve;
		if (curve && curve->isVisible() && curve->name() == connectionLineCurveName) {
			const auto& point = q->markerpoints.at(i).customPoint;
			const auto* cSystem = q->plot()->coordinateSystem(point->coordinateSystemIndex());
			pointPos = cSystem->mapLogicalToScene(point->positionLogical(), insidePlot, AbstractCoordinateSystem::MappingFlag::SuppressPageClippingVisible);
			validPos = true;
			break;
		}
	}

	if (!validPos)
		return;

	if (!insidePlot)
		return;

	// use limit function like in the cursor! So the line will be drawn only till the border of the cartesian Plot
	QPointF m_titlePos;
	if (gluePointIndex < 0)
		m_titlePos = q->m_title->findNearestGluePoint(pointPos);
	else
		m_titlePos = q->m_title->gluePointAt(gluePointIndex).point;

	// connection line
	const QPointF m_titlePosItemCoords = mapFromParent(m_titlePos); // calculate item coords from scene coords
	const QPointF pointPosItemCoords = mapFromParent(mapPlotAreaToParent(pointPos)); // calculate item coords from scene coords
	if (newBoundingRect.contains(m_titlePosItemCoords) && newBoundingRect.contains(pointPosItemCoords))
		m_connectionLine = QLineF(m_titlePosItemCoords.x(), m_titlePosItemCoords.y(), pointPosItemCoords.x(), pointPosItemCoords.y());
	else
		m_connectionLine = QLineF();
	QDEBUG(Q_FUNC_INFO << ", connection line = " << m_connectionLine)

	// vertical line
	const QRectF& dataRect = mapFromParent(m_plot->dataRect()).boundingRect();
	xposLine = QLineF(pointPosItemCoords.x(), dataRect.bottom(), pointPosItemCoords.x(), dataRect.top());
	QDEBUG(Q_FUNC_INFO << ", vertical line " << xposLine)

	recalcShapeAndBoundingRect(newBoundingRect);
}

void InfoElementPrivate::updatePosition() {
	// TODO?
}

/*!
 * Repainting to update xposLine
 */
void InfoElementPrivate::updateVerticalLine() {
	recalcShapeAndBoundingRect();
}

/*!
 * Repainting to updateConnectionLine
 */
void InfoElementPrivate::updateConnectionLine() {
	recalcShapeAndBoundingRect();
}

bool InfoElementPrivate::changeVisibility(bool on) {
	bool oldValue = isVisible();
	setVisible(on);
	for (auto& markerpoint : q->markerpoints)
		markerpoint.customPoint->setVisible(on);
	if (q->m_title) {
		q->m_title->setUndoAware(false);
		q->m_title->setVisible(on);
		q->m_title->setUndoAware(true);
	}
	update(boundingRect());
	return oldValue;
}

void InfoElementPrivate::paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*) {
	if (!insidePlot || !valid)
		return;

	if (q->markerpoints.isEmpty())
		return;

	// do not draw connection line when the label is not visible
	if (connectionLine->style() != Qt::NoPen && q->m_title->isVisible() && !m_connectionLine.isNull()) {
		painter->setOpacity(connectionLine->opacity());
		painter->setPen(connectionLine->pen());
		painter->drawLine(m_connectionLine);
	}

	// draw vertical line, which connects all points together
	if (verticalLine->style() != Qt::NoPen && !xposLine.isNull()) {
		painter->setOpacity(verticalLine->opacity());
		painter->setPen(verticalLine->pen());
		painter->drawLine(xposLine);
	}
}

void InfoElementPrivate::mousePressEvent(QGraphicsSceneMouseEvent* event) {
	if (event->button() == Qt::MouseButton::LeftButton) {
		if (verticalLine->style() != Qt::NoPen) {
			const double width = verticalLine->pen().widthF();
			if (abs(xposLine.x1() - event->pos().x()) < ((width < 3) ? 3 : width)) {
				if (!isSelected())
					setSelected(true);
				m_suppressKeyPressEvents = false;
				oldMousePos = mapToParent(event->pos());
				event->accept();
				setFocus();
				return;
			}
		} /* else {
			 for (int i=0; i< q->markerPointsCount(); i++) {
				 InfoElement::MarkerPoints_T markerpoint =  q->markerPointAt(i);
				 //if (markerpoint.customPoint->symbolSize())
			 }
		 }*/

		// https://stackoverflow.com/questions/11604680/point-laying-near-line
		double dx12 = m_connectionLine.x2() - m_connectionLine.x1();
		double dy12 = m_connectionLine.y2() - m_connectionLine.y1();
		double vecLenght = sqrt(pow(dx12, 2) + pow(dy12, 2));
		QPointF unitvec(dx12 / vecLenght, dy12 / vecLenght);

		double dx1m = event->pos().x() - m_connectionLine.x1();
		double dy1m = event->pos().y() - m_connectionLine.y1();

		double dist_segm = abs(dx1m * unitvec.y() - dy1m * unitvec.x());
		double scalar_product = dx1m * unitvec.x() + dy1m * unitvec.y();
		// DEBUG("DIST_SEGMENT   " << dist_segm << "SCALAR_PRODUCT: " << scalar_product << "VEC_LENGTH: " << vecLenght);

		if (scalar_product > 0) {
			const double width = connectionLine->width();
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
	const auto eventPos = mapToParent(event->pos());
	// 	DEBUG("EventPos: " << eventPos.x() << " Y: " << eventPos.y());
	const auto delta = eventPos - oldMousePos;
	if (delta == QPointF(0, 0))
		return;

	if (!q->cSystem->isValid())
		return;
	const auto eventLogicPos = q->cSystem->mapSceneToLogical(eventPos, AbstractCoordinateSystem::MappingFlag::SuppressPageClipping);
	const auto delta_logic = eventLogicPos - q->cSystem->mapSceneToLogical(oldMousePos);

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
			x = q->markerpoints[i].customPoint->positionLogical().x(); // q->markerpoints[i].curve->xColumn()->valueAt(m_index)
			activeIndex = i;
			break;
		}
	}
	x += delta_logic.x();
	auto xColumn = q->markerpoints[activeIndex].curve->xColumn();
	if (!xColumn)
		return;
	int xindex = xColumn->indexForValue(x);
	double x_new = NAN;
	if (xColumn->isNumeric())
		x_new = xColumn->valueAt(xindex);
	else
		x_new = xColumn->dateTimeAt(xindex).toMSecsSinceEpoch();

	auto pointPos = q->markerpoints[activeIndex].customPoint->positionLogical().x();
	if (abs(x_new - pointPos) > 0) {
		if ((xColumn->rowCount() - 1 == xindex && pointPos > x_new) || (xindex == 0 && pointPos < x_new)) {
			q->setPositionLogical(x_new);
		} else {
			oldMousePos = eventPos;
			q->setPositionLogical(x);
		}
	}
}

void InfoElementPrivate::keyPressEvent(QKeyEvent* event) {
	if (m_suppressKeyPressEvents) {
		event->ignore();
		return QGraphicsItem::keyPressEvent(event);
	}

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
			const auto* curve = q->markerpoints[i].curve;
			if (curve->name().compare(connectionLineCurveName) == 0) {
				if (!curve->xColumn())
					return;
				auto rowCount = curve->xColumn()->rowCount();
				m_index += index;
				if (m_index > rowCount - 1)
					m_index = rowCount - 1;
				if (m_index < 0)
					m_index = 0;
				if (curve->xColumn()->isNumeric())
					q->setPositionLogical(curve->xColumn()->valueAt(m_index));
				else
					q->setPositionLogical(curve->xColumn()->dateTimeAt(m_index).toMSecsSinceEpoch());
				break;
			}
		}
	}
}

bool InfoElementPrivate::activate(QPointF mouseScenePos, double /*maxDist*/) {
	if (!isVisible())
		return false;

	return m_shape.contains(mouseScenePos);
}

void InfoElementPrivate::contextMenuEvent(QGraphicsSceneContextMenuEvent* event) {
	if (activate(event->pos())) {
		q->createContextMenu()->exec(event->screenPos());
		return;
	}
	QGraphicsItem::contextMenuEvent(event);
}

void InfoElementPrivate::recalcShape() {
	m_shape = QPainterPath();

	if (verticalLine->style() != Qt::PenStyle::NoPen) {
		QPainterPath path;
		path.moveTo(xposLine.p1());
		path.lineTo(xposLine.p2());
		m_shape.addPath(WorksheetElement::shapeFromPath(path, verticalLine->pen()));
	}

	if (connectionLine->style() != Qt::PenStyle::NoPen) {
		QPainterPath path;
		path.moveTo(m_connectionLine.p1());
		path.lineTo(m_connectionLine.p2());
		m_shape.addPath(WorksheetElement::shapeFromPath(path, connectionLine->pen()));
	}
}

void InfoElementPrivate::recalcShapeAndBoundingRect(const QRectF& rect) {
	prepareGeometryChange();
	m_boundingRectangle = rect;
	recalcShape();
	update(m_boundingRectangle);
}

void InfoElementPrivate::recalcShapeAndBoundingRect() {
	prepareGeometryChange();
	recalcShape();
	update(m_boundingRectangle);
}

// ##############################################################################
// ##################  Serialization/Deserialization  ###########################
// ##############################################################################
void InfoElement::save(QXmlStreamWriter* writer) const {
	Q_D(const InfoElement);

	writer->writeStartElement(QStringLiteral("infoElement"));
	writeBasicAttributes(writer);
	writeCommentElement(writer);

	// general
	writer->writeStartElement(QStringLiteral("general"));
	writer->writeAttribute(QStringLiteral("position"), QString::number(d->positionLogical));
	writer->writeAttribute(QStringLiteral("curve"), d->connectionLineCurveName);
	writer->writeAttribute(QStringLiteral("gluePointIndex"), QString::number(d->gluePointIndex));
	writer->writeAttribute(QStringLiteral("markerIndex"), QString::number(d->m_index));
	writer->writeAttribute(QStringLiteral("plotRangeIndex"), QString::number(m_cSystemIndex));
	writer->writeAttribute(QStringLiteral("visible"), QString::number(d->isVisible()));
	writer->writeEndElement();

	// lines
	d->verticalLine->save(writer);
	d->connectionLine->save(writer);

	// text label
	m_title->save(writer);

	// custom points
	if (!markerpoints.isEmpty()) {
		writer->writeStartElement(QStringLiteral("points"));
		for (const auto& custompoint : markerpoints) {
			writer->writeStartElement(QStringLiteral("point"));
			writer->writeAttribute(QLatin1String("curvepath"), custompoint.curve->path());
			writer->writeAttribute(QLatin1String("visible"), QString::number(custompoint.visible));
			custompoint.customPoint->save(writer);
			writer->writeEndElement(); // close "point"
		}
		writer->writeEndElement(); // close "points"
	}

	writer->writeEndElement(); // close "infoElement"
}

bool InfoElement::load(XmlStreamReader* reader, bool preview) {
	if (!readBasicAttributes(reader))
		return false;

	Q_D(InfoElement);

	QXmlStreamAttributes attribs;
	QString str;

	while (!reader->atEnd()) {
		reader->readNext();
		if (reader->isEndElement() && reader->name() == QLatin1String("infoElement"))
			break;

		if (!reader->isStartElement())
			continue;

		if (!preview && reader->name() == QLatin1String("comment")) {
			if (!readCommentElement(reader))
				return false;
		} else if (reader->name() == QLatin1String("general")) {
			attribs = reader->attributes();

			READ_INT_VALUE_DIRECT("plotRangeIndex", m_cSystemIndex, int);

			str = attribs.value(QStringLiteral("visible")).toString();
			if (str.isEmpty())
				reader->raiseMissingAttributeWarning(QStringLiteral("x"));
			else
				setVisible(str.toInt());

			READ_DOUBLE_VALUE("position", positionLogical);
			READ_INT_VALUE("gluePointIndex", gluePointIndex, int);
			READ_INT_VALUE("markerIndex", m_index, int);
			READ_STRING_VALUE("curve", connectionLineCurveName);
		} else if (reader->name() == QLatin1String("verticalLine")) {
			d->verticalLine->load(reader, preview);
		} else if (reader->name() == QLatin1String("connectionLine")) {
			d->connectionLine->load(reader, preview);
		} else if (reader->name() == QLatin1String("textLabel")) {
			if (!m_title) {
				m_title = new TextLabel(i18n("Label"), d->m_plot);
				m_title->setIsLoading(true);
				this->addChild(m_title);
			}
			if (!m_title->load(reader, preview))
				return false;
		} else if (reader->name() == QLatin1String("points")) {
			loadPoints(reader, preview);
		} else { // unknown element
			reader->raiseUnknownElementWarning();
			if (!reader->skipToEndElement())
				return false;
		}
	}

	return true;
}

void InfoElement::loadPoints(XmlStreamReader* reader, bool preview) {
	reader->readNextStartElement();
	if (!reader->isStartElement())
		return;

	while (!(reader->isEndElement() && reader->name() == QStringLiteral("points"))) {
		Q_D(const InfoElement);
		if (!reader->isStartElement()) {
			reader->readNext();
			continue;
		}
		if (reader->name() != QLatin1String("point"))
			break;
		const auto& attribs = reader->attributes();
		const QString curvePath = attribs.value(QStringLiteral("curvepath")).toString();
		const bool visible = attribs.value(QStringLiteral("visible")).toInt();

		reader->readNextStartElement();
		if (reader->name() != CustomPoint::xmlName())
			break;

		auto* point = new CustomPoint(d->m_plot, QString());
		point->setIsLoading(true);
		if (!point->load(reader, preview)) {
			delete point;
			return;
		}
		point->setVisible(visible);
		this->addChild(point);
		addCurvePath(curvePath, point);
	}
}

// ##############################################################################
// #########################  Theme management ##################################
// ##############################################################################
void InfoElement::loadThemeConfig(const KConfig& config) {
	// use the color for the axis line from the theme also for info element's lines
	const KConfigGroup& group = config.group(QStringLiteral("Axis"));

	const QColor& themeColor = group.readEntry(QStringLiteral("LineColor"), QColor(Qt::black));
	Q_D(InfoElement);
	d->verticalLine->loadThemeConfig(group, themeColor);
	d->connectionLine->loadThemeConfig(group, themeColor);

	// load the theme for all the children
	const auto& children = this->children<WorksheetElement>(ChildIndexFlag::IncludeHidden);
	for (auto* child : children)
		child->loadThemeConfig(config);
}
