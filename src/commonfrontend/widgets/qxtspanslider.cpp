/*
 * SPDX-FileCopyrightText: 2006-2011 the LibQxt project <http://libqxt.org, foundation@libqxt.org>
 * SPDX-License-Identifier: BSD-3-Clause
*/

#include "qxtspanslider.h"

#include "qxtspanslider_p.h"
#include <QKeyEvent>
#include <QMouseEvent>
#include <QStylePainter>
#include <QStyleOptionSlider>

QxtSpanSliderPrivate::QxtSpanSliderPrivate() :
	lower(0),
	upper(0),
	lowerPos(0),
	upperPos(0),
	offset(0),
	position(0),
	lastPressed(QxtSpanSlider::NoHandle),
	mainControl(QxtSpanSlider::LowerHandle),
	lowerPressed(QStyle::SC_None),
	upperPressed(QStyle::SC_None),
	movement(QxtSpanSlider::FreeMovement),
	firstMovement(false),
	blockTracking(false) {
}

void QxtSpanSliderPrivate::initStyleOption(QStyleOptionSlider* option, QxtSpanSlider::SpanHandle handle) const {
	const QxtSpanSlider* p = &qxt_p();
	p->initStyleOption(option);
	option->sliderPosition = (handle == QxtSpanSlider::LowerHandle ? lowerPos : upperPos);
	option->sliderValue = (handle == QxtSpanSlider::LowerHandle ? lower : upper);
}

int QxtSpanSliderPrivate::pixelPosToRangeValue(int pos) const {
	QStyleOptionSlider opt;
	initStyleOption(&opt);

	int sliderMin = 0;
	int sliderMax = 0;
	int sliderLength = 0;
	const QSlider* p = &qxt_p();
	const QRect gr = p->style()->subControlRect(QStyle::CC_Slider, &opt, QStyle::SC_SliderGroove, p);
	const QRect sr = p->style()->subControlRect(QStyle::CC_Slider, &opt, QStyle::SC_SliderHandle, p);
	if (p->orientation() == Qt::Horizontal) {
		sliderLength = sr.width();
		sliderMin = gr.x();
		sliderMax = gr.right() - sliderLength + 1;
	} else {
		sliderLength = sr.height();
		sliderMin = gr.y();
		sliderMax = gr.bottom() - sliderLength + 1;
	}
	return QStyle::sliderValueFromPosition(p->minimum(), p->maximum(), pos - sliderMin,
	                                       sliderMax - sliderMin, opt.upsideDown);
}

void QxtSpanSliderPrivate::handleMousePress(QPoint pos, QStyle::SubControl& control, int value, QxtSpanSlider::SpanHandle handle) {
	QStyleOptionSlider opt;
	initStyleOption(&opt, handle);
	QxtSpanSlider* p = &qxt_p();
	const QStyle::SubControl oldControl = control;
	control = p->style()->hitTestComplexControl(QStyle::CC_Slider, &opt, pos, p);
	const QRect sr = p->style()->subControlRect(QStyle::CC_Slider, &opt, QStyle::SC_SliderHandle, p);
	if (control == QStyle::SC_SliderHandle) {
		position = value;
		offset = pick(pos - sr.topLeft());
		lastPressed = handle;
		p->setSliderDown(true);
		emit p->sliderPressed(handle);
	}
	if (control != oldControl)
		p->update(sr);
}

void QxtSpanSliderPrivate::setupPainter(QPainter* painter, Qt::Orientation orientation, qreal x1, qreal y1, qreal x2, qreal y2) const {
	QColor highlight = qxt_p().palette().color(QPalette::Highlight);
	QLinearGradient gradient(x1, y1, x2, y2);
	gradient.setColorAt(0, highlight.darker(120));
	gradient.setColorAt(1, highlight.lighter(108));
	painter->setBrush(gradient);

	if (orientation == Qt::Horizontal)
		painter->setPen(QPen(highlight.darker(130), 0));
	else
		painter->setPen(QPen(highlight.darker(150), 0));
}

void QxtSpanSliderPrivate::drawSpan(QStylePainter* painter, QRect rect) const {
	QStyleOptionSlider opt;
	initStyleOption(&opt);
	const QSlider* p = &qxt_p();

	// area
	QRect groove = p->style()->subControlRect(QStyle::CC_Slider, &opt, QStyle::SC_SliderGroove, p);
	if (opt.orientation == Qt::Horizontal)
		groove.adjust(0, 0, -1, 0);
	else
		groove.adjust(0, 0, 0, -1);

	// pen & brush
	painter->setPen(QPen(p->palette().color(QPalette::Dark).lighter(110), 0));
	if (opt.orientation == Qt::Horizontal)
		setupPainter(painter, opt.orientation, groove.center().x(), groove.top(), groove.center().x(), groove.bottom());
	else
		setupPainter(painter, opt.orientation, groove.left(), groove.center().y(), groove.right(), groove.center().y());

	// draw groove
	painter->drawRect(rect.intersected(groove));
}

void QxtSpanSliderPrivate::drawHandle(QStylePainter* painter, QxtSpanSlider::SpanHandle handle) const {
	QStyleOptionSlider opt;
	initStyleOption(&opt, handle);
	opt.subControls = QStyle::SC_SliderHandle;
	QStyle::SubControl pressed = (handle == QxtSpanSlider::LowerHandle ? lowerPressed : upperPressed);
	if (pressed == QStyle::SC_SliderHandle) {
		opt.activeSubControls = pressed;
		opt.state |= QStyle::State_Sunken;
	}
	painter->drawComplexControl(QStyle::CC_Slider, opt);
}

void QxtSpanSliderPrivate::triggerAction(QAbstractSlider::SliderAction action, bool main) {
	int value = 0;
	bool update = true;
	bool isUpperHandle = false;
	const int min = qxt_p().minimum();
	const int max = qxt_p().maximum();
	const QxtSpanSlider::SpanHandle altControl = (mainControl == QxtSpanSlider::LowerHandle ? QxtSpanSlider::UpperHandle : QxtSpanSlider::LowerHandle);

	blockTracking = true;

	switch (action) {
	case QAbstractSlider::SliderSingleStepAdd:
		if ((main && mainControl == QxtSpanSlider::UpperHandle) || (!main && altControl == QxtSpanSlider::UpperHandle)) {
			value = qBound(min, upper + qxt_p().singleStep(), max);
			isUpperHandle = true;
		} else {
			value = qBound(min, lower + qxt_p().singleStep(), max);
		}
		break;
	case QAbstractSlider::SliderSingleStepSub:
		if ((main && mainControl == QxtSpanSlider::UpperHandle) || (!main && altControl == QxtSpanSlider::UpperHandle)) {
			value = qBound(min, upper - qxt_p().singleStep(), max);
			isUpperHandle = true;
		} else {
			value = qBound(min, lower - qxt_p().singleStep(), max);
		}
		break;
	case QAbstractSlider::SliderToMinimum:
		value = min;
		if ((main && mainControl == QxtSpanSlider::UpperHandle) || (!main && altControl == QxtSpanSlider::UpperHandle))
			isUpperHandle = true;
		break;
	case QAbstractSlider::SliderToMaximum:
		value = max;
		if ((main && mainControl == QxtSpanSlider::UpperHandle) || (!main && altControl == QxtSpanSlider::UpperHandle))
			isUpperHandle = true;
		break;
	case QAbstractSlider::SliderMove:
		// This is handled not here, but in QxtSpanSlider::mouseMoveEvent
		// so update not needed
		update = false;
		break;
	case QAbstractSlider::SliderNoAction:
		update = false;
		break;
	case QAbstractSlider::SliderPageStepAdd:
	case QAbstractSlider::SliderPageStepSub:
		break;
	}

	if (update) {
		if ( isUpperHandle ) {
			if (movement == QxtSpanSlider::NoCrossing)
				value = qMin(value, upper);
			else if (movement == QxtSpanSlider::NoOverlapping)
				value = qMin(value, upper - 1);

			if (movement == QxtSpanSlider::FreeMovement && value < lower) {
				swapControls();
				qxt_p().setLowerPosition(value);
			} else
				qxt_p().setUpperPosition(value);
		} else {
			if (movement == QxtSpanSlider::NoCrossing)
				value = qMax(value, lower);
			else if (movement == QxtSpanSlider::NoOverlapping)
				value = qMax(value, lower + 1);

			if (movement == QxtSpanSlider::FreeMovement && value > upper) {
				swapControls();
				qxt_p().setUpperPosition(value);
			} else
				qxt_p().setLowerPosition(value);
		}
	}

	blockTracking = false;
	qxt_p().setLowerValue(lowerPos);
	qxt_p().setUpperValue(upperPos);
}

void QxtSpanSliderPrivate::swapControls() {
	qSwap(lower, upper);
	qSwap(lowerPressed, upperPressed);
	lastPressed = (lastPressed == QxtSpanSlider::LowerHandle ? QxtSpanSlider::UpperHandle : QxtSpanSlider::LowerHandle);
	mainControl = (mainControl == QxtSpanSlider::LowerHandle ? QxtSpanSlider::UpperHandle : QxtSpanSlider::LowerHandle);
}

void QxtSpanSliderPrivate::updateRange(int /*min*/, int /*max*/) {
	// setSpan() takes care of keeping span in range
	qxt_p().setSpan(lower, upper);
}

void QxtSpanSliderPrivate::movePressedHandle() {
	switch (lastPressed) {
	case QxtSpanSlider::LowerHandle:
		if (lowerPos != lower) {
			bool main = (mainControl == QxtSpanSlider::LowerHandle);
			triggerAction(QAbstractSlider::SliderMove, main);
		}
		break;
	case QxtSpanSlider::UpperHandle:
		if (upperPos != upper) {
			bool main = (mainControl == QxtSpanSlider::UpperHandle);
			triggerAction(QAbstractSlider::SliderMove, main);
		}
		break;
	case QxtSpanSlider::NoHandle:
		break;
	}
}

/*!
    \class QxtSpanSlider
    \inmodule QxtWidgets
    \brief The QxtSpanSlider widget is a QSlider with two handles.

    QxtSpanSlider is a slider with two handles. QxtSpanSlider is
    handy for letting user to choose an span between min/max.

    The span color is calculated based on QPalette::Highlight.

    The keys are bound according to the following table:
    <table>
    <tr><th>Orientation     <th>Key            <th>Handle
    <tr><td>Qt::Horizontal  <td>Qt::Key_Left   <td>lower
    <tr><td>Qt::Horizontal  <td>Qt::Key_Right  <td>lower
    <tr><td>Qt::Horizontal  <td>Qt::Key_Up     <td>upper
    <tr><td>Qt::Horizontal  <td>Qt::Key_Down   <td>upper
    <tr><td>Qt::Vertical    <td>Qt::Key_Up     <td>lower
    <tr><td>Qt::Vertical    <td>Qt::Key_Down   <td>lower
    <tr><td>Qt::Vertical    <td>Qt::Key_Left   <td>upper
    <tr><td>Qt::Vertical    <td>Qt::Key_Right  <td>upper
    </table>

    Keys are bound by the time the slider is created. A key is bound
    to same handle for the lifetime of the slider. So even if the handle
    representation might change from lower to upper, the same key binding
    remains.

    \image qxtspanslider.png "QxtSpanSlider in Plastique style."

    <b>Note:</b> QxtSpanSlider inherits QSlider for implementation specific
    reasons. Adjusting any single handle specific properties like
    -# QAbstractSlider::sliderPosition
    -# QAbstractSlider::value
    has no effect. However, all slider specific properties like
    -# QAbstractSlider::invertedAppearance
    -# QAbstractSlider::invertedControls
    -# QAbstractSlider::minimum
    -# QAbstractSlider::maximum
    -# QAbstractSlider::orientation
    -# QAbstractSlider::pageStep
    -# QAbstractSlider::singleStep
    -# QSlider::tickInterval
    -# QSlider::tickPosition
    are taken into consideration.
 */

/*!
    \enum QxtSpanSlider::HandleMovementMode

    This enum describes the available handle movement modes.

    \value FreeMovement The handles can be moved freely.
    \value NoCrossing The handles cannot cross, but they can still overlap each other. The lower and upper values can be the same.
    \value NoOverlapping The handles cannot overlap each other. The lower and upper values cannot be the same.
 */

/*!
    \enum QxtSpanSlider::SpanHandle

    This enum describes the available span handles.

    \omitvalue NoHandle \omit Internal only (for now). \endomit
    \var LowerHandle The lower boundary handle.
    \var UpperHandle The upper boundary handle.
 */

/*!
    \fn QxtSpanSlider::lowerValueChanged(int lower)

    This signal is emitted whenever the \a lower value has changed.
 */

/*!
    \fn QxtSpanSlider::upperValueChanged(int upper)

    This signal is emitted whenever the \a upper value has changed.
 */

/*!
    \fn QxtSpanSlider::spanChanged(int lower, int upper)

    This signal is emitted whenever both the \a lower and the \a upper
    values have changed ie. the span has changed.
 */

/*!
    \fn QxtSpanSlider::lowerPositionChanged(int lower)

    This signal is emitted whenever the \a lower position has changed.
 */

/*!
    \fn QxtSpanSlider::upperPositionChanged(int upper)

    This signal is emitted whenever the \a upper position has changed.
 */

/*!
    \fn QxtSpanSlider::sliderPressed(SpanHandle handle)

    This signal is emitted whenever the \a handle has been pressed.
 */

/*!
    Constructs a new QxtSpanSlider with \a parent.
 */
QxtSpanSlider::QxtSpanSlider(QWidget* parent) : QSlider(parent) {
	QXT_INIT_PRIVATE(QxtSpanSlider);
	connect(this, SIGNAL(rangeChanged(int,int)), &qxt_d(), SLOT(updateRange(int,int)));
	connect(this, SIGNAL(sliderReleased()), &qxt_d(), SLOT(movePressedHandle()));
}

/*!
    Constructs a new QxtSpanSlider with \a orientation and \a parent.
 */
QxtSpanSlider::QxtSpanSlider(Qt::Orientation orientation, QWidget* parent) : QSlider(orientation, parent) {
	QXT_INIT_PRIVATE(QxtSpanSlider);
	connect(this, SIGNAL(rangeChanged(int,int)), &qxt_d(), SLOT(updateRange(int,int)));
	connect(this, SIGNAL(sliderReleased()), &qxt_d(), SLOT(movePressedHandle()));
}

/*!
    Destructs the span slider.
 */
QxtSpanSlider::~QxtSpanSlider() = default;

/*!
    \property QxtSpanSlider::handleMovementMode
    \brief the handle movement mode
 */
QxtSpanSlider::HandleMovementMode QxtSpanSlider::handleMovementMode() const {
	return qxt_d().movement;
}

void QxtSpanSlider::setHandleMovementMode(QxtSpanSlider::HandleMovementMode mode) {
	qxt_d().movement = mode;
}

/*!
    \property QxtSpanSlider::lowerValue
    \brief the lower value of the span
 */
int QxtSpanSlider::lowerValue() const {
	return qMin(qxt_d().lower, qxt_d().upper);
}

void QxtSpanSlider::setLowerValue(int lower) {
	setSpan(lower, qxt_d().upper);
}

/*!
    \property QxtSpanSlider::upperValue
    \brief the upper value of the span
 */
int QxtSpanSlider::upperValue() const {
	return qMax(qxt_d().lower, qxt_d().upper);
}

void QxtSpanSlider::setUpperValue(int upper) {
	setSpan(qxt_d().lower, upper);
}

/*!
    Sets the span from \a lower to \a upper.
 */
void QxtSpanSlider::setSpan(int lower, int upper) {
	const int low = qBound(minimum(), qMin(lower, upper), maximum());
	const int upp = qBound(minimum(), qMax(lower, upper), maximum());
	if (low != qxt_d().lower || upp != qxt_d().upper) {
		if (low != qxt_d().lower) {
			qxt_d().lower = low;
			qxt_d().lowerPos = low;
			emit lowerValueChanged(low);
		}
		if (upp != qxt_d().upper) {
			qxt_d().upper = upp;
			qxt_d().upperPos = upp;
			emit upperValueChanged(upp);
		}
		emit spanChanged(qxt_d().lower, qxt_d().upper);
		update();
	}
}

/*!
    \property QxtSpanSlider::lowerPosition
    \brief the lower position of the span
 */
int QxtSpanSlider::lowerPosition() const {
	return qxt_d().lowerPos;
}

void QxtSpanSlider::setLowerPosition(int lower) {
	if (qxt_d().lowerPos != lower) {
		qxt_d().lowerPos = lower;
		if (!hasTracking())
			update();
		if (isSliderDown())
			emit lowerPositionChanged(lower);
		if (hasTracking() && !qxt_d().blockTracking) {
			bool main = (qxt_d().mainControl == QxtSpanSlider::LowerHandle);
			qxt_d().triggerAction(SliderMove, main);
		}
	}
}

/*!
    \property QxtSpanSlider::upperPosition
    \brief the upper position of the span
 */
int QxtSpanSlider::upperPosition() const {
	return qxt_d().upperPos;
}

void QxtSpanSlider::setUpperPosition(int upper) {
	if (qxt_d().upperPos != upper) {
		qxt_d().upperPos = upper;
		if (!hasTracking())
			update();
		if (isSliderDown())
			emit upperPositionChanged(upper);
		if (hasTracking() && !qxt_d().blockTracking) {
			bool main = (qxt_d().mainControl == QxtSpanSlider::UpperHandle);
			qxt_d().triggerAction(SliderMove, main);
		}
	}
}

/*!
    \reimp
 */
void QxtSpanSlider::keyPressEvent(QKeyEvent* event) {
	QSlider::keyPressEvent(event);

	bool main = true;
	SliderAction action = SliderNoAction;
	switch (event->key()) {
	case Qt::Key_Left:
		main   = (orientation() == Qt::Horizontal);
		action = !invertedAppearance() ? SliderSingleStepSub : SliderSingleStepAdd;
		break;
	case Qt::Key_Right:
		main   = (orientation() == Qt::Horizontal);
		action = !invertedAppearance() ? SliderSingleStepAdd : SliderSingleStepSub;
		break;
	case Qt::Key_Up:
		main   = (orientation() == Qt::Vertical);
		action = invertedControls() ? SliderSingleStepSub : SliderSingleStepAdd;
		break;
	case Qt::Key_Down:
		main   = (orientation() == Qt::Vertical);
		action = invertedControls() ? SliderSingleStepAdd : SliderSingleStepSub;
		break;
	case Qt::Key_Home:
		main   = (qxt_d().mainControl == QxtSpanSlider::LowerHandle);
		action = SliderToMinimum;
		break;
	case Qt::Key_End:
		main   = (qxt_d().mainControl == QxtSpanSlider::UpperHandle);
		action = SliderToMaximum;
		break;
	default:
		event->ignore();
		break;
	}

	if (action)
		qxt_d().triggerAction(action, main);
}

/*!
    \reimp
 */
void QxtSpanSlider::mousePressEvent(QMouseEvent* event) {
	if (minimum() == maximum() || (event->buttons() ^ event->button())) {
		event->ignore();
		return;
	}

	qxt_d().handleMousePress(event->pos(), qxt_d().upperPressed, qxt_d().upper, QxtSpanSlider::UpperHandle);
	if (qxt_d().upperPressed != QStyle::SC_SliderHandle)
		qxt_d().handleMousePress(event->pos(), qxt_d().lowerPressed, qxt_d().lower, QxtSpanSlider::LowerHandle);

	qxt_d().firstMovement = true;
	event->accept();
}

/*!
    \reimp
 */
void QxtSpanSlider::mouseMoveEvent(QMouseEvent* event) {
	if (qxt_d().lowerPressed != QStyle::SC_SliderHandle && qxt_d().upperPressed != QStyle::SC_SliderHandle) {
		event->ignore();
		return;
	}

	QStyleOptionSlider opt;
	qxt_d().initStyleOption(&opt);
	const int m = style()->pixelMetric(QStyle::PM_MaximumDragDistance, &opt, this);
	int newPosition = qxt_d().pixelPosToRangeValue(qxt_d().pick(event->pos()) - qxt_d().offset);
	if (m >= 0) {
		const QRect r = rect().adjusted(-m, -m, m, m);
		if (!r.contains(event->pos()))
			newPosition = qxt_d().position;
	}

	// pick the preferred handle on the first movement
	if (qxt_d().firstMovement) {
		if (qxt_d().lower == qxt_d().upper) {
			if (newPosition < lowerValue()) {
				qxt_d().swapControls();
				qxt_d().firstMovement = false;
			}
		} else
			qxt_d().firstMovement = false;
	}

	if (qxt_d().lowerPressed == QStyle::SC_SliderHandle) {
		if (qxt_d().movement == NoCrossing)
			newPosition = qMin(newPosition, upperValue());
		else if (qxt_d().movement == NoOverlapping)
			newPosition = qMin(newPosition, upperValue() - 1);

		if (qxt_d().movement == FreeMovement && newPosition > qxt_d().upper) {
			qxt_d().swapControls();
			setUpperPosition(newPosition);
		} else
			setLowerPosition(newPosition);
	} else if (qxt_d().upperPressed == QStyle::SC_SliderHandle) {
		if (qxt_d().movement == NoCrossing)
			newPosition = qMax(newPosition, lowerValue());
		else if (qxt_d().movement == NoOverlapping)
			newPosition = qMax(newPosition, lowerValue() + 1);

		if (qxt_d().movement == FreeMovement && newPosition < qxt_d().lower) {
			qxt_d().swapControls();
			setLowerPosition(newPosition);
		} else
			setUpperPosition(newPosition);
	}
	event->accept();
}

/*!
    \reimp
 */
void QxtSpanSlider::mouseReleaseEvent(QMouseEvent* event) {
	QSlider::mouseReleaseEvent(event);
	setSliderDown(false);
	qxt_d().lowerPressed = QStyle::SC_None;
	qxt_d().upperPressed = QStyle::SC_None;
	update();
}

/*!
    \reimp
 */
void QxtSpanSlider::paintEvent(QPaintEvent*) {
	QStylePainter painter(this);

	// groove & ticks
	QStyleOptionSlider opt;
	qxt_d().initStyleOption(&opt);
	opt.sliderValue = 0;
	opt.sliderPosition = 0;
	opt.subControls = QStyle::SC_SliderGroove | QStyle::SC_SliderTickmarks;
	painter.drawComplexControl(QStyle::CC_Slider, opt);

	// handle rects
	opt.sliderPosition = qxt_d().lowerPos;
	const QRect lr = style()->subControlRect(QStyle::CC_Slider, &opt, QStyle::SC_SliderHandle, this);
	const int lrv  = qxt_d().pick(lr.center());
	opt.sliderPosition = qxt_d().upperPos;
	const QRect ur = style()->subControlRect(QStyle::CC_Slider, &opt, QStyle::SC_SliderHandle, this);
	const int urv  = qxt_d().pick(ur.center());

	// span
	const int minv = qMin(lrv, urv);
	const int maxv = qMax(lrv, urv);
	const QPoint c = QRect(lr.center(), ur.center()).center();
	QRect spanRect;
	if (orientation() == Qt::Horizontal)
		spanRect = QRect(QPoint(minv, c.y() - 2), QPoint(maxv, c.y() + 1));
	else
		spanRect = QRect(QPoint(c.x() - 2, minv), QPoint(c.x() + 1, maxv));
	qxt_d().drawSpan(&painter, spanRect);

	// handles
	qxt_d().drawHandle(&painter, QxtSpanSlider::UpperHandle);
	qxt_d().drawHandle(&painter, QxtSpanSlider::LowerHandle);
}
