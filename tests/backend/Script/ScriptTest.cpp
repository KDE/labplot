/*
	File                 : ScriptTest.cpp
	Project              : LabPlot
	Description          : Tests for Script (Python debugging)
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2026 Martin Marmsoler <martin.marmsoler@gmail.com>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "ScriptTest.h"
#include "backend/core/Project.h"
#include "backend/script/Script.h"
#include "backend/script/ScriptRuntime.h"
#include "frontend/script/FloatingToolBar.h"
#include "frontend/script/ScriptEditor.h"

#include <KTextEditor/Document>
#include <KTextEditor/View>

#include <QAction>
#include <QTest>
#include <QTextBlock>
#include <QTextDocument>
#include <QTimer>
#include <QToolButton>
#include <QVBoxLayout>

namespace {
using Command = ScriptRuntime::DebugCommand;

const QString testScript = QStringLiteral(
	"def f(x):\n" // 1
	"    y = x * 2\n" // 2
	"    return y + 1\n" // 3
	"\n" // 4
	"a = 1\n" // 5
	"b = f(a)\n" // 6
	"c = b + 1\n" // 7
	"d = c + 1\n"); // 8

struct DebugResult {
	QList<int> pausedLines; // 1-based
	QList<QStringList> variables; // variable names at each pause
};

/*!
 * debugs the script with the given breakpoints (1-based), at each pause the next command of \c commands is sent
 * (Continue if no commands are left). \c onPause is called at each pause before the command is sent.
 */
DebugResult debug(Script* script,
				  const QList<int>& breakpoints,
				  QList<Command> commands,
				  const std::function<void(int)>& onPause = nullptr) {
	for (int line : breakpoints)
		script->toggleBreakpoint(line - 1);

	DebugResult result;
	auto* runtime = script->scriptRuntime();
	auto conn = QObject::connect(runtime, &ScriptRuntime::debugPaused, [&](int line) {
		result.pausedLines << line + 1;

		QStringList names;
		const auto* model = script->variableModel();
		for (int row = 0; row < model->rowCount(); ++row)
			names << model->data(model->index(row, 0)).toString();
		result.variables << names;

		const auto command = commands.isEmpty() ? Command::Continue : commands.takeFirst();
		// act once the nested event loop of the paused runtime is running, like a user interacting with the GUI
		QTimer::singleShot(0, runtime, [runtime, command, line, &onPause]() {
			if (onPause)
				onPause(line + 1);
			runtime->sendDebugCommand(command);
		});
	});

	script->debugScript();
	QObject::disconnect(conn);
	return result;
}

Script* createScript(Project& project, const QString& code = testScript) {
	auto* script = new Script(QStringLiteral("script"), QStringLiteral("Python"));
	project.addChild(script);
	script->kTextEditorDocument()->setText(code);
	return script;
}
}

void ScriptTest::debugContinueBreakpoints() {
	Project project;
	auto* script = createScript(project);
	QVERIFY(script->supportsDebugging());

	const auto result = debug(script, {2, 7}, {});
	QCOMPARE(result.pausedLines, QList<int>({2, 7}));
	QVERIFY(!script->isDebugging());
	QCOMPARE(script->scriptErrorLine(), -1);
}

void ScriptTest::debugStepOver() {
	Project project;
	auto* script = createScript(project);

	// step over the function call in line 6, the breakpoint in f isn't set
	const auto result = debug(script, {5}, {Command::StepOver, Command::StepOver, Command::StepOver, Command::Continue});
	QCOMPARE(result.pausedLines, QList<int>({5, 6, 7, 8}));
}

void ScriptTest::debugStepIntoAndOut() {
	Project project;
	auto* script = createScript(project);

	const auto result = debug(script, {6}, {Command::StepInto, Command::StepOut, Command::Continue});
	QCOMPARE(result.pausedLines, QList<int>({6, 2, 7}));
}

void ScriptTest::debugStop() {
	Project project;
	auto* script = createScript(project);

	debug(script, {7}, {Command::Stop});
	QVERIFY(!script->isDebugging());

	// line 7 and 8 were not executed
	QStringList names;
	const auto* model = script->variableModel();
	for (int row = 0; row < model->rowCount(); ++row)
		names << model->data(model->index(row, 0)).toString();
	QVERIFY(names.contains(QStringLiteral("b")));
	QVERIFY(!names.contains(QStringLiteral("c")));
	QVERIFY(!names.contains(QStringLiteral("d")));
}

void ScriptTest::debugVariables() {
	Project project;
	auto* script = createScript(project);

	bool executionMarkSet = false;
	const auto result = debug(script, {3, 7}, {}, [&](int line) {
		QVERIFY(script->isDebugPaused());
		QCOMPARE(script->executionLine(), line - 1);
		executionMarkSet = script->kTextEditorDocument()->mark(line - 1) & KTextEditor::Document::Execution;
	});
	QVERIFY(executionMarkSet);
	QCOMPARE(result.pausedLines, QList<int>({3, 7}));

	// inside of f() only the local variables are shown
	QCOMPARE(result.variables.at(0), QStringList({QStringLiteral("x"), QStringLiteral("y")}));
	// on module level, functions are filtered out
	QCOMPARE(result.variables.at(1), QStringList({QStringLiteral("a"), QStringLiteral("b")}));

	// the execution mark is removed after the run
	QVERIFY(!(script->kTextEditorDocument()->mark(6) & KTextEditor::Document::Execution));
	QCOMPARE(script->executionLine(), -1);
}

void ScriptTest::debugAddBreakpointWhilePaused() {
	Project project;
	auto* script = createScript(project);

	bool added = false;
	const auto result = debug(script, {5}, {}, [&](int) {
		if (!added) {
			added = true;
			script->toggleBreakpoint(7); // 0-based -> line 8
		}
	});
	QCOMPARE(result.pausedLines, QList<int>({5, 8}));
}

void ScriptTest::debugErrorLine() {
	Project project;
	auto* script = createScript(project,
								QStringLiteral("def f():\n"
											   "    return 1 / 0\n"
											   "\n"
											   "x = f()\n"));

	debug(script, {}, {});
	QVERIFY(!script->isDebugging());
	// the error is reported in the script, not in the debugger helper
	QCOMPARE(script->scriptErrorLine(), 3); // 0-based, the traceback starts at the call in line 4
	QVERIFY(script->kTextEditorDocument()->mark(3) & KTextEditor::Document::Error);
}

void ScriptTest::toggleBreakpoint() {
	Project project;
	auto* script = createScript(project);

	script->toggleBreakpoint(4);
	script->toggleBreakpoint(1);
	QCOMPARE(script->breakpoints(), QList<int>({2, 5}));
	script->toggleBreakpoint(4);
	QCOMPARE(script->breakpoints(), QList<int>({2}));
	script->toggleBreakpoint(100); // out of range, ignored
	QCOMPARE(script->breakpoints(), QList<int>({2}));
}

void ScriptTest::breakpointsNotSaved() {
	QString savePath;
	{
		Project project;
		auto* script = createScript(project);
		project.setChanged(false);

		// toggling breakpoints doesn't modify the project
		script->toggleBreakpoint(1);
		script->toggleBreakpoint(6);
		QVERIFY(!project.isChanged());

		SAVE_PROJECT("TestScriptBreakpoints");
	}

	Project project;
	QVERIFY(project.load(savePath));
	const auto& scripts = project.children<Script>();
	QCOMPARE(scripts.size(), 1);
	QVERIFY(scripts.first()->breakpoints().isEmpty());
	QCOMPARE(scripts.first()->kTextEditorDocument()->text(), testScript);
}

namespace {
QModelIndex findVariable(QAbstractItemModel* model, const QString& name, const QModelIndex& parent = QModelIndex()) {
	if (model->canFetchMore(parent))
		model->fetchMore(parent);
	for (int row = 0; row < model->rowCount(parent); ++row) {
		const auto& index = model->index(row, 0, parent);
		if (index.data().toString() == name)
			return index;
	}
	return {};
}

QString value(const QModelIndex& index) {
	return index.siblingAtColumn(1).data().toString();
}
}

void ScriptTest::variableViewerChildren() {
	Project project;
	auto* script = createScript(project,
								QStringLiteral("class P:\n"
											   "    pass\n"
											   "p = P()\n"
											   "p.x = 3\n"
											   "l = [1, [2, 3]]\n"
											   "d = {'k': 5}\n"
											   "s = 'abc'\n"
											   "big = list(range(1500))\n"));
	debug(script, {}, {});

	auto* model = script->variableModel();

	// list with nested list
	const auto& l = findVariable(model, QStringLiteral("l"));
	QVERIFY(l.isValid());
	QVERIFY(model->hasChildren(l));
	QCOMPARE(value(findVariable(model, QStringLiteral("[0]"), l)), QStringLiteral("1"));
	const auto& nested = findVariable(model, QStringLiteral("[1]"), l);
	QVERIFY(model->hasChildren(nested));
	QCOMPARE(value(findVariable(model, QStringLiteral("[1]"), nested)), QStringLiteral("3"));

	// dict
	const auto& d = findVariable(model, QStringLiteral("d"));
	QCOMPARE(value(findVariable(model, QStringLiteral("['k']"), d)), QStringLiteral("5"));

	// object attributes
	const auto& p = findVariable(model, QStringLiteral("p"));
	QCOMPARE(value(findVariable(model, QStringLiteral("x"), p)), QStringLiteral("3"));

	// scalars can't be expanded
	const auto& str = findVariable(model, QStringLiteral("s"));
	QCOMPARE(value(str), QStringLiteral("'abc'"));
	QVERIFY(!model->hasChildren(str));

	// large containers are limited, the value is truncated
	const auto& big = findVariable(model, QStringLiteral("big"));
	if (model->canFetchMore(big))
		model->fetchMore(big);
	QCOMPARE(model->rowCount(big), 1001);
	QCOMPARE(model->index(1000, 0, big).data().toString(), QStringLiteral("…"));
	QVERIFY(value(big).endsWith(QStringLiteral("…")));
}

void ScriptTest::variableViewerWhilePaused() {
	Project project;
	auto* script = createScript(project,
								QStringLiteral("def f(items):\n"
											   "    total = sum(items)\n"
											   "    return total\n"
											   "f([4, 5])\n"));

	QString element;
	debug(script, {3}, {}, [&](int) {
		auto* model = script->variableModel();
		const auto& items = findVariable(model, QStringLiteral("items"));
		element = value(findVariable(model, QStringLiteral("[1]"), items));
	});
	QCOMPARE(element, QStringLiteral("5"));
}

/*!
 * the main window has actions with a window-wide shortcut using the same keys as the debugger
 * (e.g. F10 = "Open Main Menu" of the hamburger menu), the debug actions must win while the editor has the focus.
 */
void ScriptTest::debugShortcutsWithConflictingWindowAction() {
	Project project;
	auto* script = createScript(project);
	script->toggleBreakpoint(4); // line 5

	QWidget window;
	auto* layout = new QVBoxLayout(&window);
	auto* editor = script->view();
	layout->addWidget(editor);

	bool windowActionTriggered = false;
	auto* windowAction = new QAction(QStringLiteral("Open Main Menu"), &window);
	windowAction->setShortcuts({QKeySequence(Qt::Key_F10), QKeySequence(Qt::Key_F5)});
	windowAction->setShortcutContext(Qt::WindowShortcut);
	window.addAction(windowAction);
	connect(windowAction, &QAction::triggered, [&]() {
		windowActionTriggered = true;
	});

	window.show();
	QApplication::setActiveWindow(&window);
	QVERIFY(QTest::qWaitForWindowActive(&window));

	QList<int> pausedLines;
	auto* runtime = script->scriptRuntime();
	connect(runtime, &ScriptRuntime::debugPaused, [&](int line) {
		pausedLines << line + 1;
		QTimer::singleShot(0, [&]() {
			// the cursor is placed in the editor at the paused line, press the keys there
			auto* focus = QApplication::focusWidget();
			QVERIFY(focus);
			QVERIFY(editor->isAncestorOf(focus));
			QTest::keyClick(focus, pausedLines.size() == 1 ? Qt::Key_F10 : Qt::Key_F5); // step over, then continue
		});
	});

	// start debugging with F5 in the editor
	QTimer::singleShot(0, [&]() {
		auto* focus = QApplication::focusWidget();
		QVERIFY(focus);
		QTest::keyClick(focus, Qt::Key_F5);
	});

	// the text editor has the focus
	auto* textView = editor->findChild<KTextEditor::View*>();
	QVERIFY(textView);
	textView->setFocus();
	QTRY_VERIFY_WITH_TIMEOUT(!pausedLines.isEmpty(), 5000);
	QTRY_VERIFY_WITH_TIMEOUT(!script->isDebugging(), 5000);

	QCOMPARE(pausedLines, QList<int>({5, 6}));
	QVERIFY(!windowActionTriggered);
}

/*!
 * toggle a breakpoint with the button in the toolbar of the editor at the line of the cursor and debug
 */
void ScriptTest::toggleBreakpointToolbar() {
	Project project;
	auto* script = createScript(project);

	auto* editor = script->view();
	editor->resize(800, 600);
	editor->show();
	QVERIFY(QTest::qWaitForWindowExposed(editor));
	auto* textView = editor->findChild<KTextEditor::View*>();
	QVERIFY(textView);

	QToolButton* button = nullptr;
	for (auto* b : editor->findChildren<QToolButton*>())
		if (b->defaultAction() && b->defaultAction()->text() == QStringLiteral("Toggle Breakpoint"))
			button = b;
	QVERIFY(button);

	textView->setCursorPosition(KTextEditor::Cursor(6, 0)); // line 7 "c = b + 1"
	QTest::mouseClick(button, Qt::LeftButton);
	QCOMPARE(script->breakpoints(), QList<int>({7}));

	script->toggleBreakpoint(2); // line 3
	const auto result = debug(script, {}, {});
	QCOMPARE(result.pausedLines, QList<int>({3, 7}));
}

/*!
 * multi-line plain text set via the python bindings keeps the line breaks,
 * HTML (e.g. created with QTextEdit.toHtml()) is still accepted as before
 */
void ScriptTest::textLabelMultilineText() {
	Project project;
	auto* script = createScript(project,
								QStringLiteral("from pylabplot import *\n"
											   "from PySide6.QtWidgets import QTextEdit\n"
											   "label = TextLabel('label')\n"
											   "label.setText('first line\\nsecond line')\n"
											   "plain = label.text().text\n"
											   "te = QTextEdit()\n"
											   "te.setHtml('<p>html one</p><p>html two</p>')\n"
											   "label.setText(te.toHtml())\n"
											   "html = label.text().text\n"));
	debug(script, {}, {});
	QCOMPARE(script->scriptErrorLine(), -1);

	// read the texts set in the script from the variable viewer
	QString plain, html;
	const auto* model = script->variableModel();
	for (int row = 0; row < model->rowCount(); ++row) {
		const auto& name = model->index(row, 0).data().toString();
		// the value is the python repr() of the string
		const auto& value = model->index(row, 1).data().toString();
		if (name == QStringLiteral("plain"))
			plain = value;
		else if (name == QStringLiteral("html"))
			html = value;
	}

	auto paragraphs = [](QString repr) {
		repr = repr.mid(1, repr.size() - 2).replace(QStringLiteral("\\'"), QStringLiteral("'")).replace(QStringLiteral("\\n"), QStringLiteral("\n"));
		QTextDocument doc;
		doc.setHtml(repr);
		QStringList lines;
		for (auto block = doc.begin(); block != doc.end(); block = block.next())
			lines << block.text();
		return lines;
	};

	QCOMPARE(paragraphs(plain), QStringList({QStringLiteral("first line"), QStringLiteral("second line")}));
	QCOMPARE(paragraphs(html), QStringList({QStringLiteral("html one"), QStringLiteral("html two")}));
}

/*!
 * the toolbar floats on top of the editor and can be moved horizontally by dragging it
 */
void ScriptTest::floatingToolBar() {
	Project project;
	auto* script = createScript(project);
	auto* editor = script->view();
	editor->resize(800, 600);
	editor->show();
	QVERIFY(QTest::qWaitForWindowExposed(editor));

	auto* toolBar = editor->findChild<FloatingToolBar*>();
	QVERIFY(toolBar);
	auto* parent = toolBar->parentWidget();
	auto* textView = editor->findChild<KTextEditor::View*>();
	QVERIFY(parent->isAncestorOf(textView)); // on top of the text editor
	QVERIFY(toolBar->isVisible());
	QCOMPARE(toolBar->y(), 0);

	toolBar->setPosition(0.5);
	QApplication::processEvents();
	const int freeSpace = parent->width() - toolBar->width();
	QVERIFY(freeSpace > 0);
	QCOMPARE(toolBar->x(), qRound(0.5 * freeSpace)); // centered


	// drag it at the grip (left border) to the left, it stops at the border of the editor
	auto send = [](QWidget* w, QEvent::Type type, QPoint pos, Qt::MouseButtons buttons) {
		QMouseEvent event(type, pos, w->mapToGlobal(pos), type == QEvent::MouseMove ? Qt::NoButton : Qt::LeftButton, buttons, Qt::NoModifier);
		QApplication::sendEvent(w, &event);
	};
	const QPoint grip(4, toolBar->height() / 2);
	auto* gripWidget = toolBar->childAt(grip);
	QVERIFY(gripWidget);
	QVERIFY(!qobject_cast<QToolButton*>(gripWidget));
	const QPoint gripPos = gripWidget->mapFrom(toolBar, grip);

	send(gripWidget, QEvent::MouseButtonPress, gripPos, Qt::LeftButton);
	send(gripWidget, QEvent::MouseMove, gripPos - QPoint(100, 0), Qt::LeftButton);
	QCOMPARE(toolBar->x(), qRound(0.5 * freeSpace) - 100);
	QCOMPARE(toolBar->y(), 0); // only horizontally
	send(gripWidget, QEvent::MouseMove, gripPos - QPoint(5000, 30), Qt::LeftButton);
	QCOMPARE(toolBar->x(), 0);
	QCOMPARE(toolBar->y(), 0);
	send(gripWidget, QEvent::MouseButtonRelease, gripPos - QPoint(5000, 0), Qt::NoButton);
	QCOMPARE(toolBar->position(), 0.);

	// to the right border
	send(gripWidget, QEvent::MouseButtonPress, gripPos, Qt::LeftButton);
	send(gripWidget, QEvent::MouseMove, gripPos + QPoint(5000, 0), Qt::LeftButton);
	send(gripWidget, QEvent::MouseButtonRelease, gripPos + QPoint(5000, 0), Qt::NoButton);
	QCOMPARE(toolBar->x(), freeSpace);
	QCOMPARE(toolBar->position(), 1.);

	// the relative position is kept when the editor is resized
	editor->resize(500, 600);
	QApplication::processEvents();
	QCOMPARE(toolBar->x(), parent->width() - toolBar->width());

	// the buttons still work
	QToolButton* toggle = nullptr;
	for (auto* b : toolBar->findChildren<QToolButton*>())
		if (b->defaultAction() && b->defaultAction()->text() == QStringLiteral("Toggle Breakpoint"))
			toggle = b;
	QVERIFY(toggle);
	textView->setCursorPosition(KTextEditor::Cursor(2, 0));
	QTest::mouseClick(toggle, Qt::LeftButton);
	QCOMPARE(script->breakpoints(), QList<int>({3}));
}

QTEST_MAIN(ScriptTest)
