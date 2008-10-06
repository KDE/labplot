/***************************************************************************
    File                 : AbstractScript.h
    Project              : SciDAVis
    --------------------------------------------------------------------
    Copyright            : (C) 2006 by Ion Vasilief, 
                           Tilman Benkert,
                           Knut Franke
    Copyright            : (C) 2008 by Knut Franke
    Email (use @ for *)  : ion_vasilief*yahoo.fr, thzs*gmx.net,
                           knut.franke*gmx.de
    Description          : A chunk of scripting code.
                           
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
#ifndef ABSTRACT_SCRIPT_H
#define ABSTRACT_SCRIPT_H

#include <QVariant>
#include <QString>
#include <QObject>

class ApplicationWindow;
class AbstractScriptingEngine;

//! A chunk of scripting code.
  /**
   * AbstractScript objects represent a chunk of code, possibly together with local
   * variables. The code may be changed and executed multiple times during the
   * lifetime of an object.
   */
class AbstractScript : public QObject
{
  Q_OBJECT

  public:
    AbstractScript(AbstractScriptingEngine *env, const QString &code, QObject *context=0, const QString &name="<input>");
    ~AbstractScript();

    //! Return the code that will be executed/evaluated when calling exec() or eval()
    const QString code() const { return m_code; }
    //! Return the context in which the code is to be executed.
    const QObject* context() const { return m_context; }
    //! Like QObject::name, but with unicode support.
    const QString name() const { return m_name; }
    //! Return whether errors / exceptions are to be emitted or silently ignored
    const bool emitErrors() const { return m_emit_errors; }
    //! Append to the code that will be executed when calling exec() or eval()
    virtual void addCode(const QString &code) { m_code.append(code); m_compiled = notCompiled; emit codeChanged(); }
    //! Set the code that will be executed when calling exec() or eval()
    virtual void setCode(const QString &code) { m_code=code; m_compiled = notCompiled; emit codeChanged(); }
    //! Set the context in which the code is to be executed.
    virtual void setContext(QObject *context) { m_context = context; m_compiled = notCompiled; }
    //! Like QObject::setName, but with unicode support.
    void setName(const QString &name) { m_name = name; m_compiled = notCompiled; }
    //! Set whether errors / exceptions are to be emitted or silently ignored
    void setEmitErrors(bool value) { m_emit_errors = value; }

  public slots:
    //! Compile the content of #m_code.
	 /**
	  * \param for_eval whether the code is to be evaluated later on (as opposed to executed)
	  * \return True iff compilation was successful or the implementation doesn't support compilation.
	  */
    virtual bool compile(bool for_eval=true) { Q_UNUSED(for_eval); return true; }
    //! Evaluate #m_code, returning QVariant() on an error / exception.
    virtual QVariant eval() = 0;
    //! Execute #m_code, returning false on an error / exception.
    virtual bool exec() = 0;

    // local variables
    virtual bool setQObject(const QObject*, const char*) { return false; }
    virtual bool setInt(int, const char*) { return false; }
    virtual bool setDouble(double, const char*) { return false; }

  signals:
    //! This is emitted whenever the code to be executed by exec() and eval() is changed.
    void codeChanged();
    //! signal an error condition / exception
    void error(const QString & message, const QString & scriptName, int lineNumber);
    //! output generated by the code
    void print(const QString & output);
    
  protected:
    AbstractScriptingEngine *m_engine;
    QString m_code, m_name;
    QObject *m_context;
    enum compileStatus { notCompiled, isCompiled, compileErr } m_compiled;
    bool m_emit_errors;

    void emit_error(const QString & message, int line_number)
      { if(m_emit_errors) emit error(message, m_name, line_number); }
};

#endif

