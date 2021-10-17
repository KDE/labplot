/*
    File                 : NotesDock.cpp
    Project              : LabPlot
    Description          : Dock for configuring notes
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2016 Garvit Khatri <garvitdelhi@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "NoteDock.h"
#include "kdefrontend/TemplateHandler.h"

#include <QDir>
#include <KConfigGroup>
#include <KLocalizedString>
#include <KConfig>

NoteDock::NoteDock(QWidget *parent) : BaseDock(parent) {
	ui.setupUi(this);
	m_leName = ui.leName;
	m_teComment = ui.teComment;
	m_teComment->setFixedHeight(1.2 * m_leName->height());

	connect(ui.leName, &QLineEdit::textChanged, this, &NoteDock::nameChanged);
	connect(ui.teComment, &QTextEdit::textChanged, this, &NoteDock::commentChanged);

	connect(ui.kcbBgColor, &KColorButton::changed, this, &NoteDock::backgroundColorChanged);
	connect(ui.kcbTextColor, &KColorButton::changed, this, &NoteDock::textColorChanged);
	connect(ui.kfrTextFont, &KFontRequester::fontSelected, this, &NoteDock::textFontChanged);

	auto* templateHandler = new TemplateHandler(this, TemplateHandler::ClassName::Worksheet);
	ui.gridLayout->addWidget(templateHandler, 8, 3);
	templateHandler->show();
	connect(templateHandler, &TemplateHandler::loadConfigRequested, this, &NoteDock::loadConfigFromTemplate);
	connect(templateHandler, &TemplateHandler::saveConfigRequested, this, &NoteDock::saveConfigAsTemplate);
}

void NoteDock::setNotesList(QList< Note* > list) {
	m_notesList = list;
	m_notes = list.first();
	m_aspect = list.first();

	m_initializing = true;
	ui.leName->setText(m_notes->name());
	ui.leName->setStyleSheet("");
	ui.leName->setToolTip("");
	ui.kcbBgColor->setColor(m_notes->backgroundColor());
	ui.kcbTextColor->setColor(m_notes->textColor());
	ui.kfrTextFont->setFont(m_notes->textFont());
	m_initializing = false;

	connect(m_notes, &Note::aspectDescriptionChanged, this, &NoteDock::aspectDescriptionChanged);
}

//*************************************************************
//********** SLOTs for changes triggered in NoteDock **********
//*************************************************************
void NoteDock::backgroundColorChanged(const QColor& color) {
	if (m_initializing)
		return;

	for (auto* note : m_notesList)
		note->setBackgroundColor(color);
}

void NoteDock::textColorChanged(const QColor& color) {
	if (m_initializing)
		return;

	for (auto* note : m_notesList)
		note->setTextColor(color);
}

void NoteDock::textFontChanged(const QFont& font) {
	if (m_initializing)
		return;

	for (auto* note : m_notesList)
		note->setTextFont(font);
}

//*************************************************************
//************************* Settings **************************
//*************************************************************
void NoteDock::loadConfigFromTemplate(KConfig& config) {
	QString name;
	int index = config.name().lastIndexOf(QLatin1String("/"));
	if (index != -1)
		name = config.name().right(config.name().size() - index - 1);
	else
		name = config.name();

	KConfigGroup group = config.group("Notes");
	ui.kcbBgColor->setColor(group.readEntry("BackgroundColor", m_notes->backgroundColor()));
	ui.kcbTextColor->setColor(group.readEntry("TextColor", m_notes->textColor()));
	ui.kfrTextFont->setFont(group.readEntry("TextColor", m_notes->textFont()));
}

void NoteDock::saveConfigAsTemplate(KConfig& config) {
	KConfigGroup group = config.group("Notes");

	group.writeEntry("BackgroundColor", ui.kcbBgColor->color());
	group.writeEntry("TextColor", ui.kcbTextColor->color());
	group.writeEntry("TextFont", ui.kfrTextFont->font());
}
