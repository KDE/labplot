/***************************************************************************
    File                 : ImportFileWidget.h
    Project              : LabPlot
    Description          : MQTT related helper functions
    --------------------------------------------------------------------
    Copyright            : (C) 2019 Alexander Semke (alexander.semke@web.de)
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

#include <QVector>

class QTreeWidgetItem;
class QStringList;

class MQTTHelpers {
public:
	static void addSubscriptionChildren(QTreeWidgetItem*, QTreeWidgetItem*);
	static void restoreSubscriptionChildren(QTreeWidgetItem * topic, QTreeWidgetItem * subscription, const QStringList& list, int level);
	static void findSubscriptionLeafChildren(QVector<QTreeWidgetItem*>&, QTreeWidgetItem*);
	static int checkCommonChildCount(int levelIdx, int level, QStringList& namelist, QTreeWidgetItem* currentItem);
	static int commonLevelIndex(const QString& first, const QString& second);
	static QString checkCommonLevel(const QString&, const QString&);
	static bool checkTopicContains(const QString&, const QString&);
};
