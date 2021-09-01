/*
    File                 : Segments.h
    Project              : LabPlot
    Description          : Contain Methods to trace curve of image/plot
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2015 Ankit Wagadre <wagadre.ankit@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

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
