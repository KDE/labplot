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

#ifndef WORKSHEETVIEW_H
#define WORKSHEETVIEW_H

#include <QGraphicsView>

class QParallelAnimationGroup;
class QPropertyAnimation;

class Worksheet;

class WorksheetView : public QGraphicsView
{
  Q_OBJECT
  public:
    WorksheetView(Worksheet* scene, QWidget* parent);
    ~WorksheetView();

    void makeVisible(const QRectF& sceneRect);
    bool isVisible(const QRectF& sceneRect);
    bool isAtEnd();
    void scrollToEnd();
    void scrollBy(int dy);

    QPoint viewCursorPos();
    QPointF sceneCursorPos();

    QRectF viewRect();

    void resizeEvent(QResizeEvent* event);

    qreal scaleFactor();

    void updateSceneSize();

  Q_SIGNALS:
    void viewRectChanged(QRectF rect);

  public Q_SLOTS:
    void zoomIn();
    void zoomOut();
    void endAnimation();
    void sceneRectChanged(const QRectF& sceneRect);
    void sendViewRectChange();

  private:
    qreal m_scale;
    QParallelAnimationGroup* m_animation;
    QPropertyAnimation* m_hAnimation;
    QPropertyAnimation* m_vAnimation;
    Worksheet* m_worksheet;
};

#endif //WORKSHEETVIEW_H
