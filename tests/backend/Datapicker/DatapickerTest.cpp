/*
	File                 : DatapickerTest.cpp
	Project              : LabPlot
	Description          : Tests for Datapicker
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2022 Martin Marmsoler <martin.marmsoler@gmail.com>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "DatapickerTest.h"
//#include "backend/core/Project.h"
//#include "backend/lib/trace.h"
//#include "backend/worksheet/TextLabel.h"
//#include "backend/worksheet/TextLabelPrivate.h"
//#include "kdefrontend/widgets/LabelWidget.h"
#include "backend/datapicker/DatapickerImage.h"
#include "backend/datapicker/Transform.h"

#define VECTOR3D_EQUAL(vec, ref)                                                                                                                               \
	VALUES_EQUAL(vec.x(), ref.x());                                                                                                                            \
	VALUES_EQUAL(vec.y(), ref.y());                                                                                                                            \
	VALUES_EQUAL(vec.z(), ref.z());

void DatapickerTest::mapTypeToCartesian() {
	{
		DatapickerImage::ReferencePoints points;
		points.type = DatapickerImage::GraphType::Cartesian;
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
		points.type = DatapickerImage::GraphType::LogarithmicNaturalX;
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
		points.type = DatapickerImage::GraphType::LogarithmicNaturalY;
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
		points.type = DatapickerImage::GraphType::LogarithmicNaturalXY;
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
		points.type = DatapickerImage::GraphType::Logarithmic10X;
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
		points.type = DatapickerImage::GraphType::Logarithmic10Y;
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
		points.type = DatapickerImage::GraphType::Logarithmic10XY;
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
		points.type = DatapickerImage::GraphType::Cartesian;
		QPointF point{5, 2343.23};

		Transform t;
		VECTOR3D_EQUAL(t.mapCartesianToType(point, points), QVector3D(5, 2343.23, 0));
	}

	{
		DatapickerImage::ReferencePoints points;
		points.type = DatapickerImage::GraphType::LogarithmicNaturalX;
		QPointF point{5, 2343.23};

		Transform t;
		VECTOR3D_EQUAL(t.mapCartesianToType(point, points), QVector3D(exp(5), 2343.23, 0));
	}

	{
		DatapickerImage::ReferencePoints points;
		points.type = DatapickerImage::GraphType::LogarithmicNaturalY;
		QPointF point{5, 2.23};

		Transform t;
		VECTOR3D_EQUAL(t.mapCartesianToType(point, points), QVector3D(5, exp(2.23), 0));
	}

	{
		DatapickerImage::ReferencePoints points;
		points.type = DatapickerImage::GraphType::LogarithmicNaturalXY;
		QPointF point{5, 2.23};

		Transform t;
		VECTOR3D_EQUAL(t.mapCartesianToType(point, points), QVector3D(exp(5), exp(2.23), 0));
	}

	// log10
	{
		DatapickerImage::ReferencePoints points;
		points.type = DatapickerImage::GraphType::Logarithmic10X;
		QPointF point{5, 2343.23};

		Transform t;
		VECTOR3D_EQUAL(t.mapCartesianToType(point, points), QVector3D(pow(10, 5), 2343.23, 0));
	}

	{
		DatapickerImage::ReferencePoints points;
		points.type = DatapickerImage::GraphType::Logarithmic10Y;
		QPointF point{5, 2.23};

		Transform t;
		VECTOR3D_EQUAL(t.mapCartesianToType(point, points), QVector3D(5, pow(10, 2.23), 0));
	}

	{
		DatapickerImage::ReferencePoints points;
		points.type = DatapickerImage::GraphType::Logarithmic10XY;
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
		points.type = DatapickerImage::GraphType::Logarithmic10XY;
		QPointF point{5, 2.23};

		Transform t;
		VECTOR3D_EQUAL(t.mapCartesianToType(point, points), QVector3D(pow(10, 5), pow(10, 2.23), 0));
	}
}

QTEST_MAIN(DatapickerTest)
