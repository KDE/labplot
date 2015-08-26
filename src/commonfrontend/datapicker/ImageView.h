/***************************************************************************
    File                 : ImageView.h
    Project              : LabPlot
    Description          : Image view for datapicker
    --------------------------------------------------------------------
    Copyright            : (C) 2015 by Ankit Wagadre (wagadre.ankit@gmail.com)

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

#ifndef IMAGEVIEW_H
#define IMAGEVIEW_H

#include <QGraphicsView>

class QMenu;
class QToolBar;
class QToolButton;
class QWheelEvent;

class AbstractAspect;
class Image;
class WorksheetElement;
class Transform;
class CustomItem;
class QActionGroup;

class ImageView : public QGraphicsView {
    Q_OBJECT

    public:
        explicit ImageView(Image* image);

        enum ExportFormat{Pdf, Eps, Svg, Png};

        void setScene(QGraphicsScene*);
        void exportToFile(const QString&, const ExportFormat, const int);

    private:
        enum MouseMode{SelectAndEditMode, NavigationMode, ZoomSelectionMode, SelectAndMoveMode};

        void initActions();
        void initMenus();
        void addAxisPoint(const QPointF&);
        void drawForeground(QPainter*, const QRectF&);
        void drawBackground(QPainter*, const QRectF&);
        void exportPaint(QPainter* painter, const QRectF& targetRect, const QRectF& sourceRect);

        //events
        void contextMenuEvent(QContextMenuEvent*);
        void wheelEvent(QWheelEvent*);
        void mousePressEvent(QMouseEvent*);
        void mouseReleaseEvent(QMouseEvent*);
        void mouseMoveEvent(QMouseEvent*);

        Image* m_image;
        Transform* m_transform;
        MouseMode m_mouseMode;
        bool m_selectionBandIsShown;
        QPoint m_selectionStart;
        QPoint m_selectionEnd;
        int magnificationFactor;

        //Menus
        QMenu* m_zoomMenu;
        QMenu* m_viewMouseModeMenu;
        QMenu* m_viewImageMenu;
        QMenu* m_navigationMenu;
        QMenu* m_magnificationMenu;
        QMenu* m_curveMenu;
        QMenu* m_activeCurveMenu;


        QToolButton* tbZoom;
        QAction* currentZoomAction;

        //Actions
        QAction* zoomInViewAction;
        QAction* zoomOutViewAction;
        QAction* zoomOriginAction;
        QAction* zoomFitPageHeightAction;
        QAction* zoomFitPageWidthAction;

        QAction* setAxisPointsAction;
        QAction* setCurvePointsAction;
        QAction* selectSegmentAction;

        QAction* addCurveAction;
        QActionGroup* m_activeCurveActionGroup;

        QAction* navigationModeAction;
        QAction* zoomSelectionModeAction;
        QAction* selectAndEditModeAction;
        QAction* selectAndMoveModeAction;

        QAction* shiftLeftAction;
        QAction* shiftRightAction;
        QAction* shiftDownAction;
        QAction* shiftUpAction;

        QAction* noMagnificationAction;
        QAction* twoTimesMagnificationAction;
        QAction* threeTimesMagnificationAction;
        QAction* fourTimesMagnificationAction;
        QAction* fiveTimesMagnificationAction;

    public slots:
        void createContextMenu(QMenu*) const;
        void fillToolBar(QToolBar*);
        void print(QPrinter*) const;

    private slots:
        void mouseModeChanged(QAction*);
        void magnificationChanged(QAction*);
        void changeZoom(QAction*);
        void changeSelectedItemsPosition(QAction*);
        void changePointsType(QAction*);
        void activeCurveChanged(QAction*);
        void handleImageActions();
        void handleActiveCurveChanged();
        void updateBackground();
        void addCurve();
        void changeRotationAngle();

    signals:
        void statusInfo(const QString&);
};

#endif
