#ifndef PYTHONSCRIPTRUNTIME_H
#define PYTHONSCRIPTRUNTIME_H

#include <sbkpython.h>

#include "backend/script/ScriptRuntime.h"

class PythonLogger;

class PythonScriptRuntime : public ScriptRuntime {
    Q_OBJECT

public:
    explicit PythonScriptRuntime(Script*);
    virtual ~PythonScriptRuntime();

    virtual bool init() override;
    virtual bool cancel() override;
    virtual bool exec(const QString&) override;
    virtual QIcon icon() override;

private:
    PythonLogger* m_loggerStdOut{nullptr};
    PythonLogger* m_loggerStdErr{nullptr};
    PyObject* m_localDict{nullptr};

    // instance methods
    bool reset();
    bool redirectOutput();
    bool unRedirectOutput();

    // singleton methods (called once for all PythonScripts)
    static bool initPython();
    // static bool bindProject(Project*);
    // static bool unBindProject();

    // utilities
    static PyObject* getModuleDict(const QString&);
    static PyObject* getPyObjectFromModule(const QString&, const QString&);
    static bool setPyObjectInModule(const QString&, const QString&, PyObject*);
    static bool setObjectInModule(const QString&, const QString&, int, void*);
    static PyObject* shibokenConvertToPyObject(int, void*);
    static PyObject* createLocalDict();
    static int getPyErrorLine();

    // singletons (shared between PythonScripts)
    static bool ready;
    static PyObject* sysStdOut;
    static PyObject* sysStdErr;
};

#endif