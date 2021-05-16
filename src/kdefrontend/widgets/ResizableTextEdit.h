/***************************************************************************
    File                 : ResizableTextEdit.h
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
#ifndef RESIZABLETEXTEDIT_H
#define RESIZABLETEXTEDIT_H

#include <QTextEdit>

class ResizableTextEdit;

class GrabBar : public QWidget {
	Q_OBJECT

public:
	GrabBar(ResizableTextEdit*, bool vertResizeOnly);
	QSize sizeHint() const override;

protected:
	void paintEvent(QPaintEvent*) override;
	void mousePressEvent(QMouseEvent*) override;
	void mouseReleaseEvent(QMouseEvent*) override;
	void mouseMoveEvent(QMouseEvent*) override;
	void enterEvent(QEvent*) override;
	void leaveEvent(QEvent*) override;

private:
	ResizableTextEdit* m_parent;
	QPoint m_pos;
	bool m_pressed{false};
	bool m_hovered{false};
	bool m_vertResizeOnly;
};

class ResizableTextEdit : public QTextEdit {
	Q_OBJECT
public:
	explicit ResizableTextEdit(QWidget*, bool vertResizeOnly = true);
	void addSize(QSize);
	QSize sizeHint() const override;

protected:
	void resizeEvent(QResizeEvent*) override;

private:
	GrabBar* m_grabBar;
	QSize m_size;
	bool m_vertResizeOnly;
};

#endif
