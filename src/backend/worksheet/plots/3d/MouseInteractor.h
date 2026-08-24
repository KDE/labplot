/***************************************************************************
	File                 : MouseInteractor.h
	Project              : LabPlot
	Description          : 3D Mouse Interactor
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2024-2026 Alexander Semke <alexander.semke@web.de>
	SPDX-FileCopyrightText: 2024 Kuntal Bar <barkuntal6@gmail.com>
	SPDX-License-Identifier: GPL-2.0-or-later
***************************************************************************/

#ifndef MOUSEINTERACTOR_H
#define MOUSEINTERACTOR_H

#include <QtDataVisualization/QAbstract3DInputHandler>

class MouseInteractor : public QAbstract3DInputHandler {
	Q_OBJECT
public:
	MouseInteractor(QObject* parent = nullptr);

	void mousePressEvent(QMouseEvent*, const QPoint&) override;
	void mouseReleaseEvent(QMouseEvent*, const QPoint&) override;
	void mouseMoveEvent(QMouseEvent*, const QPoint&) override;
	void wheelEvent(QWheelEvent*) override;

private:
	QPoint mousePoint;
	bool mouseRotation;
	float zoomFactor;
	float xRotation;
	float yRotation;
	static const int deltaZoom;
};

#endif // MOUSEINTERACTOR_H
