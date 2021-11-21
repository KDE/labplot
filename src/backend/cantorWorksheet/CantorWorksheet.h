/*
    File                 : CantorWorksheet.h
    Project              : LabPlot
    Description          : Aspect providing a Cantor Worksheets for Multiple backends
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2015 Garvit Khatri <garvitdelhi@gmail.com>
    SPDX-FileCopyrightText: 2016 Alexander Semke <alexander.semke@web.de>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

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

	bool init(QByteArray* content = nullptr);
	const QString& error() const;

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
	QString m_error;
	Cantor::Session* m_session{nullptr};
	KParts::ReadWritePart* m_part{nullptr};
	QList<Cantor::PanelPlugin*> m_plugins;
	QAbstractItemModel* m_variableModel{nullptr};
	Cantor::WorksheetAccessInterface* m_worksheetAccess{nullptr};

private slots:
	void dataChanged(const QModelIndex&);
	void rowsInserted(const QModelIndex & parent, int first, int last);
	void rowsAboutToBeRemoved(const QModelIndex & parent, int first, int last);
	void modelReset();
	void modified();

signals:
	void requestProjectContextMenu(QMenu*);
	void statusChanged(Cantor::Session::Status);
};

#endif // CANTORWORKSHEET_H
