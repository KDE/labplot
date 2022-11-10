/*
	File                 : DatapickerTest.cpp
	Project              : LabPlot
	Description          : Tests for Datapicker
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2022 Martin Marmsoler <martin.marmsoler@gmail.com>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "DatapickerTest.h"
#include "backend/core/AbstractColumn.h"
#include "backend/datapicker/Datapicker.h"
#include "backend/datapicker/DatapickerCurve.h"
#include "backend/datapicker/DatapickerCurvePrivate.h"
#include "backend/datapicker/DatapickerImage.h"
#include "backend/datapicker/DatapickerPoint.h"
#include "backend/datapicker/Transform.h"
#include "kdefrontend/widgets/DatapickerImageWidget.h"

#define VECTOR3D_EQUAL(vec, ref)                                                                                                                               \
	VALUES_EQUAL(vec.x(), ref.x());                                                                                                                            \
	VALUES_EQUAL(vec.y(), ref.y());                                                                                                                            \
	VALUES_EQUAL(vec.z(), ref.z());

void DatapickerTest::mapTypeToCartesian() {
	{
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
		VALUES_EQUAL(t.x[0], 1);
		VALUES_EQUAL(t.y[0], 2);
		VALUES_EQUAL(t.x[1], 3);
		VALUES_EQUAL(t.y[1], 4);
		VALUES_EQUAL(t.x[2], 5);
		VALUES_EQUAL(t.y[2], 6);
		VALUES_EQUAL(t.X[0], 6.21);
		VALUES_EQUAL(t.Y[0], 7.23);
		VALUES_EQUAL(t.X[1], -51.2);
		VALUES_EQUAL(t.Y[1], 3234);
		VALUES_EQUAL(t.X[2], -23);
		VALUES_EQUAL(t.Y[2], +5e6);
	}

	// ln
	{
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
		VALUES_EQUAL(t.x[0], 1);
		VALUES_EQUAL(t.y[0], 2);
		VALUES_EQUAL(t.x[1], 2);
		VALUES_EQUAL(t.y[1], 4);
		VALUES_EQUAL(t.x[2], 3);
		VALUES_EQUAL(t.y[2], 6);
		VALUES_EQUAL(t.X[0], 6.21);
		VALUES_EQUAL(t.Y[0], 7.23);
		VALUES_EQUAL(t.X[1], -51.2);
		VALUES_EQUAL(t.Y[1], 3234);
		VALUES_EQUAL(t.X[2], -23);
		VALUES_EQUAL(t.Y[2], +5e6);
	}

	{
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
		VALUES_EQUAL(t.x[0], 1);
		VALUES_EQUAL(t.y[0], 1);
		VALUES_EQUAL(t.x[1], 3);
		VALUES_EQUAL(t.y[1], 2);
		VALUES_EQUAL(t.x[2], 5);
		VALUES_EQUAL(t.y[2], 3);
		VALUES_EQUAL(t.X[0], 6.21);
		VALUES_EQUAL(t.Y[0], 7.23);
		VALUES_EQUAL(t.X[1], -51.2);
		VALUES_EQUAL(t.Y[1], 3234);
		VALUES_EQUAL(t.X[2], -23);
		VALUES_EQUAL(t.Y[2], +5e6);
	}

	{
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
		VALUES_EQUAL(t.x[0], 5);
		VALUES_EQUAL(t.y[0], 1);
		VALUES_EQUAL(t.x[1], 6);
		VALUES_EQUAL(t.y[1], 2);
		VALUES_EQUAL(t.x[2], 7);
		VALUES_EQUAL(t.y[2], 3);
		VALUES_EQUAL(t.X[0], 6.21);
		VALUES_EQUAL(t.Y[0], 7.23);
		VALUES_EQUAL(t.X[1], -51.2);
		VALUES_EQUAL(t.Y[1], 3234);
		VALUES_EQUAL(t.X[2], -23);
		VALUES_EQUAL(t.Y[2], +5e6);
	}

	// log10
	{
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
		VALUES_EQUAL(t.x[0], 1);
		VALUES_EQUAL(t.y[0], 2);
		VALUES_EQUAL(t.x[1], 2);
		VALUES_EQUAL(t.y[1], 4);
		VALUES_EQUAL(t.x[2], 3);
		VALUES_EQUAL(t.y[2], 6);
		VALUES_EQUAL(t.X[0], 6.21);
		VALUES_EQUAL(t.Y[0], 7.23);
		VALUES_EQUAL(t.X[1], -51.2);
		VALUES_EQUAL(t.Y[1], 3234);
		VALUES_EQUAL(t.X[2], -23);
		VALUES_EQUAL(t.Y[2], +5e6);
	}

	{
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
		VALUES_EQUAL(t.x[0], 1);
		VALUES_EQUAL(t.y[0], 1);
		VALUES_EQUAL(t.x[1], 3);
		VALUES_EQUAL(t.y[1], 2);
		VALUES_EQUAL(t.x[2], 5);
		VALUES_EQUAL(t.y[2], 3);
		VALUES_EQUAL(t.X[0], 6.21);
		VALUES_EQUAL(t.Y[0], 7.23);
		VALUES_EQUAL(t.X[1], -51.2);
		VALUES_EQUAL(t.Y[1], 3234);
		VALUES_EQUAL(t.X[2], -23);
		VALUES_EQUAL(t.Y[2], +5e6);
	}

	{
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
		VALUES_EQUAL(t.x[0], 5);
		VALUES_EQUAL(t.y[0], 1);
		VALUES_EQUAL(t.x[1], 6);
		VALUES_EQUAL(t.y[1], 2);
		VALUES_EQUAL(t.x[2], 7);
		VALUES_EQUAL(t.y[2], 3);
		VALUES_EQUAL(t.X[0], 6.21);
		VALUES_EQUAL(t.Y[0], 7.23);
		VALUES_EQUAL(t.X[1], -51.2);
		VALUES_EQUAL(t.Y[1], 3234);
		VALUES_EQUAL(t.X[2], -23);
		VALUES_EQUAL(t.Y[2], +5e6);
	}

	// {
	//     DatapickerImage::ReferencePoints points;
	//     points.type = DatapickerImage::GraphType::PolarInRadians;
	//     points.logicalPos[0].setX(1);
	//     points.logicalPos[0].setY(0);
	//     points.logicalPos[1].setX(3);
	//     points.logicalPos[1].setY(2.1);
	//     points.logicalPos[2].setX(5);
	//     points.logicalPos[2].setY(3.8);
	//     points.scenePos[0].setX(6.21);
	//     points.scenePos[0].setY(7.23);
	//     points.scenePos[1].setX(-51.2);
	//     points.scenePos[1].setY(3234);
	//     points.scenePos[2].setX(-23);
	//     points.scenePos[2].setY(+5e6);

	// Transform t;
	// QCOMPARE(t.mapTypeToCartesian(points), true);
	// VALUES_EQUAL(t.x[0], 1);
	// VALUES_EQUAL(t.y[0], 0);
	// VALUES_EQUAL(t.x[1], -1.51454);
	// VALUES_EQUAL(t.y[1], 2.5896);
	// VALUES_EQUAL(t.x[2], -3.9548);
	// VALUES_EQUAL(t.y[2], 3.0593);
	// VALUES_EQUAL(t.X[0], 6.21);
	// VALUES_EQUAL(t.Y[0], 7.23);
	// VALUES_EQUAL(t.X[1], -51.2);
	// VALUES_EQUAL(t.Y[1], 3234);
	// VALUES_EQUAL(t.X[2], -23);
	// VALUES_EQUAL(t.Y[2], +5e6);
	// }

	// {
	//     DatapickerImage::ReferencePoints points;
	//     points.type = DatapickerImage::GraphType::PolarInDegree;
	//     points.logicalPos[0].setX(1);
	//     points.logicalPos[0].setY(0);
	//     points.logicalPos[1].setX(3);
	//     points.logicalPos[1].setY(30);
	//     points.logicalPos[2].setX(5);
	//     points.logicalPos[2].setY(50);
	//     points.scenePos[0].setX(6.21);
	//     points.scenePos[0].setY(7.23);
	//     points.scenePos[1].setX(-51.2);
	//     points.scenePos[1].setY(3234);
	//     points.scenePos[2].setX(-23);
	//     points.scenePos[2].setY(+5e6);

	// Transform t;
	// QCOMPARE(t.mapTypeToCartesian(points), true);
	// VALUES_EQUAL(t.x[0], 1);
	// VALUES_EQUAL(t.y[0], 0);
	// VALUES_EQUAL(t.x[1], 2.5981);
	// VALUES_EQUAL(t.y[1], 1.5);
	// VALUES_EQUAL(t.x[2], 3.2139);
	// VALUES_EQUAL(t.y[2], 3.8302);
	// VALUES_EQUAL(t.X[0], 6.21);
	// VALUES_EQUAL(t.Y[0], 7.23);
	// VALUES_EQUAL(t.X[1], -51.2);
	// VALUES_EQUAL(t.Y[1], 3234);
	// VALUES_EQUAL(t.X[2], -23);
	// VALUES_EQUAL(t.Y[2], +5e6);
	// }
	// TODO: implement ternary
}

void DatapickerTest::mapCartesianToType() {
	{
		DatapickerImage::ReferencePoints points;
		points.type = DatapickerImage::GraphType::Linear;
		QPointF point{5, 2343.23};

		Transform t;
		VECTOR3D_EQUAL(t.mapCartesianToType(point, points), QVector3D(5, 2343.23, 0));
	}

	{
		DatapickerImage::ReferencePoints points;
		points.type = DatapickerImage::GraphType::LnX;
		QPointF point{5, 2343.23};

		Transform t;
		VECTOR3D_EQUAL(t.mapCartesianToType(point, points), QVector3D(exp(5), 2343.23, 0));
	}

	{
		DatapickerImage::ReferencePoints points;
		points.type = DatapickerImage::GraphType::LnY;
		QPointF point{5, 2.23};

		Transform t;
		VECTOR3D_EQUAL(t.mapCartesianToType(point, points), QVector3D(5, exp(2.23), 0));
	}

	{
		DatapickerImage::ReferencePoints points;
		points.type = DatapickerImage::GraphType::LnXY;
		QPointF point{5, 2.23};

		Transform t;
		VECTOR3D_EQUAL(t.mapCartesianToType(point, points), QVector3D(exp(5), exp(2.23), 0));
	}

	// log10
	{
		DatapickerImage::ReferencePoints points;
		points.type = DatapickerImage::GraphType::Log10X;
		QPointF point{5, 2343.23};

		Transform t;
		VECTOR3D_EQUAL(t.mapCartesianToType(point, points), QVector3D(pow(10, 5), 2343.23, 0));
	}

	{
		DatapickerImage::ReferencePoints points;
		points.type = DatapickerImage::GraphType::Log10Y;
		QPointF point{5, 2.23};

		Transform t;
		VECTOR3D_EQUAL(t.mapCartesianToType(point, points), QVector3D(5, pow(10, 2.23), 0));
	}

	{
		DatapickerImage::ReferencePoints points;
		points.type = DatapickerImage::GraphType::Log10XY;
		QPointF point{5, 2.23};

		Transform t;
		VECTOR3D_EQUAL(t.mapCartesianToType(point, points), QVector3D(pow(10, 5), pow(10, 2.23), 0));
	}

	// {
	//     DatapickerImage::ReferencePoints points;
	//     points.type = DatapickerImage::GraphType::PolarInDegree;
	//     QPointF point{5, 30};

	// Transform t;
	// VECTOR3D_EQUAL(t.mapCartesianToType(point, points), QVector3D(30.4138, 0.1651, 0));
	// }

	// {
	//     DatapickerImage::ReferencePoints points;
	//     points.type = DatapickerImage::GraphType::PolarInRadians;
	//     QPointF point{5, 30};

	// Transform t;
	// VECTOR3D_EQUAL(t.mapCartesianToType(point, points), QVector3D(1.4056, 30.414, 0));
	// }

	// TODO: implement Ternary
}

void DatapickerTest::mapSceneToLogical() {
	{
		DatapickerImage::ReferencePoints points;
		points.type = DatapickerImage::GraphType::Log10XY;
		QPointF point{5, 2.23};

		Transform t;
		VECTOR3D_EQUAL(t.mapCartesianToType(point, points), QVector3D(pow(10, 5), pow(10, 2.23), 0));
	}
}

void DatapickerTest::cartesianMapping() {
	Datapicker datapicker(QStringLiteral("Test"));
	datapicker.addNewPoint(QPointF(0, 1), datapicker.m_image);
	datapicker.addNewPoint(QPointF(0, 0), datapicker.m_image);
	datapicker.addNewPoint(QPointF(1, 0), datapicker.m_image);

	auto ap = datapicker.m_image->axisPoints();
	ap.type = DatapickerImage::GraphType::Linear;
	datapicker.m_image->setAxisPoints(ap);

	DatapickerImageWidget w(nullptr);
	w.setImages({datapicker.m_image});
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
	curve->addDatasheet(datapicker.m_image->axisPoints().type);
	datapicker.addChild(curve);

	datapicker.addNewPoint(QPointF(0.5, 0.5), curve); // updates the curve data
	VALUES_EQUAL(curve->d_ptr->posXColumn->valueAt(0), 5);
	VALUES_EQUAL(curve->d_ptr->posYColumn->valueAt(0), 5);

	datapicker.addNewPoint(QPointF(0.7, 0.65), curve); // updates the curve data
	VALUES_EQUAL(curve->d_ptr->posXColumn->valueAt(1), 7);
	VALUES_EQUAL(curve->d_ptr->posYColumn->valueAt(1), 6.5);
}

void DatapickerTest::logarithmicNaturalXMapping() {
	Datapicker datapicker(QStringLiteral("Test"));
	datapicker.addNewPoint(QPointF(3, 10), datapicker.m_image);
	datapicker.addNewPoint(QPointF(3, 0), datapicker.m_image);
	datapicker.addNewPoint(QPointF(13, 0), datapicker.m_image);

	auto ap = datapicker.m_image->axisPoints();
	ap.type = DatapickerImage::GraphType::LnX;
	datapicker.m_image->setAxisPoints(ap);

	DatapickerImageWidget w(nullptr);
	w.setImages({datapicker.m_image});
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
	curve->addDatasheet(datapicker.m_image->axisPoints().type);
	datapicker.addChild(curve);

	datapicker.addNewPoint(QPointF(5, 6), curve); // updates the curve data
	VALUES_EQUAL(curve->d_ptr->posXColumn->valueAt(0), 0); // x start is zero, which is not valid therefore the result is 0
	VALUES_EQUAL(curve->d_ptr->posYColumn->valueAt(0), 0); // x start is zero, which is not valid therefore the result is 0

	w.ui.sbPositionX1->setValue(1); // axisPointsChanged will call updatePoint()
	w.ui.sbPositionX2->setValue(1); // axisPointsChanged will call updatePoint()

	// Value validated manually, not reverse calculated
	VALUES_EQUAL(curve->d_ptr->posXColumn->valueAt(0), 2.51188635826);
	VALUES_EQUAL(curve->d_ptr->posYColumn->valueAt(0), 60);
}

void DatapickerTest::logarithmicNaturalYMapping() {
	Datapicker datapicker(QStringLiteral("Test"));
	datapicker.addNewPoint(QPointF(3, 10), datapicker.m_image);
	datapicker.addNewPoint(QPointF(3, 0), datapicker.m_image);
	datapicker.addNewPoint(QPointF(13, 0), datapicker.m_image);

	auto ap = datapicker.m_image->axisPoints();
	ap.type = DatapickerImage::GraphType::LnY;
	datapicker.m_image->setAxisPoints(ap);

	DatapickerImageWidget w(nullptr);
	w.setImages({datapicker.m_image});
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
	curve->addDatasheet(datapicker.m_image->axisPoints().type);
	datapicker.addChild(curve);

	datapicker.addNewPoint(QPointF(5, 6), curve); // updates the curve data
	VALUES_EQUAL(curve->d_ptr->posXColumn->valueAt(0), 0); // x start is zero, which is not valid therefore the result is 0
	VALUES_EQUAL(curve->d_ptr->posYColumn->valueAt(0), 0); // x start is zero, which is not valid therefore the result is 0

	w.ui.sbPositionY2->setValue(1); // axisPointsChanged will call updatePoint()
	w.ui.sbPositionY3->setValue(1); // axisPointsChanged will call updatePoint()

	VALUES_EQUAL(curve->d_ptr->posXColumn->valueAt(0), 20);
	// Value validated manually, not reverse calculated
	VALUES_EQUAL(curve->d_ptr->posYColumn->valueAt(0), 15.8489322662);
}

void DatapickerTest::logarithmicNaturalXYMapping() {
	Datapicker datapicker(QStringLiteral("Test"));
	datapicker.addNewPoint(QPointF(3, 10), datapicker.m_image);
	datapicker.addNewPoint(QPointF(3, 0), datapicker.m_image);
	datapicker.addNewPoint(QPointF(13, 0), datapicker.m_image);

	auto ap = datapicker.m_image->axisPoints();
	ap.type = DatapickerImage::GraphType::LnXY;
	datapicker.m_image->setAxisPoints(ap);

	DatapickerImageWidget w(nullptr);
	w.setImages({datapicker.m_image});
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
	curve->addDatasheet(datapicker.m_image->axisPoints().type);
	datapicker.addChild(curve);

	datapicker.addNewPoint(QPointF(5, 6), curve); // updates the curve data
	VALUES_EQUAL(curve->d_ptr->posXColumn->valueAt(0), 0); // x start is zero, which is not valid therefore the result is 0
	VALUES_EQUAL(curve->d_ptr->posYColumn->valueAt(0), 0); // x start is zero, which is not valid therefore the result is 0

	w.ui.sbPositionX1->setValue(1); // axisPointsChanged will call updatePoint()
	w.ui.sbPositionX2->setValue(1); // axisPointsChanged will call updatePoint()
	w.ui.sbPositionY2->setValue(1); // axisPointsChanged will call updatePoint()
	w.ui.sbPositionY3->setValue(1); // axisPointsChanged will call updatePoint()

	// Values validated manually, not reverse calculated
	VALUES_EQUAL(curve->d_ptr->posXColumn->valueAt(0), 2.51188635826);
	VALUES_EQUAL(curve->d_ptr->posYColumn->valueAt(0), 15.8489322662);
}

void DatapickerTest::logarithmic10XMapping() {
	Datapicker datapicker(QStringLiteral("Test"));
	datapicker.addNewPoint(QPointF(3, 10), datapicker.m_image);
	datapicker.addNewPoint(QPointF(3, 0), datapicker.m_image);
	datapicker.addNewPoint(QPointF(13, 0), datapicker.m_image);

	auto ap = datapicker.m_image->axisPoints();
	ap.type = DatapickerImage::GraphType::Log10X;
	datapicker.m_image->setAxisPoints(ap);

	DatapickerImageWidget w(nullptr);
	w.setImages({datapicker.m_image});
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
	curve->addDatasheet(datapicker.m_image->axisPoints().type);
	datapicker.addChild(curve);

	datapicker.addNewPoint(QPointF(5, 6), curve); // updates the curve data
	VALUES_EQUAL(curve->d_ptr->posXColumn->valueAt(0), 0); // x start is zero, which is not valid therefore the result is 0
	VALUES_EQUAL(curve->d_ptr->posYColumn->valueAt(0), 0); // x start is zero, which is not valid therefore the result is 0

	w.ui.sbPositionX1->setValue(1); // axisPointsChanged will call updatePoint()
	w.ui.sbPositionX2->setValue(1); // axisPointsChanged will call updatePoint()

	QCOMPARE(datapicker.m_image->axisPoints().type, DatapickerImage::GraphType::Log10X);

	// Value validated manually, not reverse calculated
	VALUES_EQUAL(curve->d_ptr->posXColumn->valueAt(0), 2.51188635826); // TODO: correct?
	VALUES_EQUAL(curve->d_ptr->posYColumn->valueAt(0), 60);
}

void DatapickerTest::logarithmic10YMapping() {
	Datapicker datapicker(QStringLiteral("Test"));
	datapicker.addNewPoint(QPointF(3, 10), datapicker.m_image);
	datapicker.addNewPoint(QPointF(3, 0), datapicker.m_image);
	datapicker.addNewPoint(QPointF(13, 0), datapicker.m_image);

	auto ap = datapicker.m_image->axisPoints();
	ap.type = DatapickerImage::GraphType::Log10Y;
	datapicker.m_image->setAxisPoints(ap);

	DatapickerImageWidget w(nullptr);
	w.setImages({datapicker.m_image});
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
	curve->addDatasheet(datapicker.m_image->axisPoints().type);
	datapicker.addChild(curve);

	datapicker.addNewPoint(QPointF(5, 6), curve); // updates the curve data
	VALUES_EQUAL(curve->d_ptr->posXColumn->valueAt(0), 0); // x start is zero, which is not valid therefore the result is 0
	VALUES_EQUAL(curve->d_ptr->posYColumn->valueAt(0), 0); // x start is zero, which is not valid therefore the result is 0

	w.ui.sbPositionY2->setValue(1); // axisPointsChanged will call updatePoint()
	w.ui.sbPositionY3->setValue(1); // axisPointsChanged will call updatePoint()

	QCOMPARE(datapicker.m_image->axisPoints().type, DatapickerImage::GraphType::Log10Y);

	VALUES_EQUAL(curve->d_ptr->posXColumn->valueAt(0), 20);
	// Value validated manually, not reverse calculated
	VALUES_EQUAL(curve->d_ptr->posYColumn->valueAt(0), 15.8489322662);
}

void DatapickerTest::logarithmic10XYMapping() {
	Datapicker datapicker(QStringLiteral("Test"));
	datapicker.addNewPoint(QPointF(3, 10), datapicker.m_image);
	datapicker.addNewPoint(QPointF(3, 0), datapicker.m_image);
	datapicker.addNewPoint(QPointF(13, 0), datapicker.m_image);

	auto ap = datapicker.m_image->axisPoints();
	ap.type = DatapickerImage::GraphType::Log10XY;
	datapicker.m_image->setAxisPoints(ap);

	DatapickerImageWidget w(nullptr);
	w.setImages({datapicker.m_image});
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
	curve->addDatasheet(datapicker.m_image->axisPoints().type);
	datapicker.addChild(curve);

	datapicker.addNewPoint(QPointF(5, 6), curve); // updates the curve data
	VALUES_EQUAL(curve->d_ptr->posXColumn->valueAt(0), 0); // x start is zero, which is not valid therefore the result is 0
	VALUES_EQUAL(curve->d_ptr->posYColumn->valueAt(0), 0); // x start is zero, which is not valid therefore the result is 0

	w.ui.sbPositionX1->setValue(1); // axisPointsChanged will call updatePoint()
	w.ui.sbPositionX2->setValue(1); // axisPointsChanged will call updatePoint()
	w.ui.sbPositionY2->setValue(1); // axisPointsChanged will call updatePoint()
	w.ui.sbPositionY3->setValue(1); // axisPointsChanged will call updatePoint()

	// Values validated manually, not reverse calculated
	VALUES_EQUAL(curve->d_ptr->posXColumn->valueAt(0), 2.51188635826);
	VALUES_EQUAL(curve->d_ptr->posYColumn->valueAt(0), 15.8489322662);
}

// ALEX uncomment and fix ;)
void DatapickerTest::referenceMove() {
	// // Reference points are moved after placement
	// // Column data is changing respectivetly
	// Datapicker datapicker(QStringLiteral("Test"));
	// datapicker.addNewPoint(QPointF(0, 1), datapicker.m_image);
	// datapicker.addNewPoint(QPointF(0, 0), datapicker.m_image);
	// datapicker.addNewPoint(QPointF(1, 0), datapicker.m_image);

	// auto ap = datapicker.m_image->axisPoints();
	// ap.type = DatapickerImage::GraphType::Linear;
	// datapicker.m_image->setAxisPoints(ap);

	// DatapickerImageWidget w(nullptr);
	// w.setImages({datapicker.m_image});
	// w.ui.sbPositionX1->setValue(0);
	// w.ui.sbPositionY1->setValue(10);
	// w.ui.sbPositionZ1->setValue(0);
	// w.ui.sbPositionX2->setValue(0);
	// w.ui.sbPositionY2->setValue(0);
	// w.ui.sbPositionZ2->setValue(0);
	// w.ui.sbPositionX3->setValue(10);
	// w.ui.sbPositionY3->setValue(0);
	// w.ui.sbPositionZ3->setValue(0);
	// w.logicalPositionChanged();

	// auto* curve = new DatapickerCurve(i18n("Curve"));
	// curve->addDatasheet(datapicker.m_image->axisPoints().type);
	// datapicker.addChild(curve);

	// datapicker.addNewPoint(QPointF(0.5, 0.6), curve); // updates the curve data
	// VALUES_EQUAL(curve->d_ptr->posXColumn->valueAt(0), 5);
	// VALUES_EQUAL(curve->d_ptr->posYColumn->valueAt(0), 6);

	// // Points are stored in the image
	// auto points = datapicker.m_image->children<DatapickerPoint>(AbstractAspect::ChildIndexFlag::IncludeHidden);
	// QCOMPARE(points.count(), 3);

	// points[0]->setPosition(QPointF(0.1, 1));
	// points[0]->setPosition(QPointF(0.1, 0));
	// points[0]->setPosition(QPointF(1.1, 0));

	// VALUES_EQUAL(curve->d_ptr->posXColumn->valueAt(0), 5/1.1);
	// VALUES_EQUAL(curve->d_ptr->posYColumn->valueAt(0), 6);
}

// ALEX uncomment and fix ;)
void DatapickerTest::curvePointMove() {
	// // Curve point was moved after placement
	// // Column data is changing respectivetly
	//   // Reference points are moved after placement
	//   // Column data is changing respectivetly
	//   Datapicker datapicker(QStringLiteral("Test"));
	//   datapicker.addNewPoint(QPointF(0, 1), datapicker.m_image);
	//   datapicker.addNewPoint(QPointF(0, 0), datapicker.m_image);
	//   datapicker.addNewPoint(QPointF(1, 0), datapicker.m_image);

	// auto ap = datapicker.m_image->axisPoints();
	// ap.type = DatapickerImage::GraphType::Linear;
	// datapicker.m_image->setAxisPoints(ap);

	// DatapickerImageWidget w(nullptr);
	// w.setImages({datapicker.m_image});
	// w.ui.sbPositionX1->setValue(0);
	// w.ui.sbPositionY1->setValue(10);
	// w.ui.sbPositionZ1->setValue(0);
	// w.ui.sbPositionX2->setValue(0);
	// w.ui.sbPositionY2->setValue(0);
	// w.ui.sbPositionZ2->setValue(0);
	// w.ui.sbPositionX3->setValue(10);
	// w.ui.sbPositionY3->setValue(0);
	// w.ui.sbPositionZ3->setValue(0);
	// w.logicalPositionChanged();

	// auto* curve = new DatapickerCurve(i18n("Curve"));
	// curve->addDatasheet(datapicker.m_image->axisPoints().type);
	// datapicker.addChild(curve);

	// datapicker.addNewPoint(QPointF(0.5, 0.6), curve); // updates the curve data
	// VALUES_EQUAL(curve->d_ptr->posXColumn->valueAt(0), 5);
	// VALUES_EQUAL(curve->d_ptr->posYColumn->valueAt(0), 6);

	// // Points are stored in the image
	// auto points = curve->children<DatapickerPoint>(AbstractAspect::ChildIndexFlag::IncludeHidden);
	// QCOMPARE(points.count(), 1);

	// points[0]->setPosition(QPointF(0.2, 0.9)); // Changing the position of the point

	// VALUES_EQUAL(curve->d_ptr->posXColumn->valueAt(0), 2);
	// VALUES_EQUAL(curve->d_ptr->posYColumn->valueAt(0), 9);
}

QTEST_MAIN(DatapickerTest)
