/***************************************************************************
    File                 : TreeViewComboBox.h
    Project              : LabPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2008-2011 by Alexander Semke (alexander.semke*web.de)
    Copyright            : (C) 2008 Tilman Benkert (thzs*gmx.net)
                           (replace * with @ in the email addresses)
    Description          : Provides a QTreeView in a QComboBox

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

#ifndef TREEVIEWCOMBOBOX_H
#define TREEVIEWCOMBOBOX_H

#include <QTreeView>
#include <QComboBox>
#include <QEvent>

class AbstractAspect;

class TreeViewComboBox : public QComboBox{
Q_OBJECT

public:
    explicit TreeViewComboBox(QWidget* parent = 0);
    ~TreeViewComboBox();

 	void setModel(QAbstractItemModel *model);
	void setCurrentModelIndex(const QModelIndex&);
	void setTopLevelClasses(QList<const char *>);
	QModelIndex currentModelIndex() const;
	virtual void showPopup();

private:
	QTreeView m_treeView;
	QList<const char *> m_topLevelClasses;
	void showTopLevelOnly(const QModelIndex & index);
	bool eventFilter(QObject *obj, QEvent *event);

private slots:
	void treeViewIndexActivated(const QModelIndex&);
	
signals:
	void currentModelIndexChanged(const QModelIndex&);
};

#endif
