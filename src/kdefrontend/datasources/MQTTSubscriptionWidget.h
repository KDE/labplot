/***************************************************************************
File                 : MQTTSubscriptionWidget.h
Project              : LabPlot
Description          : manage topics and subscribing
--------------------------------------------------------------------
Copyright            : (C) 2019 by Kovacs Ferencz (kferike98@gmail.com)
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

#ifndef MQTTSUBSCRIPTIONWIDGET_H
#define MQTTSUBSCRIPTIONWIDGET_H

#include <QVector>

#include "ui_mqttsubscriptionwidget.h"
class QMqttSubscription;

class MQTTSubscriptionWidget : public QWidget {
	Q_OBJECT

public:
	explicit MQTTSubscriptionWidget(QWidget* parent = nullptr);
	~MQTTSubscriptionWidget() override;
	enum MQTTParentWidget {
		ImportFileWidget = 0,
		LiveDataDock = 1
	};

	void setTopicList(const QStringList& topicList);
	QStringList getTopicList();

	int subscriptionCount();
	QTreeWidgetItem* topLevelTopic(int);
	QTreeWidgetItem* topLevelSubscription(int);
	QTreeWidgetItem* currentItem() const;
	void addTopic(QTreeWidgetItem*);
	int topicCount();
	void setTopicTreeText(const QString&);
	void makeVisible(bool);
	void testSubscribe(QTreeWidgetItem*);
	void testUnsubscribe(QTreeWidgetItem*);

	static bool checkTopicContains(const QString&, const QString&);
	static void findSubscriptionLeafChildren(QVector<QTreeWidgetItem*>&, QTreeWidgetItem*);

signals:
	void subscriptionChanged();
	void makeSubscription(const QString& name, quint8 QoS);
	void MQTTUnsubscribeFromTopic(const QString&, QVector<QTreeWidgetItem*> children);
	void removeMQTTSubscription(const QString&);
	void addBeforeRemoveSubscription(const QString&, quint8);
	void reparentTopic(const QString& topic, const QString& parent);
	void enableWill(bool);

private:
	Ui::MQTTSubscriptionWidget ui;
	MQTTParentWidget m_parent;
	QWidget* m_parentWidget;
	QCompleter* m_subscriptionCompleter{nullptr};
	QCompleter* m_topicCompleter{nullptr};
	QStringList m_topicList;
	bool m_searching{false};
	QTimer* m_searchTimer;

	void unsubscribeFromTopic(const QString&);
	void manageCommonLevelSubscriptions();
	void updateSubscriptionCompleter();

	static void addSubscriptionChildren(QTreeWidgetItem*, QTreeWidgetItem*);
	static void restoreSubscriptionChildren(QTreeWidgetItem * topic, QTreeWidgetItem * subscription, const QStringList& list, int level);
	static int checkCommonChildCount(int levelIdx, int level, QStringList& namelist, QTreeWidgetItem* currentItem);
	static int commonLevelIndex(const QString& first, const QString& second);
	static QString checkCommonLevel(const QString&, const QString&);

private slots:
	void mqttAvailableTopicDoubleClicked(QTreeWidgetItem* item, int column);
	void mqttSubscribedTopicDoubleClicked(QTreeWidgetItem* item, int column);
	void mqttSubscribe();
	void mqttUnsubscribe();
	void setTopicCompleter(const QString&);
	void scrollToTopicTreeItem(const QString& rootName);
	void scrollToSubsriptionTreeItem(const QString& rootName);
	void topicTimeout();
	void updateSubscriptionTree(const QVector<QString>&);
	void clearWidgets();
	void onDisconnect();
};

#endif // MQTTSUBSCRIPTIONWIDGET_H
