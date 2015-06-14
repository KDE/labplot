#ifndef IMAGEREADERELEMENT_H
#define IMAGEREADERELEMENT_H

#include <QGraphicsView>
#include "backend/worksheet/Worksheet.h"

class QMenu;
class QToolBar;
class QToolButton;
class QWheelEvent;

class AbstractAspect;
class WorksheetElement;

class ImageReaderElement: public QGraphicsView {
	Q_OBJECT

  public:
    explicit ImageReaderElement(Worksheet* worksheet);

	enum ExportFormat{Pdf, Eps, Svg, Png};
	enum ExportArea{ExportBoundingBox, ExportSelection, ExportWorksheet};

	void setScene(QGraphicsScene*);
	void exportToFile(const QString&, const ExportFormat, const ExportArea, const bool, const int);

  private:
	enum MouseMode{SelectionMode, NavigationMode, ZoomSelectionMode};

	void initActions();
	void initMenus();
	void processResize();
	void drawForeground(QPainter*, const QRectF&);
	void drawBackground(QPainter*, const QRectF&);
	void exportPaint(QPainter* painter, const QRectF& targetRect, const QRectF& sourceRect, const bool);

	//events
	void resizeEvent(QResizeEvent*);
	void contextMenuEvent(QContextMenuEvent*);
	void wheelEvent(QWheelEvent*);
	void mousePressEvent(QMouseEvent*);
	void mouseReleaseEvent(QMouseEvent*);
	void mouseMoveEvent(QMouseEvent*);

	Worksheet* m_worksheet;
	MouseMode m_mouseMode;
	bool m_selectionBandIsShown;
	QPoint m_selectionStart;
	QPoint m_selectionEnd;
	QList<QGraphicsItem*> m_selectedItems;
	bool m_suppressSelectionChangedEvent;

	QMenu* m_zoomMenu;
	QMenu* m_viewMouseModeMenu;
    QMenu* m_imageMenu;

	QToolButton* tbZoom;
	QAction* currentZoomAction;

    QAction* setReferencePointsAction;
    QAction* setCurvePointsAction;

	QAction* selectAllAction;
	QAction* deleteAction;
	QAction* backspaceAction;

	QAction* zoomInViewAction;
	QAction* zoomOutViewAction;
	QAction* zoomOriginAction;
	QAction* zoomFitPageHeightAction;
	QAction* zoomFitPageWidthAction;
	QAction* zoomFitSelectionAction;

	QAction* navigationModeAction;
	QAction* zoomSelectionModeAction;
	QAction* selectionModeAction;

    QAction* addImageAction;

  public slots:
	void createContextMenu(QMenu*) const;
	void fillToolBar(QToolBar*);
	void print(QPrinter*) const;
	void selectItem(QGraphicsItem*);

  private slots:
    void addNewImage();
    void aspectRemoved(const AbstractAspect* parent, const AbstractAspect* before, const AbstractAspect* child);
    void selectAllElements();
	void deleteElement();

    void setImageReferencePoints();
    void setImageCurvePoints();
    void handleImageActions();

	void mouseModeChanged(QAction*);
	void useViewSizeRequested();
	void changeZoom(QAction*);

	void deselectItem(QGraphicsItem*);
	void selectionChanged();
	void updateBackground();

  signals:
	void statusInfo(const QString&);
};

#endif
