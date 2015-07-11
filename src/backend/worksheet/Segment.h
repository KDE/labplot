#ifndef SEGMENT_H
#define SEGMENT_H

#include "backend/lib/macros.h"

class QGraphicsItem;
class QLine;

class SegmentPrivate;
class Image;

class Segment {

	public:
        explicit Segment(Image*);
		~Segment();

        QList<QLine*> path;
        int yLast;
        int length;

        QGraphicsItem *graphicsItem() const;
		void setParentGraphicsItem(QGraphicsItem*);

        bool isVisible() const;
        void setVisible(bool);

		typedef SegmentPrivate Private;

    public slots:
        void retransform();

	protected:
		SegmentPrivate* const d_ptr;

	private:
    	Q_DECLARE_PRIVATE(Segment)
        void init();
        Image* m_image;
};

#endif
