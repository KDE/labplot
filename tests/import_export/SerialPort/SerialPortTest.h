/*
	File                 : SerialPortTest.h
	Project              : LabPlot
	Description          : Tests for Serial Port related features
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2025 Martin Marmsoler <martin.marmsoler@gmail.com>

	SPDX-License-Identifier: GPL-2.0-or-later
*/
#ifndef SERIALPORTTEST_H
#define SERIALPORTTEST_H

#include "../../CommonTest.h"

class SerialPortTest : public CommonTest {
	Q_OBJECT

private Q_SLOTS:
	void initTestCase() override;
	void cleanupTestCase();

	void testReading();

private:
	QProcess m_process_socat;
	QProcess m_process_send;
	QString m_receiverDevice;
	QString m_senderDevice;
	bool m_socat_command_available{false};
};

#endif
