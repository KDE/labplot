/*
	File                 : ExportWorksheetDialog.h
	Project              : LabPlot
	Description          : export worksheet dialog
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2011-2024 Alexander Semke <alexander.semke@web.de>
	SPDX-FileCopyrightText: 2025 Stefan Gerlach <stefan.gerlach@uni.kn>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef EXPORTWORKSHEETDIALOG_H
#define EXPORTWORKSHEETDIALOG_H

#include "backend/worksheet/Worksheet.h"
#include <QDialog>

namespace Ui {
class ExportWorksheetWidget;
}

class QPushButton;
class QAbstractButton;

class ExportWorksheetDialog : public QDialog {
	Q_OBJECT

public:
	explicit ExportWorksheetDialog(QWidget*);
	~ExportWorksheetDialog() override;

	QString path() const;
	void setProjectFileName(const QString&);
	void setFileName(const QString&);
	Worksheet::ExportFormat exportFormat() const;
	Worksheet::ExportArea exportArea() const;
	bool exportBackground() const;
	int exportResolution() const;

private:
	QString formatExtension(Worksheet::ExportFormat);
	QString formatCaption(Worksheet::ExportFormat);
	Ui::ExportWorksheetWidget* ui;
	bool m_showOptions{true};
	bool m_askOverwrite{true};
	bool m_initializing{false};
	QString m_projectPath;
	QPushButton* m_showOptionsButton;
	QPushButton* m_okButton;
	QPushButton* m_cancelButton;

private Q_SLOTS:
	void slotButtonClicked(QAbstractButton*);
	void okClicked();
	void toggleOptions();
	void selectFile();
	void formatChanged(int);
	void exportToChanged(int);
	void fileNameChanged(const QString&);
};

#endif
