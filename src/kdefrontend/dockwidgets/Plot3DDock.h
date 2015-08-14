/***************************************************************************
    File                 : Plot3DDock.h
    Project              : LabPlot
    Description          : widget for 3D plot properties
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

#ifndef PLOT3DDOCK_H
#define PLOT3DDOCK_H

#include "backend/worksheet/Worksheet.h"
#include "backend/worksheet/plots/3d/Plot3D.h"
#include "backend/worksheet/plots/PlotArea.h"
#include "ui_plot3ddock.h"

#include "DockHelpers.h"

class KUrl;
class AbstractColumn;
class Matrix;
class Plot3D;
class AspectTreeModel;

class Plot3DDock: public QWidget {
	Q_OBJECT

	public:
		explicit Plot3DDock(QWidget* parent);
		void setPlots(const QList<Plot3D*>& plots);

	private:
		Ui::Plot3DDock ui;
		DockHelpers::ChildrenRecorder recorder;
		Plot3D* m_plot;
		QList<Plot3D*> m_plotsList;
		bool m_initializing;

		void load();
		void loadConfig(KConfig&);

	private slots:
		//SLOTs for changes triggered in Plot3DDock
		void retranslateUi();

		//"General"-tab
		void onNameChanged();
		void onCommentChanged();
		void onVisibilityChanged(int);
		void onGeometryChanged();
		void onLayoutChanged(Worksheet::Layout);
		void onXScalingChanged(int);
		void onYScalingChanged(int);
		void onZScalingChanged(int);
		void onAutoScaleXChanged(int);
		void onAutoScaleYChanged(int);
		void onAutoScaleZChanged(int);
		void onXMinChanged();
		void onYMinChanged();
		void onZMinChanged();
		void onXMaxChanged();
		void onYMaxChanged();
		void onZMaxChanged();

		//"Background"-tab
		void onBackgroundTypeChanged(int);
		void onBackgroundColorStyleChanged(int);
		void onBackgroundImageStyleChanged(int);
		void onBackgroundBrushStyleChanged(int);
		void onBackgroundFirstColorChanged(const QColor&);
		void onBackgroundSecondColorChanged(const QColor&);
		void onBackgroundOpacityChanged(int);
		void onBackgroundSelectFile();
		void onBackgroundFileNameChanged();

		// Light
		void onIntensityChanged(double);
		void onAmbientChanged(const QColor&);
		void onDiffuseChanged(const QColor&);
		void onSpecularChanged(const QColor&);
		void onElevationChanged(int);
		void onAzimuthChanged(int);
		void onConeAngleChanged(int);

		//SLOTs for changes triggered in Plot3D
		//"General"-tab
		void descriptionChanged(const AbstractAspect*);
		void rectChanged(const QRectF&);
		void visibleChanged(bool);
		void xScalingChanged(Plot3D::Scaling);
		void yScalingChanged(Plot3D::Scaling);
		void zScalingChanged(Plot3D::Scaling);

		//"Background"-tab
		void backgroundTypeChanged(PlotArea::BackgroundType);
		void backgroundColorStyleChanged(PlotArea::BackgroundColorStyle);
		void backgroundImageStyleChanged(PlotArea::BackgroundImageStyle);
		void backgroundBrushStyleChanged(Qt::BrushStyle);
		void backgroundFirstColorChanged(const QColor&);
		void backgroundSecondColorChanged(const QColor&);
		void backgroundFileNameChanged(const QString&);
		void backgroundOpacityChanged(float);

		// Light
		void intensityChanged(double);
		void ambientChanged(const QColor&);
		void diffuseChanged(const QColor&);
		void specularChanged(const QColor&);
		void elevationChanged(double);
		void azimuthChanged(double);
		void coneAngleChanged(double);

		//saving/loading
		void loadConfigFromTemplate(KConfig&);
		void saveConfigAsTemplate(KConfig&);
};

#endif

