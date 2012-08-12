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

#include "../AbstractPlot.h"

#ifndef ACTIVATE_SCIDAVIS_SPECIFIC_CODE
class KAction;
#endif
class CartesianPlotPrivate;

class CartesianPlot:public AbstractPlot{
	Q_OBJECT

	public:
		CartesianPlot(const QString &name);
		~CartesianPlot();
		
		QIcon icon() const;
		QMenu* createContextMenu();
		void setRect(const QRectF&);
		
		virtual void save(QXmlStreamWriter *) const;
		virtual bool load(XmlStreamReader *);
		
		typedef CartesianPlot BaseClass;
		typedef CartesianPlotPrivate Private;

	private:
		void init();
		void initActions();
		void initMenus();

		QAction* addCurveAction;
		QAction* addHorizontalAxisAction;
		QAction* addVerticalAxisAction;
 		
		QMenu* addNewMenu;

		Q_DECLARE_PRIVATE(CartesianPlot)

	private slots:
		void addCurve();
	
	protected:
		CartesianPlot(const QString &name, CartesianPlotPrivate *dd);
	
	signals:
		void positionChanged();		
};

#endif
