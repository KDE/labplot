/***************************************************************************
    File                 : TreeViewComboBox.h
    Project              : LabPlot
    Description          : Provides a QTreeView in a QComboBox
    --------------------------------------------------------------------
    Copyright            : (C) 2008-2016 by Alexander Semke (alexander.semke@web.de)

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

#include <QComboBox>

class AbstractAspect;
class AbstractColumn;
class AspectTreeModel;
class QGroupBox;
class QLineEdit;
class QTreeView;

enum class AspectType : quint64;

class TreeViewComboBox : public QComboBox {
	Q_OBJECT

public:
	explicit TreeViewComboBox(QWidget* parent = nullptr);

	void setModel(AspectTreeModel*);
	void setCurrentModelIndex(const QModelIndex&);
	void setAspect(const AbstractAspect*);
	void setColumn(const AbstractColumn*, const QString&);
	QModelIndex currentModelIndex() const;

	void setTopLevelClasses(const QList<AspectType>&);
	void setHiddenAspects(const QList<const AbstractAspect*>&);

	void showPopup() override;
	void hidePopup() override;
	void setInvalid(bool invalid, const QString& tooltip = QString());

	void useCurrentIndexText(const bool set);

	QString currentText() const;
	void setText(const QString& text);

private:
	AspectTreeModel* m_model{nullptr};
	QTreeView* m_treeView;
	QGroupBox* m_groupBox;
	QLineEdit* m_lineEdit;
	QString m_lineEditText{""};
	bool m_useCurrentIndexText{true};

	QList<AspectType> m_topLevelClasses;
	QList<const char*> m_selectableClasses;
	QList<const AbstractAspect*> m_hiddenAspects;

	void showTopLevelOnly(const QModelIndex&);
	bool eventFilter(QObject*, QEvent*) override;
	bool filter(const QModelIndex&, const QString&);
	bool isTopLevel(const AbstractAspect*) const;
	bool isHidden(const AbstractAspect*) const;

	void paintEvent(QPaintEvent*) override;

private slots:
	void treeViewIndexActivated(const QModelIndex&);
	void filterChanged(const QString&);

signals:
	void currentModelIndexChanged(const QModelIndex&);
};

#endif
