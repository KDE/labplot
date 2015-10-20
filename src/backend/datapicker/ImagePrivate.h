#ifndef IMAGEPRIVATE_H
#define IMAGEPRIVATE_H

#include <QBrush>

class QGraphicsScene;

class ImagePrivate{
	public:
		explicit ImagePrivate(Image*);
		virtual ~ImagePrivate();
		
		Image::ReferencePoints axisPoints;
		Image::EditorSettings settings;
		Image::PointsType plotPointsType;
		Image* const q;
		QRectF pageRect;
		QGraphicsScene* m_scene;
		float rotationAngle;
        QString fileName;
        int pointSeparation;
        int minSegmentLength;

		QString name() const;
		void update();
		void updateFileName();
        void discretize();
        void makeSegments();
        bool uploadImage(const QString&);
};

#endif
