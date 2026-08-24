/***************************************************************************
	File                 : MouseInteractor.h
	Project              : LabPlot
	Description          : 3D Mouse Interactor
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2024-2026 Alexander Semke <alexander.semke@web.de>
	SPDX-FileCopyrightText: 2024 Kuntal Bar <barkuntal6@gmail.com>
	SPDX-License-Identifier: GPL-2.0-or-later
***************************************************************************/

#include "MouseInteractor.h"

#include <QMouseEvent>
#include <QWheelEvent>
#include <QtDataVisualization/Q3DCamera>
#include <QtDataVisualization/Q3DScene>

const int MouseInteractor::deltaZoom = 1;

MouseInteractor::MouseInteractor(QObject* parent)
	: QAbstract3DInputHandler(parent)
	, mouseRotation(false)
	, zoomFactor(100)
	, xRotation(40)
	, yRotation(30) {
}

void MouseInteractor::mousePressEvent(QMouseEvent* event, const QPoint& mousePos) {
	if (event->button() == Qt::MouseButton::LeftButton) {
		mousePoint = mousePos;
		xRotation = scene()->activeCamera()->xRotation();
		yRotation = scene()->activeCamera()->yRotation();
		mouseRotation = true;
	} else if (event->button() == Qt::MouseButton::RightButton)
		mouseRotation = false;

	QAbstract3DInputHandler::mousePressEvent(event, mousePos);
}

void MouseInteractor::mouseReleaseEvent(QMouseEvent* event, const QPoint& mousePos) {
	if (event->button() == Qt::MouseButton::LeftButton) {
		mouseRotation = false;
	}
	QAbstract3DInputHandler::mouseReleaseEvent(event, mousePos);
}

void MouseInteractor::mouseMoveEvent(QMouseEvent* event, const QPoint& mousePos) {
	if (mouseRotation) {
		QPoint point = mousePos - mousePoint;
		int xrotation = xRotation + point.x();
		int yrotation = yRotation + point.y();
		scene()->activeCamera()->setXRotation(xrotation);
		scene()->activeCamera()->setYRotation(yrotation);
		scene()->activeCamera()->setZoomLevel(zoomFactor);
	}
	QAbstract3DInputHandler::mouseMoveEvent(event, mousePos);
}

void MouseInteractor::wheelEvent(QWheelEvent* event) {
	zoomFactor = scene()->activeCamera()->zoomLevel();
	(event->angleDelta().y() > 0) ? zoomFactor += deltaZoom : zoomFactor -= deltaZoom;
	scene()->activeCamera()->setZoomLevel(zoomFactor);
	QAbstract3DInputHandler::wheelEvent(event);
}
