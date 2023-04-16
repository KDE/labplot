/*
	File                 : SearchReplaceWidget.cpp
	Project              : LabPlot
	Description          : Search&Replace widget for the spreadsheet
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2023 Alexander Semke <alexander.semke@web.de>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "SearchReplaceWidget.h"

#include <QLineEdit>
#include <QMenu>
#include <QStack>

enum SearchMode {
	// NOTE: Concrete values are important here to work with the combobox index!
	MODE_PLAIN_TEXT = 0,
	MODE_WHOLE_WORDS = 1,
	MODE_ESCAPE_SEQUENCES = 2,
	MODE_REGEX = 3
};

class AddMenuManager {
private:
	QVector<QString> m_insertBefore;
	QVector<QString> m_insertAfter;
	QSet<QAction*> m_actionPointers;
	uint m_indexWalker{0};
	QMenu* m_menu{nullptr};

public:
	AddMenuManager(QMenu* parent, int expectedItemCount)
		: m_insertBefore(QVector<QString>(expectedItemCount))
		, m_insertAfter(QVector<QString>(expectedItemCount)) {
		Q_ASSERT(parent != nullptr);

		m_menu = parent->addMenu(i18n("Add..."));
		if (!m_menu)
			return;

		m_menu->setIcon(QIcon::fromTheme(QStringLiteral("list-add")));
	}

	void enableMenu(bool enabled) {
		if (m_menu == nullptr)
			return;

		m_menu->setEnabled(enabled);
	}

	void addEntry(const QString& before,
				  const QString& after,
				  const QString& description,
				  const QString& realBefore = QString(),
				  const QString& realAfter = QString()) {
		if (!m_menu)
			return;

		auto* const action = m_menu->addAction(before + after + QLatin1Char('\t') + description);
		m_insertBefore[m_indexWalker] = QString(realBefore.isEmpty() ? before : realBefore);
		m_insertAfter[m_indexWalker] = QString(realAfter.isEmpty() ? after : realAfter);
		action->setData(QVariant(m_indexWalker++));
		m_actionPointers.insert(action);
	}

	void addSeparator() {
		if (!m_menu)
			return;

		m_menu->addSeparator();
	}

	void handle(QAction* action, QLineEdit* lineEdit) {
		if (!m_actionPointers.contains(action))
			return;

		const int cursorPos = lineEdit->cursorPosition();
		const int index = action->data().toUInt();
		const QString& before = m_insertBefore[index];
		const QString& after = m_insertAfter[index];
		lineEdit->insert(before + after);
		lineEdit->setCursorPosition(cursorPos + before.count());
		lineEdit->setFocus();
	}
};

struct ParInfo {
	int openIndex;
	bool capturing;
	int captureNumber; // 1..9
};

SearchReplaceWidget::SearchReplaceWidget(QWidget* parent)
	: QWidget(parent) {
	auto* layout = new QVBoxLayout(this);
	this->setLayout(layout);
	QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
	this->setSizePolicy(sizePolicy);
}

SearchReplaceWidget::~SearchReplaceWidget() {
}

void SearchReplaceWidget::setReplaceEnabled(bool enabled) {
	m_replaceEnabled = enabled;
	switchFindReplace();
}

void SearchReplaceWidget::clear() {
}

// SLOTS
void SearchReplaceWidget::findNext() {
}

void SearchReplaceWidget::findPrevious() {
}

void SearchReplaceWidget::findAll() {
}

void SearchReplaceWidget::replaceNext() {
}

void SearchReplaceWidget::replaceAll() {
}

void SearchReplaceWidget::cancel() {
	close();
}

void SearchReplaceWidget::findContextMenuRequest(const QPoint& pos) {
	showExtendedContextMenu(false /* replace */, pos);
}

void SearchReplaceWidget::replaceContextMenuRequest(const QPoint& pos) {
	showExtendedContextMenu(true /* replace */, pos);
}

void SearchReplaceWidget::modeChanged() {
}

void SearchReplaceWidget::matchCaseToggled() {
}

// settings
void SearchReplaceWidget::switchFindReplace() {
	if (m_replaceEnabled) { // show the find&replace widget
		if (!m_searchReplaceWidget)
			initSearchReplaceWidget();

		m_searchReplaceWidget->show();

		if (m_searchWidget && m_searchWidget->isVisible())
			m_searchWidget->hide();
	} else { // show the find widget
		if (!m_searchWidget)
			initSearchWidget();

		m_searchWidget->show();

		if (m_searchReplaceWidget && m_searchReplaceWidget->isVisible())
			m_searchReplaceWidget->hide();
	}
}

void SearchReplaceWidget::initSearchWidget() {
	m_searchWidget = new QWidget(this);
	uiSearch.setupUi(m_searchWidget);
	layout()->addWidget(m_searchWidget);

	connect(uiSearch.tbSwitchFindReplace, &QToolButton::clicked, this, &SearchReplaceWidget::switchFindReplace);
	connect(uiSearch.bCancel, &QPushButton::clicked, this, &SearchReplaceWidget::cancel);

	connect(uiSearch.tbFindNext, &QToolButton::clicked, this, &SearchReplaceWidget::findNext);
	connect(uiSearch.tbFindPrev, &QToolButton::clicked, this, &SearchReplaceWidget::findPrevious);
	connect(uiSearch.tbMatchCase, &QToolButton::toggled, this, &SearchReplaceWidget::matchCaseToggled);
}

void SearchReplaceWidget::initSearchReplaceWidget() {
	m_searchReplaceWidget = new QWidget(this);
	uiSearchReplace.setupUi(m_searchReplaceWidget);
	layout()->addWidget(m_searchReplaceWidget);

	connect(uiSearchReplace.tbSwitchFindReplace, &QToolButton::clicked, this, &SearchReplaceWidget::switchFindReplace);
	connect(uiSearchReplace.bCancel, &QPushButton::clicked, this, &SearchReplaceWidget::cancel);

	connect(uiSearchReplace.tbFindNext, &QToolButton::clicked, this, &SearchReplaceWidget::findNext);
	connect(uiSearchReplace.tbFindPrev, &QToolButton::clicked, this, &SearchReplaceWidget::findPrevious);
	connect(uiSearchReplace.bFindAll, &QPushButton::clicked, this, &SearchReplaceWidget::findAll);
	connect(uiSearchReplace.bReplaceNext, &QPushButton::clicked, this, &SearchReplaceWidget::replaceNext);
	connect(uiSearchReplace.bReplaceAll, &QPushButton::clicked, this, &SearchReplaceWidget::replaceAll);
	connect(uiSearchReplace.cbMode, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &SearchReplaceWidget::modeChanged);
	connect(uiSearchReplace.tbMatchCase, &QToolButton::toggled, this, &SearchReplaceWidget::matchCaseToggled);

	// custom context menus for LineEdit in ComboBox
	uiSearchReplace.cbFind->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(uiSearchReplace.cbFind, &QComboBox::customContextMenuRequested, this, QOverload<const QPoint&>::of(&SearchReplaceWidget::findContextMenuRequest));

	uiSearchReplace.cbReplace->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(uiSearchReplace.cbReplace,
			&QComboBox::customContextMenuRequested,
			this,
			QOverload<const QPoint&>::of(&SearchReplaceWidget::replaceContextMenuRequest));
}

void SearchReplaceWidget::showExtendedContextMenu(bool replace, const QPoint& pos) {
	// Make original menu
	auto* comboBox = replace ? uiSearchReplace.cbReplace : uiSearchReplace.cbFind;
	auto* const contextMenu = comboBox->lineEdit()->createStandardContextMenu();

	if (!contextMenu)
		return;

	bool extendMenu = false;
	bool regexMode = false;
	switch (uiSearchReplace.cbMode->currentIndex()) {
	case MODE_REGEX:
		regexMode = true;
		// FALLTHROUGH
	case MODE_ESCAPE_SEQUENCES:
		extendMenu = true;
		break;
	default:
		break;
	}

	AddMenuManager addMenuManager(contextMenu, 37);
	if (!extendMenu)
		addMenuManager.enableMenu(extendMenu);
	else {
		// Build menu
		if (!replace) {
			if (regexMode) {
				addMenuManager.addEntry(QStringLiteral("^"), QString(), i18n("Beginning of line"));
				addMenuManager.addEntry(QStringLiteral("$"), QString(), i18n("End of line"));
				addMenuManager.addSeparator();
				addMenuManager.addEntry(QStringLiteral("."), QString(), i18n("Match any character excluding new line (by default)"));
				addMenuManager.addEntry(QStringLiteral("+"), QString(), i18n("One or more occurrences"));
				addMenuManager.addEntry(QStringLiteral("*"), QString(), i18n("Zero or more occurrences"));
				addMenuManager.addEntry(QStringLiteral("?"), QString(), i18n("Zero or one occurrences"));
				addMenuManager.addEntry(QStringLiteral("{a"),
										QStringLiteral(",b}"),
										i18n("<a> through <b> occurrences"),
										QStringLiteral("{"),
										QStringLiteral(",}"));

				addMenuManager.addSeparator();
				addMenuManager.addSeparator();
				addMenuManager.addEntry(QStringLiteral("("), QStringLiteral(")"), i18n("Group, capturing"));
				addMenuManager.addEntry(QStringLiteral("|"), QString(), i18n("Or"));
				addMenuManager.addEntry(QStringLiteral("["), QStringLiteral("]"), i18n("Set of characters"));
				addMenuManager.addEntry(QStringLiteral("[^"), QStringLiteral("]"), i18n("Negative set of characters"));
				addMenuManager.addSeparator();
			}
		} else {
			addMenuManager.addEntry(QStringLiteral("\\0"), QString(), i18n("Whole match reference"));
			addMenuManager.addSeparator();
			if (regexMode) {
				const QString pattern = uiSearchReplace.cbReplace->currentText();
				const QVector<QString> capturePatterns = this->capturePatterns(pattern);

				const int captureCount = capturePatterns.count();
				for (int i = 1; i <= 9; i++) {
					const QString number = QString::number(i);
					const QString& captureDetails =
						(i <= captureCount) ? QLatin1String(" = (") + QStringView(capturePatterns[i - 1]).left(30) + QLatin1Char(')') : QString();
					addMenuManager.addEntry(QLatin1String("\\") + number, QString(), i18n("Reference") + QLatin1Char(' ') + number + captureDetails);
				}

				addMenuManager.addSeparator();
			}
		}

		addMenuManager.addEntry(QStringLiteral("\\n"), QString(), i18n("Line break"));
		addMenuManager.addEntry(QStringLiteral("\\t"), QString(), i18n("Tab"));

		if (!replace && regexMode) {
			addMenuManager.addEntry(QStringLiteral("\\b"), QString(), i18n("Word boundary"));
			addMenuManager.addEntry(QStringLiteral("\\B"), QString(), i18n("Not word boundary"));
			addMenuManager.addEntry(QStringLiteral("\\d"), QString(), i18n("Digit"));
			addMenuManager.addEntry(QStringLiteral("\\D"), QString(), i18n("Non-digit"));
			addMenuManager.addEntry(QStringLiteral("\\s"), QString(), i18n("Whitespace (excluding line breaks)"));
			addMenuManager.addEntry(QStringLiteral("\\S"), QString(), i18n("Non-whitespace"));
			addMenuManager.addEntry(QStringLiteral("\\w"), QString(), i18n("Word character (alphanumerics plus '_')"));
			addMenuManager.addEntry(QStringLiteral("\\W"), QString(), i18n("Non-word character"));
		}

		addMenuManager.addEntry(QStringLiteral("\\0???"), QString(), i18n("Octal character 000 to 377 (2^8-1)"), QStringLiteral("\\0"));
		addMenuManager.addEntry(QStringLiteral("\\x{????}"), QString(), i18n("Hex character 0000 to FFFF (2^16-1)"), QStringLiteral("\\x{....}"));
		addMenuManager.addEntry(QStringLiteral("\\\\"), QString(), i18n("Backslash"));

		if (!replace && regexMode) {
			addMenuManager.addSeparator();
			addMenuManager.addEntry(QStringLiteral("(?:E"), QStringLiteral(")"), i18n("Group, non-capturing"), QStringLiteral("(?:"));
			addMenuManager.addEntry(QStringLiteral("(?=E"), QStringLiteral(")"), i18n("Positive Lookahead"), QStringLiteral("(?="));
			addMenuManager.addEntry(QStringLiteral("(?!E"), QStringLiteral(")"), i18n("Negative lookahead"), QStringLiteral("(?!"));
			// variable length positive/negative lookbehind is an experimental feature in Perl 5.30
			// see: https://perldoc.perl.org/perlre.html
			// currently QRegularExpression only supports fixed-length positive/negative lookbehind (2020-03-01)
			addMenuManager.addEntry(QStringLiteral("(?<=E"), QStringLiteral(")"), i18n("Fixed-length positive lookbehind"), QStringLiteral("(?<="));
			addMenuManager.addEntry(QStringLiteral("(?<!E"), QStringLiteral(")"), i18n("Fixed-length negative lookbehind"), QStringLiteral("(?<!"));
		}

		if (replace) {
			addMenuManager.addSeparator();
			addMenuManager.addEntry(QStringLiteral("\\L"), QString(), i18n("Begin lowercase conversion"));
			addMenuManager.addEntry(QStringLiteral("\\U"), QString(), i18n("Begin uppercase conversion"));
			addMenuManager.addEntry(QStringLiteral("\\E"), QString(), i18n("End case conversion"));
			addMenuManager.addEntry(QStringLiteral("\\l"), QString(), i18n("Lowercase first character conversion"));
			addMenuManager.addEntry(QStringLiteral("\\u"), QString(), i18n("Uppercase first character conversion"));
			addMenuManager.addEntry(QStringLiteral("\\#[#..]"), QString(), i18n("Replacement counter (for Replace All)"), QStringLiteral("\\#"));
		}
	}

	// Show menu
	auto* const result = contextMenu->exec(comboBox->mapToGlobal(pos));
	if (result)
		addMenuManager.handle(result, comboBox->lineEdit());
}

QVector<QString> SearchReplaceWidget::capturePatterns(const QString& pattern) const {
	QVector<QString> capturePatterns;
	capturePatterns.reserve(9);
	QStack<ParInfo> parInfos;

	const int inputLen = pattern.length();
	int input = 0; // walker index
	bool insideClass = false;
	int captureCount = 0;

	while (input < inputLen) {
		if (insideClass) {
			// Wait for closing, unescaped ']'
			if (pattern[input].unicode() == L']')
				insideClass = false;

			input++;
		} else {
			switch (pattern[input].unicode()) {
			case L'\\':
				// Skip this and any next character
				input += 2;
				break;

			case L'(':
				ParInfo curInfo;
				curInfo.openIndex = input;
				curInfo.capturing = (input + 1 >= inputLen) || (pattern[input + 1].unicode() != '?');
				if (curInfo.capturing) {
					captureCount++;
				}
				curInfo.captureNumber = captureCount;
				parInfos.push(curInfo);

				input++;
				break;

			case L')':
				if (!parInfos.empty()) {
					ParInfo& top = parInfos.top();
					if (top.capturing && (top.captureNumber <= 9)) {
						const int start = top.openIndex + 1;
						const int len = input - start;
						if (capturePatterns.size() < top.captureNumber) {
							capturePatterns.resize(top.captureNumber);
						}
						capturePatterns[top.captureNumber - 1] = pattern.mid(start, len);
					}
					parInfos.pop();
				}

				input++;
				break;

			case L'[':
				input++;
				insideClass = true;
				break;

			default:
				input++;
				break;
			}
		}
	}

	return capturePatterns;
}
