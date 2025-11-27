/*
	File                 : FlattenColumnsDialog.cpp
	Project              : LabPlot
	Description          : Dialog for flattening of spreadsheet columns
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2022 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "FlattenColumnsDialog.h"
#include "backend/core/Settings.h"
#include "backend/core/column/ColumnStringIO.h"
#include "backend/lib/macros.h"
#include "backend/spreadsheet/Spreadsheet.h"

#include <QComboBox>
#include <QPushButton>
#include <QThreadPool>
#include <QWindow>

#include <KLocalizedString>
#include <KMessageBox>
#include <KWindowConfig>

/*!
	\class FlattenColumnsDialog
	\brief Dialog for flattening of the selected columns in the spreadsheet.

	\ingroup frontend
 */
FlattenColumnsDialog::FlattenColumnsDialog(Spreadsheet* s, QWidget* parent)
	: QDialog(parent)
	, m_spreadsheet(s) {
	setAttribute(Qt::WA_DeleteOnClose);

	// setup the main widget and the button box
	auto* mainWidget = new QWidget(this);
	ui.setupUi(mainWidget);

	auto* btnBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
	ui.verticalLayout->addWidget(btnBox);
	m_okButton = btnBox->button(QDialogButtonBox::Ok);
	m_okButton->setText(i18n("&Flatten"));
	m_okButton->setToolTip(i18n("Flatten selected columns"));

	auto* layout = new QVBoxLayout(this);
	layout->addWidget(mainWidget);
	layout->addWidget(btnBox);
	setLayout(layout);

	setWindowTitle(i18nc("@title:window", "Flatten Selected Columns"));

	// create "add new reference column" button
	m_buttonNew = new QPushButton();
	m_buttonNew->setIcon(QIcon::fromTheme(QStringLiteral("list-add")));
	m_buttonNew->setToolTip(i18n("Add additional reference column"));
	m_gridLayout = static_cast<QGridLayout*>(ui.scrollArea->widget()->layout());
	m_gridLayout->addWidget(m_buttonNew, 1, 2, 1, 1);

	// signal/slot connections
	connect(m_buttonNew, &QPushButton::clicked, this, &FlattenColumnsDialog::addReferenceColumn);
	connect(m_okButton, &QPushButton::clicked, this, &FlattenColumnsDialog::flattenColumns);
	connect(btnBox->button(QDialogButtonBox::Cancel), &QPushButton::clicked, this, &FlattenColumnsDialog::close);
	connect(btnBox, &QDialogButtonBox::accepted, this, &FlattenColumnsDialog::accept);
	connect(btnBox, &QDialogButtonBox::rejected, this, &FlattenColumnsDialog::reject);

	// restore saved settings if available
	KConfigGroup conf = Settings::group(QLatin1String("FlattenColumnsDialog"));
	create(); // ensure there's a window created
	if (conf.exists()) {
		KWindowConfig::restoreWindowSize(windowHandle(), conf);
		resize(windowHandle()->size()); // workaround for QTBUG-40584
	} else
		resize(QSize(400, 0).expandedTo(minimumSize()));
}

FlattenColumnsDialog::~FlattenColumnsDialog() {
	// save the current settings
	KConfigGroup conf = Settings::group(QLatin1String("FlattenColumnsDialog"));
	KWindowConfig::saveWindowSize(windowHandle(), conf);
}

void FlattenColumnsDialog::setColumns(const QVector<Column*>& columns) {
	m_columns = columns;

	// names of reference columns - columns that are not in the current selection in the spreadsheet
	const auto& allColumns = m_spreadsheet->children<Column>();
	for (auto* column : allColumns) {
		if (m_columns.indexOf(column) == -1)
			m_referenceColumnNames << column->name();
	}

	if (!m_referenceColumnNames.isEmpty()) {
		// add all other columns in the spreadsheet that were not selected as reference columns
		for (int i = 0; i < m_referenceColumnNames.count(); ++i)
			addReferenceColumn();
	} else
		m_buttonNew->setEnabled(false);

	// resize the dialog to have the minimum height
	layout()->activate();
	resize(QSize(this->width(), 0).expandedTo(minimumSize()));
}

void FlattenColumnsDialog::addReferenceColumn() {
	const int index = m_columnComboBoxes.size();

	// add new combo box
	auto* cb = new QComboBox;
	cb->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum));
	cb->addItems(m_referenceColumnNames);
	cb->setCurrentText(m_referenceColumnNames.at(index));
	m_gridLayout->addWidget(cb, index, 1, 1, 1);
	m_columnComboBoxes << cb;

	// add new "remove" button to the same grid line
	auto* button = new QPushButton();
	button->setIcon(QIcon::fromTheme(QStringLiteral("list-remove")));
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
	auto* sender = static_cast<QPushButton*>(QObject::sender());
	if (!sender)
		return;

	// remove button was clicked, determine which one and
	// delete it together with the corresponding combobox
	for (int i = 0; i < m_removeButtons.count(); ++i) {
		if (sender == m_removeButtons.at(i)) {
			delete m_columnComboBoxes.takeAt(i);
			delete m_removeButtons.takeAt(i);
		}
	}

	m_buttonNew->show();
}

void FlattenColumnsDialog::flattenColumns() const {
	WAIT_CURSOR_AUTO_RESET;
	m_spreadsheet->beginMacro(i18n("%1: flatten values", m_spreadsheet->name()));

	// reference columns in the source spreadsheet
	QVector<Column*> referenceColumns;
	for (const auto* cb : m_columnComboBoxes)
		referenceColumns << m_spreadsheet->column(cb->currentText());

	flatten(m_spreadsheet, m_columns, referenceColumns);

	m_spreadsheet->endMacro();
}

/*!
 * unit-testable helper function that is creating the new target spreadsheet and doing the actual flattening
 */
void FlattenColumnsDialog::flatten(const Spreadsheet* sourceSpreadsheet, const QVector<Column*>& valueColumns, const QVector<Column*>& referenceColumns) const {
	// create target spreadsheet
	const int referenceColumnCount = referenceColumns.count();
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
	auto valueColumnMode = valueColumns.at(0)->columnMode();
	valueColumn->setColumnMode(valueColumnMode);

	// flatten
	int row = 0; // current row in the target spreadsheet
	for (int i = 0; i < sourceSpreadsheet->rowCount(); ++i) {
		for (int j = 0; j < valueColumns.count(); ++j) {
			auto* sourceColumn = valueColumns.at(j);

			// skip the current row if there are now source values to be flattened
			if (sourceColumn->asStringColumn()->textAt(i).isEmpty())
				continue;

			// skip the current row if there is not a single reference value
			bool hasReferenceValues = false;
			for (auto* col : referenceColumns) {
				if (!col->asStringColumn()->textAt(i).isEmpty()) {
					hasReferenceValues = true;
					break;
				}
			}

			if (referenceColumnCount > 0 && !hasReferenceValues)
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

	targetSpreadsheet->setRowCount(row);

	sourceSpreadsheet->parentAspect()->addChild(targetSpreadsheet);
}
