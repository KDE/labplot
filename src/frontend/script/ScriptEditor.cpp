/*
	File                 : ScriptEditor.cpp
	Project              : LabPlot
	Description          : Script editor
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2025 Israel Galadima <izzygaladima@gmail.com>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "ScriptEditor.h"
#include "backend/script/Script.h"
#include <backend/lib/macros.h>

#include <KTextEditor/Editor>
#include <KTextEditor/Document>
#include <KTextEditor/View>
#include <KConfig>
#include <KConfigGroup>

#include <QPushButton>
#include <QHBoxLayout>
#include <QMenu>
#include <QAction>
#include <QToolBar>
#include <QString>
#include <QToolButton>
#include <QVariant>
#include <QFont>
#include <QObject>
#include <QTextBrowser>
#include <QRegularExpression>
#include <QTextCursor>
#include <QTextCharFormat>
#include <QClipboard>
#include <QApplication>

ScriptEditor::ScriptEditor(Script* script, QWidget* parent)
	: QWidget(parent), m_script(script) {

	ui.setupUi(this);
	m_kTextEditorView = m_script->kTextEditorDocument()->createView(this);
	m_kTextEditorView->setContextMenu(m_kTextEditorView->defaultContextMenu(nullptr));
	ui.editorParent->layout()->addWidget(m_kTextEditorView);

	KConfig config;
	auto group = config.group(QStringLiteral("ScriptEditor"));
	// we dont manage default editor font or themes ourselves, so no need to check our config
	setOutputFont(group.readEntry(QStringLiteral("OutputFont"), QFont(QStringLiteral("monospace"), 10)));
	setSplitterState(group.readEntry(QStringLiteral("SplitterState"), splitterState())); // need reasonable default for splitter

	initActions();

	connect(m_script, &Script::requestProjectContextMenu, this, &ScriptEditor::createContextMenu);
	connect(m_script, &Script::viewPrint, [kTextEditorView = m_kTextEditorView] {
			kTextEditorView->print();
		});
	connect(m_script, &Script::viewPrintPreview, [kTextEditorView = m_kTextEditorView] {
			kTextEditorView->printPreview();
		});

	ui.output->setReadOnly(true);
	ui.output->setOpenLinks(false);

	// Connect anchor click handler for line navigation
	connect(ui.output, &QTextBrowser::anchorClicked, this, &ScriptEditor::handleAnchorClicked);

	// Setup context menu for output
	ui.output->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(ui.output, &QTextBrowser::customContextMenuRequested, this, &ScriptEditor::showOutputContextMenu);
}

ScriptEditor::~ScriptEditor() {    
	KConfig config;
	auto group = config.group(QStringLiteral("ScriptEditor"));
	// we dont manage default editor font or themes ourselves, so no need to save in our config
	group.writeEntry(QStringLiteral("OutputFont"), outputFont());
	group.writeEntry(QStringLiteral("SplitterState"), splitterState());
}

bool ScriptEditor::isInitialized() const {
	if (!m_script)
		return false;
	return m_script->isInitialized();
}

void ScriptEditor::createContextMenu(QMenu* menu) {
	Q_ASSERT(menu);
	if (!m_script)
		return;

	if (!m_script->isInitialized()) {
		m_runScriptAction->setEnabled(false);
		m_clearOutputAction->setEnabled(false);
	} else {
		m_runScriptAction->setEnabled(true);
		m_clearOutputAction->setEnabled(true);
	}

	QAction* firstAction = nullptr;

	if (menu->actions().size() > 1)
		firstAction = menu->actions().at(1);

	menu->insertAction(firstAction, m_runScriptAction);
	menu->insertSeparator(firstAction);
	menu->insertAction(firstAction, m_clearOutputAction);

	if (firstAction)
		menu->insertSeparator(firstAction);
}

void ScriptEditor::initActions() {
	m_runScriptAction = new QAction(QIcon::fromTheme(QStringLiteral("quickopen")), QStringLiteral("Run"), this);
	m_runScriptAction->setWhatsThis(QStringLiteral("Run the script"));
	connect(m_runScriptAction, &QAction::triggered, this, &ScriptEditor::run);

	m_clearOutputAction = new QAction(QIcon::fromTheme(QStringLiteral("edit-clear")), QStringLiteral("Clear Output"), this);
	m_clearOutputAction->setWhatsThis(QStringLiteral("Clear the output of the script editor"));
	connect(m_clearOutputAction, &QAction::triggered, this, &ScriptEditor::clearOutput);

	m_copySelectedAction = new QAction(QIcon::fromTheme(QStringLiteral("edit-copy")), QStringLiteral("Copy Selected"), this);
	m_copySelectedAction->setWhatsThis(QStringLiteral("Copy selected text from output"));
	connect(m_copySelectedAction, &QAction::triggered, [this]() {
		QApplication::clipboard()->setText(ui.output->textCursor().selectedText());
	});

	m_copyAllOutputAction = new QAction(QIcon::fromTheme(QStringLiteral("edit-copy")), QStringLiteral("Copy All Output"), this);
	m_copyAllOutputAction->setWhatsThis(QStringLiteral("Copy all output text"));
	connect(m_copyAllOutputAction, &QAction::triggered, [this]() {
		QApplication::clipboard()->setText(ui.output->toPlainText());
	});
}

void ScriptEditor::writeOutput(bool isErr, const QString& msg) {
	DEBUG(Q_FUNC_INFO << ", text = '" << msg.toStdString() << "'")
	if (msg.isEmpty())
		return;

	// Process the output text to add links and formatting
	QString processedHtml = processOutputText(isErr, msg);

	// Insert formatted HTML at the end
	auto cursor = ui.output->textCursor();
	cursor.movePosition(QTextCursor::End);
	ui.output->setTextCursor(cursor);
	ui.output->insertHtml(processedHtml);
}

void ScriptEditor::setSplitterState(const QByteArray& state) {
	ui.splitter->restoreState(state);
}

QByteArray ScriptEditor::splitterState() {
	return ui.splitter->saveState();
}

QString ScriptEditor::outputText() {
	return ui.output->toPlainText();
}

void ScriptEditor::setOutputFont(const QFont& font) {
	ui.output->setFont(font);
}

QFont ScriptEditor::outputFont() {
	return ui.output->font();
}

void ScriptEditor::registerShortcuts() {
}

void ScriptEditor::unregisterShortcuts() {
}

// ##############################################################################
// ####################################  SLOTs   ################################
// ##############################################################################
void ScriptEditor::run() {
	m_script->runScript();
}

void ScriptEditor::clearOutput() {
	INFO(Q_FUNC_INFO)
	QFont currentOutputFont = outputFont();
	ui.output->clear();
	ui.output->setReadOnly(true);
	setOutputFont(currentOutputFont);
}

QString ScriptEditor::processOutputText(bool isErr, const QString& text) {
	QString html;
	QString escapedText = text.toHtmlEscaped();

	// Detect and format Python traceback patterns
	// Pattern 1: "File "<string>", line 5, in <module>"
	static QRegularExpression fileLinePattern(
		QStringLiteral(R"(File\s+"[^"]*",\s+line\s+(\d+))"),
		QRegularExpression::CaseInsensitiveOption
	);

	// Pattern 2: Error/Exception names (e.g., "NameError:", "ValueError:", "TypeError:")
	static QRegularExpression errorPattern(
		QStringLiteral(R"(^(\w+Error|\w+Exception|Traceback):)"),
		QRegularExpression::MultilineOption
	);

	// Pattern 3: Warning patterns
	static QRegularExpression warningPattern(
		QStringLiteral(R"(\bwarning\b|\bWARN\b)"),
		QRegularExpression::CaseInsensitiveOption
	);

	QString processedText = escapedText;

	// Add clickable links for line references
	QRegularExpressionMatchIterator it = fileLinePattern.globalMatch(processedText);
	int offset = 0;
	while (it.hasNext()) {
		QRegularExpressionMatch match = it.next();
		QString lineNum = match.captured(1);
		QString matchedText = match.captured(0);
		QString link = QStringLiteral("<a href=\"line:%1\">%2</a>").arg(lineNum, matchedText);

		int startPos = match.capturedStart(0) + offset;
		int length = match.capturedLength(0);
		processedText.replace(startPos, length, link);
		offset += link.length() - length;
	}

	// Apply color formatting
	if (isErr) {
		// Error output - make it red
		html = QStringLiteral("<span style=\"color: #d32f2f;\">%1</span>").arg(processedText);
	} else if (warningPattern.match(processedText).hasMatch()) {
		// Warning - make it orange
		html = QStringLiteral("<span style=\"color: #f57c00;\">%1</span>").arg(processedText);
	} else if (errorPattern.match(processedText).hasMatch()) {
		// Exception names - make them red
		html = QStringLiteral("<span style=\"color: #d32f2f;\">%1</span>").arg(processedText);
	} else {
		// Normal output
		html = processedText;
	}

	return html;
}

void ScriptEditor::handleAnchorClicked(const QUrl& url) {
	DEBUG(Q_FUNC_INFO << ", URL = " << url.toString().toStdString())

	// Handle line:N links
	if (url.scheme() == QLatin1String("line")) {
		QString lineStr = url.path();
		bool ok;
		int line = lineStr.toInt(&ok);
		if (ok && line > 0 && m_kTextEditorView) {
			// Jump to the specified line in the editor (1-based)
			m_kTextEditorView->setCursorPosition(KTextEditor::Cursor(line - 1, 0));
			m_kTextEditorView->setFocus();
		}
	}
}

void ScriptEditor::showOutputContextMenu(const QPoint& pos) {
	QMenu menu(this);

	bool hasSelection = ui.output->textCursor().hasSelection();
	m_copySelectedAction->setEnabled(hasSelection);

	menu.addAction(m_copySelectedAction);
	menu.addAction(m_copyAllOutputAction);
	menu.addSeparator();
	menu.addAction(m_clearOutputAction);

	menu.exec(ui.output->mapToGlobal(pos));
}
