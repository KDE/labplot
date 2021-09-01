/*
    File                 : ExportWorksheetDialog.h
    Project              : LabPlot
    Description          : export worksheet dialog
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2011-2019 Alexander Semke (alexander.semke@web.de)

*/

/***************************************************************************
 *                                                                         *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *                                                                         *
 ***************************************************************************/

#ifndef EXPORTWORKSHEETDIALOG_H
#define EXPORTWORKSHEETDIALOG_H

#include <QDialog>
#include "commonfrontend/worksheet/WorksheetView.h"

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
	void setFileName(const QString&);
	WorksheetView::ExportFormat exportFormat() const;
	WorksheetView::ExportArea exportArea() const;
	bool exportBackground() const;
	int exportResolution() const;

private:
	Ui::ExportWorksheetWidget* ui;
	bool m_showOptions{true};
	bool m_askOverwrite{true};
	bool m_initializing{false};
	QPushButton* m_showOptionsButton;
	QPushButton* m_okButton;
	QPushButton* m_cancelButton;

private slots:
	void slotButtonClicked(QAbstractButton *);
	void okClicked();
	void toggleOptions();
	void selectFile();
	void formatChanged(int);
	void exportToChanged(int);
	void fileNameChanged(const QString&);
};

#endif
