/***************************************************************************
    File                 : OpenProjectDialog.h
    Project              : SciDAVis
    --------------------------------------------------------------------
    Copyright            : (C) 2007 by Knut Franke, Ion Vasilief
    Email (use @ for *)  : knut.franke*gmx.de, ion_vasilief*yahoo.fr
    Description          : Dialog for opening project files.

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
#ifndef OPEN_PROJECT_DIALOG_H
#define OPEN_PROJECT_DIALOG_H

#include "../lib/ExtensibleFileDialog.h"

#include<QComboBox>

class OpenProjectDialog : public ExtensibleFileDialog
{
	Q_OBJECT
	public:
		enum OpenMode { NewProject, NewFolder };
		OpenProjectDialog(QWidget *parent=0, bool extended = true, Qt::WFlags flags=0);
		OpenMode openMode() const { return (OpenMode) m_open_mode->currentIndex(); }

	private:
		QComboBox *m_open_mode;

    protected slots:
		void closeEvent(QCloseEvent* );
        //! Update which options are visible and enabled based on the output format.
        void updateAdvancedOptions (const QString &filter);
};

#endif // ifndef OPEN_PROJECT_DIALOG_H
