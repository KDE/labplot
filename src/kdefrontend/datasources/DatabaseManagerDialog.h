/*
    File                 : DatabaseManagerDialog.h
    Project              : LabPlot
    Description          : dialog for managing database connections
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2016-2017 Alexander Semke <alexander.semke@web.de>
    SPDX-License-Identifier: GPL-2.0-or-later
*/


#ifndef DATABASEMANAGERDIALOG_H
#define DATABASEMANAGERDIALOG_H

#include <QDialog>

class DatabaseManagerWidget;

class DatabaseManagerDialog : public QDialog {
	Q_OBJECT

public:
	explicit DatabaseManagerDialog(QWidget*, const QString&);
	~DatabaseManagerDialog() override;

	QString connection() const;

private:
	DatabaseManagerWidget* mainWidget;
	bool m_changed{false};

private slots:
	void changed();
	void save();
};

#endif
