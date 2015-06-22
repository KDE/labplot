#ifndef IMAGEPRIVATE_H
#define IMAGEPRIVATE_H

#include <QBrush>

class Image;
class QGraphicsScene;

class ImagePrivate{
	public:
		explicit ImagePrivate(Image*);
		virtual ~ImagePrivate();
		
		Image::ReferencePoints points;
		Image::EditorSettings settings;
		Image* const q;
		QRectF pageRect;
		QGraphicsScene* m_scene;
		float rotationAngle;
		bool drawPoints;

		QString name() const;
		void update();
		void updateFileName();
		void updatePageRect();

		QString imageFileName;
};

#endif
