/*
	File                 : FitParametersWidget.cc
	Project              : LabPlot
	Description          : widget for editing fit parameters
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2014-2016 Alexander Semke <alexander.semke@web.de>
	SPDX-FileCopyrightText: 2016-2018 Stefan Gerlach <stefan.gerlach@uni.kn>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "FitParametersWidget.h"

#include <KLocalizedString>

#include <QCheckBox>
#include <QKeyEvent>
#include <QLineEdit>
#include <QScrollBar>

/*!
	\class FitParametersWidget
	\brief Widget for editing fit parameters. For predefined models the number of parameters,
	their names and default values are given - the user can change the start values.
	For custom models the user has to define here the parameter names and their start values.

	\ingroup kdefrontend
 */
FitParametersWidget::FitParametersWidget(QWidget* parent)
	: QWidget(parent) {
	ui.setupUi(this);

	ui.tableWidget->setColumnCount(5);

	auto* headerItem = new QTableWidgetItem();
	headerItem->setText(i18n("Name"));
	ui.tableWidget->setHorizontalHeaderItem(0, headerItem);

	headerItem = new QTableWidgetItem();
	headerItem->setText(i18n("Start value"));
	ui.tableWidget->setHorizontalHeaderItem(1, headerItem);

	headerItem = new QTableWidgetItem();
	headerItem->setText(i18n("Fixed"));
	ui.tableWidget->setHorizontalHeaderItem(2, headerItem);

	headerItem = new QTableWidgetItem();
	headerItem->setText(i18n("Lower limit"));
	ui.tableWidget->setHorizontalHeaderItem(3, headerItem);

	headerItem = new QTableWidgetItem();
	headerItem->setText(i18n("Upper limit"));
	ui.tableWidget->setHorizontalHeaderItem(4, headerItem);

	ui.tableWidget->horizontalHeader()->resizeSections(QHeaderView::ResizeToContents);
	ui.tableWidget->horizontalHeader()->setStretchLastSection(true);

	ui.tableWidget->installEventFilter(this);

	connect(ui.tableWidget, SIGNAL(cellChanged(int, int)), this, SLOT(changed()));
	updateTableSize();
}

void FitParametersWidget::setFitData(XYFitCurve::FitData* data) {
	DEBUG(Q_FUNC_INFO);
	m_initializing = true;
	m_fitData = data;

	int np = m_fitData->paramNames.size();
	DEBUG("# params = " << np);
	DEBUG("# start values = " << m_fitData->paramStartValues.size());
	const auto numberLocale = QLocale();
	if (m_fitData->modelCategory != nsl_fit_model_custom) { // pre-defined models
		ui.tableWidget->setRowCount(np);

		for (int i = 0; i < np; ++i) {
			// name
			auto* item = new QTableWidgetItem(m_fitData->paramNamesUtf8.at(i));
			item->setFlags(item->flags() ^ Qt::ItemIsEditable);
			item->setBackground(QApplication::palette().color(QPalette::Window));
			ui.tableWidget->setItem(i, 0, item);

			// start value
			auto* le = new QLineEdit(ui.tableWidget);
			le->setValidator(new QDoubleValidator(le));
			le->setFrame(false);
			le->insert(numberLocale.toString(m_fitData->paramStartValues.at(i)));
			ui.tableWidget->setCellWidget(i, 1, le);
			connect(le, SIGNAL(textChanged(QString)), this, SLOT(startValueChanged()));

			// fixed
			auto* widget = new QWidget();
			auto* cb = new QCheckBox();
			cb->setChecked(m_fitData->paramFixed.at(i));
			auto* cbl = new QHBoxLayout(widget);
			cbl->addWidget(cb);
			cbl->setAlignment(Qt::AlignCenter);
			cbl->setContentsMargins(0, 0, 0, 0);
			widget->setLayout(cbl);
			ui.tableWidget->setCellWidget(i, 2, widget);
			connect(cb, SIGNAL(stateChanged(int)), this, SLOT(changed()));

			// limits
			le = new QLineEdit(ui.tableWidget);
			le->setValidator(new QDoubleValidator(le));
			le->setFrame(false);
			if (m_fitData->paramLowerLimits.at(i) > -std::numeric_limits<double>::max())
				le->insert(numberLocale.toString(m_fitData->paramLowerLimits.at(i)));
			ui.tableWidget->setCellWidget(i, 3, le);
			connect(le, SIGNAL(textChanged(QString)), this, SLOT(lowerLimitChanged()));

			le = new QLineEdit(ui.tableWidget);
			le->setValidator(new QDoubleValidator(le));
			le->setFrame(false);
			if (m_fitData->paramUpperLimits.at(i) < std::numeric_limits<double>::max())
				le->insert(numberLocale.toString(m_fitData->paramUpperLimits.at(i)));
			ui.tableWidget->setCellWidget(i, 4, le);
			connect(le, SIGNAL(textChanged(QString)), this, SLOT(upperLimitChanged()));
		}
		ui.tableWidget->setCurrentCell(0, 1);
	} else { // custom model
		if (!m_fitData->paramNames.isEmpty()) { // parameters for the custom model are already available -> show them
			ui.tableWidget->setRowCount(np);

			for (int i = 0; i < np; ++i) {
				// name
				auto* item = new QTableWidgetItem(m_fitData->paramNames.at(i));
				item->setBackground(QApplication::palette().color(QPalette::Window));
				ui.tableWidget->setItem(i, 0, item);

				// start value
				auto* le = new QLineEdit(ui.tableWidget);
				le->setValidator(new QDoubleValidator(le));
				le->setFrame(false);
				le->insert(numberLocale.toString(m_fitData->paramStartValues.at(i)));
				ui.tableWidget->setCellWidget(i, 1, le);
				connect(le, SIGNAL(textChanged(QString)), this, SLOT(startValueChanged()));

				// fixed
				auto* widget = new QWidget();
				auto* cb = new QCheckBox();
				cb->setChecked(m_fitData->paramFixed.at(i));
				auto* cbl = new QHBoxLayout(widget);
				cbl->addWidget(cb);
				cbl->setAlignment(Qt::AlignCenter);
				cbl->setContentsMargins(0, 0, 0, 0);
				widget->setLayout(cbl);
				ui.tableWidget->setCellWidget(i, 2, widget);
				connect(cb, SIGNAL(stateChanged(int)), this, SLOT(changed()));

				// limits
				le = new QLineEdit(ui.tableWidget);
				le->setValidator(new QDoubleValidator(le));
				le->setFrame(false);
				if (m_fitData->paramLowerLimits.at(i) > -std::numeric_limits<double>::max())
					le->insert(numberLocale.toString(m_fitData->paramLowerLimits.at(i)));
				ui.tableWidget->setCellWidget(i, 3, le);
				connect(le, SIGNAL(textChanged(QString)), this, SLOT(lowerLimitChanged()));

				le = new QLineEdit(ui.tableWidget);
				le->setValidator(new QDoubleValidator(le));
				le->setFrame(false);
				if (m_fitData->paramUpperLimits.at(i) < std::numeric_limits<double>::max())
					le->insert(numberLocale.toString(m_fitData->paramUpperLimits.at(i)));
				ui.tableWidget->setCellWidget(i, 4, le);
				connect(le, SIGNAL(textChanged(QString)), this, SLOT(upperLimitChanged()));
			}
		} else { // no parameters available yet -> create the first row in the table for the first parameter
			ui.tableWidget->setRowCount(1);
			// name
			auto* item = new QTableWidgetItem();
			item->setBackground(QApplication::palette().color(QPalette::Window));
			ui.tableWidget->setItem(0, 0, item);

			// start value
			auto* le = new QLineEdit(ui.tableWidget);
			le->setValidator(new QDoubleValidator(le));
			le->setFrame(false);
			ui.tableWidget->setCellWidget(0, 1, le);
			connect(le, SIGNAL(textChanged(QString)), this, SLOT(startValueChanged()));

			// fixed
			auto* widget = new QWidget();
			auto* cb = new QCheckBox();
			auto* cbl = new QHBoxLayout(widget);
			cbl->addWidget(cb);
			cbl->setAlignment(Qt::AlignCenter);
			cbl->setContentsMargins(0, 0, 0, 0);
			widget->setLayout(cbl);
			ui.tableWidget->setCellWidget(0, 2, widget);
			connect(cb, SIGNAL(stateChanged(int)), this, SLOT(changed()));

			// limits
			le = new QLineEdit(ui.tableWidget);
			le->setValidator(new QDoubleValidator(le));
			le->setFrame(false);
			ui.tableWidget->setCellWidget(0, 3, le);
			connect(le, SIGNAL(textChanged(QString)), this, SLOT(lowerLimitChanged()));

			le = new QLineEdit(ui.tableWidget);
			le->setValidator(new QDoubleValidator(le));
			le->setFrame(false);
			ui.tableWidget->setCellWidget(0, 4, le);
			connect(le, SIGNAL(textChanged(QString)), this, SLOT(upperLimitChanged()));
		}
		ui.tableWidget->setCurrentCell(0, 0);
	}
	m_initializing = false;

	updateTableSize();
}

bool FitParametersWidget::eventFilter(QObject* watched, QEvent* event) {
	if (watched == ui.tableWidget) {
		if (event->type() == QEvent::KeyPress) {
			auto* keyEvent = static_cast<QKeyEvent*>(event);
			if (keyEvent->key() == Qt::Key_Return || keyEvent->key() == Qt::Key_Enter) {
				// on the second column with the values is editable.
				// navigate to the next cell in the second column
				if (ui.tableWidget->currentRow() == ui.tableWidget->rowCount() - 1)
					ui.tableWidget->clearSelection();
				else
					ui.tableWidget->setCurrentCell(ui.tableWidget->currentRow() + 1, 1);

				return true;
			}
		}
	}

	return QWidget::eventFilter(watched, event);
}

void FitParametersWidget::resizeEvent(QResizeEvent*) {
	updateTableSize();
}

void FitParametersWidget::updateTableSize() {
	if (m_resizing)
		return;

	m_resizing = true;
	auto horHeader = ui.tableWidget->horizontalHeader();
	auto vertHeader = ui.tableWidget->verticalHeader();
	int count = vertHeader->count();

	// show the vertrical scrollbar if we have more than 5 rows and limit the max size to 5 rows
	if (count <= 5)
		ui.tableWidget->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	else {
		ui.tableWidget->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
		count = 5;
	}

	// set the size of the table to the minimum possible
	int h = horHeader->height();
	h += vertHeader->sectionSize(0) * count;
	if (ui.tableWidget->horizontalScrollBar()->isVisible())
		h += ui.tableWidget->horizontalScrollBar()->height();
	setMinimumSize(16777215, h);
	setMaximumSize(16777215, h);
	m_resizing = false;
}

void FitParametersWidget::changed() {
	DEBUG("FitParametersWidget::changed()");
	if (!m_initializing) {
		apply();
		Q_EMIT parametersChanged(false);
	}
}

/*
 * Apply parameter settings by setting m_fitData
 */
void FitParametersWidget::apply() {
	DEBUG("FitParametersWidget::apply()");
	if (m_fitData->modelCategory != nsl_fit_model_custom) { // pre-defined models
		for (int i = 0; i < ui.tableWidget->rowCount(); ++i) {
			SET_DOUBLE_FROM_LE(m_fitData->paramStartValues[i], (QLineEdit*)ui.tableWidget->cellWidget(i, 1))

			QWidget* widget = ui.tableWidget->cellWidget(i, 2)->layout()->itemAt(0)->widget();
			m_fitData->paramFixed[i] = (qobject_cast<QCheckBox*>(widget))->isChecked();

			if (!((QLineEdit*)ui.tableWidget->cellWidget(i, 3))->text().isEmpty())
				SET_DOUBLE_FROM_LE(m_fitData->paramLowerLimits[i], (QLineEdit*)ui.tableWidget->cellWidget(i, 3))
			else
				m_fitData->paramLowerLimits[i] = -std::numeric_limits<double>::max();
			if (!((QLineEdit*)ui.tableWidget->cellWidget(i, 4))->text().isEmpty())
				SET_DOUBLE_FROM_LE(m_fitData->paramUpperLimits[i], (QLineEdit*)ui.tableWidget->cellWidget(i, 4))
			else
				m_fitData->paramUpperLimits[i] = std::numeric_limits<double>::max();
		}
	} else { // custom model
		m_fitData->paramNames.clear();
		m_fitData->paramNamesUtf8.clear();
		m_fitData->paramStartValues.clear();
		m_fitData->paramFixed.clear();
		m_fitData->paramLowerLimits.clear();
		m_fitData->paramUpperLimits.clear();

		const auto numberLocale = QLocale();
		for (int i = 0; i < ui.tableWidget->rowCount(); ++i) {
			// skip those rows where either the name or the value is empty
			if (!ui.tableWidget->item(i, 0)->text().simplified().isEmpty() && !((QLineEdit*)ui.tableWidget->cellWidget(i, 1))->text().simplified().isEmpty()) {
				m_fitData->paramNames.append(ui.tableWidget->item(i, 0)->text());
				m_fitData->paramNamesUtf8.append(ui.tableWidget->item(i, 0)->text());
				m_fitData->paramStartValues.append(numberLocale.toDouble(((QLineEdit*)ui.tableWidget->cellWidget(i, 1))->text()));

				QWidget* widget = ui.tableWidget->cellWidget(i, 2)->layout()->itemAt(0)->widget();
				m_fitData->paramFixed.append((qobject_cast<QCheckBox*>(widget))->isChecked());

				if (!((QLineEdit*)ui.tableWidget->cellWidget(i, 3))->text().isEmpty())
					m_fitData->paramLowerLimits.append(numberLocale.toDouble(((QLineEdit*)ui.tableWidget->cellWidget(i, 3))->text()));
				else
					m_fitData->paramLowerLimits.append(-std::numeric_limits<double>::max());
				if (!((QLineEdit*)ui.tableWidget->cellWidget(i, 4))->text().isEmpty())
					m_fitData->paramUpperLimits.append(numberLocale.toDouble(((QLineEdit*)ui.tableWidget->cellWidget(i, 4))->text()));
				else
					m_fitData->paramUpperLimits.append(std::numeric_limits<double>::max());
			}
		}
	}
}

/*
 * called when a start value is changed
 */
void FitParametersWidget::startValueChanged() {
	DEBUG("FitParametersWidget::startValueChanged()");
	const int row = ui.tableWidget->currentRow();
	double value{1};
	SET_DOUBLE_FROM_LE(value, (QLineEdit*)ui.tableWidget->cellWidget(row, 1))

	double lowerLimit{-std::numeric_limits<double>::max()}, upperLimit{std::numeric_limits<double>::max()};
	if (!((QLineEdit*)ui.tableWidget->cellWidget(row, 3))->text().isEmpty())
		SET_DOUBLE_FROM_LE(lowerLimit, (QLineEdit*)ui.tableWidget->cellWidget(row, 3))
	if (!((QLineEdit*)ui.tableWidget->cellWidget(row, 4))->text().isEmpty())
		SET_DOUBLE_FROM_LE(upperLimit, (QLineEdit*)ui.tableWidget->cellWidget(row, 4))

	const bool invalid = (value < lowerLimit || value > upperLimit);
	highlightInvalid(row, 1, invalid);
	if (invalid)
		m_invalidRanges = true;

	if (m_rehighlighting)
		return;

	// start value was changed -> check whether the lower and upper limits are valid and highlight them if not
	m_invalidRanges = invalid;
	m_rehighlighting = true;
	lowerLimitChanged();
	upperLimitChanged();
	m_rehighlighting = false;

	changed();
}

// check if lower limit fits to start value and upper limit
void FitParametersWidget::lowerLimitChanged() {
	DEBUG("FitParametersWidget::lowerLimitChanged()");
	const int row = ui.tableWidget->currentRow();

	double value{1};
	SET_DOUBLE_FROM_LE(value, (QLineEdit*)ui.tableWidget->cellWidget(row, 1))

	double lowerLimit{-std::numeric_limits<double>::max()}, upperLimit{std::numeric_limits<double>::max()};
	if (!((QLineEdit*)ui.tableWidget->cellWidget(row, 3))->text().isEmpty())
		SET_DOUBLE_FROM_LE(lowerLimit, (QLineEdit*)ui.tableWidget->cellWidget(row, 3))
	if (!((QLineEdit*)ui.tableWidget->cellWidget(row, 4))->text().isEmpty())
		SET_DOUBLE_FROM_LE(upperLimit, (QLineEdit*)ui.tableWidget->cellWidget(row, 4))

	const bool invalid = (lowerLimit > value || lowerLimit > upperLimit);
	highlightInvalid(row, 3, invalid);
	if (invalid)
		m_invalidRanges = true;

	if (m_rehighlighting)
		return;

	// lower limit was changed -> check whether the start value and the upper limit are valid and highlight them if not
	m_invalidRanges = invalid;
	m_rehighlighting = true;
	startValueChanged();
	upperLimitChanged();
	m_rehighlighting = false;

	changed();
}

// check if upper limit fits to start value and lower limit
void FitParametersWidget::upperLimitChanged() {
	DEBUG("FitParametersWidget::upperLimitChanged()");
	const int row = ui.tableWidget->currentRow();

	double value{1};
	SET_DOUBLE_FROM_LE(value, (QLineEdit*)ui.tableWidget->cellWidget(row, 1))

	double lowerLimit{-std::numeric_limits<double>::max()}, upperLimit{std::numeric_limits<double>::max()};
	if (!((QLineEdit*)ui.tableWidget->cellWidget(row, 3))->text().isEmpty())
		SET_DOUBLE_FROM_LE(lowerLimit, (QLineEdit*)ui.tableWidget->cellWidget(row, 3))
	if (!((QLineEdit*)ui.tableWidget->cellWidget(row, 4))->text().isEmpty())
		SET_DOUBLE_FROM_LE(upperLimit, (QLineEdit*)ui.tableWidget->cellWidget(row, 4))

	const bool invalid = (upperLimit < value || upperLimit < lowerLimit);
	highlightInvalid(row, 4, invalid);
	if (invalid)
		m_invalidRanges = true;

	if (m_rehighlighting)
		return;

	// upper limit was changed -> check whether the start value and the lower limit are valid and highlight them if not
	m_invalidRanges = invalid;
	m_rehighlighting = true;
	startValueChanged();
	lowerLimitChanged();
	m_rehighlighting = false;

	changed();
}

void FitParametersWidget::highlightInvalid(int row, int col, bool invalid) {
	QLineEdit* le = ((QLineEdit*)ui.tableWidget->cellWidget(row, col));
	if (invalid)
		SET_WARNING_STYLE(le)
	else
		le->setStyleSheet(QString());

	if (m_invalidRanges)
		Q_EMIT parametersValid(false);
	else
		Q_EMIT parametersValid(true);
}
