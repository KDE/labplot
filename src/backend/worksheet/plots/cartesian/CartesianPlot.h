/***************************************************************************
    File                 : CartesianPlot.h
    Project              : LabPlot
    Description          : Cartesian plot
    --------------------------------------------------------------------
    Copyright            : (C) 2011-2012 by Alexander Semke (alexander.semke*web.de)
                           (replace * with @ in the email addresses) 
                           
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

#ifndef CARTESIANPLOT_H
#define CARTESIANPLOT_H

#include "backend/worksheet/plots/AbstractPlot.h"

#ifndef ACTIVATE_SCIDAVIS_SPECIFIC_CODE
class KAction;
#endif
class QToolBar;
class CartesianPlotPrivate;

class CartesianPlot:public AbstractPlot{
	Q_OBJECT

	public:
		CartesianPlot(const QString &name);
		virtual ~CartesianPlot();

		enum Scale {ScaleLinear, ScaleLog10, ScaleLog2, ScaleLn, ScaleSqrt, ScaleX2};

		void initDefault();
		QIcon icon() const;
		QMenu* createContextMenu();
		void fillToolBar(QToolBar*) const;
		void setRect(const QRectF&);
		
		virtual void save(QXmlStreamWriter *) const;
		virtual bool load(XmlStreamReader *);
		
		BASIC_D_ACCESSOR_DECL(bool, autoScaleX, AutoScaleX)
		BASIC_D_ACCESSOR_DECL(bool, autoScaleY, AutoScaleY)
		BASIC_D_ACCESSOR_DECL(float, xMin, XMin)
		BASIC_D_ACCESSOR_DECL(float, xMax, XMax)
		BASIC_D_ACCESSOR_DECL(float, yMin, YMin)
		BASIC_D_ACCESSOR_DECL(float, yMax, YMax)
		BASIC_D_ACCESSOR_DECL(CartesianPlot::Scale, xScale, XScale)
		BASIC_D_ACCESSOR_DECL(CartesianPlot::Scale, yScale, YScale)
		
		typedef CartesianPlot BaseClass;
		typedef CartesianPlotPrivate Private;

	private:
		void init();
		void initActions();
		void initMenus();

		QAction* addCurveAction;
		QAction* addHorizontalAxisAction;
		QAction* addVerticalAxisAction;
 		
		QAction* scaleAutoXAction;
		QAction* scaleAutoYAction;
		QAction* scaleAutoAction;
		QAction* zoomInAction;
		QAction* zoomOutAction;
		QAction* zoomInXAction;
		QAction* zoomOutXAction;
		QAction* zoomInYAction;
		QAction* zoomOutYAction;
		QAction* shiftLeftXAction;
		QAction* shiftRightXAction;
		QAction* shiftUpYAction;
		QAction* shiftDownYAction;
		
		QMenu* addNewMenu;
		QMenu* zoomMenu;
		
		Q_DECLARE_PRIVATE(CartesianPlot)

	private slots:
		void addAxis();
		void addCurve();

		void xDataChanged();
		void yDataChanged();

		void scaleAuto();
		void scaleAutoX();
		void scaleAutoY();
		void zoomIn();
		void zoomOut();
		void zoomInX();
		void zoomOutX();
		void zoomInY();
		void zoomOutY();
		void shiftLeftX();
		void shiftRightX();
		void shiftUpY();
		void shiftDownY();
	
	protected:
		CartesianPlot(const QString &name, CartesianPlotPrivate *dd);

	signals:

		//friend class CartesianPlotSetPositionCmd;
		friend class CartesianPlotSetXMinCmd;
		friend class CartesianPlotSetXMaxCmd;
		friend class CartesianPlotSetXScaleCmd;
		friend class CartesianPlotSetYMinCmd;
		friend class CartesianPlotSetYMaxCmd;
		friend class CartesianPlotSetYScaleCmd;
		void positionChanged();
		void xMinChanged(float);
		void xMaxChanged(float);
		void xScaleChanged(int);
		void yMinChanged(float);
		void yMaxChanged(float);
		void yScaleChanged(int);
};

#endif
