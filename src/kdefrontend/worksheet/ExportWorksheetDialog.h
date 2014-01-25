/***************************************************************************
    File                 : ExportWorksheetDialog.h
    Project              : LabPlot
    Description          : export worksheet dialog
    --------------------------------------------------------------------
    Copyright            : (C) 2011-2014 by Alexander Semke
    Email (use @ for *)  : alexander.semke*web.de

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

#include <KDialog>
#include "ui_exportworksheetwidget.h"
#include "commonfrontend/worksheet/WorksheetView.h"

class ExportWorksheetDialog: public KDialog{
  Q_OBJECT

  public:
	ExportWorksheetDialog(QWidget*);
	QString path() const;
	void setFileName(const QString&);
	WorksheetView::ExportFormat exportFormat() const;
	WorksheetView::ExportArea exportArea() const;
	bool exportBackground() const;

  private:
	QWidget* mainWidget;
	Ui::ExportWorksheetWidget ui;

  private slots:
	void slotButtonClicked(int);
	void okClicked();
	void toggleOptions();
	void selectFile();
	void formatChanged(int);
	void fileNameChanged(const QString&);
};

#endif
