/*
    File                 : EquidistantValuesDialog.cpp
    Project              : LabPlot
    Description          : Dialog for generating equidistant numbers
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2014-2022 Alexander Semke <alexander.semke@web.de>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "EquidistantValuesDialog.h"
#include "backend/core/column/Column.h"
#include "backend/lib/macros.h"
#include "backend/spreadsheet/Spreadsheet.h"

#include <QDialogButtonBox>
#include <QPushButton>
#include <QWindow>

#include <KLocalizedString>
#include <KWindowConfig>

/*!
	\class EquidistantValuesDialog
	\brief Dialog for equidistant values.

	\ingroup kdefrontend
 */

EquidistantValuesDialog::EquidistantValuesDialog(Spreadsheet* s, QWidget* parent) : QDialog(parent), m_spreadsheet(s) {
	setWindowTitle(i18nc("@title:window", "Equidistant Values"));

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

	ui.cbType->addItem(i18n("Number"));
	ui.cbType->addItem(i18n("Increment"));

	ui.leFrom->setClearButtonEnabled(true);
	ui.leTo->setClearButtonEnabled(true);
	ui.leIncrement->setClearButtonEnabled(true);
	ui.leNumber->setClearButtonEnabled(true);

	ui.leFrom->setValidator( new QDoubleValidator(ui.leFrom) );
	ui.leTo->setValidator( new QDoubleValidator(ui.leTo) );
	ui.leIncrement->setValidator( new QDoubleValidator(ui.leIncrement) );
	ui.leNumber->setValidator( new QIntValidator(ui.leNumber) );

	connect( ui.cbType, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &EquidistantValuesDialog::typeChanged);
	connect( ui.leFrom, &QLineEdit::textChanged, this, &EquidistantValuesDialog::checkValues);
	connect( ui.leTo, &QLineEdit::textChanged, this, &EquidistantValuesDialog::checkValues);
	connect( ui.leNumber, &QLineEdit::textChanged, this, &EquidistantValuesDialog::checkValues);
	connect( ui.leIncrement, &QLineEdit::textChanged, this, &EquidistantValuesDialog::checkValues);
	connect(m_okButton, &QPushButton::clicked, this, &EquidistantValuesDialog::generate);

	//generated data the  default
	this->typeChanged(0);

	//restore saved settings if available
	create(); // ensure there's a window created
	KConfigGroup conf(KSharedConfig::openConfig(), "EquidistantValuesDialog");
	if (conf.exists()) {
		KWindowConfig::restoreWindowSize(windowHandle(), conf);
		resize(windowHandle()->size()); // workaround for QTBUG-40584
	} else
		resize(QSize(300, 0).expandedTo(minimumSize()));

	ui.cbType->setCurrentIndex(conf.readEntry("Type", 0));
	ui.leFrom->setText(QString::number(conf.readEntry("From", 1)));
	ui.leTo->setText(QString::number(conf.readEntry("To", 100)));
	SET_NUMBER_LOCALE
	ui.leIncrement->setText(numberLocale.toString(conf.readEntry("Increment", 1.)));
}

EquidistantValuesDialog::~EquidistantValuesDialog() {
	//save current settings
	KConfigGroup conf(KSharedConfig::openConfig(), "EquidistantValuesDialog");
	KWindowConfig::saveWindowSize(windowHandle(), conf);

	conf.writeEntry("Type", ui.cbType->currentIndex());
	SET_NUMBER_LOCALE
	conf.writeEntry("From", numberLocale.toInt(ui.leFrom->text()));
	conf.writeEntry("To", numberLocale.toInt(ui.leTo->text()));
	conf.writeEntry("Increment", numberLocale.toDouble(ui.leIncrement->text()));
}

void EquidistantValuesDialog::setColumns(const QVector<Column*>& columns) {
	m_columns = columns;
	SET_NUMBER_LOCALE
	ui.leNumber->setText( numberLocale.toString(m_columns.first()->rowCount()) );
}

void EquidistantValuesDialog::typeChanged(int index) {
	if (index == 0) { //fixed number
		ui.lIncrement->hide();
		ui.leIncrement->hide();
		ui.lNumber->show();
		ui.leNumber->show();
	} else { //fixed increment
		ui.lIncrement->show();
		ui.leIncrement->show();
		ui.lNumber->hide();
		ui.leNumber->hide();
	}
}

void EquidistantValuesDialog::checkValues() {
	if (ui.leFrom->text().simplified().isEmpty() || ui.leTo->text().simplified().isEmpty()) {
		m_okButton->setEnabled(false);
		return;
	}

	SET_NUMBER_LOCALE
	if (ui.cbType->currentIndex() == 0) {	// INT
		if (ui.leNumber->text().simplified().isEmpty() || numberLocale.toInt(ui.leNumber->text().simplified()) == 0) {
			m_okButton->setEnabled(false);
			return;
		}
	} else {	// DOUBLE
		if (ui.leIncrement->text().simplified().isEmpty() || qFuzzyIsNull( numberLocale.toDouble(ui.leIncrement->text().simplified()) )) {
			m_okButton->setEnabled(false);
			return;
		}
	}

	m_okButton->setEnabled(true);
}

void EquidistantValuesDialog::generate() {
	Q_ASSERT(m_spreadsheet);

	WAIT_CURSOR;
	m_spreadsheet->beginMacro(i18np("%1: fill column with equidistant numbers",
									"%1: fill columns with equidistant numbers",
									m_spreadsheet->name(),
									m_columns.size()));

	SET_NUMBER_LOCALE
	bool ok;
	double start  = numberLocale.toDouble(ui.leFrom->text(), &ok);
	if (!ok) {
		DEBUG("Double value start invalid!")
	        m_spreadsheet->endMacro();
		RESET_CURSOR;
		return;
	}
	double end  = numberLocale.toDouble(ui.leTo->text(), &ok);
	if (!ok) {
		DEBUG("Double value end invalid!")
	        m_spreadsheet->endMacro();
		RESET_CURSOR;
		return;
	}
	int number{0};
	double dist{0};
	if (ui.cbType->currentIndex() == 0) { //fixed number
		number = numberLocale.toInt(ui.leNumber->text(), &ok);
		if (ok && number != 1)
			dist = (end - start)/ (number - 1);
	} else { //fixed increment
		dist = numberLocale.toDouble(ui.leIncrement->text(), &ok);
		if (ok)
			number = (end-start)/dist + 1;
	}

	if (m_spreadsheet->rowCount() < number)
		m_spreadsheet->setRowCount(number);

	QVector<double> newData;
	for (int i = 0; i < number; ++i)
		newData.push_back(start + dist*i);

	for (auto* col : m_columns)
		col->replaceValues(0, newData);

	m_spreadsheet->endMacro();
	RESET_CURSOR;
}
