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
#include <QtPlugin>
#include <QStringList>

#include "core/customevents.h"

class AbstractScript;
class QString;

//! An interpreter for evaluating scripting code.
  /**
   * AbstractScriptingEngine objects represent a running interpreter, possibly with global
   * variables, and are responsible for generating AbstractScript objects (which do
   * the actual evaluation of code).
	*
	* The class also keeps a static list of available interpreters and instantiates
	* them on demand.
   */
class AbstractScriptingEngine : public QObject
{
  Q_OBJECT

  public:
    //! Only initialize general information here.
	 /**
	  * All AbstractScriptingEngine subclasses are instantiated at startup (more precisely: when
	  * instantiating ScriptingEngineManager). Therefor, loading the actual interpreter is done
	  * in initialize().
	  */
    AbstractScriptingEngine(const char *lang_name);
    //! Initialize the scripting environment.
	 /**
	  * Don't forget to set m_initialized to true in implemenations after a successfull
	  * initialization.
	  */
    virtual void initialize() = 0;
    //! initialization of the interpreter may fail; or there could be other errors setting up the environment
    bool initialized() const { return m_initialized; }
    //! whether asynchronuous execution is enabled (if supported by the implementation)
    virtual bool isRunning() const { return false; }
    
    //! Instantiate the AbstractScript subclass matching the AbstractScriptingEngine subclass.
    virtual AbstractScript *makeScript(const QString&, QObject*, const QString&) = 0;
      
    //! If an exception / error occured, return a nicely formated stack backtrace.
    virtual QString stackTraceString() { return QString::null; }

    //! Return a list of supported mathematical functions.
	 /**
	  * These should be imported into the global namespace.
	  */
    virtual const QStringList mathFunctions() const { return QStringList(); }
    //! Return a documentation string for the given mathematical function.
    virtual const QString mathFunctionDoc(const QString&) const { return QString::null; }
    //! Return a list of file extensions commonly used for this language.
    virtual const QStringList fileExtensions() const { return QStringList(); };
    //! Construct a filter expression from fileExtensions(), suitable for QFileDialog.
    const QString nameAndPatterns() const;

//    virtual QSyntaxHighlighter syntaxHighlighter(QTextEdit *textEdit) const;

  public slots:
    // global variables
    virtual bool setQObject(QObject*, const char*) { return false; }
    virtual bool setInt(int, const char*) { return false; }
    virtual bool setDouble(double, const char*) { return false; }

    //! Clear the global environment.
	 /**
	  * What exactly happens depends on the implementation.
	  */
    virtual void clear() {}
    //! If the implementation supports asynchronuos execution, deactivate it.
    virtual void stopExecution() {}
    //! If the implementation supports asynchronuos execution, activate it.
    virtual void startExecution() {}

    //! Increase the reference count.
	 /**
	  * This should only be called by scripted and Script to avoid memory leaks.
	  */
    void incref();
    //! Decrease the reference count.
	 /**
	  * This should only be called by scripted and Script to avoid segfaults.
	  */
    void decref();

  signals:
    //! signal an error condition / exception
    void error(const QString & message, const QString & scriptName, int lineNumber);
    //! output that is not handled by a Script
    void print(const QString & output);
    
  protected:
    //! whether the interpreter has been successfully initialized
    bool m_initialized;

  private:
    //! the reference counter
    int m_refcount;
};

Q_DECLARE_INTERFACE(AbstractScriptingEngine, "net.sf.scidavis.scriptingengine/0.1")

/******************************************************************************\
 *Helper classes for managing instances of AbstractScriptingEngine subclasses.*
\******************************************************************************/

//! notify an object that it should update its scripting environment (see class scripted)
class ScriptingChangeEvent : public QEvent
{
  public:
    ScriptingChangeEvent(AbstractScriptingEngine *engine) : QEvent(SCRIPTING_CHANGE_EVENT), m_scripting_engine(engine) {}
    AbstractScriptingEngine *scriptingEngine() const { return m_scripting_engine; }
    Type type() const { return SCRIPTING_CHANGE_EVENT; }
  private:
    AbstractScriptingEngine *m_scripting_engine;
};

//! Interface for maintaining a reference to the current AbstractScriptingEngine
  /**
   * Every class that wants to use a AbstractScriptingEngine should subclass this one and
   * implement slot customEvent(QEvent*) such that it forwards any
   * AbstractScriptingChangeEvents to scripted::scriptingChangeEvent.
   */
class scripted
{
  public:
   scripted(AbstractScriptingEngine* engine);
   ~scripted();
   void scriptingChangeEvent(ScriptingChangeEvent*);
  protected:
    AbstractScriptingEngine *m_scripting_engine;
};

#endif // ifndef ABSTRACT_SCRIPTING_ENGINE_H
