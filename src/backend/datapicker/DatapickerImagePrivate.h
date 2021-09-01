/*
    File                 : DatapickerImagePrivate.h
    Project              : LabPlot
    Description          : Worksheet for Datapicker, private class
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2015-2016 Ankit Wagadre (wagadre.ankit@gmail.com)
    SPDX-FileCopyrightText: 2015-2021 Alexander Semke (alexander.semke@web.de)

    SPDX-License-Identifier: GPL-2.0-or-later
*/
#ifndef DATAPICKERIMAGEPRIVATE_H
#define DATAPICKERIMAGEPRIVATE_H

class Symbol;
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

	//symbols
	Symbol* symbol{nullptr};
	bool pointVisibility{true};

	QString name() const;
	void retransform();
	void updateFileName();
	void discretize();
	void makeSegments();
	bool uploadImage(const QString&);
};

#endif
