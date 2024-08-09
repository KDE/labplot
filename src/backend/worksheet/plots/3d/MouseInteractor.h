#ifndef MOUSEINTERACTER_H
#define MOUSEINTERACTER_H
#include <QtGraphs>

class MouseInteractor : public QAbstract3DInputHandler {
	Q_OBJECT
public:
	MouseInteractor(QObject* parent = nullptr);

	virtual void mousePressEvent(QMouseEvent* event, const QPoint& mousePos) override;
	virtual void mouseReleaseEvent(QMouseEvent* event, const QPoint& mousePos) override;
	virtual void mouseMoveEvent(QMouseEvent* event, const QPoint& mousePos) override;
	virtual void wheelEvent(QWheelEvent* event) override;

Q_SIGNALS:
	void showContextMenu();
	void activateParentWindow();

private:
	QPoint mousePoint;
	bool mouseRotation;
	float zoomFactor;
	float xRotation;
	float yRotation;
	static const int deltaZoom;
};
#endif // MOUSEINTERACTER_H
