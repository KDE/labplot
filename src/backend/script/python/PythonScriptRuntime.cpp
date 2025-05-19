#include "PythonScriptRuntime.h"
#include "PythonLogger.h"

#include <QDebug>

#include <sbkconverter.h>
#include <sbkmodule.h>

#include "pyerrors.h"
#include "pylabplot/pylabplot_python.h"
#include "backend/script/Script.h"
#include "backend/script/ScriptRuntime.h"
#include "backend/core/Project.h"

static const char moduleName[] = "pylabplot";
extern "C" PyObject* PyInit_pylabplot();
extern PyTypeObject** SbkpylabplotTypes;
extern SbkConverter** SbkpylabplotTypeConverters;

bool PythonScriptRuntime::ready{false};
PyObject* PythonScriptRuntime::sysStdOut{nullptr};
PyObject* PythonScriptRuntime::sysStdErr{nullptr};

PythonScriptRuntime::PythonScriptRuntime(Script* script)
    : ScriptRuntime(QStringLiteral("Python"), script)
    , m_loggerStdOut(new PythonLogger(this, false))
    , m_loggerStdErr(new PythonLogger(this, true)) {
}

PythonScriptRuntime::~PythonScriptRuntime() {
    Py_XDECREF(m_localDict);

    delete m_loggerStdOut;
    delete m_loggerStdErr;
}

bool PythonScriptRuntime::init() {
    // bool init = PythonScriptRuntime::initPython() && PythonScriptRuntime::bindProject(m_project);
    bool init = PythonScriptRuntime::initPython();
    m_localDict = PythonScriptRuntime::createLocalDict();

    return init && m_localDict != nullptr;
}


bool PythonScriptRuntime::initPython() {
    if (Py_IsInitialized() && ready) {
        // "Python interpreter is already initialized.";
        return true;
    }

    if (PyImport_AppendInittab(moduleName, PyInit_pylabplot) == -1) {
        // "Failed to add the module to the table of built-in modules.";
        return false;
    }

    Py_Initialize();

    const bool pythonInitialized = PyInit_pylabplot() != nullptr;
    const bool pyErrorOccurred = PyErr_Occurred() != nullptr;
    if (!pythonInitialized || pyErrorOccurred) {
        // "Failed to initialize the module.";
        return false;
    }

    ready = true;
    return true;
}

bool PythonScriptRuntime::reset() {
    if (!Py_IsInitialized() || !ready) {
        // "Python interpreter is not initialized.";
        return false;
    }

    Py_XDECREF(m_localDict);
    m_localDict = PythonScriptRuntime::createLocalDict();
    // PythonScriptRuntime::bindProject(m_project);

    return m_localDict != nullptr;
}

bool PythonScriptRuntime::cancel() {
    if (!Py_IsInitialized() || !ready) {
        // "Python interpreter is not initialized.";
        return false;
    }

    // not yet implemented
    return false;
}

bool PythonScriptRuntime::redirectOutput() {
    if (!Py_IsInitialized() || !ready) {
        // "Python interpreter is not initialized.";
        return false;
    }

    // redirect stdout
    PythonScriptRuntime::sysStdOut = PySys_GetObject("stdout");
    if (!PythonScriptRuntime::sysStdOut) {
        // "Failed to get stdout from sys module";
        return false;
    }
    if (!PythonScriptRuntime::setObjectInModule(QStringLiteral("stdout"), QStringLiteral("sys"), SBK_PYTHONLOGGER_IDX, m_loggerStdOut)) {
        // "Failed to redirect python stdout";
        return false;
    }

    // redirect stderr
    PythonScriptRuntime::sysStdErr = PySys_GetObject("stderr");
    if (!PythonScriptRuntime::sysStdErr) {
        // "Failed to get stderr from sys module";
        return false;
    }
    if (!PythonScriptRuntime::setObjectInModule(QStringLiteral("stderr"), QStringLiteral("sys"), SBK_PYTHONLOGGER_IDX, m_loggerStdErr)) {
        // "Failed to redirect python stderr";
        return false;
    }

    return true;
}

bool PythonScriptRuntime::unRedirectOutput() {
    if (!Py_IsInitialized() || !ready) {
        // "Python interpreter is not initialized.";
        return false;
    }

    // reset sys.stdout with original
    if (PySys_SetObject("stdout", PythonScriptRuntime::sysStdOut) < 0) {
        // "Failed to redirect python stdout";
        return false;
    }
    // reset sys.stderr with original
    if (PySys_SetObject("stderr", PythonScriptRuntime::sysStdErr) < 0) {
        // "Failed to redirect python stderr";
        return false;
    }

    return true;
}

bool PythonScriptRuntime::exec(const QString& code) {
    if (!Py_IsInitialized() || !ready) {
        // "Python interpreter is not initialized.";
        return false;
    }

    if (!reset()) {
        // "Python script local context not reset.";
        return false;
    }

    if (m_localDict == nullptr) {
        // "Python script local context not created.";
        return false;
    }

    if (!redirectOutput())
        return false;

    PyErr_Clear();

    PyObject* compiled = Py_CompileString(code.toLocal8Bit().constData(), m_name.toLocal8Bit().constData(), Py_file_input);
    if (!compiled) {
        if (PyErr_Occurred()) {
            m_errorLine = PythonScriptRuntime::getPyErrorLine();
            PyErr_Print();
            return true; // this is ok
        }
        return false;
    }

    PyErr_Clear();

    auto* res = PyEval_EvalCode(compiled, m_localDict, m_localDict);
    if (!res) {
        Py_DECREF(compiled);
        if (PyErr_Occurred()) {
            m_errorLine = PythonScriptRuntime::getPyErrorLine();
            PyErr_Print();
            return true; // this is ok
        }
        return false;
    }

    Py_DECREF(compiled);
    Py_DECREF(res);

    if (!unRedirectOutput())
        return false;

    return true;
}

// bool PythonScriptRuntime::bindProject(Project* project) {
//     if (!Py_IsInitialized()) {
//         // "Python interpreter is not initialized.";
//         return false;
//     }
//     if (ready) {
//         // "Already added project to Python interpreter.";
//         return true;
//     }
//     if (!setObjectInModule(QStringLiteral("project"), QStringLiteral("__main__"), SBK_PROJECT_IDX, project)) {
//         // "Failed to add Project wrapper to __main__ module";
//         return false;
//     }
//     ready = true;
//     return true;
// }

// bool PythonScriptRuntime::unBindProject() {
//     auto* mainDict = getModuleDict(QStringLiteral("__main__"));
//     if (!mainDict) {
//         // "Failed to remove project from __main__";
//         return false;
//     } else {
//         if (PyDict_DelItemString(mainDict, "project") < 0) {
//             Py_DECREF(mainDict);
//             // "Failed to remove project from __main__";
//             return false;
//         }
//     }
//     Py_DECREF(mainDict);
//     return true;
// }

bool PythonScriptRuntime::setObjectInModule(const QString& name, const QString& moduleName, int index, void* o) {
    if (!Py_IsInitialized() || !ready) {
        // "Python interpreter is not initialized.";
        return false;
    }

    auto* po = shibokenConvertToPyObject(index, o);
    if (!po) {
        return false;
    }

    return setPyObjectInModule(name, moduleName, po);
}

// returns an owned reference
PyObject* PythonScriptRuntime::shibokenConvertToPyObject(int index, void* o) {
    if (!Py_IsInitialized() || !ready) {
        // "Python interpreter is not initialized.";
        return nullptr;
    }

    PyTypeObject* typeObject = SbkpylabplotTypes[index];
    
    // returns a borrowed reference
    PyObject* po = Shiboken::Conversions::pointerToPython(reinterpret_cast<SbkObjectType*>(typeObject), o);
    if (!po) {
        return nullptr;
    }

    Py_INCREF(po);
    return po;
}

// returns an owned reference
PyObject* PythonScriptRuntime::getModuleDict(const QString& moduleName) {
    if (!Py_IsInitialized() || !ready) {
        // "Python interpreter is not initialized.";
        return nullptr;
    }

    // PyImport_AddModule returns a borrowed reference so we create own reference Py_INCREF(o); and Py_DECREF(o); when done
    PyObject *module = PyImport_AddModule(moduleName.toLocal8Bit().constData());
    if (!module) {
        // __FUNCTION__ << "Failed to locate module" << moduleName;
        return nullptr;
    }

    Py_INCREF(module);

    // returns borrowed reference so we create own reference Py_INCREF(o); and Py_DECREF(o); when done
    PyObject* moduleDict = PyModule_GetDict(module);
    if(!moduleDict) {
        Py_DECREF(module);
        // __FUNCTION__ << "Failed to get module __dict__" << moduleName;
        return nullptr;
    }

    Py_DECREF(module);
    Py_INCREF(moduleDict);

    return moduleDict;
}

// returns an owned reference
PyObject* PythonScriptRuntime::getPyObjectFromModule(const QString& name, const QString& moduleName) {
    if (!Py_IsInitialized() || !ready) {
        // "Python interpreter is not initialized.";
        return nullptr;
    }

    auto* moduleDict = getModuleDict(moduleName);
    if(!moduleDict) {
        return nullptr;
    }

    // returns a borrowed reference, call Py_INCCREF(module); and return owned reference
    PyObject* o = PyDict_GetItemString(moduleDict, name.toLocal8Bit().constData());
    if (!o) {
        Py_DECREF(moduleDict);
        // __FUNCTION__ << "Failed to get object from module" << moduleName;
        return nullptr;
    }

    Py_DECREF(moduleDict);
    Py_INCREF(o);

    return o;
}

// steals reference to o, calls DECREF for us on success and failure
bool PythonScriptRuntime::setPyObjectInModule(const QString& name, const QString& moduleName, PyObject* o) {
    if (!Py_IsInitialized() || !ready) {
        // "Python interpreter is not initialized.";
        return false;
    }

    // PyImport_AddModule returns a borrowed reference so we create own reference Py_INCREF(o); and Py_DECREF(o); when done
    PyObject *module = PyImport_AddModule(moduleName.toLocal8Bit().constData());
    if (!module) {
        // __FUNCTION__ << "Failed to locate module" << moduleName;
        return false;
    }

    Py_INCREF(module);

    // PyModule_AddObject steals our refernce on success, calls Py_DECREF(o);
    if (PyModule_AddObject(module, name.toLocal8Bit().constData(), o) < 0) {
        Py_DECREF(module);
        Py_DECREF(o);
        // __FUNCTION__ << "Failed to add object" << name << "to" << moduleName;
        return false;
    }

    Py_DECREF(module);

    return true;
}

// returns owned reference
PyObject* PythonScriptRuntime::createLocalDict() {
    if (!Py_IsInitialized() || !ready) {
        // "Python interpreter is not initialized.";
        return nullptr;
    }

    auto* mainDict = PythonScriptRuntime::getModuleDict(QStringLiteral("__main__"));
    if (!mainDict) {
        return nullptr;
    }

    auto* localDict = PyDict_New();
    if (!localDict) {
        Py_DECREF(mainDict);
        return nullptr;
    }

    // if (PyDict_SetItemString(localDict, "__builtins__", PyDict_GetItemString(mainDict, "__builtins__")) < 0 || PyDict_SetItemString(localDict, "project", PyDict_GetItemString(mainDict, "project")) < 0) {
    //     Py_DECREF(localDict);
    //     Py_DECREF(mainDict);
    //     return nullptr;
    // }

    if (PyDict_SetItemString(localDict, "__builtins__", PyDict_GetItemString(mainDict, "__builtins__")) < 0) {
        Py_DECREF(localDict);
        Py_DECREF(mainDict);
        return nullptr;
    }

    Py_DECREF(mainDict);

    return localDict;
}

int PythonScriptRuntime::getPyErrorLine() {
    if (!Py_IsInitialized() || !ready) {
        // "Python interpreter is not initialized.";
        return -1;
    }

    int errorLine = -1;

    PyObject *type, *value, *traceback;

    PyErr_Fetch(&type, &value, &traceback);
    PyErr_NormalizeException(&type, &value, &traceback);

    if (traceback) {
        PyObject* p = PyObject_GetAttrString(traceback, "tb_lineno");
        if (p) {
            long line = PyLong_AsLong(p);
            if (line >= INT_MIN && line <= INT_MAX) {
                errorLine = static_cast<int>(line);
            }
            Py_DECREF(p);
        }
    }

    if (errorLine == -1 && type && value && PyErr_GivenExceptionMatches(type, PyExc_SyntaxError) != -1) {
        PyObject* q = PyObject_GetAttrString(value, "lineno");
        if (q) {
            long line = PyLong_AsLong(q);
            if (line >= INT_MIN && line <= INT_MAX) {
                errorLine = static_cast<int>(line);
            }
            Py_DECREF(q);
        }
    }

    PyErr_Restore(type, value, traceback);

    return errorLine;
}

QIcon PythonScriptRuntime::icon() {
    return QIcon::fromTheme(QStringLiteral("quickopen")); // get python icon
}
