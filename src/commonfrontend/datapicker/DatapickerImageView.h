/*
    File                 : DatapickerImageView.h
    Project              : LabPlot
    Description          : DatapickerImage view for datapicker
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2015 Ankit Wagadre (wagadre.ankit@gmail.com)

*/
/***************************************************************************
 *                                                                         *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *                                                                         *
 ***************************************************************************/

#ifndef DATAPICKERIMAGEVIEW_H
#define DATAPICKERIMAGEVIEW_H

#include "commonfrontend/worksheet/WorksheetView.h"

class AbstractAspect;
class DatapickerImage;
class Datapicker;
class Transform;

class QActionGroup;
class QMenu;
class QPrinter;
class QToolBar;
class QToolButton;
class QWheelEvent;

class DatapickerImageView : public QGraphicsView {
	Q_OBJECT

public:
	explicit DatapickerImageView(DatapickerImage*);
	~DatapickerImageView() override;

	void setScene(QGraphicsScene*);
	void exportToFile(const QString&, const WorksheetView::ExportFormat, const int);

private:
	enum class MouseMode {Navigation, ZoomSelection,
		ReferencePointsEntry, CurvePointsEntry, CurveSegmentsEntry};

	void initActions();
	void initMenus();
	void drawForeground(QPainter*, const QRectF&) override;
	void drawBackground(QPainter*, const QRectF&) override;
	void exportPaint(QPainter*, const QRectF& targetRect, const QRectF& sourceRect);
	void updateMagnificationWindow();

	//events
	void contextMenuEvent(QContextMenuEvent*) override;
	void wheelEvent(QWheelEvent*) override;
	void mousePressEvent(QMouseEvent*) override;
	void mouseReleaseEvent(QMouseEvent*) override;
	void mouseMoveEvent(QMouseEvent*) override;

	DatapickerImage* m_image;
	Datapicker* m_datapicker;
	Transform* m_transform;
	MouseMode m_mouseMode{MouseMode::ReferencePointsEntry};
	bool m_selectionBandIsShown{false};
	QPoint m_selectionStart;
	QPoint m_selectionEnd;
	int magnificationFactor{0};
	float m_rotationAngle{0.0};
	int m_numScheduledScalings{0};

	//Menus
	QMenu* m_zoomMenu;
	QMenu* m_viewMouseModeMenu;
	QMenu* m_viewImageMenu;
	QMenu* m_navigationMenu;
	QMenu* m_magnificationMenu;

	QToolButton* tbZoom{nullptr};
	QToolButton* tbMagnification{nullptr};
	QAction* currentZoomAction{nullptr};
	QAction* currentMagnificationAction{nullptr};
	QAction* currentPlotPointsTypeAction{nullptr};

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

	QAction* navigationModeAction;
	QAction* zoomSelectionModeAction;

	QActionGroup* navigationActionGroup;
	QAction* shiftLeftAction;
	QAction* shiftRightAction;
	QAction* shiftDownAction;
	QAction* shiftUpAction;

	QActionGroup* magnificationActionGroup;
	QAction* noMagnificationAction;
	QAction* twoTimesMagnificationAction;
	QAction* threeTimesMagnificationAction;
	QAction* fourTimesMagnificationAction;
	QAction* fiveTimesMagnificationAction;

public slots:
	void createContextMenu(QMenu*) const;
	void fillToolBar(QToolBar*);
	void print(QPrinter*);

private slots:
	void mouseModeChanged(QAction*);
	void magnificationChanged(QAction*);
	void changeZoom(QAction*);
	void changeSelectedItemsPosition(QAction*);
	void handleImageActions();
	void updateBackground();
	void addCurve();
	void changeRotationAngle();

	void zoom(int);
	void scalingTime();
	void animFinished();

signals:
	void statusInfo(const QString&);
};

#endif
