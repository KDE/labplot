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
#include "backend/worksheet/plots/AbstractPlotPrivate.h"

#include <QSet>

#include <vtkSmartPointer.h>

class Axes;
class Curve3D;
class Surface3D;

class vtkImageActor;
class vtkRenderer;
class vtkLight;

class QGLContext;
class VTKGraphicsItem;

class Plot3DPrivate : public AbstractPlotPrivate{
	public:
		explicit Plot3DPrivate(Plot3D* owner);
		virtual ~Plot3DPrivate();

		void init();
		void initLights();

		void resetCamera();

		virtual void retransform();
		void updateLight(bool notify = true);
		void updateBackground(bool notify = true);
		void updateXScaling();
		void updateYScaling();
		void updateZScaling();
		void setupCamera();
		void mousePressEvent(QGraphicsSceneMouseEvent* event);
		void mouseReleaseEvent(QGraphicsSceneMouseEvent* event);
		virtual void paint(QPainter*, const QStyleOptionGraphicsItem*, QWidget* widget = 0);

		Plot3D* const q;
		QGLContext* context;
		VTKGraphicsItem *vtkItem;
		bool isInitialized;
		bool rectSet;
		double rangeBounds[6];
		bool isRangeInitialized;

		Axes* axes;
		QSet<Surface3D*> surfaces;
		QSet<Curve3D*> curves;

		vtkSmartPointer<vtkRenderer> renderer;
		vtkSmartPointer<vtkRenderer> backgroundRenderer;
		vtkSmartPointer<vtkImageActor> backgroundImageActor;
		vtkSmartPointer<vtkLight> lightAbove;
		vtkSmartPointer<vtkLight> lightBelow;

		// General parameters
		Plot3D::Scaling xScaling;
		Plot3D::Scaling yScaling;
		Plot3D::Scaling zScaling;

		// Background parameters
		PlotArea::BackgroundType backgroundType;
		PlotArea::BackgroundColorStyle backgroundColorStyle;
		PlotArea::BackgroundImageStyle backgroundImageStyle;
		Qt::BrushStyle backgroundBrushStyle;
		QColor backgroundFirstColor;
		QColor backgroundSecondColor;
		QString backgroundFileName;
		float backgroundOpacity;

		// Light parameters
		double intensity;
		QColor ambient;
		QColor diffuse;
		QColor specular;
		double elevation;
		double azimuth;
		double coneAngle;
};

#endif
