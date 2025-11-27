/*
	File                 : ImportDatasetDialog.h
	Project              : LabPlot
	Description          : import dataset data dialog
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2019 Ferencz Koovacs <kferike98@gmail.com>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef IMPORTDATASETDIALOG_H
#define IMPORTDATASETDIALOG_H

#include "ImportDialog.h"

class MainWin;
class ImportDatasetWidget;
class DatasetHandler;

class ImportDatasetDialog : public ImportDialog {
	Q_OBJECT

public:
	explicit ImportDatasetDialog(MainWin*);
	~ImportDatasetDialog() override;

	QString selectedObject() const override;
	bool importTo(QStatusBar*) const override;

private:
	ImportDatasetWidget* m_importDatasetWidget;
	enum class Status { SUCCESS = 0, FAILURE = 1 };

protected Q_SLOTS:
	void checkOkButton() override;
};

#endif // IMPORTDATASETDIALOG_H
