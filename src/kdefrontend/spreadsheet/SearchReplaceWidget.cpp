/*
	File                 : SearchReplaceWidget.cpp
	Project              : LabPlot
	Description          : Search&Replace widget for the spreadsheet
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2023 Alexander Semke <alexander.semke@web.de>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "SearchReplaceWidget.h"

SearchReplaceWidget::SearchReplaceWidget(QWidget* parent) : QWidget(parent) {
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

//SLOTS
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

}

void SearchReplaceWidget::replaceContextMenuRequest(const QPoint& pos) {

}

void SearchReplaceWidget::modeChanged() {

}

void SearchReplaceWidget::matchCaseToggled() {

}

//settings
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

	connect(uiSearchReplace.cbFindPattern, &QComboBox::customContextMenuRequested,
			this, QOverload<const QPoint&>::of(&SearchReplaceWidget::findContextMenuRequest));
	connect(uiSearchReplace.cbReplacePattern, &QComboBox::customContextMenuRequested,
			this, QOverload<const QPoint&>::of(&SearchReplaceWidget::replaceContextMenuRequest));
}
