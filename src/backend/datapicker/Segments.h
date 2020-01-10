/***************************************************************************
    File                 : Segments.h
    Project              : LabPlot
    Description          : Contain Methods to trace curve of image/plot
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

#ifndef SEGMENTS_H
#define SEGMENTS_H

#include <QVector>

class QImage;
class DatapickerImage;
class Segment;

class Segments {

public:
	explicit Segments(DatapickerImage*);

	void makeSegments(QImage&);
	void setSegmentsVisible(bool);
	void setAcceptHoverEvents(bool);

private:
	DatapickerImage* m_image;
	QVector<Segment*> segments;

	void clearSegments();
	void loadSegment(Segment**, int);
	int adjacentRuns(bool*, int, int, int);
	Segment* adjacentSegment(Segment**, int, int, int);
	int adjacentSegments(Segment**, int, int, int);
	void matchRunsToSegments(int, int, bool*, Segment**, bool*, Segment**, bool*);
	void finishRun(bool*, bool*, Segment**, Segment**, int, int, int, int);
	void scrollSegment(Segment**, Segment**, int);
	void removeUnneededLines(Segment**, Segment**, int);
};
#endif
