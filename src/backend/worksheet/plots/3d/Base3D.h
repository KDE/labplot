/***************************************************************************
    File                 : Base3D.h
    Project              : LabPlot
    Description          : Base class for 3D objects
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

#ifndef PLOT3D_BASE3D_H
#define PLOT3D_BASE3D_H

#include "backend/core/AbstractAspect.h"
#include "Plot3D.h"

class vtkProp;
class vtkProperty;
class vtkRenderer;

class Base3DPrivate;
class Base3D : public AbstractAspect {
		Q_OBJECT
		Q_DISABLE_COPY(Base3D)
		Q_DECLARE_PRIVATE(Base3D)
	public:
		explicit Base3D(Base3DPrivate* priv);
		virtual ~Base3D();
		QIcon icon() const;
		void setRenderer(vtkRenderer* renderer);
		void setXScaling(Plot3D::Scaling scaling);
		void setYScaling(Plot3D::Scaling scaling);
		void setZScaling(Plot3D::Scaling scaling);

		void setRange(const double bounds[6]);
		void getBounds(double bounds[6]) const;

		void show(bool pred);
		bool isVisible() const;
		void highlight(bool pred);
		void select(bool pred);
		bool operator==(vtkProp* prop) const;
		bool operator!=(vtkProp* prop) const;

	public slots:
		void remove();
		void recover();

	signals:
		void removed();
		// TODO: Emit signals when it's appropriate
		// Emits when parameters have been changed
		void parametersChanged();
		// Emits when visibility has been changed
		void visibilityChanged(bool);

	protected:
		const QScopedPointer<Base3DPrivate> d_ptr;
};

#endif