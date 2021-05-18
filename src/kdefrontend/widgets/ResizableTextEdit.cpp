/***************************************************************************
    File                 : ResizableTextEdit.cpp
    Project              : LabPlot
    Description          : Extended TextEdit to allow the manual resize by the user
    --------------------------------------------------------------------
    Copyright            : (C) 2018 Alexander Semke (alexander.semke@web.de)

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
#include "ResizableTextEdit.h"

#include <QApplication>
#include <QMouseEvent>
#include <QPainter>
#include <QStyle>
#include <QStyleOption>

GrabBar::GrabBar(ResizableTextEdit* parent, bool vertResizeOnly) : QWidget(parent),
	m_parent(parent), m_vertResizeOnly(vertResizeOnly) {

	resize(20,10);
}

QSize GrabBar::sizeHint() const {
	return QSize{20, 10};
}

void GrabBar::paintEvent(QPaintEvent*) {
	QPainter p(this);

	//s. qsplitter.cpp
	QStyleOption opt(0);
	opt.rect = rect();
	opt.palette = palette();
	opt.state = QStyle::State_Horizontal;

	if (m_hovered)
		opt.state |= QStyle::State_MouseOver;
	if (m_pressed)
		opt.state |= QStyle::State_Sunken;
	if (isEnabled())
		opt.state |= QStyle::State_Enabled;

	parentWidget()->style()->drawControl(QStyle::CE_Splitter, &opt, &p, this);
}

void GrabBar::mousePressEvent(QMouseEvent* e) {
	if (e->button() == Qt::LeftButton) {
		m_pos = e->pos();
		m_pressed = true;
	}

	e->accept();
}

void GrabBar::mouseReleaseEvent(QMouseEvent* e) {
	if (e->button() == Qt::LeftButton)
		m_pressed = false;

	e->accept();
}

void GrabBar::mouseMoveEvent(QMouseEvent* e) {
	if (m_pressed) {
		const QPoint delta = e->pos() - m_pos;
		if (m_vertResizeOnly)
			m_parent->addSize( QSize( 0, delta.y() ) );
		else
			m_parent->addSize( QSize( delta.x(), delta.y() ) );
	}

	e->accept();
}

void GrabBar::enterEvent(QEvent* e) {
	if (isEnabled()) {
		if (m_vertResizeOnly)
			QApplication::setOverrideCursor(QCursor(Qt::SizeVerCursor));
		else
			QApplication::setOverrideCursor(QCursor(Qt::SizeFDiagCursor));

		m_hovered = true;
	}

	e->accept();
}

void GrabBar::leaveEvent(QEvent* e) {
	QApplication::restoreOverrideCursor();
	m_hovered = false;
	e->accept();
}

/**
 * \class ResizableTextEdit
 * \brief Extended QTextEdit supporting the manual resize by the user.
 *
 * \ingroup frontend/widgets
 */
ResizableTextEdit::ResizableTextEdit(QWidget* parent, bool vertResizeOnly) : QTextEdit(parent),
	m_grabBar(new GrabBar(this, vertResizeOnly)),
	m_size( QTextEdit::sizeHint() ),
	m_vertResizeOnly(vertResizeOnly) {

}

void ResizableTextEdit::addSize(QSize size) {
	if (m_size.height() + size.height() < minimumHeight())
		return;

	m_size = QSize(m_size.width(), m_size.height() + size.height());
	updateGeometry();
	setMaximumHeight(m_size.height());
}

QSize ResizableTextEdit::sizeHint() const {
	return m_size;
}

QString ResizableTextEdit::text() const {
	return document()->toPlainText();
}

void ResizableTextEdit::resizeEvent(QResizeEvent* e) {
	if (m_vertResizeOnly)
		m_grabBar->move(width()/2 - m_grabBar->width()/2, height() - m_grabBar->height());
	else
		m_grabBar->move(width() - m_grabBar->width(), height() - m_grabBar->height());

	m_size = e->size();

	QTextEdit::resizeEvent(e);

	e->accept();
}
