/*
	File                 : ImportSQLDatabaseDialog.h
	Project              : LabPlot
	Description          : import SQL database dialog
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2016-2023 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef IMPORTSQLDATABASEDIALOG_H
#define IMPORTSQLDATABASEDIALOG_H

#include "frontend/datasources/ImportDialog.h"

class ImportSQLDatabaseWidget;
class MainWin;
class QStatusBar;

class ImportSQLDatabaseDialog : public ImportDialog {
	Q_OBJECT

public:
	explicit ImportSQLDatabaseDialog(MainWin*);
	~ImportSQLDatabaseDialog() override;

	bool importTo(QStatusBar*) const override;
	QString selectedObject() const override;

private:
	ImportSQLDatabaseWidget* importSQLDatabaseWidget;

protected Q_SLOTS:
	void checkOkButton() override;
};

#endif // IMPORTSQLDATABASEDIALOG_H
