/*
	File                 : SpreadsheetHeaderView.cpp
	Project              : LabPlot
	Description          : Horizontal header for SpreadsheetView displaying comments in a second header
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2016-2024 Alexander Semke <alexander.semke@web.de>
	SPDX-FileCopyrightText: 2007 Tilman Benkert <thzs@gmx.net>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "SpreadsheetHeaderView.h"
#include "SpreadsheetCommentsHeaderModel.h"
#include "SpreadsheetSparkLineHeaderModel.h"
#include "backend/spreadsheet/Spreadsheet.h"
#include <QPainter>

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

\ingroup frontend
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
 \class SpreadsheetSparkLinesHeaderView
 \brief Slave header for SpreadsheetDoubleHeaderView

This class is only to be used by SpreadsheetDoubleHeaderView.
It allows for displaying two horizontal headers in a SpreadsheetView.
A SpreadsheetSparkLinesHeaderView displays the column comments
in a second header below the normal header. It is completely
controlled by a SpreadsheetDoubleHeaderView object and thus has
a master-slave relationship to it. This would be an inner class
of SpreadsheetDoubleHeaderView if Qt allowed this.

\ingroup frontend
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
	auto* new_model = new SpreadsheetSparkLinesHeaderModel(static_cast<SpreadsheetModel*>(model));
	QHeaderView::setModel(new_model);
}

SpreadsheetSparkLinesHeaderModel* SpreadsheetSparkLineHeaderView::getModel() const {
	return static_cast<SpreadsheetSparkLinesHeaderModel*>(model());
}

/*!
 \class SpreadsheetHeaderView
 \brief Horizontal header for SpreadsheetView displaying sparkline in second header and comments in a third header

This class is only to be used by SpreadsheetView.
It allows for displaying 3 horizontal headers.
A \c SpreadsheetHeaderView displays the column name, plot designation, and
type icon in a normal QHeaderView and below that a second header
which displays the column sparkline and third header showing column comments.

\sa SpreadsheetCommentsHeaderView
\sa SpreadsheetSparklineHeaderView
\sa QHeaderView

\ingroup frontend
*/

SpreadsheetHeaderView::SpreadsheetHeaderView(QWidget* parent)
	: QHeaderView(Qt::Horizontal, parent)
	, m_commentSlave(new SpreadsheetCommentsHeaderView())
	, m_sparkLineSlave(new SpreadsheetSparkLineHeaderView()) {
	setDefaultAlignment(Qt::AlignLeft | Qt::AlignVCenter);
	m_commentSlave->setDefaultAlignment(Qt::AlignCenter | Qt::AlignVCenter);
	m_sparkLineSlave->setDefaultAlignment(Qt::AlignCenter | Qt::AlignVCenter);
}

SpreadsheetHeaderView::~SpreadsheetHeaderView() {
	delete m_commentSlave;
	delete m_sparkLineSlave;
}

QSize SpreadsheetHeaderView::sizeHint() const {
	QSize masterSize = QHeaderView::sizeHint();
	int totalHeight =
		masterSize.height() + (m_showSparkLines ? m_sparkLineSlave->sizeHint().height() : 0) + (m_showComments ? m_commentSlave->sizeHint().height() : 0);

	return QSize(masterSize.width(), totalHeight);
}

void SpreadsheetHeaderView::setModel(QAbstractItemModel* model) {
	Q_ASSERT(model->inherits("SpreadsheetModel"));
	m_commentSlave->setModel(model);
	m_sparkLineSlave->setModel(model);
	QHeaderView::setModel(model);
	connect(model, &QAbstractItemModel::headerDataChanged, this, &SpreadsheetHeaderView::headerDataChanged);
	connect(model, &QAbstractItemModel::headerDataChanged, this, &SpreadsheetHeaderView::refresh);
}

SpreadsheetSparkLinesHeaderModel* SpreadsheetHeaderView::model() const {
	return static_cast<SpreadsheetSparkLinesHeaderModel*>(m_sparkLineSlave->model());
}

void SpreadsheetHeaderView::paintSection(QPainter* painter, const QRect& rect, int logicalIndex) const {
	QRect master_rect = rect;
	auto* model = m_sparkLineSlave->getModel();
	QPixmap pixmap = model->headerData(logicalIndex, Qt::Horizontal, static_cast<int>(SpreadsheetModel::CustomDataRole::SparkLineRole)).value<QPixmap>();
	// initalise header
	if (m_showComments && m_showSparkLines) {
		int totalHeight = m_commentSlave->sizeHint().height() + m_sparkLineSlave->sizeHint().height();
		master_rect = rect.adjusted(0, 0, 0, -totalHeight);
	} else if (m_showComments || m_showSparkLines) {
		if (m_showComments)
			master_rect = rect.adjusted(0, 0, 0, -m_commentSlave->sizeHint().height());
		else
			master_rect = rect.adjusted(0, 0, 0, -m_sparkLineSlave->sizeHint().height());
	}
	painter->save();
	QHeaderView::paintSection(painter, master_rect, logicalIndex);
	painter->restore();
	if (m_showComments && m_showSparkLines && rect.height() > QHeaderView::sizeHint().height()) {
		if (m_showSparkLines && rect.height() > QHeaderView::sizeHint().height()) {
			QRect slave2_rect = rect.adjusted(0, QHeaderView::sizeHint().height(), 0, -m_commentSlave->sizeHint().height());
			painter->setClipping(false); // Disable clipping
			if (pixmap.size().isValid())
				painter->drawPixmap(slave2_rect, pixmap.scaled(slave2_rect.size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
		}
		if (m_showComments && rect.height() > QHeaderView::sizeHint().height()) {
			QRect slave_rect = rect.adjusted(0, m_sparkLineSlave->sizeHint().height() + QHeaderView::sizeHint().height(), 0, 0);
			m_commentSlave->paintSection(painter, slave_rect, logicalIndex);
		}
		return;
	} else if (m_showComments || m_showSparkLines) {
		if (m_showComments && rect.height() > QHeaderView::sizeHint().height()) {
			QRect slave_rect = rect.adjusted(0, QHeaderView::sizeHint().height(), 0, 0);
			m_commentSlave->paintSection(painter, slave_rect, logicalIndex);
			return;
		}
		if (m_showSparkLines && rect.height() > QHeaderView::sizeHint().height()) {
			QRect slave_rect = rect.adjusted(0, m_sparkLineSlave->sizeHint().height(), 0, 0);
			painter->setClipping(false); // Disable clipping
			if (pixmap.size().isValid())
				painter->drawPixmap(slave_rect, pixmap.scaled(slave_rect.size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
			return;
		}
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
	if (on) {
		m_sparkLineSlave->getModel()->spreadsheetModel()->spreadsheet()->isSparklineShown = true;
		if (!m_sparklineCalled) {
			Q_EMIT sparklineToggled();
			m_sparklineCalled = true;
		}
	}
	refresh();
}

void SpreadsheetHeaderView::refresh() {
	// TODO
	// adjust geometry and repaint header (still looking for a more elegant solution)
	int width = sectionSize(count() - 1);
	m_commentSlave->setStretchLastSection(true); // ugly hack (flaw in Qt? Does anyone know a better way?)
	m_commentSlave->updateGeometry();
	m_commentSlave->setStretchLastSection(false); // ugly hack part 2
	m_sparkLineSlave->setStretchLastSection(true); // ugly hack (flaw in Qt? Does anyone know a better way?)
	m_sparkLineSlave->updateGeometry();
	m_sparkLineSlave->setStretchLastSection(false); // ugly hack part 2
	setStretchLastSection(true); // ugly hack (flaw in Qt? Does anyone know a better way?)
	updateGeometry();
	setStretchLastSection(false); // ugly hack part 2
	resizeSection(count() - 1, width);
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
