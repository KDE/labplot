/*
	File                 : HistoryDialog.h
	Project              : LabPlot
	Description          : history dialog
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2012-2016 Alexander Semke <alexander.semke@web.de>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef HISTORYDIALOG_H
#define HISTORYDIALOG_H

#include <QDialog>

class UndoStack;
class QPushButton;

class HistoryDialog : public QDialog {
	Q_OBJECT

public:
	HistoryDialog(QWidget*, UndoStack*, const QString&);
	~HistoryDialog() override;

private:
	UndoStack* m_undoStack;
	QPushButton* m_okButton;
	QPushButton* m_clearUndoStackButton{nullptr};

private Q_SLOTS:
	void clearUndoStack();
};

#endif
