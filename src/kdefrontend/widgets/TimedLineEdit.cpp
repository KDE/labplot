/*
	File                 : TimedLineEdit.cpp
	Project              : LabPlot
	Description          : Extended LineEdit to emit TextChanged event after some time has passed between edits
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2024 Israel Galadima <izzygaladima@gmail.com>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "TimedLineEdit.h"

TimedLineEdit::TimedLineEdit(QWidget* parent)
	: QLineEdit(parent) {
	m_timer.setSingleShot(true);
	connect(this, &QLineEdit::textChanged, [=]() {
		m_timer.start(m_time);
	});
	connect(&m_timer, &QTimer::timeout, [=]() {
		Q_EMIT textChanged();
	});
}

TimedLineEdit::TimedLineEdit(const QString& contents, QWidget* parent)
	: QLineEdit(contents, parent) {
	m_timer.setSingleShot(true);
	connect(this, &QLineEdit::textChanged, [=]() {
		m_timer.start(m_time);
	});
	connect(&m_timer, &QTimer::timeout, [=]() {
		Q_EMIT textChanged();
	});
}

int TimedLineEdit::getTime() {
	return m_time;
}

void TimedLineEdit::setTime(int time) {
	m_time = time;
	m_timer.start(m_time);
}
