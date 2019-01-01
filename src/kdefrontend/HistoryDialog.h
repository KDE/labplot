
/***************************************************************************
    File                 : HistoryDialog.h
    Project              : LabPlot
    Description          : history dialog
    --------------------------------------------------------------------
    Copyright            : (C) 2012-2016 by Alexander Semke (alexander.semke@web.de)

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

#ifndef HISTORYDIALOG_H
#define HISTORYDIALOG_H

#include <QDialog>

class QUndoStack;
class QPushButton;

class HistoryDialog: public QDialog {
	Q_OBJECT

public:
	HistoryDialog(QWidget*, QUndoStack*, const QString&);
	~HistoryDialog() override;

private:
	QUndoStack* m_undoStack;
	QPushButton* m_okButton;
	QPushButton* m_clearUndoStackButton{nullptr};

private slots:
	void clearUndoStack();
};

#endif
