/*
	File                 : Script.cpp
	Project              : LabPlot
	Description          : Script
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2025 Israel Galadima <izzygaladima@gmail.com>
	SPDX-FileCopyrightText: 2026 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include <QDir>
#include <QFileDialog>
#include <QIcon>
#include <QMessageBox>
#include <QPainter>
#include <QPainterPath>
#include <QPointer>
#include <QWidget>

#include <KConfigGroup>
#include <KLocalizedString>
#include <KTextEditor/Document>
#include <KTextEditor/Editor>
#include <KTextEditor/View>

#include "backend/core/Project.h"
#include "backend/core/Settings.h"
#include "backend/lib/XmlStreamReader.h"
#include "backend/lib/macros.h"

#include "frontend/script/ScriptEditor.h"

#include "Script.h"
#include "ScriptRuntime.h"

#ifdef HAVE_PYTHON_SCRIPTING
#include "backend/script/python/PythonScriptRuntime.h"
#endif

namespace {
constexpr auto BreakpointMark = KTextEditor::Document::markType08;
constexpr auto ExecutionIconMark = KTextEditor::Document::markType09;
static_assert(ExecutionIconMark > BreakpointMark, "the current line icon must be drawn on top of the breakpoint icon");
}

/*!
  \class Script
  \brief Aspect providing an application scripting interface

  Script holds a pointer to a ScriptEditor, which manages the display of its contents.
  Script holds a pointer to a KTextDocument, which is the actual owner of its scripting
  contents.
  Script holds a pointer to a scripting runtime which actually implements the scripting
  functionality for that particular language.

  A script is left in an uninitialized state if it is initialized with an unknown language.

  \ingroup backend
*/
Script::Script(const QString& name, const QString& lang)
	: AbstractPart(name, AspectType::Script)
	, m_kTextEditorDocument(KTextEditor::Editor::instance()->createDocument(this)) {
	if (!lang.isEmpty())
		setRuntime(lang);

	connect(m_kTextEditorDocument, &KTextEditor::Document::textChanged, this, [this]() {
		setProjectChanged(true);
	});

	initMarks();

	connect(this, &Script::viewAboutToBeDeleted, [this]() {
		// keep the output of the editor so it is still available when saving the project or re-opening the view
		if (hasView()) {
			auto* editor = view();
			m_outputHtml = editor->outputText().isEmpty() ? QString() : editor->outputHtml();
		}
	});
}

bool Script::setRuntime(const QString& runtime) {
	if (m_initialized && m_language.compare(runtime, Qt::CaseInsensitive) == 0)
		return true;

	delete m_scriptRuntime;
	m_scriptRuntime = nullptr;
	m_language.clear();
	m_initialized = false;

	if (!Script::languages.contains(runtime, Qt::CaseInsensitive)) {
		WARN("Unsupported scripting language: " << STDSTRING(runtime))
		return false;
	}

	m_language = runtime;
	m_scriptRuntime = Script::newScriptRuntime(m_language, this);
	if (!m_scriptRuntime) {
		WARN("Failed to initialize the " << STDSTRING(m_language) << " script runtime. Check that the interpreter is available.")
		m_language.clear();
		return false;
	}

	m_kTextEditorDocument->setMode(m_language);
	m_initialized = true;
	prepareDocument();
	return true;
}

Script::~Script() {
	delete m_scriptRuntime;
}

QIcon Script::icon() const {
	if (!m_initialized)
		return {};
	return Script::icon(m_language);
}

bool Script::printView() {
	Q_EMIT viewPrint();
	return true;
}

bool Script::printPreview() const {
	Q_EMIT viewPrintPreview();
	return true;
}

bool Script::exportView() const {
	auto conf = Settings::group(QStringLiteral("ExportScript"));
	QString dir = conf.readEntry("LastDir", "");
	QString extensions = i18n("Python file (*.py)");

	const QString path = QFileDialog::getSaveFileName(view(), i18nc("@title:window", "Export to File"), dir, extensions);

	if (path.isEmpty())
		return false;

	int pos = path.lastIndexOf(QStringLiteral("/"));
	if (pos != -1) {
		QString newDir = path.left(pos);
		if (newDir != dir)
			conf.writeEntry(QStringLiteral("LastDir"), newDir);
	}

	QFile file(path);
	if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
		QMessageBox::critical(view(), i18n("Export failed"), i18n("Failed to open '%1' for writing.", QDir::toNativeSeparators(path)));
		return false;
	}

	QTextStream out(&file);
	out << m_kTextEditorDocument->text();
	file.close();

	return true;
}

QWidget* Script::createView() const {
	auto* newView = new ScriptEditor(const_cast<Script*>(this));
	if (!m_outputHtml.isEmpty())
		newView->setOutputHtml(m_outputHtml);
	return newView;
}

ScriptEditor* Script::view() const {
	return static_cast<ScriptEditor*>(AbstractPart::view());
}

void Script::save(QXmlStreamWriter* writer) const {
	if (!m_initialized)
		return;

	writer->writeStartElement(QStringLiteral("script"));
	writeBasicAttributes(writer);
	writer->writeAttribute(QStringLiteral("runtime"), m_language);

	writeCommentElement(writer);

	writer->writeStartElement(QStringLiteral("editor"));
	writer->writeAttribute(QStringLiteral("text"), m_kTextEditorDocument->text());
	writer->writeEndElement();

	const auto group = Settings::group(QStringLiteral("ScriptEditor"));
	if (group.readEntry(QStringLiteral("SaveOutput"), true) && !m_outputHtml.isEmpty()) {
		writer->writeStartElement(QStringLiteral("output"));
		writer->writeCharacters(m_outputHtml);
		writer->writeEndElement();
	}

	writer->writeEndElement(); // close "script" section
}

bool Script::load(XmlStreamReader* reader, bool preview) {
	if (!reader->isStartElement() || reader->name() != QLatin1String("script")) {
		reader->raiseError(i18n("no script element found"));
		return false;
	}
	const QString runtime = readRuntime(reader);
	if (runtime.isEmpty())
		return false;

	if (!setRuntime(runtime)) {
		reader->raiseError(i18n("failed to initialize the script runtime '%1'", runtime));
		return false;
	}

	if (!readBasicAttributes(reader))
		return false;

	QXmlStreamAttributes attribs;

	while (!reader->atEnd()) {
		reader->readNext();
		if (reader->isEndElement() && reader->name() == QLatin1String("script"))
			break;

		if (!reader->isStartElement())
			continue;

		if (reader->name() == QLatin1String("comment")) {
			if (!readCommentElement(reader))
				return false;
		} else if (!preview && reader->name() == QLatin1String("editor")) {
			attribs = reader->attributes();

			// editor text
			m_kTextEditorDocument->setText(attribs.value(QStringLiteral("text")).toString());
		} else if (!preview && reader->name() == QLatin1String("output")) {
			m_outputHtml = reader->readElementText(QXmlStreamReader::SkipChildElements);
			if (hasView())
				view()->setOutputHtml(m_outputHtml);
		} else { // unknown element
			reader->raiseUnknownElementWarning();
			if (!reader->skipToEndElement())
				return false;
		}
	}

	return true;
}

bool Script::isInitialized() {
	return m_initialized;
}

void Script::prepareRun() {
	clearRunMarks();
	m_scriptRuntime->clearErrorLine();
	view()->clearOutput();
	m_outputHtml.clear();
}

void Script::finishRun() {
	const int errorLine = m_scriptRuntime->errorLine();
	if (errorLine != -1)
		m_kTextEditorDocument->addMark(errorLine, KTextEditor::Document::MarkTypes::Error);
}

void Script::runScript() {
	if (!m_initialized)
		return;

	if (m_isRunning) {
		QMessageBox::critical(view(), i18n("Script"), i18n("A script is already running."));
		return;
	}

	auto* scriptView = static_cast<ScriptEditor*>(view());
	prepareRun();

	// connect to the writeOutput signal from the script runtime
	auto conn = connect(m_scriptRuntime, &ScriptRuntime::writeOutput, [this, scriptView](bool isErr, const QString& msg) {
		scriptView->writeOutput(isErr, msg); // write the output to the scripteditor output
		m_outputHtml = scriptView->outputHtml(); // sync backend copy
	});

	// since we're potentially creating multiple new objects or modifying existing objects when executing the script,
	// suppress the aspect change signal in the project explorer to avoid to many aspect changes and
	// push all changes at once to undo stack
	project()->setSuppressAspectAddedSignal(true);
	beginMacro(i18n("%1: run %2 script", name(), m_language));
	Script::runScript(this, m_kTextEditorDocument->text()); // run the script
	endMacro();
	project()->setSuppressAspectAddedSignal(false);

	// disconnect from writeOutput signal
	disconnect(conn);

	finishRun();
}

/*!
 * runs the script under the control of the debugger, the execution pauses at the breakpoints.
 * While paused, the GUI stays responsive and the execution is continued via debugContinue(), debugStepOver(), etc.
 */
void Script::debugScript() {
	if (!m_initialized || !supportsDebugging())
		return;

	if (m_isRunning) {
		QMessageBox::critical(view(), i18n("Script"), i18n("A script is already running."));
		return;
	}

	// the view can be closed while the execution is paused
	QPointer<ScriptEditor> scriptView = view();
	prepareRun();

	auto outputConn = connect(m_scriptRuntime, &ScriptRuntime::writeOutput, [this, scriptView](bool isErr, const QString& msg) {
		if (!scriptView)
			return;
		scriptView->writeOutput(isErr, msg);
		m_outputHtml = scriptView->outputHtml();
	});
	auto pausedConn = connect(m_scriptRuntime, &ScriptRuntime::debugPaused, this, [this](int line) {
		setExecutionLine(line);
		Q_EMIT debugStateChanged();
	});
	auto resumedConn = connect(m_scriptRuntime, &ScriptRuntime::debugResumed, this, [this]() {
		setExecutionLine(-1);
		Q_EMIT debugStateChanged();
	});

	m_isRunning = true;
	m_lastRunScript = this;
	m_debuggingScript = this;

	// the code being executed must match the content of the editor, don't allow modifications while debugging
	m_kTextEditorDocument->setReadWrite(false);
	Q_EMIT debugStateChanged();

	project()->setSuppressAspectAddedSignal(true);
	beginMacro(i18n("%1: debug %2 script", name(), m_language));
	const bool ok = m_scriptRuntime->debug(m_kTextEditorDocument->text(), breakpoints());
	endMacro();
	project()->setSuppressAspectAddedSignal(false);

	m_isRunning = false;
	m_debuggingScript = nullptr;

	disconnect(outputConn);
	disconnect(pausedConn);
	disconnect(resumedConn);

	m_kTextEditorDocument->setReadWrite(true);
	setExecutionLine(-1);
	finishRun();
	Q_EMIT debugStateChanged();

	if (!ok)
		QMessageBox::critical(scriptView, i18n("Script"), i18n("Failed to debug the script."));
}

void Script::debugContinue() {
	if (m_scriptRuntime)
		m_scriptRuntime->sendDebugCommand(ScriptRuntime::DebugCommand::Continue);
}

void Script::debugStepOver() {
	if (m_scriptRuntime)
		m_scriptRuntime->sendDebugCommand(ScriptRuntime::DebugCommand::StepOver);
}

void Script::debugStepInto() {
	if (m_scriptRuntime)
		m_scriptRuntime->sendDebugCommand(ScriptRuntime::DebugCommand::StepInto);
}

void Script::debugStepOut() {
	if (m_scriptRuntime)
		m_scriptRuntime->sendDebugCommand(ScriptRuntime::DebugCommand::StepOut);
}

void Script::debugStop() {
	if (m_scriptRuntime)
		m_scriptRuntime->sendDebugCommand(ScriptRuntime::DebugCommand::Stop);
}

bool Script::supportsDebugging() const {
	return m_initialized && m_scriptRuntime && m_scriptRuntime->supportsDebugging();
}

bool Script::isDebugging() const {
	return m_debuggingScript == this;
}

bool Script::isDebugPaused() const {
	return isDebugging() && m_scriptRuntime && m_scriptRuntime->isDebugPaused();
}

bool Script::isAnyScriptDebugging() {
	return m_debuggingScript != nullptr;
}

int Script::executionLine() const {
	return m_executionLine;
}

/*!
 * returns the sorted list of the (1-based) lines with a breakpoint
 */
QList<int> Script::breakpoints() const {
	QList<int> lines;
	const auto& marks = m_kTextEditorDocument->marks();
	for (const auto* mark : marks) {
		if (mark->type & BreakpointMark)
			lines << mark->line + 1;
	}
	std::sort(lines.begin(), lines.end());
	return lines;
}

/*!
 * toggles the breakpoint at the given 0-based line
 */
void Script::toggleBreakpoint(int line) {
	if (line < 0 || line >= m_kTextEditorDocument->lines())
		return;

	if (m_kTextEditorDocument->mark(line) & BreakpointMark)
		m_kTextEditorDocument->removeMark(line, BreakpointMark);
	else
		m_kTextEditorDocument->addMark(line, BreakpointMark);
}

/*!
 * icon for the line where the execution is paused
 */
static QIcon executionIcon() {
	QIcon icon;
	for (int size : {16, 22, 32, 64}) {
		QPixmap pixmap(size, size);
		pixmap.fill(Qt::transparent);

		QPainter painter(&pixmap);
		painter.setRenderHint(QPainter::Antialiasing);
		painter.scale(size / 64., size / 64.);

		QPainterPath arrow;
		arrow.moveTo(6, 22);
		arrow.lineTo(30, 22);
		arrow.lineTo(30, 6);
		arrow.lineTo(58, 32);
		arrow.lineTo(30, 58);
		arrow.lineTo(30, 42);
		arrow.lineTo(6, 42);
		arrow.closeSubpath();

		painter.setPen(QPen(QColor(64, 64, 64), 5, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
		painter.setBrush(QColor(255, 204, 0));
		painter.drawPath(arrow);
		painter.end();

		icon.addPixmap(pixmap);
	}
	return icon;
}

void Script::initMarks() {
	using KTextEditor::Document;
	m_kTextEditorDocument->setMarkDescription(BreakpointMark, i18n("Breakpoint"));
	m_kTextEditorDocument->setMarkIcon(BreakpointMark,
									   QIcon::fromTheme(QStringLiteral("debug-breakpoint"), QIcon::fromTheme(QStringLiteral("media-record"))));
	m_kTextEditorDocument->setMarkDescription(ExecutionIconMark, i18n("Current Line"));
	m_kTextEditorDocument->setMarkIcon(ExecutionIconMark, executionIcon());
	m_kTextEditorDocument->setMarkDescription(Document::Error, i18n("Error"));

	// allow to toggle breakpoints by clicking on the icon border of the editor
	m_kTextEditorDocument->setEditableMarks(BreakpointMark);

	connect(m_kTextEditorDocument,
			&Document::markChanged,
			this,
			[this](Document*, KTextEditor::Mark mark, Document::MarkChangeAction) {
				if (!(mark.type & BreakpointMark))
					return;

				// breakpoints are not saved in the project, only apply the change to the running debug session
				if (isDebugPaused())
					m_scriptRuntime->setBreakpoints(breakpoints());
			});
}

/*!
 * removes the marks of the previous run (error and current execution line), the breakpoints are kept
 */
void Script::clearRunMarks() {
	const uint runMarks = KTextEditor::Document::Error | KTextEditor::Document::Execution | ExecutionIconMark;
	QList<int> lines;
	const auto& marks = m_kTextEditorDocument->marks();
	for (const auto* mark : marks) {
		if (mark->type & runMarks)
			lines << mark->line;
	}
	for (int line : lines)
		m_kTextEditorDocument->removeMark(line, runMarks);
	m_executionLine = -1;
}

/*!
 * marks the 0-based line where the execution is currently paused, -1 to remove the mark
 */
void Script::setExecutionLine(int line) {
	// Execution: highlight of the line, ExecutionIconMark: icon in the icon border
	const uint marks = KTextEditor::Document::Execution | ExecutionIconMark;
	if (m_executionLine >= 0)
		m_kTextEditorDocument->removeMark(m_executionLine, marks);
	m_executionLine = line;
	if (m_executionLine >= 0)
		m_kTextEditorDocument->addMark(m_executionLine, marks);
}

QAbstractItemModel* Script::variableModel() {
	if (!m_scriptRuntime)
		return nullptr;
	return m_scriptRuntime->variableModel();
}

KTextEditor::Document* Script::kTextEditorDocument() const {
	return m_kTextEditorDocument;
}

ScriptRuntime* Script::scriptRuntime() const {
	if (!m_initialized)
		return nullptr;

	return m_scriptRuntime;
}

int Script::scriptErrorLine() const {
	if (!m_initialized)
		return -1;

	return m_scriptRuntime->errorLine();
}

void Script::prepareDocument() const {
	if (!m_initialized)
		return;

	if (!m_kTextEditorDocument->isEmpty())
		return;
#ifdef HAVE_PYTHON_SCRIPTING
	if (m_language.compare(QStringLiteral("python"), Qt::CaseInsensitive) == 0)
		m_kTextEditorDocument->setText(QStringLiteral("from pylabplot import *"));
#endif
}

void Script::cancelScript() {
	if (!m_initialized)
		return;

	Script::cancelScript(this);
}

QString Script::language() const {
	if (!m_initialized)
		return {};

	return m_language;
}

QMenu* Script::createContextMenu() {
	QMenu* menu = AbstractPart::createContextMenu();
	Q_ASSERT(menu);
	Q_EMIT requestProjectContextMenu(menu);
	return menu;
}

// static data members
QStringList Script::languages = {
#ifdef HAVE_PYTHON_SCRIPTING
	QStringLiteral("Python")
#endif
};
bool Script::m_isRunning = false;
Script* Script::m_lastRunScript = nullptr;
Script* Script::m_debuggingScript = nullptr;

// static member functions
ScriptRuntime* Script::newScriptRuntime(const QString& language, Script* script) {
	ScriptRuntime* scriptRuntime{nullptr};

#ifdef HAVE_PYTHON_SCRIPTING
	if (language.compare(QStringLiteral("python"), Qt::CaseInsensitive) == 0)
		scriptRuntime = new PythonScriptRuntime(script);
#endif

	if (!scriptRuntime)
		return nullptr;

	if (scriptRuntime->init()) {
		return scriptRuntime;
	} else {
		delete scriptRuntime;
		return nullptr;
	}
}

void Script::runScript(Script* script, const QString& code) {
	if (!m_isRunning) {
		m_isRunning = true;
		m_lastRunScript = script;

		if (!script->scriptRuntime()->exec(code)) {
			m_isRunning = false;
			QMessageBox::critical(script->view(), i18n("Script"), i18n("Failed to run the script."));
		}

		m_isRunning = false;
	} else {
		QMessageBox::critical(script->view(), i18n("Script"), i18n("A script is already running."));
	}
}

bool Script::cancelScript(Script* script) {
	if (m_lastRunScript != script)
		return false;

	// return script->scriptRuntime()->cancel();

	return false; // cannot have cancel functionality until scripts are run on separate thread from gui thread
}

QString Script::readRuntime(XmlStreamReader* reader) {
	if (!reader->isStartElement() || reader->name() != QLatin1String("script")) {
		reader->raiseError(i18n("no script element found"));
		return {};
	}

	auto attribs = reader->attributes();
	QString str = attribs.value(QStringLiteral("runtime")).toString();

	if (str.isEmpty()) {
		reader->raiseError(i18n("attribute 'runtime' is missing or empty"));
		return {};
	}

	if (!Script::languages.contains(str, Qt::CaseInsensitive)) {
		reader->raiseError(i18n("unsupported script runtime '%1'", str));
		return {};
	}

	return str;
}

QIcon Script::icon(const QString& lang) {
	if (!Script::languages.contains(lang, Qt::CaseInsensitive))
		return {};

	if (lang.compare(QStringLiteral("python"), Qt::CaseInsensitive) == 0)
		return QIcon::fromTheme(QStringLiteral("pythonbackend"));

	return QIcon::fromTheme(QStringLiteral("quickopen"));
}
