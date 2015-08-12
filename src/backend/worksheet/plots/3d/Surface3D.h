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

#ifndef PLOT3D_SURFACE3D_H
#define PLOT3D_SURFACE3D_H

#include "Base3D.h"
#include "backend/lib/macros.h"

class DemoDataHandler;
class SpreadsheetDataHandler;
class MatrixDataHandler;
class FileDataHandler;

class Surface3DPrivate;
class Surface3D : public Base3D {
		Q_OBJECT
		Q_DECLARE_PRIVATE(Surface3D)
		Q_DISABLE_COPY(Surface3D)
	public:
		enum VisualizationType {
			VisualizationType_Triangles = 0,
			VisualizationType_Wireframe = 1
		};

		enum DataSource {
			DataSource_File,
			DataSource_Spreadsheet,
			DataSource_Matrix,
			DataSource_Empty,
			DataSource_MAX
		};

		enum ColorFilling {
			ColorFilling_Empty,
			ColorFilling_SolidColor,
			ColorFilling_ColorMap,
			ColorFilling_ColorMapFromMatrix,
			ColorFilling_ElevationLevel,
			ColorFilling_MAX
		};

		Surface3D();
		virtual ~Surface3D();

		virtual void save(QXmlStreamWriter*) const;
		virtual bool load(XmlStreamReader*);

		DemoDataHandler& demoDataHandler();
		SpreadsheetDataHandler& spreadsheetDataHandler();
		MatrixDataHandler& matrixDataHandler();
		FileDataHandler& fileDataHandler();

		BASIC_D_ACCESSOR_DECL(VisualizationType, visualizationType, VisualizationType)
		BASIC_D_ACCESSOR_DECL(DataSource, dataSource, DataSource)
		BASIC_D_ACCESSOR_DECL(ColorFilling, colorFilling, ColorFilling)
		CLASS_D_ACCESSOR_DECL(QColor, color, Color)
		BASIC_D_ACCESSOR_DECL(double, opacity, Opacity)
		BASIC_D_ACCESSOR_DECL(bool, showXYProjection, ShowXYProjection)
		BASIC_D_ACCESSOR_DECL(bool, showXZProjection, ShowXZProjection)
		BASIC_D_ACCESSOR_DECL(bool, showYZProjection, ShowYZProjection)

		typedef Surface3D BaseClass;
		typedef Surface3DPrivate Private;

	signals:
		friend class Surface3DSetVisualizationTypeCmd;
		friend class Surface3DSetDataSourceCmd;
		friend class Surface3DSetColorFillingCmd;
		friend class Surface3DSetColorCmd;
		friend class Surface3DSetOpacityCmd;
		friend class Surface3DSetShowXYProjectionCmd;
		friend class Surface3DSetShowXZProjectionCmd;
		friend class Surface3DSetShowYZProjectionCmd;
		void visualizationTypeChanged(Surface3D::VisualizationType);
		void sourceTypeChanged(Surface3D::DataSource);
		void colorFillingChanged(Surface3D::ColorFilling);
		void colorChanged(const QColor&);
		void opacityChanged(double);
		void showXYProjectionChanged(bool);
		void showXZProjectionChanged(bool);
		void showYZProjectionChanged(bool);
};

#endif