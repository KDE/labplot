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
class ImageEditor;

class Image: public AbstractPart, public scripted {
	Q_OBJECT

	public:
		Image(AbstractScriptingEngine* engine, const QString& name, bool loading = false);
		~Image();

        enum GraphType {Cartesian,Polar,Logarithmic};
        enum ColorAttributes{ None, Intensity, Foreground, Hue, Saturation, Value };

        struct ReferencePoints{
            GraphType type;
            QPointF scenePos[3];
            QPointF logicalPos[3];
        };

        struct EditorSettings
        {
          ColorAttributes type;
          int intensityThresholdLow;
          int intensityThresholdHigh;
          int foregroundThresholdLow;
          int foregroundThresholdHigh;
          int hueThresholdLow;
          int hueThresholdHigh;
          int saturationThresholdLow;
          int saturationThresholdHigh;
          int valueThresholdLow;
          int valueThresholdHigh;
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
        CLASS_D_ACCESSOR_DECL(Image::EditorSettings, settings, Settings)
        BASIC_D_ACCESSOR_DECL(float, rotationAngle, RotationAngle)
        BASIC_D_ACCESSOR_DECL(bool, drawPoints, DrawPoints)


		typedef ImagePrivate Private;

	private:
		void init();
		ImagePrivate* const d;
		friend class ImagePrivate;

        Transform* m_transform;
        ImageEditor* m_imageEditor;

	 private slots:
		void handleAspectAdded(const AbstractAspect*);
		void handleAspectAboutToBeRemoved(const AbstractAspect*);
		void handleAspectRemoved(const AbstractAspect* parent, const AbstractAspect* before, const AbstractAspect* child);

	 signals:
		void requestProjectContextMenu(QMenu*);
		void requestUpdate();
        void updateLogicalPositions();
        void settingsChanged(const Image::EditorSettings&);


        void addDataToSheet(const QPointF&, int);
        void imageFileNameChanged(const QString&);
        void rotationAngleChanged(float);
        friend class ImageSetSettingsCmd;
        friend class ImageSetImageFileNameCmd;
        friend class ImageSetRotationAngleCmd;
};

#endif
