/***************************************************************************
    File                 : Notes.h
    Project              : LabPlot
    Description          : Widget for taking notes
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

#ifndef NOTE_H
#define NOTE_H

#include "backend/core/AbstractPart.h"

#include <QIcon>
#include <QColor>
#include <QFont>

class Note : public AbstractPart {
	Q_OBJECT

	public:
		explicit Note(const QString& name);

		virtual QWidget* view() const;
		virtual QIcon icon() const;

		virtual bool exportView() const;
		virtual bool printView();
		virtual bool printPreview() const;

		void setNote(const QString&);
		const QString& note() const;

		void setBackgroundColor(const QColor&);
		const QColor& backgroundColor() const;

		void setTextColor(const QColor&);
		const QColor& textColor() const;

		void setTextFont(const QFont&);
		const QFont& textFont() const;

		virtual void save(QXmlStreamWriter*) const;
		virtual bool load(XmlStreamReader*);

	signals:
		void backgroundColorChanged(QColor);
		void textColorChanged(QColor);
		void textFontChanged(QFont);

	private:
		QColor m_backgroundColor;
		QColor m_textColor;
		QFont m_textFont;
		QString m_note;
};

#endif // NOTE_H
