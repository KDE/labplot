/*
	File                 : AddValueLabelDialog.h
	Project              : LabPlot
	Description          : Dialog to add a new the value label
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2021-2022 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef ADDVALUELABELDIALOG_H
#define ADDVALUELABELDIALOG_H

#include "backend/core/AbstractColumn.h"
#include <QDialog>

class Column;
class QComboBox;
class QDateTimeEdit;
class QLineEdit;

class AddValueLabelDialog : public QDialog {
	Q_OBJECT

public:
	explicit AddValueLabelDialog(QWidget* parent, const Column*);
	~AddValueLabelDialog() override;

	void setDateTimeFormat(const QString&);

	double value() const;
	int valueInt() const;
	qint64 valueBigInt() const;
	QString valueText() const;
	QDateTime valueDateTime() const;

	QString label() const;

private:
	QLineEdit* leValue{nullptr};
	QComboBox* cbValue{nullptr};
	QLineEdit* leLabel{nullptr};
	QDateTimeEdit* dateTimeEdit{nullptr};
};

#endif
