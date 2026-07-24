/*
	File                 : DataColumnsWidget.h
	Project              : LabPlot
	Description          : widget to handle multiple data source columns
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2025 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef DATACOLUMNWIDGET_H
#define DATACOLUMNWIDGET_H

#include <QWidget>

class AbstractColumn;
class AspectTreeModel;
class TreeViewComboBox;
class QPushButton;
class QGridLayout;

class DataColumnsWidget : public QWidget {
	Q_OBJECT

public:
	explicit DataColumnsWidget(QWidget*);

	void setDataColumns(const QVector<const AbstractColumn*>&, const QVector<QString>&,  AspectTreeModel*);
	void setEnabled(bool);

private:
	bool m_initializing{false};
	AspectTreeModel* m_model{nullptr};
	QGridLayout* m_gridLayout;
	QPushButton* m_buttonNew;
	QVector<TreeViewComboBox*> m_dataComboBoxes;
	QVector<QPushButton*> m_removeButtons;

	void addDataColumn();
	void removeDataColumn();
	void updateDataColumns();

Q_SIGNALS:
	void dataColumnsChanged(QVector<const AbstractColumn*>);

private Q_SLOTS:
	void dataColumnChanged(const QModelIndex&);
};

#endif // DATACOLUMNSWIDGET_H
