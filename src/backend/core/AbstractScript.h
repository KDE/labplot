/***************************************************************************
    File                 : AbstractScript.h
    Project              : SciDAVis
    Description          : A chunk of scripting code.
    --------------------------------------------------------------------
    Copyright            : (C) 2006 by Ion Vasilief (ion_vasilief@yahoo.fr)
    Copyright            : (C) 2006 Tilman Benkert (thzs@gmx.net)
    Copyright            : (C) 2006-2008 Knut Franke (knut.franke@gmx.de)

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

#include <QString>
#include <QObject>
#include <QVariant>

class ApplicationWindow;
class AbstractScriptingEngine;

class AbstractScript : public QObject
{
  Q_OBJECT

  public:
    AbstractScript(AbstractScriptingEngine *env, QString code, QObject *context=nullptr, QString name="<input>");
    ~AbstractScript() override;

    const QString code() const;
    const QObject* context() const;
    const QString name() const;
    bool emitErrors() const;
    virtual void addCode(const QString &code);
    virtual void setCode(const QString &code);
    virtual void setContext(QObject *context);
    void setName(const QString &name);
    void setEmitErrors(bool value);

  public slots:
    virtual bool compile(bool for_eval=true);
    virtual QVariant eval() = 0;
    virtual bool exec() = 0;

    // local variables
    virtual bool setQObject(const QObject*, const char*) { return false; }
    virtual bool setInt(int, const char*) { return false; }
    virtual bool setDouble(double, const char*) { return false; }

  signals:
    void codeChanged();
    void error(const QString &message, const QString &scriptName, int lineNumber);
    void print(const QString &output);

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

