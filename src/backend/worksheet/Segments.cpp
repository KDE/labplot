
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

#include "Segments.h"
#include "backend/worksheet/Segment.h"
#include "backend/core/ImageEditor.h"

#include <QImage>
#include <QLine>

/**
 * \class Segments
 * \brief container to open image/plot.
 *
 * this class is a container for all of the segment. a segment is a set
 * of linked lines that run along a curve in the original image. the
 * main complication is that curves in the original image cross each other
 * and other things like grid lines. we rely on the user to link
 * multiple segments together to get points along the entire curve length
 * * \ingroup datapicker
 */

Segments::Segments(Image* image): m_image(image) {
}

Segments::~Segments() {
}

/*!
    segments are built when the original image is loaded. they start out hidden
     and remain so until showSegments is called
*/
void Segments::makeSegments(QImage &imageProcessed) {
    clearSegments();

    int width = imageProcessed.width();
    int height = imageProcessed.height();

    // for each new column of pixels, loop through the runs. a run is defined as
    // one or more colored pixels that are all touching, with one uncolored pixel or the
    // image boundary at each end of the set. for each set in the current column, count
    // the number of runs it touches in the adjacent (left and right) columns. here is
    // the pseudocode:
    //   if ((L > 1) || (R > 1))
    //     "this run is at a branch point so ignore the set"
    //   else
    //     if (L == 0)
    //       "this run is the start of a new segment"
    //     else
    //       "this run is appended to the segment on the left

    bool* lastBool = new bool [height];
    bool* currBool = new bool [height];
    bool* nextBool = new bool [height];
    Segment** lastSegment = new Segment* [height];
    Segment** currSegment = new Segment* [height];
    loadSegment(lastSegment, height);

    ImageEditor editor;
    loadBool(&editor, lastBool, &imageProcessed, -1);
    loadBool(&editor, currBool, &imageProcessed, 0);
    loadBool(&editor, nextBool, &imageProcessed, 1);

    for (int x = 0; x < width; x++) {
        matchRunsToSegments(x, height, lastBool, lastSegment, currBool, currSegment, nextBool);

        // get ready for next column
        scrollBool(lastBool, currBool, height);
        scrollBool(currBool, nextBool, height);
        if (x + 1 < width)
            loadBool(&editor, nextBool, &imageProcessed, x + 1);
        scrollSegment(lastSegment, currSegment, height);
    }

    delete[] lastBool;
    delete[] currBool;
    delete[] nextBool;
    delete[] lastSegment;
    delete[] currSegment;
}

/*!
  scroll the boolean flags of the right column into the left column
*/
void Segments::scrollBool(bool* left, bool* right, int height) {
    for (int y = 0; y < height; y++)
        left [y] = right [y];
}

/*!
    scroll the segment pointers of the right column into the left column
*/
void Segments::scrollSegment(Segment** left, Segment** right, int height) {
    for (int y = 0; y < height; y++)
        left [y] = right [y];
}

/*!
    identify the runs in a column, and connect them to segments
*/
void Segments::matchRunsToSegments(int x, int height, bool* lastBool, Segment** lastSegment,
                                   bool* currBool, Segment** currSegment, bool* nextBool) {
    loadSegment(currSegment, height);

    int yStart = 0;
    bool inRun = false;
    for (int y = 0; y < height; y++) {
        if (!inRun && currBool [y]) {
            inRun = true;
            yStart = y;
        }

        if ((y + 1 >= height) || !currBool [y + 1]) {
            if (inRun)
                finishRun(lastBool, nextBool, lastSegment, currSegment, x, yStart, y, height);

            inRun = false;
        }
    }

    removeUnneededLines(lastSegment, currSegment, height);
}

/*!
    remove unneeded lines belonging to segments that just finished in the previous column.
    the results of this function are displayed in the debug spew of makeSegments
*/
void Segments::removeUnneededLines(Segment** lastSegment, Segment** currSegment, int height) {
    Segment* segLast = 0;
    for (int yLast = 0; yLast < height; yLast++) {
        if (lastSegment [yLast] && (lastSegment [yLast] != segLast)) {
            segLast = lastSegment [yLast];

            bool found = false;
            for (int yCur = 0; yCur < height; yCur++)
                if (segLast == currSegment [yCur]) {
                    found = true;
                    break;
                }

            if (!found) {
                if (segLast->length < m_image->minSegmentLength()) {
                    // remove whole segment since it is too short
                    m_image->scene()->removeItem(segLast->graphicsItem());
                    segments.removeOne(segLast);
                }
            }
        }
    }
}

/*!
    initialize one column of boolean flags using the pixels of the specified column
*/
void Segments::loadBool(const ImageEditor* editor, bool* columnBool,
                        QImage* image, int x) {
    for (int y = 0; y < image->height(); y++)
        if (x < 0)
            columnBool [y] = false;
        else
            columnBool [y] = editor->processedPixelIsOn(*image, x, y);
}

/*!
    initialize one column of segment pointers
*/
void Segments::loadSegment(Segment** columnSegment, int height) {
    for (int y = 0; y < height; y++)
        columnSegment [y] = 0;
}

void Segments::clearSegments() {
    foreach(Segment* seg, segments)
        m_image->scene()->removeItem(seg->graphicsItem());

    segments.clear();
}

/*!
    set segments visible
*/
void Segments::setSegmentsVisible(bool on) {
    foreach(Segment* seg, segments)
        seg->setVisible(on);
}

/*!
    process a run of pixels. if there are fewer than two adjacent pixel runs on
    either side, this run will be added to an existing segment, or the start of
    a new segment
*/
void Segments::finishRun(bool* lastBool, bool* nextBool, Segment** lastSegment, Segment** currSegment,
                         int x, int yStart, int yStop, int height) {

    // count runs that touch on the left
    if (adjacentRuns(lastBool, yStart, yStop, height) > 1)
        return;

    // count runs that touch on the right
    if (adjacentRuns(nextBool, yStart, yStop, height) > 1)
        return;

    Segment* seg;
    int y = (int) ((yStart + yStop) / 2);
    if (adjacentSegments(lastSegment, yStart, yStop, height) == 0) {
        seg = new Segment(m_image);
        QLine* line = new QLine(QPoint(x, y),QPoint( x, y));
        seg->path.append(line);
        seg->yLast  = y;
        segments.append(seg);
    } else {
        // this is the continuation of an existing segment
        seg = adjacentSegment(lastSegment, yStart, yStop, height);
        QLine* line = new QLine(QPoint(x - 1, seg->yLast),QPoint( x, y));
        seg->length  += abs(1 + (seg->yLast - y)*(seg->yLast - y));
        seg->path.append(line);
        seg->yLast = y;
    }

    seg->retransform();

    for (int y = yStart; y <= yStop; y++)
        currSegment [y] = seg;
}

/*!
    return the number of runs adjacent to the pixels from yStart to yStop (inclusive)
*/
int Segments::adjacentRuns(bool* columnBool, int yStart, int yStop, int height) {
    int runs = 0;
    bool inRun = false;
    for (int y = yStart - 1; y <= yStop + 1; y++) {
        if ((0 <= y) && (y < height)) {
            if (!inRun && columnBool [y]) {
                inRun = true;
                ++runs;
            } else if (inRun && !columnBool [y]) {
                inRun = false;
            }
        }
    }

    return runs;
}

/*!
    find the single segment pointer among the adjacent pixels from yStart-1 to yStop+1
*/
Segment* Segments::adjacentSegment(Segment** lastSegment, int yStart, int yStop, int height) {
    for (int y = yStart - 1; y <= yStop + 1; y++) {
        if ((0 <= y) && (y < height))
            if (lastSegment [y])
                return lastSegment [y];
    }

    return 0;
}

/*!
    return the number of segments adjacent to the pixels from yStart to yStop (inclusive)
*/
int Segments::adjacentSegments(Segment** lastSegment, int yStart, int yStop, int height) {
    int count = 0;
    bool inSegment = false;
    for (int y = yStart - 1; y <= yStop + 1; y++) {
        if ((0 <= y) && (y < height)) {
            if (!inSegment && lastSegment [y]) {
                inSegment = true;
                ++count;
            } else if (inSegment && !lastSegment [y]) {
                inSegment = false;
            }
        }
    }

    return count;
}
