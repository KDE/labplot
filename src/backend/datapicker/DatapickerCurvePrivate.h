/*
	File                 : DatapickerCurvePrivate.h
	Project              : LabPlot
	Description          : Graphic Item for coordinate points of Datapicker
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2015 Ankit Wagadre <wagadre.ankit@gmail.com>
	SPDX-FileCopyrightText: 2015-2021 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef DATAPICKERCURVEPRIVATE_H
#define DATAPICKERCURVEPRIVATE_H

#include "backend/worksheet/Worksheet.h"

class Symbol;
class QBrush;
class QPen;

class DatapickerCurvePrivate {
public:
	explicit DatapickerCurvePrivate(DatapickerCurve* curve);

	QString name() const;

	DatapickerCurve* const q;
	void retransform();

	Symbol* symbol{nullptr};
	DatapickerCurve::Errors curveErrorTypes{DatapickerCurve::ErrorType::NoError, DatapickerCurve::ErrorType::NoError};
	QBrush pointErrorBarBrush;
	QPen pointErrorBarPen;
	qreal pointErrorBarSize{Worksheet::convertToSceneUnits(8, Worksheet::Unit::Point)};
	bool pointVisibility{true};

	AbstractColumn* posXColumn{nullptr};
	// Name of the column without path, since we know that it is a child of this element
	QString posXColumnName;
	AbstractColumn* posYColumn{nullptr};
	QString posYColumnName;
	AbstractColumn* posZColumn{nullptr};
	QString posZColumnName;
	AbstractColumn* plusDeltaXColumn{nullptr};
	QString plusDeltaXColumnName;
	AbstractColumn* minusDeltaXColumn{nullptr};
	QString minusDeltaXColumnName;
	AbstractColumn* plusDeltaYColumn{nullptr};
	QString plusDeltaYColumnName;
	AbstractColumn* minusDeltaYColumn{nullptr};
	QString minusDeltaYColumnName;

	friend class DatapickerTest;
};

#endif
