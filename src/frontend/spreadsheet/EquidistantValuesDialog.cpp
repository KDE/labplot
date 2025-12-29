/*
	File                 : EquidistantValuesDialog.cpp
	Project              : LabPlot
	Description          : Dialog for generating equidistant numbers
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2014-2025 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "EquidistantValuesDialog.h"
#include "backend/core/Settings.h"
#include "backend/core/column/Column.h"
#include "backend/core/datatypes/DateTime2StringFilter.h"
#include "backend/lib/macros.h"
#include "backend/spreadsheet/Spreadsheet.h"

#include <QDialogButtonBox>
#include <QMessageBox>
#include <QPushButton>
#include <QWindow>

#include <KLocalizedString>
#include <KWindowConfig>

#include <cmath>

/*!
	\class EquidistantValuesDialog
	\brief Dialog for equidistant values.

	\ingroup frontend
 */

EquidistantValuesDialog::EquidistantValuesDialog(Spreadsheet* s, QWidget* parent, bool dateTimeMode)
	: QDialog(parent)
	, m_spreadsheet(s)
	, m_dateTimeMode(dateTimeMode) {
	Q_ASSERT(m_spreadsheet);

	auto* mainWidget = new QWidget(this);
	ui.setupUi(mainWidget);
	auto* layout = new QVBoxLayout(this);

	auto* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
	ui.gridLayout->addWidget(buttonBox);
	m_okButton = buttonBox->button(QDialogButtonBox::Ok);
	m_okButton->setText(i18n("&Generate"));
	m_okButton->setToolTip(i18n("Generate equidistant values"));

	connect(buttonBox->button(QDialogButtonBox::Cancel), &QPushButton::clicked, this, &EquidistantValuesDialog::close);
	connect(buttonBox, &QDialogButtonBox::accepted, this, &EquidistantValuesDialog::accept);
	connect(buttonBox, &QDialogButtonBox::rejected, this, &EquidistantValuesDialog::reject);

	layout->addWidget(mainWidget);
	layout->addWidget(buttonBox);
	setLayout(layout);
	setAttribute(Qt::WA_DeleteOnClose);

	ui.cbType->addItem(i18n("Number"), static_cast<int>(Type::FixedNumber));
	ui.cbType->addItem(i18n("Increment"), static_cast<int>(Type::FixedIncrement));
	ui.cbType->addItem(i18n("Number and Increment"), static_cast<int>(Type::FixedNumberIncrement));

	ui.cbIncrementDateTimeUnit->addItem(i18n("Years"), static_cast<int>(DateTimeUnit::Year));
	ui.cbIncrementDateTimeUnit->addItem(i18n("Months"), static_cast<int>(DateTimeUnit::Month));
	ui.cbIncrementDateTimeUnit->addItem(i18n("Days"), static_cast<int>(DateTimeUnit::Day));
	ui.cbIncrementDateTimeUnit->addItem(i18n("Hours"), static_cast<int>(DateTimeUnit::Hour));
	ui.cbIncrementDateTimeUnit->addItem(i18n("Minutes"), static_cast<int>(DateTimeUnit::Minute));
	ui.cbIncrementDateTimeUnit->addItem(i18n("Seconds"), static_cast<int>(DateTimeUnit::Second));
	ui.cbIncrementDateTimeUnit->addItem(i18n("Milliseconds"), static_cast<int>(DateTimeUnit::Millisecond));

	ui.leFrom->setValidator(new QDoubleValidator(ui.leFrom));
	ui.leTo->setValidator(new QDoubleValidator(ui.leTo));
	ui.leIncrement->setValidator(new QDoubleValidator(ui.leIncrement));
	ui.leNumber->setValidator(new QIntValidator(ui.leNumber));
	ui.leIncrementDateTime->setValidator(new QIntValidator(ui.leIncrementDateTime));

	connect(ui.cbType, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &EquidistantValuesDialog::typeChanged);
	connect(ui.leFrom, &QLineEdit::textChanged, this, &EquidistantValuesDialog::checkValues);
	connect(ui.leTo, &QLineEdit::textChanged, this, &EquidistantValuesDialog::checkValues);
	connect(ui.leNumber, &QLineEdit::textChanged, this, &EquidistantValuesDialog::checkValues);
	connect(ui.leIncrement, &QLineEdit::textChanged, this, &EquidistantValuesDialog::checkValues);
	connect(m_okButton, &QPushButton::clicked, this, &EquidistantValuesDialog::generate);

	// restore saved settings if available
	create(); // ensure there's a window created
	auto conf = Settings::group(QStringLiteral("EquidistantValuesDialog"));
	if (conf.exists()) {
		KWindowConfig::restoreWindowSize(windowHandle(), conf);
		resize(windowHandle()->size()); // workaround for QTBUG-40584
	} else
		resize(QSize(300, 0).expandedTo(minimumSize()));

	const int type = conf.readEntry("Type", static_cast<int>(Type::FixedNumber));
	ui.cbType->setCurrentIndex(ui.cbType->findData(type));
	// no need to restore 'Number', it's set to the the spreadsheet size in setColumns()

	// settings for numeric
	// all values are saved as doubles, try to show them as int or long first
	double from = conf.readEntry("From", 1.);
	double to = conf.readEntry("To", 100.);
	double increment = conf.readEntry("Increment", 1.);
	setNumericValue(from, ui.leFrom);
	setNumericValue(to, ui.leTo);
	setNumericValue(increment, ui.leIncrement);

	// settings for datetime
	qint64 now = QDateTime::currentDateTime().toMSecsSinceEpoch();
	ui.dteFrom->setMSecsSinceEpochUTC(conf.readEntry("FromDateTime", now));
	ui.dteTo->setMSecsSinceEpochUTC(conf.readEntry("ToDateTime", now));
	ui.leIncrementDateTime->setText(QLocale().toString(conf.readEntry("IncrementDateTime", 1)));
	ui.cbIncrementDateTimeUnit->setCurrentIndex(conf.readEntry("IncrementDateTimeUnit", 0));
}

EquidistantValuesDialog::~EquidistantValuesDialog() {
	// save current settings
	auto conf = Settings::group(QStringLiteral("EquidistantValuesDialog"));
	KWindowConfig::saveWindowSize(windowHandle(), conf);
	const auto numberLocale = QLocale();

	conf.writeEntry("Type", ui.cbType->itemData(ui.cbType->currentIndex()).toInt());
	// no need to save&restore 'Number', it's set to the the spreadsheet size in setColumns()

	// settings for numeric
	conf.writeEntry("From", numberLocale.toDouble(ui.leFrom->text()));
	conf.writeEntry("To", numberLocale.toDouble(ui.leTo->text()));
	conf.writeEntry("Increment", numberLocale.toDouble(ui.leIncrement->text()));

	// settings for datetime
	conf.writeEntry("FromDateTime", ui.dteFrom->dateTime().toMSecsSinceEpoch());
	conf.writeEntry("ToDateTime", ui.dteTo->dateTime().toMSecsSinceEpoch());
	conf.writeEntry("IncrementDateTime", numberLocale.toDouble(ui.leIncrementDateTime->text()));
	conf.writeEntry("IncrementDateTimeUnit", ui.cbIncrementDateTimeUnit->currentIndex());
}

void EquidistantValuesDialog::setNumericValue(double value, QLineEdit* le) const {
	const auto numberLocale = QLocale();
	if (floor(value) == ceil(value)) {
		if (value <= std::numeric_limits<int>::max()) {
			int valueInt = static_cast<int>(value);
			le->setText(numberLocale.toString(valueInt));
		} else {
			qint64 valueInt = static_cast<qint64>(value);
			le->setText(numberLocale.toString(valueInt));
		}
	} else
		le->setText(numberLocale.toString(value));
}

void EquidistantValuesDialog::setColumns(const QVector<Column*>& columns) {
	m_columns = columns;
	ui.leNumber->setText(QLocale().toString(m_columns.first()->rowCount()));
	QString dateTimeFormat;

	// if the datetime mode is not forced, check the column modes to see what kind of data we have
	if (!m_dateTimeMode) {
		for (auto* col : m_columns) {
			const auto mode = col->columnMode();
			if (!m_hasDouble && mode == AbstractColumn::ColumnMode::Double)
				m_hasDouble = true;

			if (!m_hasInteger && mode == AbstractColumn::ColumnMode::Integer)
				m_hasInteger = true;

			if (!m_hasBigInteger && mode == AbstractColumn::ColumnMode::BigInt)
				m_hasBigInteger = true;

			if (!m_hasDateTime && mode == AbstractColumn::ColumnMode::DateTime) {
				m_hasDateTime = true;
				auto* filter = static_cast<DateTime2StringFilter*>(col->outputFilter());
				dateTimeFormat = filter->format();
			}
		}

		m_hasNumeric = m_hasDouble || m_hasInteger || m_hasBigInteger;
	} else {
		m_hasNumeric = false;
		m_hasDateTime = true;
	}

	ui.lNumeric->setVisible(m_hasNumeric);
	ui.lIncrement->setVisible(m_hasNumeric);
	ui.leIncrement->setVisible(m_hasNumeric);
	ui.lFrom->setVisible(m_hasNumeric);
	ui.leFrom->setVisible(m_hasNumeric);
	ui.lTo->setVisible(m_hasNumeric);
	ui.leTo->setVisible(m_hasNumeric);

	ui.lDateTime->setVisible(m_hasDateTime);
	ui.lIncrementDateTime->setVisible(m_hasDateTime);
	ui.frameIncrementDateTime->setVisible(m_hasDateTime);
	ui.lFromDateTime->setVisible(m_hasDateTime);
	ui.dteFrom->setVisible(m_hasDateTime);
	ui.lToDateTime->setVisible(m_hasDateTime);
	ui.dteTo->setVisible(m_hasDateTime);

	if (m_hasNumeric && m_hasDateTime) {
		ui.lNumeric->show();
		ui.lDateTime->show();
		setWindowTitle(i18nc("@title:window", "Equidistant Numeric and Date&Time Values"));
	} else {
		ui.lNumeric->hide();
		ui.lDateTime->hide();
		if (m_hasNumeric)
			setWindowTitle(i18nc("@title:window", "Equidistant Numeric Values"));
		else
			setWindowTitle(i18nc("@title:window", "Equidistant Date&Time Values"));
	}

	if (m_hasDateTime) {
		ui.dteFrom->setDisplayFormat(dateTimeFormat);
		ui.dteTo->setDisplayFormat(dateTimeFormat);
	}

	// call typeChanged() to adjust the visibility of the mode specific widgets to the current type
	typeChanged(ui.cbType->currentIndex());

	// resize the dialog to have the minimum height
	layout()->activate();
	resize(QSize(this->width(), 0).expandedTo(minimumSize()));
}

/*!
 * \brief called when the method type to generate values (fixed number of fixed increment)
 * was called. Shows/hides the corresponding widgets depending on the type and on the column modes.
 */
void EquidistantValuesDialog::typeChanged(int) {
	const auto type = static_cast<Type>(ui.cbType->currentData().toInt());

	ui.lNumber->setVisible(type != Type::FixedIncrement);
	ui.leNumber->setVisible(type != Type::FixedIncrement);

	if (m_hasNumeric) {
		ui.lIncrement->setVisible(type != Type::FixedNumber);
		ui.leIncrement->setVisible(type != Type::FixedNumber);
		ui.lTo->setVisible(type != Type::FixedNumberIncrement);
		ui.leTo->setVisible(type != Type::FixedNumberIncrement);
	}

	if (m_hasDateTime) {
		ui.lIncrementDateTime->setVisible(type != Type::FixedNumber);
		ui.frameIncrementDateTime->setVisible(type != Type::FixedNumber);
		ui.lToDateTime->setVisible(type != Type::FixedNumberIncrement);
		ui.dteTo->setVisible(type != Type::FixedNumberIncrement);
	}
}

/*!
 * \brief Checks the validness of the user input and enables/disables the ok-Button and showw
 * the tooltip accordingly.
 */
void EquidistantValuesDialog::checkValues() const {
	bool ok;
	if (m_hasNumeric) {
		const auto numberLocale = QLocale();
		const double start = numberLocale.toDouble(ui.leFrom->text(), &ok);
		if (!ok) {
			m_okButton->setToolTip(i18n("Invalid start value"));
			m_okButton->setEnabled(false);
			return;
		}

		const double end = numberLocale.toDouble(ui.leTo->text(), &ok);
		if (!ok || end < start) {
			m_okButton->setToolTip(i18n("Invalid end value, must be greater than the start value"));
			m_okButton->setEnabled(false);
			return;
		}
	}

	const auto type = static_cast<Type>(ui.cbType->currentData().toInt());
	switch (type) {
	case Type::FixedNumber: {
		bool valid = checkNumberValue();
		if (!valid)
			return;
		break;
	}
	case Type::FixedIncrement: {
		bool valid = checkIncrementValue();
		if (!valid)
			return;
		break;
	}
	case Type::FixedNumberIncrement: {
		bool valid = checkNumberValue();
		if (!valid)
			return;
		valid = checkIncrementValue();
		if (!valid)
			return;
		break;
	}
	}

	m_okButton->setToolTip(QString());
	m_okButton->setEnabled(true);
}

/*!
 * checks whether a valid integer value greater than 1 was provided for the parameter 'number'
 */
bool EquidistantValuesDialog::checkNumberValue() const {
	bool ok;
	const auto numberLocale = QLocale();
	const int number = numberLocale.toDouble(ui.leNumber->text(), &ok);
	if (!ok || number < 1) {
		m_okButton->setToolTip(i18n("The number of values to be generated must be greater than one"));
		m_okButton->setEnabled(false);
		return false;
	}

	return true;
}

/*!
 * checks whether a valid integer value was provided for the parameter 'increment'
 */
bool EquidistantValuesDialog::checkIncrementValue() const {
	bool ok;
	const auto numberLocale = QLocale();
	if (m_hasNumeric) {
		const double increment = numberLocale.toDouble(ui.leIncrement->text(), &ok);
		if (!ok || increment == 0.) {
			m_okButton->setToolTip(i18n("Invalid numeric increment value, must be greater than zero"));
			m_okButton->setEnabled(false);
			return false;
		}
	}

	if (m_hasDateTime) {
		const int increment = numberLocale.toInt(ui.leIncrementDateTime->text(), &ok);
		if (!ok || increment == 0) {
			m_okButton->setToolTip(i18n("Invalid Date&Time increment value, must be greater than zero"));
			m_okButton->setEnabled(false);
			return false;
		}
	}

	return true;
}

void EquidistantValuesDialog::generate() {
	QVector<int> newIntData;
	QVector<qint64> newBigIntData;
	QVector<double> newDoubleData;
	QVector<QDateTime> newDateTimeData;
	bool integerModePossible = false;
	bool bigIntRequired = false;

	WAIT_CURSOR_AUTO_RESET;
	bool rc = true;
	if (m_hasNumeric) {
		int number{0};
		double increment{0};

		// check the validness of the user input for numeric values
		const auto numberLocale = QLocale();
		bool ok;
		const double start = numberLocale.toDouble(ui.leFrom->text(), &ok);
		if (!ok) {
			DEBUG("Invalid double value for 'start'!")
			return;
		}

		const auto type = static_cast<Type>(ui.cbType->currentData().toInt());
		double end = 0.;
		if (type != Type::FixedNumberIncrement) {
			end = numberLocale.toDouble(ui.leTo->text(), &ok);
			if (!ok) {
				DEBUG("Invalid double value for 'end'!")
				return;
			}
		}

		switch (type) {
		case Type::FixedNumber: {
			// fixed number -> determine the increment
			number = QLocale().toInt(ui.leNumber->text(), &ok);
			if (!ok || number == 1) {
				DEBUG("Invalid integer value for 'number'!")
				return;
			}

			increment = (end - start) / (number - 1);
			break;
		}
		case Type::FixedIncrement: {
			// fixed increment -> determine the number
			increment = QLocale().toDouble(ui.leIncrement->text(), &ok);
			if (ok)
				number = (end - start) / increment + 1;
			break;
		}
		case Type::FixedNumberIncrement: {
			// fixed number and increment -> determine the end value
			number = QLocale().toInt(ui.leNumber->text(), &ok);
			if (!ok || number == 1) {
				DEBUG("Invalid integer value for 'number'!")
				return;
			}

			increment = QLocale().toDouble(ui.leIncrement->text(), &ok);
			if (!ok) {
				DEBUG("Invalid integer value for 'increment'!")
				return;
			}

			end = start + increment * number;
			break;
		}
		}

		// check whether we have integer values for the input parameters for start, begin and increment
		// which would allow to work with int values only or whether we need to convert the columns from int to double
		if (m_hasInteger || m_hasBigInteger) {
			if (floor(start) == ceil(start) && floor(end) == ceil(end) && floor(increment) == ceil(increment)) {
				integerModePossible = true;
				if (start > std::numeric_limits<int>::max() || end > std::numeric_limits<int>::max())
					bigIntRequired = true;

				if (m_hasBigInteger || bigIntRequired)
					rc = generateBigInt(newBigIntData, start, increment, number);
				else
					rc = generateInt(newIntData, start, increment, number);
			}

			if (!rc) {
				return;
			}
		}
		if (m_hasDouble || ((m_hasInteger || m_hasBigInteger) && !integerModePossible)) {
			rc = generateDouble(newDoubleData, start, increment, number);
			if (!rc) {
				return;
			}
		}
	}

	if (m_hasDateTime) {
		bool ok;
		const int number = QLocale().toInt(ui.leNumber->text(), &ok);
		const auto increment = QLocale().toInt(ui.leIncrementDateTime->text(), &ok);
		const auto unit = static_cast<DateTimeUnit>(ui.cbIncrementDateTimeUnit->currentData().toInt());
		const auto type = static_cast<Type>(ui.cbType->currentData().toInt());
		const auto start = ui.dteFrom->dateTime();
		QDateTime end;
		if (type != Type::FixedNumberIncrement)
			end = ui.dteTo->dateTime();
		else {
			switch (unit) {
			case DateTimeUnit::Year:
				end = start.addYears((number - 1) * increment);
				break;
			case DateTimeUnit::Month:
				end = start.addMonths((number - 1) * increment);
				break;
			case DateTimeUnit::Day:
				end = start.addDays((number - 1) * increment);
				break;
			case DateTimeUnit::Hour: {
				const int seconds = increment * 60 * 60;
				end = start.addSecs((number - 1) * seconds);
				break;
			}
			case DateTimeUnit::Minute: {
				const int seconds = increment * 60;
				end = start.addSecs((number - 1) * seconds);
				break;
			}
			case DateTimeUnit::Second:
				end = start.addSecs((number - 1) * increment);
				break;
			case DateTimeUnit::Millisecond:
				end = start.addMSecs((number - 1) * increment);
				break;
			}
		}

		rc = generateDateTime(newDateTimeData, type, start, end, number, increment, unit);
		if (!rc) {
			return;
		}
	}

	m_spreadsheet->beginMacro(
		i18np("%1: fill column with equidistant numbers", "%1: fill columns with equidistant numbers", m_spreadsheet->name(), m_columns.size()));

	// Adjust the sizes of the spreadsheet and of the column data vectors
	// Note, when numeric and datetime columns are selected, the input parameters are
	// different which can result into different number of values generated for each type.
	int maxSize = std::max(newDoubleData.size(), newIntData.size());
	maxSize = std::max(maxSize, static_cast<int>(newBigIntData.size()));
	maxSize = std::max(maxSize, static_cast<int>(newDateTimeData.size()));
	if (m_spreadsheet->rowCount() < maxSize)
		m_spreadsheet->setRowCount(maxSize);
	else
		maxSize = m_spreadsheet->rowCount();

	// make sure all data vectors have the size of the spreadsheet, extend if needed
	if (m_hasDouble && newDoubleData.size() != maxSize) {
		const int size = newDoubleData.size();
		newDoubleData.resize(maxSize);
		for (int i = 0; i < maxSize - size; ++i)
			newDoubleData[size + i] = std::numeric_limits<double>::quiet_NaN();
	}

	if (m_hasInteger && newIntData.size() != maxSize) {
		const int size = newIntData.size();
		newIntData.resize(maxSize);
		for (int i = 0; i < maxSize - size; ++i)
			newIntData[size + i] = 0;
	}

	if (m_hasBigInteger && newBigIntData.size() != maxSize) {
		const int size = newBigIntData.size();
		newBigIntData.resize(maxSize);
		for (int i = 0; i < maxSize - size; ++i)
			newBigIntData[size + i] = 0;
	}

	if (m_hasDateTime && newDateTimeData.size() != maxSize) {
		const int size = newDateTimeData.size();
		newDateTimeData.resize(maxSize);
		for (int i = 0; i < maxSize - size; ++i)
			newDateTimeData[size + i] = QDateTime();
	}

	// set the vectors with the generated data in the columns and adjust the column mode, if needed
	for (auto* col : m_columns) {
		col->clearFormula(); // clear the potentially available column formula

		if (m_dateTimeMode) {
			col->setColumnMode(AbstractColumn::ColumnMode::DateTime);
			col->setDateTimes(newDateTimeData);
			continue;
		}

		switch (col->columnMode()) {
		case AbstractColumn::ColumnMode::Double:
			col->setValues(newDoubleData);
			break;
		case AbstractColumn::ColumnMode::Integer: {
			if (integerModePossible) {
				if (!bigIntRequired)
					col->setIntegers(newIntData);
				else {
					col->setColumnMode(AbstractColumn::ColumnMode::BigInt);
					col->setBigInts(newBigIntData);
				}
			} else {
				col->setColumnMode(AbstractColumn::ColumnMode::Double);
				col->setValues(newDoubleData);
			}
			break;
		}
		case AbstractColumn::ColumnMode::BigInt: {
			if (integerModePossible)
				col->setBigInts(newBigIntData);
			else {
				col->setColumnMode(AbstractColumn::ColumnMode::Double);
				col->setValues(newDoubleData);
			}
			break;
		}
		case AbstractColumn::ColumnMode::DateTime:
			col->setDateTimes(newDateTimeData);
			break;
		case AbstractColumn::ColumnMode::Month:
		case AbstractColumn::ColumnMode::Day:
		case AbstractColumn::ColumnMode::Text:
			// not supported
			break;
		}
	}

	m_spreadsheet->endMacro();
}

/*!
 * \brief Helper function generating equidistant double values based on the user input
 * \param newData - vector of doubles for the new data to be generated.
 * \return returns \c false if not enough memory available to create new data, returns \c true otherwise.
 */
bool EquidistantValuesDialog::generateDouble(QVector<double>& newData, double start, double increment, int number) {
	try {
		newData.resize(number);
	} catch (std::bad_alloc&) {
		RESET_CURSOR;
		QMessageBox::critical(this, i18n("Failed to allocate memory"), i18n("Not enough memory to perform this operation."));
		return false;
	}

	for (int i = 0; i < number; ++i)
		newData[i] = start + increment * i;

	return true;
}

/*!
 * \brief Helper function generating equidistant integer values based on the user input
 * \param newData - vector of integers for the new data to be generated.
 * \return returns \c false if not enough memory available to create new data, returns \c true otherwise.
 */
bool EquidistantValuesDialog::generateInt(QVector<int>& newData, int start, int increment, int number) {
	try {
		newData.resize(number);
	} catch (std::bad_alloc&) {
		RESET_CURSOR;
		QMessageBox::critical(this, i18n("Failed to allocate memory"), i18n("Not enough memory to perform this operation."));
		return false;
	}

	for (int i = 0; i < number; ++i)
		newData[i] = start + increment * i;

	return true;
}

/*!
 * \brief Helper function generating equidistant big integer (aka long, aka int64) values based on the user input
 * \param newData - vector of int64's for the new data to be generated.
 * \return returns \c false if not enough memory available to create new data, returns \c true otherwise.
 */
bool EquidistantValuesDialog::generateBigInt(QVector<qint64>& newData, qint64 start, qint64 increment, int number) {
	try {
		newData.resize(number);
	} catch (std::bad_alloc&) {
		RESET_CURSOR;
		QMessageBox::critical(this, i18n("Failed to allocate memory"), i18n("Not enough memory to perform this operation."));
		return false;
	}

	for (int i = 0; i < number; ++i)
		newData[i] = start + increment * i;

	return true;
}

/*!
 * \brief Helper function generating equidistand DateTime values based on the user input
 * \param newData - vector of QDateTimes for the new data to be generated.
 * \return \c false if the user input was wrong or not enough memory available to create new data,
 * \c true if the generation of values was successful.
 */
bool EquidistantValuesDialog::generateDateTime(QVector<QDateTime>& newData,
											   Type type,
											   const QDateTime& start,
											   const QDateTime& end,
											   int number,
											   int increment,
											   DateTimeUnit unit) {
	switch (type) {
	case Type::FixedNumber: {
		const auto startValue = start.toMSecsSinceEpoch();
		const auto endValue = end.toMSecsSinceEpoch();
		int increment = 1;
		if (number != 1)
			increment = (endValue - startValue) / (number - 1);

		try {
			newData.resize(number);
		} catch (std::bad_alloc&) {
			RESET_CURSOR;
			QMessageBox::critical(this, i18n("Failed to allocate memory"), i18n("Not enough memory to perform this operation."));
			return false;
		}

		for (int i = 0; i < number; ++i)
			newData[i] = QDateTime::fromMSecsSinceEpoch(startValue + increment * i, Qt::UTC);

		break;
	}
	case Type::FixedIncrement:
	case Type::FixedNumberIncrement: {
		QDateTime value = start;
		switch (unit) {
		case DateTimeUnit::Year:
			while (value <= end) {
				newData << value;
				value = value.addYears(increment);
			}
			break;
		case DateTimeUnit::Month:
			while (value <= end) {
				newData << value;
				value = value.addMonths(increment);
			}
			break;
		case DateTimeUnit::Day:
			while (value <= end) {
				newData << value;
				value = value.addDays(increment);
			}
			break;
		case DateTimeUnit::Hour: {
			const int seconds = increment * 60 * 60;
			while (value <= end) {
				newData << value;
				value = value.addSecs(seconds);
			}
			break;
		}
		case DateTimeUnit::Minute: {
			const int seconds = increment * 60;
			while (value <= end) {
				newData << value;
				value = value.addSecs(seconds);
			}
			break;
		}
		case DateTimeUnit::Second:
			while (value <= end) {
				newData << value;
				value = value.addSecs(increment);
			}
			break;
		case DateTimeUnit::Millisecond:
			while (value <= end) {
				newData << value;
				value = value.addMSecs(increment);
			}
			break;
		}

		break;
	}
	}

	return true;
}

// **********************************************************
// *********** Helper functions used in the tests ***********
// **********************************************************
void EquidistantValuesDialog::setType(Type type) const {
	ui.cbType->setCurrentIndex(ui.cbType->findData(static_cast<int>(type)));
}

void EquidistantValuesDialog::setNumber(int value) const {
	ui.leNumber->setText(QLocale().toString(value));
}

void EquidistantValuesDialog::setIncrement(double value) const {
	setNumericValue(value, ui.leIncrement);
}

void EquidistantValuesDialog::setIncrementDateTime(int value) const {
	ui.leIncrementDateTime->setText(QLocale().toString(value));
}

void EquidistantValuesDialog::setIncrementDateTimeUnit(DateTimeUnit value) {
	ui.cbIncrementDateTimeUnit->setCurrentIndex(ui.cbIncrementDateTimeUnit->findData(static_cast<int>(value)));
}

void EquidistantValuesDialog::setFromValue(double value) const {
	setNumericValue(value, ui.leFrom);
}

void EquidistantValuesDialog::setToValue(double value) const {
	setNumericValue(value, ui.leTo);
}

void EquidistantValuesDialog::setFromDateTime(qint64 value) const {
	ui.dteFrom->setMSecsSinceEpochUTC(value);
}

void EquidistantValuesDialog::setToDateTime(qint64 value) const {
	ui.dteTo->setMSecsSinceEpochUTC(value);
}
