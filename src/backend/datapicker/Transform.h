/*
    File                 : Transform.h
    Project              : LabPlot
    Description          : transformation for mapping between scene and
    logical coordinates of Datapicker-image
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2015 Ankit Wagadre <wagadre.ankit@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef TRANSFORM_H
#define TRANSFORM_H

#include "backend/datapicker/DatapickerImage.h"

class Transform {
public:
	Transform() = default;
	QVector3D mapSceneToLogical(QPointF,const DatapickerImage::ReferencePoints&);
	QVector3D mapSceneLengthToLogical(QPointF,const DatapickerImage::ReferencePoints&);

private:
	bool mapTypeToCartesian(const DatapickerImage::ReferencePoints&);
	QVector3D mapCartesianToType(QPointF, const DatapickerImage::ReferencePoints&) const;

	//logical coordinates
	double x[4];
	double y[4];

	//Scene coordinates
	double X[4];
	double Y[4];

};

#endif // TRANSFORM_H
