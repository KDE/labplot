/*
	File                 : SpreadsheetSparkLineHeaderModel.cpp
	Project              : LabPlot
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2023 Kuntal Bar <barkuntal6@gmail.com>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "SpreadsheetSparkLineHeaderModel.h"
#include "backend/core/column/Column.h"
#include "qscreen.h"

#include <backend/worksheet/Worksheet.h>

#include <backend/worksheet/plots/cartesian/BarPlot.h>
#include <backend/worksheet/plots/cartesian/XYCurve.h>

#include <QIcon>

/*!
   \class SpreadsheetSparkLineHeaderModel
   \brief Model class wrapping a SpreadsheetModel to display column SparkLine in a SpreadsheetSparkLineHeaderView

\ingroup commonfrontend
*/

SpreadsheetSparkLinesHeaderModel::SpreadsheetSparkLinesHeaderModel(SpreadsheetModel* spreadsheet_model, QObject* parent)
	: QAbstractTableModel(parent)
	, m_spreadsheet_model(spreadsheet_model) {
	connect(m_spreadsheet_model, &SpreadsheetModel::headerDataChanged, this, &SpreadsheetSparkLinesHeaderModel::headerDataChanged);
	connect(m_spreadsheet_model, &SpreadsheetModel::columnsAboutToBeInserted, this, &SpreadsheetSparkLinesHeaderModel::columnsAboutToBeInserted);
	connect(m_spreadsheet_model, &SpreadsheetModel::columnsAboutToBeRemoved, this, &SpreadsheetSparkLinesHeaderModel::columnsAboutToBeRemoved);
	connect(m_spreadsheet_model, &SpreadsheetModel::columnsInserted, this, &SpreadsheetSparkLinesHeaderModel::columnsInserted);
	connect(m_spreadsheet_model, &SpreadsheetModel::columnsRemoved, this, &SpreadsheetSparkLinesHeaderModel::columnsRemoved);
}

Qt::ItemFlags SpreadsheetSparkLinesHeaderModel::flags(const QModelIndex& index) const {
	if (index.isValid())
		return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
	else
		return Qt::ItemIsEnabled;
}
void SpreadsheetSparkLinesHeaderModel::sparkLine(Column* col) {
	if (!col->isFirstSparkLineShown) {
		if (col->hasValues()) {
			col->isFirstSparkLineShown = true;
			col->setSparkline(SpreadsheetSparkLinesHeaderModel::showSparkLines(col));
		}
	}
}

// show sparkLine of respective column
QPixmap SpreadsheetSparkLinesHeaderModel::showSparkLines(const Column* col) {
	static const QString sparklineTheme = QStringLiteral("Sparkline");
	static const QString sparklineText = QStringLiteral("add-sparkline");
	auto* worksheet = new Worksheet(sparklineText);
	DEBUG(Q_FUNC_INFO << "Called")

	if (col->columnMode() == Column::ColumnMode::Text) {
		worksheet->setTheme(sparklineTheme);
		worksheet->view();
		worksheet->setLayoutBottomMargin(0);
		worksheet->setLayoutTopMargin(0);

		auto* plot = new CartesianPlot(sparklineText);
		plot->setTheme(sparklineTheme);
		plot->setVerticalPadding(0);
		plot->setHorizontalPadding(0);
		plot->setRightPadding(0);
		plot->setBottomPadding(0);
		worksheet->addChild(plot);

		auto* barPlot = new BarPlot(QString());
		plot->addChild(barPlot);

		barPlot->setOrientation(BarPlot::Orientation::Vertical);

		// generate columns holding the data and the labels
		auto* dataColumn = new Column(QStringLiteral("data"));
		dataColumn->setColumnMode(AbstractColumn::ColumnMode::Integer);

		// sort the frequencies and the accompanying labels
		const auto& frequencies = col->frequencies();
		auto i = frequencies.constBegin();
		QVector<QPair<QString, int>> pairs;
		while (i != frequencies.constEnd()) {
			pairs << QPair<QString, int>(i.key(), i.value());
			++i;
		}

		QVector<int> data;
		QVector<QString> labels;
		for (const auto& pair : pairs)
			data << pair.second;
		dataColumn->replaceInteger(0, data);
		QVector<const AbstractColumn*> columns;
		columns << dataColumn;
		barPlot->setDataColumns(columns);
		plot->scaleAuto(-1, -1);
		plot->retransform();
		worksheet->setSuppressLayoutUpdate(false);
		worksheet->updateLayout();
		// Export to pixmap
		QPixmap pixmap(worksheet->view()->size());
		const bool exportSuccess = worksheet->exportView(pixmap);

		// Use a placeholder preview if the view is not available yet
		if (!exportSuccess) {
			const auto placeholderIcon = QIcon::fromTheme(i18n("view-preview"));
			const int iconSize = std::ceil(5.0 / 2.54 * QApplication::primaryScreen()->physicalDotsPerInchX());
			pixmap = placeholderIcon.pixmap(iconSize, iconSize);
		}
		delete worksheet;

		return pixmap;
	} else {
		worksheet->setTheme(sparklineTheme);
		worksheet->view();
		worksheet->setLayoutBottomMargin(0);
		worksheet->setLayoutTopMargin(0);

		auto* plot = new CartesianPlot(sparklineText);
		plot->setType(CartesianPlot::Type::TwoAxes);
		plot->setTheme(sparklineTheme);
		plot->setVerticalPadding(0);
		plot->setHorizontalPadding(0);
		plot->setRightPadding(0);
		plot->setBottomPadding(0);

		const int rowCount = col->rowCount();
		QVector<double> xData(rowCount);

		for (int i = 0; i < rowCount; ++i)
			xData[i] = i;

		Column* xColumn = new Column(sparklineText, xData);
		worksheet->addChild(plot);

		auto* curve = new XYCurve(sparklineText);
		curve->setSuppressRetransform(true);
		curve->setXColumn(xColumn);
		curve->setYColumn(col);
		curve->setSuppressRetransform(false);
		plot->addChild(curve);

		plot->scaleAuto(-1, -1);
		plot->retransform();
		worksheet->setSuppressLayoutUpdate(false);
		worksheet->updateLayout();

		// Export to pixmap
		QPixmap pixmap(worksheet->view()->size());
		const bool exportSuccess = worksheet->exportView(pixmap);

		// Use a placeholder preview if the view is not available yet
		if (!exportSuccess) {
			const auto placeholderIcon = QIcon::fromTheme(i18n("view-preview"));
			const int iconSize = std::ceil(5.0 / 2.54 * QApplication::primaryScreen()->physicalDotsPerInchX());
			pixmap = placeholderIcon.pixmap(iconSize, iconSize);
		}

		delete worksheet;
		return pixmap;
	}
}

QVariant SpreadsheetSparkLinesHeaderModel::data(const QModelIndex& /*index*/, int /*role*/) const {
	return {};
}

QVariant SpreadsheetSparkLinesHeaderModel::headerData(int section, Qt::Orientation orientation, int role) const {
	if (orientation != Qt::Horizontal || section < 0 || section >= columnCount())
		return {};

	return {m_spreadsheet_model->headerData(section, Qt::Horizontal, role)};
}

int SpreadsheetSparkLinesHeaderModel::rowCount(const QModelIndex& /*parent*/) const {
	return m_spreadsheet_model->rowCount();
}

int SpreadsheetSparkLinesHeaderModel::columnCount(const QModelIndex& /*parent*/) const {
	return m_spreadsheet_model->columnCount();
}
