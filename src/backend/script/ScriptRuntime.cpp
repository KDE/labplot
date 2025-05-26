#include "ScriptRuntime.h"

#include "Script.h"

/*!
 * \class ScriptRuntime
 * \brief Abstract class providing an interface for scripting runtimes
 *
 * Each runtime hides its implementation details and we only call methods
 * implemented from the ScriptRuntime class
 *
 * \ingroup backend
 */

/*!
 * \fn ScriptRuntime::ScriptRuntime
 * \brief Constructs a new ScriptRuntime
 * \param lang - The programming language which the script runtime executes
 * \param script - The Script instance that owns this script runtime
 * \return returns a ScriptRuntime instance
 */
ScriptRuntime::ScriptRuntime(const QString& lang, Script* script)
	: QObject()
	, lang(lang)
	, m_name(script->name()) {
}

/*!
 * \brief Gets the line in the most recently executed script where an error occurred
 * \return returns the line where the error occurred or -1 if no error occurred or the line is unknown
 */
int ScriptRuntime::errorLine() const {
	return m_errorLine;
}

/*!
 * \fn virtual bool ScriptRuntime::init() = 0;
 * \brief Initializes the script runtime
 *
 * This is the first method called for every instance of
 * the script runtime
 *
 * \return Whether the runtime initialization was successful or not
 */

/*!
 * \fn virtual bool ScriptRuntime::cancel() = 0;
 * \brief Stops the current execution
 * \return Whether stopping the execution was successful or not
 */

/*!
 * \fn virtual bool ScriptRuntime::exec(const QString& code) = 0;
 * \brief Executes the statements using the runtime
 * \param code - The statements to execute
 * \return Whether the execution was successful or not
 */

/*!
 * \fn virtual QIcon ScriptRuntime::icon() = 0;
 * \return The icon for the script runtime
 */
