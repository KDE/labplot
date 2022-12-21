/*
	File                 : AddSubtractValueDialog.cpp
	Project              : LabPlot
	Description          : Dialog for adding/subtracting a value from column values
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2018-2022 Alexander Semke <alexander.semke@web.de>
	SPDX-FileCopyrightText: 2020 Stefan Gerlach <stefan.gerlach@uni.kn>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "AddSubtractValueDialog.h"
#include "backend/core/column/Column.h"
#include "backend/core/datatypes/DateTime2StringFilter.h"
#include "backend/lib/macros.h"
#include "backend/matrix/Matrix.h"
#include "backend/spreadsheet/Spreadsheet.h"

#include <KLocalizedString>
#include <KMessageBox>
#include <KWindowConfig>

#include <QDialogButtonBox>
#include <QPushButton>
#include <QWindow>

#include <cmath>

/*!
	\class AddSubtractValueDialog
	\brief Dialog for adding/subtracting a value from column values.

	\ingroup kdefrontend
 */

AddSubtractValueDialog::AddSubtractValueDialog(Spreadsheet* s, Operation op, QWidget* parent)
	: QDialog(parent)
	, m_spreadsheet(s)
	, m_operation(op) {
	Q_ASSERT(s != nullptr);

	init();

	switch (m_operation) {
	case Add:
		m_okButton->setToolTip(i18n("Add the specified value to column values"));
		break;
	case Subtract:
		m_okButton->setToolTip(i18n("Subtract the specified value from column values"));
		break;
	case Multiply:
		m_okButton->setToolTip(i18n("Multiply column values by the specified value"));
		break;
	case Divide:
		m_okButton->setToolTip(i18n("Divide column values by the specified value"));
		break;
	}
}

AddSubtractValueDialog::AddSubtractValueDialog(Matrix* m, Operation op, QWidget* parent)
	: QDialog(parent)
	, m_matrix(m)
	, m_operation(op) {
	Q_ASSERT(m != nullptr);

	init();

	switch (m_operation) {
	case Add:
		m_okButton->setToolTip(i18n("Add the specified value to matrix values"));
		break;
	case Subtract:
		m_okButton->setToolTip(i18n("Subtract the specified value from matrix values"));
		break;
	case Multiply:
		m_okButton->setToolTip(i18n("Multiply matrix values by the specified value"));
		break;
	case Divide:
		m_okButton->setToolTip(i18n("Divide matrix values by the specified value"));
	}
}

void AddSubtractValueDialog::init() {
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

	auto* btnBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

	ui.gridLayout->addWidget(btnBox, 7, 0, 1, 3);
	m_okButton = btnBox->button(QDialogButtonBox::Ok);

	switch (m_operation) {
	case Add:
		m_okButton->setText(i18n("&Add"));
		break;
	case Subtract:
		m_okButton->setText(i18n("&Subtract"));
		break;
	case Multiply:
		m_okButton->setText(i18n("&Multiply"));
		break;
	case Divide:
		m_okButton->setText(i18n("&Divide"));
		break;
	}

	if (m_operation == Add || m_operation == Subtract) {
		ui.cbType->addItem(i18n("Absolute Value"));
		ui.cbType->addItem(i18n("Difference"));
		ui.cbTimeUnits->addItem(i18n("Milliseconds"));
		ui.cbTimeUnits->addItem(i18n("Seconds"));
		ui.cbTimeUnits->addItem(i18n("Minutes"));
		ui.cbTimeUnits->addItem(i18n("Hours"));
		ui.cbTimeUnits->addItem(i18n("Days"));
	} else {
		ui.lType->hide();
		ui.cbType->hide();
	}

	connect(m_okButton, &QPushButton::clicked, this, &AddSubtractValueDialog::generate);
	connect(btnBox, &QDialogButtonBox::accepted, this, &AddSubtractValueDialog::accept);
	connect(btnBox, &QDialogButtonBox::rejected, this, &AddSubtractValueDialog::reject);
	connect(ui.cbType, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &AddSubtractValueDialog::typeChanged);
	connect(ui.leValue, &QLineEdit::textChanged, this, [=]() {
		m_okButton->setEnabled(!ui.leValue->text().isEmpty());
	});

	// restore saved settings if available
	create(); // ensure there's a window created
	KConfigGroup conf(KSharedConfig::openConfig(), "AddSubtractValueDialog");
	if (conf.exists()) {
		ui.cbType->setCurrentIndex(conf.readEntry("Type", 0));
		KWindowConfig::restoreWindowSize(windowHandle(), conf);
		resize(windowHandle()->size()); // workaround for QTBUG-40584
	} else
		resize(QSize(300, 0).expandedTo(minimumSize()));
}

AddSubtractValueDialog::~AddSubtractValueDialog() {
	KConfigGroup conf(KSharedConfig::openConfig(), "AddSubtractValueDialog");
	KWindowConfig::saveWindowSize(windowHandle(), conf);
	conf.writeEntry("Type", ui.cbType->currentIndex());
}

void AddSubtractValueDialog::setColumns(const QVector<Column*>& columns) {
	m_columns = columns;

	// depending on the current column mode, activate/deactivate the corresponding widgets
	// and show the first valid value in the first selected column as the value to add/subtract
	const Column* column = m_columns.first();
	const auto numberLocale = QLocale();

	switch (column->columnMode()) {
	case AbstractColumn::ColumnMode::Integer: {
		m_numeric = true;
		const auto str = numberLocale.toString(column->integerAt(0));
		ui.leValue->setValidator(new QIntValidator(ui.leValue));
		ui.leValue->setText(str);
		ui.leValueStart->setValidator(new QIntValidator(ui.leValueStart));
		ui.leValueStart->setText(str);
		ui.leValueEnd->setValidator(new QIntValidator(ui.leValueEnd));
		ui.leValueEnd->setText(str);
		break;
	}
	case AbstractColumn::ColumnMode::BigInt: {
		m_numeric = true;
		const auto str = numberLocale.toString(column->bigIntAt(0));
		// TODO: QLongLongValidator
		ui.leValue->setValidator(new QIntValidator(ui.leValue));
		ui.leValue->setText(str);
		ui.leValueStart->setValidator(new QIntValidator(ui.leValueStart));
		ui.leValueStart->setText(str);
		ui.leValueEnd->setValidator(new QIntValidator(ui.leValueEnd));
		ui.leValueEnd->setText(str);
		break;
	}
	case AbstractColumn::ColumnMode::Double: {
		m_numeric = true;
		ui.leValue->setValidator(new QDoubleValidator(ui.leValue));
		ui.leValueStart->setValidator(new QDoubleValidator(ui.leValueStart));
		ui.leValueEnd->setValidator(new QDoubleValidator(ui.leValueEnd));

		for (int row = 0; row < column->rowCount(); ++row) {
			const double value = column->valueAt(row);
			if (std::isfinite(value)) {
				const auto str = numberLocale.toString(column->valueAt(row), 'g', 16);
				ui.leValue->setText(str);
				ui.leValueStart->setText(str);
				ui.leValueEnd->setText(str);
				break;
			}
		}
		break;
	}
	case AbstractColumn::ColumnMode::Month:
	case AbstractColumn::ColumnMode::Day:
	case AbstractColumn::ColumnMode::DateTime: {
		m_numeric = false;
		auto* filter = static_cast<DateTime2StringFilter*>(column->outputFilter());
		ui.dteTimeValueStart->setDisplayFormat(filter->format());
		ui.dteTimeValueEnd->setDisplayFormat(filter->format());

		for (int row = 0; row < column->rowCount(); ++row) {
			const QDateTime& value = column->dateTimeAt(row);
			if (value.isValid()) {
				ui.dteTimeValueStart->setDateTime(value);
				ui.dteTimeValueEnd->setDateTime(value);
				break;
			}
		}
	}
	case AbstractColumn::ColumnMode::Text: {
		// not supported
		break;
	}
	}

	updateWidgetsVisiblity();
}

void AddSubtractValueDialog::setMatrices() {
	const auto mode = m_matrix->mode();

	const auto numberLocale = QLocale();
	switch (mode) {
	case AbstractColumn::ColumnMode::Integer:
		m_numeric = true;
		ui.leValue->setValidator(new QIntValidator(ui.leValue));
		ui.leValue->setText(numberLocale.toString(m_matrix->cell<int>(0, 0)));
		break;
	case AbstractColumn::ColumnMode::BigInt:
		m_numeric = true;
		// TODO: QLongLongValidator
		ui.leValue->setValidator(new QIntValidator(ui.leValue));
		ui.leValue->setText(numberLocale.toString(m_matrix->cell<qint64>(0, 0)));
		break;
	case AbstractColumn::ColumnMode::Double:
		m_numeric = true;
		ui.leValue->setValidator(new QDoubleValidator(ui.leValue));
		ui.leValue->setText(numberLocale.toString(m_matrix->cell<double>(0, 0)));
		break;
	case AbstractColumn::ColumnMode::DateTime:
	case AbstractColumn::ColumnMode::Day:
	case AbstractColumn::ColumnMode::Month:
	case AbstractColumn::ColumnMode::Text:
		m_numeric = false;
	}

	updateWidgetsVisiblity();
}

void AddSubtractValueDialog::typeChanged(int index) {
	bool diff = (index != 0);
	if (m_numeric) {
		ui.lValue->setVisible(!diff);
		ui.leValue->setVisible(!diff);
		ui.lValueStart->setVisible(diff);
		ui.leValueStart->setVisible(diff);
		ui.lValueEnd->setVisible(diff);
		ui.leValueEnd->setVisible(diff);
	} else {
		ui.lTimeValue->setVisible(!diff);
		ui.leTimeValue->setVisible(!diff);
		ui.cbTimeUnits->setVisible(!diff);
		ui.lTimeValueStart->setVisible(diff);
		ui.dteTimeValueStart->setVisible(diff);
		ui.lTimeValueEnd->setVisible(diff);
		ui.dteTimeValueEnd->setVisible(diff);
	}
}

void AddSubtractValueDialog::updateWidgetsVisiblity() {
	ui.lValue->setVisible(m_numeric);
	ui.leValue->setVisible(m_numeric);
	ui.lTimeValue->setVisible(!m_numeric);
	ui.leTimeValue->setVisible(!m_numeric);

	if (m_operation == Add || m_operation == Subtract) {
		ui.lValueStart->setVisible(m_numeric);
		ui.leValueStart->setVisible(m_numeric);
		ui.lValueEnd->setVisible(m_numeric);
		ui.leValueEnd->setVisible(m_numeric);

		ui.cbTimeUnits->setVisible(!m_numeric);
		ui.lTimeValueStart->setVisible(!m_numeric);
		ui.dteTimeValueStart->setVisible(!m_numeric);
		ui.lTimeValueEnd->setVisible(!m_numeric);
		ui.dteTimeValueEnd->setVisible(!m_numeric);

		typeChanged(ui.cbType->currentIndex());
	} else {
		ui.lValueStart->hide();
		ui.leValueStart->hide();
		ui.lValueEnd->hide();
		ui.leValueEnd->hide();
		ui.cbTimeUnits->hide();
		ui.lTimeValueStart->hide();
		ui.dteTimeValueStart->hide();
		ui.lTimeValueEnd->hide();
		ui.dteTimeValueEnd->hide();
	}
}

/*!
 * generates new values in the selected columns by adding/subtracting the value provided in this dialog
 * from every column element.
 */

QString AddSubtractValueDialog::getMessage(const QString& name) {
	QString msg;
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
	if (m_spreadsheet != nullptr)
		generateForColumns();
	else if (m_matrix != nullptr)
		generateForMatrices();
}

void AddSubtractValueDialog::generateForColumns() {
	Q_ASSERT(m_spreadsheet);

	WAIT_CURSOR;

	QString msg = getMessage(m_spreadsheet->name());
	bool ok;
	const auto mode = m_columns.first()->columnMode();
	const int rows = m_spreadsheet->rowCount();
	if (mode == AbstractColumn::ColumnMode::Integer) {
		int value;
		ok = setIntValue(value);

		if (!ok) {
			RESET_CURSOR;
			KMessageBox::error(this, i18n("Wrong numeric value provided."));
			return;
		}

		QVector<int> new_data(rows);
		m_spreadsheet->beginMacro(msg);

		switch (m_operation) {
		case Subtract:
			value *= -1;
			// fall through
		case Add:
			for (auto* col : m_columns) {
				auto* data = static_cast<QVector<int>*>(col->data());
				for (int i = 0; i < rows; ++i)
					new_data[i] = data->operator[](i) + value;

				col->replaceInteger(0, new_data);
			}
			break;
		case Multiply:
			for (auto* col : m_columns) {
				auto* data = static_cast<QVector<int>*>(col->data());
				for (int i = 0; i < rows; ++i)
					new_data[i] = data->operator[](i) * value;

				col->replaceInteger(0, new_data);
			}
			break;
		case Divide:
			for (auto* col : m_columns) {
				auto* data = static_cast<QVector<int>*>(col->data());
				for (int i = 0; i < rows; ++i)
					new_data[i] = data->operator[](i) / value;

				col->replaceInteger(0, new_data);
			}
			break;
		}
	} else if (mode == AbstractColumn::ColumnMode::BigInt) {
		qint64 value;
		ok = setBigIntValue(value);

		if (!ok) {
			RESET_CURSOR;
			KMessageBox::error(this, i18n("Wrong numeric value provided."));
			return;
		}

		QVector<qint64> new_data(rows);
		m_spreadsheet->beginMacro(msg);

		switch (m_operation) {
		case Subtract:
			value *= -1;
			// fall through
		case Add:
			for (auto* col : m_columns) {
				auto* data = static_cast<QVector<qint64>*>(col->data());
				for (int i = 0; i < rows; ++i)
					new_data[i] = data->operator[](i) + value;

				col->replaceBigInt(0, new_data);
			}
			break;
		case Multiply:
			for (auto* col : m_columns) {
				auto* data = static_cast<QVector<qint64>*>(col->data());
				for (int i = 0; i < rows; ++i)
					new_data[i] = data->operator[](i) * value;

				col->replaceBigInt(0, new_data);
			}
			break;
		case Divide:
			for (auto* col : m_columns) {
				auto* data = static_cast<QVector<qint64>*>(col->data());
				for (int i = 0; i < rows; ++i)
					new_data[i] = data->operator[](i) / value;

				col->replaceBigInt(0, new_data);
			}
			break;
		}
	} else if (mode == AbstractColumn::ColumnMode::Double) {
		double value;
		ok = setDoubleValue(value);

		if (!ok) {
			RESET_CURSOR;
			KMessageBox::error(this, i18n("Wrong numeric value provided."));
			return;
		}

		QVector<double> new_data(rows);
		m_spreadsheet->beginMacro(msg);

		switch (m_operation) {
		case Subtract:
			value *= -1.;
			// fall through
		case Add:
			for (auto* col : m_columns) {
				auto* data = static_cast<QVector<double>*>(col->data());
				for (int i = 0; i < rows; ++i)
					new_data[i] = data->operator[](i) + value;

				col->replaceValues(0, new_data);
			}
			break;
		case Multiply:
			for (auto* col : m_columns) {
				auto* data = static_cast<QVector<double>*>(col->data());
				for (int i = 0; i < rows; ++i)
					new_data[i] = data->operator[](i) * value;

				col->replaceValues(0, new_data);
			}
			break;
		case Divide:
			for (auto* col : m_columns) {
				auto* data = static_cast<QVector<double>*>(col->data());
				for (int i = 0; i < rows; ++i)
					new_data[i] = data->operator[](i) / value;

				col->replaceValues(0, new_data);
			}
			break;
		}
	} else { // datetime
		qint64 value;
		ok = setDateTimeValue(value);

		if (!ok) {
			RESET_CURSOR;
			KMessageBox::error(this, i18n("Wrong numeric value provided."));
			return;
		}

		QVector<QDateTime> new_data(rows);
		m_spreadsheet->beginMacro(msg);

		switch (m_operation) {
		case Subtract:
			value *= -1;
			// fall through
		case Add:
			for (auto* col : m_columns) {
				auto* data = static_cast<QVector<QDateTime>*>(col->data());
				// QDEBUG(Q_FUNC_INFO << ", DT OLD:" << data->operator[](0))
				// QDEBUG(Q_FUNC_INFO << ", OLD VALUE:" << data->operator[](0).toMSecsSinceEpoch())
				// QDEBUG(Q_FUNC_INFO << ", NEW VALUE:" << data->operator[](0).toMSecsSinceEpoch() + value)
				// QDEBUG(Q_FUNC_INFO << ", DT NEW:" << QDateTime::fromMSecsSinceEpoch(data->operator[](0).toMSecsSinceEpoch() + value, Qt::UTC))
				for (int i = 0; i < rows; ++i)
					new_data[i] = QDateTime::fromMSecsSinceEpoch(data->operator[](i).toMSecsSinceEpoch() + value, Qt::UTC);

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
	auto mode = m_matrix->mode();
	bool ok;
	const int rows = m_matrix->rowCount();
	const int cols = m_matrix->columnCount();

	if (mode == AbstractColumn::ColumnMode::Integer) {
		int value;
		ok = setIntValue(value);

		if (!ok) {
			RESET_CURSOR;
			KMessageBox::error(this, i18n("Wrong numeric value provided."));
			return;
		}

		int new_data;
		m_matrix->beginMacro(msg);

		switch (m_operation) {
		case Subtract:
			value *= -1;
			// fall through
		case Add:
			for (int i = 0; i < rows; ++i)
				for (int j = 0; j < cols; ++j) {
					new_data = m_matrix->cell<int>(i, j);
					new_data += value;
					m_matrix->setCell(i, j, new_data);
				}
			break;
		case Multiply:
			for (int i = 0; i < rows; ++i)
				for (int j = 0; j < cols; ++j) {
					new_data = m_matrix->cell<int>(i, j);
					new_data *= value;
					m_matrix->setCell(i, j, new_data);
				}
			break;
		case Divide:
			for (int i = 0; i < rows; ++i)
				for (int j = 0; j < cols; ++j) {
					new_data = m_matrix->cell<int>(i, j);
					new_data /= value;
					m_matrix->setCell(i, j, new_data);
				}
			break;
		}
	} else if (mode == AbstractColumn::ColumnMode::BigInt) {
		qint64 value;
		ok = setBigIntValue(value);

		if (!ok) {
			RESET_CURSOR;
			KMessageBox::error(this, i18n("Wrong numeric value provided."));
			return;
		}

		qint64 new_data;
		m_matrix->beginMacro(msg);

		switch (m_operation) {
		case Subtract:
			value *= -1;
			// fall through
		case Add:
			for (int i = 0; i < rows; ++i)
				for (int j = 0; j < cols; ++j) {
					new_data = m_matrix->cell<qint64>(i, j);
					new_data += value;
					m_matrix->setCell(i, j, new_data);
				}
			break;
		case Multiply:
			for (int i = 0; i < rows; ++i)
				for (int j = 0; j < cols; ++j) {
					new_data = m_matrix->cell<qint64>(i, j);
					new_data *= value;
					m_matrix->setCell(i, j, new_data);
				}
			break;
		case Divide:
			for (int i = 0; i < rows; ++i)
				for (int j = 0; j < cols; ++j) {
					new_data = m_matrix->cell<qint64>(i, j);
					new_data /= value;
					m_matrix->setCell(i, j, new_data);
				}
			break;
		}
	} else if (mode == AbstractColumn::ColumnMode::Double) {
		double value;
		ok = setDoubleValue(value);

		if (!ok) {
			RESET_CURSOR;
			KMessageBox::error(this, i18n("Wrong numeric value provided."));
			return;
		}

		double new_data;
		m_matrix->beginMacro(msg);

		switch (m_operation) {
		case Subtract:
			value *= -1.;
			// fall through
		case Add:
			for (int i = 0; i < rows; ++i)
				for (int j = 0; j < cols; ++j) {
					new_data = m_matrix->cell<double>(i, j);
					new_data += value;
					m_matrix->setCell(i, j, new_data);
				}
			break;
		case Multiply:
			for (int i = 0; i < rows; ++i)
				for (int j = 0; j < cols; ++j) {
					new_data = m_matrix->cell<double>(i, j);
					new_data *= value;
					m_matrix->setCell(i, j, new_data);
				}
			break;
		case Divide:
			for (int i = 0; i < rows; ++i)
				for (int j = 0; j < cols; ++j) {
					new_data = m_matrix->cell<double>(i, j);
					new_data /= value;
					m_matrix->setCell(i, j, new_data);
				}
			break;
		}
	} else { // datetime
		qint64 value;
		ok = setDateTimeValue(value);

		if (!ok) {
			RESET_CURSOR;
			KMessageBox::error(this, i18n("Wrong numeric value provided."));
			return;
		}

		QDateTime new_data;
		m_matrix->beginMacro(msg);

		switch (m_operation) {
		case Subtract:
			value *= -1;
			// fall through
		case Add:
			for (int i = 0; i < rows; ++i)
				for (int j = 0; j < cols; ++j) {
					auto dateTime = m_matrix->cell<QDateTime>(i, j);
					new_data = QDateTime::fromMSecsSinceEpoch(dateTime.toMSecsSinceEpoch() + value, dateTime.timeSpec(), Qt::UTC);
					m_matrix->setCell(i, j, new_data);
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

bool AddSubtractValueDialog::setIntValue(int& value) const {
	bool ok;
	const auto numberLocale = QLocale();
	if (m_operation == Add || m_operation == Subtract) {
		if (ui.cbType->currentIndex() == 0) // add/subtract an absolute value
			value = numberLocale.toInt(ui.leValue->text(), &ok);
		else // add/subtract a difference
			value = numberLocale.toInt(ui.leValueEnd->text(), &ok) - numberLocale.toInt(ui.leValueStart->text(), &ok);
	} else
		value = numberLocale.toInt(ui.leValue->text(), &ok);

	return ok;
}

bool AddSubtractValueDialog::setBigIntValue(qint64& value) const {
	bool ok;
	const auto numberLocale = QLocale();
	if (m_operation == Add || m_operation == Subtract) {
		if (ui.cbType->currentIndex() == 0) // add/subtract an absolute value
			value = numberLocale.toLongLong(ui.leValue->text(), &ok);
		else // add/subtract a difference
			value = numberLocale.toLongLong(ui.leValueEnd->text(), &ok) - numberLocale.toLongLong(ui.leValueStart->text(), &ok);
	} else
		value = numberLocale.toLongLong(ui.leValue->text(), &ok);

	return ok;
}

bool AddSubtractValueDialog::setDoubleValue(double& value) const {
	bool ok;
	const auto numberLocale = QLocale();
	if (m_operation == Add || m_operation == Subtract) {
		if (ui.cbType->currentIndex() == 0) // add/subtract an absolute value
			value = numberLocale.toDouble(ui.leValue->text(), &ok);
		else // add/subtract a difference
			value = numberLocale.toDouble(ui.leValueEnd->text(), &ok) - numberLocale.toDouble(ui.leValueStart->text(), &ok);
	} else
		value = numberLocale.toDouble(ui.leValue->text(), &ok);

	return ok;
}

bool AddSubtractValueDialog::setDateTimeValue(qint64& value) const {
	if (m_operation == Add || m_operation == Subtract) {
		if (ui.cbType->currentIndex() == 0) { // add/subtract an absolute value
			const auto numberLocale = QLocale();
			bool ok;
			quint64 msecsValue = numberLocale.toLongLong(ui.leTimeValue->text(), &ok);
			if (!ok)
				return false;

			int unitIndex = ui.cbTimeUnits->currentIndex();
			if (unitIndex == 1)
				msecsValue *= 1000; // seconds
			else if (unitIndex == 2)
				msecsValue *= 60000; // minutes
			else if (unitIndex == 3)
				msecsValue *= 3600000; // hours
			else if (unitIndex == 4)
				msecsValue *= 8.64e+07; // days

			value = msecsValue;
		} else // add/subtract a difference
			value = ui.dteTimeValueEnd->dateTime().toMSecsSinceEpoch() - ui.dteTimeValueStart->dateTime().toMSecsSinceEpoch();
	}

	return true;
}
