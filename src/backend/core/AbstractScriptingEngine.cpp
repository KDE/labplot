/***************************************************************************
    File                 : AbstractScriptingEngine.cpp
    Project              : SciDAVis
    --------------------------------------------------------------------
    Copyright            : (C) 2006,2008 by Knut Franke
    Email (use @ for *)  : knut.franke*gmx.de
    Description          : Implementations of generic scripting classes
                           
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
#include "AbstractScriptingEngine.h"

#include <QStringList>

#include <KLocalizedString>

/**
 * \class AbstractScriptingEngine
 * \brief An interpreter for evaluating scripting code.
 *
 * AbstractScriptingEngine objects represent a running interpreter, possibly with global
 * variables, and are responsible for generating AbstractScript objects (which do
 * the actual evaluation of code).
 *
 * The class also keeps a static list of available interpreters and instantiates
 * them on demand.
 */

/**
 * \brief Only initialize general information here.
 *
 * All AbstractScriptingEngine subclasses are instantiated at startup (more precisely: when
 * instantiating ScriptingEngineManager). Therefor, loading the actual interpreter is done
 * in initialize().
 */
AbstractScriptingEngine::AbstractScriptingEngine(const char *lang_name) {
	setObjectName(lang_name);
}

/**
 * \fn void AbstractScriptingEngine::initialize()
 * \brief Initialize the scripting environment.
 *
 * Don't forget to set m_initialized to true in implementations after a successful
 * initialization.
 */

/**
 * \brief initialization of the interpreter may fail; or there could be other errors setting up the environment
 */
bool AbstractScriptingEngine::initialized() const {
	return m_initialized;
}

/**
 * \brief whether asynchronuous execution is enabled (if supported by the implementation)
 */
bool AbstractScriptingEngine::isRunning() const {
	return false;
}

/**
 * \fn AbstractScript *AbstractScriptingEngine::makeScript(const QString &code, QObject *context, const QString &name)
 * \brief Instantiate the AbstractScript subclass matching the AbstractScriptingEngine subclass.
 */

/**
 * \brief If an exception / error occurred, return a nicely formatted stack backtrace.
 */
QString AbstractScriptingEngine::stackTraceString() {
	return QString();
}

/**
 * \brief Clear the global environment.
 *
 * What exactly happens depends on the implementation.
 */
void AbstractScriptingEngine::clear() {}

/**
 * \brief If the implementation supports asynchronuos execution, deactivate it.
 */
void AbstractScriptingEngine::stopExecution() {}

/**
 * \brief If the implementation supports asynchronuos execution, activate it.
 */
void AbstractScriptingEngine::startExecution() {}

/**
 * \brief Return a list of supported mathematical functions.
 *
 * These should be imported into the global namespace.
 */
const QStringList AbstractScriptingEngine::mathFunctions() const {
	return QStringList();
}

/**
 * \brief Return a documentation string for the given mathematical function.
 */
const QString AbstractScriptingEngine::mathFunctionDoc(const QString&) const {
	return QString();
}

/**
 * \brief Return a list of file extensions commonly used for this language.
 */
const QStringList AbstractScriptingEngine::fileExtensions() const {
	return QStringList();
}

/**
 * \brief Construct a filter expression from fileExtensions(), suitable for QFileDialog.
 */
const QString AbstractScriptingEngine::nameAndPatterns() const
{
	QStringList extensions = fileExtensions();
	if (extensions.isEmpty())
		return "";
	else
		return i18n("%1 Source (*.%2)", objectName(), extensions.join(" *."));
}

/**
 * \brief Increase the reference count.
 *
 * This should only be called by scripted and Script to avoid memory leaks.
 */
void AbstractScriptingEngine::incref()
{
	m_refcount++;
}

/**
 * \brief Decrease the reference count.
 *
 * This should only be called by scripted and Script to avoid segfaults.
 */
void AbstractScriptingEngine::decref()
{
	m_refcount--;
	if (m_refcount==0)
		delete this;
}

/**
 * \fn void AbstractScriptingEngine::error(const QString &message, const QString &scriptName, int lineNumber)
 * \brief signal an error condition / exception
 */

/**
 * \fn void AbstractScriptingEngine::print(const QString &output)
 * \brief output that is not handled by a Script
 */

/**
 * \var AbstractScriptingEngine::m_initialized
 * \brief whether the interpreter has been successfully initialized
 */

/**
 * \var AbstractScriptingEngine::m_refcount
 * \brief the reference counter
 */

/******************************************************************************\
 *Helper classes for managing instances of AbstractScriptingEngine subclasses.*
\******************************************************************************/

/**
 * \class ScriptingChangeEvent
 * \brief notify an object that it should update its scripting environment (see class scripted)
 */

/**
 * \class scripted
 * \brief Interface for maintaining a reference to the current AbstractScriptingEngine
 *
 * Every class that wants to use a AbstractScriptingEngine should subclass this one and
 * implement slot customEvent(QEvent*) such that it forwards any
 * AbstractScriptingChangeEvents to scripted::scriptingChangeEvent.
 */

scripted::scripted(AbstractScriptingEngine *engine)
{
	if (engine)
		engine->incref();
	m_scripting_engine = engine;
}

scripted::~scripted()
{
	if (m_scripting_engine)
		m_scripting_engine->decref();
}

void scripted::scriptingChangeEvent(ScriptingChangeEvent *sce)
{
	m_scripting_engine->decref();
	sce->scriptingEngine()->incref();
	m_scripting_engine = sce->scriptingEngine();
}
