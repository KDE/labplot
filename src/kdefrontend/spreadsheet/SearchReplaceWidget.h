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

#include "ui_searchreplacewidget.h"
#include "ui_searchwidget.h"
#include <QWidget>

class Spreadsheet;
class SpreadsheetView;
class QRadioButton;

class SearchReplaceWidget : public QWidget {
	Q_OBJECT

public:
	explicit SearchReplaceWidget(Spreadsheet*, QWidget* parent = nullptr);
	~SearchReplaceWidget() override;

	enum class Operator { EqualTo, NotEqualTo, BetweenIncl, BetweenExcl, GreaterThan, GreaterThanEqualTo, LessThan, LessThanEqualTo };
	enum class OperatorText { EqualTo, NotEqualTo, StartsWith, EndsWith, Contain, NotContain, RegEx };

	void setReplaceEnabled(bool enabled);
	void setFocus();
	void clear();

private:
	Ui::SearchWidget uiSearch;
	Ui::SearchReplaceWidget uiSearchReplace;
	QWidget* m_searchWidget{nullptr};
	QWidget* m_searchReplaceWidget{nullptr};

	bool m_replaceEnabled{false};
	Spreadsheet* m_spreadsheet{nullptr};
	SpreadsheetView* m_view{nullptr};

	void initSearchWidget();
	void initSearchReplaceWidget();

	bool findNextImpl(const QString&, bool proceed);
	bool findPrevImpl(const QString&, bool proceed);

	bool findNextText(const QString&, bool proceed);
	bool findPrevText(const QString&, bool proceed);
	bool checkCellText(const QString& cellText, const QString& text, OperatorText, Qt::CaseSensitivity);

	bool findNextNumeric(const QString&, bool proceed);
	bool findPrevNumeric(const QString&, bool proceed);

	bool findNextDateTime(const QString&, bool proceed);
	bool findPrevDateTime(const QString&, bool proceed);

	void showExtendedContextMenu(bool forPattern, const QPoint&);
	QVector<QString> capturePatterns(const QString& pattern) const;

	void showEvent(QShowEvent*) override;

private Q_SLOTS:
	void dataTypeChanged(int index);
	void operatorChanged(int) const;
	void operatorDateTimeChanged(int) const;

	void findNext(bool proceed);
	void findPrevious(bool proceed);
	void findAll();
	void replaceNext();
	void replaceAll();
	void cancel();
	void modeChanged();
	void matchCaseToggled();

	void findContextMenuRequest(const QPoint&);
	void replaceContextMenuRequest(const QPoint&);

	void switchFindReplace();

Q_SIGNALS:
};

#endif
