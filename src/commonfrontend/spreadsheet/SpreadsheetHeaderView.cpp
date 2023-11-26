/*
	File                 : SpreadsheetHeaderView.cpp
	Project              : LabPlot
	Description          : Horizontal header for SpreadsheetView displaying comments in a second header
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2016 Alexander Semke <alexander.semke@web.de>
	SPDX-FileCopyrightText: 2007 Tilman Benkert <thzs@gmx.net>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "SpreadsheetHeaderView.h"
#include "SpreadsheetCommentsHeaderModel.h"
#include "SpreadsheetSparkLineHeaderModel.h"
#include "backend/lib/macros.h"

/*!
 \class SpreadsheetCommentsHeaderView
 \brief Slave header for SpreadsheetDoubleHeaderView

 This class is only to be used by SpreadsheetDoubleHeaderView.
 It allows for displaying two horizontal headers in a SpreadsheetView.
 A SpreadsheetCommentsHeaderView displays the column comments
 in a second header below the normal header. It is completely
 controlled by a SpreadsheetDoubleHeaderView object and thus has
 a master-slave relationship to it. This would be an inner class
 of SpreadsheetDoubleHeaderView if Qt allowed this.

 \ingroup commonfrontend
*/

SpreadsheetCommentsHeaderView::~SpreadsheetCommentsHeaderView() {
	delete model();
}

void SpreadsheetCommentsHeaderView::setModel(QAbstractItemModel* model) {
	Q_ASSERT(model->inherits("SpreadsheetModel"));
	delete QHeaderView::model();
	auto* new_model = new SpreadsheetCommentsHeaderModel(static_cast<SpreadsheetModel*>(model));
	QHeaderView::setModel(new_model);
}

/*!
 \class SpreadsheetSparkLinesHeaderView
 \brief Slave header for SpreadsheetDoubleHeaderView

This class is only to be used by SpreadsheetDoubleHeaderView.
It allows for displaying two horizontal headers in a SpreadsheetView.
A SpreadsheetSparkLinesHeaderView displays the column comments
in a second header below the normal header. It is completely
controlled by a SpreadsheetDoubleHeaderView object and thus has
a master-slave relationship to it. This would be an inner class
of SpreadsheetDoubleHeaderView if Qt allowed this.

\ingroup commonfrontend
*/

SpreadsheetSparkLineHeaderView::~SpreadsheetSparkLineHeaderView() {
	delete model();
}

void SpreadsheetSparkLineHeaderView::setModel(QAbstractItemModel* model) {
	Q_ASSERT(model->inherits("SpreadsheetModel"));
	delete QHeaderView::model();
	auto* new_model = new SpreadsheetSparkLinesHeaderModel(static_cast<SpreadsheetModel*>(model));
	QHeaderView::setModel(new_model);
}

/*!
 \class SpreadsheetDoubleHeaderView
 \brief Horizontal header for SpreadsheetView displaying comments in a second header

 This class is only to be used by SpreadsheetView.
 It allows for displaying two horizontal headers.
 A \c SpreadsheetDoubleHeaderView displays the column name, plot designation, and
 type icon in a normal QHeaderView and below that a second header
 which displays the column comments.

 \sa SpreadsheetCommentsHeaderView
 \sa QHeaderView

 \ingroup commonfrontend
*/
SpreadsheetHeaderView::SpreadsheetHeaderView(QWidget* parent)
	: QHeaderView(Qt::Horizontal, parent) {
	setDefaultAlignment(Qt::AlignLeft | Qt::AlignVCenter);
	m_commentSlave = new SpreadsheetCommentsHeaderView();
	m_sparkLineSlave = new SpreadsheetSparkLineHeaderView();
	m_commentSlave->setDefaultAlignment(Qt::AlignLeft | Qt::AlignVCenter);
	m_sparkLineSlave->setDefaultAlignment(Qt::AlignLeft | Qt::AlignCenter);
	m_showComments = true;
	m_showSparkLines = true;
}

SpreadsheetHeaderView::~SpreadsheetHeaderView() {
	delete m_commentSlave;
	delete m_sparkLineSlave;
}

QSize SpreadsheetHeaderView::sizeHint() const {
	QSize masterSize = QHeaderView::sizeHint();
	QSize sparkLineSize = m_sparkLineSlave->sizeHint();
	QSize commentSize = m_commentSlave->sizeHint();

	int totalHeight = masterSize.height();
	if (m_showSparkLines)
		totalHeight += sparkLineSize.height();
	if (m_showComments)
		totalHeight += commentSize.height();

	return QSize(masterSize.width(), totalHeight);
}

void SpreadsheetHeaderView::setModel(QAbstractItemModel* model) {
	Q_ASSERT(model->inherits("SpreadsheetModel"));
	m_commentSlave->setModel(model);
	QHeaderView::setModel(model);
	connect(model, &QAbstractItemModel::headerDataChanged, this, &SpreadsheetHeaderView::headerDataChanged);
}

void SpreadsheetHeaderView::paintSection(QPainter* painter, const QRect& rect, int logicalIndex) const {
	QRect master_rect = rect;

	if (m_showComments)
		master_rect = rect.adjusted(0, 0, 0, -m_commentSlave->sizeHint().height());

	QHeaderView::paintSection(painter, master_rect, logicalIndex);
	if (m_showComments) {
		QRect slave_comment_rect = rect.adjusted(0, QHeaderView::sizeHint().height(), 0, 0);
		m_commentSlave->paintSection(painter, slave_comment_rect, logicalIndex);
	}
	if (m_showSparkLines)
		master_rect = rect.adjusted(0, 0, 0, -m_commentSlave->sizeHint().height());

	QHeaderView::paintSection(painter, master_rect, logicalIndex);
	if (m_showSparkLines) {
		QRect slave_sparkline_rect = rect.adjusted(0, QHeaderView::sizeHint().height(), 0, 0);
		m_sparkLineSlave->paintSection(painter, slave_sparkline_rect, logicalIndex);
	}
}

/*!
  Returns whether comments are shown currently or not.
*/
bool SpreadsheetHeaderView::areCommentsShown() const {
	return m_showComments;
}

/*!
  Show or hide (if \c on = \c false) the column comments.
*/
void SpreadsheetHeaderView::showComments(bool on) {
	m_showComments = on;
	DEBUG("Comments" << on);
	refresh();
}
/*!
  Show or hide (if \c on = \c false) the column spark lines.
*/
bool SpreadsheetHeaderView::areSparkLinesShown() const {
	return m_showSparkLines;
}
/*!
  Returns whether spark lines are shown currently or not.
*/
void SpreadsheetHeaderView::showSparkLines(bool on) {
	m_showSparkLines = on;
	DEBUG("SparkLines" << on);
	refresh();
}

/*!
  adjust geometry and repaint header .
*/
void SpreadsheetHeaderView::refresh() {
	// Calculate total width and height
	int totalWidth = 0;
	int totalHeight = 0;

	for (int i = 0; i < count(); ++i) {
		totalWidth += sectionSize(i);
	}

	// Calculate height of each section
	int masterHeight = QHeaderView::sizeHint().height();
	int sparkLineHeight = m_commentSlave->sizeHint().height();
	int commentHeight = m_commentSlave->sizeHint().height();

	// Update total height based on visible sections
	totalHeight = masterHeight + (m_showSparkLines ? commentHeight : 0) + (m_showComments ? commentHeight : 0);

	// Update geometry for both slaves
	m_sparkLineSlave->setGeometry(0, masterHeight + (m_showComments ? commentHeight : 0), totalWidth, commentHeight);
	m_commentSlave->setGeometry(0, masterHeight, totalWidth, commentHeight);

	setGeometry(0, 0, totalWidth, totalHeight);
	DEBUG("Total Height" << totalHeight);
	DEBUG("Total Width" << totalWidth);
	DEBUG("SparkLine Height" << sparkLineHeight);
	DEBUG("Comment Height" << totalHeight);

	update();
}

/*!
  Reacts to a header data change.
*/
void SpreadsheetHeaderView::headerDataChanged(Qt::Orientation orientation, int /*logicalFirst*/, int logicalLast) {
	if (logicalLast < 0)
		return;
	if (orientation == Qt::Horizontal)
		refresh();
}

