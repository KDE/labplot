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
#include <cantor/session.h>

namespace Cantor {
class PanelPlugin;
class WorksheetAccessInterface;
}

namespace KParts {
class ReadWritePart;
}

class CantorWorksheetView;
class Column;
class QAbstractItemModel;

class CantorWorksheet : public AbstractPart {
	Q_OBJECT

public:
	explicit CantorWorksheet(const QString& name, bool loading = false);

	QWidget* view() const override;
	QMenu* createContextMenu() override;
	QIcon icon() const override;

	bool exportView() const override;
	bool printView() override;
	bool printPreview() const override;

	void save(QXmlStreamWriter*) const override;
	bool load(XmlStreamReader*, bool preview) override;

	QString backendName();
	KParts::ReadWritePart* part();
	QList<Cantor::PanelPlugin*> getPlugins();

private:
	mutable CantorWorksheetView* m_view{nullptr};
	QString m_backendName;
	Cantor::Session* m_session{nullptr};
	KParts::ReadWritePart* m_part{nullptr};
	QList<Cantor::PanelPlugin*> m_plugins;
	QAbstractItemModel* m_variableModel{nullptr};
	Cantor::WorksheetAccessInterface* m_worksheetAccess{nullptr};

	bool init(QByteArray* content = nullptr);

private slots:
	void rowsInserted(const QModelIndex & parent, int first, int last);
	void rowsAboutToBeRemoved(const QModelIndex & parent, int first, int last);
	void modelReset();
	void modified();

signals:
	void requestProjectContextMenu(QMenu*);
	void statusChanged(Cantor::Session::Status);
};

#endif // CANTORWORKSHEET_H
