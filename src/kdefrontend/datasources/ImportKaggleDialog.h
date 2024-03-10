/*
	File                 : ImportKaggleDialog.h
	Project              : LabPlot
	Description          : import kaggle dataset dialog
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2024 Israel Galadima <izzygaladima@gmail.com>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef IMPORTKAGGLEDIALOG_H
#define IMPORTKAGGLEDIALOG_H

#include "ImportDialog.h"
#include "kdefrontend/MainWin.h"

class ImportKaggleWidget;

class ImportKaggleDialog : public ImportDialog {
	Q_OBJECT

public:
	explicit ImportKaggleDialog(MainWin*);
	~ImportKaggleDialog() override;

	QString selectedObject() const override;
	bool importTo(QStatusBar*) const override;

private:
	MainWin* m_mainWin{nullptr};
	ImportKaggleWidget* m_importKaggleWidget;

protected Q_SLOTS:
	void checkOkButton() override;
};

#endif // IMPORTKAGGLEDIALOG_H
