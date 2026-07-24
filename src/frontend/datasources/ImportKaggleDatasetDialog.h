/*
	File                 : ImportKaggleDatasetDialog.h
	Project              : LabPlot
	Description          : import kaggle dataset dialog
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2024 Israel Galadima <izzygaladima@gmail.com>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef IMPORTKAGGLEDATASETDIALOG_H
#define IMPORTKAGGLEDATASETDIALOG_H

#include "ImportDialog.h"
#include "frontend/MainWin.h"

class ImportKaggleDatasetWidget;

class ImportKaggleDatasetDialog : public ImportDialog {
	Q_OBJECT

public:
	explicit ImportKaggleDatasetDialog(MainWin*);
	~ImportKaggleDatasetDialog() override;

	QString selectedObject() const override;
	bool importTo(QStatusBar*) const override;
	static bool checkKaggle();

private:
	ImportKaggleDatasetWidget* m_importKaggleDatasetWidget;
	QPushButton* m_optionsButton{nullptr};

protected Q_SLOTS:
	void checkOkButton() override;
};

#endif // IMPORTKAGGLEDATASETDIALOG_H
