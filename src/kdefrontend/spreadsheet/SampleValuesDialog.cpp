/*
	File                 : SampleValuesDialog.cpp
	Project              : LabPlot
	Description          : Dialog for sampling values in columns
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2022 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "SampleValuesDialog.h"
#include "backend/core/Settings.h"
#include "backend/core/column/Column.h"
#include "backend/lib/macros.h"
#include "backend/spreadsheet/Spreadsheet.h"

#include <QDialogButtonBox>
#include <QPushButton>
#include <QThreadPool>
#include <QWindow>

#include <KLocalizedString>
#include <KMessageBox>
#include <KWindowConfig>

#include <cmath>

#include <gsl/gsl_randist.h>
#include <gsl/gsl_rng.h>

enum class Method { Periodic, Random };

/*!
	\class SampleValuesDialog
	\brief Dialog for generating values from a mathematical function.

	\ingroup kdefrontend
 */
SampleValuesDialog::SampleValuesDialog(Spreadsheet* s, QWidget* parent)
	: QDialog(parent)
	, m_spreadsheet(s) {
	ui.setupUi(this);
	setAttribute(Qt::WA_DeleteOnClose);

	ui.cbMethod->addItem(i18n("Periodic"));
	ui.cbMethod->addItem(i18n("Random"));

	QString info = i18n(
		"Sampling method:"
		"<ul>"
		"<li>Periodic - samples are created according to the specified interval.</li>"
		"<li>Random - samples are created randomly based on the uniform distribution. </li>"
		"</ul>");
	ui.lMethod->setToolTip(info);
	ui.cbMethod->setToolTip(info);

	auto* btnBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
	ui.gridLayout->addWidget(btnBox, 2, 1, 1, 2);
	m_okButton = btnBox->button(QDialogButtonBox::Ok);

	connect(btnBox->button(QDialogButtonBox::Cancel), &QPushButton::clicked, this, &SampleValuesDialog::close);

	m_okButton->setText(i18n("&Sample"));
	m_okButton->setToolTip(i18n("Sample values in the selected spreadsheet columns"));
	setWindowTitle(i18nc("@title:window", "Sample Values"));

	connect(ui.cbMethod, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &SampleValuesDialog::methodChanged);
	connect(m_okButton, &QPushButton::clicked, this, &SampleValuesDialog::sampleValues);
	connect(btnBox, &QDialogButtonBox::accepted, this, &SampleValuesDialog::accept);
	connect(btnBox, &QDialogButtonBox::rejected, this, &SampleValuesDialog::reject);

	// restore saved settings if available
	KConfigGroup conf = Settings::group(QLatin1String("SampleValuesDialog"));
	ui.cbMethod->setCurrentIndex(conf.readEntry("Method", 0));
	ui.sbValue->setValue(conf.readEntry("Value", 1));
	methodChanged(ui.cbMethod->currentIndex());

	create(); // ensure there's a window created
	if (conf.exists()) {
		KWindowConfig::restoreWindowSize(windowHandle(), conf);
		resize(windowHandle()->size()); // workaround for QTBUG-40584
	} else
		resize(QSize(400, 0).expandedTo(minimumSize()));
}

SampleValuesDialog::~SampleValuesDialog() {
	// save the current settings
	KConfigGroup conf = Settings::group(QLatin1String("SampleValuesDialog"));
	conf.writeEntry("Method", ui.cbMethod->currentIndex());
	conf.writeEntry("Value", ui.sbValue->value());
	KWindowConfig::saveWindowSize(windowHandle(), conf);
}

void SampleValuesDialog::setColumns(const QVector<Column*>& columns) {
	m_columns = columns;

	// resize the dialog to have the minimum height
	layout()->activate();
	resize(QSize(this->width(), 0).expandedTo(minimumSize()));
}

void SampleValuesDialog::methodChanged(int index) const {
	if (index == 0)
		ui.lValue->setText(i18n("Period:"));
	else
		ui.lValue->setText(i18n("Sample Size:"));
}

class SampleValuesTask : public QRunnable {
public:
	SampleValuesTask(Column* source, Column* target, const QVector<int>& rows)
		: m_rows(rows) {
		m_source = source;
		m_target = target;
	}

	void run() override {
		auto mode = m_source->columnMode();
		int size = m_rows.size();
		switch (mode) {
		case AbstractColumn::ColumnMode::Double: {
			auto* dataSource = static_cast<QVector<double>*>(m_source->data());
			QVector<double> dataTarget(size);
			for (int i = 0; i < size; ++i)
				dataTarget[i] = dataSource->at(m_rows.at(i));

			m_target->setValues(dataTarget);
			break;
		}
		case AbstractColumn::ColumnMode::Text: {
			auto* dataSource = static_cast<QVector<QString>*>(m_source->data());
			QVector<QString> dataTarget(size);
			for (int i = 0; i < size; ++i)
				dataTarget[i] = dataSource->at(m_rows.at(i));

			m_target->setText(dataTarget);
			break;
		}
		case AbstractColumn::ColumnMode::Month:
		case AbstractColumn::ColumnMode::Day:
		case AbstractColumn::ColumnMode::DateTime: {
			auto* dataSource = static_cast<QVector<QDateTime>*>(m_source->data());
			QVector<QDateTime> dataTarget(size);
			for (int i = 0; i < size; ++i)
				dataTarget[i] = dataSource->at(m_rows.at(i));

			m_target->setDateTimes(dataTarget);
			break;
		}
		case AbstractColumn::ColumnMode::Integer: {
			auto* dataSource = static_cast<QVector<int>*>(m_source->data());
			QVector<int> dataTarget(size);
			for (int i = 0; i < size; ++i)
				dataTarget[i] = dataSource->at(m_rows.at(i));

			m_target->setIntegers(dataTarget);
			break;
		}
		case AbstractColumn::ColumnMode::BigInt: {
			auto* dataSource = static_cast<QVector<qint64>*>(m_source->data());
			QVector<qint64> dataTarget(size);
			for (int i = 0; i < size; ++i)
				dataTarget[i] = dataSource->at(m_rows.at(i));

			m_target->setBigInts(dataTarget);
			break;
		}
		}
	}

private:
	Column* m_source;
	Column* m_target;
	QVector<int> m_rows;
};

void SampleValuesDialog::sampleValues() const {
	WAIT_CURSOR;
	const auto method = static_cast<Method>(ui.cbMethod->currentIndex());

	QVector<int> rows;
	switch (method) {
	case Method::Periodic: {
		int period = ui.sbValue->value();
		int count = std::floor(m_spreadsheet->rowCount() / period);
		for (int i = 0; i < count; ++i)
			rows << period * (i + 1) - 1;
		break;
	}
	case Method::Random: {
		// create a generator chosen by the environment variable GSL_RNG_TYPE
		gsl_rng_env_setup();
		const gsl_rng_type* T = gsl_rng_default;
		gsl_rng* r = gsl_rng_alloc(T);
		gsl_rng_set(r, QDateTime::currentMSecsSinceEpoch());

		int sampleSize = ui.sbValue->value();
		int a = 0;
		int b = m_spreadsheet->rowCount() - 1;
		for (int i = 0; i < sampleSize; ++i)
			rows << (int)round(gsl_ran_flat(r, a, b));
		break;
	}
	}

	m_spreadsheet->beginMacro(i18n("%1: sample values", m_spreadsheet->name()));

	// create target spreadsheet
	auto* targetSpreadsheet = new Spreadsheet(i18n("Sample of %1", m_spreadsheet->name()));
	targetSpreadsheet->setColumnCount(m_columns.count());
	targetSpreadsheet->setRowCount(rows.count());

	int index = 0;
	const auto& targetColumns = targetSpreadsheet->children<Column>();
	for (auto* source : m_columns) {
		auto* target = targetColumns.at(index);
		target->setName(source->name());
		target->setColumnMode(source->columnMode());
		++index;
	}

	for (int i = 0; i < m_columns.count(); ++i) {
		auto* task = new SampleValuesTask(m_columns.at(i), targetColumns.at(i), rows);
		QThreadPool::globalInstance()->start(task);
	}

	// wait until all columns were processed
	QThreadPool::globalInstance()->waitForDone();

	m_spreadsheet->parentAspect()->addChild(targetSpreadsheet);

	m_spreadsheet->endMacro();
	RESET_CURSOR;
}
