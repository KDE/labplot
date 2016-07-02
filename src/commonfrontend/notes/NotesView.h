/***************************************************************************
    File                 : NotesView.cpp
    Project              : LabPlot
    Description          : Notes View for taking notes
    --------------------------------------------------------------------
    Copyright            : (C) 2016 Garvit Khatri (garvitdelhi@gmail.com)
    Copyright            : (C) 2016 Alexander Semke (alexander.semke@web.de)

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

class Notes;
class QTextEdit;

class NotesView : public QWidget {
	Q_OBJECT

	public:
		NotesView(Notes* notes);

	public slots:
		void print(QPrinter*) const;

	private slots:
		void backgroundColorChanged(QColor);
		void textColorChanged(QColor);
		void textFontChanged(QFont);
		void textChanged();

	private:
		Notes* m_notes;
		QTextEdit* m_textEdit;

};

#endif // NOTESVIEW_H
