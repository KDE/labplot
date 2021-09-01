/*
    File                 : ResizableTextEdit.h
    Project              : LabPlot
    Description          : Extended TextEdit to allow the manual resize by the user
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2018 Alexander Semke (alexander.semke@web.de)

*/

/***************************************************************************
 *                                                                         *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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
	QString text() const;

protected:
	void resizeEvent(QResizeEvent*) override;

private:
	GrabBar* m_grabBar;
	QSize m_size;
	bool m_vertResizeOnly;
};

#endif
