#include <QIcon>
#include <QMessageBox>
#include <QWidget>

#include <KConfigGroup>
#include <KLocalizedString>
#include <KTextEditor/Document>
#include <KTextEditor/Editor>
#include <KTextEditor/View>

#include "backend/core/Project.h"
#include "backend/lib/XmlStreamReader.h"
#include "backend/lib/macros.h"

#include "frontend/script/ScriptEditor.h"

#include "Script.h"
#include "ScriptRuntime.h"

#ifdef HAVE_PYTHON_SCRIPTING
#include "backend/script/python/PythonScriptRuntime.h"
#endif

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
	if (!Script::languages.contains(lang, Qt::CaseInsensitive)) {
		m_initialized = false;
		return;
	}

	m_language = lang;
	m_scriptRuntime = Script::newScriptRuntime(m_language, this);
	m_kTextEditorDocument->setMode(m_language);
	m_initialized = true;
	prepareDocument();
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
	return false;
}

QWidget* Script::view() const {
	if (!m_partView) {
		m_view = new ScriptEditor(const_cast<Script*>(this));
		m_partView = m_view;
	}

	return m_partView;
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

	writer->writeStartElement(QStringLiteral("output"));
	writer->writeAttribute(QStringLiteral("text"), dynamic_cast<ScriptEditor*>(view())->outputText());
	writer->writeEndElement();

	writer->writeEndElement(); // close "script" section
}

bool Script::load(XmlStreamReader* reader, bool preview) {
	if (!m_initialized)
		return false;

	if (!reader->isStartElement() || reader->name() != QLatin1String("script")) {
		reader->raiseError(i18n("no script element found"));
		return false;
	}

	if (!readBasicAttributes(reader))
		return false;

	QXmlStreamAttributes attribs;
	QString str;

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
			attribs = reader->attributes();

			// output text
			dynamic_cast<ScriptEditor*>(view())->writeOutput(false, attribs.value(QStringLiteral("text")).toString());
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

void Script::runScript() {
	if (!m_initialized)
		return;

	ScriptEditor* scriptView = dynamic_cast<ScriptEditor*>(view());

	m_kTextEditorDocument->clearMarks();
	m_scriptRuntime->clearErrorLine();
	scriptView->clearOutput();

	// connect to the writeOutput signal from the script runtime
	auto conn = connect(m_scriptRuntime, &ScriptRuntime::writeOutput, [scriptView](bool isErr, const QString& msg) {
		scriptView->writeOutput(isErr, msg); // write the output to the scripteditor output
	});

	// push all changes at once to undo stack
	beginMacro(i18n("%1: run %2 script", name(), m_language));
	Script::runScript(this, m_kTextEditorDocument->text()); // run the script
	endMacro();

	// disconnect from writeOutput signal
	disconnect(conn);

	int errorLine = m_scriptRuntime->errorLine();
	if (errorLine != -1)
		m_kTextEditorDocument->addMark(errorLine, KTextEditor::Document::MarkTypes::Error);
}

QAbstractItemModel* Script::variableModel() {
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

void Script::registerShortcuts() {
	static_cast<ScriptEditor*>(view())->registerShortcuts();
}

void Script::unregisterShortcuts() {
	static_cast<ScriptEditor*>(view())->unregisterShortcuts();
}

// static data members
QStringList Script::languages = {
#ifdef HAVE_PYTHON_SCRIPTING
	QStringLiteral("Python")
#endif
};
bool Script::m_isRunning = false;
Script* Script::m_lastRunScript = nullptr;

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

	QXmlStreamAttributes attribs = reader->attributes();
	QString str = attribs.value(QStringLiteral("runtime")).toString();

	if (str.isEmpty()) {
		reader->raiseError(QStringLiteral("runtime"));
		return {};
	}

	if (!Script::languages.contains(str, Qt::CaseInsensitive)) {
		reader->raiseError(QStringLiteral("runtime"));
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
