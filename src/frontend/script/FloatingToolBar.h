/*
	File                 : FloatingToolBar.h
	Project              : LabPlot
	Description          : Toolbar floating on top of another widget, movable horizontally
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2026 Martin Marmsoler <martin.marmsoler@gmail.com>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef FLOATINGTOOLBAR_H
#define FLOATINGTOOLBAR_H

#include <QFrame>

class QToolBar;

class FloatingToolBar : public QFrame {
	Q_OBJECT

public:
	explicit FloatingToolBar(QWidget* parent, const QString& configKey);

	void addAction(QAction*);
	void addSeparator();

	double position() const;
	void setPosition(double);

protected:
	bool eventFilter(QObject*, QEvent*) override;
	void mousePressEvent(QMouseEvent*) override;
	void mouseMoveEvent(QMouseEvent*) override;
	void mouseReleaseEvent(QMouseEvent*) override;

private:
	QToolBar* m_toolBar{nullptr};
	QString m_configKey;
	double m_position{0.5}; // horizontal position relative to the free space in the parent, 0 = left, 1 = right
	int m_dragOffset{-1}; // x-offset of the mouse in the toolbar while dragging, -1 if not dragging

	void updatePlacement();
};

#endif
