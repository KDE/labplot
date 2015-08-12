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

class KUrl;

class Matrix;
class AbstractColumn;

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

		// General parameters
		BASIC_D_ACCESSOR_DECL(VisualizationType, visualizationType, VisualizationType)
		BASIC_D_ACCESSOR_DECL(DataSource, dataSource, DataSource)
		BASIC_D_ACCESSOR_DECL(ColorFilling, colorFilling, ColorFilling)
		CLASS_D_ACCESSOR_DECL(QColor, color, Color)
		BASIC_D_ACCESSOR_DECL(double, opacity, Opacity)
		BASIC_D_ACCESSOR_DECL(bool, showXYProjection, ShowXYProjection)
		BASIC_D_ACCESSOR_DECL(bool, showXZProjection, ShowXZProjection)
		BASIC_D_ACCESSOR_DECL(bool, showYZProjection, ShowYZProjection)

		// Matrix parameters
		POINTER_D_ACCESSOR_DECL(const Matrix, matrix, Matrix);
		const QString& matrixPath() const;

		// Spreadsheet parameters
		POINTER_D_ACCESSOR_DECL(const AbstractColumn, xColumn, XColumn);
		POINTER_D_ACCESSOR_DECL(const AbstractColumn, yColumn, YColumn);
		POINTER_D_ACCESSOR_DECL(const AbstractColumn, zColumn, ZColumn);

		POINTER_D_ACCESSOR_DECL(const AbstractColumn, firstNode, FirstNode);
		POINTER_D_ACCESSOR_DECL(const AbstractColumn, secondNode, SecondNode);
		POINTER_D_ACCESSOR_DECL(const AbstractColumn, thirdNode, ThirdNode);

		const QString& xColumnPath() const;
		const QString& yColumnPath() const;
		const QString& zColumnPath() const;

		const QString& firstNodePath() const;
		const QString& secondNodePath() const;
		const QString& thirdNodePath() const;

		// FileData parameters
		CLASS_D_ACCESSOR_DECL(KUrl, file, File);

		typedef Surface3D BaseClass;
		typedef Surface3DPrivate Private;

	private slots:
		// Spreadsheet slots
		void xColumnAboutToBeRemoved(const AbstractAspect*);
		void yColumnAboutToBeRemoved(const AbstractAspect*);
		void zColumnAboutToBeRemoved(const AbstractAspect*);

		void firstNodeAboutToBeRemoved(const AbstractAspect*);
		void secondNodeAboutToBeRemoved(const AbstractAspect*);
		void thirdNodeAboutToBeRemoved(const AbstractAspect*);

		// Matrix slots
		void matrixAboutToBeRemoved(const AbstractAspect*);

	signals:
		// General parameters
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

		// Matrix parameters
		friend class Surface3DSetMatrixCmd;
		void matrixChanged(const Matrix*);

		// Spreadsheet parameters
		friend class Surface3DSetXColumnCmd;
		friend class Surface3DSetYColumnCmd;
		friend class Surface3DSetZColumnCmd;
		friend class Surface3DSetFirstNodeCmd;
		friend class Surface3DSetSecondNodeCmd;
		friend class Surface3DSetThirdNodeCmd;
		void xColumnChanged(const AbstractColumn*);
		void yColumnChanged(const AbstractColumn*);
		void zColumnChanged(const AbstractColumn*);
		void firstNodeChanged(const AbstractColumn*);
		void secondNodeChanged(const AbstractColumn*);
		void thirdNodeChanged(const AbstractColumn*);

		// FileData parameters
		friend class Surface3DSetFileCmd;
		void pathChanged(const KUrl&);
};

#endif