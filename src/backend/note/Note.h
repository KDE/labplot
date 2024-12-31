/*
	File                 : Notes.h
	Project              : LabPlot
	Description          : Widget for taking notes
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2016 Garvit Khatri <garvitdelhi@gmail.com>
	SPDX-FileCopyrightText: 2016-2024 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef NOTE_H
#define NOTE_H

#include "backend/core/AbstractPart.h"
#include "backend/lib/macros.h"

#include <QFont>
#include <QIcon>

class NotePrivate;
#ifndef SDK
class NoteView;
#endif

class Note : public AbstractPart {
	Q_OBJECT

public:
	explicit Note(const QString& name);

	QWidget* view() const override;
	QIcon icon() const override;

	bool exportView() const override;
	bool printView() override;
	bool printPreview() const override;

	CLASS_D_ACCESSOR_DECL(QString, text, Text)
	CLASS_D_ACCESSOR_DECL(QColor, backgroundColor, BackgroundColor)
	CLASS_D_ACCESSOR_DECL(QColor, textColor, TextColor)
	CLASS_D_ACCESSOR_DECL(QFont, textFont, TextFont)

	void save(QXmlStreamWriter*) const override;
	bool load(XmlStreamReader*, bool preview) override;

	typedef NotePrivate Private;

Q_SIGNALS:
	void textChanged(const QString&);
	void backgroundColorChanged(const QColor&);
	void textColorChanged(const QColor&);
	void textFontChanged(const QFont&);

private:
	Q_DECLARE_PRIVATE(Note)
	NotePrivate* const d_ptr;
#ifndef SDK
	mutable NoteView* m_view{nullptr};
#endif
};

#endif // NOTE_H
