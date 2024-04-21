/*
	File                 : TimedLineEdit.cpp
	Project              : LabPlot
	Description          : Extended LineEdit to emit TextChanged/TextEdited event after some time has passed between edits
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2024 Israel Galadima <izzygaladima@gmail.com>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "TimedLineEdit.h"

TimedLineEdit::TimedLineEdit(QWidget* parent)
	: QLineEdit(parent) {
	initTimers();
}

TimedLineEdit::TimedLineEdit(const QString& contents, QWidget* parent)
	: QLineEdit(contents, parent) {
	initTimers();
}

void TimedLineEdit::initTimers() {
	m_textChangedTimer.setSingleShot(true);
	connect(this, &QLineEdit::textChanged, [&]() {
		m_textChangedTimer.start(m_time);
	});
	connect(&m_textChangedTimer, &QTimer::timeout, [&]() {
		Q_EMIT textChanged();
	});

	m_textEditedTimer.setSingleShot(true);
	connect(this, &QLineEdit::textEdited, [&]() {
		m_textEditedTimer.start(m_time);
	});
	connect(&m_textEditedTimer, &QTimer::timeout, [&]() {
		Q_EMIT textEdited();
	});
}

int TimedLineEdit::getTime() {
	return m_time;
}

void TimedLineEdit::setTime(int time) {
	m_time = time;
}
