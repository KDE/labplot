/***************************************************************************
    File                 : CantorWorksheet.h
    Project              : LabPlot
    Description          : Aspect providing a Cantor Worksheets for Multiple backends
    --------------------------------------------------------------------
    Copyright            : (C) 2015 Garvit Khatri (garvitdelhi@gmail.com)
	Copyright            : (C) 2016 by Alexander Semke (alexander.semke@web.de)

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
#ifndef CANTORWORKSHEET_H
#define CANTORWORKSHEET_H

#include <backend/core/AbstractPart.h>
#include "backend/core/AbstractScriptingEngine.h"

namespace Cantor {
class Session;
class PanelPlugin;
class WorksheetAccessInterface;
}

namespace KParts {
class ReadWritePart;
}

class QAbstractItemModel;
class Column;

class CantorWorksheet : public AbstractPart, public scripted {
	Q_OBJECT

public:
	CantorWorksheet(AbstractScriptingEngine* engine, const QString& name, bool loading = false);

	virtual QWidget* view() const;
	virtual QMenu* createContextMenu();
	virtual QIcon icon() const;

	virtual void exportView() const;
	virtual void printView();
	virtual void printPreview() const;

	virtual void save(QXmlStreamWriter*) const;
	virtual bool load(XmlStreamReader*);

	QString backendName();
	KParts::ReadWritePart* part();
	QList<Cantor::PanelPlugin*> getPlugins();

private:
	QString m_backendName;
	Cantor::Session* m_session;
	KParts::ReadWritePart* m_part;
	QList<Cantor::PanelPlugin*> m_plugins;
	QAbstractItemModel* m_variableModel;
	Cantor::WorksheetAccessInterface* m_worksheetAccess;

	bool init(QByteArray* content = NULL);

private slots:
	void rowsInserted(const QModelIndex & parent, int first, int last);
	void rowsAboutToBeRemoved(const QModelIndex & parent, int first, int last);
	void modelReset();
	void sessionChanged();

signals:
	void requestProjectContextMenu(QMenu*);
};

#endif // CANTORWORKSHEET_H
