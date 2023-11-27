/*
	File                 : SearchReplaceWidget.h
	Project              : LabPlot
	Description          : Search&Replace widget for the spreadsheet
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2023 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef SEARCHREPLACEWIDGET_H
#define SEARCHREPLACEWIDGET_H

#include "backend/core/AbstractColumn.h"
#include "ui_searchreplacewidget.h"
#include "ui_searchwidget.h"
#include <QWidget>

class Column;
class Spreadsheet;
class SpreadsheetView;
class KMessageWidget;
class QRadioButton;

class SearchReplaceWidget : public QWidget {
	Q_OBJECT

public:
	explicit SearchReplaceWidget(Spreadsheet*, QWidget* parent = nullptr);
	~SearchReplaceWidget() override;

	enum class DataType { Text, Numeric, DateTime };
	enum class Operator { EqualTo, NotEqualTo, BetweenIncl, BetweenExcl, GreaterThan, GreaterThanEqualTo, LessThan, LessThanEqualTo };
	enum class OperatorText { EqualTo, NotEqualTo, StartsWith, EndsWith, Contain, NotContain, RegEx };
	enum class Order { ColumnMajor, RowMajor };

	void setReplaceEnabled(bool enabled);
	void setInitialPattern(AbstractColumn::ColumnMode, const QString&);
	void setFocus();

private:
	Ui::SearchWidget uiSearch;
	Ui::SearchReplaceWidget uiSearchReplace;
	QWidget* m_searchWidget{nullptr};
	QWidget* m_searchReplaceWidget{nullptr};
	AbstractColumn::ColumnMode m_initialColumnMode{AbstractColumn::ColumnMode::Text};
	bool m_patternFound{false};
	bool m_replaceEnabled{false};
	Spreadsheet* m_spreadsheet{nullptr};
	SpreadsheetView* m_view{nullptr};
	KMessageWidget* m_messageWidget{nullptr};

	void initSearchWidget();
	void initSearchReplaceWidget();

	bool checkColumnType(Column*, DataType);
	bool checkColumnRow(Column*,
						DataType,
						int row,
						OperatorText opText,
						Operator opNumeric,
						Operator opDateTime,
						const QString& pattern1,
						const QString pattern2,
						Qt::CaseSensitivity cs);
	bool checkCellText(const QString& value, const QString& pattern, OperatorText, Qt::CaseSensitivity);
	bool checkCellNumeric(double value, const QString& pattern1, const QString& pattern2, Operator);
	bool checkCellDateTime(const QDateTime& value, const QDateTime& pattern1, const QDateTime& pattern2, Operator);
	void setValue(Column*, DataType, int row, const QString& value);
	void highlight(DataType, bool invalid);
	void showMessage(const QString&);

	void addCurrentTextToHistory(QComboBox*) const;
	void showExtendedContextMenu(bool forPattern, const QPoint&);
	QVector<QString> capturePatterns(const QString& pattern) const;

	friend class SpreadsheetTest;
	void setDataType(DataType);
	void setOrder(Order);
	void setTextOperator(OperatorText);
	void setReplaceText(const QString&);

private Q_SLOTS:
	void dataTypeChanged(int index);
	void operatorChanged(int) const;
	void operatorDateTimeChanged(int) const;

	bool findNextSimple(bool proceed);
	bool findPreviousSimple(bool proceed);
	bool findNext(bool proceed, bool findAndReplace = false);
	bool findPrevious(bool proceed);
	void findAll();
	void replaceNext();
	void replaceAll();
	void cancel();

	void findContextMenuRequest(const QPoint&);
	void replaceContextMenuRequest(const QPoint&);

	void switchFindReplace();

Q_SIGNALS:
};

#endif
