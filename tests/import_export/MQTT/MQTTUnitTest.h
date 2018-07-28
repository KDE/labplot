/***************************************************************************
	File                 : MQTTUnitTest.h
	Project              : LabPlot
	Description          : Tests for MQTT related features
	--------------------------------------------------------------------
	Copyright            : (C) 2018 Kovacs Ferencz (kferike98@gmail.com)
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *  This program is free software; you can redistribute it and/or modify   *
 *  it under the terms of the GNU General Public License as published by   *
 *  the Free Software Foundation; either version 2 of the License, or      *
 *  (at your option) any later version.                                    *
 *                                                                         *
 *  This program is distributed in the hope that it will be useful,        *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *  GNU General Public License for more details.                           *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the Free Software           *
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor,                    *
 *   Boston, MA  02110-1301  USA                                           *
 *                                                                         *
 ***************************************************************************/

#ifdef HAVE_MQTT
#include <QtTest/QtTest>

#ifndef MQTT_UNIT_TEST
#define MQTT_UNIT_TEST
#endif

class MQTTUnitTest : public QObject {
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
};

#endif //HAVE_MQTT
