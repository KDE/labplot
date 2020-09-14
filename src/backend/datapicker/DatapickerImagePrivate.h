/***************************************************************************
  File                 : DatapickerImagePrivate.h
  Project              : LabPlot
  Description          : Worksheet for Datapicker, private class
  --------------------------------------------------------------------
  Copyright            : (C) 2015-2016 by Ankit Wagadre (wagadre.ankit@gmail.com)
  Copyright            : (C) 2015-2016 by Alexander Semke (alexander.semke@web.de)

***************************************************************************/
/***************************************************************************
 *                                                                         *
 *  This program is free software; you can redistribute it and/or modify   *
 *  it under the terms of the GNU General Public License as published by   *
 *  the Free Software Foundation; either version 2 of the License, or      *
 *  (at your option) any later version.                                    *
 *                                                                         *
 *  This program is distributed in the hope that it will be useful,        *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *  GNU General Public License for more details.                           *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the Free Software           *
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor,                    *
 *   Boston, MA  02110-1301  USA                                           *
 *                                                                         *
 ***************************************************************************/
#ifndef DATAPICKERIMAGEPRIVATE_H
#define DATAPICKERIMAGEPRIVATE_H

class QBrush;
class QGraphicsScene;

class DatapickerImagePrivate {
public:
	explicit DatapickerImagePrivate(DatapickerImage*);
	virtual ~DatapickerImagePrivate();

	DatapickerImage::ReferencePoints axisPoints;
	DatapickerImage::EditorSettings settings;
	DatapickerImage::PointsType plotPointsType{DatapickerImage::PointsType::AxisPoints};
	DatapickerImage::PlotImageType plotImageType{DatapickerImage::PlotImageType::NoImage};

	DatapickerImage* const q;
	QRectF pageRect;
	QGraphicsScene* m_scene;
	float rotationAngle{0.0};
	QString fileName;
	int pointSeparation{30};
	int minSegmentLength{30};

	qreal pointRotationAngle{0.0};
	Symbol::Style pointStyle;
	QBrush pointBrush;
	QPen pointPen;
	qreal pointOpacity;
	qreal pointSize;
	bool pointVisibility{true};

	QString name() const;
	void retransform();
	void updateFileName();
	void discretize();
	void makeSegments();
	bool uploadImage(const QString&);
};

#endif
