#ifndef IMAGE_H
#define IMAGE_H

#include "backend/core/AbstractPart.h"
#include "backend/core/AbstractScriptingEngine.h"
#include "backend/lib/macros.h"
#include <QGraphicsScene>

class QGraphicsItem;
class QRectF;
class ImagePrivate;
class Transform;

class Image: public AbstractPart, public scripted {
	Q_OBJECT

	public:
		Image(AbstractScriptingEngine* engine, const QString& name, bool loading = false);
		~Image();

        enum GraphType {Cartesian,Polar,Logarithmic};

        struct ReferencePoints{
            GraphType type;
            QPointF scenePos[3];
            QPointF logicalPos[3];
        };

		virtual QIcon icon() const;
		virtual QMenu* createContextMenu();
		virtual QWidget* view() const;

		virtual void save(QXmlStreamWriter*) const;
		virtual bool load(XmlStreamReader*);

		QRectF pageRect() const;
		void setPageRect(const QRectF&);
		QGraphicsScene *scene() const;
		void update();
		void setPrinting(bool) const;
        void setSelectedInView(const bool);
        //void setPoints(const ReferencePoints&);

        bool isLoaded;
        QPixmap imagePixmap;

        CLASS_D_ACCESSOR_DECL(QString, imageFileName, ImageFileName)
        CLASS_D_ACCESSOR_DECL(Image::ReferencePoints, points, Points)
        BASIC_D_ACCESSOR_DECL(float, rotationAngle, RotationAngle)
        BASIC_D_ACCESSOR_DECL(bool, drawPoints, DrawPoints)


		typedef ImagePrivate Private;

	private:
		void init();
		ImagePrivate* const d;
		friend class ImagePrivate;

        Transform* m_transform;

	 private slots:
		void handleAspectAdded(const AbstractAspect*);
		void handleAspectAboutToBeRemoved(const AbstractAspect*);
		void handleAspectRemoved(const AbstractAspect* parent, const AbstractAspect* before, const AbstractAspect* child);

	 signals:
		void requestProjectContextMenu(QMenu*);
		void requestUpdate();
        void updateLogicalPositions();

        void addDataToSheet(const QPointF&, int);
        friend class ImageSetImageFileNameCmd;
        void imageFileNameChanged(const QString&);
        friend class ImageSetRotationAngleCmd;
        void rotationAngleChanged(float);
};

#endif
