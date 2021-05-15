/***************************************************************************
	File                 : BatchEditValueLabelsDialog.cpp
    Project              : LabPlot
	Description          : Dialog to modify multiply value labels in a batch mode
    --------------------------------------------------------------------
    Copyright            : (C) 2021 by Alexander Semke (alexander.semke@web.de)

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
#include "BatchEditValueLabelsDialog.h"
#include "backend/core/column/Column.h"
#include "backend/core/datatypes/DateTime2StringFilter.h"
#include "backend/lib/macros.h"

#include <QDialogButtonBox>
#include <QVBoxLayout>
#include <QTextEdit>
#include <QLabel>
#include <QPushButton>
#include <QWindow>

#include <KLocalizedString>
#include <KSharedConfig>
#include <KWindowConfig>

/*!
	\class BatchEditValueLabelsDialog
	\brief Dialog to add a new the value label

	\ingroup kdefrontend
 */
BatchEditValueLabelsDialog::BatchEditValueLabelsDialog(QWidget* parent) : QDialog(parent) {

	setWindowTitle(i18nc("@title:window", "Edit Value Labels"));

	auto* layout = new QVBoxLayout(this);

	auto* label = new QLabel(i18n("Value Labels:"));
	layout->addWidget(label);

	teValueLabels = new QTextEdit();
	layout->addWidget(teValueLabels);

	auto* btnBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
	connect(btnBox, &QDialogButtonBox::accepted, this, &BatchEditValueLabelsDialog::save);
	connect(btnBox, &QDialogButtonBox::accepted, this, &BatchEditValueLabelsDialog::accept);
	connect(btnBox, &QDialogButtonBox::rejected, this, &BatchEditValueLabelsDialog::reject);
	layout->addWidget(btnBox);

	//restore saved settings if available
	KConfigGroup conf(KSharedConfig::openConfig(), QLatin1String("BatchEditValueLabelsDialog"));

	create(); // ensure there's a window created
	if (conf.exists()) {
		KWindowConfig::restoreWindowSize(windowHandle(), conf);
		resize(windowHandle()->size()); // workaround for QTBUG-40584
	} else
		resize(QSize(200, 0).expandedTo(minimumSize()));
}

BatchEditValueLabelsDialog::~BatchEditValueLabelsDialog() {
	//save the current settings
	KConfigGroup conf(KSharedConfig::openConfig(), QLatin1String("BatchEditValueLabelsDialog"));
	KWindowConfig::saveWindowSize(windowHandle(), conf);
}

void BatchEditValueLabelsDialog::setColumns(QList<Column*> columns) {
	m_columns = columns;

	if (m_columns.isEmpty())
		return;

	m_column = m_columns.first();

	//show the available value labels for the first columm
	if (m_column->hasValueLabels()) {
		auto mode = m_column->columnMode();
		QString text;

		switch (mode) {
		case AbstractColumn::ColumnMode::Numeric: {
			auto labels = m_column->valueLabels();
			auto it = labels.constBegin();
			while (it != labels.constEnd()) {
				if (!text.isEmpty())
					text += QLatin1Char('\n');
				text += QString::number(it.key()) + QLatin1String(" = ") + it.value();
				++it;
			}
			break;
		}
		case AbstractColumn::ColumnMode::Integer: {
			auto labels = m_column->intValueLabels();
			auto it = labels.constBegin();
			while (it != labels.constEnd()) {
				if (!text.isEmpty())
					text += QLatin1Char('\n');
				text += QString::number(it.key()) + QLatin1String(" = ") + it.value();
				++it;
			}
			break;
		}
		case AbstractColumn::ColumnMode::BigInt: {
			auto labels = m_column->bigIntValueLabels();
			auto it = labels.constBegin();
			while (it != labels.constEnd()) {
				if (!text.isEmpty())
					text += QLatin1Char('\n');
				text += QString::number(it.key()) + QLatin1String(" = ") + it.value();
				++it;
			}
			break;
		}
		case AbstractColumn::ColumnMode::Text: {
			auto labels = m_column->textValueLabels();
			auto it = labels.constBegin();
			while (it != labels.constEnd()) {
				if (!text.isEmpty())
					text += QLatin1Char('\n');
				text += it.key() + QLatin1String(" = ") + it.value();
				++it;
			}
			break;
		}
		case AbstractColumn::ColumnMode::Month:
		case AbstractColumn::ColumnMode::Day:
		case AbstractColumn::ColumnMode::DateTime: {
			auto labels = m_column->dateTimeValueLabels();
			const auto* filter = static_cast<DateTime2StringFilter*>(m_column->outputFilter());
			const auto& dateTimeFormat = filter->format();
			auto it = labels.constBegin();
			while (it != labels.constEnd()) {
				if (!text.isEmpty())
					text += QLatin1Char('\n');
				text += it.key().toString(dateTimeFormat) + QLatin1String(" = ") + it.value();
				++it;
			}
			break;
		}
		}

		teValueLabels->setText(text);
	} else {
		QString text = i18n("Provide the list of value-label pairs like in the following example:");
		text += QLatin1String("\n1 = true\n0 = false");
		teValueLabels->setPlaceholderText(text);
	}
}

void  BatchEditValueLabelsDialog::save() const {
	//remove all already available labels first
	for (auto* column : m_columns)
		column->clearValueLabels();

	//add new labels
	SET_NUMBER_LOCALE
	QString label;
	QString valueStr;
	bool ok;
	const auto mode = m_column->columnMode();
	const QString& text = teValueLabels->toPlainText();
	const auto& lines = text.split(QLatin1Char('\n'));

	for (const auto& line : lines) {
		auto pair = line.split(QLatin1Char('='));
		if (pair.size() != 2)
			continue;

		valueStr = pair.at(0).simplified();
		label = pair.at(1).simplified();
		if (valueStr.isEmpty() || label.isEmpty())
			continue;

		switch(mode) {
		case AbstractColumn::ColumnMode::Numeric: {
			double value = numberLocale.toDouble(valueStr, &ok);
			if (!ok)
				continue;
			for (auto* col : m_columns)
				col->addValueLabel(value, label);
			break;
		}
		case AbstractColumn::ColumnMode::Integer: {
			double value = numberLocale.toInt(valueStr, &ok);
			if (!ok)
				continue;
			for (auto* col : m_columns)
				col->addValueLabel(value, label);
			break;
		}
		case AbstractColumn::ColumnMode::BigInt: {
			double value = numberLocale.toLongLong(valueStr, &ok);
			if (!ok)
				continue;
			for (auto* col : m_columns)
				col->addValueLabel(value, label);
			break;
		}
		case AbstractColumn::ColumnMode::Text: {
			for (auto* col : m_columns)
				col->addValueLabel(valueStr, label);
			break;
		}
		case AbstractColumn::ColumnMode::Month:
		case AbstractColumn::ColumnMode::Day:
		case AbstractColumn::ColumnMode::DateTime: {
			const auto* filter = static_cast<DateTime2StringFilter*>(m_column->outputFilter());
			const QDateTime& value = QDateTime::fromString(valueStr, filter->format());
			qDebug()<<"value " << pair.at(0) << "  " << value;
			if (!value.isValid())
				continue;
			for (auto* col : m_columns)
				col->addValueLabel(value, label);
			break;
		}
		}
	}
}
