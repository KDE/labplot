/***************************************************************************
    File                 : NotesView.cpp
    Project              : LabPlot
    Description          : Notes View for taking notes
    --------------------------------------------------------------------
    Copyright            : (C) 2016-2016 Garvit Khatri (garvitdelhi@gmail.com)

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

#ifndef NOTESVIEW_H
#define NOTESVIEW_H

#include <QWidget>
#include <QTextEdit>
#include <QToolBar>


class Notes;
class NotesView : public QWidget {
	Q_OBJECT

public:
	NotesView(Notes* notes);
	~NotesView();
	
public slots:
	void createContextMenu(QMenu*) const;
	void fillToolBar(QToolBar*);
	void bgColorChanged(QColor);
	void textColorChanged(QColor);
	void textChanged();
private:
	Notes* m_notes;
	QTextEdit* m_textEdit;

};

#endif // NOTESVIEW_H
