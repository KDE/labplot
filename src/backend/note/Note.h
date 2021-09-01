/*
    File                 : Notes.h
    Project              : LabPlot
    Description          : Widget for taking notes
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2016 Garvit Khatri <garvitdelhi@gmail.com>
    SPDX-FileCopyrightText: 2016-2017 Alexander Semke <alexander.semke@web.de>
    SPDX-License-Identifier: GPL-2.0-or-later
*/


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
