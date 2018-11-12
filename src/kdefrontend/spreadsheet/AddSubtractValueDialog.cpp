/***************************************************************************
    File                 : AddSubtractValueDialog.cpp
    Project              : LabPlot
    Description          : Dialog for adding/subtracting a value from column values
    --------------------------------------------------------------------
    Copyright            : (C) 2018 by Alexander Semke (alexander.semke@web.de)

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
#include "AddSubtractValueDialog.h"
#include "backend/core/column/Column.h"
#include "backend/core/datatypes/DateTime2StringFilter.h"
#include "backend/lib/macros.h"
#include "backend/spreadsheet/Spreadsheet.h"

#include <QDialogButtonBox>
#include <QPushButton>

#include <KLocalizedString>
#include <KSharedConfig>
#include <KWindowConfig>

/*!
	\class AddSubtractValueDialog
	\brief Dialog for adding/subtracting a value from column values.

	\ingroup kdefrontend
 */

AddSubtractValueDialog::AddSubtractValueDialog(Spreadsheet* s, bool addValue, QWidget* parent) : QDialog(parent),
	m_spreadsheet(s), m_addValue(addValue) {
	Q_ASSERT(s != nullptr);

	if (addValue)
		setWindowTitle(i18nc("@title:window", "Add Value"));
	else
		setWindowTitle(i18nc("@title:window", "Subtract Value"));

	ui.setupUi(this);
	setAttribute(Qt::WA_DeleteOnClose);

	QDialogButtonBox* btnBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

	ui.gridLayout->addWidget(btnBox, 3, 0, 1, 2);
	m_okButton = btnBox->button(QDialogButtonBox::Ok);

	if (addValue) {
		m_okButton->setText(i18n("&Add"));
		m_okButton->setToolTip(i18n("Add value to column values"));
	} else {
		m_okButton->setText(i18n("&Subtract"));
		m_okButton->setToolTip(i18n("Subtract value from column values"));
	}

	connect(m_okButton, &QPushButton::clicked, this, &AddSubtractValueDialog::generate);
	connect(btnBox, &QDialogButtonBox::accepted, this, &AddSubtractValueDialog::accept);
	connect(btnBox, &QDialogButtonBox::rejected, this, &AddSubtractValueDialog::reject);

	//restore saved settings if available
	KConfigGroup conf(KSharedConfig::openConfig(), "AddSubtractValueDialog");
	if (conf.exists())
		KWindowConfig::restoreWindowSize(windowHandle(), conf);
	else
		resize(QSize(300, 0).expandedTo(minimumSize()));
}

AddSubtractValueDialog::~AddSubtractValueDialog() {
	KConfigGroup conf(KSharedConfig::openConfig(), "AddSubtractValueDialog");
	KWindowConfig::saveWindowSize(windowHandle(), conf);
}

void AddSubtractValueDialog::setColumns(QVector<Column*> columns) {
	m_columns = columns;

	//depending on the current column mode, activate/deactivate the corresponding widgets
	//and show the first valid value in the first selected column as the value to add/subtract
	const Column* column = m_columns.first();
	AbstractColumn::ColumnMode mode = column->columnMode();
	if (mode == AbstractColumn::Integer) {
		ui.lTimeValue->setVisible(false);
		ui.dateTimeEdit->setVisible(false);
		ui.leValue->setValidator(new QIntValidator(ui.leValue));
		ui.leValue->setText(QString::number(column->integerAt(0)));
	} else 	if (mode == AbstractColumn::Numeric) {
		ui.lTimeValue->setVisible(false);
		ui.dateTimeEdit->setVisible(false);
		ui.leValue->setValidator(new QDoubleValidator(ui.leValue));

		for (int row = 0; row < column->rowCount(); ++row) {
			const double value = column->valueAt(row);
			if (!std::isnan(value)) {
				ui.leValue->setText(QString::number(column->valueAt(row), 'g', 16));
				break;
			}
		}
	} else 	{ //datetime
		ui.lValue->setVisible(false);
		ui.leValue->setVisible(false);
		auto* filter = static_cast<DateTime2StringFilter*>(column->outputFilter());
		ui.dateTimeEdit->setDisplayFormat(filter->format());

		for (int row = 0; row < column->rowCount(); ++row) {
			const QDateTime& value = column->dateTimeAt(row);
			if (value.isValid()) {
				ui.dateTimeEdit->setDateTime(value);
				break;
			}
		}
	}

	valueChanged();
}

void AddSubtractValueDialog::valueChanged() {

}

/*!
 * generates new values in the selected columns by adding/subtracting the value provided in this dialog
 * from every column element.
 */
void AddSubtractValueDialog::generate() {
	Q_ASSERT(m_spreadsheet);

	WAIT_CURSOR;
	QString msg;
	if (m_addValue)
		msg = i18np("%1: add value to column", "%1: add value to columns",
					m_spreadsheet->name(), m_columns.size());
	else
		msg = i18np("%1: subtract value from column", "%1: subtract value from columns",
					m_spreadsheet->name(), m_columns.size());

	m_spreadsheet->beginMacro(msg);

	AbstractColumn::ColumnMode mode = m_columns.first()->columnMode();
	const int rows = m_spreadsheet->rowCount();
	if (mode == AbstractColumn::Integer) {
		QVector<int> new_data(rows);
		int value = ui.leValue->text().toInt();
		if (!m_addValue)
			value *= -1;

		for (auto* col : m_columns) {
			QVector<int>* data = static_cast<QVector<int>* >(col->data());
			for (int i = 0; i<rows; ++i)
				new_data[i] = data->operator[](i) + value;

			col->replaceInteger(0, new_data);
		}
	} else if (mode == AbstractColumn::Numeric) {
		QVector<double> new_data(rows);
		double value = ui.leValue->text().toDouble();
		if (!m_addValue)
			value *= -1.;

		for (auto* col : m_columns) {
			QVector<double>* data = static_cast<QVector<double>* >(col->data());
			for (int i = 0; i<rows; ++i)
				new_data[i] = data->operator[](i) + value;

			col->replaceValues(0, new_data);
		}
	} else { //datetime
		QVector<QDateTime> new_data(rows);
		quint64 value = ui.dateTimeEdit->dateTime().toMSecsSinceEpoch();
		if (!m_addValue)
			value *= -1;

		for (auto* col : m_columns) {
			QVector<QDateTime>* data = static_cast<QVector<QDateTime>* >(col->data());
			for (int i = 0; i<rows; ++i)
				new_data[i] = QDateTime::fromMSecsSinceEpoch(data->operator[](i).toMSecsSinceEpoch() + value);

			col->replaceDateTimes(0, new_data);
		}
	}

	m_spreadsheet->endMacro();

	RESET_CURSOR;
}
