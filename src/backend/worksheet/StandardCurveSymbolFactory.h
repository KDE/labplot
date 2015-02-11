/***************************************************************************
    File                 : StandardCurveSymbolFactory.h
    Project              : LabPlot
    Description          : Factory of built-in curve symbols.
    --------------------------------------------------------------------
    Copyright            : (C) 2009 Tilman Benkert (thzs@gmx.net)
    Copyright            : (C) 2010 Alexander Semke (alexander.semke@web.de)

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

#ifndef STANDARDCURVESYMBOLFACTORY_H
#define STANDARDCURVESYMBOLFACTORY_H

#include <QObject>
#include <QStringList>
#include "backend/worksheet/interfaces.h"

class StandardCurveSymbolFactory: public QObject, public CurveSymbolFactory {
	Q_OBJECT
	Q_INTERFACES(CurveSymbolFactory)

	public:
		virtual ~StandardCurveSymbolFactory();
		virtual QList<const AbstractCurveSymbol *> prototypes();
		virtual const AbstractCurveSymbol *prototype(const QString &id);
		virtual QStringList ids();

	private:
		QList<const AbstractCurveSymbol *> m_prototypes;
		void init();
};

#endif


