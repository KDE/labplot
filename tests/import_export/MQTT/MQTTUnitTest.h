/*
	File                 : MQTTUnitTest.h
	Project              : LabPlot
	Description          : Tests for MQTT related features
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2018 Kovacs Ferencz (kferike98@gmail.com)
*/

/***************************************************************************
 *                                                                         *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *                                                                         *
 ***************************************************************************/
#ifndef MQTTUNITTEST_H
#define MQTTUNITTEST_H

#include <QtTest>

class MQTTUnitTest : public QObject {
#ifdef HAVE_MQTT
	Q_OBJECT	

private slots:
	void initTestCase();

	//check superior and inferior relations
	void testContainFalse();
	void testContainTrue();

	//check common topics
	void testCommonTrue();
	void testCommonFalse();

	//test for different tipes of messages
	void testIntegerMessage();
	void testNumericMessage();
	void testTextMessage();

	//test subscribing and unsubscribing
	void testSubscriptions();



private:
	QString m_dataDir;
#endif //HAVE_MQTT
};

#endif
