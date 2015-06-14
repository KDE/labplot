#ifndef IMAGE_H
#define IMAGE_H

#include <QObject>
#include <QFont>
#include <QBrush>
#include <QPen>
#include "backend/lib/macros.h"
#include "backend/worksheet/WorksheetElement.h"

class ImagePrivate;
class Transform;

class Image : public WorksheetElement{
	Q_OBJECT

	public:
		enum HorizontalPosition {hPositionLeft, hPositionCenter, hPositionRight, hPositionCustom};
		enum VerticalPosition {vPositionTop, vPositionCenter, vPositionBottom, vPositionCustom};
        enum GraphType {Cartesian,Polar,Logarithmic};

		struct PositionWrapper{
			QPointF 		   point;
			HorizontalPosition horizontalPosition;
			VerticalPosition   verticalPosition;
		};

        struct ReferencePoints{
            GraphType type;
            QPointF scenePos[2];
            QPointF logicalPos[2];
        };

        explicit Image(const QString& name, QString filename );
        ~Image();

        QString m_filename;
        QPointF referencePoint[2];
        bool selectPoint;
        void setImage();
		virtual QIcon icon() const;
		virtual QMenu* createContextMenu();
		virtual QGraphicsItem *graphicsItem() const;

		virtual void save(QXmlStreamWriter *) const;
		virtual bool load(XmlStreamReader *);

		CLASS_D_ACCESSOR_DECL(PositionWrapper, position, Position);
        CLASS_D_ACCESSOR_DECL(Image::ReferencePoints, points, Points);

		void setPosition(const QPointF&);
        void setLogicalPoints(const ReferencePoints&);
		BASIC_D_ACCESSOR_DECL(float, rotationAngle, RotationAngle);

		virtual void setVisible(bool on);
		virtual bool isVisible() const;
		virtual void setPrinting(bool);
        void setSelectPoint(bool on);

        typedef ImagePrivate Private;

	public slots:
		virtual void retransform();
        void setReferencePoints();
        void setCurvePoints();

	private slots:
		void visibilityChanged();
        void handleAspectAdded(const AbstractAspect*);
        void handleAspectRemoved();


	protected:
        ImagePrivate* const d_ptr;

	private:
        Q_DECLARE_PRIVATE(Image)
		void init();
		void initActions();

		QAction* visibilityAction;
        QAction* setReferencePointsAction;
        QAction* setCurvePointsAction;
        Transform* m_transform;

	signals:
        friend class ImageSetPositionCmd;
        friend class ImageSetRotationAngleCmd;
        void positionChanged(const Image::PositionWrapper&);
		void rotationAngleChanged(float);
		void visibleChanged(bool);
		void changed();
        void updateLogicalPositions();
};

#endif
