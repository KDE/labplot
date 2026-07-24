/*
	File                 : SpreadsheetHeaderView.h
	Project              : LabPlot
	Description          : Horizontal header for SpreadsheetView displaying comments in a second header
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2016-2025 Alexander Semke <alexander.semke@web.de>
	SPDX-FileCopyrightText: 2007 Tilman Benkert <thzs@gmx.net>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef SPREADSHEETHEADERVIEW_H
#define SPREADSHEETHEADERVIEW_H

#include "SpreadsheetSparkLineHeaderModel.h"
#include <QHeaderView>

class Spreadsheet;

class SpreadsheetCommentsHeaderView : public QHeaderView {
	Q_OBJECT

public:
	explicit SpreadsheetCommentsHeaderView(QWidget* parent = nullptr);
	~SpreadsheetCommentsHeaderView() override;

	void setModel(QAbstractItemModel*) override;
	friend class SpreadsheetHeaderView; // access to paintSection (protected)
};

class SpreadsheetSparkLineHeaderView : public QHeaderView {
	Q_OBJECT
public:
	explicit SpreadsheetSparkLineHeaderView(QWidget* parent = nullptr);

	~SpreadsheetSparkLineHeaderView() override;

	void setModel(QAbstractItemModel*) override;
	friend class SpreadsheetHeaderView; // access to paintSection (protected)

	SpreadsheetSparkLinesHeaderModel* getModel() const;
	QSize sizeHint() const override;
};

class SpreadsheetHeaderView : public QHeaderView {
	Q_OBJECT

public:
	explicit SpreadsheetHeaderView(QWidget* parent, const Spreadsheet*);
	~SpreadsheetHeaderView() override;

	void setModel(QAbstractItemModel*) override;
	SpreadsheetSparkLinesHeaderModel* model() const;
	QSize sizeHint() const override;
	void addSlaveHeader(QHeaderView* slaveHeader);

	void showComments(bool on = true);
	void showSparklines(bool on = true);

Q_SIGNALS:
	void sparklineToggled();

public Q_SLOTS:
	void refresh();

private:
	const Spreadsheet* m_spreadsheet{nullptr};
	bool m_showComments{false};
	bool m_showSparklines{false};
	bool m_sparklineCalled{false};
	SpreadsheetCommentsHeaderView* m_commentSlave;
	SpreadsheetSparkLineHeaderView* m_sparklineSlave;

private Q_SLOTS:
	void headerDataChanged(Qt::Orientation, int logicalFirst, int logicalLast);

protected:
	void paintSection(QPainter*, const QRect&, int logicalIndex) const override;
	bool viewportEvent(QEvent* e) override;

	friend class SpreadsheetView;
};

#endif
