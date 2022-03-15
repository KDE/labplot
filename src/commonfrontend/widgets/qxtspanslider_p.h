/*
 * SPDX-FileCopyrightText: 2006-2011 the LibQxt project <http://libqxt.org, foundation@libqxt.org>
 * SPDX-License-Identifier: BSD-3-Clause
*/

#ifndef QXTSPANSLIDER_P_H
#define QXTSPANSLIDER_P_H

#include <QStyle>
#include <QObject>
#include "qxtspanslider.h"

QT_FORWARD_DECLARE_CLASS(QStylePainter)
QT_FORWARD_DECLARE_CLASS(QStyleOptionSlider)

class QxtSpanSliderPrivate : public QObject, public QxtPrivate<QxtSpanSlider> {
	Q_OBJECT

public:
	QXT_DECLARE_PUBLIC(QxtSpanSlider)

	QxtSpanSliderPrivate();
	void initStyleOption(QStyleOptionSlider* option, QxtSpanSlider::SpanHandle handle = QxtSpanSlider::UpperHandle) const;
	int pick(QPoint pt) const {
		return qxt_p().orientation() == Qt::Horizontal ? pt.x() : pt.y();
	}
	int pixelPosToRangeValue(int pos) const;
	void handleMousePress(QPoint pos, QStyle::SubControl& control, int value, QxtSpanSlider::SpanHandle handle);
	void drawHandle(QStylePainter* painter, QxtSpanSlider::SpanHandle handle) const;
	void setupPainter(QPainter* painter, Qt::Orientation orientation, qreal x1, qreal y1, qreal x2, qreal y2) const;
	void drawSpan(QStylePainter* painter, QRect rect) const;
	void triggerAction(QAbstractSlider::SliderAction action, bool main);
	void swapControls();

	int lower{0};
	int upper{0};
	int lowerPos{0};
	int upperPos{0};
	int offset{0};
	int position{0};
	QxtSpanSlider::SpanHandle lastPressed{QxtSpanSlider::NoHandle};
	QxtSpanSlider::SpanHandle mainControl{QxtSpanSlider::LowerHandle};
	QStyle::SubControl lowerPressed{QStyle::SC_None};
	QStyle::SubControl upperPressed{QStyle::SC_None};
	QxtSpanSlider::HandleMovementMode movement{QxtSpanSlider::FreeMovement};
	bool firstMovement{false};
	bool blockTracking{false};

public Q_SLOTS:
	void updateRange(int min, int max);
	void movePressedHandle();
};

#endif // QXTSPANSLIDER_P_H
