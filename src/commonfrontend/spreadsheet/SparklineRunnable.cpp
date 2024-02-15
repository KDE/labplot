/*
	File                 : SparklineRunnable.cpp
	Project              : LabPlot
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2023 Kuntal Bar <barkuntal6@gmail.com>
	SPDX-License-Identifier: GPL-2.0-or-later
*/
#include "SparklineRunnable.h"
#include "backend/worksheet/Worksheet.h"
#include "backend/worksheet/plots/cartesian/BarPlot.h"
#include "backend/worksheet/plots/cartesian/XYCurve.h"
#include "qwidget.h"

void SparkLineRunnable::run() {
	if (col->columnMode() != Column::ColumnMode::Text && !col->isPlottable()) {
		mPixmap = QPixmap(1, 1);
		mPixmap.fill(QColor(49, 54, 59));
		return;
	}

	DEBUG(Q_FUNC_INFO)
	static const QString sparklineTheme = QStringLiteral("Sparkline");
	static const QString sparklineText = QStringLiteral("add-sparkline");
	auto* worksheet = new Worksheet(sparklineText);
	worksheet->setTheme(sparklineTheme);
	worksheet->setLayoutBottomMargin(0);
	worksheet->setLayoutTopMargin(0);
	worksheet->setLayoutLeftMargin(0);
	worksheet->setLayoutRightMargin(0);
	auto* plot = new CartesianPlot(sparklineText);
	plot->setTheme(sparklineTheme);
	worksheet->addChild(plot);
	if (col->columnMode() == Column::ColumnMode::Text) {
		auto* barPlot = new BarPlot(QString());
		plot->addChild(barPlot);
		plot->setVerticalPadding(0);
		plot->setRightPadding(0);
		plot->setBottomPadding(0);
		plot->setHorizontalPadding(0);

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
		for (const auto& pair : qAsConst(pairs))
			data << pair.second;
		dataColumn->replaceInteger(0, data);
		QVector<const AbstractColumn*> columns;
		columns << dataColumn;
		barPlot->setDataColumns(columns);

	} else {
		const int rowCount = col->rowCount();
		QVector<double> xData(rowCount);

		for (int i = 0; i < rowCount; ++i)
			xData[i] = i;

		Column* xColumn = new Column(sparklineText, xData);
		auto* curve = new XYCurve(sparklineText);
		curve->setSuppressRetransform(true);
		curve->setXColumn(xColumn);
		curve->setYColumn(col);
		curve->setSuppressRetransform(false);
		plot->addChild(curve);
		plot->setVerticalPadding(0);
		plot->setRightPadding(0);
		plot->setBottomPadding(0);
		plot->setHorizontalPadding(0);
	}
	worksheet->setSuppressLayoutUpdate(false);
	worksheet->updateLayout();
	// Export to pixmap
	QPixmap pixmap(worksheet->view()->size());
	worksheet->exportView(pixmap);
	delete worksheet;
	mPixmap = pixmap;
	Q_EMIT taskFinished(mPixmap);
}

QPixmap SparkLineRunnable::pixmap() {
	return mPixmap;
};
