#ifndef IMAGEVIEW_H
#define IMAGEVIEW_H

#include <QGraphicsView>
#include "backend/worksheet/Image.h"

class QMenu;
class QToolBar;
class QToolButton;
class QWheelEvent;

class AbstractAspect;
class WorksheetElement;

class ImageView : public QGraphicsView {
	Q_OBJECT

  public:
    explicit ImageView(Image* image);

	enum ExportFormat{Pdf, Eps, Svg, Png};

	void setScene(QGraphicsScene*);
    void exportToFile(const QString&, const ExportFormat, const int);

  private:
	enum MouseMode{SelectionMode, NavigationMode, ZoomSelectionMode};

	void initActions();
	void initMenus();
	void drawForeground(QPainter*, const QRectF&);
	void drawBackground(QPainter*, const QRectF&);
    void exportPaint(QPainter* painter, const QRectF& targetRect, const QRectF& sourceRect);

	//events
	void contextMenuEvent(QContextMenuEvent*);
	void wheelEvent(QWheelEvent*);
	void mousePressEvent(QMouseEvent*);
	void mouseReleaseEvent(QMouseEvent*);
	void mouseMoveEvent(QMouseEvent*);
    CustomItem *addCustomItem(const QPointF&);
    void updateData(const CustomItem*);

    Image* m_image;
	MouseMode m_mouseMode;
	bool m_selectionBandIsShown;
	QPoint m_selectionStart;
	QPoint m_selectionEnd;

	//Menus
	QMenu* m_zoomMenu;
	QMenu* m_viewMouseModeMenu;
    QMenu* m_viewImageMenu;


	QToolButton* tbZoom;
	QAction* currentZoomAction;

	//Actions
	QAction* zoomInViewAction;
	QAction* zoomOutViewAction;
	QAction* zoomOriginAction;
	QAction* zoomFitPageHeightAction;
	QAction* zoomFitPageWidthAction;
	QAction* zoomFitSelectionAction;

    QAction* setAxisPointsAction;
    QAction* setCurvePointsAction;
    QAction* selectSegmentAction;

    QAction* updateDatasheetAction;

	QAction* navigationModeAction;
	QAction* zoomSelectionModeAction;
	QAction* selectionModeAction;

  public slots:
	void createContextMenu(QMenu*) const;
	void fillToolBar(QToolBar*);
	void print(QPrinter*) const;

  private slots:
	void mouseModeChanged(QAction*);
	void changeZoom(QAction*);
    void handleImageActions();
	void updateBackground();
    void changePointsType(QAction*);
    void updateDatasheet();

  signals:
	void statusInfo(const QString&);
};

#endif
