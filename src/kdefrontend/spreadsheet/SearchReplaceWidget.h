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

class SearchReplaceWidget : public QWidget {
	Q_OBJECT

public:
	explicit SearchReplaceWidget(Spreadsheet*, QWidget* parent = nullptr);
	~SearchReplaceWidget() override;

	void setReplaceEnabled(bool enabled);
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

	void showExtendedContextMenu(bool forPattern, const QPoint&);
	QVector<QString> capturePatterns(const QString& pattern) const;

private Q_SLOTS:
	bool findNext(bool procced = true);
	bool findPrevious(bool procced = true);
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
