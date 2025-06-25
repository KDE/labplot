/*
	File                 : RescaleDialog.h
	Project              : LabPlot
	Description          : Dialog to provide the rescale interval
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2020 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef RESCALEDIALOG_H
#define RESCALEDIALOG_H

#include <QDialog>
#include <ui_rescalewidget.h>
class Column;

class RescaleDialog : public QDialog {
	Q_OBJECT

public:
	explicit RescaleDialog(QWidget* parent = nullptr);
	~RescaleDialog() override;

	void setColumns(const QVector<Column*>&);
	double min() const;
	double max() const;

private:
	Ui::RescaleWidget ui;

private Q_SLOTS:
	void validateOkButton();
};

#endif
