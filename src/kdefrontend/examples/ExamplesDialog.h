/***************************************************************************
	File                 : ExamplesDialog.h
	Project              : LabPlot
	Description          : dialog showing the available example projects
	--------------------------------------------------------------------
	Copyright            : (C) 2021 by Alexander Semke (alexander.semke@web.de)

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
#ifndef EXAMPLESDIALOG_H
#define EXAMPLESDIALOG_H

#include <QDialog>

class ExamplesWidget;

class ExamplesDialog : public QDialog {
	Q_OBJECT

public:
	explicit ExamplesDialog(QWidget*);
	~ExamplesDialog() override;

	QString path() const;

private:
	ExamplesWidget* m_examplesWidget;

};

#endif // EXAMPLESDIALOG_H
