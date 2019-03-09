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
#include "backend/matrix/Matrix.h"

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

AddSubtractValueDialog::AddSubtractValueDialog(Spreadsheet* s, Operation op, QWidget* parent) : QDialog(parent),
	m_spreadsheet(s), m_operation(op) {
	Q_ASSERT(s != nullptr);

	switch (m_operation) {
	case Add:
		setWindowTitle(i18nc("@title:window", "Add Value"));
		break;
	case Subtract:
		setWindowTitle(i18nc("@title:window", "Subtract Value"));
		break;
	case Multiply:
		setWindowTitle(i18nc("@title:window", "Multiply by Value"));
		break;
	case Divide:
		setWindowTitle(i18nc("@title:window", "Divide by Value"));
		break;
	}
	ui.setupUi(this);
	setAttribute(Qt::WA_DeleteOnClose);

	QDialogButtonBox* btnBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

	ui.gridLayout->addWidget(btnBox, 3, 0, 1, 2);
	m_okButton = btnBox->button(QDialogButtonBox::Ok);

	switch (m_operation) {
	case Add:
		m_okButton->setText(i18n("&Add"));
		m_okButton->setToolTip(i18n("Add the specified value to column values"));
		break;
	case Subtract:
		m_okButton->setText(i18n("&Subtract"));
		m_okButton->setToolTip(i18n("Subtract the specified value from column values"));
		break;
	case Multiply:
		m_okButton->setText(i18n("&Multiply"));
		m_okButton->setToolTip(i18n("Multiply column values by the specified value"));
		break;
	case Divide:
		m_okButton->setText(i18n("&Divide"));
		m_okButton->setToolTip(i18n("Divide column values by the specified value"));
		break;
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

AddSubtractValueDialog::AddSubtractValueDialog(Matrix* m, Operation op, QWidget* parent) : QDialog(parent),
	m_matrix(m), m_operation(op) {
	Q_ASSERT(m != nullptr);

	switch (m_operation) {
	case Add:
		setWindowTitle(i18nc("@title:window", "Add Value"));
		break;
	case Subtract:
		setWindowTitle(i18nc("@title:window", "Subtract Value"));
		break;
	case Multiply:
		setWindowTitle(i18nc("@title:window", "Multiply by Value"));
		break;
	case Divide:
		setWindowTitle(i18nc("@title:window", "Divide by Value"));
		break;
	}
	ui.setupUi(this);
	setAttribute(Qt::WA_DeleteOnClose);

	QDialogButtonBox* btnBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

	ui.gridLayout->addWidget(btnBox, 3, 0, 1, 2);
	m_okButton = btnBox->button(QDialogButtonBox::Ok);

	switch (m_operation) {
	case Add:
		m_okButton->setText(i18n("&Add"));
		m_okButton->setToolTip(i18n("Add the specified value to column values"));
		break;
	case Subtract:
		m_okButton->setText(i18n("&Subtract"));
		m_okButton->setToolTip(i18n("Subtract the specified value from column values"));
		break;
	case Multiply:
		m_okButton->setText(i18n("&Multiply"));
		m_okButton->setToolTip(i18n("Multiply column values by the specified value"));
		break;
	case Divide:
		m_okButton->setText(i18n("&Divide"));
		m_okButton->setToolTip(i18n("Divide column values by the specified value"));
		break;
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

void AddSubtractValueDialog::setMatrices() {
	if (m_matrix->mode() == AbstractColumn::Integer) {
		ui.lTimeValue->setVisible(false);
		ui.dateTimeEdit->setVisible(false);
		ui.leValue->setValidator(new QIntValidator(ui.leValue));
		ui.leValue->setText(QString::number(m_matrix->cell<int>(0,0)));
	} else 	if (m_matrix->mode() == AbstractColumn::Numeric) {
		ui.lTimeValue->setVisible(false);
		ui.dateTimeEdit->setVisible(false);
		ui.leValue->setValidator(new QDoubleValidator(ui.leValue));
		ui.leValue->setText(QString::number(m_matrix->cell<double>(0,0)));
	} else 	{ //datetime
		ui.lValue->setVisible(false);
		ui.leValue->setVisible(false);
	}
	valueChanged();
}

void AddSubtractValueDialog::valueChanged() {

}

/*!
 * generates new values in the selected columns by adding/subtracting the value provided in this dialog
 * from every column element.
 */

QString AddSubtractValueDialog::getMessage(QString name)
{
	QString msg = "";
	QString value = ui.leValue->text();
	switch (m_operation) {
	case Add:
		msg = i18n("%1: add %2 to column values", name, value);
		break;
	case Subtract:
		msg = i18n("%1: subtract %2 from column values", name, value);
		break;
	case Multiply:
		msg = i18n("%1: multiply column values by %2", name, value);
		break;
	case Divide:
		msg = i18n("%1: divide column values by %2", name, value);
		break;
	}
	return msg;
}

void AddSubtractValueDialog::generate() {
	if(m_spreadsheet != nullptr)
		generateForColumns();
	else if(m_matrix != nullptr)
		generateForMatrices();
}

void AddSubtractValueDialog::generateForColumns() {
	Q_ASSERT(m_spreadsheet);

	WAIT_CURSOR;
	QString msg = getMessage(m_spreadsheet->name());
	m_spreadsheet->beginMacro(msg);

	AbstractColumn::ColumnMode mode = m_columns.first()->columnMode();
	const int rows = m_spreadsheet->rowCount();
	if (mode == AbstractColumn::Integer) {
		QVector<int> new_data(rows);
		int value = ui.leValue->text().toInt();

		switch (m_operation) {
		case Subtract:
			value *= -1;
			//fall through
		case Add:
			for (auto* col : m_columns) {
				auto* data = static_cast<QVector<int>* >(col->data());
				for (int i = 0; i<rows; ++i)
					new_data[i] = data->operator[](i) + value;

				col->replaceInteger(0, new_data);
			}
			break;
		case Multiply:
			for (auto* col : m_columns) {
				auto* data = static_cast<QVector<int>* >(col->data());
				for (int i = 0; i<rows; ++i)
					new_data[i] = data->operator[](i) * value;

				col->replaceInteger(0, new_data);
			}
			break;
		case Divide:
			for (auto* col : m_columns) {
				auto* data = static_cast<QVector<int>* >(col->data());
				for (int i = 0; i<rows; ++i)
					new_data[i] = data->operator[](i) / value;

				col->replaceInteger(0, new_data);
			}
			break;
		}
	} else if (mode == AbstractColumn::Numeric) {
		QVector<double> new_data(rows);
		double value = ui.leValue->text().toDouble();
		switch (m_operation) {
		case Subtract:
			value *= -1.;
			//fall through
		case Add:
			for (auto* col : m_columns) {
				auto* data = static_cast<QVector<double>* >(col->data());
				for (int i = 0; i<rows; ++i)
					new_data[i] = data->operator[](i) + value;

				col->replaceValues(0, new_data);
			}
			break;
		case Multiply:
			for (auto* col : m_columns) {
				auto* data = static_cast<QVector<double>* >(col->data());
				for (int i = 0; i<rows; ++i)
					new_data[i] = data->operator[](i) * value;

				col->replaceValues(0, new_data);
			}
			break;
		case Divide:
			for (auto* col : m_columns) {
				auto* data = static_cast<QVector<double>* >(col->data());
				for (int i = 0; i<rows; ++i)
					new_data[i] = data->operator[](i) / value;

				col->replaceValues(0, new_data);
			}
			break;
	}
	} else { //datetime
		QVector<QDateTime> new_data(rows);
		quint64 value = ui.dateTimeEdit->dateTime().toMSecsSinceEpoch();
		switch (m_operation) {
		case Subtract:
			value *= -1.;
			//fall through
		case Add:
			for (auto* col : m_columns) {
				auto* data = static_cast<QVector<QDateTime>* >(col->data());
				for (int i = 0; i<rows; ++i)
					new_data[i] = QDateTime::fromMSecsSinceEpoch(data->operator[](i).toMSecsSinceEpoch() + value);

				col->replaceDateTimes(0, new_data);
			}
		case Multiply:
		case Divide:
			break;
		}

	}

	m_spreadsheet->endMacro();

	RESET_CURSOR;
}

void AddSubtractValueDialog::generateForMatrices() {
	Q_ASSERT(m_matrix);

	WAIT_CURSOR;
	QString msg = getMessage(m_matrix->name());
	m_matrix->beginMacro(msg);

	AbstractColumn::ColumnMode mode = m_matrix->mode();

	const int rows = m_matrix->rowCount();
	const int cols = m_matrix->columnCount();
	if (mode == AbstractColumn::Integer) {
		int new_data;
		int value = ui.leValue->text().toInt();

		switch (m_operation) {
		case Subtract:
			value *= -1;
			//fall through
		case Add:
			for (int i = 0; i<rows; ++i)
				for(int j = 0; j<cols; ++j)
				{
					new_data = m_matrix->cell<int>(i,j);
					new_data += value;
					m_matrix->setCell(i,j,new_data);
				}
			break;
		case Multiply:
			for (int i = 0; i<rows; ++i)
				for(int j = 0; j<cols; ++j)
				{
					new_data = m_matrix->cell<int>(i,j);
					new_data *= value;
					m_matrix->setCell(i,j,new_data);
				}
			break;
		case Divide:
		for (int i = 0; i<rows; ++i)
				for(int j = 0; j<cols; ++j)
				{
					new_data = m_matrix->cell<int>(i,j);
					new_data /= value;
					m_matrix->setCell(i,j,new_data);
				}
			break;
		}
	} else if (mode == AbstractColumn::Numeric) {
		double new_data;
		double value = ui.leValue->text().toDouble();
		switch (m_operation) {
		case Subtract:
			value *= -1.;
			//fall through
		case Add:
			for (int i = 0; i<rows; ++i)
				for(int j = 0; j<cols; ++j)
				{
					new_data = m_matrix->cell<double>(i,j);
					new_data += value;
					m_matrix->setCell(i,j,new_data);
				}
			break;
		case Multiply:
			for (int i = 0; i<rows; ++i)
				for(int j = 0; j<cols; ++j)
				{
					new_data = m_matrix->cell<double>(i,j);
					new_data *= value;
					m_matrix->setCell(i,j,new_data);
				}
			break;
		case Divide:
			for (int i = 0; i<rows; ++i)
				for(int j = 0; j<cols; ++j)
				{
					new_data = m_matrix->cell<double>(i,j);
					new_data /= value;
					m_matrix->setCell(i,j,new_data);
				}
			break;
	}
	} else { //datetime
		QDateTime new_data;
		quint64 value = ui.dateTimeEdit->dateTime().toMSecsSinceEpoch();
		switch (m_operation) {
		case Subtract:
			value *= -1.;
			//fall through
		case Add:
			for (int i = 0; i<rows; ++i)
			for(int j = 0; j<cols; ++j)
			{
				quint64 data = (m_matrix->cell<QDateTime>(i,j)).toMSecsSinceEpoch();
				new_data = QDateTime::fromMSecsSinceEpoch(data + value);
				m_matrix->setCell(i,j,new_data);
			}
		break;
		case Multiply:
		case Divide:
			break;
		}
	}

	m_matrix->endMacro();

	RESET_CURSOR;

}