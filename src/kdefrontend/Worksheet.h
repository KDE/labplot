/***************************************************************************
    File                 : Worksheet.h
    Project              : LabPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2008 by Stefan Gerlach, Alexander Semke
    Email (use @ for *)  : stefan.gerlach*uni-konstanz.de, alexander.semke*web.de
    Description          : worksheet class

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
#ifndef WORKSHEET_H
#define WORKSHEET_H

#include <QtGui>
#include <QtXml>

#include "plots/Plot.h"

#include "core/AbstractPart.h"
#include "core/AbstractScriptingEngine.h"
#include "core/globals.h"

class WorksheetPrivate;

class Worksheet : public AbstractPart, public scripted{
	Q_OBJECT

	public:
		Worksheet(AbstractScriptingEngine*, const QString &name);
		~Worksheet();

		QString title() const;
		void setTitle(const QString& title);
		QWidget *view();
		int plotCount() const;
		void addSet(const Set set, const Plot::PlotType ptype);
		void createPlot(const Plot::PlotType ptype);
		Plot* activePlot() const;
		QList<Plot*>* listPlots();
		void repaint();
		QIcon icon() const;

		QMenu *createContextMenu();
	private:
		WorksheetPrivate* d;
};

#endif //WORKSHEET
