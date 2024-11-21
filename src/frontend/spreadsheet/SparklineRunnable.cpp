/*
	File                 : SparklineRunnable.cpp
	Project              : LabPlot
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2023 Kuntal Bar <barkuntal6@gmail.com>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "SparklineRunnable.h"
#include "backend/worksheet/Background.h"
#include "backend/worksheet/Worksheet.h"
#include "backend/worksheet/plots/cartesian/BarPlot.h"
#include "backend/worksheet/plots/cartesian/XYCurve.h"

#include <QWidget>

void SparkLineRunnable::run() {
	if (col->columnMode() != Column::ColumnMode::Text && !col->isPlottable()) {
		mPixmap = QPixmap(1, 1);
		mPixmap.fill(QColor(49, 54, 59));
		return;
	}

	static const QString sparklineTheme = QStringLiteral("Sparkline");
	static const QString sparklineText = QStringLiteral("add-sparkline");

	auto* worksheet = new Worksheet(sparklineText);
	worksheet->setUndoAware(false);
	worksheet->setUseViewSize(true);
	worksheet->setLayoutBottomMargin(0);
	worksheet->setLayoutTopMargin(0);
	worksheet->setLayoutLeftMargin(0);
	worksheet->setLayoutRightMargin(0);

	auto* plot = new CartesianPlot(sparklineText);
	plot->setSuppressRetransform(true);
	plot->setVerticalPadding(2);
	plot->setRightPadding(2);
	plot->setBottomPadding(2);
	plot->setHorizontalPadding(2);
	worksheet->addChild(plot);

	QApplication::processEvents(QEventLoop::AllEvents, 100);
	if (col->columnMode() == Column::ColumnMode::Text) {
		auto* barPlot = new BarPlot(QString());
		barPlot->setSuppressRetransform(true);
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
		for (const auto& pair : std::as_const(pairs))
			data << pair.second;
		dataColumn->replaceInteger(0, data);
		const QVector<const AbstractColumn*> columns{dataColumn};
		barPlot->setDataColumns(columns);
		plot->addChild(barPlot);
		barPlot->setSuppressRetransform(false);
	} else {
		const int rowCount = col->rowCount();
		QVector<int> xData(rowCount);
		xData.resize(rowCount);

		for (int i = 0; i < rowCount; ++i)
			xData[i] = i;

		auto* xColumn = new Column(QStringLiteral("x"), AbstractColumn::ColumnMode::Integer);
		xColumn->setIntegers(xData);

		auto* curve = new XYCurve(sparklineText);
		curve->setSuppressRetransform(false);
		curve->setXColumn(xColumn);
		curve->setYColumn(col);
		plot->addChild(curve);
		curve->setSuppressRetransform(false);
	}

	worksheet->setTheme(sparklineTheme);
	worksheet->background()->setOpacity(1);
	worksheet->background()->setFirstColor(QApplication::palette().color(QPalette::Base));
	plot->setSuppressRetransform(false);
	plot->retransform();

	// Export to pixmap
	mPixmap = QPixmap(worksheet->view()->size());
	worksheet->exportView(mPixmap);

	delete worksheet;
	Q_EMIT taskFinished(mPixmap);
}

QPixmap SparkLineRunnable::pixmap() {
	return mPixmap;
}
