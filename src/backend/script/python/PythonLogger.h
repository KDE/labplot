#ifndef PYTHON_LOGGER
#define PYTHON_LOGGER

#include <QString>

class PythonScriptRuntime;

class PythonLogger {
public:
	explicit PythonLogger(PythonScriptRuntime*, bool);
	void write(const QString&);

private:
	PythonScriptRuntime* m_pythonScript{nullptr};
	bool m_isStdErr{false};
};

#endif
