/*
	File                 : LiveDataTest.cpp
	Project              : LabPlot
	Description          : Tests for reading live data from files
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2024 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef LIVEDATATEST_H
#define LIVEDATATEST_H

#include "tests/CommonTest.h"

class QTcpServer;
class QUdpSocket;

class LiveDataTest : public CommonTest {
	Q_OBJECT

private:
	void initTestCase() override;
	void cleanupTestCase();

	template<typename T1, typename T2>
	bool waitForSignal(T1* sender, T2 signal) {
		QTimer timer(this);
		timer.setSingleShot(true);

		bool timeout = false;

		QEventLoop loop;
		QTimer::connect(&timer, &QTimer::timeout, [&loop, &timeout] {
			timeout = true;
			loop.quit();
		});

		connect(sender, signal, [&loop, &timer] {
			timer.stop();
			loop.quit();
		});
		timer.start(2000);
		loop.exec();

		return timeout;
	}

	QUdpSocket* m_udpSocket{nullptr};
	QTcpServer* m_tcpServer{nullptr};
	QTcpSocket* m_tcpSocket{nullptr};
	QTimer* m_tcpSendTimer{nullptr};

private Q_SLOTS:
	// Reading from files:

	// Continuous fixed - fixed amount of samples is processed starting from the beginning of the newly received data
	void testReadContinuousFixed00();
	void testReadContinuousFixed01();
	void testReadContinuousFixed02();
	void testReadContinuousFixedWithIndex();
	void testReadContinuousFixedWithTimestamp();
	void testReadContinuousFixedWithIndexTimestamp();

	// From End - fixed amount of samples is processed starting from the end of the newly received data
	void testReadFromEnd00();
	void testReadFromEnd01();
	void testReadFromEnd02();

	// Till the End - all newly received data is processed
	void testReadTillEnd00();
	void testReadTillEnd01();

	// Whole file - on every read the whole file is re-read completely and processed
	void testReadWholeFile00();
	void testReadWholeFile01();
	void testReadWholeFileSameContentSize();

	void testPlotting();

	// reading from TCP sockets:
	void testTcpReadContinuousFixed00();

	// reading from UDP sockets:
	void testUdpReadContinuousFixed00();

	void testLoadSaveLiveDataLinkedFile_FileExists();
	void testLoadSaveLiveDataLinkedFile_FileNotExists();
	void testLoadSaveLiveDataLinkedFile_FileNotExistsRemoveLivedata();
	void testLoadSaveLiveDataNoLinkedFile();
	// void testLoadSaveLiveDataSource();
};

#endif // LIVEDATATEST_H
