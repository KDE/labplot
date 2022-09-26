/*
	File                 : FlattenColumnsDialog.cpp
	Project              : LabPlot
	Description          : Dialog for flattening of spreadsheet columns
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2022 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "FlattenColumnsDialog.h"
#include "backend/core/column/Column.h"
#include "backend/lib/macros.h"
#include "backend/spreadsheet/Spreadsheet.h"

#include <QPushButton>
#include <QThreadPool>
#include <QWindow>

#include <KLocalizedString>
#include <KMessageBox>
#include <KWindowConfig>

/*!
	\class FlattenColumnsDialog
	\brief Dialog for flattening of the selected columns in the spreadsheet.

	\ingroup kdefrontend
 */
FlattenColumnsDialog::FlattenColumnsDialog(Spreadsheet* s, QWidget* parent)
	: QDialog(parent)
	, m_spreadsheet(s) {
	ui.setupUi(this);
	setAttribute(Qt::WA_DeleteOnClose);

	m_buttonNew = new QPushButton();
	m_buttonNew->setIcon(QIcon::fromTheme("list-add"));
	m_buttonNew->setToolTip(i18n("Add additional reference column"));
	connect(m_buttonNew, &QPushButton::clicked, this, &FlattenColumnsDialog::addReferenceColumn);

	m_gridLayout = static_cast<QGridLayout*>(ui.gbColumns->layout());
	m_gridLayout->addWidget(m_buttonNew, 1, 2, 1, 1);

	auto* btnBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
	ui.gridLayout->addWidget(btnBox, 3, 1, 1, 2);
	m_okButton = btnBox->button(QDialogButtonBox::Ok);

	connect(btnBox->button(QDialogButtonBox::Cancel), &QPushButton::clicked, this, &FlattenColumnsDialog::close);

	m_okButton->setText(i18n("&Flatten"));
	m_okButton->setToolTip(i18n("Flatten values in the selected spreadsheet columns"));
	setWindowTitle(i18nc("@title:window", "Flatten Selected Columns"));

	connect(m_okButton, &QPushButton::clicked, this, &FlattenColumnsDialog::flattenColumns);
	connect(btnBox, &QDialogButtonBox::accepted, this, &FlattenColumnsDialog::accept);
	connect(btnBox, &QDialogButtonBox::rejected, this, &FlattenColumnsDialog::reject);

	// restore saved settings if available
	KConfigGroup conf(KSharedConfig::openConfig(), QLatin1String("FlattenColumnsDialog"));
	create(); // ensure there's a window created
	if (conf.exists()) {
		KWindowConfig::restoreWindowSize(windowHandle(), conf);
		resize(windowHandle()->size()); // workaround for QTBUG-40584
	} else
		resize(QSize(400, 0).expandedTo(minimumSize()));
}

FlattenColumnsDialog::~FlattenColumnsDialog() {
	// save the current settings
	KConfigGroup conf(KSharedConfig::openConfig(), QLatin1String("FlattenColumnsDialog"));
	KWindowConfig::saveWindowSize(windowHandle(), conf);
}

void FlattenColumnsDialog::setColumns(const QVector<Column*>& columns) {
	m_columns = columns;

	// add column names
	const auto& allColumns = m_spreadsheet->children<Column>();
	for (auto* column : allColumns) {
		if (m_columns.indexOf(column) == -1)
			m_referenceColumnNames << column->name();
	}

	ui.cbReferenceColumn->addItems(m_referenceColumnNames);

	m_columnComboBoxes << ui.cbReferenceColumn;

	// resize the dialog to have the minimum height
	layout()->activate();
	resize(QSize(this->width(), 0).expandedTo(minimumSize()));
}

void FlattenColumnsDialog::addReferenceColumn() {
	const int index = m_columnComboBoxes.size();

	// add new combo box
	auto* cb = new QComboBox;
	cb->addItems(m_referenceColumnNames);
	m_gridLayout->addWidget(cb, index, 1, 1, 1);
	m_columnComboBoxes << cb;

	// add new "remove" button to the same grid line
	auto* button = new QPushButton();
	button->setIcon(QIcon::fromTheme("list-remove"));
	button->setToolTip(i18n("Remove reference column"));
	connect(button, &QPushButton::clicked, this, &FlattenColumnsDialog::removeReferenceColumn);
	m_gridLayout->addWidget(button, index, 2, 1, 1);
	m_removeButtons << button;

	// move the "add new" button to the next grid line
	if (m_columnComboBoxes.count() < m_referenceColumnNames.count())
		m_gridLayout->addWidget(m_buttonNew, index + 1, 2, 1, 1);
	else
		m_buttonNew->hide();
}

void FlattenColumnsDialog::removeReferenceColumn() {
	// TODO:
}

void FlattenColumnsDialog::flattenColumns() const {
	WAIT_CURSOR;

	m_spreadsheet->beginMacro(i18n("%1: flatten values", m_spreadsheet->name()));

	// reference columns in the source spreadsheet
	QVector<Column*> referenceColumns;
	for (const auto* cb : m_columnComboBoxes)
		referenceColumns << m_spreadsheet->column(cb->currentText());
	const int referenceColumnCount = referenceColumns.count();

	// create target spreadsheet
	auto* targetSpreadsheet = new Spreadsheet(i18n("Flatten of %1", m_spreadsheet->name()));
	targetSpreadsheet->setColumnCount(referenceColumnCount + 2);
	const auto& targetColumns = targetSpreadsheet->children<Column>();

	// set the names and modes for the reference columns in the target spreadsheet
	for (int i = 0; i < referenceColumnCount; ++i) {
		auto* source = referenceColumns.at(i);
		auto* target = targetSpreadsheet->column(i);
		target->setName(source->name());
		target->setColumnMode(source->columnMode());
	}

	// add "Category" and "Value" columns which will contain the flattened data
	auto* categoryColumn = targetSpreadsheet->column(referenceColumnCount);
	categoryColumn->setName(i18n("Category"));
	categoryColumn->setColumnMode(AbstractColumn::ColumnMode::Text);
	categoryColumn->setPlotDesignation(AbstractColumn::PlotDesignation::X);

	auto* valueColumn = targetSpreadsheet->column(referenceColumnCount + 1);
	valueColumn->setName(i18n("Value"));
	auto valueColumnMode = m_columns.at(0)->columnMode();
	valueColumn->setColumnMode(valueColumnMode);

	// flatten
	int row = 0; // current row in the target spreadsheet
	for (int i = 0; i < m_spreadsheet->rowCount(); ++i) {
		for (int j = 0; j < m_columns.count(); ++j) {
			auto* sourceColumn = m_columns.at(j);
			if (sourceColumn->asStringColumn()->textAt(i).isEmpty())
				continue;

			// add reference values for every source column to be flattened
			int refIndex = 0;
			for (auto* col : referenceColumns) {
				switch (col->columnMode()) {
				case AbstractColumn::ColumnMode::Double: {
					auto value = col->valueAt(i);
					targetColumns.at(refIndex)->setValueAt(row, value);
					break;
				}
				case AbstractColumn::ColumnMode::Integer: {
					const auto value = col->integerAt(i);
					targetColumns.at(refIndex)->setIntegerAt(row, value);
					break;
				}
				case AbstractColumn::ColumnMode::BigInt: {
					const auto value = col->bigIntAt(i);
					targetColumns.at(refIndex)->setBigIntAt(row, value);
					break;
				}
				case AbstractColumn::ColumnMode::Text: {
					const auto& value = col->textAt(i);
					targetColumns.at(refIndex)->setTextAt(row, value);
					break;
				}
				case AbstractColumn::ColumnMode::Month:
				case AbstractColumn::ColumnMode::Day:
				case AbstractColumn::ColumnMode::DateTime: {
					const auto& value = col->dateTimeAt(i);
					targetColumns.at(refIndex)->setDateTimeAt(row, value);
					break;
				}
				}

				++refIndex;
			}

			// "Category" column
			categoryColumn->setTextAt(row, sourceColumn->name());

			// "Value" column
			switch (valueColumnMode) {
			case AbstractColumn::ColumnMode::Double: {
				valueColumn->setValueAt(row, sourceColumn->valueAt(i));
				break;
			}
			case AbstractColumn::ColumnMode::Integer: {
				valueColumn->setIntegerAt(row, sourceColumn->integerAt(i));
				break;
			}
			case AbstractColumn::ColumnMode::BigInt: {
				valueColumn->setBigIntAt(row, sourceColumn->bigIntAt(i));
				break;
			}
			case AbstractColumn::ColumnMode::Text: {
				valueColumn->setTextAt(row, sourceColumn->textAt(i));
				break;
			}
			case AbstractColumn::ColumnMode::Month:
			case AbstractColumn::ColumnMode::Day:
			case AbstractColumn::ColumnMode::DateTime: {
				valueColumn->setDateTimeAt(row, sourceColumn->dateTimeAt(i));
				break;
			}
			}

			++row;
		}
	}

	m_spreadsheet->parentAspect()->addChild(targetSpreadsheet);

	m_spreadsheet->endMacro();
	RESET_CURSOR;
}
