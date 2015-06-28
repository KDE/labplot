#ifndef IMAGE_H
#define IMAGE_H

#include "backend/core/AbstractPart.h"
#include "backend/core/AbstractScriptingEngine.h"
#include "backend/lib/macros.h"
#include <QGraphicsScene>

class QGraphicsItem;
class QRectF;
class ImagePrivate;
//class Transform;
class ImageEditor;
class CustomItem;

class Image: public AbstractPart, public scripted {
	Q_OBJECT

	public:
		Image(AbstractScriptingEngine* engine, const QString& name, bool loading = false);
        ~Image();

        enum GraphType { Cartesian, Polar, Logarithmic };
        enum ColorAttributes { None, Intensity, Foreground, Hue, Saturation, Value };
        enum PlotImageType { OriginalImage, ProcessedImage };
        enum PointsType { AxisPoints, CurvePoints, SegmentPoints };
        enum ErrorType { NoError, SymmetricError, AsymmetricError };

        struct ReferencePoints {
            GraphType type;
            QPointF scenePos[3];
            QPointF logicalPos[3];
        };

        struct Errors {
            ErrorType x;
            ErrorType y;
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
        void discretize(const EditorSettings&);
        void setPlotImageType(const Image::PlotImageType&);
        void updateAxisPoints();

        bool isLoaded;
        QImage originalPlotImage;
        QImage processedPlotImage;
        PlotImageType plotImageType;

        CLASS_D_ACCESSOR_DECL(QString, plotFileName, PlotFileName)
        CLASS_D_ACCESSOR_DECL(Image::ReferencePoints, axisPoints, AxisPoints)
        CLASS_D_ACCESSOR_DECL(Image::EditorSettings, settings, Settings)
        BASIC_D_ACCESSOR_DECL(float, rotationAngle, RotationAngle)
        BASIC_D_ACCESSOR_DECL(Errors, plotErrors, PlotErrors)
        BASIC_D_ACCESSOR_DECL(PointsType, plotPointsType, PlotPointsType)

		typedef ImagePrivate Private;

	private:
		void init();
		ImagePrivate* const d;
		friend class ImagePrivate;

        ImageEditor* m_imageEditor;
        //Transform* m_transform;

	 private slots:
		void handleAspectAdded(const AbstractAspect*);
		void handleAspectAboutToBeRemoved(const AbstractAspect*);
		void handleAspectRemoved(const AbstractAspect* parent, const AbstractAspect* before, const AbstractAspect* child);

	 signals:
		void requestProjectContextMenu(QMenu*);
		void requestUpdate();
        void requestUpdateAxisPoints();

        void plotFileNameChanged(const QString&);
        void rotationAngleChanged(float);
        void plotErrorsChanged(Image::Errors);
        friend class ImageSetPlotFileNameCmd;
        friend class ImageSetRotationAngleCmd;
        friend class ImageSetPlotErrorsCmd;
};
#endif
