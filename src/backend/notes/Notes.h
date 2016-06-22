/***************************************************************************
    File                 : Notes.h
    Project              : LabPlot
    Description          : Notes Widget for taking notes
    --------------------------------------------------------------------
    Copyright            : (C) 2009-2015 Garvit Khatri (garvitdelhi@gmail.com)

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

#ifndef NOTES_H
#define NOTES_H

#include "backend/core/AbstractPart.h"
#include "backend/core/AbstractScriptingEngine.h"
#include "backend/lib/macros.h"
#include "commonfrontend/notes/NotesView.h"

#include <QColor>
#include <QIcon>

class Notes : public AbstractPart, public scripted {
	Q_OBJECT
public:
	Notes(AbstractScriptingEngine* engine, const QString& name, bool loading = false);
	virtual ~Notes();

	virtual QWidget* view() const;
	virtual QMenu* createContextMenu();
	virtual QIcon icon() const;

	virtual bool exportView() const;
	virtual bool printView();
	virtual bool printPreview() const;
	
	void changeBgColor(QColor);
	void changeTextColor(QColor);

	virtual void save(QXmlStreamWriter*) const;
	virtual bool load(XmlStreamReader*);

	QColor bgColor();
	QColor textColor();
	void setNote(QString);
	QString note();
signals:
	void bgColorChanged(QColor);
	void textColorChanged(QColor);
private:
	void init();
	QColor m_bgColor;
	QColor m_textColor;
	QString m_note;
signals:
	void requestProjectContextMenu(QMenu*);

};

#endif // NOTES_H
