/*
    File                 : ImportSQLDatabaseDialog.h
    Project              : LabPlot
    Description          : import SQL database dialog
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2016 Ankit Wagadre (wagadre.ankit@gmail.com)
    SPDX-FileCopyrightText: 2016-2019 Alexander Semke (alexander.semke@web.de)
    SPDX-License-Identifier: GPL-2.0-or-later
*/


#ifndef IMPORTSQLDATABASEDIALOG_H
#define IMPORTSQLDATABASEDIALOG_H

#include "kdefrontend/datasources/ImportDialog.h"

class MainWin;
class QStatusBar;
class ImportSQLDatabaseWidget;

class ImportSQLDatabaseDialog : public ImportDialog {
	Q_OBJECT

public:
	explicit ImportSQLDatabaseDialog(MainWin*);
	~ImportSQLDatabaseDialog() override;

	void importTo(QStatusBar*) const override;
	QString selectedObject() const override;

private:
	ImportSQLDatabaseWidget* importSQLDatabaseWidget;

protected  slots:
	void checkOkButton() override;
};

#endif //IMPORTSQLDATABASEDIALOG_H
