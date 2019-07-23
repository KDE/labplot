/***************************************************************************
File                 : CursorDock.cpp
Project              : LabPlot
Description 	     : This dock represents the data from the cursors in the cartesian plots
--------------------------------------------------------------------
Copyright            : (C) 2019 Martin Marmsoler (martin.marmsoler@gmail.com)

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

public slots:
	void plotCursor0EnableChanged(bool);
	void plotCursor1EnableChanged(bool);

private:
	void collapseAll();
	void expandAll();
	void cursor0EnableChanged(bool);
	void cursor1EnableChanged(bool);

	Ui::CursorDock* ui;

private:
	bool m_initializing{false};
	QVector<CartesianPlot*> m_plotList;
	CartesianPlot* m_plot{nullptr};
};

#endif // CURSORDOCK_H
