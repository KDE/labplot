/*
	File                 : ScriptDock.cpp
	Project              : LabPlot
	Description          : Dock for configuring scripts
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2025 Israel Galadima <izzygaladima@gmail.com>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "ScriptDock.h"
#include "frontend/TemplateHandler.h"
#include "backend/script/Script.h"

#include <KConfig>
#include <KConfigGroup>
#include <KSyntaxHighlighting/Repository>
#include <KSyntaxHighlighting/Theme>
#include <KTextEditor/Editor>

ScriptDock::ScriptDock(QWidget* parent)
	: BaseDock(parent) {
	ui.setupUi(this);
	setBaseWidgets(ui.leName, ui.teComment);

    QStringList themeNames;
    for (KSyntaxHighlighting::Theme theme : KTextEditor::Editor::instance()->repository().themes())
        themeNames << theme.name();
    ui.cbTheme->addItems(themeNames);

	connect(ui.kfrEditorFont, &KFontRequester::fontSelected, this, &ScriptDock::setScriptEditorFont);
    connect(ui.kfrOutputFont, &KFontRequester::fontSelected, this, &ScriptDock::setScriptOutputFont);
    connect(ui.cbTheme, &QComboBox::currentTextChanged, this, &ScriptDock::setScriptEditorTheme);

	auto* templateHandler = new TemplateHandler(this, QLatin1String("Script"));
    templateHandler->setSaveDefaultAvailable(false);
	connect(templateHandler, &TemplateHandler::loadConfigRequested, this, &ScriptDock::loadConfigFromTemplate);
	connect(templateHandler, &TemplateHandler::saveConfigRequested, this, &ScriptDock::saveConfigAsTemplate);
    connect(templateHandler, &TemplateHandler::info, this, &ScriptDock::info);
    ui.gridLayout->addWidget(templateHandler, 10, 3);
	templateHandler->show();
}

void ScriptDock::setScriptsList(QList<Script*> list) {
	m_scriptsList = list;
	m_script = list.first();
	setAspects(std::move(list));

	connect(m_script, &Script::editorFontChanged, this, &ScriptDock::setEditorFont);
    connect(m_script, &Script::outputFontChanged, this, &ScriptDock::setOutputFont);
    connect(m_script, &Script::editorThemeChanged, this, &ScriptDock::setEditorTheme);

	CONDITIONAL_LOCK_RETURN;

	ui.kfrEditorFont->setFont(m_script->editorFont());
    ui.kfrOutputFont->setFont(m_script->outputFont());
    ui.leRuntime->setText(m_script->language());
    ui.cbTheme->setCurrentText(m_script->editorTheme());
}

void ScriptDock::retranslateUi() {
}

//*************************************************************
//************ SLOTs for changes triggered in Script **********
//*************************************************************

void ScriptDock::setEditorFont(const QFont& font) {
	CONDITIONAL_LOCK_RETURN;
	ui.kfrEditorFont->setFont(font);
}

void ScriptDock::setOutputFont(const QFont& font) {
	CONDITIONAL_LOCK_RETURN;
	ui.kfrOutputFont->setFont(font);
}

void ScriptDock::setEditorTheme(const QString& theme) {
	CONDITIONAL_LOCK_RETURN;
	ui.cbTheme->setCurrentText(theme);
}

//*************************************************************
//************************* Settings **************************
//*************************************************************
void ScriptDock::loadConfigFromTemplate(KConfig& config) {
	KConfigGroup group = config.group(QStringLiteral("ScriptEditor"));

	ui.kfrEditorFont->setFont(group.readEntry(QStringLiteral("EditorFont"), m_script->editorFont()));
    ui.kfrOutputFont->setFont(group.readEntry(QStringLiteral("OutputFont"), m_script->outputFont()));
    ui.cbTheme->setCurrentText(group.readEntry(QStringLiteral("EditorTheme"), m_script->editorTheme()));
}

void ScriptDock::saveConfigAsTemplate(KConfig& config) {
	KConfigGroup group = config.group(QStringLiteral("ScriptEditor"));

    group.writeEntry(QStringLiteral("EditorFont"), ui.kfrEditorFont->font());
    group.writeEntry(QStringLiteral("OutputFont"), ui.kfrOutputFont->font());
    group.writeEntry(QStringLiteral("EditorTheme"), ui.cbTheme->currentText());

    config.sync();
}

void ScriptDock::setScriptEditorFont(const QFont& font) {
    CONDITIONAL_LOCK_RETURN;
    m_script->setEditorFont(font);
}

void ScriptDock::setScriptOutputFont(const QFont& font) {
    CONDITIONAL_LOCK_RETURN;
    m_script->setOutputFont(font);
}

void ScriptDock::setScriptEditorTheme(const QString& theme) {
    CONDITIONAL_LOCK_RETURN;
    m_script->setEditorTheme(theme);
}
