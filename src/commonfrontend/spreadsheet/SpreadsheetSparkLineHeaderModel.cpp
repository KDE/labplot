/*
	File                 : SpreadsheetSparkLineHeaderModel.cpp
	Project              : LabPlot
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2023 Kuntal Bar <barkuntal6@gmail.com>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "SpreadsheetSparkLineHeaderModel.h"
#include "backend/core/column/Column.h"
#include "backend/lib/trace.h"
#include "commonfrontend/spreadsheet/SparklineRunnable.h"
#include "qscreen.h"
#include "qtconcurrentrun.h"

#include <backend/worksheet/Worksheet.h>

#include <backend/worksheet/plots/cartesian/BarPlot.h>
#include <backend/worksheet/plots/cartesian/XYCurve.h>

#include <QFutureWatcher>
#include <QIcon>
#include <QThreadPool>

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
	if (col->mDataChanged) {
		if (col->hasValues()) {
			col->mDataChanged = false;
			col->setSparkline(SpreadsheetSparkLinesHeaderModel::showSparkLines(col));
		}
	}
}

// show sparkLine of respective column
QPixmap SpreadsheetSparkLinesHeaderModel::showSparkLines(Column* col) {
	// Create a QThreadPool instance and set the maximum number of threads
	QThreadPool threadPool;
	threadPool.setMaxThreadCount(QThread::idealThreadCount());
	PERFTRACE(QLatin1String(Q_FUNC_INFO));
	// Create a QFutureWatcher to monitor the task's progress
	QFutureWatcher<QPixmap> watcher;
	// Create an instance of SparkLineRunnable
	SparkLineRunnable* runnable = new SparkLineRunnable(col);

	// Connect the finished signal of the runnable to the watcher's setFuture slot
	QObject::connect(runnable, &SparkLineRunnable::taskFinished, [&]() {
		QPixmap resultPixmap = runnable->getResultPixmap();
		// Check if the result is valid
		if (!resultPixmap.isNull()) {
			watcher.setFuture(QtConcurrent::run([=]() {
				return resultPixmap;
			}));
		} else
			watcher.cancel();
	});

	// Start the runnable in the thread pool
	threadPool.start(runnable);

	// Wait for the task to finish
	QEventLoop loop;
	QObject::connect(runnable, &SparkLineRunnable::taskFinished, &loop, &QEventLoop::quit);
	loop.exec();

	return watcher.result();
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
