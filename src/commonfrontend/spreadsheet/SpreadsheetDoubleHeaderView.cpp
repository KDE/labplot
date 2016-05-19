/***************************************************************************
    File                 : SpreadsheetDoubleHeaderView.cpp
    Project              : LabPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2007 by Tilman Benkert (thzs@gmx.net)
    Email (use @ for *)  : thzs*gmx.net
    Description          : Horizontal header for SpreadsheetView displaying comments in a second header

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *  This program is free software; you can redistribute it and/or modify   *
 *  it under the terms of the GNU General Public License as published by   *
 *  the Free Software Foundation; either version 2 of the License, or      *
 *  (at your option) any later version.                                    *
 *                                                                         *
 *  This program is distributed in the hope that it will be useful,        *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *  GNU General Public License for more details.                           *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the Free Software           *
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor,                    *
 *   Boston, MA  02110-1301  USA                                           *
 *                                                                         *
 ***************************************************************************/

#include "SpreadsheetDoubleHeaderView.h"
#include "SpreadsheetCommentsHeaderModel.h"

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

SpreadsheetCommentsHeaderView::SpreadsheetCommentsHeaderView(QWidget *parent) 
	: QHeaderView(Qt::Horizontal, parent){
}

SpreadsheetCommentsHeaderView::~SpreadsheetCommentsHeaderView(){	
	delete model();
}

void SpreadsheetCommentsHeaderView::setModel(QAbstractItemModel * model){
	Q_ASSERT(model->inherits("SpreadsheetModel"));
	QAbstractItemModel *old_model = QHeaderView::model();
	SpreadsheetCommentsHeaderModel *new_model = new SpreadsheetCommentsHeaderModel(static_cast<SpreadsheetModel *>(model));
	QHeaderView::setModel(new_model);
    QObject::disconnect(new_model, SIGNAL(columnsInserted(QModelIndex,int,int)),
   		this, SLOT(sectionsInserted(QModelIndex,int,int))); // We have to make sure sectionsInserted is called in the right order
	delete old_model;
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
SpreadsheetDoubleHeaderView::SpreadsheetDoubleHeaderView(QWidget * parent) 
: QHeaderView(Qt::Horizontal, parent){ 
	setDefaultAlignment(Qt::AlignLeft | Qt::AlignTop);
	m_slave = new SpreadsheetCommentsHeaderView(); 
	m_slave->setDefaultAlignment(Qt::AlignLeft | Qt::AlignTop);
	m_showComments = true;
}

SpreadsheetDoubleHeaderView::~SpreadsheetDoubleHeaderView(){
	delete m_slave;
}

QSize SpreadsheetDoubleHeaderView::sizeHint() const{
	QSize master_size = QHeaderView::sizeHint();
	master_size.setHeight(master_size.height() + 5);
	if(m_showComments)
		master_size.setHeight(master_size.height() + m_slave->sizeHint().height());
	return master_size;
}

void SpreadsheetDoubleHeaderView::setModel(QAbstractItemModel * model){
	Q_ASSERT(model->inherits("SpreadsheetModel"));
	m_slave->setModel(model);
	QHeaderView::setModel(model);
	connect(model, SIGNAL(headerDataChanged(Qt::Orientation,int,int)), this, SLOT(headerDataChanged(Qt::Orientation,int,int)));
}

void SpreadsheetDoubleHeaderView::paintSection ( QPainter * painter, const QRect & rect, int logicalIndex ) const{
	QRect master_rect = rect;
	if(m_showComments)
		master_rect = rect.adjusted(0, 0, 0, -m_slave->sizeHint().height());
	QHeaderView::paintSection(painter, master_rect, logicalIndex);
	if(m_showComments && rect.height() > QHeaderView::sizeHint().height()) 
	{
		QRect slave_rect = rect.adjusted(0, QHeaderView::sizeHint().height(), 0, 0);
		m_slave->paintSection(painter, slave_rect, logicalIndex);
	}
}

/*!
  Returns whether comments are shown currently or not.
*/
bool SpreadsheetDoubleHeaderView::areCommentsShown() const{
	return m_showComments;
}

/*!
  Show or hide (if \c on = \c false) the column comments.
*/
void SpreadsheetDoubleHeaderView::showComments(bool on){
	m_showComments = on;
	refresh();
}

/*!
  adjust geometry and repaint header .
*/
void SpreadsheetDoubleHeaderView::refresh(){
  //TODO
	// adjust geometry and repaint header (still looking for a more elegant solution)
	m_slave->setStretchLastSection(true);  // ugly hack (flaw in Qt? Does anyone know a better way?)
	m_slave->updateGeometry();
	m_slave->setStretchLastSection(false); // ugly hack part 2
	setStretchLastSection(true);  // ugly hack (flaw in Qt? Does anyone know a better way?)
	updateGeometry();
	setStretchLastSection(false); // ugly hack part 2
	update();
}

/*!
  Reacts to a header data change.
*/
void SpreadsheetDoubleHeaderView::headerDataChanged(Qt::Orientation orientation, int logicalFirst, int logicalLast){	
	Q_UNUSED(logicalFirst);
	Q_UNUSED(logicalLast);
	if(orientation == Qt::Horizontal)
		refresh();
}
		
void SpreadsheetDoubleHeaderView::sectionsInserted(const QModelIndex & parent, int logicalFirst, int logicalLast ){
	m_slave->sectionsInserted(parent, logicalFirst, logicalLast);
	QHeaderView::sectionsInserted(parent, logicalFirst, logicalLast);
	Q_ASSERT(m_slave->count() == QHeaderView::count());
}
