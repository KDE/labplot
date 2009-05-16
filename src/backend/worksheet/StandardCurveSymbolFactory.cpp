/***************************************************************************
    File                 : StandardCurveSymbolFactory.cpp
    Project              : LabPlot/SciDAVis
    Description          : Factory of built-in curve symbols.
    --------------------------------------------------------------------
    Copyright            : (C) 2009 Tilman Benkert (thzs*gmx.net)
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

#include "worksheet/StandardCurveSymbolFactory.h"
#include "worksheet/symbols/CrossCurveSymbol.h"
#ifndef QT_STATICPLUGIN
#define QT_STATICPLUGIN
#endif
#include <QtGlobal>

/**
 * \class StandardCurveSymbolFactory
 * \brief Factory of built-in curve symbols.
 *
 * 
 */

StandardCurveSymbolFactory::~StandardCurveSymbolFactory() {
	qDeleteAll(m_prototypes);
}

void StandardCurveSymbolFactory::init() {
	m_prototypes.append(new CrossCurveSymbol());
}

QList<const AbstractCurveSymbol *> StandardCurveSymbolFactory::prototypes() {
	if (m_prototypes.isEmpty())
		init();

	return m_prototypes;
}
		
QStringList StandardCurveSymbolFactory::ids() {
	if (m_prototypes.isEmpty())
		init();

	QStringList result;
	foreach (const AbstractCurveSymbol *prototype, m_prototypes)
		result.append(prototype->id());

	return result;
}

const AbstractCurveSymbol *StandardCurveSymbolFactory::prototype(const QString &id)
{
	if (m_prototypes.isEmpty())
		init();

	foreach (const AbstractCurveSymbol *prototype, m_prototypes) {
		if (id == prototype->id())
			return prototype;
	}
	return NULL;
}

Q_EXPORT_PLUGIN2(scidavis_standardcurvesymbolfactory, StandardCurveSymbolFactory)
Q_IMPORT_PLUGIN(scidavis_standardcurvesymbolfactory)
