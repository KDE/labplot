/***************************************************************************
    File                 : Surface3D.h
    Project              : LabPlot
    Description          : 3D surface class
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

#ifndef SURFACE3D_H
#define SURFACE3D_H

#include "backend/lib/macros.h"
#include "backend/core/AbstractAspect.h"

#include <vtkSmartPointer.h>

class vtkProp;
class vtkRenderer;

class Plot3D;
class DemoDataHandler;
class SpreadsheetDataHandler;
class MatrixDataHandler;
class FileDataHandler;

class Surface3DPrivate;
class Surface3D : public AbstractAspect {
		Q_OBJECT
		Q_DECLARE_PRIVATE(Surface3D)
		Q_DISABLE_COPY(Surface3D)
	public:
		enum VisualizationType {
			VisualizationType_Triangles = 0
		};

		enum DataSource {
			DataSource_File,
			DataSource_Spreadsheet,
			DataSource_Matrix,
			DataSource_Empty,
			DataSource_MAX
		};

		enum CollorFilling {NoFilling, SolidColor, ColorMap, ColorMapFromMatrix};

		Surface3D(vtkRenderer* renderer = 0);
		void setRenderer(vtkRenderer* renderer);
		void init();
		void highlight(bool pred);
		virtual ~Surface3D();

		virtual void save(QXmlStreamWriter*) const;
		virtual bool load(XmlStreamReader*);

		bool operator==(vtkProp* prop) const;
		bool operator!=(vtkProp* prop) const;

		DemoDataHandler& demoDataHandler();
		SpreadsheetDataHandler& spreadsheetDataHandler();
		MatrixDataHandler& matrixDataHandler();
		FileDataHandler& fileDataHandler();

		BASIC_D_ACCESSOR_DECL(VisualizationType, visualizationType, VisualizationType)
		BASIC_D_ACCESSOR_DECL(DataSource, dataSource, DataSource)

		typedef Surface3D BaseClass;
		typedef Surface3DPrivate Private;

	public slots:
		void remove();

	private slots:
		void update();

	signals:
		friend class Surface3DSetVisualizationTypeCmd;
		friend class Surface3DSetDataSourceCmd;
		void visualizationTypeChanged(Surface3D::VisualizationType);
		void sourceTypeChanged(Surface3D::DataSource);
		void parametersChanged();
		void removed();

	private:
		const QScopedPointer<Surface3DPrivate> d_ptr;
};

#endif