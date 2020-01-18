/***************************************************************************
    File                 : Segment.h
    Project              : LabPlot
    Description          : Graphics-item for curve of Datapicker
    --------------------------------------------------------------------
    Copyright            : (C) 2015 by Ankit Wagadre (wagadre.ankit@gmail.com)
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
