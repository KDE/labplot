/*
    File                 : Segment.h
    Project              : LabPlot
    Description          : Graphics-item for curve of Datapicker
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2015 Ankit Wagadre (wagadre.ankit@gmail.com)
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef SEGMENT_H
#define SEGMENT_H

#include <QVector>
#include "backend/lib/macros.h"

class QGraphicsItem;
class QLine;

class SegmentPrivate;
class DatapickerImage;

class Segment {
public:
	explicit Segment(DatapickerImage*);

	QVector<QLine*> path;
	int yLast{0};
	int length{0};

	QGraphicsItem *graphicsItem() const;
	void setParentGraphicsItem(QGraphicsItem*);

	bool isVisible() const;
	void setVisible(bool);
	void retransform();

	typedef SegmentPrivate Private;

private:
	Q_DECLARE_PRIVATE(Segment)
	DatapickerImage* m_image;

protected:
	SegmentPrivate* const d_ptr;
};

#endif
