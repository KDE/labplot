/*
	File                 : NotesDock.cpp
	Project              : LabPlot
	Description          : Dock for configuring notes
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2016 Garvit Khatri <garvitdelhi@gmail.com>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "NoteDock.h"
#include "frontend/TemplateHandler.h"

#include <KConfig>
#include <KConfigGroup>
#include <KLocalizedString>
#include <QDir>

NoteDock::NoteDock(QWidget* parent)
	: BaseDock(parent) {
	ui.setupUi(this);
	setBaseWidgets(ui.leName, ui.teComment);

	connect(ui.kcbBgColor, &KColorButton::changed, this, &NoteDock::backgroundColorChanged);
	connect(ui.kcbTextColor, &KColorButton::changed, this, &NoteDock::textColorChanged);
	connect(ui.kfrTextFont, &KFontRequester::fontSelected, this, &NoteDock::textFontChanged);

	auto* templateHandler = new TemplateHandler(this, QLatin1String("Note"));
	ui.gridLayout->addWidget(templateHandler, 8, 3);
	templateHandler->show();
	connect(templateHandler, &TemplateHandler::loadConfigRequested, this, &NoteDock::loadConfigFromTemplate);
	connect(templateHandler, &TemplateHandler::saveConfigRequested, this, &NoteDock::saveConfigAsTemplate);
}

void NoteDock::setNotesList(QList<Note*> list) {
	m_notesList = list;
	m_note = list.first();
	setAspects(std::move(list));

	connect(m_note, &Note::backgroundColorChanged, this, &NoteDock::noteBackgroundColorChanged);
	connect(m_note, &Note::textColorChanged, this, &NoteDock::noteTextColorChanged);
	connect(m_note, &Note::textFontChanged, this, &NoteDock::noteTextFontChanged);

	CONDITIONAL_LOCK_RETURN;

	ui.kcbBgColor->setColor(m_note->backgroundColor());
	ui.kcbTextColor->setColor(m_note->textColor());
	ui.kfrTextFont->setFont(m_note->textFont());
}

//*************************************************************
//********** SLOTs for changes triggered in NoteDock **********
//*************************************************************
void NoteDock::backgroundColorChanged(const QColor& color) {
	CONDITIONAL_LOCK_RETURN;

	for (auto* note : m_notesList)
		note->setBackgroundColor(color);
}

void NoteDock::textColorChanged(const QColor& color) {
	CONDITIONAL_LOCK_RETURN;

	for (auto* note : m_notesList)
		note->setTextColor(color);
}

void NoteDock::textFontChanged(const QFont& font) {
	CONDITIONAL_LOCK_RETURN;

	for (auto* note : m_notesList)
		note->setTextFont(font);
}

//*************************************************************
//************ SLOTs for changes triggered in Note ************
//*************************************************************
void NoteDock::noteBackgroundColorChanged(const QColor& color) {
	CONDITIONAL_LOCK_RETURN;
	ui.kcbBgColor->setColor(color);
}

void NoteDock::noteTextColorChanged(const QColor& color) {
	CONDITIONAL_LOCK_RETURN;
	ui.kcbTextColor->setColor(color);
}

void NoteDock::noteTextFontChanged(const QFont& font) {
	CONDITIONAL_LOCK_RETURN;
	ui.kfrTextFont->setFont(font);
}

//*************************************************************
//************************* Settings **************************
//*************************************************************
void NoteDock::loadConfigFromTemplate(KConfig& config) {
	KConfigGroup group = config.group(QStringLiteral("Notes"));
	ui.kcbBgColor->setColor(group.readEntry(QStringLiteral("BackgroundColor"), m_note->backgroundColor()));
	ui.kcbTextColor->setColor(group.readEntry(QStringLiteral("TextColor"), m_note->textColor()));
	ui.kfrTextFont->setFont(group.readEntry(QStringLiteral("TextColor"), m_note->textFont()));
}

void NoteDock::saveConfigAsTemplate(KConfig& config) {
	KConfigGroup group = config.group(QStringLiteral("Notes"));

	group.writeEntry(QStringLiteral("BackgroundColor"), ui.kcbBgColor->color());
	group.writeEntry(QStringLiteral("TextColor"), ui.kcbTextColor->color());
	group.writeEntry(QStringLiteral("TextFont"), ui.kfrTextFont->font());
}
