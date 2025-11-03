/*
	File                 : TreeViewComboBox.h
	Project              : LabPlot
	Description          : Provides a QTreeView in a QComboBox
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2008-2016 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef TREEVIEWCOMBOBOX_H
#define TREEVIEWCOMBOBOX_H

#include <QComboBox>

class AbstractAspect;
class AbstractColumn;
class AspectTreeModel;
class Matrix;
class QGroupBox;
class QLineEdit;
class QTreeView;

enum class AspectType : quint64;

class TreeViewComboBox : public QComboBox {
	Q_OBJECT

public:
	explicit TreeViewComboBox(QWidget* parent = nullptr);

	void setModel(AspectTreeModel*);
	void setModel(QAbstractItemModel*) override;
	void setCurrentModelIndex(const QModelIndex&);
	void setAspect(const AbstractAspect*);
	void setAspect(const AbstractAspect* aspect, const QString& path);
	void setMatrix(const Matrix*, const QString&);
	AbstractAspect* currentAspect() const;
	QModelIndex currentModelIndex() const;

	void setTopLevelClasses(const QList<AspectType>&);
	void setHiddenAspects(const QList<const AbstractAspect*>&);

	void showPopup() override;
	void hidePopup() override;
	void setInvalid(bool invalid, const QString& tooltip = QString());

	QString currentText() const;
	void setText(const QString& text);

	static QList<AspectType> plotColumnTopLevelClasses();

private:
	AspectTreeModel* m_model{nullptr};
	QTreeView* m_treeView;
	QGroupBox* m_groupBox;
	QLineEdit* m_lineEdit;
	QString m_currentText{QLatin1String("")};

	QList<AspectType> m_topLevelClasses;
	QList<const char*> m_selectableClasses;
	QList<const AbstractAspect*> m_hiddenAspects;

	void showTopLevelOnly(const QModelIndex&);
	bool eventFilter(QObject*, QEvent*) override;
	bool filter(const QModelIndex&, const QString&);
	bool isTopLevel(const AbstractAspect*) const;
	bool isHidden(const AbstractAspect*) const;

	void paintEvent(QPaintEvent*) override;

private Q_SLOTS:
	void treeViewIndexActivated(const QModelIndex&);
	void filterChanged(const QString&);

Q_SIGNALS:
	void currentModelIndexChanged(const QModelIndex&);
};

#endif
