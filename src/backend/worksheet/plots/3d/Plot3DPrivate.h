/***************************************************************************
    File                 : Plot3DPrivate.h
    Project              : LabPlot
    Description          : Private members of Plot3D.
    --------------------------------------------------------------------
    Copyright            : (C) 2015 Minh Ngo (minh@fedoraproject.org)

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

#ifndef CARTESIANPLOTPRIVATE_H
#define CARTESIANPLOTPRIVATE_H

#include "Plot3D.h"
#include "backend/worksheet/plots/AbstractPlotPrivate.h"

#include <KUrl>

#include <vtkSmartPointer.h>
#include <vtkRenderer.h>

class QGLContext;
class QVTKGraphicsItem;
class vtkRenderer;
class vtkActor;
class AbstractColumn;

class Plot3DPrivate:public AbstractPlotPrivate{
	public:
		explicit Plot3DPrivate(Plot3D* owner, QGLContext *context);
		virtual ~Plot3DPrivate();

		void init();

		virtual void retransform();

	private:
		void addSphere();
		void readFromFile();
		void readFromColumns();

		void clearActors();

	private:
		Plot3D* const q_ptr;
		Q_DECLARE_PUBLIC(Plot3D);
		QGLContext *context;
		QVTKGraphicsItem *vtkItem;
		Plot3D::VisualizationType visType;
		Plot3D::DataSource sourceType;
		KUrl path;
		bool isChanged;

		vtkSmartPointer<vtkRenderer> renderer;
		QVector<vtkSmartPointer<vtkActor> > actors;

		AbstractColumn *xColumn;
		AbstractColumn *yColumn;
		AbstractColumn *zColumn;
};

#endif