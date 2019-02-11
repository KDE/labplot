#ifndef MQTTSUBSCRIPTIONWIDGET_H
#define MQTTSUBSCRIPTIONWIDGET_H

#include <QWidget>
#include <QVector>

#ifdef HAVE_MQTT
#include "ui_mqttsubscriptionwidget.h"
class QMqttSubscription;
#endif

class MQTTSubscriptionWidget : public QWidget {
#ifdef HAVE_MQTT
    Q_OBJECT

public:
    explicit MQTTSubscriptionWidget(QWidget* parent = nullptr);
    ~MQTTSubscriptionWidget() override;
    enum MQTTParentWidget {
        ImportFileWidget = 0,
        LiveDataDock = 1
    };

    void setTopicList (QStringList topicList);
    QStringList getTopicList();\
    int subscriptionCount();
    QTreeWidgetItem* topLevelTopic(int);
    QTreeWidgetItem* topLevelSubscription(int);
    void addTopic(QTreeWidgetItem*);
    int topicCount();
    void setTopicTreeText(const QString&);
    void makeVisible(bool);
    void testSubscribe(QTreeWidgetItem*);
    void testUnsubscribe(QTreeWidgetItem*);

signals:
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



#endif	// HAVE_MQTT
};


#endif // MQTTSUBSCRIPTIONWIDGET_H
