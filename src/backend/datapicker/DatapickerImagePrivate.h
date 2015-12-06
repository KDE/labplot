#ifndef DATAPICKERIMAGEPRIVATE_H
#define DATAPICKERIMAGEPRIVATE_H

#include <QBrush>

class QGraphicsScene;

class DatapickerImagePrivate{
	public:
        explicit DatapickerImagePrivate(DatapickerImage*);
        virtual ~DatapickerImagePrivate();
		
        DatapickerImage::ReferencePoints axisPoints;
        DatapickerImage::EditorSettings settings;
        DatapickerImage::PointsType plotPointsType;
        DatapickerImage* const q;
		QRectF pageRect;
		QGraphicsScene* m_scene;
		float rotationAngle;
        QString fileName;
        int pointSeparation;
        int minSegmentLength;

		QString name() const;
		void updateFileName();
        void discretize();
        void makeSegments();
        bool uploadImage(const QString&);
};

#endif
