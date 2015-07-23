
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

#ifndef ZOOMWINDOW_H
#define ZOOMWINDOW_H

#include <QObject>
#include "backend/worksheet/WorksheetElement.h"


class ZoomWindowPrivate;
class ZoomWindow : public WorksheetElement {
    Q_OBJECT

	public:
        explicit ZoomWindow(const QString& name);
		~ZoomWindow();

		virtual QIcon icon() const;
		virtual QMenu* createContextMenu();
		virtual QGraphicsItem *graphicsItem() const;
		void setParentGraphicsItem(QGraphicsItem*);
        void updatePixmap(const WId&, const QPoint&);
        void setPosition(const QPointF&);
        void setScaleFactor(const int);
        virtual void setVisible(bool on);
        virtual bool isVisible() const;
        virtual void setPrinting(bool);

        virtual void save(QXmlStreamWriter *) const;
        virtual bool load(XmlStreamReader *);

		typedef ZoomWindowPrivate Private;

    public slots:
		virtual void retransform();
		
	protected:
		ZoomWindowPrivate* const d_ptr;

	private:
    	Q_DECLARE_PRIVATE(ZoomWindow)
        QPixmap m_pixmap;
        int scaleFactor;

	signals:
        void visibleChanged(bool);
		void changed();
};

#endif
