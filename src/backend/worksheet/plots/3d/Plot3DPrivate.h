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

#ifndef PLOT3DPRIVATE_H
#define PLOT3DPRIVATE_H

#include "Plot3D.h"
#include "Axes.h"
#include "DataHandlers.h"
#include "backend/worksheet/plots/AbstractPlotPrivate.h"

#include <vtkSmartPointer.h>

class Matrix;
class Axes;

class vtkActor;
class vtkImageActor;
class vtkRenderer;
class vtkPolyDataMapper;
class vtkPoints;
class vtkCellArray;

class QGLContext;
class QVTKGraphicsItem;

class Plot3DPrivate:public AbstractPlotPrivate{
	public:
		explicit Plot3DPrivate(Plot3D* owner);
		virtual ~Plot3DPrivate();

		void init();

		virtual void retransform();
		void updatePlot();
		void updateBackground();
		void mousePressEvent(QGraphicsSceneMouseEvent* event);
		void mouseReleaseEvent(QGraphicsSceneMouseEvent* event);

		Plot3D* const q;
		QGLContext* context;
		QVTKGraphicsItem *vtkItem;
		Plot3D::VisualizationType visType;
		Plot3D::DataSource sourceType;
		bool isInitialized;
		bool rectSet;

		DemoDataHandler demoHandler;
		SpreadsheetDataHandler spreadsheetHandler;
		MatrixDataHandler matrixHandler;
		FileDataHandler fileHandler;

		QVector<IDataHandler*> dataHandlers;

		QScopedPointer<Axes> axes;
		vtkSmartPointer<vtkRenderer> renderer;
		vtkSmartPointer<vtkRenderer> backgroundRenderer;
		vtkSmartPointer<vtkImageActor> backgroundImageActor;

		//background
		PlotArea::BackgroundType backgroundType;
		PlotArea::BackgroundColorStyle backgroundColorStyle;
		PlotArea::BackgroundImageStyle backgroundImageStyle;
		Qt::BrushStyle backgroundBrushStyle;
		QColor backgroundFirstColor;
		QColor backgroundSecondColor;
		QString backgroundFileName;
		float backgroundOpacity;

		//light

};

#endif
