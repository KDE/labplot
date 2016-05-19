/***************************************************************************
    File                 : ExportSpreadsheetDialog.h
    Project              : LabPlot
    Description          : export spreadsheet dialog
    --------------------------------------------------------------------
    Copyright            : (C) 2014-2016 by Alexander Semke (alexander.semke@web.de)

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

#ifndef EXPORTSPREADSHEETDIALOG_H
#define EXPORTSPREADSHEETDIALOG_H

#include <KDialog>
#include <KUrlCompletion>
#include "ui_exportspreadsheetwidget.h"

class ExportSpreadsheetDialog: public KDialog {
	Q_OBJECT

	public:
		explicit ExportSpreadsheetDialog(QWidget*);
		virtual ~ExportSpreadsheetDialog();

		QString path() const;
		void setFileName(const QString&);
		void setMatrixMode(bool);
		bool exportHeader() const;
        bool exportLatexHeader() const;
        bool gridLines() const;
        bool captions() const;
        bool skipEmptyRows() const;
        bool exportSelection() const;
        bool entireSpreadheet() const;
        bool matrixVerticalHeader() const;
        bool matrixHorizontalHeader() const;
		QString separator() const;

        enum Format {
            ASCII = 0,
            Binary,
            LaTeX
        };

        Format format() const;
	private:
		QWidget* mainWidget;
		Ui::ExportSpreadsheetWidget ui;
		bool m_showOptions;
        bool m_matrixMode;
        Format m_format;
        KUrlCompletion *urlCompletion;

	private slots:
        void setFormat(Format format);
		void slotButtonClicked(int);
		void okClicked();
		void toggleOptions();
		void selectFile();
		void formatChanged(int);
		void fileNameChanged(const QString&);
};

#endif
