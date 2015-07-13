
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

Segments::Segments(Image* image): m_image(image) {
}

Segments::~Segments() {
}

void Segments::makeSegments(QImage &imageProcessed) {
    clearSegments();

    int width = imageProcessed.width();
    int height = imageProcessed.height();

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

void Segments::scrollBool(bool* left, bool* right, int height) {
    for (int y = 0; y < height; y++)
        left [y] = right [y];
}

void Segments::scrollSegment(Segment** left, Segment** right, int height) {
    for (int y = 0; y < height; y++)
        left [y] = right [y];
}

void Segments::matchRunsToSegments(int x, int height, bool* lastBool, Segment** lastSegment,
                                   bool* currBool, Segment** currSegment, bool* nextBool) {
    loadSegment(currSegment, height);

    int yStart = 0;
    bool inRun = false;
    for (int y = 0; y < height; y++)
    {
        if (!inRun && currBool [y])
        {
            inRun = true;
            yStart = y;
        }

        if ((y + 1 >= height) || !currBool [y + 1])
        {
            if (inRun)
                finishRun(lastBool, nextBool, lastSegment, currSegment, x, yStart, y, height);

            inRun = false;
        }
    }

    removeUnneededLines(lastSegment, currSegment, height);
}


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
                    m_image->scene()->removeItem(segLast->graphicsItem());
                    segments.removeOne(segLast);
                }
//                TODO
//                else
//                    segLast->removeUnneededLines();
            }
        }
    }
}



void Segments::loadBool(const ImageEditor* editor, bool* columnBool,
                        QImage* image, int x) {
    for (int y = 0; y < image->height(); y++)
        if (x < 0)
            columnBool [y] = false;
        else
            columnBool [y] = editor->processedPixelIsOn(*image, x, y);
}

void Segments::loadSegment(Segment** columnSegment, int height) {
    for (int y = 0; y < height; y++)
        columnSegment [y] = 0;
}

void Segments::clearSegments() {
    foreach(Segment* seg, segments)
        m_image->scene()->removeItem(seg->graphicsItem());

    segments.clear();
}

void Segments::setSegmentsVisible(bool on) {
    foreach(Segment* seg, segments)
        seg->setVisible(on);
}

void Segments::finishRun(bool* lastBool, bool* nextBool, Segment** lastSegment, Segment** currSegment,
                         int x, int yStart, int yStop, int height) {

    if (adjacentRuns(lastBool, yStart, yStop, height) > 1)
        return;

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

int Segments::adjacentRuns(bool* columnBool, int yStart, int yStop, int height) {
    int runs = 0;
    bool inRun = false;
    for (int y = yStart - 1; y <= yStop + 1; y++)
    {
        if ((0 <= y) && (y < height))
        {
            if (!inRun && columnBool [y])
            {
                inRun = true;
                ++runs;
            }
            else if (inRun && !columnBool [y])
                inRun = false;
        }
    }

    return runs;
}

Segment* Segments::adjacentSegment(Segment** lastSegment, int yStart, int yStop, int height) {
    for (int y = yStart - 1; y <= yStop + 1; y++)
        if ((0 <= y) && (y < height))
            if (lastSegment [y])
                return lastSegment [y];

    return 0;
}

int Segments::adjacentSegments(Segment** lastSegment, int yStart, int yStop, int height) {
    int count = 0;
    bool inSegment = false;
    for (int y = yStart - 1; y <= yStop + 1; y++)
    {
        if ((0 <= y) && (y < height))
        {
            if (!inSegment && lastSegment [y])
            {
                inSegment = true;
                ++count;
            }
            else if (inSegment && !lastSegment [y])
                inSegment = false;
        }
    }

    return count;
}
