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

SpreadsheetCommentsHeaderView::SpreadsheetCommentsHeaderView(QWidget* parent) : QHeaderView(Qt::Horizontal, parent) {
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
SpreadsheetHeaderView::SpreadsheetHeaderView(QWidget* parent) : QHeaderView(Qt::Horizontal, parent) {
	setDefaultAlignment(Qt::AlignLeft | Qt::AlignVCenter);
	m_slave = new SpreadsheetCommentsHeaderView();
	m_slave->setDefaultAlignment(Qt::AlignLeft | Qt::AlignVCenter);
	m_showComments = true;
}

SpreadsheetHeaderView::~SpreadsheetHeaderView() {
	delete m_slave;
}

QSize SpreadsheetHeaderView::sizeHint() const {
	QSize master_size = QHeaderView::sizeHint();
	if (m_showComments)
		master_size.setHeight(master_size.height() + m_slave->sizeHint().height());
	else
		master_size.setHeight(master_size.height());

	return master_size;
}

void SpreadsheetHeaderView::setModel(QAbstractItemModel* model) {
	Q_ASSERT(model->inherits("SpreadsheetModel"));
	m_slave->setModel(model);
	QHeaderView::setModel(model);
	connect(model, &QAbstractItemModel::headerDataChanged, this, &SpreadsheetHeaderView::headerDataChanged);
}

void SpreadsheetHeaderView::paintSection(QPainter* painter, const QRect& rect, int logicalIndex) const {
	QRect master_rect = rect;
	if (m_showComments)
		master_rect = rect.adjusted(0, 0, 0, -m_slave->sizeHint().height());

	QHeaderView::paintSection(painter, master_rect, logicalIndex);
	if (m_showComments && rect.height() > QHeaderView::sizeHint().height()) {
		QRect slave_rect = rect.adjusted(0, QHeaderView::sizeHint().height(), 0, 0);
		m_slave->paintSection(painter, slave_rect, logicalIndex);
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
  adjust geometry and repaint header .
*/
void SpreadsheetHeaderView::refresh() {
	//TODO
	// adjust geometry and repaint header (still looking for a more elegant solution)
	int width = sectionSize(count()-1);
	m_slave->setStretchLastSection(true);  // ugly hack (flaw in Qt? Does anyone know a better way?)
	m_slave->updateGeometry();
	m_slave->setStretchLastSection(false); // ugly hack part 2
	setStretchLastSection(true);  // ugly hack (flaw in Qt? Does anyone know a better way?)
	updateGeometry();
	setStretchLastSection(false); // ugly hack part 2
	resizeSection(count()-1, width);

	update();
}

/*!
  Reacts to a header data change.
*/
void SpreadsheetHeaderView::headerDataChanged(Qt::Orientation orientation, int /*logicalFirst*/, int /*logicalLast*/) {
	if (orientation == Qt::Horizontal)
		refresh();
}
