/*
	File                 : TimedLineEdit.h
	Project              : LabPlot
	Description          : Extended LineEdit to emit TextChanged/TextEdited event after some time has passed between edits
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2024 Israel Galadima <izzygaladima@gmail.com>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef TIMEDLINEEDIT_H
#define TIMEDLINEEDIT_H

#include <QLineEdit>
#include <QString>
#include <QTimer>

class TimedLineEdit : public QLineEdit {
	Q_OBJECT

public:
	explicit TimedLineEdit(QWidget* parent = nullptr);
	explicit TimedLineEdit(const QString&, QWidget* parent = nullptr);

	int getTime();
	void setTime(int time);

private:
	int m_time{1000};
	QTimer m_textChangedTimer;
	QTimer m_textEditedTimer;

	void initTimers();

Q_SIGNALS:
	void textChanged();
	void textEdited();
};

#endif
