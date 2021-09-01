/*
    File                 : ColumnDock.h
    Project              : LabPlot
    Description          : widget for column properties
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2011-2021 Alexander Semke (alexander.semke@web.de)
    SPDX-FileCopyrightText: 2017 Stefan Gerlach (stefan.gerlach@uni.kn)

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef COLUMNDOCK_H
#define COLUMNDOCK_H

#include "backend/core/column/Column.h"
#include "kdefrontend/dockwidgets/BaseDock.h"
#include "ui_columndock.h"

template <class T> class QList;

class ColumnDock : public BaseDock {
	Q_OBJECT

public:
	explicit ColumnDock(QWidget*);
	void setColumns(QList<Column*>);

private:
	Ui::ColumnDock ui;
	QList<Column*> m_columnsList;
	Column* m_column{nullptr};

	void updateTypeWidgets(AbstractColumn::ColumnMode);
	void showValueLabels();

private slots:
	void retranslateUi();

	void typeChanged(int);
	void numericFormatChanged(int);
	void precisionChanged(int);
	void dateTimeFormatChanged(const QString&);
	void plotDesignationChanged(int);

	//value labels
	void addLabel();
	void removeLabel();
	void batchEditLabels();

	//SLOTs for changes triggered in Column
	void columnModeChanged(const AbstractAspect*);
	void columnFormatChanged();
	void columnPrecisionChanged();
	void columnPlotDesignationChanged(const AbstractColumn*);

signals:
	void info(const QString&);
};

#endif // COLUMNDOCK_H
