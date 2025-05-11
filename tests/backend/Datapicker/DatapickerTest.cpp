/*
	File                 : DatapickerTest.cpp
	Project              : LabPlot
	Description          : Tests for Datapicker
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2022 Martin Marmsoler <martin.marmsoler@gmail.com>
	SPDX-FileCopyrightText: 2022 Alexander Semke <alexander.semke@web.de>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "DatapickerTest.h"
#include "backend/core/Project.h"
#include "backend/core/column/Column.h"
#include "backend/datapicker/Datapicker.h"
#include "backend/datapicker/DatapickerCurve.h"
#include "backend/datapicker/DatapickerCurvePrivate.h"
#include "backend/datapicker/DatapickerImage.h"
#include "backend/datapicker/DatapickerPoint.h"
#include "backend/datapicker/DatapickerPointPrivate.h"
#include "backend/datapicker/Transform.h"
#include "backend/spreadsheet/Spreadsheet.h"
#include "frontend/datapicker/DatapickerImageView.h"
#include "frontend/widgets/DatapickerImageWidget.h"

#include <QAction>
#include "backend/lib/UndoStack.h"

#define VECTOR3D_EQUAL(vec, ref)                                                                                                                               \
	VALUES_EQUAL(vec.x(), ref.x());                                                                                                                            \
	VALUES_EQUAL(vec.y(), ref.y());                                                                                                                            \
	VALUES_EQUAL(vec.z(), ref.z());

namespace {
QImage createImage() {
	QImage image(1000, 1000, QImage::Format_RGB32);
	QRgb value;

	value = qRgb(189, 149, 39); // 0xffbd9527
	image.setPixel(1, 1, value);

	value = qRgb(122, 163, 39); // 0xff7aa327
	image.setPixel(0, 1, value);
	image.setPixel(1, 0, value);

	value = qRgb(237, 187, 51); // 0xffedba31
	image.setPixel(2, 1, value);
	return image;
}

// Different image to the one created by createImage()
QImage createImage2() {
	QImage image(3, 3, QImage::Format_RGB32);
	QRgb value;

	value = qRgb(189, 149, 39); // 0xffbd9527
	image.setPixel(0, 1, value);

	value = qRgb(122, 163, 39); // 0xff7aa327
	image.setPixel(1, 1, value);
	image.setPixel(1, 0, value);

	value = qRgb(237, 187, 51); // 0xffedba31
	image.setPixel(2, 1, value);
	return image;
}
} // anonymous namespace

void DatapickerTest::mapCartesianToCartesian() {
	DatapickerImage::ReferencePoints points;
	points.type = DatapickerImage::GraphType::Linear;
	points.logicalPos[0].setX(1);
	points.logicalPos[0].setY(2);
	points.logicalPos[1].setX(3);
	points.logicalPos[1].setY(4);
	points.logicalPos[2].setX(5);
	points.logicalPos[2].setY(6);
	points.scenePos[0].setX(6.21);
	points.scenePos[0].setY(7.23);
	points.scenePos[1].setX(-51.2);
	points.scenePos[1].setY(3234);
	points.scenePos[2].setX(-23);
	points.scenePos[2].setY(+5e6);

	Transform t;
	QCOMPARE(t.mapTypeToCartesian(points), true);
	VALUES_EQUAL(t.x[0], 1.);
	VALUES_EQUAL(t.y[0], 2.);
	VALUES_EQUAL(t.x[1], 3.);
	VALUES_EQUAL(t.y[1], 4.);
	VALUES_EQUAL(t.x[2], 5.);
	VALUES_EQUAL(t.y[2], 6.);
	VALUES_EQUAL(t.X[0], 6.21);
	VALUES_EQUAL(t.Y[0], 7.23);
	VALUES_EQUAL(t.X[1], -51.2);
	VALUES_EQUAL(t.Y[1], 3234.);
	VALUES_EQUAL(t.X[2], -23.);
	VALUES_EQUAL(t.Y[2], +5.e6);
}

void DatapickerTest::maplnXToCartesian() {
	DatapickerImage::ReferencePoints points;
	points.type = DatapickerImage::GraphType::LnX;
	points.logicalPos[0].setX(exp(1));
	points.logicalPos[0].setY(2);
	points.logicalPos[1].setX(exp(2));
	points.logicalPos[1].setY(4);
	points.logicalPos[2].setX(exp(3));
	points.logicalPos[2].setY(6);
	points.scenePos[0].setX(6.21);
	points.scenePos[0].setY(7.23);
	points.scenePos[1].setX(-51.2);
	points.scenePos[1].setY(3234);
	points.scenePos[2].setX(-23);
	points.scenePos[2].setY(+5e6);

	Transform t;
	QCOMPARE(t.mapTypeToCartesian(points), true);
	VALUES_EQUAL(t.x[0], 1.);
	VALUES_EQUAL(t.y[0], 2.);
	VALUES_EQUAL(t.x[1], 2.);
	VALUES_EQUAL(t.y[1], 4.);
	VALUES_EQUAL(t.x[2], 3.);
	VALUES_EQUAL(t.y[2], 6.);
	VALUES_EQUAL(t.X[0], 6.21);
	VALUES_EQUAL(t.Y[0], 7.23);
	VALUES_EQUAL(t.X[1], -51.2);
	VALUES_EQUAL(t.Y[1], 3234.);
	VALUES_EQUAL(t.X[2], -23.);
	VALUES_EQUAL(t.Y[2], +5.e6);
}

void DatapickerTest::maplnYToCartesian() {
	DatapickerImage::ReferencePoints points;
	points.type = DatapickerImage::GraphType::LnY;
	points.logicalPos[0].setX(1);
	points.logicalPos[0].setY(exp(1));
	points.logicalPos[1].setX(3);
	points.logicalPos[1].setY(exp(2));
	points.logicalPos[2].setX(5);
	points.logicalPos[2].setY(exp(3));
	points.scenePos[0].setX(6.21);
	points.scenePos[0].setY(7.23);
	points.scenePos[1].setX(-51.2);
	points.scenePos[1].setY(3234);
	points.scenePos[2].setX(-23);
	points.scenePos[2].setY(+5e6);

	Transform t;
	QCOMPARE(t.mapTypeToCartesian(points), true);
	VALUES_EQUAL(t.x[0], 1.);
	VALUES_EQUAL(t.y[0], 1.);
	VALUES_EQUAL(t.x[1], 3.);
	VALUES_EQUAL(t.y[1], 2.);
	VALUES_EQUAL(t.x[2], 5.);
	VALUES_EQUAL(t.y[2], 3.);
	VALUES_EQUAL(t.X[0], 6.21);
	VALUES_EQUAL(t.Y[0], 7.23);
	VALUES_EQUAL(t.X[1], -51.2);
	VALUES_EQUAL(t.Y[1], 3234.);
	VALUES_EQUAL(t.X[2], -23.);
	VALUES_EQUAL(t.Y[2], +5.e6);
}

void DatapickerTest::maplnXYToCartesian() {
	DatapickerImage::ReferencePoints points;
	points.type = DatapickerImage::GraphType::LnXY;
	points.logicalPos[0].setX(exp(5));
	points.logicalPos[0].setY(exp(1));
	points.logicalPos[1].setX(exp(6));
	points.logicalPos[1].setY(exp(2));
	points.logicalPos[2].setX(exp(7));
	points.logicalPos[2].setY(exp(3));
	points.scenePos[0].setX(6.21);
	points.scenePos[0].setY(7.23);
	points.scenePos[1].setX(-51.2);
	points.scenePos[1].setY(3234);
	points.scenePos[2].setX(-23);
	points.scenePos[2].setY(+5e6);

	Transform t;
	QCOMPARE(t.mapTypeToCartesian(points), true);
	VALUES_EQUAL(t.x[0], 5.);
	VALUES_EQUAL(t.y[0], 1.);
	VALUES_EQUAL(t.x[1], 6.);
	VALUES_EQUAL(t.y[1], 2.);
	VALUES_EQUAL(t.x[2], 7.);
	VALUES_EQUAL(t.y[2], 3.);
	VALUES_EQUAL(t.X[0], 6.21);
	VALUES_EQUAL(t.Y[0], 7.23);
	VALUES_EQUAL(t.X[1], -51.2);
	VALUES_EQUAL(t.Y[1], 3234.);
	VALUES_EQUAL(t.X[2], -23.);
	VALUES_EQUAL(t.Y[2], +5.e6);
}

void DatapickerTest::maplog10XToCartesian() {
	DatapickerImage::ReferencePoints points;
	points.type = DatapickerImage::GraphType::Log10X;
	points.logicalPos[0].setX(pow(10, 1));
	points.logicalPos[0].setY(2);
	points.logicalPos[1].setX(pow(10, 2));
	points.logicalPos[1].setY(4);
	points.logicalPos[2].setX(pow(10, 3));
	points.logicalPos[2].setY(6);
	points.scenePos[0].setX(6.21);
	points.scenePos[0].setY(7.23);
	points.scenePos[1].setX(-51.2);
	points.scenePos[1].setY(3234);
	points.scenePos[2].setX(-23);
	points.scenePos[2].setY(+5e6);

	Transform t;
	QCOMPARE(t.mapTypeToCartesian(points), true);
	VALUES_EQUAL(t.x[0], 1.);
	VALUES_EQUAL(t.y[0], 2.);
	VALUES_EQUAL(t.x[1], 2.);
	VALUES_EQUAL(t.y[1], 4.);
	VALUES_EQUAL(t.x[2], 3.);
	VALUES_EQUAL(t.y[2], 6.);
	VALUES_EQUAL(t.X[0], 6.21);
	VALUES_EQUAL(t.Y[0], 7.23);
	VALUES_EQUAL(t.X[1], -51.2);
	VALUES_EQUAL(t.Y[1], 3234.);
	VALUES_EQUAL(t.X[2], -23.);
	VALUES_EQUAL(t.Y[2], +5.e6);
}

void DatapickerTest::maplog10YToCartesian() {
	DatapickerImage::ReferencePoints points;
	points.type = DatapickerImage::GraphType::Log10Y;
	points.logicalPos[0].setX(1);
	points.logicalPos[0].setY(pow(10, 1));
	points.logicalPos[1].setX(3);
	points.logicalPos[1].setY(pow(10, 2));
	points.logicalPos[2].setX(5);
	points.logicalPos[2].setY(pow(10, 3));
	points.scenePos[0].setX(6.21);
	points.scenePos[0].setY(7.23);
	points.scenePos[1].setX(-51.2);
	points.scenePos[1].setY(3234);
	points.scenePos[2].setX(-23);
	points.scenePos[2].setY(+5e6);

	Transform t;
	QCOMPARE(t.mapTypeToCartesian(points), true);
	VALUES_EQUAL(t.x[0], 1.);
	VALUES_EQUAL(t.y[0], 1.);
	VALUES_EQUAL(t.x[1], 3.);
	VALUES_EQUAL(t.y[1], 2.);
	VALUES_EQUAL(t.x[2], 5.);
	VALUES_EQUAL(t.y[2], 3.);
	VALUES_EQUAL(t.X[0], 6.21);
	VALUES_EQUAL(t.Y[0], 7.23);
	VALUES_EQUAL(t.X[1], -51.2);
	VALUES_EQUAL(t.Y[1], 3234.);
	VALUES_EQUAL(t.X[2], -23.);
	VALUES_EQUAL(t.Y[2], +5.e6);
}

void DatapickerTest::maplog10XYToCartesian() {
	DatapickerImage::ReferencePoints points;
	points.type = DatapickerImage::GraphType::Log10XY;
	points.logicalPos[0].setX(pow(10, 5));
	points.logicalPos[0].setY(pow(10, 1));
	points.logicalPos[1].setX(pow(10, 6));
	points.logicalPos[1].setY(pow(10, 2));
	points.logicalPos[2].setX(pow(10, 7));
	points.logicalPos[2].setY(pow(10, 3));
	points.scenePos[0].setX(6.21);
	points.scenePos[0].setY(7.23);
	points.scenePos[1].setX(-51.2);
	points.scenePos[1].setY(3234);
	points.scenePos[2].setX(-23);
	points.scenePos[2].setY(+5e6);

	Transform t;
	QCOMPARE(t.mapTypeToCartesian(points), true);
	VALUES_EQUAL(t.x[0], 5.);
	VALUES_EQUAL(t.y[0], 1.);
	VALUES_EQUAL(t.x[1], 6.);
	VALUES_EQUAL(t.y[1], 2.);
	VALUES_EQUAL(t.x[2], 7.);
	VALUES_EQUAL(t.y[2], 3.);
	VALUES_EQUAL(t.X[0], 6.21);
	VALUES_EQUAL(t.Y[0], 7.23);
	VALUES_EQUAL(t.X[1], -51.2);
	VALUES_EQUAL(t.Y[1], 3234.);
	VALUES_EQUAL(t.X[2], -23.);
	VALUES_EQUAL(t.Y[2], +5.e6);
}

void DatapickerTest::mapPolarInRadiansToCartesian() {
	DatapickerImage::ReferencePoints points;
	points.type = DatapickerImage::GraphType::PolarInRadians;
	points.logicalPos[0].setX(1);
	points.logicalPos[0].setY(0);
	points.logicalPos[1].setX(3);
	points.logicalPos[1].setY(2.1f);
	points.logicalPos[2].setX(5);
	points.logicalPos[2].setY(3.8f);
	points.scenePos[0].setX(6.21);
	points.scenePos[0].setY(7.23);
	points.scenePos[1].setX(-51.2);
	points.scenePos[1].setY(3234);
	points.scenePos[2].setX(-23);
	points.scenePos[2].setY(+5e6);

	Transform t;
	QCOMPARE(t.mapTypeToCartesian(points), true);
	VALUES_EQUAL(t.x[0], 1.);
	VALUES_EQUAL(t.y[0], 0.);

	// TODO: VALUES_EQUAL doesn't work for the next comparisons since the precision 1.e-7 is too high for the numerical error involved here.
	// to improve the precision we need to get rid of QVector3D using floats internally and to switch to doubles. For now, just use a somewhat
	// lower precision here since it's not essential.
	// VALUES_EQUAL(t.x[1], -1.5145383137996); // precision seems to be not correct. Reference value is correct
	double v1 = t.x[1];
	double ref = -1.5145383137996;
	QVERIFY2(nsl_math_approximately_equal_eps(v1, ref, 1.e-5) == true,
			 qPrintable(QStringLiteral("v1:%1, ref:%2").arg(v1, 0, 'g', 15, QLatin1Char(' ')).arg(ref, 0, 'g', 15, QLatin1Char(' '))));

	VALUES_EQUAL(t.y[1], 2.5896280999466);
	VALUES_EQUAL(t.x[2], -3.9548385595721);
	VALUES_EQUAL(t.y[2], -3.0592894547136);
	VALUES_EQUAL(t.X[0], 6.21);
	VALUES_EQUAL(t.Y[0], 7.23);
	VALUES_EQUAL(t.X[1], -51.2);
	VALUES_EQUAL(t.Y[1], 3234.);
	VALUES_EQUAL(t.X[2], -23.);
	VALUES_EQUAL(t.Y[2], +5.e6);
}

void DatapickerTest::mapPolarInDegreeToCartesian() {
	DatapickerImage::ReferencePoints points;
	points.type = DatapickerImage::GraphType::PolarInDegree;
	points.logicalPos[0].setX(1);
	points.logicalPos[0].setY(0);
	points.logicalPos[1].setX(3);
	points.logicalPos[1].setY(30);
	points.logicalPos[2].setX(5);
	points.logicalPos[2].setY(50);
	points.scenePos[0].setX(6.21);
	points.scenePos[0].setY(7.23);
	points.scenePos[1].setX(-51.2);
	points.scenePos[1].setY(3234);
	points.scenePos[2].setX(-23);
	points.scenePos[2].setY(+5e6);

	Transform t;
	QCOMPARE(t.mapTypeToCartesian(points), true);
	VALUES_EQUAL(t.x[0], 1.);
	VALUES_EQUAL(t.y[0], 0.);
	VALUES_EQUAL(t.x[1], 2.5980762113533);
	VALUES_EQUAL(t.y[1], 1.5);
	VALUES_EQUAL(t.x[2], 3.2139380484327);
	VALUES_EQUAL(t.y[2], 3.8302222155949);
	VALUES_EQUAL(t.X[0], 6.21);
	VALUES_EQUAL(t.Y[0], 7.23);
	VALUES_EQUAL(t.X[1], -51.2);
	VALUES_EQUAL(t.Y[1], 3234.);
	VALUES_EQUAL(t.X[2], -23.);
	VALUES_EQUAL(t.Y[2], +5.e6);
}

// TODO: implement ternary

// Reference calculations done with https://keisan.casio.com/exec/system/1223526375

void DatapickerTest::mapCartesianToLinear() {
	DatapickerImage::ReferencePoints points;
	points.type = DatapickerImage::GraphType::Linear;
	QPointF point{5, 2343.23};

	Transform t;
	VECTOR3D_EQUAL(t.mapCartesianToType(point, points), QVector3D(5, 2343.23f, 0));
}

void DatapickerTest::mapCartesianToLnX() {
	DatapickerImage::ReferencePoints points;
	points.type = DatapickerImage::GraphType::LnX;
	QPointF point{5, 2343.23};

	Transform t;
	VECTOR3D_EQUAL(t.mapCartesianToType(point, points), QVector3D(exp(5), 2343.23f, 0));
}

void DatapickerTest::mapCartesianToLnY() {
	DatapickerImage::ReferencePoints points;
	points.type = DatapickerImage::GraphType::LnY;
	QPointF point{5, 2.23};

	Transform t;
	VECTOR3D_EQUAL(t.mapCartesianToType(point, points), QVector3D(5, exp(2.23), 0));
}

void DatapickerTest::mapCartesianToLnXY() {
	DatapickerImage::ReferencePoints points;
	points.type = DatapickerImage::GraphType::LnXY;
	QPointF point{5, 2.23};

	Transform t;
	VECTOR3D_EQUAL(t.mapCartesianToType(point, points), QVector3D(exp(5), exp(2.23), 0));
}

void DatapickerTest::mapCartesianToLog10X() {
	DatapickerImage::ReferencePoints points;
	points.type = DatapickerImage::GraphType::Log10X;
	QPointF point{5, 2343.23};

	Transform t;
	VECTOR3D_EQUAL(t.mapCartesianToType(point, points), QVector3D(pow(10, 5), 2343.23f, 0));
}

void DatapickerTest::mapCartesianToLog10Y() {
	DatapickerImage::ReferencePoints points;
	points.type = DatapickerImage::GraphType::Log10Y;
	QPointF point{5, 2.23};

	Transform t;
	VECTOR3D_EQUAL(t.mapCartesianToType(point, points), QVector3D(5, pow(10, 2.23), 0));
}

void DatapickerTest::mapCartesianToLog10XY() {
	DatapickerImage::ReferencePoints points;
	points.type = DatapickerImage::GraphType::Log10XY;
	QPointF point{5, 2.23};

	Transform t;
	VECTOR3D_EQUAL(t.mapCartesianToType(point, points), QVector3D(pow(10, 5), pow(10, 2.23), 0));
}

void DatapickerTest::mapCartesianToPolarInDegree() {
	DatapickerImage::ReferencePoints points;
	points.type = DatapickerImage::GraphType::PolarInDegree;
	QPointF point{5, 30};

	Transform t;
	VECTOR3D_EQUAL(t.mapCartesianToType(point, points), QVector3D(30.413812651491f, 80.537677791974f, 0));
}

void DatapickerTest::mapCartesianToPolarInRadians() {
	DatapickerImage::ReferencePoints points;
	points.type = DatapickerImage::GraphType::PolarInRadians;
	QPointF point{5, 30};

	Transform t;
	VECTOR3D_EQUAL(t.mapCartesianToType(point, points), QVector3D(30.413812651491f, 1.4056476493803f, 0)); // first is radius, second theta
}

// TODO: implement Ternary

// Test if setting curve points on the image result in correct values for Linear mapping
void DatapickerTest::linearMapping() {
	Datapicker datapicker(QStringLiteral("Test"));
	auto* image = datapicker.image();

	// Set reference points
	datapicker.addNewPoint(QPointF(0, 1), image);
	datapicker.addNewPoint(QPointF(0, 0), image);
	datapicker.addNewPoint(QPointF(1, 0), image);

	auto ap = image->axisPoints();
	ap.type = DatapickerImage::GraphType::Linear;
	image->setAxisPoints(ap);

	DatapickerImageWidget w(nullptr);
	w.setImages({image});
	w.ui.sbPositionX1->setValue(0);
	w.ui.sbPositionY1->setValue(10);
	w.ui.sbPositionZ1->setValue(0);
	w.ui.sbPositionX2->setValue(0);
	w.ui.sbPositionY2->setValue(0);
	w.ui.sbPositionZ2->setValue(0);
	w.ui.sbPositionX3->setValue(10);
	w.ui.sbPositionY3->setValue(0);
	w.ui.sbPositionZ3->setValue(0);
	w.logicalPositionChanged();

	auto* curve = new DatapickerCurve(i18n("Curve"));
	curve->addDatasheet(image->axisPoints().type);
	datapicker.addChild(curve);

	datapicker.addNewPoint(QPointF(0.5, 0.5), curve); // updates the curve data
	VALUES_EQUAL(curve->posXColumn()->valueAt(0), 5.);
	VALUES_EQUAL(curve->posYColumn()->valueAt(0), 5.);

	datapicker.addNewPoint(QPointF(0.7, 0.65), curve); // updates the curve data
	VALUES_EQUAL(curve->posXColumn()->valueAt(1), 7.);
	VALUES_EQUAL(curve->posYColumn()->valueAt(1), 6.5);
}

// Test if setting curve points on the image result in correct values for lnX mapping
void DatapickerTest::logarithmicNaturalXMapping() {
	Datapicker datapicker(QStringLiteral("Test"));
	auto* image = datapicker.image();

	// Set reference points
	datapicker.addNewPoint(QPointF(3, 10), image);
	datapicker.addNewPoint(QPointF(3, 0), image);
	datapicker.addNewPoint(QPointF(13, 0), image);

	auto ap = image->axisPoints();
	ap.type = DatapickerImage::GraphType::LnX;
	image->setAxisPoints(ap);

	DatapickerImageWidget w(nullptr);
	w.setImages({image});
	w.ui.sbPositionX1->setValue(0);
	w.ui.sbPositionY1->setValue(100);
	w.ui.sbPositionZ1->setValue(0);
	w.ui.sbPositionX2->setValue(0);
	w.ui.sbPositionY2->setValue(0);
	w.ui.sbPositionZ2->setValue(0);
	w.ui.sbPositionX3->setValue(100);
	w.ui.sbPositionY3->setValue(0);
	w.ui.sbPositionZ3->setValue(0);
	w.logicalPositionChanged();

	auto* curve = new DatapickerCurve(i18n("Curve"));
	curve->addDatasheet(image->axisPoints().type);
	datapicker.addChild(curve);

	datapicker.addNewPoint(QPointF(5, 6), curve); // updates the curve data
	VALUES_EQUAL(curve->posXColumn()->valueAt(0), 0.); // x start is zero, which is not valid therefore the result is 0
	VALUES_EQUAL(curve->posYColumn()->valueAt(0), 0.); // x start is zero, which is not valid therefore the result is 0

	QCOMPARE(w.ui.sbPositionX1->setValue(1), true);
	QCOMPARE(w.ui.sbPositionX2->setValue(1), true);
	w.ui.sbPositionX1->valueChanged(1); // axisPointsChanged will call updatePoint()

	// Value validated manually, not reverse calculated
	VALUES_EQUAL(curve->posXColumn()->valueAt(0), 2.51188635826);
	VALUES_EQUAL(curve->posYColumn()->valueAt(0), 60.);
}

// Test if setting curve points on the image result in correct values for lnY mapping
void DatapickerTest::logarithmicNaturalYMapping() {
	Datapicker datapicker(QStringLiteral("Test"));
	auto* image = datapicker.image();

	// Set reference points
	datapicker.addNewPoint(QPointF(3, 10), image);
	datapicker.addNewPoint(QPointF(3, 0), image);
	datapicker.addNewPoint(QPointF(13, 0), image);

	auto ap = image->axisPoints();
	ap.type = DatapickerImage::GraphType::LnY;
	image->setAxisPoints(ap);

	DatapickerImageWidget w(nullptr);
	w.setImages({image});
	w.ui.sbPositionX1->setValue(0);
	w.ui.sbPositionY1->setValue(100);
	w.ui.sbPositionZ1->setValue(0);
	w.ui.sbPositionX2->setValue(0);
	w.ui.sbPositionY2->setValue(0);
	w.ui.sbPositionZ2->setValue(0);
	w.ui.sbPositionX3->setValue(100);
	w.ui.sbPositionY3->setValue(0);
	w.ui.sbPositionZ3->setValue(0);
	w.logicalPositionChanged();

	auto* curve = new DatapickerCurve(i18n("Curve"));
	curve->addDatasheet(image->axisPoints().type);
	datapicker.addChild(curve);

	datapicker.addNewPoint(QPointF(5, 6), curve); // updates the curve data
	VALUES_EQUAL(curve->posXColumn()->valueAt(0), 0.); // x start is zero, which is not valid therefore the result is 0
	VALUES_EQUAL(curve->posYColumn()->valueAt(0), 0.); // x start is zero, which is not valid therefore the result is 0

	QCOMPARE(w.ui.sbPositionY2->setValue(1), true); // axisPointsChanged will call updatePoint()
	QCOMPARE(w.ui.sbPositionY3->setValue(1), true); // axisPointsChanged will call updatePoint()
	w.ui.sbPositionY3->valueChanged(1); // axisPointsChanged will call updatePoint()

	VALUES_EQUAL(curve->posXColumn()->valueAt(0), 20.);
	// Value validated manually, not reverse calculated
	VALUES_EQUAL(curve->posYColumn()->valueAt(0), 15.8489322662);
}

// Test if setting curve points on the image result in correct values for lnXY mapping
void DatapickerTest::logarithmicNaturalXYMapping() {
	Datapicker datapicker(QStringLiteral("Test"));
	auto* image = datapicker.image();

	// Set reference points
	datapicker.addNewPoint(QPointF(3, 10), image);
	datapicker.addNewPoint(QPointF(3, 0), image);
	datapicker.addNewPoint(QPointF(13, 0), image);

	auto ap = image->axisPoints();
	ap.type = DatapickerImage::GraphType::LnXY;
	image->setAxisPoints(ap);

	DatapickerImageWidget w(nullptr);
	w.setImages({image});
	w.ui.sbPositionX1->setValue(0);
	w.ui.sbPositionY1->setValue(100);
	w.ui.sbPositionZ1->setValue(0);
	w.ui.sbPositionX2->setValue(0);
	w.ui.sbPositionY2->setValue(0);
	w.ui.sbPositionZ2->setValue(0);
	w.ui.sbPositionX3->setValue(100);
	w.ui.sbPositionY3->setValue(0);
	w.ui.sbPositionZ3->setValue(0);
	w.logicalPositionChanged();

	auto* curve = new DatapickerCurve(i18n("Curve"));
	curve->addDatasheet(image->axisPoints().type);
	datapicker.addChild(curve);

	datapicker.addNewPoint(QPointF(5, 6), curve); // updates the curve data
	VALUES_EQUAL(curve->posXColumn()->valueAt(0), 0.); // x start is zero, which is not valid therefore the result is 0
	VALUES_EQUAL(curve->posYColumn()->valueAt(0), 0.); // x start is zero, which is not valid therefore the result is 0

	QCOMPARE(w.ui.sbPositionX1->setValue(1), true); // axisPointsChanged will call updatePoint()
	QCOMPARE(w.ui.sbPositionX2->setValue(1), true); // axisPointsChanged will call updatePoint()
	QCOMPARE(w.ui.sbPositionY2->setValue(1), true); // axisPointsChanged will call updatePoint()
	QCOMPARE(w.ui.sbPositionY3->setValue(1), true); // axisPointsChanged will call updatePoint()
	w.ui.sbPositionY1->valueChanged(1); // axisPointsChanged will call updatePoint()

	// Values validated manually, not reverse calculated
	VALUES_EQUAL(curve->posXColumn()->valueAt(0), 2.51188635826);
	VALUES_EQUAL(curve->posYColumn()->valueAt(0), 15.8489322662);
}

// Test if setting curve points on the image result in correct values for logX mapping
void DatapickerTest::logarithmic10XMapping() {
	Datapicker datapicker(QStringLiteral("Test"));
	auto* image = datapicker.image();

	// Set reference points
	datapicker.addNewPoint(QPointF(3, 10), image);
	datapicker.addNewPoint(QPointF(3, 0), image);
	datapicker.addNewPoint(QPointF(13, 0), image);

	auto ap = image->axisPoints();
	ap.type = DatapickerImage::GraphType::Log10X;
	image->setAxisPoints(ap);

	DatapickerImageWidget w(nullptr);
	w.setImages({image});
	w.ui.sbPositionX1->setValue(0);
	w.ui.sbPositionY1->setValue(100);
	w.ui.sbPositionZ1->setValue(0);
	w.ui.sbPositionX2->setValue(0);
	w.ui.sbPositionY2->setValue(0);
	w.ui.sbPositionZ2->setValue(0);
	w.ui.sbPositionX3->setValue(100);
	w.ui.sbPositionY3->setValue(0);
	w.ui.sbPositionZ3->setValue(0);
	w.logicalPositionChanged();

	auto* curve = new DatapickerCurve(i18n("Curve"));
	curve->addDatasheet(image->axisPoints().type);
	datapicker.addChild(curve);

	datapicker.addNewPoint(QPointF(5, 6), curve); // updates the curve data
	VALUES_EQUAL(curve->posXColumn()->valueAt(0), 0.); // x start is zero, which is not valid therefore the result is 0
	VALUES_EQUAL(curve->posYColumn()->valueAt(0), 0.); // x start is zero, which is not valid therefore the result is 0

	QCOMPARE(w.ui.sbPositionX1->setValue(1), true);
	QCOMPARE(w.ui.sbPositionX2->setValue(1), true);
	w.ui.sbPositionX2->valueChanged(1);
	QCOMPARE(image->axisPoints().type, DatapickerImage::GraphType::Log10X);

	// Value validated manually, not reverse calculated
	VALUES_EQUAL(curve->posXColumn()->valueAt(0), 2.51188635826); // TODO: correct?
	VALUES_EQUAL(curve->posYColumn()->valueAt(0), 60.);
}

// Test if setting curve points on the image result in correct values for logY mapping
void DatapickerTest::logarithmic10YMapping() {
	Datapicker datapicker(QStringLiteral("Test"));
	auto* image = datapicker.image();

	// Set reference points
	datapicker.addNewPoint(QPointF(3, 10), image);
	datapicker.addNewPoint(QPointF(3, 0), image);
	datapicker.addNewPoint(QPointF(13, 0), image);

	auto ap = image->axisPoints();
	ap.type = DatapickerImage::GraphType::Log10Y;
	image->setAxisPoints(ap);

	DatapickerImageWidget w(nullptr);
	w.setImages({image});
	w.ui.sbPositionX1->setValue(0);
	w.ui.sbPositionY1->setValue(100);
	w.ui.sbPositionZ1->setValue(0);
	w.ui.sbPositionX2->setValue(0);
	w.ui.sbPositionY2->setValue(0);
	w.ui.sbPositionZ2->setValue(0);
	w.ui.sbPositionX3->setValue(100);
	w.ui.sbPositionY3->setValue(0);
	w.ui.sbPositionZ3->setValue(0);
	w.logicalPositionChanged();

	auto* curve = new DatapickerCurve(i18n("Curve"));
	curve->addDatasheet(image->axisPoints().type);
	datapicker.addChild(curve);

	datapicker.addNewPoint(QPointF(5, 6), curve); // updates the curve data
	VALUES_EQUAL(curve->posXColumn()->valueAt(0), 0.); // x start is zero, which is not valid therefore the result is 0
	VALUES_EQUAL(curve->posYColumn()->valueAt(0), 0.); // x start is zero, which is not valid therefore the result is 0

	QCOMPARE(w.ui.sbPositionY2->setValue(1), true);
	QCOMPARE(w.ui.sbPositionY3->setValue(1), true);
	w.ui.sbPositionY3->valueChanged(1); // axisPointsChanged will call updatePoint()
	QCOMPARE(image->axisPoints().type, DatapickerImage::GraphType::Log10Y);

	VALUES_EQUAL(curve->posXColumn()->valueAt(0), 20.);
	// Value validated manually, not reverse calculated
	VALUES_EQUAL(curve->posYColumn()->valueAt(0), 15.8489322662);
}

// Test if setting curve points on the image result in correct values for logXY mapping
void DatapickerTest::logarithmic10XYMapping() {
	Datapicker datapicker(QStringLiteral("Test"));
	auto* image = datapicker.image();

	// Set reference points
	datapicker.addNewPoint(QPointF(3, 10), image);
	datapicker.addNewPoint(QPointF(3, 0), image);
	datapicker.addNewPoint(QPointF(13, 0), image);

	auto ap = image->axisPoints();
	ap.type = DatapickerImage::GraphType::Log10XY;
	image->setAxisPoints(ap);

	DatapickerImageWidget w(nullptr);
	w.setImages({image});
	w.ui.sbPositionX1->setValue(0);
	w.ui.sbPositionY1->setValue(100);
	w.ui.sbPositionZ1->setValue(0);
	w.ui.sbPositionX2->setValue(0);
	w.ui.sbPositionY2->setValue(0);
	w.ui.sbPositionZ2->setValue(0);
	w.ui.sbPositionX3->setValue(100);
	w.ui.sbPositionY3->setValue(0);
	w.ui.sbPositionZ3->setValue(0);
	w.logicalPositionChanged();

	auto* curve = new DatapickerCurve(i18n("Curve"));
	curve->addDatasheet(image->axisPoints().type);
	datapicker.addChild(curve);

	datapicker.addNewPoint(QPointF(5, 6), curve); // updates the curve data
	VALUES_EQUAL(curve->posXColumn()->valueAt(0), 0.); // x start is zero, which is not valid therefore the result is 0
	VALUES_EQUAL(curve->posYColumn()->valueAt(0), 0.); // x start is zero, which is not valid therefore the result is 0

	QCOMPARE(w.ui.sbPositionX1->setValue(1), true);
	QCOMPARE(w.ui.sbPositionX2->setValue(1), true);
	QCOMPARE(w.ui.sbPositionY2->setValue(1), true);
	QCOMPARE(w.ui.sbPositionY3->setValue(1), true);
	w.ui.sbPositionY3->valueChanged(1); // axisPointsChanged will call updatePoint()
	// Values validated manually, not reverse calculated
	VALUES_EQUAL(curve->posXColumn()->valueAt(0), 2.51188635826);
	VALUES_EQUAL(curve->posYColumn()->valueAt(0), 15.8489322662);
}

/*!
 * check the correctness of the data points after one of the reference points was moved on the scene.
 * In real this is not possible, because it is not implemented, but nevertheless it shall be shown
 * that when moving reference points, the curves will be updated
 */
void DatapickerTest::referenceMove() {
	Datapicker datapicker(QStringLiteral("Test"));
	datapicker.addNewPoint(QPointF(0, 1), datapicker.image());
	datapicker.addNewPoint(QPointF(0, 0), datapicker.image());
	datapicker.addNewPoint(QPointF(1, 0), datapicker.image());

	auto ap = datapicker.image()->axisPoints();
	ap.type = DatapickerImage::GraphType::Linear;
	datapicker.image()->setAxisPoints(ap);

	DatapickerImageWidget w(nullptr);
	w.setImages({datapicker.image()});
	w.ui.sbPositionX1->setValue(0);
	w.ui.sbPositionY1->setValue(10);
	w.ui.sbPositionZ1->setValue(0);
	w.ui.sbPositionX2->setValue(0);
	w.ui.sbPositionY2->setValue(0);
	w.ui.sbPositionZ2->setValue(0);
	w.ui.sbPositionX3->setValue(10);
	w.ui.sbPositionY3->setValue(0);
	w.ui.sbPositionZ3->setValue(0);
	w.logicalPositionChanged();

	auto* curve = new DatapickerCurve(i18n("Curve"));
	curve->addDatasheet(datapicker.image()->axisPoints().type);
	datapicker.addChild(curve);

	datapicker.addNewPoint(QPointF(0.5, 0.6), curve); // updates the curve data
	VALUES_EQUAL(curve->posXColumn()->valueAt(0), 5.);
	VALUES_EQUAL(curve->posYColumn()->valueAt(0), 6.);

	// Points are stored in the image
	auto points = datapicker.image()->children<DatapickerPoint>(AbstractAspect::ChildIndexFlag::IncludeHidden);
	QCOMPARE(points.count(), 3);

	// set position after mouse was released
	points[0]->setPosition(QPointF(0., 4));
	// points[1]->setPosition(QPointF(0.1, 0.));
	points[2]->setPosition(QPointF(2., 0.));

	// Currently not supported
	//    VALUES_EQUAL(curve->posXColumn()->valueAt(0), 0.5 / 2 * 10);
	//    VALUES_EQUAL(curve->posYColumn()->valueAt(0), 0.6 / 4  * 10);
}

void DatapickerTest::referenceMoveKeyPress() {
	Datapicker datapicker(QStringLiteral("Test"));
	datapicker.addNewPoint(QPointF(0, 1), datapicker.image());
	datapicker.addNewPoint(QPointF(0, 0), datapicker.image());
	datapicker.addNewPoint(QPointF(1, 0), datapicker.image());

	auto ap = datapicker.image()->axisPoints();
	ap.type = DatapickerImage::GraphType::Linear;
	datapicker.image()->setAxisPoints(ap);

	DatapickerImageWidget w(nullptr);
	w.setImages({datapicker.image()});
	w.ui.sbPositionX1->setValue(0);
	w.ui.sbPositionY1->setValue(10);
	w.ui.sbPositionZ1->setValue(0);
	w.ui.sbPositionX2->setValue(0);
	w.ui.sbPositionY2->setValue(0);
	w.ui.sbPositionZ2->setValue(0);
	w.ui.sbPositionX3->setValue(10);
	w.ui.sbPositionY3->setValue(0);
	w.ui.sbPositionZ3->setValue(0);
	w.logicalPositionChanged();

	auto* curve = new DatapickerCurve(i18n("Curve"));
	curve->addDatasheet(datapicker.image()->axisPoints().type);
	datapicker.addChild(curve);

	datapicker.addNewPoint(QPointF(0.5, 0.6), curve); // (scene coordinates) updates the curve data
	VALUES_EQUAL(curve->posXColumn()->valueAt(0), 5.);
	VALUES_EQUAL(curve->posYColumn()->valueAt(0), 6.);

	// Points are stored in the image
	auto points = datapicker.image()->children<DatapickerPoint>(AbstractAspect::ChildIndexFlag::IncludeHidden);
	QCOMPARE(points.count(), 3);

	points[0]->d_ptr->setSelected(false);
	points[1]->d_ptr->setSelected(false);
	points[2]->d_ptr->setSelected(true);
	auto view = static_cast<DatapickerImageView*>(datapicker.image()->view());
	view->shiftLeftAction->triggered(true);

	VALUES_EQUAL(points[0]->position().x(), 0.);
	VALUES_EQUAL(points[1]->position().x(), 0.);
	VALUES_EQUAL(points[2]->position().x(), 2.);

	VALUES_EQUAL(curve->posXColumn()->valueAt(0), 0.5 / 2 * 10);
	VALUES_EQUAL(curve->posYColumn()->valueAt(0), 6.);
}

/*!
 * check the correctness of the data point after the point was moved on the scene.
 */
void DatapickerTest::curvePointMove() {
	Datapicker datapicker(QStringLiteral("Test"));
	auto* image = datapicker.image();

	// add reference points
	datapicker.addNewPoint(QPointF(0, 1), image);
	datapicker.addNewPoint(QPointF(0, 0), image);
	datapicker.addNewPoint(QPointF(1, 0), image);

	auto ap = image->axisPoints();
	ap.type = DatapickerImage::GraphType::Linear;
	image->setAxisPoints(ap);

	// set logical coordinates for the reference points
	DatapickerImageWidget w(nullptr);
	w.setImages({image});
	w.ui.sbPositionX1->setValue(0);
	w.ui.sbPositionY1->setValue(10);
	w.ui.sbPositionZ1->setValue(0);
	w.ui.sbPositionX2->setValue(0);
	w.ui.sbPositionY2->setValue(0);
	w.ui.sbPositionZ2->setValue(0);
	w.ui.sbPositionX3->setValue(10);
	w.ui.sbPositionY3->setValue(0);
	w.ui.sbPositionZ3->setValue(0);
	w.logicalPositionChanged();

	auto* curve = new DatapickerCurve(i18n("Curve"));
	curve->addDatasheet(image->axisPoints().type);
	datapicker.addChild(curve);

	// add new curve point and check its logical coordinates
	datapicker.addNewPoint(QPointF(0.5, 0.6), curve);
	VALUES_EQUAL(curve->posXColumn()->valueAt(0), 5.);
	VALUES_EQUAL(curve->posYColumn()->valueAt(0), 6.);

	// move the last added point to a new position and check its logical coordinates again
	auto points = curve->children<DatapickerPoint>(AbstractAspect::ChildIndexFlag::IncludeHidden);
	QCOMPARE(points.count(), 1);
	points[0]->setPosition(QPointF(0.2, 0.9)); // Changing the position of the point
	VALUES_EQUAL(curve->posXColumn()->valueAt(0), 2.);
	VALUES_EQUAL(curve->posYColumn()->valueAt(0), 9.);
}

/*!
 * check the correctness of the data point after the point was moved on the scene with undo and redo after this.
 */
void DatapickerTest::curvePointMoveUndoRedo() {
	Project project;
	auto* datapicker = new Datapicker(QStringLiteral("Test"));
	project.addChild(datapicker);
	auto* image = datapicker->image();

	// add reference points
	datapicker->addNewPoint(QPointF(0, 1), image);
	datapicker->addNewPoint(QPointF(0, 0), image);
	datapicker->addNewPoint(QPointF(1, 0), image);

	auto ap = image->axisPoints();
	ap.type = DatapickerImage::GraphType::Linear;
	image->setAxisPoints(ap);

	// set logical coordinates for the reference points
	DatapickerImageWidget w(nullptr);
	w.setImages({image});
	w.ui.sbPositionX1->setValue(0);
	w.ui.sbPositionY1->setValue(10);
	w.ui.sbPositionZ1->setValue(0);
	w.ui.sbPositionX2->setValue(0);
	w.ui.sbPositionY2->setValue(0);
	w.ui.sbPositionZ2->setValue(0);
	w.ui.sbPositionX3->setValue(10);
	w.ui.sbPositionY3->setValue(0);
	w.ui.sbPositionZ3->setValue(0);
	w.logicalPositionChanged();

	auto* curve = new DatapickerCurve(i18n("Curve"));
	curve->addDatasheet(image->axisPoints().type);
	datapicker->addChild(curve);

	// add new curve point and check its logical coordinates
	datapicker->addNewPoint(QPointF(0.5, 0.6), curve);
	VALUES_EQUAL(curve->posXColumn()->valueAt(0), 5.);
	VALUES_EQUAL(curve->posYColumn()->valueAt(0), 6.);

	// move the last added point to a new position and check its logical coordinates again
	auto points = curve->children<DatapickerPoint>(AbstractAspect::ChildIndexFlag::IncludeHidden);
	QCOMPARE(points.count(), 1);
	points[0]->setPosition(QPointF(0.2, 0.9)); // Changing the position of the point
	VALUES_EQUAL(curve->posXColumn()->valueAt(0), 2.);
	VALUES_EQUAL(curve->posYColumn()->valueAt(0), 9.);

	// undo the move step and check the position again
	auto* undoStack = project.undoStack();
	undoStack->undo();
	points = curve->children<DatapickerPoint>(AbstractAspect::ChildIndexFlag::IncludeHidden);
	QCOMPARE(points.count(), 1);
	QCOMPARE(points[0]->position(), QPointF(0.5, 0.6));
	VALUES_EQUAL(curve->posXColumn()->valueAt(0), 5.);
	VALUES_EQUAL(curve->posYColumn()->valueAt(0), 6.);

	// redo the last step and check the position again
	undoStack->redo();
	points = curve->children<DatapickerPoint>(AbstractAspect::ChildIndexFlag::IncludeHidden);
	QCOMPARE(points.count(), 1);
	QCOMPARE(points[0]->position(), QPointF(0.2, 0.9));
	VALUES_EQUAL(curve->posXColumn()->valueAt(0), 2.);
	VALUES_EQUAL(curve->posYColumn()->valueAt(0), 9.);
}

void DatapickerTest::selectReferencePoint() {
	Datapicker datapicker(QStringLiteral("Test"));
	auto* image = datapicker.image();

	// Set reference points
	datapicker.addNewPoint(QPointF(0, 1), image);
	datapicker.addNewPoint(QPointF(0, 0), image);
	datapicker.addNewPoint(QPointF(1, 0), image);

	auto ap = image->axisPoints();
	ap.type = DatapickerImage::GraphType::Linear;
	image->setAxisPoints(ap);

	DatapickerImageWidget w(nullptr);
	w.setImages({image});

	QCOMPARE(w.ui.rbRefPoint1->isChecked(), false);
	QCOMPARE(w.ui.rbRefPoint2->isChecked(), false);
	QCOMPARE(w.ui.rbRefPoint3->isChecked(), false);

	auto points = image->children<DatapickerPoint>(AbstractAspect::ChildIndexFlag::IncludeHidden);
	QCOMPARE(points.count(), 3);

	// Change reference point selection
	points[0]->pointSelected(points[0]);
	QCOMPARE(w.ui.rbRefPoint1->isChecked(), true);
	QCOMPARE(w.ui.rbRefPoint2->isChecked(), false);
	QCOMPARE(w.ui.rbRefPoint3->isChecked(), false);

	points[1]->pointSelected(points[1]);
	QCOMPARE(w.ui.rbRefPoint1->isChecked(), false);
	QCOMPARE(w.ui.rbRefPoint2->isChecked(), true);
	QCOMPARE(w.ui.rbRefPoint3->isChecked(), false);

	points[2]->pointSelected(points[2]);
	QCOMPARE(w.ui.rbRefPoint1->isChecked(), false);
	QCOMPARE(w.ui.rbRefPoint2->isChecked(), false);
	QCOMPARE(w.ui.rbRefPoint3->isChecked(), true);
}

void DatapickerTest::recreateAxisReferencePoints() {
	Datapicker datapicker(QStringLiteral("Test"));
	auto* image = datapicker.image();

	// Set reference points
	datapicker.addNewPoint(QPointF(0, 1), image);
	datapicker.addNewPoint(QPointF(0, 0), image);
	datapicker.addNewPoint(QPointF(1, 0), image);

	auto ap = image->axisPoints();
	ap.type = DatapickerImage::GraphType::Linear;
	image->setAxisPoints(ap);

	DatapickerImageWidget w(nullptr);
	w.setImages({image});
	w.ui.sbPositionX1->setValue(0);
	w.ui.sbPositionY1->setValue(10);
	w.ui.sbPositionZ1->setValue(0);
	w.ui.sbPositionX2->setValue(0);
	w.ui.sbPositionY2->setValue(0);
	w.ui.sbPositionZ2->setValue(0);
	w.ui.sbPositionX3->setValue(10);
	w.ui.sbPositionY3->setValue(0);
	w.ui.sbPositionZ3->setValue(0);
	w.logicalPositionChanged();

	auto* curve = new DatapickerCurve(i18n("Curve"));
	curve->addDatasheet(image->axisPoints().type);
	datapicker.addChild(curve);

	datapicker.addNewPoint(QPointF(0.5, 0.5), curve); // updates the curve data
	VALUES_EQUAL(curve->posXColumn()->valueAt(0), 5.);
	VALUES_EQUAL(curve->posYColumn()->valueAt(0), 5.);

	datapicker.addNewPoint(QPointF(0.7, 0.65), curve); // updates the curve data
	VALUES_EQUAL(curve->posXColumn()->valueAt(1), 7.);
	VALUES_EQUAL(curve->posYColumn()->valueAt(1), 6.5);

	// QVERIFY(datapicker.image()->plotPointsType() != DatapickerImage::PointsType::AxisPoints);
	// datapicker.image()->setPlotPointsType(DatapickerImage::PointsType::AxisPoints);

	datapicker.image()->clearReferencePoints();
	QCOMPARE(datapicker.image()->children<DatapickerPoint>(AbstractAspect::ChildIndexFlag::IncludeHidden).count(), 0);

	QVERIFY(std::isnan(curve->posXColumn()->valueAt(0)));
	QVERIFY(std::isnan(curve->posYColumn()->valueAt(0)));
	QVERIFY(std::isnan(curve->posXColumn()->valueAt(1)));
	QVERIFY(std::isnan(curve->posYColumn()->valueAt(1)));

	// Recreate points again
	datapicker.addNewPoint(QPointF(0, 1), image);
	datapicker.addNewPoint(QPointF(0, 0), image);
	datapicker.addNewPoint(QPointF(1, 0), image);

	VALUES_EQUAL(curve->posXColumn()->valueAt(0), 5.);
	VALUES_EQUAL(curve->posYColumn()->valueAt(0), 5.);
	VALUES_EQUAL(curve->posXColumn()->valueAt(1), 7.);
	VALUES_EQUAL(curve->posYColumn()->valueAt(1), 6.5);

	// Change position
	{
		auto points = datapicker.image()->children<DatapickerPoint>(AbstractAspect::ChildIndexFlag::IncludeHidden);

		// The logical position is still 10 for y1. So at 0.5 we have now 10 instead of 5.
		points.at(0)->setPosition(QPointF(0., 0.5));
		VALUES_EQUAL(curve->posXColumn()->valueAt(0), 5.);
		VALUES_EQUAL(curve->posYColumn()->valueAt(0), 10.);
		VALUES_EQUAL(curve->posXColumn()->valueAt(1), 7.);
		VALUES_EQUAL(curve->posYColumn()->valueAt(1), 13.);

		// Now we stretch the x axis as well
		points.at(2)->setPosition(QPointF(0.5, 0.));
		VALUES_EQUAL(curve->posXColumn()->valueAt(0), 10.);
		VALUES_EQUAL(curve->posYColumn()->valueAt(0), 10.);
		VALUES_EQUAL(curve->posXColumn()->valueAt(1), 14.);
		VALUES_EQUAL(curve->posYColumn()->valueAt(1), 13.);
	}
}

void DatapickerTest::imageAxisPointsChanged() {
	DatapickerImageWidget w(nullptr);
	Datapicker datapicker(QStringLiteral("Test"));
	auto* image = datapicker.image();
	w.setImages({image});

	DatapickerImage::ReferencePoints points;
	points.logicalPos[0] = Vector3D(-1., 2., 4.);
	points.logicalPos[1] = Vector3D(-5., -867., 236.);
	points.logicalPos[2] = Vector3D(43., -231.2f, 234.);
	points.scenePos[0] = QPointF(-3, 2);
	points.scenePos[1] = QPointF(-291, 3249);
	points.scenePos[2] = QPointF(-239, 349);
	points.ternaryScale = -23;

	points.type = DatapickerImage::GraphType::Linear;
	w.imageAxisPointsChanged(points);
	QCOMPARE(static_cast<DatapickerImage::GraphType>(w.ui.cbGraphType->currentData().toInt()), DatapickerImage::GraphType::Linear);

	points.type = DatapickerImage::GraphType::PolarInDegree;
	w.imageAxisPointsChanged(points);
	QCOMPARE(static_cast<DatapickerImage::GraphType>(w.ui.cbGraphType->currentData().toInt()), DatapickerImage::GraphType::PolarInDegree);

	points.type = DatapickerImage::GraphType::PolarInRadians;
	w.imageAxisPointsChanged(points);
	QCOMPARE(static_cast<DatapickerImage::GraphType>(w.ui.cbGraphType->currentData().toInt()), DatapickerImage::GraphType::PolarInRadians);

	points.type = DatapickerImage::GraphType::LnX;
	w.imageAxisPointsChanged(points);
	QCOMPARE(static_cast<DatapickerImage::GraphType>(w.ui.cbGraphType->currentData().toInt()), DatapickerImage::GraphType::LnX);

	points.type = DatapickerImage::GraphType::LnY;
	w.imageAxisPointsChanged(points);
	QCOMPARE(static_cast<DatapickerImage::GraphType>(w.ui.cbGraphType->currentData().toInt()), DatapickerImage::GraphType::LnY);

	points.type = DatapickerImage::GraphType::Ternary;
	w.imageAxisPointsChanged(points);
	QCOMPARE(static_cast<DatapickerImage::GraphType>(w.ui.cbGraphType->currentData().toInt()), DatapickerImage::GraphType::Ternary);

	points.type = DatapickerImage::GraphType::LnXY;
	w.imageAxisPointsChanged(points);
	QCOMPARE(static_cast<DatapickerImage::GraphType>(w.ui.cbGraphType->currentData().toInt()), DatapickerImage::GraphType::LnXY);

	points.type = DatapickerImage::GraphType::Log10XY;
	w.imageAxisPointsChanged(points);
	QCOMPARE(static_cast<DatapickerImage::GraphType>(w.ui.cbGraphType->currentData().toInt()), DatapickerImage::GraphType::Log10XY);

	points.type = DatapickerImage::GraphType::Log10X;
	w.imageAxisPointsChanged(points);
	QCOMPARE(static_cast<DatapickerImage::GraphType>(w.ui.cbGraphType->currentData().toInt()), DatapickerImage::GraphType::Log10X);

	points.type = DatapickerImage::GraphType::Log10Y;
	w.imageAxisPointsChanged(points);
	QCOMPARE(static_cast<DatapickerImage::GraphType>(w.ui.cbGraphType->currentData().toInt()), DatapickerImage::GraphType::Log10Y);
}

void DatapickerTest::datapickerDateTime() {
	DatapickerImageWidget w(nullptr);
	Datapicker datapicker(QStringLiteral("Test"));
	auto* image = datapicker.image();
	image->setImage(createImage(), QStringLiteral("Image"), true);

	// add reference points
	datapicker.addNewPoint(QPointF(0., 1.), image); // scene coordinates
	datapicker.addNewPoint(QPointF(0., 0.), image); // scene coordinates
	datapicker.addNewPoint(QPointF(1., 0.), image); // scene coordinates

	w.setImages({image});

	w.ui.sbPositionX1->setValue(0);
	w.ui.sbPositionY1->setValue(10.);
	w.ui.sbPositionZ1->setValue(0);
	w.ui.sbPositionX2->setValue(0);
	w.ui.sbPositionY2->setValue(0);
	w.ui.sbPositionZ2->setValue(0);
	w.ui.sbPositionX3->setValue(10.);
	w.ui.sbPositionY3->setValue(0);
	w.ui.sbPositionZ3->setValue(0);
	w.logicalPositionChanged();

	auto* curve = new DatapickerCurve(i18n("Curve"));
	curve->addDatasheet(image->axisPoints().type);
	datapicker.addChild(curve);

	// add new curve point and check its logical coordinates
	datapicker.addNewPoint(QPointF(0.5, 0.6), curve); // scene coordinates
	VALUES_EQUAL(curve->posXColumn()->valueAt(0), 5.); // logical coordinates
	VALUES_EQUAL(curve->posYColumn()->valueAt(0), 6.); // logical coordinates

	QCOMPARE(w.ui.cbDatetime->isChecked(), false);
	QCOMPARE(w.ui.dtePositionX1->isVisible(), false);
	QCOMPARE(w.ui.dtePositionX2->isVisible(), false);
	QCOMPARE(w.ui.dtePositionX3->isVisible(), false);
	// QCOMPARE(w.ui.sbPositionX1->isVisible(), true); // Not possible if no MainWin?
	// QCOMPARE(w.ui.sbPositionX2->isVisible(), true);
	// QCOMPARE(w.ui.sbPositionX3->isVisible(), true);

	QCOMPARE(curve->posXColumn()->columnMode(), AbstractColumn::ColumnMode::Double);

	QCOMPARE(w.ui.cbDatetime->isEnabled(), true);
	w.ui.cbDatetime->clicked(true);

	// QCOMPARE(w.ui.cbDatetime->isChecked(), true);
	// QCOMPARE(w.ui.dtePositionX1->isVisible(), true);
	// QCOMPARE(w.ui.dtePositionX2->isVisible(), true);
	// QCOMPARE(w.ui.dtePositionX3->isVisible(), true);
	QCOMPARE(w.ui.sbPositionX1->isVisible(), false);
	QCOMPARE(w.ui.sbPositionX2->isVisible(), false);
	QCOMPARE(w.ui.sbPositionX3->isVisible(), false);

	QDateTime dt1 = QDateTime::fromString(QLatin1String("2000-12-01 00:00:00:000Z"), QStringLiteral("yyyy-MM-dd hh:mm:ss:zzzt"));
	QDateTime dt2 = QDateTime::fromString(QLatin1String("2000-12-01 00:00:00:000Z"), QStringLiteral("yyyy-MM-dd hh:mm:ss:zzzt"));
	QDateTime dt3 = QDateTime::fromString(QLatin1String("2000-12-01 06:00:00:000Z"), QStringLiteral("yyyy-MM-dd hh:mm:ss:zzzt"));
	w.ui.dtePositionX1->setMSecsSinceEpochUTC(dt1.toMSecsSinceEpoch());
	w.ui.dtePositionX2->setMSecsSinceEpochUTC(dt2.toMSecsSinceEpoch());
	w.ui.dtePositionX3->setMSecsSinceEpochUTC(dt3.toMSecsSinceEpoch());

	QCOMPARE(curve->posXColumn()->rowCount(), 1);
	QCOMPARE(curve->posXColumn()->columnMode(), AbstractColumn::ColumnMode::DateTime);
#if !defined(_WIN32)
	QCOMPARE(curve->posXColumn()->dateTimeAt(0),
			 QDateTime::fromString(QLatin1String("2000-12-01 03:00:00:000Z"), QStringLiteral("yyyy-MM-dd hh:mm:ss:zzzt"))); // logical coordinates
#endif
	QCOMPARE(curve->posYColumn()->rowCount(), 1);
	VALUES_EQUAL(curve->posYColumn()->valueAt(0), 6.); // logical coordinates
}

void DatapickerTest::datapickerDeleteCurvePoint() {
	DatapickerImageWidget w(nullptr);
	Datapicker datapicker(QStringLiteral("Test"));
	auto* image = datapicker.image();

	// add reference points
	datapicker.addNewPoint(QPointF(0, 1), image); // scene coordinates
	datapicker.addNewPoint(QPointF(0, 0), image); // scene coordinates
	datapicker.addNewPoint(QPointF(1, 0), image); // scene coordinates

	w.setImages({image});

	w.ui.sbPositionX1->setValue(0);
	w.ui.sbPositionY1->setValue(10.);
	w.ui.sbPositionZ1->setValue(0);
	w.ui.sbPositionX2->setValue(0);
	w.ui.sbPositionY2->setValue(0);
	w.ui.sbPositionZ2->setValue(0);
	w.ui.sbPositionX3->setValue(10.);
	w.ui.sbPositionY3->setValue(0);
	w.ui.sbPositionZ3->setValue(0);
	w.logicalPositionChanged();

	auto* curve = new DatapickerCurve(i18n("Curve"));
	curve->addDatasheet(image->axisPoints().type);
	datapicker.addChild(curve);

	// add new curve point and check its logical coordinates
	datapicker.addNewPoint(QPointF(0.5, 0.6), curve); // scene coordinates
	QCOMPARE(curve->posXColumn()->rowCount(), 1);
	QCOMPARE(curve->posYColumn()->rowCount(), 1);

	datapicker.addNewPoint(QPointF(0.5, 0.7), curve); // scene coordinates
	QCOMPARE(curve->posXColumn()->rowCount(), 2);
	QCOMPARE(curve->posYColumn()->rowCount(), 2);

	auto points = curve->children<DatapickerPoint>(AbstractAspect::ChildIndexFlag::IncludeHidden);
	QCOMPARE(points.count(), 2);

	points.at(0)->remove();

	QCOMPARE(curve->posXColumn()->rowCount(), 1);
	QCOMPARE(curve->posYColumn()->rowCount(), 1);
}

void DatapickerTest::datapickerImageLoadImageAbsolute() {
	QString savePath;
	QString imgFileName;
	{
		QTemporaryFile imgFile(QStringLiteral("Testimage_XXXXXX.png"));
		const auto img = createImage();
		QVERIFY(imgFile.open());
		imgFileName = imgFile.fileName();
		QVERIFY(img.save(imgFileName, "PNG"));
		QVERIFY(QFile::exists(imgFile.fileName()));

		{
			Project project;
			auto* datapicker = new Datapicker(QStringLiteral("Test"));
			project.addChild(datapicker);
			auto* image = datapicker->image();

			DatapickerImageWidget w(nullptr);
			w.setImages({image});

			QCOMPARE(w.ui.cbFileEmbedd->isEnabled(), false);
			QCOMPARE(w.ui.cbFileRelativePath->isEnabled(), false);
			QCOMPARE(w.ui.cbFileEmbedd->isChecked(), false);
			QCOMPARE(w.ui.cbFileRelativePath->isChecked(), false);

			w.ui.leFileName->setText(imgFileName);
			// w.fileNameChanged(); will be called

			QCOMPARE(w.ui.cbFileEmbedd->isEnabled(), true);
			QCOMPARE(w.ui.cbFileRelativePath->isEnabled(), false); // project is not saved
			QCOMPARE(w.ui.cbFileEmbedd->isChecked(), false);
			QCOMPARE(w.ui.cbFileRelativePath->isChecked(), false);

			QCOMPARE(image->fileName(), imgFile.fileName());
			QCOMPARE(image->originalPlotImage.isNull(), false); // valid image loaded
			QCOMPARE(w.ui.leFileName->text(), imgFile.fileName());
			QCOMPARE(w.ui.leFileName->styleSheet(), QStringLiteral("")); // Valid image

			SAVE_PROJECT("DatapickerTestProject");

			QCOMPARE(w.ui.cbFileRelativePath->isEnabled(), true); // project is now saved so calculating the relative path is possible
		}

		// Load project
		{
			Project project;
			QCOMPARE(project.load(savePath), true);

			auto datapicker = project.child<Datapicker>(0);
			QVERIFY(datapicker);
			auto* image = datapicker->image();

			DatapickerImageWidget w(nullptr);
			w.setImages({image});

			QCOMPARE(w.ui.cbFileEmbedd->isEnabled(), true);
			QCOMPARE(w.ui.cbFileRelativePath->isEnabled(), true); // project was loaded
			QCOMPARE(w.ui.cbFileEmbedd->isChecked(), false);
			QCOMPARE(w.ui.cbFileRelativePath->isChecked(), false);

			QCOMPARE(image->fileName(), imgFileName);
			QCOMPARE(image->originalPlotImage.isNull(), false); // valid image loaded
			QCOMPARE(w.ui.leFileName->text(), imgFileName);
			QCOMPARE(w.ui.leFileName->styleSheet(), QStringLiteral("")); // Valid image
		}
	}

	// Deleting image
	QCOMPARE(QFile::exists(imgFileName), false); // Deleted because QTemporaryFile goes out of scope
	//	QFile f(imgFileName.fileName());
	//	QVERIFY(f.remove()); // Does not work on windows, see MR for more info

	// Load project
	{
		Project project;
		QCOMPARE(project.load(savePath), true);

		auto datapicker = project.child<Datapicker>(0);
		QVERIFY(datapicker);
		auto* image = datapicker->image();

		DatapickerImageWidget w(nullptr);
		w.setImages({image});

		QCOMPARE(w.ui.cbFileEmbedd->isEnabled(), false); // image is invalid, because it is anymore available
		QCOMPARE(w.ui.cbFileRelativePath->isEnabled(), false); // image is invalid
		QCOMPARE(w.ui.cbFileEmbedd->isChecked(), false);
		QCOMPARE(w.ui.cbFileRelativePath->isChecked(), false);

		QCOMPARE(image->fileName(), imgFileName);
		QCOMPARE(image->originalPlotImage.isNull(), true); // invalid image loaded
		QCOMPARE(w.ui.leFileName->text(), imgFileName);
		QVERIFY(w.ui.leFileName->styleSheet() != QStringLiteral("")); // Invalid image
	}
}

void DatapickerTest::datapickerImageLoadImageRelative() {
	QString savePath;
	QString imgFileName;
	{
		const auto img = createImage();

		QTemporaryFile imgFile(QStringLiteral("Testimage_XXXXXX.png"));
		QVERIFY(imgFile.open());
		imgFileName = imgFile.fileName();
		img.save(imgFileName, "PNG");
		QVERIFY(QFile::exists(imgFileName));
		{
			Project project;
			auto* datapicker = new Datapicker(QStringLiteral("Test"));
			project.addChild(datapicker);
			auto* image = datapicker->image();

			DatapickerImageWidget w(nullptr);
			w.setImages({image});

			QCOMPARE(w.ui.cbFileEmbedd->isEnabled(), false);
			QCOMPARE(w.ui.cbFileRelativePath->isEnabled(), false);
			QCOMPARE(w.ui.cbFileEmbedd->isChecked(), false);
			QCOMPARE(w.ui.cbFileRelativePath->isChecked(), false);

			w.ui.leFileName->setText(imgFileName);
			// w.fileNameChanged(); will be called

			QCOMPARE(w.ui.cbFileEmbedd->isEnabled(), true);
			QCOMPARE(w.ui.cbFileRelativePath->isEnabled(), false); // project is not saved
			QCOMPARE(w.ui.cbFileEmbedd->isChecked(), false);
			QCOMPARE(w.ui.cbFileRelativePath->isChecked(), false);

			QCOMPARE(image->fileName(), imgFileName);
			QCOMPARE(image->originalPlotImage.isNull(), false); // valid image loaded
			QCOMPARE(w.ui.leFileName->text(), imgFileName);
			QCOMPARE(w.ui.leFileName->styleSheet(), QStringLiteral("")); // Valid image

			SAVE_PROJECT("DatapickerTestProject");

			QCOMPARE(w.ui.cbFileRelativePath->isEnabled(), true); // project is now saved so calculating the relative path is possible

			w.ui.cbFileRelativePath->clicked(true);

			// QCOMPARE(w.ui.cbFileRelativePath->isChecked(), true);
			QFileInfo fi(imgFile);
			QCOMPARE(image->fileName(), fi.fileName());
			QCOMPARE(w.ui.leFileName->text(), fi.fileName());

			SAVE_PROJECT("DatapickerTestProject"); // Save again to save relative path
		}

		// Load project
		{
			Project project;
			QCOMPARE(project.load(savePath), true);

			auto datapicker = project.child<Datapicker>(0);
			QVERIFY(datapicker);
			auto* image = datapicker->image();

			DatapickerImageWidget w(nullptr);
			w.setImages({image});

			QCOMPARE(w.ui.cbFileEmbedd->isEnabled(), true);
			QCOMPARE(w.ui.cbFileRelativePath->isEnabled(), true); // project was loaded
			QCOMPARE(w.ui.cbFileEmbedd->isChecked(), false);
			QCOMPARE(w.ui.cbFileRelativePath->isChecked(), true);

			QFileInfo fi(imgFileName);
			QCOMPARE(image->fileName(), fi.fileName());
			QCOMPARE(image->originalPlotImage.isNull(), false); // valid image loaded
			QCOMPARE(w.ui.leFileName->text(), fi.fileName());
			QCOMPARE(w.ui.leFileName->styleSheet(), QStringLiteral("")); // Valid image
		}
	}

	// Deleting image
	QCOMPARE(QFile::exists(imgFileName), false); // Deleted because QTemporaryFile goes out of scope
	//	QFile f(imgFileName.fileName());
	//	QVERIFY(f.remove()); // Does not work on windows, see MR for more info

	// Load project
	{
		Project project;
		QCOMPARE(project.load(savePath), true);

		auto datapicker = project.child<Datapicker>(0);
		QVERIFY(datapicker);
		auto* image = datapicker->image();

		DatapickerImageWidget w(nullptr);
		w.setImages({image});

		QCOMPARE(w.ui.cbFileEmbedd->isEnabled(), false); // image is invalid, because it is anymore available
		QCOMPARE(w.ui.cbFileRelativePath->isEnabled(), false); // image is invalid
		QCOMPARE(w.ui.cbFileEmbedd->isChecked(), false);
		QCOMPARE(w.ui.cbFileRelativePath->isChecked(), true);

		QFileInfo fi(imgFileName);
		QCOMPARE(image->fileName(), fi.fileName());
		QCOMPARE(image->originalPlotImage.isNull(), true); // invalid image loaded
		QCOMPARE(w.ui.leFileName->text(), fi.fileName());
		QVERIFY(w.ui.leFileName->styleSheet() != QStringLiteral("")); // Invalid image
	}
}

void DatapickerTest::datapickerImageLoadImageEmbeddAbsolute() {
	QString savePath;
	QString imgFileName;

	{
		const auto img = createImage();

		QTemporaryFile imgFile(QStringLiteral("Testimage_XXXXXX.png"));
		QVERIFY(imgFile.open());
		imgFileName = imgFile.fileName();
		img.save(imgFileName, "PNG");
		QVERIFY(QFile::exists(imgFileName));

		Project project;
		auto* datapicker = new Datapicker(QStringLiteral("Test"));
		project.addChild(datapicker);
		auto* image = datapicker->image();

		DatapickerImageWidget w(nullptr);
		w.setImages({image});

		QCOMPARE(w.ui.cbFileEmbedd->isEnabled(), false);
		QCOMPARE(w.ui.cbFileRelativePath->isEnabled(), false);
		QCOMPARE(w.ui.cbFileEmbedd->isChecked(), false);
		QCOMPARE(w.ui.cbFileRelativePath->isChecked(), false);

		w.ui.leFileName->setText(imgFileName);
		// w.fileNameChanged(); will be called

		QCOMPARE(w.ui.cbFileEmbedd->isEnabled(), true);
		QCOMPARE(w.ui.cbFileRelativePath->isEnabled(), false); // project is not saved
		QCOMPARE(w.ui.cbFileEmbedd->isChecked(), false);
		QCOMPARE(w.ui.cbFileRelativePath->isChecked(), false);

		QCOMPARE(image->fileName(), imgFileName);
		QCOMPARE(image->originalPlotImage.isNull(), false); // valid image loaded
		QCOMPARE(w.ui.leFileName->text(), imgFileName);
		QCOMPARE(w.ui.leFileName->styleSheet(), QStringLiteral("")); // Valid image

		w.ui.cbFileEmbedd->clicked(true); // Embedding image

		SAVE_PROJECT("DatapickerTestProject");
	}

	QCOMPARE(QFile::exists(imgFileName), false); // Deleted because QTemporaryFile goes out of scope

	// Load project
	// Image was deleted before
	{
		Project project;
		QCOMPARE(project.load(savePath), true);

		auto datapicker = project.child<Datapicker>(0);
		QVERIFY(datapicker);
		auto* image = datapicker->image();

		DatapickerImageWidget w(nullptr);
		w.setImages({image});

		QCOMPARE(w.ui.cbFileEmbedd->isEnabled(), true);
		QCOMPARE(w.ui.cbFileRelativePath->isEnabled(), false); // project was loaded, but file does not exist
		QCOMPARE(w.ui.cbFileEmbedd->isChecked(), true);
		QCOMPARE(w.ui.cbFileRelativePath->isChecked(), false);

		QCOMPARE(image->fileName(), imgFileName);
		QCOMPARE(image->originalPlotImage.isNull(), false); // valid image loaded
		QCOMPARE(w.ui.leFileName->text(), imgFileName);
		QCOMPARE(w.ui.leFileName->styleSheet(), QStringLiteral("")); // Valid image
	}
}

void DatapickerTest::datapickerImageLoadImageEmbeddAbsoluteUndoRedo() {
	QString imgFileName;

	Project project;
	auto* datapicker = new Datapicker(QStringLiteral("Test"));
	project.addChild(datapicker);
	auto* image = datapicker->image();

	DatapickerImageWidget w(nullptr);
	w.setImages({image});

	{
		const auto img = createImage();

		QTemporaryFile imgFile(QStringLiteral("Testimage_XXXXXX.png"));
		QVERIFY(imgFile.open());
		imgFileName = imgFile.fileName();
		img.save(imgFileName, "PNG");
		QVERIFY(QFile::exists(imgFileName));

		QCOMPARE(w.ui.cbFileEmbedd->isEnabled(), false);
		QCOMPARE(w.ui.cbFileRelativePath->isEnabled(), false);
		QCOMPARE(w.ui.cbFileEmbedd->isChecked(), false);
		QCOMPARE(w.ui.cbFileRelativePath->isChecked(), false);

		w.ui.leFileName->setText(imgFileName);
		// w.fileNameChanged(); will be called

		QCOMPARE(w.ui.cbFileEmbedd->isEnabled(), true);
		QCOMPARE(w.ui.cbFileRelativePath->isEnabled(), false); // project is not saved
		QCOMPARE(w.ui.cbFileEmbedd->isChecked(), false);
		QCOMPARE(w.ui.cbFileRelativePath->isChecked(), false);

		QCOMPARE(image->fileName(), imgFileName);
		QCOMPARE(image->originalPlotImage.isNull(), false); // valid image loaded
		QCOMPARE(w.ui.leFileName->text(), imgFileName);
		QCOMPARE(w.ui.leFileName->styleSheet(), QStringLiteral("")); // Valid image

		w.ui.cbFileEmbedd->clicked(true); // Embedding image

		QString savePath;
		SAVE_PROJECT("DatapickerTestProject");
	}

	// Deleting image to verify embedding worked
	QCOMPARE(QFile::exists(imgFileName), false); // Deleted because QTemporaryFile goes out of scope

	image->undoStack()->undo();
	QCOMPARE(w.ui.cbFileEmbedd->isEnabled(), false);
	QCOMPARE(w.ui.cbFileEmbedd->isChecked(), false);
	QCOMPARE(image->originalPlotImage.isNull(), true); // image does not exist anymore
	QCOMPARE(w.ui.leFileName->text(), imgFileName);
	QVERIFY(w.ui.leFileName->styleSheet() != QStringLiteral("")); // image does not exist anymore

	image->undoStack()->redo();
	QCOMPARE(w.ui.cbFileEmbedd->isEnabled(), true);
	QCOMPARE(w.ui.cbFileEmbedd->isChecked(), true);
	QCOMPARE(image->originalPlotImage.isNull(), false); // image is embedded
	QCOMPARE(w.ui.leFileName->text(), imgFileName);
	QCOMPARE(w.ui.leFileName->styleSheet(), QStringLiteral("")); // image is embedded
}

void DatapickerTest::datapickerImageLoadImageEmbeddRelative() {
	QString savePath;
	QString imgFileName;
	DatapickerImageWidget w(nullptr);

	{
		const auto img = createImage();

		QTemporaryFile imgFile(QStringLiteral("Testimage_XXXXXX.png"));
		QVERIFY(imgFile.open());
		imgFileName = imgFile.fileName();
		img.save(imgFileName, "PNG");
		QVERIFY(QFile::exists(imgFileName));

		Project project;
		auto* datapicker = new Datapicker(QStringLiteral("Test"));
		project.addChild(datapicker);
		auto* image = datapicker->image();

		w.setImages({image});

		QCOMPARE(w.ui.cbFileEmbedd->isEnabled(), false);
		QCOMPARE(w.ui.cbFileRelativePath->isEnabled(), false);
		QCOMPARE(w.ui.cbFileEmbedd->isChecked(), false);
		QCOMPARE(w.ui.cbFileRelativePath->isChecked(), false);

		w.ui.leFileName->setText(imgFileName);
		// w.fileNameChanged(); will be called

		QCOMPARE(w.ui.cbFileEmbedd->isEnabled(), true);
		QCOMPARE(w.ui.cbFileRelativePath->isEnabled(), false); // project is not saved
		QCOMPARE(w.ui.cbFileEmbedd->isChecked(), false);
		QCOMPARE(w.ui.cbFileRelativePath->isChecked(), false);

		QCOMPARE(image->fileName(), imgFileName);
		QCOMPARE(image->originalPlotImage.isNull(), false); // valid image loaded
		QCOMPARE(w.ui.leFileName->text(), imgFileName);
		QCOMPARE(w.ui.leFileName->styleSheet(), QStringLiteral("")); // Valid image

		SAVE_PROJECT("DatapickerTestProject");

		QCOMPARE(w.ui.cbFileRelativePath->isEnabled(), true); // project is now saved so calculating the relative path is possible

		w.ui.cbFileRelativePath->clicked(true);

		// QCOMPARE(w.ui.cbFileRelativePath->isChecked(), true);
		QFileInfo fi(imgFileName);
		QCOMPARE(image->fileName(), fi.fileName());
		QCOMPARE(w.ui.leFileName->text(), fi.fileName());

		w.ui.cbFileEmbedd->clicked(true); // Embedding image

		SAVE_PROJECT("DatapickerTestProject"); // Save again to save relative path
	}

	// Deleting image
	QCOMPARE(QFile::exists(imgFileName), false); // Deleted because QTemporaryFile goes out of scope

	// Load project
	{
		Project project;
		QCOMPARE(project.load(savePath), true);

		auto datapicker = project.child<Datapicker>(0);
		QVERIFY(datapicker);
		auto* image = datapicker->image();

		DatapickerImageWidget w(nullptr);
		w.setImages({image});

		QCOMPARE(w.ui.cbFileEmbedd->isEnabled(), true); // image is valid because embedded
		QCOMPARE(w.ui.cbFileRelativePath->isEnabled(), false); // image is valid, but embed is turned on
		QCOMPARE(w.ui.cbFileEmbedd->isChecked(), true);
		QCOMPARE(w.ui.cbFileRelativePath->isChecked(), true);

		QFileInfo fi(imgFileName);
		QCOMPARE(image->fileName(), fi.fileName());
		QCOMPARE(image->originalPlotImage.isNull(), false); // valid image loaded
		QCOMPARE(w.ui.leFileName->text(), fi.fileName());
		QCOMPARE(w.ui.leFileName->styleSheet(), QStringLiteral("")); // Valid image
	}
}

void DatapickerTest::datapickerImageLoadImageEmbeddRelativeUndoRedo() {
	QString imgFileName;

	Project project;
	auto* datapicker = new Datapicker(QStringLiteral("Test"));
	project.addChild(datapicker);
	auto* image = datapicker->image();

	DatapickerImageWidget w(nullptr);
	w.setImages({image});

	{
		const auto img = createImage();

		QTemporaryFile imgFile(QStringLiteral("Testimage_XXXXXX.png"));
		QVERIFY(imgFile.open());
		imgFileName = imgFile.fileName();
		img.save(imgFileName, "PNG");
		QVERIFY(QFile::exists(imgFileName));

		QCOMPARE(w.ui.cbFileEmbedd->isEnabled(), false);
		QCOMPARE(w.ui.cbFileRelativePath->isEnabled(), false);
		QCOMPARE(w.ui.cbFileEmbedd->isChecked(), false);
		QCOMPARE(w.ui.cbFileRelativePath->isChecked(), false);

		w.ui.leFileName->setText(imgFileName);
		// w.fileNameChanged(); will be called

		QCOMPARE(w.ui.cbFileEmbedd->isEnabled(), true);
		QCOMPARE(w.ui.cbFileRelativePath->isEnabled(), false); // project is not saved
		QCOMPARE(w.ui.cbFileEmbedd->isChecked(), false);
		QCOMPARE(w.ui.cbFileRelativePath->isChecked(), false);

		QCOMPARE(image->fileName(), imgFileName);
		QCOMPARE(image->originalPlotImage.isNull(), false); // valid image loaded
		QCOMPARE(w.ui.leFileName->text(), imgFileName);
		QCOMPARE(w.ui.leFileName->styleSheet(), QStringLiteral("")); // Valid image

		QString savePath;
		SAVE_PROJECT("DatapickerTestProject");

		QCOMPARE(w.ui.cbFileRelativePath->isEnabled(), true); // project is now saved so calculating the relative path is possible

		w.ui.cbFileRelativePath->clicked(true);

		// QCOMPARE(w.ui.cbFileRelativePath->isChecked(), true);
		QFileInfo fi(imgFileName);
		QCOMPARE(image->fileName(), fi.fileName());
		QCOMPARE(w.ui.leFileName->text(), fi.fileName());

		w.ui.cbFileEmbedd->clicked(true); // Embedding image

		SAVE_PROJECT("DatapickerTestProject"); // Save again to save relative path
	}

	// Deleting image
	QCOMPARE(QFile::exists(imgFileName), false); // Deleted because QTemporaryFile goes out of scope

	image->undoStack()->undo();

	QFileInfo fi(imgFileName);
	QCOMPARE(w.ui.cbFileEmbedd->isEnabled(), false);
	QCOMPARE(w.ui.cbFileEmbedd->isChecked(), false);
	QCOMPARE(image->originalPlotImage.isNull(), true); // image does not exist anymore
	QCOMPARE(w.ui.leFileName->text(), fi.fileName());
	QVERIFY(w.ui.leFileName->styleSheet() != QStringLiteral("")); // image does not exist anymore

	image->undoStack()->redo();
	QCOMPARE(w.ui.cbFileEmbedd->isEnabled(), true);
	QCOMPARE(w.ui.cbFileEmbedd->isChecked(), true);
	QCOMPARE(image->originalPlotImage.isNull(), false); // image is embedded
	QCOMPARE(w.ui.leFileName->text(), fi.fileName());
	QCOMPARE(w.ui.leFileName->styleSheet(), QStringLiteral("")); // image is embedded
}

void DatapickerTest::datapickerImageClipboard() {
	const auto img = createImage();

	QTemporaryFile imgFileName(QStringLiteral("Testimage_XXXXXX.png"));
	QVERIFY(imgFileName.open());
	img.save(imgFileName.fileName(), "PNG");
	QVERIFY(QFile::exists(imgFileName.fileName()));

	Project project;
	auto* datapicker = new Datapicker(QStringLiteral("Test"));
	project.addChild(datapicker);
	auto* image = datapicker->image();

	DatapickerImageWidget w(nullptr);
	w.setImages({image});

	QCOMPARE(w.ui.cbFileEmbedd->isEnabled(), false);
	QCOMPARE(w.ui.cbFileRelativePath->isEnabled(), false);
	QCOMPARE(w.ui.cbFileEmbedd->isChecked(), false);
	QCOMPARE(w.ui.cbFileRelativePath->isChecked(), false);

	// Set image from clipboard
	image->setImage(img, QStringLiteral(""), true);

	QCOMPARE(w.ui.cbFileEmbedd->isEnabled(), true);
	QCOMPARE(w.ui.cbFileRelativePath->isEnabled(), false);
	QCOMPARE(w.ui.cbFileEmbedd->isChecked(), true);
	QCOMPARE(w.ui.cbFileRelativePath->isChecked(), false);

	QCOMPARE(w.ui.leFileName->isEnabled(), false);
}

void DatapickerTest::datapickerImageClipboardSelectImageFromPath() {
	const auto img = createImage();
	const auto img2 = createImage2();

	QTemporaryFile imgFileName(QStringLiteral("Testimage_XXXXXX.png"));
	QVERIFY(imgFileName.open());
	img.save(imgFileName.fileName(), "PNG");
	QVERIFY(QFile::exists(imgFileName.fileName()));

	Project project;
	auto* datapicker = new Datapicker(QStringLiteral("Test"));
	project.addChild(datapicker);
	auto* image = datapicker->image();

	DatapickerImageWidget w(nullptr);
	w.setImages({image});

	QCOMPARE(w.ui.cbFileEmbedd->isEnabled(), false);
	QCOMPARE(w.ui.cbFileRelativePath->isEnabled(), false);
	QCOMPARE(w.ui.cbFileEmbedd->isChecked(), false);
	QCOMPARE(w.ui.cbFileRelativePath->isChecked(), false);

	// Set image from clipboard
	image->setImage(img2, QStringLiteral(""), true);

	QCOMPARE(w.ui.cbFileEmbedd->isEnabled(), true);
	QCOMPARE(w.ui.cbFileRelativePath->isEnabled(), false);
	QCOMPARE(w.ui.cbFileEmbedd->isChecked(), true);
	QCOMPARE(w.ui.cbFileRelativePath->isChecked(), false);

	QCOMPARE(w.ui.leFileName->isEnabled(), false);

	QCOMPARE(image->originalPlotImage, img2);

	w.ui.leFileName->setText(imgFileName.fileName());

	// Embedded is still turned on
	QCOMPARE(w.ui.leFileName->isEnabled(), false);
	QCOMPARE(w.ui.cbFileEmbedd->isEnabled(), true);
	QCOMPARE(w.ui.cbFileEmbedd->isChecked(), true);

	QCOMPARE(image->originalPlotImage.isNull(), false); // valid image

	Q_EMIT w.ui.cbFileEmbedd->clicked(false);

	QCOMPARE(w.ui.leFileName->isEnabled(), true);
	QCOMPARE(w.ui.cbFileEmbedd->isEnabled(), true);
	QCOMPARE(w.ui.cbFileEmbedd->isChecked(), true);

	QCOMPARE(image->originalPlotImage.isNull(), false); // valid image

	QCOMPARE(image->originalPlotImage, img);
}

void DatapickerTest::saveLoad() {
	QString savePath;
	{
		Project project;
		auto* datapicker = new Datapicker(QStringLiteral("Test"));
		project.addChild(datapicker);
		auto* image = datapicker->image();

		// Set reference points
		datapicker->addNewPoint(QPointF(0, 1), image);
		datapicker->addNewPoint(QPointF(0, 0), image);
		datapicker->addNewPoint(QPointF(1, 0), image);

		auto ap = image->axisPoints();
		ap.type = DatapickerImage::GraphType::Linear;
		image->setAxisPoints(ap);

		DatapickerImageWidget w(nullptr);
		w.setImages({image});
		w.ui.sbPositionX1->setValue(0);
		w.ui.sbPositionY1->setValue(10);
		w.ui.sbPositionZ1->setValue(0);
		w.ui.sbPositionX2->setValue(0);
		w.ui.sbPositionY2->setValue(0);
		w.ui.sbPositionZ2->setValue(0);
		w.ui.sbPositionX3->setValue(10);
		w.ui.sbPositionY3->setValue(0);
		w.ui.sbPositionZ3->setValue(0);
		w.logicalPositionChanged();

		auto* curve = new DatapickerCurve(i18n("Curve"));
		curve->addDatasheet(image->axisPoints().type);
		datapicker->addChild(curve);

		datapicker->addNewPoint(QPointF(0.5, 0.5), curve); // updates the curve data
		VALUES_EQUAL(curve->posXColumn()->valueAt(0), 5.);
		VALUES_EQUAL(curve->posYColumn()->valueAt(0), 5.);

		datapicker->addNewPoint(QPointF(0.7, 0.65), curve); // updates the curve data
		VALUES_EQUAL(curve->posXColumn()->valueAt(1), 7.);
		VALUES_EQUAL(curve->posYColumn()->valueAt(1), 6.5);

		SAVE_PROJECT("DatapickerTestProject"); // Save again to save relative path
	}

	// Load project
	{
		Project project;
		QCOMPARE(project.load(savePath), true);

		// check the datapicker is loaded
		const auto* datapicker = project.child<Datapicker>(0);
		QVERIFY(datapicker);

		// check axis/reference points
		const auto& points = datapicker->image()->children<DatapickerPoint>(AbstractAspect::ChildIndexFlag::IncludeHidden);
		QCOMPARE(points.length(), 3);

		// check the curve
		const auto* curve = datapicker->child<DatapickerCurve>(i18n("Curve"));
		QVERIFY(curve);
		const auto& curvePoints = curve->children<DatapickerPoint>(AbstractAspect::ChildIndexFlag::IncludeHidden);
		QCOMPARE(curvePoints.length(), 2);
		QCOMPARE(curve->posXColumn()->rowCount(), 2);
		QCOMPARE(curve->posYColumn()->rowCount(), 2);
		VALUES_EQUAL(curve->posXColumn()->valueAt(0), 5.);
		VALUES_EQUAL(curve->posYColumn()->valueAt(0), 5.);
		VALUES_EQUAL(curve->posXColumn()->valueAt(1), 7.);
		VALUES_EQUAL(curve->posYColumn()->valueAt(1), 6.5);

		QCOMPARE(curve->children<Spreadsheet>(AbstractAspect::ChildIndexFlag::IncludeHidden).size(), 1);
		const auto* data = curve->children<Spreadsheet>(AbstractAspect::ChildIndexFlag::IncludeHidden).at(0);
		QCOMPARE(data->columnCount(), 2);
		QCOMPARE(data->column(0), curve->posXColumn());
		QCOMPARE(data->column(1), curve->posYColumn());
	}
}

QTEST_MAIN(DatapickerTest)
