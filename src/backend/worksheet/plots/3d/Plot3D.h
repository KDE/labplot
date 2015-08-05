/***************************************************************************
    File                 : Plot3D.h
    Project              : LabPlot
    Description          : 3D plot
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

#ifndef PLOT3D_H
#define PLOT3D_H

#include "backend/worksheet/plots/AbstractPlot.h"
#include "backend/worksheet/plots/PlotArea.h"

class QMenu;
class QGLContext;
class vtkProp;
class Plot3DPrivate;

class Plot3D : public AbstractPlot {
	Q_OBJECT
	Q_DECLARE_PRIVATE(Plot3D)
	Q_DISABLE_COPY(Plot3D)
	public:
		explicit Plot3D(const QString &name);
		virtual ~Plot3D();
		void init(bool transform = true);
		void retransform();

		QIcon icon() const;
		virtual QMenu* createContextMenu();
		virtual void save(QXmlStreamWriter*) const;
		virtual bool load(XmlStreamReader*);

		void setRect(const QRectF&);
		void setContext(QGLContext *context);

		BASIC_D_ACCESSOR_DECL(float, backgroundOpacity, BackgroundOpacity)
		BASIC_D_ACCESSOR_DECL(PlotArea::BackgroundType, backgroundType, BackgroundType)
		BASIC_D_ACCESSOR_DECL(PlotArea::BackgroundColorStyle, backgroundColorStyle, BackgroundColorStyle)
		BASIC_D_ACCESSOR_DECL(PlotArea::BackgroundImageStyle, backgroundImageStyle, BackgroundImageStyle)
		BASIC_D_ACCESSOR_DECL(Qt::BrushStyle, backgroundBrushStyle, BackgroundBrushStyle)
		CLASS_D_ACCESSOR_DECL(QColor, backgroundFirstColor, BackgroundFirstColor)
		CLASS_D_ACCESSOR_DECL(QColor, backgroundSecondColor, BackgroundSecondColor)
		CLASS_D_ACCESSOR_DECL(QString, backgroundFileName, BackgroundFileName)

		// Light parameters
		BASIC_D_ACCESSOR_DECL(double, intensity, Intensity); // From 1 to 0
		CLASS_D_ACCESSOR_DECL(QColor, ambient, Ambient);
		CLASS_D_ACCESSOR_DECL(QColor, diffuse, Diffuse);
		CLASS_D_ACCESSOR_DECL(QColor, specular, Specular);
		BASIC_D_ACCESSOR_DECL(double, elevation, Elevation);
		BASIC_D_ACCESSOR_DECL(double, azimuth, Azimuth);
		BASIC_D_ACCESSOR_DECL(double, coneAngle, ConeAngle);

		typedef Plot3D BaseClass;
		typedef Plot3DPrivate Private;

	private:
		void initActions();
		void initMenus();
		void configureAspect(AbstractAspect* aspect);

		QAction* visibilityAction;

		QAction* addCurveAction;
		QAction* addEquationCurveAction;
		QAction* addSurfaceAction;

		QAction* showAxesAction;

		QAction* scaleAutoXAction;
		QAction* scaleAutoYAction;
		QAction* scaleAutoZAction;
		QAction* scaleAutoAction;
		QAction* zoomInAction;
		QAction* zoomOutAction;
		QAction* zoomInXAction;
		QAction* zoomOutXAction;
		QAction* zoomInYAction;
		QAction* zoomOutYAction;
		QAction* zoomInZAction;
		QAction* zoomOutZAction;
		QAction* shiftLeftXAction;
		QAction* shiftRightXAction;
		QAction* shiftUpYAction;
		QAction* shiftDownYAction;
		QAction* shiftUpZAction;
		QAction* shiftDownZAction;

		QAction* rotateClockwiseAction;
		QAction* rotateCounterclockwiseAction;
		QAction* tiltLeftAction;
		QAction* tiltRightAction;
		QAction* tiltUpAction;
		QAction* tiltDownAction;
		QAction* resetRotationAction;

		QMenu* addNewMenu;
		QMenu* zoomMenu;
		QMenu* rotateMenu;

	protected slots:
		void childSelected(const AbstractAspect*);
		void childDeselected(const AbstractAspect*);

	private slots:
		void updatePlot();
		void addSurface();
		void addCurve();
		void itemRemoved();
		void objectClicked(vtkProp*);
		void objectHovered(vtkProp*);

		// Zoom
		void zoomIn();
		void zoomOut();

	signals:
		friend class Plot3DSetBackgroundTypeCmd;
		friend class Plot3DSetBackgroundColorStyleCmd;
		friend class Plot3DSetBackgroundImageStyleCmd;
		friend class Plot3DSetBackgroundBrushStyleCmd;
		friend class Plot3DSetBackgroundFirstColorCmd;
		friend class Plot3DSetBackgroundSecondColorCmd;
		friend class Plot3DSetBackgroundFileNameCmd;
		friend class Plot3DSetBackgroundOpacityCmd;

		friend class Plot3DSetIntensityCmd;
		friend class Plot3DSetAmbientCmd;
		friend class Plot3DSetDiffuseCmd;
		friend class Plot3DSetSpecularCmd;
		friend class Plot3DSetElevationCmd;
		friend class Plot3DSetAzimuthCmd;
		friend class Plot3DSetConeAngleCmd;

		void backgroundTypeChanged(PlotArea::BackgroundType);
		void backgroundColorStyleChanged(PlotArea::BackgroundColorStyle);
		void backgroundImageStyleChanged(PlotArea::BackgroundImageStyle);
		void backgroundBrushStyleChanged(Qt::BrushStyle);
		void backgroundFirstColorChanged(const QColor&);
		void backgroundSecondColorChanged(const QColor&);
		void backgroundFileNameChanged(const QString&);
		void backgroundOpacityChanged(float);
		void parametersChanged();
		void currentAspectChanged(const AbstractAspect*);

		void intensityChanged(double);
		void ambientChanged(const QColor&);
		void diffuseChanged(const QColor&);
		void specularChanged(const QColor&);
		void elevationChanged(double);
		void azimuthChanged(double);
		void coneAngleChanged(double);
};

#endif
