/*
	File                 : Transform.cpp
	Project              : LabPlot
	Description          : transformation for mapping between scene and
	logical coordinates of Datapicker-image
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2015 Ankit Wagadre <wagadre.ankit@gmail.com>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "Transform.h"

#include "backend/nsl/nsl_math.h"
#include <gsl/gsl_math.h>
// TODO: replace with GSL or better methods
#include <cmath>

// Transform::Transform() = default;

bool Transform::mapTypeToCartesian(const DatapickerImage::ReferencePoints& axisPoints) {
	switch (axisPoints.type) {
	case DatapickerImage::GraphType::LnX: {
		for (int i = 0; i < 3; ++i) {
			if (axisPoints.logicalPos[i].x() <= 0)
				return false;
			x[i] = log(axisPoints.logicalPos[i].x());
			y[i] = axisPoints.logicalPos[i].y();
		}
		break;
	}
	case DatapickerImage::GraphType::LnY: {
		for (int i = 0; i < 3; ++i) {
			if (axisPoints.logicalPos[i].y() <= 0)
				return false;
			x[i] = axisPoints.logicalPos[i].x();
			y[i] = log(axisPoints.logicalPos[i].y());
		}
		break;
	}
	case DatapickerImage::GraphType::LnXY: {
		for (int i = 0; i < 3; ++i) {
			if (axisPoints.logicalPos[i].x() <= 0)
				return false;
			x[i] = log(axisPoints.logicalPos[i].x());
			y[i] = log(axisPoints.logicalPos[i].y());
		}
		break;
	}
	case DatapickerImage::GraphType::Log10X: {
		for (int i = 0; i < 3; ++i) {
			if (axisPoints.logicalPos[i].x() <= 0)
				return false;
			x[i] = log10(axisPoints.logicalPos[i].x());
			y[i] = axisPoints.logicalPos[i].y();
		}
		break;
	}
	case DatapickerImage::GraphType::Log10Y: {
		for (int i = 0; i < 3; ++i) {
			if (axisPoints.logicalPos[i].y() <= 0)
				return false;
			x[i] = axisPoints.logicalPos[i].x();
			y[i] = log10(axisPoints.logicalPos[i].y());
		}
		break;
	}
	case DatapickerImage::GraphType::Log10XY: {
		for (int i = 0; i < 3; ++i) {
			if (axisPoints.logicalPos[i].x() <= 0)
				return false;
			x[i] = log10(axisPoints.logicalPos[i].x());
			y[i] = log10(axisPoints.logicalPos[i].y());
		}
		break;
	}
	case DatapickerImage::GraphType::PolarInDegree: {
		for (int i = 0; i < 3; ++i) {
			if (axisPoints.logicalPos[i].x() < 0)
				return false;
			x[i] = axisPoints.logicalPos[i].x() * cos(axisPoints.logicalPos[i].y() * M_PI_180);
			y[i] = axisPoints.logicalPos[i].x() * sin(axisPoints.logicalPos[i].y() * M_PI_180);
		}
		break;
	}
	case DatapickerImage::GraphType::PolarInRadians: {
		for (int i = 0; i < 3; ++i) {
			if (axisPoints.logicalPos[i].x() < 0)
				return false;
			x[i] = axisPoints.logicalPos[i].x() * cos(axisPoints.logicalPos[i].y());
			y[i] = axisPoints.logicalPos[i].x() * sin(axisPoints.logicalPos[i].y());
		}
		break;
	}
	case DatapickerImage::GraphType::Ternary: {
		for (int i = 0; i < 3; ++i) {
			x[i] = (2 * axisPoints.logicalPos[i].y() + axisPoints.logicalPos[i].z()) / (2 * axisPoints.ternaryScale);
			y[i] = (M_SQRT3 * axisPoints.logicalPos[i].z()) / (2 * axisPoints.ternaryScale);
		}
		break;
	}
	case DatapickerImage::GraphType::Linear: {
		for (int i = 0; i < 3; ++i) {
			x[i] = axisPoints.logicalPos[i].x();
			y[i] = axisPoints.logicalPos[i].y();
		}
		break;
	}
	}

	for (int i = 0; i < 3; i++) {
		X[i] = axisPoints.scenePos[i].x();
		Y[i] = axisPoints.scenePos[i].y();
	}

	return true;
}

Vector3D Transform::mapSceneToLogical(QPointF scenePoint, const DatapickerImage::ReferencePoints& axisPoints) {
	X[3] = scenePoint.x();
	Y[3] = scenePoint.y();

	if (mapTypeToCartesian(axisPoints)) {
		double sin;
		double cos;
		double scaleOfX;
		double scaleOfY;
		if (((Y[1] - Y[0]) * (x[2] - x[0]) - (Y[2] - Y[0]) * (x[1] - x[0])) != 0) {
			double tan = ((X[1] - X[0]) * (x[2] - x[0]) - (X[2] - X[0]) * (x[1] - x[0])) / ((Y[1] - Y[0]) * (x[2] - x[0]) - (Y[2] - Y[0]) * (x[1] - x[0]));
			sin = tan / sqrt(1 + tan * tan);
			cos = sqrt(1 - sin * sin);
		} else {
			sin = 1;
			cos = 0;
		}

		if ((x[1] - x[0]) != 0)
			scaleOfX = (x[1] - x[0]) / ((X[1] - X[0]) * cos - (Y[1] - Y[0]) * sin);
		else
			scaleOfX = (x[2] - x[0]) / ((X[2] - X[0]) * cos - (Y[2] - Y[0]) * sin);

		if ((y[1] - y[0]) != 0)
			scaleOfY = (y[1] - y[0]) / ((X[1] - X[0]) * sin + (Y[1] - Y[0]) * cos);
		else
			scaleOfY = (y[2] - y[0]) / ((X[2] - X[0]) * sin + (Y[2] - Y[0]) * cos);

		x[3] = x[0] + (((X[3] - X[0]) * cos - (Y[3] - Y[0]) * sin) * scaleOfX);
		y[3] = y[0] + (((X[3] - X[0]) * sin + (Y[3] - Y[0]) * cos) * scaleOfY);
		return mapCartesianToType(QPointF(x[3], y[3]), axisPoints);
	}
	return {};
}

Vector3D Transform::mapSceneLengthToLogical(QPointF errorSpan, const DatapickerImage::ReferencePoints& axisPoints) {
	return mapSceneToLogical(errorSpan, axisPoints) - mapSceneToLogical(QPointF(0, 0), axisPoints);
}

Vector3D Transform::mapCartesianToType(QPointF point, const DatapickerImage::ReferencePoints& axisPoints) const {
	switch (axisPoints.type) {
	case DatapickerImage::GraphType::Linear:
		return Vector3D(point.x(), point.y(), 0);
	case DatapickerImage::GraphType::LnXY:
		return Vector3D(exp(point.x()), exp(point.y()), 0);
	case DatapickerImage::GraphType::LnX:
		return Vector3D(exp(point.x()), point.y(), 0);
	case DatapickerImage::GraphType::LnY:
		return Vector3D(point.x(), exp(point.y()), 0);
	case DatapickerImage::GraphType::Log10XY:
		return Vector3D(pow(10, point.x()), pow(10, point.y()), 0);
	case DatapickerImage::GraphType::Log10X:
		return Vector3D(pow(10, point.x()), point.y(), 0);
	case DatapickerImage::GraphType::Log10Y:
		return Vector3D(point.x(), pow(10, point.y()), 0);
	case DatapickerImage::GraphType::PolarInDegree: {
		double r = sqrt(point.x() * point.x() + point.y() * point.y());
		double angle = atan(point.y() / point.x()) * M_180_PI;
		return Vector3D(r, angle, 0);
	}
	case DatapickerImage::GraphType::PolarInRadians: {
		double r = sqrt(point.x() * point.x() + point.y() * point.y());
		double angle = atan(point.y() / point.x());
		return Vector3D(r, angle, 0);
	}
	case DatapickerImage::GraphType::Ternary: {
		double c = (point.y() * 2 * axisPoints.ternaryScale) / M_SQRT3;
		double b = (point.x() * 2 * axisPoints.ternaryScale - c) / 2;
		double a = axisPoints.ternaryScale - b - c;
		return Vector3D(a, b, c);
	}
	}
	return Vector3D(point.x(), point.y(), 0); // should never happen
}
