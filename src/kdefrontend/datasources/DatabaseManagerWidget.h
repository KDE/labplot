/*
    File                 : DatabaseManagerWidget.h
    Project              : LabPlot
    Description          : widget for managing database connections
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2017 Alexander Semke <alexander.semke@web.de>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef DATABASEMANAGERWIDGET_H
#define DATABASEMANAGERWIDGET_H

#include "ui_databasemanagerwidget.h"

#ifdef HAVE_KF5_SYNTAX_HIGHLIGHTING
#include <repository.h>
namespace KSyntaxHighlighting {
	class SyntaxHighlighter;
}
#endif

class DatabaseManagerWidget : public QWidget {
	Q_OBJECT

public:
	explicit DatabaseManagerWidget(QWidget*, QString);

	struct SQLConnection {
		int port;
		QString name;
		QString driver;
		QString hostName;
		QString dbName;
		QString userName;
		QString password;
		bool customConnectionEnabled{false};
		QString customConnectionString;
	};

	QString connection() const;
	void setCurrentConnection(const QString&);
	void saveConnections();
	static bool isFileDB(const QString&);
	static bool isODBC(const QString&);

private:
	Ui::DatabaseManagerWidget ui;
	QList<SQLConnection> m_connections;
	SQLConnection* m_current_connection = nullptr;
	bool m_initializing{false};
	QString m_configPath;
	QString m_initConnName;
#ifdef HAVE_KF5_SYNTAX_HIGHLIGHTING
	KSyntaxHighlighting::SyntaxHighlighter* m_highlighter{nullptr};
	KSyntaxHighlighting::Repository m_repository;
#endif

	QString uniqueName();
	void loadConnection();
	int defaultPort(const QString&) const;
	void dataChanged();

private Q_SLOTS:
	void loadConnections();
	void addConnection();
	void deleteConnection();
	void testConnection();
	void connectionChanged(int);

	void nameChanged(const QString&);
	void driverChanged();
	void selectFile();
	void hostChanged();
	void portChanged();
	void databaseNameChanged();
	void customConnectionEnabledChanged(bool);
	void customConnectionChanged();
	void userNameChanged();
	void passwordChanged();

Q_SIGNALS:
	void changed();
};

#endif
