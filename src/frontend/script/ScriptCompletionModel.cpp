/*
	File                 : ScriptCompletionModel.cpp
	Project              : LabPlot
	Description          : Code completion model for Python script editor
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2026 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "ScriptCompletionModel.h"
#include "ScriptEditor.h"
#include "backend/script/Script.h"
#include "backend/script/python/PythonScriptingHelper.h"
#include <backend/lib/macros.h>

#include <KTextEditor/View>
#include <KTextEditor/Document>

#include <QTimer>
#include <QRegularExpression>

ScriptCompletionModel::ScriptCompletionModel(ScriptEditor* parent)
	: KTextEditor::CodeCompletionModel(parent)
	, m_editor(parent) {
	m_debounceTimer = new QTimer(this);
	m_debounceTimer->setSingleShot(true);
	m_debounceTimer->setInterval(200);
	connect(m_debounceTimer, &QTimer::timeout, this, &ScriptCompletionModel::startCompletionRequest);

	initPythonBuiltins(); // initialize Python built-ins
	initPylabplotSymbols(); // initialize pylabplot symbols
}

ScriptCompletionModel::~ScriptCompletionModel() {
	m_debounceTimer->stop();
}

void ScriptCompletionModel::initPythonBuiltins() {
	// Common Python built-in functions
	m_pythonBuiltins = {
		QStringLiteral("print"),
		QStringLiteral("len"),
		QStringLiteral("range"),
		QStringLiteral("str"),
		QStringLiteral("int"),
		QStringLiteral("float"),
		QStringLiteral("list"),
		QStringLiteral("dict"),
		QStringLiteral("set"),
		QStringLiteral("tuple"),
		QStringLiteral("bool"),
		QStringLiteral("sum"),
		QStringLiteral("min"),
		QStringLiteral("max"),
		QStringLiteral("abs"),
		QStringLiteral("round"),
		QStringLiteral("sorted"),
		QStringLiteral("enumerate"),
		QStringLiteral("zip"),
		QStringLiteral("map"),
		QStringLiteral("filter"),
		QStringLiteral("open"),
		QStringLiteral("type"),
		QStringLiteral("isinstance"),
		QStringLiteral("hasattr"),
		QStringLiteral("getattr"),
		QStringLiteral("setattr"),
	};
}

bool ScriptCompletionModel::initPylabplotSymbols() {
	DEBUG(Q_FUNC_INFO)

	// Get pylabplot symbols via global helper (avoids Python.h dependency)
	QStringList symbolNames = pylabplotSymbolsHelper();

	if (symbolNames.isEmpty()) {
		WARN("No pylabplot symbols extracted - Python may not be initialized")
		return false;
	}

	DEBUG(Q_FUNC_INFO << ", Found " << symbolNames.size() << " symbols in pylabplot")

	// Create completion items for each symbol
	for (const QString& name : symbolNames) {
		CompletionItem item;
		item.name = name;

		// Heuristic type detection based on naming conventions
		// Classes typically start with uppercase
		if (!name.isEmpty() && name[0].isUpper()) {
			item.isClass = true;
		}
		// Function names often contain common patterns
		else if (name.contains(QLatin1String("get")) || name.contains(QLatin1String("set")) || name == QLatin1String("project")) {
			item.isFunction = true;
		}

		m_pylabplotSymbols.append(item);
	}

	DEBUG(Q_FUNC_INFO << ", Successfully extracted " << m_pylabplotSymbols.size() << " pylabplot symbols")
	return true;
}

void ScriptCompletionModel::updateUserVariables(const QString& scriptText) {
	// Extract variable names from script using simple regex
	// Matches: variable_name = ...
	static QRegularExpression assignPattern(QStringLiteral(R"(^(\w+)\s*=)"), QRegularExpression::MultilineOption);

	QSet<QString> variables;
	auto it = assignPattern.globalMatch(scriptText);

	while (it.hasNext()) {
		QRegularExpressionMatch match = it.next();
		QString varName = match.captured(1);

		// Skip if it starts with underscore or is a Python keyword
		if (!varName.isEmpty() && !varName.startsWith(QLatin1Char('_'))) {
			// Basic keyword check
			if (varName != QLatin1String("if") && varName != QLatin1String("for") && varName != QLatin1String("while") && varName != QLatin1String("def")
				&& varName != QLatin1String("class") && varName != QLatin1String("import") && varName != QLatin1String("from")
				&& varName != QLatin1String("return") && varName != QLatin1String("yield") && varName != QLatin1String("with")
				&& varName != QLatin1String("as")) {
				variables.insert(varName);
			}
		}
	}

	m_userVariables = variables.values();
	DEBUG(Q_FUNC_INFO << ", Found " << m_userVariables.size() << " user variables")
}

void ScriptCompletionModel::completionInvoked(KTextEditor::View* view, const KTextEditor::Range& range, InvocationType invocationType) {
	Q_UNUSED(invocationType)

	m_pendingView = view;
	m_pendingRange = range;
	m_debounceTimer->start();
}

void ScriptCompletionModel::startCompletionRequest() {
	if (!m_pendingView)
		return;

	auto currentPos = m_pendingView->cursorPosition();

	// Detect completion context (global vs member)
	QString objectName;
	CompletionContext context = detectContext(m_pendingView, currentPos, objectName);

	// Get current word being typed (the filter text)
	auto currentWordRange = m_pendingView->document()->wordRangeAt(currentPos);
	QString prefix = m_pendingView->document()->text(currentWordRange);

	// Get current script text
	QString scriptText = m_pendingView->document()->text();
	updateUserVariables(scriptText);

	// Collect completion candidates based on context
	beginResetModel();
	m_matches.clear();

	if (context == CompletionContext::Member) {
		// Member completion - get members of the object
		QString typeName = inferType(objectName, scriptText);
		if (!typeName.isEmpty()) {
			QList<CompletionItem> members = membersForType(typeName);
			// Filter by prefix
			for (const auto& member : members) {
				if (prefix.isEmpty() || member.name.startsWith(prefix, Qt::CaseInsensitive))
					m_matches.append(member);
			}
		}
	} else {
		// Global completion - collect all symbols
		QSet<QString> allSymbolsSet;

		// Add pylabplot symbols
		for (const auto& item : m_pylabplotSymbols)
			allSymbolsSet.insert(item.name);

		// Add Python built-ins
		for (const auto& builtin : m_pythonBuiltins)
			allSymbolsSet.insert(builtin);

		// Add user variables
		for (const auto& var : m_userVariables)
			allSymbolsSet.insert(var);

		// Filter by prefix
		for (const QString& symbol : allSymbolsSet) {
			if (prefix.isEmpty() || symbol.startsWith(prefix, Qt::CaseInsensitive)) {
				CompletionItem item;
				item.name = symbol;

				// Determine type
				for (const auto& pylabplotItem : m_pylabplotSymbols) {
					if (pylabplotItem.name == symbol) {
						item.isClass = pylabplotItem.isClass;
						item.isFunction = pylabplotItem.isFunction;
						item.isEnum = pylabplotItem.isEnum;
						break;
					}
				}

				// Check if it's a Python built-in
				if (m_pythonBuiltins.contains(symbol))
					item.isFunction = true;

				// Check if it's a user variable
				if (m_userVariables.contains(symbol))
					item.isVariable = true;

				m_matches.append(item);
			}
		}
	}

	// Sort alphabetically
	std::sort(m_matches.begin(), m_matches.end(), [](const auto& a, const auto& b) {
		return a.name.localeAwareCompare(b.name) < 0;
	});

	setRowCount(m_matches.size());
	endResetModel();
}

QVariant ScriptCompletionModel::data(const QModelIndex& index, int role) const {
	if (!index.isValid() || index.row() >= m_matches.count())
		return QVariant();

	const auto& item = m_matches.at(index.row());

	switch (role) {
	case Qt::DisplayRole:
		if (index.column() == Name) {
			// Show name with type indicator or signature
			QString display = item.name;

			// Add simple indicator based on type
			if (item.isFunction || item.isClass) {
				display += QStringLiteral("()");
			}

			return display;
		}
		break;

	case Qt::DecorationRole:
		if (index.column() == Icon) {
			// Return appropriate icon based on type
			if (item.isClass)
				return QIcon::fromTheme(QStringLiteral("code-class"));
			else if (item.isFunction)
				return QIcon::fromTheme(QStringLiteral("code-function"));
			else if (item.isEnum)
				return QIcon::fromTheme(QStringLiteral("flag"));
			else if (item.isVariable)
				return QIcon::fromTheme(QStringLiteral("code-variable"));
		}
		break;

	case ItemSelected:
		// Show docstring in the status bar or completion widget when item is selected
		if (!item.docstring.isEmpty())
			return item.docstring;
		break;
	}

	return QVariant();
}

void ScriptCompletionModel::executeCompletionItem(KTextEditor::View* view, const KTextEditor::Range& /* word */, const QModelIndex& index) const {
	if (!index.isValid() || !view)
		return;

	const auto& item = m_matches.at(index.row());
	QString textToInsert = item.name;

	// Add parentheses for functions and classes
	if (item.isFunction || item.isClass)
		textToInsert += QStringLiteral("()");

	// Replace the current word with the completion
	KTextEditor::Range rangeToReplace = view->document()->wordRangeAt(view->cursorPosition());
	view->document()->replaceText(rangeToReplace, textToInsert);

	// Position cursor inside parentheses for functions/classes
	if (item.isFunction || item.isClass) {
		KTextEditor::Cursor newCursorPos = rangeToReplace.start();
		newCursorPos.setColumn(newCursorPos.column() + item.name.length() + 1);
		view->setCursorPosition(newCursorPos);
	}
}

KTextEditor::Range ScriptCompletionModel::completionRange(KTextEditor::View* view, const KTextEditor::Cursor& cursor) {
	return view->document()->wordRangeAt(cursor);
}

bool ScriptCompletionModel::shouldStartCompletion(KTextEditor::View* view, const QString& insertedText, bool userInsertion, const KTextEditor::Cursor& position) {
	Q_UNUSED(view)
	Q_UNUSED(userInsertion)
	Q_UNUSED(position)

	if (!insertedText.isEmpty()) {
		const QChar lastChar = insertedText.back();
		// Start completion after typing a letter, underscore, or dot (for member access)
		if (lastChar.isLetter() || lastChar == QLatin1Char('_') || lastChar == QLatin1Char('.'))
			return true;
	}

	return false;
}

void ScriptCompletionModel::abortCompletion() {
	if (m_debounceTimer && m_debounceTimer->isActive())
		m_debounceTimer->stop();
}

ScriptCompletionModel::CompletionContext ScriptCompletionModel::detectContext(KTextEditor::View* view,
																				const KTextEditor::Cursor& cursor,
																				QString& objectName) {
	if (!view || !view->document())
		return CompletionContext::Global;

	// Get the line up to the cursor
	QString line = view->document()->line(cursor.line()).left(cursor.column());

	// Check for member access: "object." or "object.prefix"
	static QRegularExpression memberPattern(QStringLiteral(R"((\w+)\.(\w*)$)"));
	QRegularExpressionMatch match = memberPattern.match(line);

	if (match.hasMatch()) {
		objectName = match.captured(1);
		return CompletionContext::Member;
	}

	return CompletionContext::Global;
}

QString ScriptCompletionModel::inferType(const QString& varName, const QString& scriptText) {
	// Simple type inference based on assignment patterns
	// Look for patterns like: varName = ClassName(...) or varName = module.ClassName(...)

	// Pattern 1: Direct class instantiation - varName = ClassName()
	QString pattern1 = QStringLiteral(R"(\b%1\s*=\s*(\w+)\s*\()").arg(QRegularExpression::escape(varName));
	QRegularExpression directPattern(pattern1);
	QRegularExpressionMatch match = directPattern.match(scriptText);

	if (match.hasMatch())
		return match.captured(1); // Return the class name

	// Pattern 2: Module member access - varName = module.ClassName()
	QString pattern2 = QStringLiteral(R"(\b%1\s*=\s*\w+\.(\w+)\s*\()").arg(QRegularExpression::escape(varName));
	QRegularExpression modulePattern(pattern2);
	match = modulePattern.match(scriptText);

	if (match.hasMatch())
		return match.captured(1); // Return the class name

	return QString(); // Unknown type
}

QList<ScriptCompletionModel::CompletionItem> ScriptCompletionModel::membersForType(const QString& typeName) {
	QList<CompletionItem> members;

	if (typeName.isEmpty())
		return members;

	// Check cache first
	if (m_memberCache.contains(typeName))
		return m_memberCache[typeName];

	// Use runtime introspection to get real class members from Python
	auto pylabplotMembers = pylabplotClassMembersHelper(typeName);

	// Convert to CompletionItem format
	for (const auto& memberInfo : pylabplotMembers) {
		CompletionItem item;
		item.name = memberInfo.name;
		item.signature = memberInfo.signature;
		item.docstring = memberInfo.docstring;
		item.isFunction = memberInfo.isMethod;
		item.isVariable = memberInfo.isProperty;
		item.isMember = true;
		item.isClass = false;
		item.isEnum = false;

		members.append(item);
	}

	// Cache the results
	if (!members.isEmpty())
		m_memberCache[typeName] = members;

	return members;
}
