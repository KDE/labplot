#include "ScriptRuntime.h"

#include "Script.h"

ScriptRuntime::ScriptRuntime(const QString& lang, Script* script) 
    : QObject()
    , m_name(script->name())
    , lang(lang) {

}

int ScriptRuntime::errorLine() const {
    return m_errorLine;
}
