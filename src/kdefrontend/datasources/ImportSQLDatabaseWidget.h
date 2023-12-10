/*
	File                 : ImportSQLDatabaseWidget.cpp
	Project              : LabPlot
	Description          : widget for the import from SQL databases
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2016-2023 Alexander Semke <alexander.semke@web.de>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef IMPORTSQLDATABASEWIDGET_H
#define IMPORTSQLDATABASEWIDGET_H

#include "backend/core/AbstractColumn.h"
#include "backend/datasources/filters/AbstractFileFilter.h"
#include "ui_importsqldatabasewidget.h"
#include <QSqlDatabase>

#ifdef HAVE_KF5_SYNTAX_HIGHLIGHTING
#include <KSyntaxHighlighting/repository.h>
namespace KSyntaxHighlighting {
class SyntaxHighlighter;
}
#endif

class QStandardItemModel;

class ImportSQLDatabaseWidget : public QWidget {
	Q_OBJECT

public:
	explicit ImportSQLDatabaseWidget(QWidget* parent = nullptr);
	~ImportSQLDatabaseWidget() override;

	void read(AbstractDataSource*, AbstractFileFilter::ImportMode importMode = AbstractFileFilter::ImportMode::Replace);
	QString selectedTable() const;
	bool isValid() const;
	bool isNumericData() const;

private:
	Ui::ImportSQLDatabaseWidget ui;
	QList<QString> m_vendorList;
	QList<QString> m_tableNamesList;
	QStringList m_columnNames;
	QVector<AbstractColumn::ColumnMode> m_columnModes;
	int m_cols{0};
	int m_rows{0};
	QSqlDatabase m_db;
	QStandardItemModel* m_databaseTreeModel{nullptr};
	QString m_configPath;
	bool m_initializing{false};
	bool m_valid{false};
	bool m_numeric{false};
#ifdef HAVE_KF5_SYNTAX_HIGHLIGHTING
	KSyntaxHighlighting::SyntaxHighlighter* m_highlighter;
	KSyntaxHighlighting::Repository m_repository;
#endif

	void readConnections();
	QString currentQuery(bool preview = false);
	void setInvalid();
	void setValid();

	// helper functions for unit tests
	friend class ImportSqlDatabaseTest;
	void setStartRow(int);
	void setEndRow(int);
	void setStartColumn(int);
	void setEndColumn(int);
	void setQuery(const QString&);

public Q_SLOTS:
	void loadSettings();

private Q_SLOTS:
	void showDatabaseManager();
	void connectionChanged();
	void importFromChanged(int);
	void refreshPreview();

Q_SIGNALS:
	void completed(int);
	void stateChanged();
	void error(const QString&);
};

#endif // IMPORTSQLDATABASEWIDGET_H
