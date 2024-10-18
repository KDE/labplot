/*
	File                 : ImportOriginLayersDialog.h
	Project              : LabPlot
	Description          : Dialog providing the option to select how to import multiple layers in the Origin file.
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2024 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef IMPORTORIGINLAYERSDIALOG_H
#define IMPORTORIGINLAYERSDIALOG_H

#include <QDialog>

namespace Ui {
class ImportOriginLayersDialog;
}

class ImportOriginLayersDialog : public QDialog {
	Q_OBJECT

public:
	explicit ImportOriginLayersDialog(QWidget* parent = nullptr);
	~ImportOriginLayersDialog();
	bool graphLayersAsPlotArea() const;

private:
	Ui::ImportOriginLayersDialog* ui;
};

#endif // IMPORTORIGINLAYERSDIALOG_H
