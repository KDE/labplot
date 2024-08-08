#include "MouseInteractor.h"

#include <QtGraphs>

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
		xRotation = cameraXRotation();
		yRotation = cameraYRotation();
		mouseRotation = true;
	} else if (event->button() == Qt::MouseButton::RightButton) {
		Q_EMIT showContextMenu();
	}
	Q_EMIT activateParentWindow();
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
		setCameraXRotation(xrotation);
		setCameraYRotation(yrotation);
		setCameraZoomLevel(zoomFactor);
	}
	QAbstract3DInputHandler::mouseMoveEvent(event, mousePos);
}

void MouseInteractor::wheelEvent(QWheelEvent* event) {
	zoomFactor = cameraZoomLevel();
	(event->angleDelta().y() > 0) ? zoomFactor += deltaZoom : zoomFactor -= deltaZoom;
	setCameraZoomLevel(zoomFactor);
	QAbstract3DInputHandler::wheelEvent(event);
}
