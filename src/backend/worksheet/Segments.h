#ifndef SEGMENTS_H
#define SEGMENTS_H

#include <QList>

class ImageEditor;
class QImage;
class QLine;
class Image;
class Segment;

class Segments {

	public:
        Segments(Image*);
		~Segments();
		
        QList<Segment*> segments;
        void makeSegments(QImage&);
        void clearSegments();
        void setSegmentsVisible(bool);

    private:
        Image* m_image;

		void loadBool(const ImageEditor*, bool*, QImage*, int);
        void loadSegment(Segment**, int);
		int adjacentRuns(bool*, int, int, int);
        Segment *adjacentSegment(Segment**, int, int, int);
        int adjacentSegments(Segment**, int, int, int);
        void matchRunsToSegments(int, int, bool*, Segment**, bool*, Segment**, bool*);
        void finishRun(bool*, bool*, Segment**, Segment**, int, int, int, int);
		void scrollBool(bool*, bool*, int);
        void scrollSegment(Segment**, Segment**, int);
        void removeUnneededLines(Segment**, Segment**, int);
};
#endif
