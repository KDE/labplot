/***************************************************************************
    File                 : Notes.h
    Project              : LabPlot
    Description          : Widget for taking notes
    --------------------------------------------------------------------
    Copyright            : (C) 2016 Garvit Khatri (garvitdelhi@gmail.com)
    Copyright            : (C) 2016-2017 Alexander Semke (alexander.semke@web.de)

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

#include <QFont>
#include <QIcon>

class QColor;
class NoteView;

class Note : public AbstractPart {
	Q_OBJECT

public:
	explicit Note(const QString& name);

	QWidget* view() const override;
	QIcon icon() const override;

	bool exportView() const override;
	bool printView() override;
	bool printPreview() const override;

	void setNote(const QString&);
	void setText(const QString& s) {this->setNote(s);}
	const QString& note() const;

	void setBackgroundColor(const QColor&);
	const QColor& backgroundColor() const;

	void setTextColor(const QColor&);
	const QColor& textColor() const;

	void setTextFont(const QFont&);
	const QFont& textFont() const;

	void save(QXmlStreamWriter*) const override;
	bool load(XmlStreamReader*, bool preview) override;

signals:
	void backgroundColorChanged(QColor);
	void textColorChanged(QColor);
	void textFontChanged(QFont);

private:
	mutable NoteView* m_view{nullptr};
	QColor m_backgroundColor;
	QColor m_textColor;
	QFont m_textFont;
	QString m_note;
};

#endif // NOTE_H
