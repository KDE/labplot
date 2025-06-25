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
#include "backend/core/Settings.h"
#include "backend/core/column/Column.h"
#include "backend/core/datatypes/DateTime2StringFilter.h"
#include "backend/lib/macros.h"
#include "backend/matrix/Matrix.h"
#include "backend/nsl/nsl_baseline.h"
#include "backend/spreadsheet/Spreadsheet.h"
#include "backend/worksheet/Background.h"
#include "backend/worksheet/Line.h"
#include "backend/worksheet/TextLabel.h"
#include "backend/worksheet/Worksheet.h"
#include "backend/worksheet/plots/PlotArea.h"
#include "backend/worksheet/plots/cartesian/Axis.h"
#include "backend/worksheet/plots/cartesian/CartesianPlot.h"
#include "backend/worksheet/plots/cartesian/XYCurve.h"
#include "frontend/GuiTools.h"

#include <KLocalizedString>
#include <KMessageBox>
#include <KWindowConfig>

#include <QDialogButtonBox>
#include <QPushButton>
#include <QTimer>
#include <QWindow>

#include <cmath>

enum class ValueType { CustomValue, Difference, Minimum, Maximum, Median, Mean, Baseline };
/*!
	\class AddSubtractValueDialog
	\brief Dialog for adding/subtracting a value from column values.

	\ingroup frontend
 */

AddSubtractValueDialog::AddSubtractValueDialog(Spreadsheet* s, const QVector<Column*>& columns, Operation op, QWidget* parent)
	: QDialog(parent)
	, m_spreadsheet(s)
	, m_columns(columns)
	, m_operation(op) {
	Q_ASSERT(s != nullptr);

	ui.setupUi(this);
	init();
}

AddSubtractValueDialog::AddSubtractValueDialog(Matrix* m, Operation op, QWidget* parent)
	: QDialog(parent)
	, m_matrix(m)
	, m_operation(op) {
	Q_ASSERT(m != nullptr);

	ui.setupUi(this);
	init();
}

AddSubtractValueDialog::~AddSubtractValueDialog() {
	delete m_project;

	KConfigGroup conf = Settings::group(QStringLiteral("AddSubtractValueDialog"));
	conf.writeEntry(QStringLiteral("Type"), ui.cbType->currentData().toInt());
	conf.writeEntry(QStringLiteral("Preview"), ui.chbPreview->isChecked());

	// baseline subtraction specific parameters
	const auto numberLocale = QLocale();
	conf.writeEntry(QStringLiteral("BaselineParameter1"), ui.sbBaselineParameter1->value());
	conf.writeEntry(QStringLiteral("BaselineParameter2"), numberLocale.toDouble(ui.leBaselineParameter2->text()));
	conf.writeEntry(QStringLiteral("BaselineParameter3"), ui.sbBaselineParameter3->value());

	KWindowConfig::saveWindowSize(windowHandle(), conf);
}

void AddSubtractValueDialog::init() {
	// initilize the line edits with the values based on the values in the data container
	if (m_spreadsheet)
		initValuesSpreadsheet();
	else
		initValuesMatrix();

	setAttribute(Qt::WA_DeleteOnClose);

	auto* btnBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
	ui.gridLayout->addWidget(btnBox, 14, 0, 1, 3);
	m_okButton = btnBox->button(QDialogButtonBox::Ok);

	switch (m_operation) {
	case Add:
		setWindowTitle(i18nc("@title:window", "Add Value"));
		m_okButton->setText(i18n("&Add"));
		break;
	case Subtract:
		setWindowTitle(i18nc("@title:window", "Subtract Value"));
		ui.lType->setText(i18n("Subtract:"));
		m_okButton->setText(i18n("&Subtract"));
		break;
	case SubtractBaseline:
		setWindowTitle(i18nc("@title:window", "Subtract Baseline"));
		ui.lType->setText(i18n("Subtract:"));
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

	ui.lBaselineParameter1->hide();
	ui.sbBaselineParameter1->hide();
	ui.lBaselineParameter2->hide();
	ui.leBaselineParameter2->hide();
	ui.lBaselineParameter3->hide();
	ui.sbBaselineParameter3->hide();

	if (m_operation == Add || m_operation == Subtract) {
		if (m_spreadsheet && m_operation == Subtract && m_numeric) {
			ui.cbType->addItem(i18n("Minimum"), static_cast<int>(ValueType::Minimum));
			ui.cbType->addItem(i18n("Maximum"), static_cast<int>(ValueType::Maximum));
			ui.cbType->addItem(i18n("Median"), static_cast<int>(ValueType::Median));
			ui.cbType->addItem(i18n("Mean"), static_cast<int>(ValueType::Mean));
		}

		ui.cbType->addItem(i18n("Custom Value"), static_cast<int>(ValueType::CustomValue));
		ui.cbType->addItem(i18n("Difference"), static_cast<int>(ValueType::Difference));

		for (int i = 0; i < ENUM_COUNT(AbstractColumn, TimeUnit); i++)
			ui.cbTimeUnits->addItem(AbstractColumn::timeUnitString((AbstractColumn::TimeUnit)i));

		ui.lValueStart->setVisible(m_numeric);
		ui.leValueStart->setVisible(m_numeric);
		ui.lValueEnd->setVisible(m_numeric);
		ui.leValueEnd->setVisible(m_numeric);
		ui.cbTimeUnits->setVisible(!m_numeric);
		ui.lTimeValueStart->setVisible(!m_numeric);
		ui.dteTimeValueStart->setVisible(!m_numeric);
		ui.lTimeValueEnd->setVisible(!m_numeric);
		ui.dteTimeValueEnd->setVisible(!m_numeric);
	} else if (m_operation == SubtractBaseline && m_numeric) {
		ui.cbType->insertSeparator(6);
		ui.cbType->insertItem(7, i18n("Baseline (arPLS Algorithm)"), static_cast<int>(ValueType::Baseline));

		ui.leBaselineParameter2->setValidator(new QDoubleValidator(ui.leBaselineParameter2));

		// add tooltip texts for the baseline subtraction algorithms.
		// at the moment only arPLS (s.a. https://pubs.rsc.org/en/content/articlelanding/2015/AN/C4AN01061B#!divAbstract)
		// is supported and we show the description of its parameters only.
		QString info = i18n("Smoothness parameter - the larger the value the smoother the resulting background.");
		ui.lBaselineParameter1->setToolTip(info);
		ui.sbBaselineParameter1->setToolTip(info);

		info = i18n("Weighting termination ratio - value between 0 and 1, smaller values allow less negative values.");
		ui.lBaselineParameter2->setToolTip(info);
		ui.leBaselineParameter2->setToolTip(info);

		info = i18n("Number of iterations to perform.");
		ui.lBaselineParameter3->setToolTip(info);
		ui.sbBaselineParameter3->setToolTip(info);

		ui.lValueStart->hide();
		ui.leValueStart->hide();
		ui.lValueEnd->hide();
		ui.leValueEnd->hide();
		ui.cbTimeUnits->hide();
		ui.lTimeValueStart->hide();
		ui.dteTimeValueStart->hide();
		ui.lTimeValueEnd->hide();
		ui.dteTimeValueEnd->hide();
	} else {
		ui.lType->hide();
		ui.cbType->hide();
		ui.lPreview->hide();
		ui.chbPreview->hide();
		ui.framePreview->hide();

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

	ui.lValue->setVisible(m_numeric);
	ui.leValue->setVisible(m_numeric);
	ui.lTimeValue->setVisible(!m_numeric);
	ui.leTimeValue->setVisible(!m_numeric);

	// restore saved settings if available
	create(); // ensure there's a window created
	KConfigGroup conf = Settings::group(QStringLiteral("AddSubtractValueDialog"));

	// baseline subtraction specific parameters
	const auto numberLocale = QLocale();
	ui.sbBaselineParameter1->setValue(conf.readEntry(QStringLiteral("BaselineParameter1"), 6));
	ui.leBaselineParameter2->setText(numberLocale.toString(conf.readEntry(QStringLiteral("BaselineParameter2"), 0.1)));
	ui.sbBaselineParameter3->setValue(conf.readEntry(QStringLiteral("BaselineParameter3"), 10));

	int typeIndex = ui.cbType->findData(conf.readEntry("Type", 0));
	if (typeIndex != -1)
		ui.cbType->setCurrentIndex(typeIndex);
	else
		ui.cbType->setCurrentIndex(0);

	if (m_spreadsheet) {
		if (m_operation == Add || m_operation == Subtract || m_operation == SubtractBaseline) {
			ui.chbPreview->setChecked(conf.readEntry("Preview", false));
			ui.framePreview->setVisible(ui.chbPreview->isChecked());
			updateSpacer(!ui.chbPreview->isChecked());
		} else {
			ui.framePreview->hide();
			updateSpacer(true);
		}
	} else {
		// no preview available for Matrix
		ui.lPreview->hide();
		ui.chbPreview->hide();
		ui.framePreview->hide();
		updateSpacer(true);
	}

	if (conf.exists()) {
		KWindowConfig::restoreWindowSize(windowHandle(), conf);
		resize(windowHandle()->size()); // workaround for QTBUG-40584
	} else {
		updateSpacer(true);
		resize(QSize(400, 0).expandedTo(minimumSize()));
	}

	if (m_operation == SubtractBaseline) {
		ui.cbType->setCurrentIndex(ui.cbType->findData(static_cast<int>(ValueType::Baseline)));
		ui.cbType->setEnabled(false);
	}

	connect(m_okButton, &QPushButton::clicked, this, &AddSubtractValueDialog::generate);
	connect(btnBox, &QDialogButtonBox::accepted, this, &AddSubtractValueDialog::accept);
	connect(btnBox, &QDialogButtonBox::rejected, this, &AddSubtractValueDialog::reject);
	connect(ui.cbType, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &AddSubtractValueDialog::typeChanged);
	connect(ui.chbPreview, &QCheckBox::clicked, this, &AddSubtractValueDialog::previewChanged);
	connect(ui.leValue, &QLineEdit::textChanged, this, [=]() {
		m_okButton->setEnabled(!ui.leValue->text().isEmpty());
		invalidatePreview();
	});
	connect(ui.leValueStart, &QLineEdit::textChanged, this, [=]() {
		m_okButton->setEnabled(!ui.leValueStart->text().isEmpty());
		invalidatePreview();
	});
	connect(ui.leValueEnd, &QLineEdit::textChanged, this, [=]() {
		m_okButton->setEnabled(!ui.leValueEnd->text().isEmpty());
		invalidatePreview();
	});
	connect(ui.sbBaselineParameter1, QOverload<int>::of(&QSpinBox::valueChanged), this, &AddSubtractValueDialog::invalidatePreview);
	connect(ui.leBaselineParameter2, &QLineEdit::textChanged, this, [=]() {
		bool valid = false;
		QLocale().toDouble(ui.leBaselineParameter2->text(), &valid);
		valid = valid && !ui.leBaselineParameter2->text().isEmpty();
		GuiTools::highlight(ui.leBaselineParameter2, !valid);
		m_okButton->setEnabled(valid);
		if (valid)
			invalidatePreview();
	});
	connect(ui.sbBaselineParameter3, QOverload<int>::of(&QSpinBox::valueChanged), this, &AddSubtractValueDialog::invalidatePreview);

	// call typeChanged() to update the status of the widgets and of the preview
	// after the dialog was completely shown
	QTimer::singleShot(0, this, [=]() {
		typeChanged(ui.cbType->currentIndex());
	});
}

/*!
 * When a spreadsheet is being modified, show the first valid value
 * in the first selected column as the value to add/subtract.
 */
void AddSubtractValueDialog::initValuesSpreadsheet() {
	const auto* column = m_columns.first();

	switch (column->columnMode()) {
	case AbstractColumn::ColumnMode::Integer: {
		m_numeric = true;
		const auto str = QLocale().toString(column->integerAt(0));
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
		const auto str = QLocale().toString(column->bigIntAt(0));
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
				const auto str = QLocale().toString(column->valueAt(row), 'g', 16);
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
}

/*!
 * When a matrix is being modified, show the value of the first cell in the matrix
 * as the value to add/subtract.
 */
void AddSubtractValueDialog::initValuesMatrix() {
	QString str;
	switch (m_matrix->mode()) {
	case AbstractColumn::ColumnMode::Integer: {
		m_numeric = true;
		str = QLocale().toString(m_matrix->cell<int>(0, 0));
		ui.leValue->setValidator(new QIntValidator(ui.leValue));
		ui.leValueStart->setValidator(new QIntValidator(ui.leValueStart));
		ui.leValueEnd->setValidator(new QIntValidator(ui.leValueEnd));
		break;
	}
	case AbstractColumn::ColumnMode::BigInt: {
		m_numeric = true;
		// TODO: QLongLongValidator
		str = QLocale().toString(m_matrix->cell<qint64>(0, 0));
		ui.leValue->setValidator(new QIntValidator(ui.leValue));
		ui.leValueStart->setValidator(new QIntValidator(ui.leValueStart));
		ui.leValueEnd->setValidator(new QIntValidator(ui.leValueEnd));
		break;
	}
	case AbstractColumn::ColumnMode::Double: {
		m_numeric = true;
		str = QLocale().toString(m_matrix->cell<double>(0, 0));
		ui.leValue->setValidator(new QDoubleValidator(ui.leValue));
		ui.leValueStart->setValidator(new QDoubleValidator(ui.leValueStart));
		ui.leValueEnd->setValidator(new QDoubleValidator(ui.leValueEnd));
		break;
	}
	case AbstractColumn::ColumnMode::DateTime:
	case AbstractColumn::ColumnMode::Day:
	case AbstractColumn::ColumnMode::Month:
	case AbstractColumn::ColumnMode::Text:
		m_numeric = false;
	}

	ui.leValue->setText(str);
	ui.leValueStart->setText(str);
	ui.leValueEnd->setText(str);
}

// ##############################################################################
// ############################  Slots  #########################################
// ##############################################################################
void AddSubtractValueDialog::typeChanged(int index) {
	auto type = static_cast<ValueType>(ui.cbType->itemData(index).toInt());
	bool diff = (type == ValueType::Difference);
	bool baseline = false;
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
		} else if (type == ValueType::Baseline) {
			ui.lValue->hide();
			ui.leValue->hide();
			baseline = true;
		} else // custom value or difference -> enable the value field
			ui.leValue->setEnabled(true);
	} else { // datetime
		ui.lTimeValue->setVisible(!diff);
		ui.leTimeValue->setVisible(!diff);
		ui.cbTimeUnits->setVisible(!diff);
		ui.lTimeValueStart->setVisible(diff);
		ui.dteTimeValueStart->setVisible(diff);
		ui.lTimeValueEnd->setVisible(diff);
		ui.dteTimeValueEnd->setVisible(diff);
	}

	ui.lBaselineParameter1->setVisible(baseline);
	ui.sbBaselineParameter1->setVisible(baseline);
	ui.lBaselineParameter2->setVisible(baseline);
	ui.leBaselineParameter2->setVisible(baseline);
	ui.lBaselineParameter3->setVisible(baseline);
	ui.sbBaselineParameter3->setVisible(baseline);

	if (m_spreadsheet && (m_operation == Add || m_operation == Subtract || m_operation == SubtractBaseline)) {
		// we changed maybe from "Minimum" to "Baseline", etc. and need
		// to recalculate the x-column for the baseline curve
		m_xColumnBaselineDirty = true;

		invalidatePreview();
	}
}

void AddSubtractValueDialog::previewChanged(bool state) {
	bool visible = state && (m_operation == Add || m_operation == Subtract || m_operation == SubtractBaseline);
	updateSpacer(!visible);

	ui.framePreview->setVisible(visible);
	updatePreview();

	// resize the dialog
	layout()->activate();
	resize(QSize(this->width(), 0).expandedTo(minimumSize()));
}

void AddSubtractValueDialog::updateSpacer(bool add) {
	if (add) {
		m_verticalSpacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);
		ui.gridLayout->addItem(m_verticalSpacer, 12, 0, 1, 1);
	} else {
		if (m_verticalSpacer)
			ui.gridLayout->removeItem(m_verticalSpacer);
	}
}

// ##############################################################################
// #####################  Preview related functions  ############################
// ##############################################################################
void AddSubtractValueDialog::initPreview() {
	// create project and worksheet
	m_project = new Project();
	m_project->setUndoAware(false);
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
	plot->plotArea()->borderLine()->setStyle(Qt::NoPen);
	m_previewPlotTitle = plot->title();

	// x-axis
	auto* axis = new Axis(QLatin1String("y"), Axis::Orientation::Horizontal);
	axis->setDefault(true);
	axis->setSuppressRetransform(true);
	plot->addChild(axis);
	axis->setPosition(Axis::Position::Bottom);
	axis->setMajorTicksDirection(Axis::ticksIn);
	axis->majorGridLine()->setStyle(Qt::NoPen);
	axis->setMinorTicksDirection(Axis::noTicks);
	axis->title()->setText(QString());
	auto font = axis->labelsFont();
	font.setPointSizeF(Worksheet::convertToSceneUnits(8, Worksheet::Unit::Point));
	axis->setLabelsFont(font);
	axis->setLabelsOffset(0);
	axis->setSuppressRetransform(false);

	// y-axis
	axis = new Axis(QLatin1String("y"), Axis::Orientation::Vertical);
	axis->setDefault(true);
	axis->setSuppressRetransform(true);
	plot->addChild(axis);
	axis->setPosition(Axis::Position::Left);
	axis->setMajorTicksDirection(Axis::ticksIn);
	axis->majorGridLine()->setStyle(Qt::NoPen);
	axis->setMinorTicksDirection(Axis::noTicks);
	axis->title()->setText(QString());
	font = axis->labelsFont();
	font.setPointSizeF(Worksheet::convertToSceneUnits(8, Worksheet::Unit::Point));
	axis->setLabelsFont(font);
	axis->setLabelsOffset(0);
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

	// add the curve for the data to be subtracted
	m_curveBaseline = new XYCurve(i18n("baseline"));
	m_xColumnBaseline = new Column(QLatin1String("xBaseline"), AbstractColumn::ColumnMode::Integer);
	m_yColumnBaseline = new Column(QLatin1String("yBaseline"));
	m_curveBaseline->setXColumn(m_xColumnBaseline);
	m_curveBaseline->setYColumn(m_yColumnBaseline);
	plot->addChild(m_curveBaseline);

	// add the curve for the result data
	m_curveResult = new XYCurve(i18n("result"));
	m_curveResult->setXColumn(xColumn);
	m_yColumnResult = new Column(QLatin1String("yResult"), yColumn->columnMode());
	m_curveResult->setYColumn(m_yColumnResult);
	plot->addChild(m_curveResult);

	plot->addLegend();
	// ws->setTheme(QLatin1String("Tufte"));

	auto* layout = new QVBoxLayout(ui.framePreview);
	layout->setSpacing(0);
	layout->addWidget(ws->view());
	ws->setInteractive(false);
	ui.framePreview->setLayout(layout);
}

void AddSubtractValueDialog::invalidatePreview() {
	m_previewDirty = true;
	updatePreview();
}

void AddSubtractValueDialog::updatePreview() {
	if (!ui.framePreview->isVisible() || !m_previewDirty)
		return;

	QApplication::processEvents(QEventLoop::AllEvents, 0);
	WAIT_CURSOR;

	if (!m_project)
		initPreview();

	// y for the result curve - copy the original data into the result column and process it
	m_yColumnResult->setColumnMode(m_columns.constFirst()->columnMode()); // set the mode to its original value, it was potentially changed in generateForColumn
	m_yColumnResult->copy(m_columns.constFirst());
	generateForColumn(m_yColumnResult, 0);

	auto valueType = static_cast<ValueType>(ui.cbType->itemData(ui.cbType->currentIndex()).toInt());
	if (valueType == ValueType::Baseline) {
		// x
		int rows = m_spreadsheet->rowCount();
		if (m_xColumnBaselineDirty) {
			QVector<int> xData;
			xData.resize(rows);
			for (int i = 0; i < rows; ++i)
				xData[i] = i;
			m_xColumnBaseline->setIntegers(xData);
			m_xColumnBaselineDirty = false;
		}

		// y
		const auto* col = m_columns.constFirst();
		QVector<double> baselineData(rows);
		const auto* newData = static_cast<QVector<double>*>(m_yColumnResult->data());
		switch (col->columnMode()) {
		case AbstractColumn::ColumnMode::Integer: {
			auto* data = static_cast<QVector<int>*>(col->data());
			for (int i = 0; i < rows; ++i)
				baselineData[i] = data->at(i) - newData->at(i);
			break;
		}
		case AbstractColumn::ColumnMode::BigInt: {
			auto* data = static_cast<QVector<qint64>*>(col->data());
			for (int i = 0; i < rows; ++i)
				baselineData[i] = data->at(i) - newData->at(i);
			break;
		}
		case AbstractColumn::ColumnMode::Double: {
			auto* data = static_cast<QVector<double>*>(col->data());
			for (int i = 0; i < rows; ++i)
				baselineData[i] = data->at(i) - newData->at(i);
			break;
		}
		case AbstractColumn::ColumnMode::DateTime:
		case AbstractColumn::ColumnMode::Day:
		case AbstractColumn::ColumnMode::Month:
		case AbstractColumn::ColumnMode::Text:
			break;
		}
		m_yColumnBaseline->setValues(baselineData);

		m_previewPlotTitle->setText(i18n("Ratio: %1", m_arplsRatio));
	} else {
		// x
		if (m_xColumnBaselineDirty) {
			m_xColumnBaseline->resizeTo(2);
			m_xColumnBaseline->setIntegerAt(0, 0);
			m_xColumnBaseline->setIntegerAt(1, m_spreadsheet->rowCount() - 1);
			m_xColumnBaselineDirty = false;
		}

		// y
		double value = 0.;
		setDoubleValue(value);
		m_yColumnBaseline->resizeTo(2);
		m_yColumnBaseline->setValueAt(0, value);
		m_yColumnBaseline->setValueAt(1, value);
	}

	m_previewDirty = false;
	RESET_CURSOR;
}

// ##############################################################################
// ############################  "generate"  ####################################
// ##############################################################################
void AddSubtractValueDialog::generate() {
	if (m_spreadsheet)
		generateForColumns();
	else // matrix
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
	// in case the result was already calculated for the first column for the preview,
	// no need to calculate it again, re-use the already available result
	if (colIndex == 0 && !m_previewDirty) {
		// for the baseline subraction the mode has to be Double, set it if not the case yet
		if (m_operation == SubtractBaseline && col->columnMode() != AbstractColumn::ColumnMode::Double)
			col->setColumnMode(AbstractColumn::ColumnMode::Double);
		col->copy(m_yColumnResult);
		return;
	}

	const auto mode = col->columnMode();
	const int rows = m_spreadsheet->rowCount();

	if (mode == AbstractColumn::ColumnMode::Integer) {
		int value;
		setIntValue(value, colIndex);
		auto* data = static_cast<QVector<int>*>(col->data());
		QVector<int> new_data(rows);

		switch (m_operation) {
		case SubtractBaseline: {
			// copy the int data to doubles
			QVector<double> new_data(rows);
			for (int i = 0; i < rows; ++i)
				new_data[i] = data->at(i);

			subtractBaseline(new_data);

			// convert the column mode from int to double and subtract the baseline
			col->setColumnMode(AbstractColumn::ColumnMode::Double);
			col->setValues(new_data);
			break;
		}
		case Subtract:
			value *= -1.;
			[[fallthrough]];
		case Add: {
			for (int i = 0; i < rows; ++i)
				new_data[i] = data->at(i) + value;

			col->setIntegers(new_data);
			break;
		}
		case Multiply:
			for (int i = 0; i < rows; ++i)
				new_data[i] = data->at(i) * value;

			col->setIntegers(new_data);
			break;
		case Divide:
			for (int i = 0; i < rows; ++i)
				new_data[i] = data->at(i) / value;

			col->setIntegers(new_data);
			break;
		}
	} else if (mode == AbstractColumn::ColumnMode::BigInt) {
		qint64 value;
		setBigIntValue(value, colIndex);
		auto* data = static_cast<QVector<qint64>*>(col->data());
		QVector<qint64> new_data(rows);

		switch (m_operation) {
		case SubtractBaseline: {
			// copy the big int data to doubles
			QVector<double> new_data(rows);
			for (int i = 0; i < rows; ++i)
				new_data[i] = data->at(i);

			subtractBaseline(new_data);

			// convert the column mode from int to double and set the new data
			col->setColumnMode(AbstractColumn::ColumnMode::Double);
			col->setValues(new_data);
			break;
		}
		case Subtract:
			value *= -1.;
			[[fallthrough]];
		case Add: {
			for (int i = 0; i < rows; ++i)
				new_data[i] = data->at(i) + value;

			col->setBigInts(new_data);
			break;
		}
		case Multiply:
			for (int i = 0; i < rows; ++i)
				new_data[i] = data->at(i) * value;

			col->setBigInts(new_data);
			break;
		case Divide:
			for (int i = 0; i < rows; ++i)
				new_data[i] = data->at(i) / value;

			col->setBigInts(new_data);
			break;
		}
	} else if (mode == AbstractColumn::ColumnMode::Double) {
		double value;
		setDoubleValue(value, colIndex);
		auto* data = static_cast<QVector<double>*>(col->data());
		QVector<double> new_data(rows);

		switch (m_operation) {
		case SubtractBaseline: {
			// copy the data
			QVector<double> new_data(*data);
			subtractBaseline(new_data);
			col->setValues(new_data);
			break;
		}
		case Subtract:
			value *= -1.;
			[[fallthrough]];
		case Add: {
			for (int i = 0; i < rows; ++i)
				new_data[i] = data->at(i) + value;

			col->setValues(new_data);
			break;
		}
		case Multiply:
			for (int i = 0; i < rows; ++i)
				new_data[i] = data->at(i) * value;

			col->setValues(new_data);
			break;
		case Divide:
			for (int i = 0; i < rows; ++i)
				new_data[i] = data->at(i) / value;

			col->setValues(new_data);
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
			[[fallthrough]];
		case Add:
			for (int i = 0; i < rows; ++i)
				new_data[i] = QDateTime::fromMSecsSinceEpoch(data->at(i).toMSecsSinceEpoch() + value, Qt::UTC);

			col->replaceDateTimes(0, new_data);
			break;
		case Multiply:
		case Divide:
		case SubtractBaseline:
			break;
		}
	}
}

void AddSubtractValueDialog::subtractBaseline(QVector<double>& newData) {
	const auto numberLocale = QLocale();
	bool ok;
	const double lambda = pow(ui.sbBaselineParameter1->value(), 10);
	const double p = numberLocale.toDouble(ui.leBaselineParameter2->text(), &ok);
	const int niter = ui.sbBaselineParameter3->value();
	int size = newData.size();
	m_arplsRatio = nsl_baseline_remove_arpls(newData.data(), static_cast<size_t>(size), p, lambda, niter);
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
			[[fallthrough]];
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
		case SubtractBaseline:
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
			[[fallthrough]];
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
		case SubtractBaseline:
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
			[[fallthrough]];
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
		case SubtractBaseline:
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
			[[fallthrough]];
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
		case SubtractBaseline:
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
		case ValueType::Baseline:
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
		case ValueType::Baseline:
			break;
		}
	} else
		value = numberLocale.toLongLong(ui.leValue->text(), &ok);

	return ok;
}

bool AddSubtractValueDialog::setDoubleValue(double& value, int columnIndex) const {
	if (m_spreadsheet && (columnIndex < 0 || columnIndex >= m_columns.count())) // should never happen...
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
		case ValueType::Baseline:
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
	case SubtractBaseline:
		msg = i18n("%1: subtract baseline from column values", name, value);
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
