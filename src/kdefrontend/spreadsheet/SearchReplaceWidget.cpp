/*
	File                 : SearchReplaceWidget.cpp
	Project              : LabPlot
	Description          : Search&Replace widget for the spreadsheet
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2023 Alexander Semke <alexander.semke@web.de>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "SearchReplaceWidget.h"
#include "backend/spreadsheet/Spreadsheet.h"
#include "backend/spreadsheet/SpreadsheetModel.h"
#include "commonfrontend/spreadsheet/SpreadsheetView.h"
#include "kdefrontend/GuiTools.h"

#include <QLineEdit>
#include <QMenu>
#include <QRadioButton>
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

SearchReplaceWidget::SearchReplaceWidget(Spreadsheet* spreadsheet, QWidget* parent)
	: QWidget(parent)
	, m_spreadsheet(spreadsheet) {
	m_view = static_cast<SpreadsheetView*>(spreadsheet->view());

	auto* layout = new QVBoxLayout(this);
	this->setLayout(layout);
	QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
	this->setSizePolicy(sizePolicy);
}

SearchReplaceWidget::~SearchReplaceWidget() {
}

void SearchReplaceWidget::setReplaceEnabled(bool enabled) {
	m_replaceEnabled = !enabled;
	switchFindReplace();
}

void SearchReplaceWidget::setFocus() {
	if (m_replaceEnabled)
		uiSearchReplace.leValueText->setFocus();
	else
		uiSearch.cbFind->setFocus();
}

void SearchReplaceWidget::clear() {
}

// SLOTS
bool SearchReplaceWidget::findNextImpl(const QString& text, bool proceed) {
	int curRow = m_view->firstSelectedRow();
	int curCol = m_view->firstSelectedColumn();
	const int colCount = m_spreadsheet->columnCount();
	const int rowCount = m_spreadsheet->rowCount();

	const bool columnMajor = (uiSearchReplace.cbOrder->currentIndex() == 0);
	const bool textMode = (uiSearchReplace.cbDataType->currentIndex() == 0);

	if (columnMajor && proceed)
		++curRow;

	if (!columnMajor && proceed)
		++curCol;

	if (textMode) {
		if (columnMajor) {
			for (int col = curCol; col < colCount; ++col) {
				auto* column = m_spreadsheet->column(col);
				if (column->columnMode() != AbstractColumn::ColumnMode::Text)
					continue;

				for (int row = curRow; row < rowCount; ++row) {
					if (column->textAt(row).indexOf(text) != -1) {
						m_view->goToCell(row, col);
						return true;
					}
				}
			}
		} else { // row-major
			for (int row = curRow; row < rowCount; ++row) {
				for (int col = curCol; col < colCount; ++col) {
					auto* column = m_spreadsheet->column(col);
					if (column->columnMode() != AbstractColumn::ColumnMode::Text)
						continue;

					if (column->textAt(row).indexOf(text) != -1) {
						m_view->goToCell(row, col);
						return true;
					}
				}
			}
		}
	} else { // numeric
	}

	return false;
}

bool SearchReplaceWidget::findPreviousImpl(const QString& text, bool proceed) {
	int curRow = m_view->firstSelectedRow();
	int curCol = m_view->firstSelectedColumn();

	const bool columnMajor = (uiSearchReplace.cbOrder->currentIndex() == 0);
	const bool textMode = (uiSearchReplace.cbDataType->currentIndex() == 0);

	if (columnMajor && proceed)
		--curRow;

	if (!columnMajor && proceed)
		--curCol;

	if (textMode) {
		if (columnMajor) {
			for (int col = curCol; col >= 0; --col) {
				auto* column = m_spreadsheet->column(col);
				if (column->columnMode() != AbstractColumn::ColumnMode::Text)
					continue;

				for (int row = curRow; row >= 0; --row) {
					if (column->textAt(row).indexOf(text) != -1) {
						m_view->goToCell(row, col);
						return true;
					}
				}
			}
		} else { // row-major
			for (int row = curRow; row >= 0; --row) {
				for (int col = curCol; col >= 0; --col) {
					auto* column = m_spreadsheet->column(col);
					if (column->columnMode() != AbstractColumn::ColumnMode::Text)
						continue;

					if (column->textAt(row).indexOf(text) != -1) {
						m_view->goToCell(row, col);
						return true;
					}
				}
			}
		}
	} else { // numeric
	}

	return false;
}

void SearchReplaceWidget::findAll() {
	QString text;
	QLineEdit* lineEdit;
	if (m_replaceEnabled)
		lineEdit = uiSearchReplace.leValueText;
	else
		lineEdit = uiSearch.cbFind->lineEdit();

	text = lineEdit->text();

	m_spreadsheet->model()->setSearchText(text);
	m_view->setFocus(); // set the focus so the table gets updated with the highlighted found entries
	lineEdit->setFocus(); // set the focus back to the line edit so we can continue typing
}

void SearchReplaceWidget::replaceNext() {
}

void SearchReplaceWidget::replaceAll() {
}

void SearchReplaceWidget::cancel() {
	m_spreadsheet->model()->setSearchText(QString()); // clear the global search text that was potentialy set during "find all"
	close();
}

void SearchReplaceWidget::findContextMenuRequest(const QPoint& pos) {
	showExtendedContextMenu(false /* replace */, pos);
}

void SearchReplaceWidget::replaceContextMenuRequest(const QPoint& pos) {
	showExtendedContextMenu(true /* replace */, pos);
}

void SearchReplaceWidget::dataTypeChanged(int index) {
	if (index == 0) { // text
		uiSearchReplace.frameNumeric->hide();
		uiSearchReplace.frameText->show();
		uiSearchReplace.frameDateTime->hide();
	} else if (index == 1) { // numeric
		uiSearchReplace.frameNumeric->show();
		uiSearchReplace.frameText->hide();
		uiSearchReplace.frameDateTime->hide();
	} else { // datetime
		uiSearchReplace.frameNumeric->hide();
		uiSearchReplace.frameText->hide();
		uiSearchReplace.frameDateTime->show();
	}
}

void SearchReplaceWidget::operatorChanged(int index) const {
	bool value2 = (index == 2) || (index == 3);
	uiSearchReplace.lMin->setVisible(value2);
	uiSearchReplace.lMax->setVisible(value2);
	uiSearchReplace.lAnd->setVisible(value2);
	uiSearchReplace.leValue2->setVisible(value2);
}

void SearchReplaceWidget::operatorDateTimeChanged(int index) const {
	bool value2 = (index == 2) || (index == 3);
	uiSearchReplace.lMinDateTime->setVisible(value2);
	uiSearchReplace.lMaxDateTime->setVisible(value2);
	uiSearchReplace.lAndDateTime->setVisible(value2);
	uiSearchReplace.dteValue2->setVisible(value2);
}

void SearchReplaceWidget::modeChanged() {
}

void SearchReplaceWidget::matchCaseToggled() {
}

// settings
void SearchReplaceWidget::switchFindReplace() {
	m_replaceEnabled = !m_replaceEnabled;
	if (m_replaceEnabled) { // show the find&replace widget
		if (!m_searchReplaceWidget)
			initSearchReplaceWidget();

		m_searchReplaceWidget->show();

		// TODO
		uiSearchReplace.cbDataType->setMinimumWidth(uiSearchReplace.cbOperator->width());
		uiSearchReplace.cbOrder->setMinimumWidth(uiSearchReplace.cbOperator->width());
		uiSearchReplace.cbOperatorText->setMinimumWidth(uiSearchReplace.cbOperator->width());
		uiSearchReplace.cbOperatorDateTime->setMinimumWidth(uiSearchReplace.cbOperator->width());

		if (m_searchWidget)
			m_searchWidget->hide();
	} else { // show the find widget
		if (!m_searchWidget)
			initSearchWidget();

		m_searchWidget->show();

		if (m_searchReplaceWidget)
			m_searchReplaceWidget->hide();
	}
}

void SearchReplaceWidget::findNext(bool proceed) {
	QLineEdit* lineEdit;
	if (m_replaceEnabled)
		lineEdit = uiSearchReplace.leValueText;
	else
		lineEdit = uiSearch.cbFind->lineEdit();

	const QString& text = lineEdit->text();

	if (!text.isEmpty()) {
		bool rc = findNextImpl(text, proceed);
		GuiTools::highlight(lineEdit, !rc);
	} else
		GuiTools::highlight(lineEdit, false);
}

void SearchReplaceWidget::findPrevious(bool proceed) {
	QLineEdit* lineEdit;
	if (m_replaceEnabled)
		lineEdit = uiSearchReplace.leValueText;
	else
		lineEdit = uiSearch.cbFind->lineEdit();

	const QString& text = lineEdit->text();

	if (!text.isEmpty()) {
		bool rc = findPreviousImpl(text, proceed);
		GuiTools::highlight(lineEdit, !rc);
	} else
		GuiTools::highlight(lineEdit, false);
}

void SearchReplaceWidget::initSearchWidget() {
	m_searchWidget = new QWidget(this);
	uiSearch.setupUi(m_searchWidget);
	static_cast<QVBoxLayout*>(layout())->insertWidget(0, m_searchWidget);

	connect(uiSearch.cbFind->lineEdit(), &QLineEdit::returnPressed, this, [=]() {
		findNext(true);
	});
	connect(uiSearch.cbFind->lineEdit(), &QLineEdit::textChanged, this, [=]() {
		findNext(false);
	});

	connect(uiSearch.tbFindNext, &QToolButton::clicked, this, [=]() {
		findNext(true);
	});
	connect(uiSearch.tbFindPrev, &QToolButton::clicked, this, [=]() {
		findPrevious(true);
	});
	connect(uiSearch.tbMatchCase, &QToolButton::toggled, this, &SearchReplaceWidget::matchCaseToggled);

	connect(uiSearch.tbSwitchFindReplace, &QToolButton::clicked, this, &SearchReplaceWidget::switchFindReplace);
	connect(uiSearch.bCancel, &QPushButton::clicked, this, &SearchReplaceWidget::cancel);
}

void SearchReplaceWidget::initSearchReplaceWidget() {
	m_searchReplaceWidget = new QWidget(this);
	uiSearchReplace.setupUi(m_searchReplaceWidget);
	static_cast<QVBoxLayout*>(layout())->insertWidget(1, m_searchReplaceWidget);

	QStringList items;
	items << i18n("Equal to");
	items << i18n("Not Equal to");
	items << i18n("Between (Incl. End Points)");
	items << i18n("Between (Excl. End Points)");
	items << i18n("Greater than");
	items << i18n("Greater than or Equal to");
	items << i18n("Less than");
	items << i18n("Less than or Equal to");

	uiSearchReplace.cbOperator->addItems(items);
	uiSearchReplace.cbOperatorDateTime->addItems(items);

	uiSearchReplace.cbOperatorText->addItem(i18n("Equal To"));
	uiSearchReplace.cbOperatorText->addItem(i18n("Not Equal To"));
	uiSearchReplace.cbOperatorText->addItem(i18n("Starts With"));
	uiSearchReplace.cbOperatorText->addItem(i18n("Ends With"));
	uiSearchReplace.cbOperatorText->addItem(i18n("Contains"));
	uiSearchReplace.cbOperatorText->addItem(i18n("Does Not Contain"));
	uiSearchReplace.cbOperatorText->insertSeparator(6);
	uiSearchReplace.cbOperatorText->addItem(i18n("Regular Expression"));

	uiSearchReplace.leValue1->setValidator(new QDoubleValidator(uiSearchReplace.leValue1));
	uiSearchReplace.leValue2->setValidator(new QDoubleValidator(uiSearchReplace.leValue2));

	// connections
	connect(uiSearchReplace.cbDataType, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &SearchReplaceWidget::dataTypeChanged);

	connect(uiSearchReplace.cbOperator, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &SearchReplaceWidget::operatorChanged);
	connect(uiSearchReplace.cbOperatorText, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [=]() {
		findNext(true);
	});
	connect(uiSearchReplace.cbOperatorDateTime, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &SearchReplaceWidget::operatorDateTimeChanged);

	connect(uiSearchReplace.leValue1, &QLineEdit::returnPressed, this, [=]() {
		findNext(true);
	});
	connect(uiSearchReplace.leValue2, &QLineEdit::returnPressed, this, [=]() {
		findNext(true);
	});
	connect(uiSearchReplace.leValue1, &QLineEdit::textChanged, this, [=]() {
		findNext(false);
	});
	connect(uiSearchReplace.leValue2, &QLineEdit::textChanged, this, [=]() {
		findNext(false);
	});

	connect(uiSearchReplace.leValueText, &QLineEdit::returnPressed, this, [=]() {
		findNext(true);
	});
	connect(uiSearchReplace.leValueText, &QLineEdit::textChanged, this, [=]() {
		findNext(false);
	});

	connect(uiSearchReplace.dteValue1, &UTCDateTimeEdit::mSecsSinceEpochUTCChanged, this, [=]() {
		findNext(false);
	});
	connect(uiSearchReplace.dteValue2, &UTCDateTimeEdit::mSecsSinceEpochUTCChanged, this, [=]() {
		findNext(false);
	});

	connect(uiSearchReplace.tbFindNext, &QToolButton::clicked, this, [=]() {
		findNext(true);
	});
	connect(uiSearchReplace.tbFindPrev, &QToolButton::clicked, this, [=]() {
		findPrevious(true);
	});
	connect(uiSearchReplace.bFindAll, &QPushButton::clicked, this, &SearchReplaceWidget::findAll);

	connect(uiSearchReplace.bReplaceNext, &QPushButton::clicked, this, &SearchReplaceWidget::replaceNext);
	connect(uiSearchReplace.bReplaceAll, &QPushButton::clicked, this, &SearchReplaceWidget::replaceAll);
	connect(uiSearchReplace.tbMatchCase, &QToolButton::toggled, this, &SearchReplaceWidget::matchCaseToggled);

	connect(uiSearchReplace.tbSwitchFindReplace, &QToolButton::clicked, this, &SearchReplaceWidget::switchFindReplace);
	connect(uiSearchReplace.bCancel, &QPushButton::clicked, this, &SearchReplaceWidget::cancel);

	// custom context menus for LineEdit in ComboBox
	uiSearchReplace.leValueText->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(uiSearchReplace.leValueText,
			&QComboBox::customContextMenuRequested,
			this,
			QOverload<const QPoint&>::of(&SearchReplaceWidget::findContextMenuRequest));

	uiSearchReplace.cbReplace->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(uiSearchReplace.cbReplace,
			&QComboBox::customContextMenuRequested,
			this,
			QOverload<const QPoint&>::of(&SearchReplaceWidget::replaceContextMenuRequest));

	// read saved settings
	// TODO:

	dataTypeChanged(uiSearchReplace.cbDataType->currentIndex());
	operatorChanged(uiSearchReplace.cbOperator->currentIndex());
	operatorDateTimeChanged(uiSearchReplace.cbOperatorDateTime->currentIndex());
}

void SearchReplaceWidget::showEvent(QShowEvent* event) {
	QWidget::showEvent(event);
}

void SearchReplaceWidget::showExtendedContextMenu(bool replace, const QPoint& pos) {
	// Make original menu
	QLineEdit* lineEdit;
	if (replace)
		lineEdit = uiSearchReplace.cbReplace->lineEdit();
	else
		lineEdit = uiSearchReplace.leValueText;

	auto* const contextMenu = lineEdit->createStandardContextMenu();
	if (!contextMenu)
		return;

	bool extendMenu = false;
	bool regexMode = false;
	if (uiSearchReplace.cbDataType->currentIndex() == 0) {
		// TODO
		extendMenu = true;
		regexMode = true;
		// switch (uiSearchReplace.cbMode->currentIndex()) {
		// case MODE_REGEX:
		// 	regexMode = true;
		// 	// FALLTHROUGH
		// case MODE_ESCAPE_SEQUENCES:
		// 	extendMenu = true;
		// 	break;
		// default:
		// 	break;
		// }
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
	auto* const result = contextMenu->exec(lineEdit->mapToGlobal(pos));
	if (result)
		addMenuManager.handle(result, lineEdit);
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
