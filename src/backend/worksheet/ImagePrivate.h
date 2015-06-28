#ifndef IMAGEPRIVATE_H
#define IMAGEPRIVATE_H

#include <QBrush>

class Image;
class QGraphicsScene;

class ImagePrivate{
	public:
		explicit ImagePrivate(Image*);
		virtual ~ImagePrivate();
		
		Image::ReferencePoints axisPoints;
		Image::EditorSettings settings;
		Image::Errors plotErrors;
		Image::PointsType plotPointsType;
		Image* const q;
		QRectF pageRect;
		QGraphicsScene* m_scene;
		float rotationAngle;
		QString plotFileName;

		QString name() const;
		void update();
		void updateFileName();
		void updatePageRect();
};

#endif
