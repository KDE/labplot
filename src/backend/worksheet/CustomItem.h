#ifndef CUSTOMITEM_H
#define CUSTOMITEM_H

#include <QObject>
#include <QFont>
#include <QBrush>
#include <QPen>
#include "backend/lib/macros.h"
#include "backend/worksheet/WorksheetElement.h"


class CustomItemPrivate;
class CustomItem : public WorksheetElement{
    Q_OBJECT

	public:
		enum HorizontalPosition {hPositionLeft, hPositionCenter, hPositionRight, hPositionCustom};
		enum VerticalPosition {vPositionTop, vPositionCenter, vPositionBottom, vPositionCustom};
        enum ItemsStyle {Circle, Square, EquilateralTriangle, RightTriangle, Bar, PeakedBar,
                        SkewedBar, Diamond, Lozenge, Tie, TinyTie, Plus, Boomerang, SmallBoomerang,
                        Star4, Star5, Line, Cross};

		struct PositionWrapper{
			QPointF 		   point;
			HorizontalPosition horizontalPosition;
			VerticalPosition   verticalPosition;
		};

		explicit CustomItem(const QString& name );
		~CustomItem();

		virtual QIcon icon() const;
		virtual QMenu* createContextMenu();
		virtual QGraphicsItem *graphicsItem() const;
		void setParentGraphicsItem(QGraphicsItem*);

		virtual void save(QXmlStreamWriter *) const;
		virtual bool load(XmlStreamReader *);

		CLASS_D_ACCESSOR_DECL(PositionWrapper, position, Position);
		void setPosition(const QPointF&);
		void setPositionInvalid(bool);
        //BASIC_D_ACCESSOR_DECL(float, itemsRotationAngle, RotationAngle);

        BASIC_D_ACCESSOR_DECL(ItemsStyle, itemsStyle, ItemsStyle)
        BASIC_D_ACCESSOR_DECL(qreal, itemsOpacity, ItemsOpacity)
        BASIC_D_ACCESSOR_DECL(qreal, itemsRotationAngle, ItemsRotationAngle)
        BASIC_D_ACCESSOR_DECL(qreal, itemsSize, ItemsSize)
        CLASS_D_ACCESSOR_DECL(QBrush, itemsBrush, ItemsBrush)
        CLASS_D_ACCESSOR_DECL(QPen, itemsPen, ItemsPen)

		virtual void setVisible(bool on);
		virtual bool isVisible() const;
		virtual void setPrinting(bool);

		typedef CustomItemPrivate Private;

        static QPainterPath itemsPathFromStyle(CustomItem::ItemsStyle);
        static QString itemsNameFromStyle(CustomItem::ItemsStyle);

	public slots:
		virtual void retransform();
		virtual void handlePageResize(double horizontalRatio, double verticalRatio);

	private slots:
		void visibilityChanged();

	protected:
		CustomItemPrivate* const d_ptr;
	private:
    	Q_DECLARE_PRIVATE(CustomItem)
		void init();
		void initActions();

		QAction* visibilityAction;

	signals:
		friend class CustomItemSetPositionCmd;
        //friend class CustomItemSetRotationAngleCmd;
		void positionChanged(const CustomItem::PositionWrapper&);
        //void rotationAngleChanged(float);
		void visibleChanged(bool);

		void changed();

        friend class CustomItemSetItemsStyleCmd;
        friend class CustomItemSetItemsSizeCmd;
        friend class CustomItemSetItemsRotationAngleCmd;
        friend class CustomItemSetItemsOpacityCmd;
        friend class CustomItemSetItemsBrushCmd;
        friend class CustomItemSetItemsPenCmd;
        void itemsStyleChanged(CustomItem::ItemsStyle);
        void itemsSizeChanged(qreal);
        void itemsRotationAngleChanged(qreal);
        void itemsOpacityChanged(qreal);
        void itemsBrushChanged(QBrush);
        void itemsPenChanged(const QPen&);
};

#endif
