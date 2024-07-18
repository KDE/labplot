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
	connect(this, &QLineEdit::textChanged, [&]() {
		if (m_textChangedTimerId != -1)
			killTimer(m_textChangedTimerId);
		m_textChangedTimerId = startTimer(m_time);
	});

	connect(this, &QLineEdit::textEdited, [&]() {
		if (m_textEditedTimerId != -1)
			killTimer(m_textEditedTimerId);
		m_textEditedTimerId = startTimer(m_time);
	});
}

void TimedLineEdit::timerEvent(QTimerEvent* event) {
	if (event->timerId() == m_textChangedTimerId) {
		killTimer(m_textChangedTimerId);
		m_textChangedTimerId = -1;
		Q_EMIT textChanged();
	} else if (event->timerId() == m_textEditedTimerId) {
		killTimer(m_textEditedTimerId);
		m_textEditedTimerId = -1;
		Q_EMIT textEdited();
	}
}

int TimedLineEdit::getTime() {
	return m_time;
}

void TimedLineEdit::setTime(int time) {
	m_time = time;
}
