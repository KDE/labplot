/*
	File                 : DatapickerImageView.h
	Project              : LabPlot
	Description          : DatapickerImage view for datapicker
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2015 Ankit Wagadre <wagadre.ankit@gmail.com>
	SPDX-FileCopyrightText: 2015-2024 Alexander Semke <alexander.semke@web.de>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef DATAPICKERIMAGEVIEW_H
#define DATAPICKERIMAGEVIEW_H

#include "backend/worksheet/Worksheet.h"
#include <QGraphicsView>

class DatapickerImage;
class Datapicker;
class ToggleActionMenu;
class Transform;

class QActionGroup;
class QMenu;
class QPrinter;
class QWheelEvent;

class DatapickerImageView : public QGraphicsView {
	Q_OBJECT

public:
	explicit DatapickerImageView(DatapickerImage*);
	~DatapickerImageView() override;

	enum class ZoomMode { ZoomIn, ZoomOut, ZoomOrigin, ZoomFitPageWidth, ZoomFitPageHeight };
	enum class MouseMode { Navigation, ZoomSelection, ReferencePointsEntry, CurvePointsEntry, CurveSegmentsEntry };
	enum class ShiftOperation { ShiftLeft, ShiftRight, ShiftUp, ShiftDown };

	void setScene(QGraphicsScene*);
	bool exportToFile(const QString&, const Worksheet::ExportFormat, const int);

	MouseMode mouseMode() const;
	ZoomMode zoomMode() const;
	int magnification() const;

	void fillZoomMenu(ToggleActionMenu*) const;
	void fillMagnificationMenu(ToggleActionMenu*) const;

private:
	void initActions();
	void initMenus();
	void drawForeground(QPainter*, const QRectF&) override;
	void drawBackground(QPainter*, const QRectF&) override;
	void exportPaint(QPainter*, const QRectF& targetRect, const QRectF& sourceRect);
	void updateMagnificationWindow();

	// events
	void contextMenuEvent(QContextMenuEvent*) override;
	void keyPressEvent(QKeyEvent* event) override;
	void wheelEvent(QWheelEvent*) override;
	void mousePressEvent(QMouseEvent*) override;
	void mouseReleaseEvent(QMouseEvent*) override;
	void mouseMoveEvent(QMouseEvent*) override;

	DatapickerImage* m_image{nullptr};
	Datapicker* m_datapicker{nullptr};
	Transform* m_transform{nullptr};
	MouseMode m_mouseMode{MouseMode::ReferencePointsEntry};
	ZoomMode m_zoomMode{ZoomMode::ZoomIn};
	bool m_selectionBandIsShown{false};
	QPoint m_selectionStart;
	QPoint m_selectionEnd;
	int m_magnificationFactor{0};
	float m_rotationAngle{0.0};
	int m_numScheduledScalings{0};

	// Menus
	QMenu* m_zoomMenu{nullptr};
	QMenu* m_viewMouseModeMenu{nullptr};
	QMenu* m_viewImageMenu{nullptr};
	QMenu* m_navigationMenu{nullptr};
	QMenu* m_magnificationMenu{nullptr};

	// Actions
	QAction* zoomInViewAction{nullptr};
	QAction* zoomOutViewAction{nullptr};
	QAction* zoomOriginAction{nullptr};
	QAction* zoomFitPageHeightAction{nullptr};
	QAction* zoomFitPageWidthAction{nullptr};

	QAction* setAxisPointsAction{nullptr};
	QAction* setCurvePointsAction{nullptr};
	QAction* selectSegmentAction{nullptr};

	QAction* addCurveAction{nullptr};
	QAction* currentPlotPointsTypeAction{nullptr};

	QAction* navigationModeAction{nullptr};
	QAction* zoomSelectionModeAction{nullptr};

	QActionGroup* navigationActionGroup{nullptr};
	QAction* shiftLeftAction{nullptr};
	QAction* shiftRightAction{nullptr};
	QAction* shiftDownAction{nullptr};
	QAction* shiftUpAction{nullptr};

	QActionGroup* magnificationActionGroup{nullptr};

public Q_SLOTS:
	void createContextMenu(QMenu*) const;
	void print(QPrinter*);

	void changeMouseMode(QAction*);
	void changeMagnification(QAction*);
	void changeZoom(QAction*);
	void changeSelectedItemsPosition(QAction*);
	void addCurve();

private Q_SLOTS:
	void handleImageActions();
	void updateBackground();
	void changeRotationAngle();

	void zoom(int);
	void scalingTime();
	void animFinished();

Q_SIGNALS:
	void statusInfo(const QString&);

	friend class DatapickerTest;
};

#endif
