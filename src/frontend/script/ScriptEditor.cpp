/*
	File                 : ScriptEditor.cpp
	Project              : LabPlot
	Description          : Script editor
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2025 Israel Galadima <izzygaladima@gmail.com>
	SPDX-FileCopyrightText: 2026 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "ScriptEditor.h"
#include "ScriptCompletionModel.h"
#include "backend/script/Script.h"
#include <backend/lib/macros.h>

#include <KTextEditor/Editor>
#include <KTextEditor/Document>
#include <KTextEditor/View>
#include <KConfig>
#include <KConfigGroup>

#include <QMenu>
#include <QClipboard>
#include <QKeySequence>

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
	const bool outputVisible = group.readEntry(QStringLiteral("OutputVisible"), true);

	initActions();
	setOutputVisible(outputVisible);
	m_toggleOutputAction->setChecked(outputVisible);

	connect(m_script, &Script::requestProjectContextMenu, this, &ScriptEditor::createContextMenu);
	connect(m_script, &Script::viewPrint, [kTextEditorView = m_kTextEditorView] {
			kTextEditorView->print();
		});
	connect(m_script, &Script::viewPrintPreview, [kTextEditorView = m_kTextEditorView] {
			kTextEditorView->printPreview();
		});

	ui.teOutput->setReadOnly(true);
	ui.teOutput->setOpenLinks(false);

	// Connect anchor click handler for line navigation
	connect(ui.teOutput, &QTextBrowser::anchorClicked, this, &ScriptEditor::handleAnchorClicked);

	// Create and register code completion model in KTextEditor view
	m_completionModel = new ScriptCompletionModel(this);
	m_kTextEditorView->registerCompletionModel(m_completionModel);
	m_kTextEditorView->setAutomaticInvocationEnabled(true);

	// Setup manual code completion shortcut
	// Use Ctrl+Shift+Space on macOS (Ctrl+Space conflicts with keyboard language switching)
	// Use Ctrl+Space on other platforms (standard KTextEditor shortcut)
	m_codeCompletionAction = new QAction(QStringLiteral("Code Completion"), this);
#ifdef Q_OS_MACOS
	m_codeCompletionAction->setShortcut(Qt::CTRL | Qt::SHIFT | Qt::Key_Space);
#else
	m_codeCompletionAction->setShortcut(Qt::CTRL | Qt::Key_Space);
#endif
	m_codeCompletionAction->setWhatsThis(QStringLiteral("Manually trigger code completion"));
	connect(m_codeCompletionAction, &QAction::triggered, [this]() {
		if (m_completionModel && m_kTextEditorView) {
			auto cursor = m_kTextEditorView->cursorPosition();
			auto wordRange = m_kTextEditorView->document()->wordRangeAt(cursor);
			m_kTextEditorView->startCompletion(wordRange, m_completionModel);
		}
	});
	m_kTextEditorView->addAction(m_codeCompletionAction);
}

ScriptEditor::~ScriptEditor() {
	// Unregister completion model
	if (m_completionModel && m_kTextEditorView)
		m_kTextEditorView->unregisterCompletionModel(m_completionModel);

	KConfig config;
	auto group = config.group(QStringLiteral("ScriptEditor"));
	// we dont manage default editor font or themes ourselves, so no need to save in our config
	group.writeEntry(QStringLiteral("OutputFont"), outputFont());
	group.writeEntry(QStringLiteral("SplitterState"), splitterState());
	group.writeEntry(QStringLiteral("OutputVisible"), m_toggleOutputAction->isChecked());
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

	m_runScriptAction->setEnabled(m_script->isInitialized());

	QAction* firstAction = nullptr;

	if (menu->actions().size() > 1)
		firstAction = menu->actions().at(1);

	menu->insertAction(firstAction, m_runScriptAction);

	if (firstAction)
		menu->insertSeparator(firstAction);
}

void ScriptEditor::initActions() {
	m_runScriptAction = new QAction(QIcon::fromTheme(QStringLiteral("quickopen")), QStringLiteral("Run"), this);
	m_runScriptAction->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_R));
	m_runScriptAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
	m_runScriptAction->setShortcutVisibleInContextMenu(true);
	m_runScriptAction->setWhatsThis(QStringLiteral("Run the script"));
	connect(m_runScriptAction, &QAction::triggered, this, &ScriptEditor::run);

	m_clearOutputAction = new QAction(QIcon::fromTheme(QStringLiteral("edit-clear")), QStringLiteral("Clear Output"), this);
	m_clearOutputAction->setWhatsThis(QStringLiteral("Clear the output of the script editor"));
	connect(m_clearOutputAction, &QAction::triggered, this, &ScriptEditor::clearOutput);

	m_toggleOutputAction = new QAction(QIcon::fromTheme(QStringLiteral("view-visible")), QStringLiteral("Toggle Output"), this);
	m_toggleOutputAction->setCheckable(true);
	m_toggleOutputAction->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_J));
	m_toggleOutputAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
	m_toggleOutputAction->setWhatsThis(QStringLiteral("Show or hide the script output"));
	connect(m_toggleOutputAction, &QAction::toggled, this, &ScriptEditor::setOutputVisible);
	addAction(m_toggleOutputAction);
	ui.tbMinimizeOutput->setDefaultAction(m_toggleOutputAction);

	m_maximizeOutputAction = new QAction(QIcon::fromTheme(QStringLiteral("view-fullscreen")), QStringLiteral("Maximize Output"), this);
	m_maximizeOutputAction->setCheckable(true);
	m_maximizeOutputAction->setWhatsThis(QStringLiteral("Maximize or restore the script output"));
	connect(m_maximizeOutputAction, &QAction::toggled, this, &ScriptEditor::setOutputMaximized);
	ui.tbMaximizeOutput->setDefaultAction(m_maximizeOutputAction);
	ui.tbClearOutput->setDefaultAction(m_clearOutputAction);

	// Local search action to override global Ctrl+F and delegate to KTextEditor
	auto* m_searchAction = new QAction(this);
	connect(m_searchAction, &QAction::triggered, [this]() {
		if (m_kTextEditorView) {
			auto* action = m_kTextEditorView->action(QStringLiteral("edit_find"));
			if (action)
				action->trigger();
		}
	});
	addAction(m_searchAction);
}

void ScriptEditor::writeOutput(bool isErr, const QString& msg) {
	DEBUG(Q_FUNC_INFO << ", text = '" << msg.toStdString() << "'")
	if (msg.isEmpty())
		return;

	KConfig config;
	const auto group = config.group(QStringLiteral("ScriptEditor"));
	if (group.readEntry(QStringLiteral("AutoShowOutput"), true) && !ui.tray->isVisible())
		m_toggleOutputAction->setChecked(true);

	// Process the output text to add links and formatting
	QString processedHtml = processOutputText(isErr, msg);

	// Insert formatted HTML at the end
	auto cursor = ui.teOutput->textCursor();
	cursor.movePosition(QTextCursor::End);
	ui.teOutput->setTextCursor(cursor);
	ui.teOutput->insertHtml(processedHtml);

	// Ensure we scroll to the bottom
	ui.teOutput->ensureCursorVisible();
}

void ScriptEditor::setSplitterState(const QByteArray& state) {
	ui.splitter->restoreState(state);
}

void ScriptEditor::setOutputVisible(bool visible) {
	if (!visible)
		m_outputSplitterSizes = ui.splitter->sizes();

	ui.tray->setVisible(visible);

	if (visible && !m_outputSplitterSizes.isEmpty())
		ui.splitter->setSizes(m_outputSplitterSizes);

}

void ScriptEditor::setOutputMaximized(bool maximized) {
	if (maximized) {
		m_maximizedOutputSplitterSizes = ui.splitter->sizes();
		ui.splitter->setSizes({0, ui.splitter->height()});
	} else if (!m_maximizedOutputSplitterSizes.isEmpty())
		ui.splitter->setSizes(m_maximizedOutputSplitterSizes);
}

QByteArray ScriptEditor::splitterState() {
	return ui.splitter->saveState();
}

QString ScriptEditor::outputText() {
	return ui.teOutput->toPlainText();
}

void ScriptEditor::setOutputFont(const QFont& font) {
	ui.teOutput->setFont(font);
}

QFont ScriptEditor::outputFont() {
	return ui.teOutput->font();
}

// ##############################################################################
// ####################################  SLOTs   ################################
// ##############################################################################
void ScriptEditor::run() {
	WAIT_CURSOR_AUTO_RESET;
	QApplication::processEvents(QEventLoop::ExcludeUserInputEvents); // make sure the wait cursor is shown right away, before the blocking script execution starts
	m_script->runScript();
}

void ScriptEditor::clearOutput() {
	INFO(Q_FUNC_INFO)
	QFont currentOutputFont = outputFont();
	ui.teOutput->clear();
	ui.teOutput->setReadOnly(true);
	setOutputFont(currentOutputFont);
}

/*!
* Processes the output text to detect line references, errors, and warnings, and formats them as HTML for display in the output QTextBrowser.
* - Detects line references in the format "File "<string>", line 5, in <module>" or "File "somefile.py", line 5" 
	and converts them into clickable links that navigate to the corresponding line in the script editor.
* - Detects error and exception names (e.g., "NameError:", "ValueError:", "IndentationError:", "Traceback:") and formats them in red.
* - Detects warning patterns (e.g., "Warning", "UserWarning", "DeprecationWarning", "WARN") and formats them in orange.
* - Normal output is displayed in the default text color.
* - The output text is HTML-escaped to prevent HTML injection and ensure proper rendering.
* - Newlines are converted to <br> tags for proper line breaks in HTML.
* @param isErr Indicates whether the output is from stderr (true) or stdout (false).
* @param text The output text to process.
* @return The processed HTML string ready for display in the output QTextBrowser.
*/
QString ScriptEditor::processOutputText(bool isErr, const QString& text) {
	// Example code covering varios cases:
	/*
	print("Normal output")
	print("Warning: something might be wrong")
	print("Another WARN message")
	print("Regular output again")

	import warnings
	print("great text")
	warnings.warn("a simple test")

	my_list = [1, 2, 3]
	print("List contents:", my_list)
	print("Accessing invalid index...")
	value = my_list[10]  # IndexError: list index out of range
	*/

	DEBUG(Q_FUNC_INFO << ", isErr = " << isErr << ", text = '" << text.toStdString() << "'")

	// Detect and format Python traceback patterns BEFORE HTML escaping
	// Pattern 1: "File "<string>", line 5, in <module>" or "File "somefile.py", line 5"
	static QRegularExpression fileLinePattern(
		QStringLiteral(R"(File\s+"[^"]*",\s+line\s+(\d+))"),
		QRegularExpression::CaseInsensitiveOption
	);

	// Pattern 2: Error/Exception names (e.g., "NameError:", "ValueError:", "IndentationError:", "Traceback:")
	static QRegularExpression errorPattern(
		QStringLiteral(R"(^(\w+Error|\w+Exception|Traceback)(\s*\(.*\))?:)"),
		QRegularExpression::MultilineOption
	);

	// Pattern 3: Warning patterns - matches "Warning", "UserWarning", "DeprecationWarning", "WARN", etc.
	static QRegularExpression warningPattern(
		QStringLiteral(R"([Ww]arning|WARN)"),
		QRegularExpression::CaseInsensitiveOption
	);

	// Parse line references BEFORE escaping HTML
	struct LineMatch {
		int start;
		int length;
		QString lineNum;
		QString matchedText;
	};

	QList<LineMatch> matches;
	QRegularExpressionMatchIterator it = fileLinePattern.globalMatch(text);
	while (it.hasNext()) {
		QRegularExpressionMatch match = it.next();
		LineMatch lm;
		lm.start = match.capturedStart(0);
		lm.length = match.capturedLength(0);
		lm.lineNum = match.captured(1);
		lm.matchedText = match.captured(0);
		matches.append(lm);

		DEBUG(Q_FUNC_INFO << ", Found line reference: " << lm.matchedText.toStdString() << " at line " << lm.lineNum.toStdString())
	}

	// Now escape HTML
	QString processedText = text.toHtmlEscaped();

	// Convert newlines to <br> tags for proper line breaks in HTML
	processedText.replace(QLatin1String("\n"), QLatin1String("<br>"));

	// Replace matches in reverse order to preserve positions
	for (int i = matches.size() - 1; i >= 0; --i) {
		const LineMatch& lm = matches[i];

		// Escape the matched text for HTML
		QString escapedMatch = lm.matchedText.toHtmlEscaped();

		// Create clickable link with blue underlined style
		QString link = QStringLiteral("<a href=\"line:%1\" style=\"color: #1976d2; text-decoration: underline;\">%2</a>")
			.arg(lm.lineNum, escapedMatch);

		// Replace in the escaped text
		int startPos = lm.start;
		QString searchText = lm.matchedText.toHtmlEscaped();
		int pos = processedText.indexOf(searchText, startPos);
		if (pos != -1)
			processedText.replace(pos, searchText.length(), link);
	}

	QString html;

	// Apply color formatting - check warnings FIRST before checking isErr flag
	// because warnings.warn() writes to stderr but should be orange, not red
	if (warningPattern.match(text).hasMatch()) {
		// Warning - make it orange
		html = QStringLiteral("<span style=\"color: #f57c00; white-space: pre-wrap;\">%1</span>").arg(processedText);
	} else if (isErr || errorPattern.match(text).hasMatch()) {
		// Error output or Exception names - make them red
		html = QStringLiteral("<span style=\"color: #d32f2f; white-space: pre-wrap;\">%1</span>").arg(processedText);
	} else {
		// Normal output - wrap in span to ensure consistent rendering
		html = QStringLiteral("<span style=\"white-space: pre-wrap;\">%1</span>").arg(processedText);
	}

	DEBUG(Q_FUNC_INFO << ", Generated HTML: " << html.toStdString())

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
