#include "PythonLogger.h"
#include "PythonScriptRuntime.h"

PythonLogger::PythonLogger(PythonScriptRuntime* pythonScript, bool isStdErr)
    : m_pythonScript(pythonScript)
    , m_isStdErr(isStdErr) {
}

void PythonLogger::write(const QString& text) {
    Q_EMIT m_pythonScript->writeOutput(m_isStdErr, text);
}
