/***************************************************************************
    File                 : Axes.h
    Project              : LabPlot
    Description          : 3D plot axes
    --------------------------------------------------------------------
    Copyright            : (C) 2015 by Minh Ngo (minh@fedoraproject.org)

 ***************************************************************************/

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

#ifndef PLOT3D_AXES_H
#define PLOT3D_AXES_H

#include "backend/core/AbstractAspect.h"
#include "backend/lib/macros.h"

#include <QColor>

#include <vtkSmartPointer.h>

class vtkProp;
class vtkActor;
class vtkRenderer;

class AxesPrivate;
class Axes : public AbstractAspect {
		Q_OBJECT
		Q_DECLARE_PRIVATE(Axes)
		Q_DISABLE_COPY(Axes)
	public:
		enum AxesType {
			AxesType_Cube = 0,
			AxesType_Plain = 1
		};

		Axes(vtkRenderer* renderer = 0);
		~Axes();

		void setRenderer(vtkRenderer* renderer);

		virtual void save(QXmlStreamWriter*) const;
		virtual bool load(XmlStreamReader*);

		QIcon icon() const;
		QMenu* createContextMenu();

		void updateBounds();

		bool operator==(vtkProp* prop) const;
		bool operator!=(vtkProp* prop) const;

		bool isVisible() const;

		BASIC_D_ACCESSOR_DECL(AxesType, type, Type)
		BASIC_D_ACCESSOR_DECL(int, fontSize, FontSize)
		BASIC_D_ACCESSOR_DECL(double, width, Width)
		CLASS_D_ACCESSOR_DECL(QColor, xLabelColor, XLabelColor)
		CLASS_D_ACCESSOR_DECL(QColor, yLabelColor, YLabelColor)
		CLASS_D_ACCESSOR_DECL(QColor, zLabelColor, ZLabelColor)

		typedef Axes BaseClass;
		typedef AxesPrivate Private;

	public slots:
		void show(bool pred);

	signals:
		friend class AxesSetTypeCmd;
		friend class AxesSetFontSizeCmd;
		friend class AxesSetWidthCmd;
		friend class AxesSetXLabelColorCmd;
		friend class AxesSetYLabelColorCmd;
		friend class AxesSetZLabelColorCmd;
		void typeChanged(Axes::AxesType);
		void fontSizeChanged(int);
		void widthChanged(double);
		void xLabelColorChanged(const QColor&);
		void yLabelColorChanged(const QColor&);
		void zLabelColorChanged(const QColor&);
		void parametersChanged();

	private:
		const QScopedPointer<AxesPrivate> d_ptr;
};

#endif