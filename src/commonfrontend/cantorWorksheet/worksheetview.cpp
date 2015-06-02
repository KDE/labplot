/*
    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License
    as published by the Free Software Foundation; either version 2
    of the License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA  02110-1301, USA.

    ---
    Copyright (C) 2012 Martin Kuettler <martin.kuettler@gmail.com>
 */

#include "worksheetview.h"
#include "worksheet.h"

#include <QParallelAnimationGroup>
#include <QPropertyAnimation>
#include <QScrollBar>
#include <QDebug>

WorksheetView::WorksheetView(Worksheet* scene, QWidget* parent)
    : QGraphicsView(scene, parent)
{
    m_scale = 1;
    m_animation = 0;
    m_hAnimation = 0;
    m_vAnimation = 0;
    m_worksheet = scene;
    connect(scene, SIGNAL(sceneRectChanged(const QRectF&)),
            this, SLOT(sceneRectChanged(const QRectF&)));
    setAlignment(Qt::AlignLeft | Qt::AlignTop);
    //setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
}

WorksheetView::~WorksheetView()
{
}

void WorksheetView::makeVisible(const QRectF& sceneRect)
{
    const qreal w = viewport()->width();
    const qreal h = viewport()->height();

    QRectF rect(m_scale*sceneRect.topLeft(), m_scale*sceneRect.size());

    qreal x,y;
    if (m_animation) {
        x = m_hAnimation->endValue().toReal();
        y = m_vAnimation->endValue().toReal();

        if (QRectF(x,y,w,h).contains(rect))
            return;
    }

    if (horizontalScrollBar())
        x = horizontalScrollBar()->value();
    else
        x = 0;
    if (verticalScrollBar())
        y = verticalScrollBar()->value();
    else
        y = 0;

    qDebug() << rect << QRectF(x,y,w,h);

    if (!m_animation && QRectF(x,y,w,h).contains(rect))
        return;

    qreal nx, ny;
    if (y > rect.y() || rect.height() > h)
        ny = rect.y();
    else
        ny = rect.y() + rect.height() - h;
    if (rect.x() + rect.width() <= w || x > rect.x())
        nx = 0;
    else
        nx = rect.x() + rect.width() - w;

    qDebug() << nx << ny;

    if (!m_worksheet->animationsEnabled()) {
        if (horizontalScrollBar())
            horizontalScrollBar()->setValue(nx);
        if (verticalScrollBar())
            verticalScrollBar()->setValue(ny);
        return;
    }

    if (!m_animation)
        m_animation = new QParallelAnimationGroup(this);

    if (horizontalScrollBar()) {
        if (!m_hAnimation) {
            m_hAnimation = new QPropertyAnimation(horizontalScrollBar(),
                                                  "value", this);
            m_hAnimation->setStartValue(horizontalScrollBar()->value());
            nx = qBound(qreal(0.0), nx, qreal(0.0+horizontalScrollBar()->maximum()));
            m_hAnimation->setEndValue(nx);
            m_hAnimation->setDuration(100);
            m_animation->addAnimation(m_hAnimation);
        } else {
            qreal progress = static_cast<qreal>(m_hAnimation->currentTime()) /
                m_hAnimation->totalDuration();
            QEasingCurve curve = m_hAnimation->easingCurve();
            qreal value = curve.valueForProgress(progress);
            qreal sx = 1/(1-value)*(m_hAnimation->currentValue().toReal() -
                                    value * nx);
            m_hAnimation->setStartValue(sx);
            m_hAnimation->setEndValue(nx);
        }
    } else {
        m_hAnimation = 0;
    }

    if (verticalScrollBar()) {
        if (!m_vAnimation) {
            m_vAnimation = new QPropertyAnimation(verticalScrollBar(),
                                                  "value", this);
            m_vAnimation->setStartValue(verticalScrollBar()->value());
            ny = qBound(qreal(0.0), ny, qreal(0.0+verticalScrollBar()->maximum()));
            m_vAnimation->setEndValue(ny);
            m_vAnimation->setDuration(100);
            m_animation->addAnimation(m_vAnimation);
        } else {
            qreal progress = static_cast<qreal>(m_vAnimation->currentTime()) /
                m_vAnimation->totalDuration();
            QEasingCurve curve = m_vAnimation->easingCurve();
            qreal value = curve.valueForProgress(progress);
            qreal sy = 1/(1-value)*(m_vAnimation->currentValue().toReal() -
                                    value * ny);
            m_vAnimation->setStartValue(sy);
            m_vAnimation->setEndValue(ny);
            //qDebug() << sy << value << ny;
        }
    } else {
        m_vAnimation = 0;
    }

    connect(m_animation, &QParallelAnimationGroup::finished, this, &WorksheetView::endAnimation);
    m_animation->start();
}

bool WorksheetView::isVisible(const QRectF& sceneRect)
{
    const qreal w = viewport()->width();
    const qreal h = viewport()->height();

    QRectF rect(m_scale*sceneRect.topLeft(), m_scale*sceneRect.size());

    qreal x,y;
    if (m_animation) {
        x = m_hAnimation->endValue().toReal();
        y = m_vAnimation->endValue().toReal();
    } else {
        if (horizontalScrollBar())
            x = horizontalScrollBar()->value();
        else
            x = 0;
        if (verticalScrollBar())
            y = verticalScrollBar()->value();
        else
            y = 0;
    }

    return QRectF(x,y,w,h).contains(rect);
}

bool WorksheetView::isAtEnd()
{
    bool atEnd = true;
    if (verticalScrollBar())
        atEnd &= (verticalScrollBar()->value()==verticalScrollBar()->maximum());
    return atEnd;
}

void WorksheetView::scrollToEnd()
{
    if (verticalScrollBar())
        verticalScrollBar()->setValue(verticalScrollBar()->maximum());
}

void WorksheetView::scrollBy(int dy)
{
    if (!verticalScrollBar())
        return;

    int ny = verticalScrollBar()->value() + dy;
    if (ny < 0)
        ny = 0;
    else if (ny > verticalScrollBar()->maximum())
        ny = verticalScrollBar()->maximum();

    int x;
    if (horizontalScrollBar())
        x = horizontalScrollBar()->value();
    else
        x = 0;

    const qreal w = viewport()->width() / m_scale;
    const qreal h = viewport()->height() / m_scale;
    makeVisible(QRectF(x, ny, w, h));
}

void WorksheetView::endAnimation()
{
    if (!m_animation)
        return;

    m_animation->deleteLater();
    m_hAnimation = 0;
    m_vAnimation = 0;
    m_animation = 0;
}

QPoint WorksheetView::viewCursorPos()
{
    return viewport()->mapFromGlobal(QCursor::pos());
}

QPointF WorksheetView::sceneCursorPos()
{
    return mapToScene(viewCursorPos());
}

QRectF WorksheetView::viewRect()
{
    const qreal w = viewport()->width() / m_scale;
    const qreal h = viewport()->height() / m_scale;
    qreal y = verticalScrollBar()->value();
    qreal x = horizontalScrollBar() ? horizontalScrollBar()->value() : 0;
    return QRectF(x, y, w, h);
}

void WorksheetView::resizeEvent(QResizeEvent * event)
{
    QGraphicsView::resizeEvent(event);
    updateSceneSize();
}

qreal WorksheetView::scaleFactor()
{
    return m_scale;
}

void WorksheetView::updateSceneSize()
{
    QSize s = viewport()->size();
    m_worksheet->setViewSize(s.width()/m_scale, s.height()/m_scale, m_scale);
    sendViewRectChange();
}

void WorksheetView::sceneRectChanged(const QRectF& sceneRect)
{
    Q_UNUSED(sceneRect);
    if (verticalScrollBar())
        connect(verticalScrollBar(), SIGNAL(valueChanged(int)),
                this, SLOT(sendViewRectChange()), Qt::UniqueConnection);
    if (horizontalScrollBar())
        connect(horizontalScrollBar(), SIGNAL(valueChanged(int)),
                this, SLOT(sendViewRectChange()), Qt::UniqueConnection);
}

void WorksheetView::sendViewRectChange()
{
    emit viewRectChanged(viewRect());
}

void WorksheetView::zoomIn()
{
    m_scale *= 1.1;
    scale(1.1, 1.1);
    updateSceneSize();
}

void WorksheetView::zoomOut()
{
    m_scale /= 1.1;
    scale(1/1.1, 1/1.1);
    updateSceneSize();
}


