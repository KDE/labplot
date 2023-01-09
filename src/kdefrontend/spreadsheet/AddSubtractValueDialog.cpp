/*
	File                 : AddSubtractValueDialog.cpp
	Project              : LabPlot
	Description          : Dialog for adding/subtracting a value from column values
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2018-2023 Alexander Semke <alexander.semke@web.de>
	SPDX-FileCopyrightText: 2020 Stefan Gerlach <stefan.gerlach@uni.kn>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "AddSubtractValueDialog.h"
#include "backend/core/Project.h"
#include "backend/core/column/Column.h"
#include "backend/core/datatypes/DateTime2StringFilter.h"
#include "backend/lib/macros.h"
#include "backend/matrix/Matrix.h"
#include "backend/spreadsheet/Spreadsheet.h"

#include "backend/worksheet/Background.h"
#include "backend/worksheet/Line.h"
#include "backend/worksheet/TextLabel.h"
#include "backend/worksheet/Worksheet.h"
#include "backend/worksheet/plots/PlotArea.h"
#include "backend/worksheet/plots/cartesian/Axis.h"
#include "backend/worksheet/plots/cartesian/CartesianPlot.h"
#include "backend/worksheet/plots/cartesian/XYCurve.h"

#include <KLocalizedString>
#include <KMessageBox>
#include <KWindowConfig>

#include <QDialogButtonBox>
#include <QPushButton>
#include <QWindow>

#include <cmath>

enum class ValueType { CustomValue, Difference, Minimum, Maximum, Median, Mean };
/*!
	\class AddSubtractValueDialog
	\brief Dialog for adding/subtracting a value from column values.

	\ingroup kdefrontend
 */

AddSubtractValueDialog::AddSubtractValueDialog(Spreadsheet* s, const QVector<Column*>& columns, Operation op, QWidget* parent)
	: QDialog(parent)
	, m_spreadsheet(s)
	, m_columns(columns)
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

	updateWidgetsVisiblity();
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

void AddSubtractValueDialog::init() {
	ui.setupUi(this);
	setAttribute(Qt::WA_DeleteOnClose);

	auto* btnBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
	ui.gridLayout->addWidget(btnBox, 11, 0, 1, 3);
	m_okButton = btnBox->button(QDialogButtonBox::Ok);

	switch (m_operation) {
	case Add:
		setWindowTitle(i18nc("@title:window", "Add Value"));
		m_okButton->setText(i18n("&Add"));
		break;
	case Subtract:
		setWindowTitle(i18nc("@title:window", "Subtract Value"));
		ui.lType->setText(i18n("Subtract:")); // only relevant for Add and for Subtract, Add is set in the ui file
		m_okButton->setText(i18n("&Subtract"));
		break;
	case Multiply:
		setWindowTitle(i18nc("@title:window", "Multiply by Value"));
		m_okButton->setText(i18n("&Multiply"));
		break;
	case Divide:
		setWindowTitle(i18nc("@title:window", "Divide by Value"));
		m_okButton->setText(i18n("&Divide"));
		break;
	}

	if (m_operation == Add || m_operation == Subtract) {
		ui.cbType->addItem(i18n("Custom Value"), static_cast<int>(ValueType::CustomValue));
		ui.cbType->addItem(i18n("Difference"), static_cast<int>(ValueType::Difference));
		for (int i = 0; i < ENUM_COUNT(AbstractColumn, TimeUnit); i++)
			ui.cbTimeUnits->addItem(AbstractColumn::timeUnitString((AbstractColumn::TimeUnit)i));
	} else {
		ui.lType->hide();
		ui.cbType->hide();
		ui.lPreview->hide();
		ui.chbPreview->hide();
		ui.framePreview->hide();
	}

	connect(m_okButton, &QPushButton::clicked, this, &AddSubtractValueDialog::generate);
	connect(btnBox, &QDialogButtonBox::accepted, this, &AddSubtractValueDialog::accept);
	connect(btnBox, &QDialogButtonBox::rejected, this, &AddSubtractValueDialog::reject);
	connect(ui.cbType, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &AddSubtractValueDialog::typeChanged);
	connect(ui.chbPreview, &QCheckBox::clicked, this, &AddSubtractValueDialog::previewChanged);
	connect(ui.leValue, &QLineEdit::textChanged, this, [=]() {
		m_okButton->setEnabled(!ui.leValue->text().isEmpty());
		updatePreview();
	});
	connect(ui.leValueStart, &QLineEdit::textChanged, this, &AddSubtractValueDialog::updatePreview);
	connect(ui.leValueEnd, &QLineEdit::textChanged, this, &AddSubtractValueDialog::updatePreview);

	if (m_spreadsheet)
		processColumns();

	// restore saved settings if available
	create(); // ensure there's a window created
	KConfigGroup conf(KSharedConfig::openConfig(), "AddSubtractValueDialog");
	if (conf.exists()) {
		int typeIndex = ui.cbType->findData(conf.readEntry("Type", 0));
		if (typeIndex != -1)
			ui.cbType->setCurrentIndex(typeIndex);
		else
			ui.cbType->setCurrentIndex(0);

		KWindowConfig::restoreWindowSize(windowHandle(), conf);
		resize(windowHandle()->size()); // workaround for QTBUG-40584

		if (m_operation == Add || m_operation == Subtract) {
			ui.chbPreview->setChecked(conf.readEntry("Preview", false));
			previewChanged(ui.chbPreview->isChecked());
		}
	} else
		resize(QSize(300, 0).expandedTo(minimumSize()));
}

AddSubtractValueDialog::~AddSubtractValueDialog() {
	delete m_project;

	KConfigGroup conf(KSharedConfig::openConfig(), "AddSubtractValueDialog");
	conf.writeEntry("Type", ui.cbType->currentData().toInt());
	conf.writeEntry("Preview", ui.chbPreview->isChecked());
	KWindowConfig::saveWindowSize(windowHandle(), conf);
}

void AddSubtractValueDialog::processColumns() {
	// depending on the current column mode, activate/deactivate the corresponding widgets
	// and show the first valid value in the first selected column as the value to add/subtract
	const auto* column = m_columns.first();
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

	if (m_operation == Subtract && m_numeric) {
		int curIndex = ui.cbType->currentIndex();
		ui.cbType->insertItem(0, i18n("Minimum"), static_cast<int>(ValueType::Minimum));
		ui.cbType->insertItem(1, i18n("Maximum"), static_cast<int>(ValueType::Maximum));
		ui.cbType->insertItem(2, i18n("Median"), static_cast<int>(ValueType::Median));
		ui.cbType->insertItem(3, i18n("Mean"), static_cast<int>(ValueType::Mean));
		ui.cbType->insertSeparator(4);

		ui.cbType->setCurrentIndex(curIndex);
	}

	updateWidgetsVisiblity();
}

void AddSubtractValueDialog::typeChanged(int index) {
	auto type = static_cast<ValueType>(ui.cbType->itemData(index).toInt());
	bool diff = (type == ValueType::Difference);
	if (m_numeric) {
		ui.lValue->setVisible(!diff);
		ui.leValue->setVisible(!diff);
		ui.lValueStart->setVisible(diff);
		ui.leValueStart->setVisible(diff);
		ui.lValueEnd->setVisible(diff);
		ui.leValueEnd->setVisible(diff);

		if (type == ValueType::Minimum || type == ValueType::Maximum || type == ValueType::Median || type == ValueType::Mean) {
			if (m_columns.count() > 1) {
				ui.lValue->hide();
				ui.leValue->hide();
			} else {
				// one single column was selected, show the actual minimum value of it, etc.
				const auto& statistics = m_columns.constFirst()->statistics();
				double value = 0.;
				if (type == ValueType::Minimum)
					value = statistics.minimum;
				else if (type == ValueType::Maximum)
					value = statistics.maximum;
				else if (type == ValueType::Median)
					value = statistics.median;
				else if (type == ValueType::Mean)
					value = statistics.arithmeticMean;

				const auto numberLocale = QLocale();
				ui.leValue->setText(numberLocale.toString(value));
				ui.leValue->setEnabled(false);
			}
		} else
			ui.leValue->setEnabled(true);
	} else {
		ui.lTimeValue->setVisible(!diff);
		ui.leTimeValue->setVisible(!diff);
		ui.cbTimeUnits->setVisible(!diff);
		ui.lTimeValueStart->setVisible(diff);
		ui.dteTimeValueStart->setVisible(diff);
		ui.lTimeValueEnd->setVisible(diff);
		ui.dteTimeValueEnd->setVisible(diff);
	}

	if (m_spreadsheet && (m_operation == Add || m_operation == Subtract))
		updatePreview();
	else {
		ui.lPreview->hide();
		ui.chbPreview->hide();
		ui.framePreview->hide();
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

void AddSubtractValueDialog::previewChanged(bool state) {
	bool visible = state && (m_operation == Add || m_operation == Subtract);
	if (visible) {
		// initialize the worksheet and the plot if not done yet
		if (!m_project) {
			// create project and worksheet
			m_project = new Project();
			auto* ws = new Worksheet(QString());
			ws->setUseViewSize(true);
			ws->setLayoutTopMargin(0.);
			ws->setLayoutBottomMargin(0.);
			ws->setLayoutLeftMargin(0.);
			ws->setLayoutRightMargin(0.);
			m_project->addChild(ws);

			// add plot
			auto* plot = new CartesianPlot(QString());
			plot->setSuppressRetransform(true);
			plot->setSymmetricPadding(false);
			const double padding = Worksheet::convertToSceneUnits(0.5, Worksheet::Unit::Centimeter);
			plot->setRightPadding(padding);
			plot->setVerticalPadding(padding);
			plot->setBottomPadding(padding);
			plot->plotArea()->borderLine()->setStyle(Qt::NoPen);

			// y-axis
			auto* axis = new Axis(QLatin1String("y"), Axis::Orientation::Vertical);
			axis->setDefault(true);
			axis->setSuppressRetransform(true);
			plot->addChild(axis);
			axis->setPosition(Axis::Position::Left);
			axis->setMajorTicksDirection(Axis::ticksIn);
			axis->majorGridLine()->setStyle(Qt::NoPen);
			axis->setMinorTicksDirection(Axis::noTicks);
			axis->title()->setText(QString());
			auto font = axis->labelsFont();
			font.setPixelSize(Worksheet::convertToSceneUnits(8, Worksheet::Unit::Point));
			axis->setLabelsFont(font);
			axis->setSuppressRetransform(false);

			ws->addChild(plot);
			plot->setSuppressRetransform(false);

			// add the curve for the original data
			auto* xColumn = new Column(QLatin1String("x"), AbstractColumn::ColumnMode::Integer);
			const auto* yColumn = m_columns.constFirst();
			QVector<int> xData;
			xData.resize(yColumn->rowCount());
			for (int i = 0; i < yColumn->rowCount(); ++i)
				xData[i] = i;
			xColumn->setIntegers(xData);

			m_curveOrigin = new XYCurve(i18n("raw data"));
			m_curveOrigin->setXColumn(xColumn);
			m_curveOrigin->setYColumn(yColumn);
			plot->addChild(m_curveOrigin);
			m_curveOrigin->line()->setWidth(0.);

			// add the curve for the data to be subtracted
			m_curveBaseline = new XYCurve(i18n("baseline"));
			auto* xColumnBaseline = new Column(QLatin1String("xBaseline"), AbstractColumn::ColumnMode::Integer);
			xColumnBaseline->setIntegerAt(0, 0);
			xColumnBaseline->setIntegerAt(1, xColumn->rowCount() - 1);
			auto* yColumnBaseline = new Column(QLatin1String("yBaseline"));
			m_curveBaseline->setXColumn(xColumnBaseline);
			m_curveBaseline->setYColumn(yColumnBaseline);
			plot->addChild(m_curveBaseline);
			m_curveBaseline->line()->setWidth(0.);

			// add the curve for the result data
			m_curveResult = new XYCurve(i18n("result"));
			m_curveResult->setXColumn(xColumn);
			auto* yColumnResult = new Column(QLatin1String("yResult"), yColumn->columnMode());
			m_curveResult->setYColumn(yColumnResult);
			plot->addChild(m_curveResult);
			m_curveResult->line()->setWidth(0.);

			plot->addLegend();
			// ws->setTheme(QLatin1String("Tufte"));

			updatePreview();

			auto* layout = new QVBoxLayout(ui.framePreview);
			layout->setSpacing(0);
			layout->addWidget(ws->view());
			ws->setInteractive(false);
			ui.framePreview->setLayout(layout);
		}
	}

	ui.framePreview->setVisible(visible);

	// resize the dialog
	layout()->activate();
	resize(QSize(this->width(), 0).expandedTo(minimumSize()));
}

void AddSubtractValueDialog::updatePreview() {
	if (!ui.framePreview->isVisible())
		return;

	double value = 0.;
	setDoubleValue(value);

	// y for the baseline curve
	auto* column = const_cast<AbstractColumn*>(m_curveBaseline->yColumn());
	column->setValueAt(0, value);
	column->setValueAt(1, value);

	// y for the result curve
	column = const_cast<AbstractColumn*>(m_curveResult->yColumn());
	column->copy(m_columns.constFirst());
	generateForColumn(static_cast<Column*>(column), 0);
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

	// if custom value or a difference is used, check whether a proper value was entered
	auto type = static_cast<ValueType>(ui.cbType->itemData(ui.cbType->currentIndex()).toInt());
	if (type == ValueType::CustomValue || type == ValueType::Difference) {
		bool ok;
		const auto mode = m_columns.first()->columnMode();
		if (mode == AbstractColumn::ColumnMode::Integer) {
			int value;
			ok = setIntValue(value);
			if (!ok) {
				KMessageBox::error(this, i18n("Wrong numeric value provided."));
				return;
			}
		} else if (mode == AbstractColumn::ColumnMode::BigInt) {
			qint64 value;
			ok = setBigIntValue(value);
			if (!ok) {
				KMessageBox::error(this, i18n("Wrong numeric value provided."));
				return;
			}
		} else if (mode == AbstractColumn::ColumnMode::Double) {
			double value;
			ok = setDoubleValue(value);
			if (!ok) {
				KMessageBox::error(this, i18n("Wrong numeric value provided."));
				return;
			}
		} else { // datetime
			qint64 value;
			ok = setDateTimeValue(value);
			if (!ok) {
				KMessageBox::error(this, i18n("Wrong numeric value provided."));
				return;
			}
		}
	}

	WAIT_CURSOR;
	const auto& msg = getMessage(m_spreadsheet->name());
	m_spreadsheet->beginMacro(msg);

	int colIndex = 0;
	for (auto* col : m_columns) {
		generateForColumn(col, colIndex);
		++colIndex;
	}

	m_spreadsheet->endMacro();
	RESET_CURSOR;
}

void AddSubtractValueDialog::generateForColumn(Column* col, int colIndex) {
	const auto mode = m_columns.constFirst()->columnMode();
	const int rows = m_spreadsheet->rowCount();

	if (mode == AbstractColumn::ColumnMode::Integer) {
		int value;
		setIntValue(value, colIndex);
		auto* data = static_cast<QVector<int>*>(col->data());
		QVector<int> new_data(rows);

		switch (m_operation) {
		case Subtract:
		case Add: {
			if (m_operation == Subtract)
				value *= -1.;

			for (int i = 0; i < rows; ++i)
				new_data[i] = data->operator[](i) + value;

			col->replaceInteger(0, new_data);
			break;
		}
		case Multiply:
			for (int i = 0; i < rows; ++i)
				new_data[i] = data->operator[](i) * value;

			col->replaceInteger(0, new_data);
			break;
		case Divide:
			for (int i = 0; i < rows; ++i)
				new_data[i] = data->operator[](i) / value;

			col->replaceInteger(0, new_data);
			break;
		}
	} else if (mode == AbstractColumn::ColumnMode::BigInt) {
		qint64 value;
		setBigIntValue(value, colIndex);
		auto* data = static_cast<QVector<qint64>*>(col->data());
		QVector<qint64> new_data(rows);

		switch (m_operation) {
		case Subtract:
		case Add: {
			if (m_operation == Subtract)
				value *= -1.;

			for (int i = 0; i < rows; ++i)
				new_data[i] = data->operator[](i) + value;

			col->replaceBigInt(0, new_data);
			break;
		}
		case Multiply:
			for (int i = 0; i < rows; ++i)
				new_data[i] = data->operator[](i) * value;

			col->replaceBigInt(0, new_data);
			break;
		case Divide:
			for (int i = 0; i < rows; ++i)
				new_data[i] = data->operator[](i) / value;

			col->replaceBigInt(0, new_data);
			break;
		}
	} else if (mode == AbstractColumn::ColumnMode::Double) {
		double value;
		setDoubleValue(value, colIndex);
		auto* data = static_cast<QVector<double>*>(col->data());
		QVector<double> new_data(rows);

		switch (m_operation) {
		case Subtract:
		case Add: {
			if (m_operation == Subtract)
				value *= -1.;

			for (int i = 0; i < rows; ++i)
				new_data[i] = data->operator[](i) + value;

			col->replaceValues(0, new_data);
			break;
		}
		case Multiply:
			for (int i = 0; i < rows; ++i)
				new_data[i] = data->operator[](i) * value;

			col->replaceValues(0, new_data);
			break;
		case Divide:
			for (int i = 0; i < rows; ++i)
				new_data[i] = data->operator[](i) / value;

			col->replaceValues(0, new_data);
			break;
		}
	} else { // datetime
		qint64 value;
		setDateTimeValue(value);
		auto* data = static_cast<QVector<QDateTime>*>(col->data());
		QVector<QDateTime> new_data(rows);

		switch (m_operation) {
		case Subtract:
			value *= -1;
			// fall through
		case Add:
			for (int i = 0; i < rows; ++i)
				new_data[i] = QDateTime::fromMSecsSinceEpoch(data->operator[](i).toMSecsSinceEpoch() + value, Qt::UTC);

			col->replaceDateTimes(0, new_data);
			break;
		case Multiply:
		case Divide:
			break;
		}
	}
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

bool AddSubtractValueDialog::setIntValue(int& value, int columnIndex) const {
	if (columnIndex < 0 || columnIndex >= m_columns.count()) // should never happen...
		return false;

	bool ok = false;
	const auto numberLocale = QLocale();
	if (m_operation == Add || m_operation == Subtract) {
		auto type = static_cast<ValueType>(ui.cbType->itemData(ui.cbType->currentIndex()).toInt());
		switch (type) {
		case ValueType::CustomValue:
			value = numberLocale.toInt(ui.leValue->text(), &ok);
			break;
		case ValueType::Difference:
			value = numberLocale.toInt(ui.leValueEnd->text(), &ok) - numberLocale.toInt(ui.leValueStart->text(), &ok);
			break;
		case ValueType::Minimum:
			value = m_columns.at(columnIndex)->statistics().minimum;
			break;
		case ValueType::Maximum:
			value = m_columns.at(columnIndex)->statistics().maximum;
			break;
		case ValueType::Median:
			value = round(m_columns.at(columnIndex)->statistics().median);
			break;
		case ValueType::Mean:
			value = round(m_columns.at(columnIndex)->statistics().arithmeticMean);
			break;
		}
	} else
		value = numberLocale.toInt(ui.leValue->text(), &ok);

	return ok;
}

bool AddSubtractValueDialog::setBigIntValue(qint64& value, int columnIndex) const {
	if (columnIndex < 0 || columnIndex >= m_columns.count()) // should never happen...
		return false;

	bool ok = true;
	const auto numberLocale = QLocale();
	if (m_operation == Add || m_operation == Subtract) {
		auto type = static_cast<ValueType>(ui.cbType->itemData(ui.cbType->currentIndex()).toInt());
		switch (type) {
		case ValueType::CustomValue:
			value = numberLocale.toLongLong(ui.leValue->text(), &ok);
			break;
		case ValueType::Difference:
			value = numberLocale.toLongLong(ui.leValueEnd->text(), &ok) - numberLocale.toLongLong(ui.leValueStart->text(), &ok);
			break;
		case ValueType::Minimum:
			value = m_columns.at(columnIndex)->statistics().minimum;
			break;
		case ValueType::Maximum:
			value = m_columns.at(columnIndex)->statistics().maximum;
			break;
		case ValueType::Median:
			value = round(m_columns.at(columnIndex)->statistics().median);
			break;
		case ValueType::Mean:
			value = round(m_columns.at(columnIndex)->statistics().arithmeticMean);
			break;
		}
	} else
		value = numberLocale.toLongLong(ui.leValue->text(), &ok);

	return ok;
}

bool AddSubtractValueDialog::setDoubleValue(double& value, int columnIndex) const {
	if (columnIndex < 0 || columnIndex >= m_columns.count()) // should never happen...
		return false;

	bool ok = true;
	const auto numberLocale = QLocale();
	if (m_operation == Add || m_operation == Subtract) {
		auto type = static_cast<ValueType>(ui.cbType->itemData(ui.cbType->currentIndex()).toInt());
		switch (type) {
		case ValueType::CustomValue:
			value = numberLocale.toDouble(ui.leValue->text(), &ok);
			break;
		case ValueType::Difference:
			value = numberLocale.toDouble(ui.leValueEnd->text(), &ok) - numberLocale.toDouble(ui.leValueStart->text(), &ok);
			break;
		case ValueType::Minimum:
			value = m_columns.at(columnIndex)->statistics().minimum;
			break;
		case ValueType::Maximum:
			value = m_columns.at(columnIndex)->statistics().maximum;
			break;
		case ValueType::Median:
			value = m_columns.at(columnIndex)->statistics().median;
			break;
		case ValueType::Mean:
			value = m_columns.at(columnIndex)->statistics().arithmeticMean;
			break;
		}
	} else
		value = numberLocale.toDouble(ui.leValue->text(), &ok);

	return ok;
}

bool AddSubtractValueDialog::setDateTimeValue(qint64& value, int columnIndex) const {
	if (columnIndex < 0 || columnIndex >= m_columns.count()) // should never happen...
		return false;

	if (m_operation == Add || m_operation == Subtract) {
		auto type = static_cast<ValueType>(ui.cbType->itemData(ui.cbType->currentIndex()).toInt());
		if (type == ValueType::Difference) { // add/subtract an absolute value
			const auto numberLocale = QLocale();
			bool ok;
			quint64 msecsValue = numberLocale.toLongLong(ui.leTimeValue->text(), &ok);
			if (!ok)
				return false;

			auto unit = (AbstractColumn::TimeUnit)ui.cbTimeUnits->currentIndex();
			switch (unit) {
			case AbstractColumn::TimeUnit::Milliseconds:
				break;
			case AbstractColumn::TimeUnit::Seconds:
				msecsValue *= 1000;
				break;
			case AbstractColumn::TimeUnit::Minutes:
				msecsValue *= 60 * 1000;
				break;
			case AbstractColumn::TimeUnit::Hours:
				msecsValue *= 60 * 60 * 1000;
				break;
			case AbstractColumn::TimeUnit::Days:
				msecsValue *= 24 * 60 * 60 * 1000;
			}

			value = msecsValue;
		} else // add/subtract a difference
			value = ui.dteTimeValueEnd->dateTime().toMSecsSinceEpoch() - ui.dteTimeValueStart->dateTime().toMSecsSinceEpoch();
	}

	return true;
}
