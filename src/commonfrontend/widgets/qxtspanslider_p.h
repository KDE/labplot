#ifndef QXTSPANSLIDER_P_H
/****************************************************************************
** Copyright (c) 2006 - 2011, the LibQxt project.
** See the Qxt AUTHORS file for a list of authors and copyright holders.
** All rights reserved.
**
** Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are met:
**     * Redistributions of source code must retain the above copyright
**       notice, this list of conditions and the following disclaimer.
**     * Redistributions in binary form must reproduce the above copyright
**       notice, this list of conditions and the following disclaimer in the
**       documentation and/or other materials provided with the distribution.
**     * Neither the name of the LibQxt project nor the
**       names of its contributors may be used to endorse or promote products
**       derived from this software without specific prior written permission.
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
** ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
** WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
** DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
** DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
** (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
** LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
** ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
** SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
**
** <http://libqxt.org>  <foundation@libqxt.org>
*****************************************************************************/

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
	int pick(const QPoint& pt) const {
		return qxt_p().orientation() == Qt::Horizontal ? pt.x() : pt.y();
	}
	int pixelPosToRangeValue(int pos) const;
	void handleMousePress(const QPoint& pos, QStyle::SubControl& control, int value, QxtSpanSlider::SpanHandle handle);
	void drawHandle(QStylePainter* painter, QxtSpanSlider::SpanHandle handle) const;
	void setupPainter(QPainter* painter, Qt::Orientation orientation, qreal x1, qreal y1, qreal x2, qreal y2) const;
	void drawSpan(QStylePainter* painter, const QRect& rect) const;
	void triggerAction(QAbstractSlider::SliderAction action, bool main);
	void swapControls();

	int lower;
	int upper;
	int lowerPos;
	int upperPos;
	int offset;
	int position;
	QxtSpanSlider::SpanHandle lastPressed;
	QxtSpanSlider::SpanHandle mainControl;
	QStyle::SubControl lowerPressed;
	QStyle::SubControl upperPressed;
	QxtSpanSlider::HandleMovementMode movement;
	bool firstMovement;
	bool blockTracking;

public Q_SLOTS:
	void updateRange(int min, int max);
	void movePressedHandle();
};

#endif // QXTSPANSLIDER_P_H
