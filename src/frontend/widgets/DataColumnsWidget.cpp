/*
	File                 : DataColumnsWidget.cpp
	Project              : LabPlot
	Description          : widget to handle multiple data source columns
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2025 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "DataColumnsWidget.h"
#include "backend/core/AbstractColumn.h"
#include "backend/core/AspectTreeModel.h"
#include "backend/lib/macros.h"
#include "frontend/widgets/TreeViewComboBox.h"

#include <QPushButton>
#include <QGridLayout>

/*!
	\class DataColumnsWidget
	\brief Widget to manage combo-boxes for columns that are used in plots that support multiple
	data source columns like box plot, etc. Used in the dock widgets of the corresponding plots.

	\ingroup frontend
 */
DataColumnsWidget::DataColumnsWidget(QWidget* parent) : QWidget(parent) {
	m_gridLayout = new QGridLayout(parent);
	m_gridLayout->setContentsMargins(0, 0, 0, 0);
	m_gridLayout->setHorizontalSpacing(2);
	m_gridLayout->setVerticalSpacing(2);
	setLayout(m_gridLayout);

	m_buttonNew = new QPushButton();
	m_buttonNew->setIcon(QIcon::fromTheme(QStringLiteral("list-add")));
	connect(m_buttonNew, &QPushButton::clicked, this, &DataColumnsWidget::addDataColumn);
}

void DataColumnsWidget::setEnabled(bool enabled) {
	m_buttonNew->setVisible(enabled);
	for (auto* cb : m_dataComboBoxes)
		cb->setEnabled(enabled);
	for (auto* b : m_removeButtons)
		b->setVisible(enabled);
}

void DataColumnsWidget::setDataColumns(const QVector<const AbstractColumn*>& columns, const QVector<QString>& columnPaths,  AspectTreeModel* model) {
	CONDITIONAL_LOCK_RETURN;

	m_model = model;

	// add the combobox for the first column, is always present
	if (m_dataComboBoxes.count() == 0)
		addDataColumn();

	int count = columns.count();

	if (count != 0) {
		// data columns are present, make sure we have the proper number of comboboxes
		int diff = count - m_dataComboBoxes.count();
		if (diff > 0) {
			for (int i = 0; i < diff; ++i)
				addDataColumn();
		} else if (diff < 0) {
			for (int i = diff; i != 0; ++i)
				removeDataColumn();
		}

		// show the columns in the comboboxes
		for (int i = 0; i < count; ++i) {
			m_dataComboBoxes.at(i)->setModel(model); // the model might have changed in-between, reset the current model
			m_dataComboBoxes.at(i)->setAspect(columns.at(i), columnPaths.at(i));
		}
	} else {
		// no data columns set yet, we show the first combo box only and reset its model
		m_dataComboBoxes.first()->setModel(model);
		m_dataComboBoxes.first()->setAspect(nullptr);
		for (int i = 1; i < m_dataComboBoxes.count(); ++i)
			removeDataColumn();
	}
}

void DataColumnsWidget::addDataColumn() {
	auto* cb = new TreeViewComboBox(this);
	cb->setTopLevelClasses(TreeViewComboBox::plotColumnTopLevelClasses());
	cb->setModel(m_model);
	connect(cb, &TreeViewComboBox::currentModelIndexChanged, this, &DataColumnsWidget::dataColumnChanged);

	const int index = m_dataComboBoxes.size();

	if (index == 0) {
		QSizePolicy sizePolicy1(QSizePolicy::Expanding, QSizePolicy::Preferred);
		sizePolicy1.setHorizontalStretch(0);
		sizePolicy1.setVerticalStretch(0);
		sizePolicy1.setHeightForWidth(cb->sizePolicy().hasHeightForWidth());
		cb->setSizePolicy(sizePolicy1);
	} else {
		auto* button = new QPushButton();
		button->setIcon(QIcon::fromTheme(QStringLiteral("list-remove")));
		connect(button, &QPushButton::clicked, this, &DataColumnsWidget::removeDataColumn);
		m_gridLayout->addWidget(button, index, 1, 1, 1);
		m_removeButtons << button;
	}

	m_gridLayout->addWidget(cb, index, 0, 1, 1);
	m_gridLayout->addWidget(m_buttonNew, index + 1, 1, 1, 1);

	m_dataComboBoxes << cb;
}

void DataColumnsWidget::removeDataColumn() {
	const auto* sender = static_cast<QPushButton*>(QObject::sender());
	if (sender) {
		// remove button was clicked, determine which one and
		// delete it together with the corresponding combobox
		for (int i = 0; i < m_removeButtons.count(); ++i) {
			if (sender == m_removeButtons.at(i)) {
				delete m_dataComboBoxes.takeAt(i + 1);
				delete m_removeButtons.takeAt(i);
			}
		}
	} else {
		// no sender is available, the function is being called directly in loadDataColumns().
		// delete the last remove button together with the corresponding combobox
		int index = m_removeButtons.count() - 1;
		if (index >= 0) {
			delete m_dataComboBoxes.takeAt(index + 1);
			delete m_removeButtons.takeAt(index);
		}
	}

	if (!m_initializing)
		updateDataColumns();
}

void DataColumnsWidget::dataColumnChanged(const QModelIndex&) {
	CONDITIONAL_LOCK_RETURN;
	updateDataColumns();
}

void DataColumnsWidget::updateDataColumns() {
	QVector<const AbstractColumn*> columns;

	for (auto* cb : m_dataComboBoxes) {
		auto* aspect = cb->currentAspect();
		if (aspect && aspect->type() == AspectType::Column)
			columns << static_cast<AbstractColumn*>(aspect);
	}

	Q_EMIT dataColumnsChanged(columns);
}
