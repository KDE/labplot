/*
    File                 : MQTTSubscriptionWidget.h
    Project              : LabPlot
    Description          : manage topics and subscribing
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2019 Kovacs Ferencz <kferike98@gmail.com>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef MQTTSUBSCRIPTIONWIDGET_H
#define MQTTSUBSCRIPTIONWIDGET_H

#include "ui_mqttsubscriptionwidget.h"
class QMqttSubscription;

class MQTTSubscriptionWidget : public QWidget {
	Q_OBJECT

public:
	explicit MQTTSubscriptionWidget(QWidget* parent = nullptr);
	~MQTTSubscriptionWidget() override;
	enum class MQTTParentWidget {
		ImportFileWidget,
		LiveDataDock
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
