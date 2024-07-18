/*
	File                 : MQTTTest.h
	Project              : LabPlot
	Description          : Tests for MQTT related features
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2018 Kovacs Ferencz <kferike98@gmail.com>
	SPDX-FileCopyrightText: 2024 Stefan Gerlach <stefan.gerlach@uni.kn>

	SPDX-License-Identifier: GPL-2.0-or-later
*/
#ifndef MQTTTEST_H
#define MQTTTEST_H

#include "../../CommonTest.h"

class MQTTTest : public CommonTest {
#ifdef HAVE_MQTT
	Q_OBJECT

private Q_SLOTS:
	void initTestCase();

	// check superior and inferior relations
	void testContainFalse();
	void testContainTrue();

	// check common topics
	void testCommonTrue();
	void testCommonFalse();

	// test for different tipes of messages
	void testIntegerMessage();
	void testNumericMessage();
	void testTextMessage();

	// test subscribing and unsubscribing
	//  TODO: hangs
	//	void testSubscriptions();

private:
	QString m_dataDir;
#endif // HAVE_MQTT
};

#endif
