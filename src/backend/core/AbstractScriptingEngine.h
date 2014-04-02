/***************************************************************************
    File                 : AbstractScriptingEngine.h
    Project              : SciDAVis
    --------------------------------------------------------------------
    Copyright            : (C) 2006 by Ion Vasilief, 
                           Tilman Benkert,
                           Knut Franke
    Copyright            : (C) 2008 by Knut Franke
    Email (use @ for *)  : ion_vasilief*yahoo.fr, thzs*gmx.net,
                           knut.franke*gmx.de
    Description          : Scripting abstraction layer
                           
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
#ifndef ABSTRACT_SCRIPTING_ENGINE_H
#define ABSTRACT_SCRIPTING_ENGINE_H

#include <QObject>
#include <QEvent>


#define SCRIPTING_CHANGE_EVENT QEvent::User

class AbstractScript;
class QString;

class AbstractScriptingEngine : public QObject
{
  Q_OBJECT

  public:
    explicit AbstractScriptingEngine(const char *lang_name);
    virtual void initialize() = 0;
    bool initialized() const;
    virtual bool isRunning() const;
    
    virtual AbstractScript *makeScript(const QString&, QObject*, const QString&) = 0;
      
    virtual QString stackTraceString();

    virtual const QStringList mathFunctions() const;
    virtual const QString mathFunctionDoc(const QString&) const;
    virtual const QStringList fileExtensions() const;
    const QString nameAndPatterns() const;

//    virtual QSyntaxHighlighter syntaxHighlighter(QTextEdit *textEdit) const;

  public slots:
    // global variables
    virtual bool setQObject(QObject*, const char*) { return false; }
    virtual bool setInt(int, const char*) { return false; }
    virtual bool setDouble(double, const char*) { return false; }

    virtual void clear();
    virtual void stopExecution();
    virtual void startExecution();

    void incref();
    void decref();

  signals:
    void error(const QString &message, const QString &scriptName, int lineNumber);
    void print(const QString &output);
    
  protected:
    bool m_initialized;

  private:
    int m_refcount;
};

Q_DECLARE_INTERFACE(AbstractScriptingEngine, "net.sf.scidavis.scriptingengine/0.1")

/******************************************************************************\
 *Helper classes for managing instances of AbstractScriptingEngine subclasses.*
\******************************************************************************/

class ScriptingChangeEvent : public QEvent
{
  public:
    ScriptingChangeEvent(AbstractScriptingEngine *engine) : QEvent(SCRIPTING_CHANGE_EVENT), m_scripting_engine(engine) {}
    AbstractScriptingEngine *scriptingEngine() const { return m_scripting_engine; }
    Type type() const { return SCRIPTING_CHANGE_EVENT; }
  private:
    AbstractScriptingEngine *m_scripting_engine;
};

class scripted
{
  public:
   explicit scripted(AbstractScriptingEngine* engine);
   ~scripted();
   void scriptingChangeEvent(ScriptingChangeEvent*);
  protected:
    AbstractScriptingEngine *m_scripting_engine;
};

#endif // ifndef ABSTRACT_SCRIPTING_ENGINE_H
