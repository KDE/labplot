
/***************************************************************************
 *                                                                         *
 *  This program is free software; you can redistribute it and/or modify   *
 *  it under the terms of the GNU General Public License as published by   *
 *  the Free Software Foundation; either version 2 of the License, or      *
 *  (at your option) any later version.                                    *
 *                                                                         *
 *  This program is distributed in the hope that it will be useful,        *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *  GNU General Public License for more details.                           *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the Free Software           *
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor,                    *
 *   Boston, MA  02110-1301  USA                                           *
 *                                                                         *
 ***************************************************************************/

#ifndef CUSTOMITEM_H
#define CUSTOMITEM_H

#include <QObject>
#include <QBrush>
#include <QPen>
#include "backend/lib/macros.h"
#include "backend/worksheet/WorksheetElement.h"


class CustomItemPrivate;
class CustomItem : public WorksheetElement {
    Q_OBJECT

	public:
        enum HorizontalPosition { hPositionLeft, hPositionCenter, hPositionRight, hPositionCustom };
        enum VerticalPosition { vPositionTop, vPositionCenter, vPositionBottom, vPositionCustom };
        enum ItemsStyle {Circle, Square, EquilateralTriangle, RightTriangle, Bar, PeakedBar,
                        SkewedBar, Diamond, Lozenge, Tie, TinyTie, Plus, Boomerang, SmallBoomerang,
                        Star4, Star5, Line, Cross };

        struct PositionWrapper {
			QPointF 		   point;
			HorizontalPosition horizontalPosition;
			VerticalPosition   verticalPosition;
		};

        struct ErrorBar {
            QPointF plusDeltaX;
            QPointF minusDeltaX;
            QPointF plusDeltaY;
            QPointF minusDeltaY;
        };

		explicit CustomItem(const QString& name );
		~CustomItem();

		virtual QIcon icon() const;
		virtual QMenu* createContextMenu();
		virtual QGraphicsItem *graphicsItem() const;
		void setParentGraphicsItem(QGraphicsItem*);

		virtual void save(QXmlStreamWriter *) const;
		virtual bool load(XmlStreamReader *);

        CLASS_D_ACCESSOR_DECL(PositionWrapper, position, Position)
		void setPosition(const QPointF&);

        CLASS_D_ACCESSOR_DECL(ErrorBar, itemErrorBar, ItemErrorBar)
        BASIC_D_ACCESSOR_DECL(ItemsStyle, itemsStyle, ItemsStyle)
        BASIC_D_ACCESSOR_DECL(qreal, itemsOpacity, ItemsOpacity)
        BASIC_D_ACCESSOR_DECL(qreal, itemsRotationAngle, ItemsRotationAngle)
        BASIC_D_ACCESSOR_DECL(qreal, itemsSize, ItemsSize)
        CLASS_D_ACCESSOR_DECL(QBrush, itemsBrush, ItemsBrush)
        CLASS_D_ACCESSOR_DECL(QPen, itemsPen, ItemsPen)

		virtual void setVisible(bool on);
		virtual bool isVisible() const;
		virtual void setPrinting(bool);
        void suppressHoverEvents(bool);

		typedef CustomItemPrivate Private;

        static QPainterPath itemsPathFromStyle(CustomItem::ItemsStyle);
        static QString itemsNameFromStyle(CustomItem::ItemsStyle);
        QPainterPath errorBarsPath();

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
		void positionChanged(const CustomItem::PositionWrapper&);
		void visibleChanged(bool);
        void changed();

        friend class CustomItemSetItemsStyleCmd;
        friend class CustomItemSetItemsSizeCmd;
        friend class CustomItemSetItemsRotationAngleCmd;
        friend class CustomItemSetItemsOpacityCmd;
        friend class CustomItemSetItemsBrushCmd;
        friend class CustomItemSetItemsPenCmd;
        friend class CustomItemSetItemErrorBarCmd;
        void itemErrorBarChanged(const CustomItem::ErrorBar&);
        void itemsStyleChanged(CustomItem::ItemsStyle);
        void itemsSizeChanged(qreal);
        void itemsRotationAngleChanged(qreal);
        void itemsOpacityChanged(qreal);
        void itemsBrushChanged(QBrush);
        void itemsPenChanged(const QPen&);
};

#endif
