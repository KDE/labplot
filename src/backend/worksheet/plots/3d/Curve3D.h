/***************************************************************************
    File                 : Curve3D.h
    Project              : LabPlot
    Description          : 3D curve class
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

#ifndef CURVE3D_H
#define CURVE3D_H

#include "backend/lib/macros.h"
#include "backend/core/AbstractAspect.h"

class vtkRenderer;
class vtkProp;

class AbstractColumn;
class Curve3DPrivate;
class Curve3D : public AbstractAspect {
		Q_OBJECT
		Q_DECLARE_PRIVATE(Curve3D)
		Q_DISABLE_COPY(Curve3D)
	public:
		Curve3D(vtkRenderer* renderer = 0);
		void setRenderer(vtkRenderer* renderer);
		void highlight(bool pred);
		virtual ~Curve3D();

		virtual void save(QXmlStreamWriter*) const;
		virtual bool load(XmlStreamReader*);
		void show(bool pred);
		bool isVisible() const;

		bool operator==(vtkProp* prop) const;
		bool operator!=(vtkProp* prop) const;

		POINTER_D_ACCESSOR_DECL(const AbstractColumn, xColumn, XColumn);
		POINTER_D_ACCESSOR_DECL(const AbstractColumn, yColumn, YColumn);
		POINTER_D_ACCESSOR_DECL(const AbstractColumn, zColumn, ZColumn);

		const QString& xColumnPath() const;
		const QString& yColumnPath() const;
		const QString& zColumnPath() const;

		BASIC_D_ACCESSOR_DECL(float, pointRadius, PointRadius);
		BASIC_D_ACCESSOR_DECL(bool, showVertices, ShowVertices);
		BASIC_D_ACCESSOR_DECL(bool, isClosed, IsClosed);

		typedef Curve3D BaseClass;
		typedef Curve3DPrivate Private;

	public slots:
		void remove();

	private slots:
		void update();

	signals:
		friend class Curve3DSetXColumnCmd;
		friend class Curve3DSetYColumnCmd;
		friend class Curve3DSetZColumnCmd;
		friend class Curve3DSetPointRadiusCmd;
		friend class Curve3DSetShowVerticesCmd;
		friend class Curve3DSetIsClosedCmd;
		void xColumnChanged(const AbstractColumn*);
		void yColumnChanged(const AbstractColumn*);
		void zColumnChanged(const AbstractColumn*);
		void pointRadiusChanged(float);
		void showVerticesChanged(bool);
		void isClosedChanged(bool);
		void parametersChanged();
		void removed();

	private:
		const QScopedPointer<Curve3DPrivate> d_ptr;
};

#endif