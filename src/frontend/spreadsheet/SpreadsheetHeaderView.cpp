/*
	File                 : SpreadsheetHeaderView.cpp
	Project              : LabPlot
	Description          : Horizontal header for SpreadsheetView displaying comments in a second header
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2016-2025 Alexander Semke <alexander.semke@web.de>
	SPDX-FileCopyrightText: 2007 Tilman Benkert <thzs@gmx.net>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "SpreadsheetHeaderView.h"
#include "SpreadsheetCommentsHeaderModel.h"
#include "SpreadsheetSparkLineHeaderModel.h"
#include "backend/core/column/Column.h"
#include "backend/spreadsheet/Spreadsheet.h"
#include "frontend/spreadsheet/SpreadsheetView.h"
#include <QPainter>
#include <QHelpEvent>
#include <QToolTip>

/*!
 * \class SpreadsheetCommentsHeaderView
 * \brief Slave header for SpreadsheetDoubleHeaderView
 *
 * This class is only to be used by SpreadsheetDoubleHeaderView.
 * It allows for displaying two horizontal headers in a SpreadsheetView.
 * A SpreadsheetCommentsHeaderView displays the column comments
 * in a second header below the normal header. It is completely
 * controlled by a SpreadsheetDoubleHeaderView object and thus has
 * a master-slave relationship to it. This would be an inner class
 * of SpreadsheetDoubleHeaderView if Qt allowed this.
 * \ingroup frontend
 */
SpreadsheetCommentsHeaderView::SpreadsheetCommentsHeaderView(QWidget* parent)
	: QHeaderView(Qt::Horizontal, parent) {
}

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
 * \class SpreadsheetSparkLinesHeaderView
 * \brief Slave header for SpreadsheetDoubleHeaderView
 *
 * This class is only to be used by SpreadsheetDoubleHeaderView.
 * It allows for displaying two horizontal headers in a SpreadsheetView.
 * A SpreadsheetSparkLinesHeaderView displays the column comments
 * in a second header below the normal header. It is completely
 * controlled by a SpreadsheetDoubleHeaderView object and thus has
 * a master-slave relationship to it. This would be an inner class
 * of SpreadsheetDoubleHeaderView if Qt allowed this.
 * \ingroup frontend
 */
SpreadsheetSparkLineHeaderView::SpreadsheetSparkLineHeaderView(QWidget* parent)
	: QHeaderView(Qt::Horizontal, parent) {
}

SpreadsheetSparkLineHeaderView::~SpreadsheetSparkLineHeaderView() {
	delete model();
}
QSize SpreadsheetSparkLineHeaderView::sizeHint() const {
	QSize sizeHint = QHeaderView::sizeHint();
	sizeHint.setHeight(30);
	return sizeHint;
}

void SpreadsheetSparkLineHeaderView::setModel(QAbstractItemModel* model) {
	Q_ASSERT(model->inherits("SpreadsheetModel"));
	delete QHeaderView::model();
	auto* newModel = new SpreadsheetSparkLinesHeaderModel(static_cast<SpreadsheetModel*>(model));
	QHeaderView::setModel(newModel);
}

SpreadsheetSparkLinesHeaderModel* SpreadsheetSparkLineHeaderView::getModel() const {
	return static_cast<SpreadsheetSparkLinesHeaderModel*>(model());
}

/*!
 * \class SpreadsheetHeaderView
 * \brief Horizontal header for SpreadsheetView displaying sparkline in second header and comments in a third header
 *
 * This class is only to be used by SpreadsheetView.
 * It allows for displaying 3 horizontal headers.
 * A \c SpreadsheetHeaderView displays the column name, plot designation, and
 * type icon in a normal QHeaderView and below that a second header
 * which displays the column sparkline and third header showing column comments.
 *
 * \sa SpreadsheetCommentsHeaderView
 * \sa SpreadsheetSparklineHeaderView
 * \sa QHeaderView
 * \ingroup frontend
*/
SpreadsheetHeaderView::SpreadsheetHeaderView(QWidget* parent, const Spreadsheet* spreadsheet)
	: QHeaderView(Qt::Horizontal, parent)
	, m_spreadsheet(spreadsheet)
	, m_commentSlave(new SpreadsheetCommentsHeaderView())
	, m_sparklineSlave(new SpreadsheetSparkLineHeaderView()) {
	setDefaultAlignment(Qt::AlignLeft | Qt::AlignVCenter);
	m_commentSlave->setDefaultAlignment(Qt::AlignCenter | Qt::AlignVCenter);
	m_sparklineSlave->setDefaultAlignment(Qt::AlignCenter | Qt::AlignVCenter);
}

SpreadsheetHeaderView::~SpreadsheetHeaderView() {
	delete m_commentSlave;
	delete m_sparklineSlave;
}

QSize SpreadsheetHeaderView::sizeHint() const {
	QSize masterSize = QHeaderView::sizeHint();
	int totalHeight =
		masterSize.height() + (m_showSparklines ? m_sparklineSlave->sizeHint().height() : 0) + (m_showComments ? m_commentSlave->sizeHint().height() : 0);

	return QSize(masterSize.width(), totalHeight);
}

void SpreadsheetHeaderView::setModel(QAbstractItemModel* model) {
	Q_ASSERT(model->inherits("SpreadsheetModel"));
	m_commentSlave->setModel(model);
	m_sparklineSlave->setModel(model);
	QHeaderView::setModel(model);
	connect(model, &QAbstractItemModel::headerDataChanged, this, &SpreadsheetHeaderView::headerDataChanged);
	connect(model, &QAbstractItemModel::headerDataChanged, this, &SpreadsheetHeaderView::refresh);
}

SpreadsheetSparkLinesHeaderModel* SpreadsheetHeaderView::model() const {
	return static_cast<SpreadsheetSparkLinesHeaderModel*>(m_sparklineSlave->model());
}

void SpreadsheetHeaderView::paintSection(QPainter* painter, const QRect& rect, int logicalIndex) const {
	QRect master_rect = rect;
	const auto* model = m_sparklineSlave->getModel();
	auto pixmap = model->headerData(logicalIndex, Qt::Horizontal, static_cast<int>(SpreadsheetModel::CustomDataRole::SparkLineRole)).value<QPixmap>();

	// Handle header painting for different sections
	if (m_showComments && m_showSparklines) {
		int totalHeight = m_commentSlave->sizeHint().height() + m_sparklineSlave->sizeHint().height();
		master_rect = rect.adjusted(0, 0, 0, -totalHeight);
	} else if (m_showComments || m_showSparklines) {
		if (m_showComments)
			master_rect = rect.adjusted(0, 0, 0, -m_commentSlave->sizeHint().height());
		else
			master_rect = rect.adjusted(0, 0, 0, -m_sparklineSlave->sizeHint().height());
	}

	painter->save();
	QHeaderView::paintSection(painter, master_rect, logicalIndex);
	painter->restore();

	if (m_showComments && m_showSparklines && rect.height() > QHeaderView::sizeHint().height()) {
		if (m_showSparklines) {
			QRect slave2_rect = rect.adjusted(0, QHeaderView::sizeHint().height(), 0, -m_commentSlave->sizeHint().height());
			painter->setClipping(false);
			if (!pixmap.isNull())
				painter->drawPixmap(slave2_rect, pixmap.scaled(slave2_rect.size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
		}

		if (m_showComments) {
			QRect slave_rect = rect.adjusted(0, m_sparklineSlave->sizeHint().height() + QHeaderView::sizeHint().height(), 0, 0);
			m_commentSlave->paintSection(painter, slave_rect, logicalIndex);
		}
		return;
	}

	// If only one of the headers (sparkline or comment) is shown
	if (m_showComments || m_showSparklines) {
		if (m_showComments) {
			QRect slave_rect = rect.adjusted(0, QHeaderView::sizeHint().height(), 0, 0);
			m_commentSlave->paintSection(painter, slave_rect, logicalIndex);
		} else if (m_showSparklines) {
			QRect slave_rect = rect.adjusted(0, m_sparklineSlave->sizeHint().height(), 0, 0);
			painter->setClipping(false);
			if (!pixmap.isNull())
				painter->drawPixmap(slave_rect, pixmap.scaled(slave_rect.size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
		}
	}
}

/*!
  Show or hide (if \c on = \c false) the column comments.
*/
void SpreadsheetHeaderView::showComments(bool on) {
	m_showComments = on;
	refresh();
}

/*!
  Returns whether spark lines are shown currently or not.
*/
void SpreadsheetHeaderView::showSparklines(bool on) {
	m_showSparklines = on;
	if (on) {
		m_sparklineSlave->getModel()->spreadsheetModel()->spreadsheet()->setShowSparklines(true);
		if (!m_sparklineCalled) {
			Q_EMIT sparklineToggled();
			m_sparklineCalled = true;
		}
	}
	refresh();
}

void SpreadsheetHeaderView::refresh() {
	int width = sectionSize(count() - 1);

	m_commentSlave->setStretchLastSection(true);
	m_commentSlave->updateGeometry();
	m_commentSlave->setStretchLastSection(false);

	m_sparklineSlave->setStretchLastSection(true);
	m_sparklineSlave->updateGeometry();
	m_sparklineSlave->setStretchLastSection(false);
	int totalHeight = QHeaderView::sizeHint().height();
	if (m_showSparklines)
		totalHeight += m_sparklineSlave->sizeHint().height();
	if (m_showComments)
		totalHeight += m_commentSlave->sizeHint().height();

	QSize sizeHint = QHeaderView::sizeHint();
	sizeHint.setHeight(totalHeight);

	setMinimumHeight(sizeHint.height());
	setMaximumHeight(sizeHint.height());
	setStretchLastSection(true);
	updateGeometry();
	setStretchLastSection(false);

	resizeSection(count() - 1, width);
	update(); // Ensure the header view is updated after resizing
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

bool SpreadsheetHeaderView::viewportEvent(QEvent* e) {
	if (e->type() == QEvent::ToolTip) {
		auto* helpEvent = static_cast<QHelpEvent*>(e);
		const int logicalIndex = logicalIndexAt(helpEvent->pos());
		auto* col = m_spreadsheet->column(logicalIndex);
		if (col)
			QToolTip::showText(helpEvent->globalPos(), col->caption(), this);
		return true;
	}
	return QHeaderView::viewportEvent(e);
}
