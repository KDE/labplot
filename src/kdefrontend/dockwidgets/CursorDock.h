/*
    File                 : CursorDock.cpp
    Project              : LabPlot
    Description 	     : This dock represents the data from the cursors in the cartesian plots
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2019 Martin Marmsoler <martin.marmsoler@gmail.com>
    SPDX-FileCopyrightText: 2019-2020 Alexander Semke <alexander.semke@web.de>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef CURSORDOCK_H
#define CURSORDOCK_H

#include <QWidget>

namespace Ui {
class CursorDock;
}
class Worksheet;
class CartesianPlot;

/*!
 * \brief The CursorDock class
 * This class represents the data from the cursors from the cartesian plots in a treeview
 */
class CursorDock : public QWidget {
	Q_OBJECT

public:
	explicit CursorDock(QWidget* parent = nullptr);
	~CursorDock();
	void setWorksheet(Worksheet*);

public Q_SLOTS:
	void plotCursor0EnableChanged(bool);
	void plotCursor1EnableChanged(bool);

private:
	void collapseAll();
	void expandAll();
	void cursor0EnableChanged(bool);
	void cursor1EnableChanged(bool);
	bool eventFilter(QObject*, QEvent*) override;

	Ui::CursorDock* ui;
	bool m_initializing{false};
	QVector<CartesianPlot*> m_plotList;
	CartesianPlot* m_plot{nullptr};
	QList<QMetaObject::Connection> selectedPlotsConnection;

private Q_SLOTS:
	void contextMenuRequested(QPoint);
	void resultCopy();
	void resultCopyAll();
};

#endif // CURSORDOCK_H
