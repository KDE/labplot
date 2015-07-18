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

class QGLContext;
class Plot3DPrivate;

class Axes;
class DemoDataHandler;
class SpreadsheetDataHandler;
class MatrixDataHandler;
class FileDataHandler;

class QMenu;

class Plot3D : public AbstractPlot {
	Q_OBJECT
	Q_DECLARE_PRIVATE(Plot3D)
	Q_DISABLE_COPY(Plot3D)
	public:
		enum VisualizationType{
			VisualizationType_Triangles = 0
		};

		enum DataSource{
			DataSource_File,
			DataSource_Spreadsheet,
			DataSource_Matrix,
			DataSource_Empty,
			DataSource_MAX
		};

		explicit Plot3D(const QString &name);
		virtual ~Plot3D();
		void init(bool transform = true);
		void retransform();

		QIcon icon() const;
		virtual QMenu* createContextMenu();
		virtual void save(QXmlStreamWriter*) const;
		virtual bool load(XmlStreamReader*);

		DemoDataHandler& demoDataHandler();
		SpreadsheetDataHandler& spreadsheetDataHandler();
		MatrixDataHandler& matrixDataHandler();
		FileDataHandler& fileDataHandler();

		Axes& axes();

		void setRect(const QRectF&);
		void setContext(QGLContext *context);

		BASIC_D_ACCESSOR_DECL(VisualizationType, visualizationType, VisualizationType)
		BASIC_D_ACCESSOR_DECL(DataSource, dataSource, DataSource)
		BASIC_D_ACCESSOR_DECL(float, backgroundOpacity, BackgroundOpacity)
		BASIC_D_ACCESSOR_DECL(PlotArea::BackgroundType, backgroundType, BackgroundType)
		BASIC_D_ACCESSOR_DECL(PlotArea::BackgroundColorStyle, backgroundColorStyle, BackgroundColorStyle)
		BASIC_D_ACCESSOR_DECL(PlotArea::BackgroundImageStyle, backgroundImageStyle, BackgroundImageStyle)
		BASIC_D_ACCESSOR_DECL(Qt::BrushStyle, backgroundBrushStyle, BackgroundBrushStyle)
		CLASS_D_ACCESSOR_DECL(QColor, backgroundFirstColor, BackgroundFirstColor)
		CLASS_D_ACCESSOR_DECL(QColor, backgroundSecondColor, BackgroundSecondColor)
		CLASS_D_ACCESSOR_DECL(QString, backgroundFileName, BackgroundFileName)

		typedef Plot3D BaseClass;
		typedef Plot3DPrivate Private;

	private:
		void initActions();
		void initMenus();

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

		QMenu* addNewMenu;
		QMenu* zoomMenu;

	private slots:
		void updatePlot();

	signals:
		friend class Plot3DSetVisualizationTypeCmd;
		friend class Plot3DSetDataSourceCmd;
		friend class Plot3DSetBackgroundTypeCmd;
		friend class Plot3DSetBackgroundColorStyleCmd;
		friend class Plot3DSetBackgroundImageStyleCmd;
		friend class Plot3DSetBackgroundBrushStyleCmd;
		friend class Plot3DSetBackgroundFirstColorCmd;
		friend class Plot3DSetBackgroundSecondColorCmd;
		friend class Plot3DSetBackgroundFileNameCmd;
		friend class Plot3DSetBackgroundOpacityCmd;
		void visualizationTypeChanged(Plot3D::VisualizationType);
		void sourceTypeChanged(Plot3D::DataSource);
		void backgroundTypeChanged(PlotArea::BackgroundType);
		void backgroundColorStyleChanged(PlotArea::BackgroundColorStyle);
		void backgroundImageStyleChanged(PlotArea::BackgroundImageStyle);
		void backgroundBrushStyleChanged(Qt::BrushStyle);
		void backgroundFirstColorChanged(const QColor&);
		void backgroundSecondColorChanged(const QColor&);
		void backgroundFileNameChanged(const QString&);
		void backgroundOpacityChanged(float);
		void parametersChanged();
};

#endif
