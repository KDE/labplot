/*
	File                 : ColumnDock.cpp
	Project              : LabPlot
	Description          : widget for column properties
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2011-2022 Alexander Semke <alexander.semke@web.de>
	SPDX-FileCopyrightText: 2013-2017 Stefan Gerlach <stefan.gerlach@uni.kn>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "ColumnDock.h"

#include "backend/core/AbstractFilter.h"
#include "backend/core/Project.h"
#include "backend/core/datatypes/DateTime2StringFilter.h"
#include "backend/core/datatypes/Double2StringFilter.h"
#include "backend/core/datatypes/SimpleCopyThroughFilter.h"
#include "backend/core/datatypes/String2DateTimeFilter.h"
#include "backend/core/datatypes/String2DoubleFilter.h"
#include "backend/spreadsheet/Spreadsheet.h"
#include "kdefrontend/spreadsheet/AddValueLabelDialog.h"
#include "kdefrontend/spreadsheet/BatchEditValueLabelsDialog.h"

#include <KLocalizedString>

#include <QInputDialog>

/*!
  \class ColumnDock
  \brief Provides a widget for editing the properties of the spreadsheet columns currently selected in the project explorer.

  \ingroup kdefrontend
*/

ColumnDock::ColumnDock(QWidget* parent)
	: BaseDock(parent) {
	ui.setupUi(this);
	m_leName = ui.leName;
	m_teComment = ui.teComment;
	m_teComment->setFixedHeight(m_leName->height());

	// add formats for numeric values
	ui.cbNumericFormat->addItem(i18n("Decimal"), QVariant('f'));
	ui.cbNumericFormat->addItem(i18n("Scientific (e)"), QVariant('e'));
	ui.cbNumericFormat->addItem(i18n("Scientific (E)"), QVariant('E'));
	ui.cbNumericFormat->addItem(i18n("Automatic (e)"), QVariant('g'));
	ui.cbNumericFormat->addItem(i18n("Automatic (E)"), QVariant('G'));

	// add format for date, time and datetime values
	for (const auto& s : AbstractColumn::dateTimeFormats())
		ui.cbDateTimeFormat->addItem(s, QVariant(s));

	ui.cbDateTimeFormat->setEditable(true);

	ui.twLabels->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
	ui.twLabels->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);

	connect(ui.leName, &QLineEdit::textChanged, this, &ColumnDock::nameChanged);
	connect(ui.teComment, &QTextEdit::textChanged, this, &ColumnDock::commentChanged);
	connect(ui.cbType, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ColumnDock::typeChanged);
	connect(ui.cbNumericFormat, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ColumnDock::numericFormatChanged);
	connect(ui.sbPrecision, QOverload<int>::of(&QSpinBox::valueChanged), this, &ColumnDock::precisionChanged);
	connect(ui.cbDateTimeFormat, &QComboBox::currentTextChanged, this, &ColumnDock::dateTimeFormatChanged);
	connect(ui.cbPlotDesignation, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ColumnDock::plotDesignationChanged);
	connect(ui.bAddLabel, &QPushButton::clicked, this, &ColumnDock::addLabel);
	connect(ui.bRemoveLabel, &QPushButton::clicked, this, &ColumnDock::removeLabel);
	connect(ui.bBatchEditLabels, &QPushButton::clicked, this, &ColumnDock::batchEditLabels);

	retranslateUi();
}

void ColumnDock::setColumns(QList<Column*> list) {
	CONDITIONAL_LOCK_RETURN;
	m_columnsList = list;
	m_column = list.first();
	setAspects(list);

	// check whether we have non-editable columns:
	// 1. columns in a LiveDataSource
	// 2. columns in the spreadsheet of a datapicker curve
	// 3. columns for residuals calculated in XYFitCurve)
	bool nonEditable = false;
	for (auto* col : m_columnsList) {
		auto* s = dynamic_cast<Spreadsheet*>(col->parentAspect());
		if (s) {
			if (s->type() == AspectType::LiveDataSource || s->parentAspect()->type() == AspectType::DatapickerCurve) {
				nonEditable = true;
				break;
			}
		} else {
			nonEditable = true;
			break;
		}
	}

	// if columns of different modes are selected, change of the mode is not possible
	bool sameMode = true;

	if (list.size() == 1) {
		// names and comments of non-editable columns in a file data source can be changed.
		if (!nonEditable && m_column->parentAspect()->type() == AspectType::LiveDataSource) {
			ui.leName->setEnabled(false);
			ui.teComment->setEnabled(false);
		} else {
			ui.leName->setEnabled(true);
			ui.teComment->setEnabled(true);
		}

		ui.leName->setText(m_column->name());
		ui.teComment->setText(m_column->comment());
	} else {
		ui.leName->setEnabled(false);
		ui.teComment->setEnabled(false);

		ui.leName->setText(QString());
		ui.teComment->setText(QString());

		auto mode = m_column->columnMode();
		for (auto* col : m_columnsList) {
			if (col->columnMode() != mode) {
				sameMode = false;
				break;
			}
		}
	}

	ui.lType->setEnabled(sameMode);
	ui.cbType->setEnabled(sameMode);
	ui.lNumericFormat->setEnabled(sameMode);
	ui.cbNumericFormat->setEnabled(sameMode);
	ui.lPrecision->setEnabled(sameMode);
	ui.sbPrecision->setEnabled(sameMode);
	ui.lDateTimeFormat->setEnabled(sameMode);
	ui.cbDateTimeFormat->setEnabled(sameMode);
	ui.lLabels->setEnabled(sameMode);
	ui.twLabels->setEnabled(sameMode);
	ui.frameLabels->setEnabled(sameMode);

	ui.leName->setStyleSheet(QString());
	ui.leName->setToolTip(QString());

	// show the properties of the first column
	updateTypeWidgets(m_column->columnMode());
	ui.cbPlotDesignation->setCurrentIndex(int(m_column->plotDesignation()));

	// show value labels of the first column if all selected columns have the same mode
	if (sameMode)
		showValueLabels();
	else {
		for (int i = 0; ui.twLabels->rowCount(); ++i)
			ui.twLabels->removeRow(0);
	}

	// slots
	connect(m_column, &AbstractColumn::aspectDescriptionChanged, this, &ColumnDock::aspectDescriptionChanged);
	connect(m_column, &AbstractColumn::modeChanged, this, &ColumnDock::columnModeChanged);
	connect(m_column->outputFilter(), &AbstractSimpleFilter::formatChanged, this, &ColumnDock::columnFormatChanged);
	connect(m_column->outputFilter(), &AbstractSimpleFilter::digitsChanged, this, &ColumnDock::columnPrecisionChanged);
	connect(m_column, &AbstractColumn::plotDesignationChanged, this, &ColumnDock::columnPlotDesignationChanged);

	// don't allow to change the column type at least one non-editable column
	if (sameMode)
		ui.cbType->setEnabled(!nonEditable);
}

/*!
  depending on the currently selected column type (column mode) updates the widgets for the column format,
  shows/hides the allowed widgets, fills the corresponding combobox with the possible entries.
  Called when the type (column mode) is changed.
*/
void ColumnDock::updateTypeWidgets(AbstractColumn::ColumnMode mode) {
	ui.cbType->setCurrentIndex(ui.cbType->findData(static_cast<int>(mode)));
	switch (mode) {
	case AbstractColumn::ColumnMode::Double: {
		auto* filter = static_cast<Double2StringFilter*>(m_column->outputFilter());
		ui.cbNumericFormat->setCurrentIndex(ui.cbNumericFormat->findData(filter->numericFormat()));
		ui.sbPrecision->setValue(filter->numDigits());
		break;
	}
	case AbstractColumn::ColumnMode::Month:
	case AbstractColumn::ColumnMode::Day:
	case AbstractColumn::ColumnMode::DateTime: {
		auto* filter = static_cast<DateTime2StringFilter*>(m_column->outputFilter());
		// 			DEBUG("	set column format: " << STDSTRING(filter->format()));
		ui.cbDateTimeFormat->setCurrentIndex(ui.cbDateTimeFormat->findData(filter->format()));
		break;
	}
	case AbstractColumn::ColumnMode::Integer: // nothing to set
	case AbstractColumn::ColumnMode::BigInt:
	case AbstractColumn::ColumnMode::Text:
		break;
	}

	// hide all the format related widgets first and
	// then show only what is required depending of the column mode(s)
	ui.lNumericFormat->hide();
	ui.cbNumericFormat->hide();
	ui.lPrecision->hide();
	ui.sbPrecision->hide();
	ui.lDateTimeFormat->hide();
	ui.cbDateTimeFormat->hide();

	if (mode == AbstractColumn::ColumnMode::Double) {
		ui.lNumericFormat->show();
		ui.cbNumericFormat->show();
		ui.lPrecision->show();
		ui.sbPrecision->show();
	}

	if (mode == AbstractColumn::ColumnMode::DateTime) {
		ui.lDateTimeFormat->show();
		ui.cbDateTimeFormat->show();
	}
}

void ColumnDock::showValueLabels() {
	for (int i = 0; ui.twLabels->rowCount(); ++i)
		ui.twLabels->removeRow(0);

	if (m_column->hasValueLabels()) {
		auto mode = m_column->columnMode();
		int i = 0;

		switch (mode) {
		case AbstractColumn::ColumnMode::Double: {
			auto labels = m_column->valueLabels();
			ui.twLabels->setRowCount(labels.size());
			auto it = labels.constBegin();
			while (it != labels.constEnd()) {
				ui.twLabels->setItem(i, 0, new QTableWidgetItem(QString::number(it.key())));
				ui.twLabels->setItem(i, 1, new QTableWidgetItem(it.value()));
				++it;
				++i;
			}
			break;
		}
		case AbstractColumn::ColumnMode::Integer: {
			auto labels = m_column->intValueLabels();
			ui.twLabels->setRowCount(labels.size());
			auto it = labels.constBegin();
			while (it != labels.constEnd()) {
				ui.twLabels->setItem(i, 0, new QTableWidgetItem(QString::number(it.key())));
				ui.twLabels->setItem(i, 1, new QTableWidgetItem(it.value()));
				++it;
				++i;
			}
			break;
		}
		case AbstractColumn::ColumnMode::BigInt: {
			auto labels = m_column->bigIntValueLabels();
			ui.twLabels->setRowCount(labels.size());
			auto it = labels.constBegin();
			while (it != labels.constEnd()) {
				ui.twLabels->setItem(i, 0, new QTableWidgetItem(QString::number(it.key())));
				ui.twLabels->setItem(i, 1, new QTableWidgetItem(it.value()));
				++it;
				++i;
			}
			break;
		}
		case AbstractColumn::ColumnMode::Text: {
			auto labels = m_column->textValueLabels();
			ui.twLabels->setRowCount(labels.size());
			auto it = labels.constBegin();
			while (it != labels.constEnd()) {
				ui.twLabels->setItem(i, 0, new QTableWidgetItem(it.key()));
				ui.twLabels->setItem(i, 1, new QTableWidgetItem(it.value()));
				++it;
				++i;
			}
			break;
		}
		case AbstractColumn::ColumnMode::Month:
		case AbstractColumn::ColumnMode::Day:
		case AbstractColumn::ColumnMode::DateTime: {
			auto labels = m_column->dateTimeValueLabels();
			ui.twLabels->setRowCount(labels.size());
			auto it = labels.constBegin();
			const QString& format = ui.cbDateTimeFormat->currentText();
			while (it != labels.constEnd()) {
				ui.twLabels->setItem(i, 0, new QTableWidgetItem(it.key().toString(format)));
				ui.twLabels->setItem(i, 1, new QTableWidgetItem(it.value()));
				++it;
				++i;
			}
			break;
		}
		}
	}
}

//*************************************************************
//******** SLOTs for changes triggered in ColumnDock **********
//*************************************************************
void ColumnDock::retranslateUi() {
	CONDITIONAL_LOCK_RETURN;

	ui.cbType->clear();
	ui.cbType->addItem(AbstractColumn::columnModeString(AbstractColumn::ColumnMode::Double), QVariant(static_cast<int>(AbstractColumn::ColumnMode::Double)));
	ui.cbType->addItem(AbstractColumn::columnModeString(AbstractColumn::ColumnMode::Integer), QVariant(static_cast<int>(AbstractColumn::ColumnMode::Integer)));
	ui.cbType->addItem(AbstractColumn::columnModeString(AbstractColumn::ColumnMode::BigInt), QVariant(static_cast<int>(AbstractColumn::ColumnMode::BigInt)));
	ui.cbType->addItem(AbstractColumn::columnModeString(AbstractColumn::ColumnMode::Text), QVariant(static_cast<int>(AbstractColumn::ColumnMode::Text)));
	ui.cbType->addItem(AbstractColumn::columnModeString(AbstractColumn::ColumnMode::Month), QVariant(static_cast<int>(AbstractColumn::ColumnMode::Month)));
	ui.cbType->addItem(AbstractColumn::columnModeString(AbstractColumn::ColumnMode::Day), QVariant(static_cast<int>(AbstractColumn::ColumnMode::Day)));
	ui.cbType->addItem(AbstractColumn::columnModeString(AbstractColumn::ColumnMode::DateTime),
					   QVariant(static_cast<int>(AbstractColumn::ColumnMode::DateTime)));

	ui.cbPlotDesignation->clear();
	for (int i = 0; i < ENUM_COUNT(AbstractColumn, PlotDesignation); i++)
		ui.cbPlotDesignation->addItem(AbstractColumn::plotDesignationString(AbstractColumn::PlotDesignation(i), false));

	ui.bAddLabel->setToolTip(i18n("Add a new value label"));
	ui.bRemoveLabel->setToolTip(i18n("Remove the selected value label"));
	ui.bBatchEditLabels->setToolTip(i18n("Modify multiple values labels in a batch mode"));
}

/*!
  called when the type (column mode - numeric, text etc.) of the column was changed.
*/
void ColumnDock::typeChanged(int index) {
	DEBUG(Q_FUNC_INFO)
	CONDITIONAL_RETURN_NO_LOCK; // TODO: lock needed?

	auto columnMode = static_cast<AbstractColumn::ColumnMode>(ui.cbType->itemData(index).toInt());
	const auto& columns = m_columnsList;

	switch (columnMode) {
	case AbstractColumn::ColumnMode::Double: {
		int digits = ui.sbPrecision->value();
		for (auto* col : columns) {
			col->beginMacro(i18n("%1: change column type", col->name()));
			col->setColumnMode(columnMode);
			auto* filter = static_cast<Double2StringFilter*>(col->outputFilter());

			// TODO: using
			// char format = ui.cbFormat->itemData(ui.cbFormat->currentIndex()).toChar().toLatin1();
			// outside of the for-loop and
			// filter->setNumericFormat(format);
			// inside the loop leads to wrong results when converting from integer to numeric -> 'f' is set instead of 'e'
			filter->setNumericFormat(ui.cbNumericFormat->itemData(ui.cbNumericFormat->currentIndex()).toChar().toLatin1());
			filter->setNumDigits(digits);
			col->endMacro();
		}
		break;
	}
	case AbstractColumn::ColumnMode::Integer:
	case AbstractColumn::ColumnMode::BigInt:
	case AbstractColumn::ColumnMode::Text:
		for (auto* col : columns)
			col->setColumnMode(columnMode);
		break;
	case AbstractColumn::ColumnMode::Month:
	case AbstractColumn::ColumnMode::Day:
		for (auto* col : columns) {
			col->beginMacro(i18n("%1: change column type", col->name()));
			// the format is saved as item data
			const QString& format = ui.cbDateTimeFormat->itemData(ui.cbDateTimeFormat->currentIndex()).toString();
			col->setColumnMode(columnMode);
			auto* filter = static_cast<DateTime2StringFilter*>(col->outputFilter());
			filter->setFormat(format);
			col->endMacro();
		}
		break;
	case AbstractColumn::ColumnMode::DateTime: // -> DateTime
		for (auto* col : columns) {
			// use standard format
			const QString& format(QStringLiteral("yyyy-MM-dd hh:mm:ss"));
			QDEBUG(Q_FUNC_INFO << ", format = " << format)

			// enable if unit cat be used
			/*if (col->isNumeric()) {	// ask how to interpret numeric input value

				QStringList items;
				for (int i = 0; i < ENUM_COUNT(AbstractColumn, TimeUnit); i++)
					items << AbstractColumn::timeUnitString((AbstractColumn::TimeUnit)i);
				QDEBUG("ITEMS: " << items)

				bool ok;
				QString item = QInputDialog::getItem(this, i18n("DateTime Filter"), i18n("Unit:"), items, 0, false, &ok);
				if (ok) {
					int index = items.indexOf(item);

					DEBUG("Selected index: " << index)
					//TODO: use index
				}
				else	// Cancel
					return;
			}*/

			col->beginMacro(i18n("%1: change column type", col->name()));
			col->setColumnMode(columnMode); // TODO: timeUnit
			auto* filter = static_cast<DateTime2StringFilter*>(col->outputFilter());
			filter->setFormat(format);
			col->endMacro();
		}
		break;
	}

	updateTypeWidgets(columnMode);
}

void ColumnDock::numericFormatChanged(int index) {
	CONDITIONAL_LOCK_RETURN;

	char format = ui.cbNumericFormat->itemData(index).toChar().toLatin1();
	for (auto* col : qAsConst(m_columnsList)) {
		auto* filter = static_cast<Double2StringFilter*>(col->outputFilter());
		filter->setNumericFormat(format);
	}
}

void ColumnDock::precisionChanged(int digits) {
	CONDITIONAL_LOCK_RETURN;

	for (auto* col : qAsConst(m_columnsList)) {
		auto* filter = static_cast<Double2StringFilter*>(col->outputFilter());
		filter->setNumDigits(digits);
	}
}

void ColumnDock::dateTimeFormatChanged(const QString& format) {
	CONDITIONAL_LOCK_RETURN;

	for (auto* col : qAsConst(m_columnsList)) {
		auto* filter = static_cast<DateTime2StringFilter*>(col->outputFilter());
		filter->setFormat(format);
	}
}

void ColumnDock::plotDesignationChanged(int index) {
	CONDITIONAL_LOCK_RETURN;

	auto pd = AbstractColumn::PlotDesignation(index);
	for (auto* col : qAsConst(m_columnsList))
		col->setPlotDesignation(pd);
}

// value labels
void ColumnDock::addLabel() {
	auto mode = m_column->columnMode();
	auto* dlg = new AddValueLabelDialog(this, m_column);

	if (mode == AbstractColumn::ColumnMode::Month || mode == AbstractColumn::ColumnMode::Day || mode == AbstractColumn::ColumnMode::DateTime)
		dlg->setDateTimeFormat(ui.cbDateTimeFormat->currentText());

	if (dlg->exec() == QDialog::Accepted) {
		const QString& label = dlg->label();
		const auto& columns = m_columnsList;
		QString valueStr;
		switch (mode) {
		case AbstractColumn::ColumnMode::Double: {
			double value = dlg->value();
			valueStr = QString::number(value);
			for (auto* col : columns)
				col->addValueLabel(value, label);
			break;
		}
		case AbstractColumn::ColumnMode::Integer: {
			int value = dlg->valueInt();
			valueStr = QString::number(value);
			for (auto* col : columns)
				col->addValueLabel(value, label);
			break;
		}
		case AbstractColumn::ColumnMode::BigInt: {
			qint64 value = dlg->valueBigInt();
			valueStr = QString::number(value);
			for (auto* col : columns)
				col->addValueLabel(value, label);
			break;
		}
		case AbstractColumn::ColumnMode::Text: {
			valueStr = dlg->valueText();
			for (auto* col : columns)
				col->addValueLabel(valueStr, label);
			break;
		}
		case AbstractColumn::ColumnMode::Month:
		case AbstractColumn::ColumnMode::Day:
		case AbstractColumn::ColumnMode::DateTime: {
			const QDateTime& value = dlg->valueDateTime();
			valueStr = value.toString(ui.cbDateTimeFormat->currentText());
			for (auto* col : columns)
				col->addValueLabel(value, label);
			break;
		}
		}

		int count = ui.twLabels->rowCount();
		ui.twLabels->insertRow(count);
		ui.twLabels->setItem(count, 0, new QTableWidgetItem(valueStr));
		ui.twLabels->setItem(count, 1, new QTableWidgetItem(label));
	}
	delete dlg;
	m_column->project()->setChanged(true);
}

void ColumnDock::removeLabel() {
	auto* item = ui.twLabels->currentItem();
	if (!item)
		return;

	int row = ui.twLabels->currentRow();
	const auto& value = ui.twLabels->itemAt(row, 0)->text();
	for (auto* col : qAsConst(m_columnsList))
		col->removeValueLabel(value);

	ui.twLabels->removeRow(ui.twLabels->currentRow());
	m_column->project()->setChanged(true);
}

void ColumnDock::batchEditLabels() {
	auto* dlg = new BatchEditValueLabelsDialog(this);
	dlg->setColumns(m_columnsList);
	if (dlg->exec() == QDialog::Accepted)
		showValueLabels(); // new value labels were saved in the dialog, show them here

	delete dlg;
	m_column->project()->setChanged(true);
}

//*************************************************************
//********* SLOTs for changes triggered in Column *************
//*************************************************************
void ColumnDock::columnModeChanged(const AbstractAspect* aspect) {
	CONDITIONAL_LOCK_RETURN;
	if (m_column != aspect)
		return;

	updateTypeWidgets(m_column->columnMode());
}

void ColumnDock::columnFormatChanged() {
	DEBUG(Q_FUNC_INFO)
	CONDITIONAL_LOCK_RETURN;
	auto columnMode = m_column->columnMode();
	switch (columnMode) {
	case AbstractColumn::ColumnMode::Double: {
		auto* filter = static_cast<Double2StringFilter*>(m_column->outputFilter());
		ui.cbNumericFormat->setCurrentIndex(ui.cbNumericFormat->findData(filter->numericFormat()));
		break;
	}
	case AbstractColumn::ColumnMode::Integer:
	case AbstractColumn::ColumnMode::BigInt:
	case AbstractColumn::ColumnMode::Text:
		break;
	case AbstractColumn::ColumnMode::Month:
	case AbstractColumn::ColumnMode::Day:
	case AbstractColumn::ColumnMode::DateTime: {
		auto* filter = static_cast<DateTime2StringFilter*>(m_column->outputFilter());
		ui.cbDateTimeFormat->setCurrentText(filter->format());
		break;
	}
	}
}

void ColumnDock::columnPrecisionChanged() {
	CONDITIONAL_LOCK_RETURN;
	auto* filter = static_cast<Double2StringFilter*>(m_column->outputFilter());
	ui.sbPrecision->setValue(filter->numDigits());
}

void ColumnDock::columnPlotDesignationChanged(const AbstractColumn* col) {
	CONDITIONAL_LOCK_RETURN;
	ui.cbPlotDesignation->setCurrentIndex(int(col->plotDesignation()));
}
