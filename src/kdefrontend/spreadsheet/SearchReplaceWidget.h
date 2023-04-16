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

#include <QWidget>
#include "ui_searchwidget.h"
#include "ui_searchreplacewidget.h"

class SearchReplaceWidget : public QWidget {
	Q_OBJECT

public:
	explicit SearchReplaceWidget(QWidget* parent = nullptr);
	~SearchReplaceWidget() override;

	void setReplaceEnabled(bool enabled);
	void clear();

private:
	Ui::SearchWidget uiSearch;
	Ui::SearchReplaceWidget uiSearchReplace;
	QWidget* m_searchWidget{nullptr};
	QWidget* m_searchReplaceWidget{nullptr};
	bool m_replaceEnabled{false};

	void initSearchWidget();
	void initSearchReplaceWidget();

private Q_SLOTS:
	void findNext();
	void findPrevious();
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
