/***************************************************************************
    File                 : ExportWorksheetDialog.h
    Project              : LabPlot
    Description          : export worksheet dialog
    --------------------------------------------------------------------
    Copyright            : (C) 2011-2019 by Alexander Semke (alexander.semke@web.de)

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *  This program is free software; you can redistribute it and/or modify   *
 *  it under the terms of the GNU General Public License as published by   *
 *  the Free Software Foundation; either version 2 of the License, or      *
 *  (at your option) any later version.                                    *
 *                                                                         *
 *  This program is distributed in the hope that it will be useful,        *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *  GNU General Public License for more details.                           *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the Free Software           *
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor,                    *
 *   Boston, MA  02110-1301  USA                                           *
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
