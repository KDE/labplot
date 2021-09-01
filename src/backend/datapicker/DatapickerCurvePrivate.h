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

class Symbol;
class QBrush;
class QPen;

class DatapickerCurvePrivate {
public:
	explicit DatapickerCurvePrivate(DatapickerCurve* curve);

	QString name() const;

	DatapickerCurve* const q;
	void retransform();

	Symbol* symbol;
	DatapickerCurve::Errors curveErrorTypes;
	QBrush pointErrorBarBrush;
	QPen pointErrorBarPen;
	qreal pointErrorBarSize;
	bool pointVisibility;

	AbstractColumn* posXColumn{nullptr};
	QString posXColumnPath;
	AbstractColumn* posYColumn{nullptr};
	QString posYColumnPath;
	AbstractColumn* posZColumn{nullptr};
	QString posZColumnPath;
	AbstractColumn* plusDeltaXColumn{nullptr};
	QString plusDeltaXColumnPath;
	AbstractColumn* minusDeltaXColumn{nullptr};
	QString minusDeltaXColumnPath;
	AbstractColumn* plusDeltaYColumn{nullptr};
	QString plusDeltaYColumnPath;
	AbstractColumn* minusDeltaYColumn{nullptr};
	QString minusDeltaYColumnPath;
};

#endif
