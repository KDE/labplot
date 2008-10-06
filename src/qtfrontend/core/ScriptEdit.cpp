/***************************************************************************
    File                 : ScriptEdit.cpp
    Project              : SciDAVis
    --------------------------------------------------------------------
    Copyright            : (C) 2006 by Ion Vasilief,
                           Tilman Benkert,
                           Knut Franke
    Email (use @ for *)  : ion_vasilief*yahoo.fr, thzs*gmx.net,
                           knut.franke*gmx.de
    Description          : Editor widget for scripting code

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
#include "core/ScriptEdit.h"
#include "core/AbstractScriptingEngine.h"
#include "core/AbstractScript.h"

#include <QAction>
#include <QMenu>
#include <QPrintDialog>
#include <QPrinter>
#include <QMessageBox>
#include <QFileDialog>
#include <QTextStream>
#include <QKeyEvent>
#include <QContextMenuEvent>
#include <QTextBlock>

ScriptEdit::ScriptEdit(AbstractScriptingEngine *engine, QWidget *parent, const char *name)
  : QTextEdit(parent), scripted(engine)
{
	setObjectName(name);

	m_script = m_scripting_engine->makeScript("", this, name);
	connect(m_script, SIGNAL(error(const QString&,const QString&,int)), this, SLOT(insertErrorMsg(const QString&)));
	connect(m_script, SIGNAL(print(const QString&)), this, SLOT(scriptPrint(const QString&)));

	setLineWrapMode(NoWrap);
	setAcceptRichText(false);
	setFontFamily("Monospace");

	printCursor = textCursor();

	actionExecute = new QAction(tr("E&xecute"), this);
	actionExecute->setShortcut( tr("Ctrl+J") );
	connect(actionExecute, SIGNAL(activated()), this, SLOT(execute()));

	actionExecuteAll = new QAction(tr("Execute &All"), this);
	actionExecuteAll->setShortcut( tr("Ctrl+Shift+J") );
	connect(actionExecuteAll, SIGNAL(activated()), this, SLOT(executeAll()));

	actionEval = new QAction(tr("&Evaluate Expression"), this);
	actionEval->setShortcut( tr("Ctrl+Return") );
	connect(actionEval, SIGNAL(activated()), this, SLOT(evaluate()));

	actionPrint = new QAction(tr("&Print"), this);
	connect(actionPrint, SIGNAL(activated()), this, SLOT(print()));

	actionImport = new QAction(tr("&Import"), this);
	connect(actionImport, SIGNAL(activated()), this, SLOT(importASCII()));

	actionExport = new QAction(tr("&Export"), this);
	connect(actionExport, SIGNAL(activated()), this, SLOT(exportASCII()));

	functionsMenu = new QMenu(this);
	Q_CHECK_PTR(functionsMenu);
	connect(functionsMenu, SIGNAL(triggered(QAction *)), this, SLOT(insertFunction(QAction *)));
}

void ScriptEdit::customEvent(QEvent *e)
{
	if (e->type() == SCRIPTING_CHANGE_EVENT)
	{
		scriptingChangeEvent((ScriptingChangeEvent*)e);
		delete m_script;
		m_script = m_scripting_engine->makeScript("", this, objectName());
		connect(m_script, SIGNAL(error(const QString&,const QString&,int)), this, SLOT(insertErrorMsg(const QString&)));
		connect(m_script, SIGNAL(print(const QString&)), this, SLOT(scriptPrint(const QString&)));
	}
}

void ScriptEdit::keyPressEvent(QKeyEvent *e)
{
	QTextEdit::keyPressEvent(e);
	if (e->key() == Qt::Key_Return)
		updateIndentation();
}

void ScriptEdit::contextMenuEvent(QContextMenuEvent *e)
{
	QMenu *menu = createStandardContextMenu();
	Q_CHECK_PTR(menu);

	menu->addAction(actionPrint);
	menu->addAction(actionImport);
	menu->addAction(actionExport);
	menu->addSeparator();

	menu->addAction(actionExecute);
	menu->addAction(actionExecuteAll);
	menu->addAction(actionEval);

	/* TODO
	if (parent()->inherits("Note"))
	{
		Note *sp = (Note*) parent();
		QAction *actionAutoexec = new QAction( tr("Auto&exec"), menu );
		actionAutoexec->setToggleAction(true);
		actionAutoexec->setOn(sp->autoexec());
		connect(actionAutoexec, SIGNAL(toggled(bool)), sp, SLOT(setAutoexec(bool)));
		menu->addAction(actionAutoexec);
	}
	*/

	functionsMenu->clear();
	functionsMenu->setTearOffEnabled(true);
	QStringList flist = m_scripting_engine->mathFunctions();
	QMenu *submenu=NULL;
	for (int i=0; i<flist.size(); i++)
	{
		QAction *newAction;
		QString menupart;
		// group by text before "_" (would make sense if we renamed several functions...)
		/*if (flist[i].contains("_") || (i<flist.size()-1 && flist[i+1].split("_")[0]==flist[i]))
			menupart = flist[i].split("_")[0];
		else
			menupart = "";*/
		// group by first letter, avoiding submenus with only one entry
		if ((i==0 || flist[i-1][0] != flist[i][0]) && (i==flist.size()-1 || flist[i+1][0] != flist[i][0]))
			menupart = "";
		else
			menupart = flist[i].left(1);
		if (!menupart.isEmpty()) {
			if (!submenu || menupart != submenu->title())
				submenu = functionsMenu->addMenu(menupart);
			newAction = submenu->addAction(flist[i]);
		} else
			newAction = functionsMenu->addAction(flist[i]);
		newAction->setData(i);
		newAction->setWhatsThis(m_scripting_engine->mathFunctionDoc(flist[i]));
	}
	functionsMenu->setTitle(tr("&Functions"));
	menu->addMenu(functionsMenu);

	menu->exec(e->globalPos());
	delete menu;
}

void ScriptEdit::insertErrorMsg(const QString &message)
{
	QString err = message;
	err.prepend("\n").replace("\n","\n#> ");
	int start = printCursor.position();
	printCursor.insertText(err);
	printCursor.setPosition(start, QTextCursor::KeepAnchor);
	setTextCursor(printCursor);
}

void ScriptEdit::scriptPrint(const QString &text)
{
	if(lineNumber(printCursor.position()) == lineNumber(textCursor().selectionEnd()))
		printCursor.insertText("\n");
	printCursor.insertText(text);
}

void ScriptEdit::insertFunction(const QString &fname)
{
	QTextCursor cursor = textCursor();
	QString markedText = cursor.selectedText();
	cursor.insertText(fname+"("+markedText+")");
	if(markedText.isEmpty()){
		// if no text is selected, place cursor inside the ()
		// instead of after it
		cursor.movePosition(QTextCursor::PreviousCharacter,QTextCursor::MoveAnchor,1);
		// the next line makes the selection visible to the user
		// (the line above only changes the selection in the
		// underlying QTextDocument)
		setTextCursor(cursor);
	}
}

void ScriptEdit::insertFunction(QAction *action)
{
	insertFunction(m_scripting_engine->mathFunctions()[action->data().toInt()]);
}

int ScriptEdit::lineNumber(int pos) const
{
	int n=1;
	for(QTextBlock i=document()->begin(); !i.contains(pos) && i!=document()->end(); i=i.next())
		n++;
	return n;
}

void ScriptEdit::execute()
{
	QString fname = "<%1:%2>";
	fname = fname.arg(objectName());
	QTextCursor codeCursor = textCursor();
	if (codeCursor.selectedText().isEmpty()){
		codeCursor.movePosition(QTextCursor::StartOfLine, QTextCursor::MoveAnchor);
		codeCursor.movePosition(QTextCursor::EndOfLine, QTextCursor::KeepAnchor);
	}
	fname = fname.arg(lineNumber(codeCursor.selectionStart()));

	m_script->setName(fname);
	m_script->setCode(codeCursor.selectedText().replace(QChar::ParagraphSeparator,"\n"));
	printCursor.setPosition(codeCursor.selectionEnd(), QTextCursor::MoveAnchor);
	printCursor.movePosition(QTextCursor::EndOfLine, QTextCursor::MoveAnchor);
	m_script->exec();
}

void ScriptEdit::executeAll()
{
	QString fname = "<%1>";
	fname = fname.arg(objectName());
	m_script->setName(fname);
	m_script->setCode(toPlainText());
	printCursor.movePosition(QTextCursor::End, QTextCursor::MoveAnchor);
	m_script->exec();
}

void ScriptEdit::evaluate()
{
	QString fname = "<%1:%2>";
	fname = fname.arg(objectName());
	QTextCursor codeCursor = textCursor();
	if (codeCursor.selectedText().isEmpty()){
		codeCursor.movePosition(QTextCursor::StartOfLine, QTextCursor::MoveAnchor);
		codeCursor.movePosition(QTextCursor::EndOfLine, QTextCursor::KeepAnchor);
	}
	fname = fname.arg(lineNumber(codeCursor.selectionStart()));

	m_script->setName(fname);
	m_script->setCode(codeCursor.selectedText().replace(QChar::ParagraphSeparator,"\n"));
	printCursor.setPosition(codeCursor.selectionEnd(), QTextCursor::MoveAnchor);
	printCursor.movePosition(QTextCursor::EndOfLine, QTextCursor::MoveAnchor);
	QVariant res = m_script->eval();
	if (res.isValid())
		if (!res.isNull() && res.canConvert(QVariant::String)){
			QString strVal = res.toString();
			strVal.replace("\n", "\n#> ");
			if (!strVal.isEmpty())
				printCursor.insertText("\n#> "+strVal+"\n");
			else
				printCursor.insertText("\n");
		}

	setTextCursor(printCursor);
}

void ScriptEdit::exportPDF(const QString& fileName)
{
	QTextDocument *doc = document();
	QPrinter printer;
	printer.setColorMode(QPrinter::GrayScale);
	printer.setCreator("SciDAVis");
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setOutputFileName(fileName);
	doc->print(&printer);
}

void ScriptEdit::print()
{
	QTextDocument *doc = document();
	QPrinter printer;
	printer.setColorMode(QPrinter::GrayScale);
	QPrintDialog printDialog(&printer);
	// TODO: Write a dialog to use more features of Qt4's QPrinter class
	if (printDialog.exec() == QDialog::Accepted)
		doc->print(&printer);
}

QString ScriptEdit::importASCII(const QString &filename)
{
	QStringList filters;
	filters << tr("Text") + " (*.txt *.TXT)";
	filters << m_scripting_engine->nameAndPatterns();
	filters << tr("All Files") + " (*)";

	QString f;
	if (filename.isEmpty())
		f = QFileDialog::getOpenFileName(this, tr("Import Text From File"), QString(), filters.join(";;"));
	else
		f = filename;
	if (f.isEmpty()) return QString::null;
	QFile file(f);
	if (!file.open(QIODevice::ReadOnly)){
		QMessageBox::critical(this, tr("Error Opening File"), tr("Could not open file \"%1\" for reading.").arg(f));
		return QString::null;
	}
	QTextStream s(&file);
	while (!s.atEnd())
		insertPlainText(s.readLine()+"\n");
	file.close();
	return f;
}

QString ScriptEdit::exportASCII(const QString &filename)
{
	QStringList filters;
	filters << tr("Text") + " (*.txt *.TXT)";
	filters << m_scripting_engine->nameAndPatterns();
	filters << tr("All Files") + " (*)";

	QString selectedFilter;
	QString fn;
	if (filename.isEmpty()) {
		fn = QFileDialog::getSaveFileName(this, tr("Save Text to File"), QString(), filters.join(";;"), &selectedFilter);
	} else
		fn = filename;
		
	if ( !fn.isEmpty() ){
		QFileInfo fi(fn);
		QString baseName = fi.fileName();
		if (!baseName.contains(".")){
			if (selectedFilter.contains(".txt"))
				fn.append(".txt");
			else if (selectedFilter.contains(".py"))
				fn.append(".py");
		}

		QFile f(fn);
		if (!f.open(QIODevice::WriteOnly)){
			QMessageBox::critical(0, tr("File Save Error"),
						tr("Could not write to file: <br><h4> %1 </h4><p>Please verify that you have the right to write to this location!").arg(fn));
			return QString::null;
		}
			
		QTextStream t( &f );
		t << toPlainText();
		f.close();
	}
	return fn;
}

void ScriptEdit::setContext(QObject *context)
{
	m_script->setContext(context);
}

void ScriptEdit::updateIndentation()
{
	QTextCursor cursor = textCursor();
	QTextBlock para = cursor.block();
	QString prev = para.previous().text();
	int i;
	for (i=0; prev[i].isSpace(); i++) {}
	QString indent = prev.mid(0, i);
	cursor.movePosition(QTextCursor::StartOfLine, QTextCursor::MoveAnchor);
	cursor.insertText(indent);
}
