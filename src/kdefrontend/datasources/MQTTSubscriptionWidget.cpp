/***************************************************************************
File                 : MQTTSubscriptionWidget.cpp
Project              : LabPlot
Description          : Widget for managing topics and subscribing
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

#include "MQTTSubscriptionWidget.h"

#include "backend/datasources/MQTTClient.h"
#include "ImportFileWidget.h"
#include "kdefrontend/dockwidgets/LiveDataDock.h"

#include <QCompleter>
#include <QMessageBox>
#include <QMqttSubscription>
#include <QTreeWidget>
#include <QTreeWidgetItem>

#include <KLocalizedString>

/*!
   \class MQTTSubscriptionWidget
   \brief Widget for managing topics and subscribing.

   \ingroup kdefrontend
*/
MQTTSubscriptionWidget::MQTTSubscriptionWidget(QWidget* parent) : QWidget(parent),
	m_searchTimer(new QTimer(this)) {

	ui.setupUi(this);

	m_searchTimer->setInterval(10000);
	const int size = ui.leTopics->height();
	ui.lTopicSearch->setPixmap( QIcon::fromTheme(QLatin1String("go-next")).pixmap(size, size) );
	ui.lSubscriptionSearch->setPixmap( QIcon::fromTheme(QLatin1String("go-next")).pixmap(size, size) );
	ui.bSubscribe->setIcon(ui.bSubscribe->style()->standardIcon(QStyle::SP_ArrowRight));
	ui.bSubscribe->setToolTip(i18n("Subscribe selected topics"));
	ui.bUnsubscribe->setIcon(ui.bUnsubscribe->style()->standardIcon(QStyle::SP_ArrowLeft));
	ui.bUnsubscribe->setToolTip(i18n("Unsubscribe selected topics"));

	//subscribe/unsubscribe buttons only enabled if something was selected
	ui.bSubscribe->setEnabled(false);
	ui.bUnsubscribe->setEnabled(false);

	QString info = i18n("Enter the name of the topic to navigate to it.");
	QString placeholder = i18n("Enter the name of the topic");
	ui.lTopicSearch->setToolTip(info);
	ui.leTopics->setToolTip(info);
	ui.leTopics->setPlaceholderText(placeholder);
	ui.lSubscriptionSearch->setToolTip(info);
	ui.leSubscriptions->setToolTip(info);
	ui.leSubscriptions->setPlaceholderText(placeholder);

	info = i18n("Set the Quality of Service (QoS) for the subscription to define the guarantee of the message delivery:"
	            "<ul>"
	            "<li>0 - deliver at most once</li>"
	            "<li>1 - deliver at least once</li>"
	            "<li>2 - deliver exactly once</li>"
	            "</ul>");
	ui.cbQos->setToolTip(info);

	auto* importWidget = dynamic_cast<ImportFileWidget*>(parent);
	if (importWidget) {
		m_parent = MQTTParentWidget::ImportFileWidget;
		connect(importWidget, &ImportFileWidget::newTopic, this, &MQTTSubscriptionWidget::setTopicCompleter);
		connect(importWidget, &ImportFileWidget::updateSubscriptionTree, this, &MQTTSubscriptionWidget::updateSubscriptionTree);
		connect(importWidget, &ImportFileWidget::MQTTClearTopics, this, &MQTTSubscriptionWidget::clearWidgets);
	} else {
		auto* liveDock = static_cast<LiveDataDock*>(parent);
		m_parent = MQTTParentWidget::ImportFileWidget;
		connect(liveDock, &LiveDataDock::MQTTClearTopics, this, &MQTTSubscriptionWidget::clearWidgets);
		connect(liveDock, &LiveDataDock::newTopic, this, &MQTTSubscriptionWidget::setTopicCompleter);
		connect(liveDock, &LiveDataDock::updateSubscriptionTree, this, &MQTTSubscriptionWidget::updateSubscriptionTree);
	}

	connect(ui.bSubscribe,  &QPushButton::clicked, this, &MQTTSubscriptionWidget::mqttSubscribe);
	connect(ui.bUnsubscribe, &QPushButton::clicked, this,&MQTTSubscriptionWidget::mqttUnsubscribe);

	connect(m_searchTimer, &QTimer::timeout, this, &MQTTSubscriptionWidget::topicTimeout);
	connect(ui.leTopics, &QLineEdit::textChanged, this, &MQTTSubscriptionWidget::scrollToTopicTreeItem);
	connect(ui.leSubscriptions, &QLineEdit::textChanged, this, &MQTTSubscriptionWidget::scrollToSubsriptionTreeItem);
	connect(ui.twTopics, &QTreeWidget::itemDoubleClicked, this, &MQTTSubscriptionWidget::mqttAvailableTopicDoubleClicked);
	connect(ui.twSubscriptions, &QTreeWidget::itemDoubleClicked, this, &MQTTSubscriptionWidget::mqttSubscribedTopicDoubleClicked);
	connect(ui.twSubscriptions, &QTreeWidget::currentItemChanged, this, &MQTTSubscriptionWidget::subscriptionChanged);

	connect(ui.twTopics, &QTreeWidget::itemSelectionChanged, this, [=]() {
		ui.bSubscribe->setEnabled(!ui.twTopics->selectedItems().isEmpty());
	});

	connect(ui.twSubscriptions, &QTreeWidget::itemSelectionChanged, this, [=]() {
		ui.bUnsubscribe->setEnabled(!ui.twSubscriptions->selectedItems().isEmpty());
	});
}

MQTTSubscriptionWidget::~MQTTSubscriptionWidget() {
	m_searchTimer->stop();
	delete m_searchTimer;
}

void MQTTSubscriptionWidget::setTopicList(const QStringList& topicList) {
	m_topicList = topicList;
}

QStringList MQTTSubscriptionWidget::getTopicList() {
	return m_topicList;
}

int MQTTSubscriptionWidget::subscriptionCount() {
	return ui.twSubscriptions->topLevelItemCount();
}

QTreeWidgetItem* MQTTSubscriptionWidget::topLevelTopic(int index) {
	return ui.twTopics->topLevelItem(index);
}

QTreeWidgetItem* MQTTSubscriptionWidget::topLevelSubscription(int index) {
	return ui.twSubscriptions->topLevelItem(index);
}

void MQTTSubscriptionWidget::addTopic(QTreeWidgetItem* item) {
	ui.twTopics->addTopLevelItem(item);
}

int MQTTSubscriptionWidget::topicCount() {
	return ui.twTopics->topLevelItemCount();
}

void MQTTSubscriptionWidget::setTopicTreeText(const QString &text) {
	ui.twTopics->headerItem()->setText(0, text);
}

QTreeWidgetItem* MQTTSubscriptionWidget::currentItem() const {
	return ui.twSubscriptions->currentItem();
}

void MQTTSubscriptionWidget::makeVisible(bool visible) {
	ui.cbQos->setVisible(visible);
	ui.twTopics->setVisible(visible);
	ui.twSubscriptions->setVisible(visible);
	ui.leTopics->setVisible(visible);
	ui.leSubscriptions->setVisible(visible);
	ui.bSubscribe->setVisible(visible);
	ui.bUnsubscribe->setVisible(visible);
	ui.lTopicSearch->setVisible(visible);
	ui.lSubscriptionSearch->setVisible(visible);
}

void MQTTSubscriptionWidget::testSubscribe(QTreeWidgetItem *item) {
	ui.twTopics->setCurrentItem(item);
	mqttSubscribe();
}

void MQTTSubscriptionWidget::testUnsubscribe(QTreeWidgetItem *item) {
	ui.twTopics->setCurrentItem(item);
	mqttUnsubscribe();
}

/*!
 *\brief Fills the children vector, with the root item's (twSubscriptions) leaf children (meaning no wildcard containing topics)
 *
 * \param children vector of TreeWidgetItem pointers
 * \param root pointer to a TreeWidgetItem of twSubscriptions
 */
void MQTTSubscriptionWidget::findSubscriptionLeafChildren(QVector<QTreeWidgetItem *>& children, QTreeWidgetItem* root) {
	if (root->childCount() == 0)
		children.push_back(root);
	else
		for (int i = 0; i < root->childCount(); ++i)
			findSubscriptionLeafChildren(children, root->child(i));
}

/*!
 *\brief Checks if a topic contains another one
 *
 * \param superior the name of a topic
 * \param inferior the name of a topic
 * \return	true if superior is equal to or contains(if superior contains wildcards) inferior,
 *			false otherwise
 */
bool MQTTSubscriptionWidget::checkTopicContains(const QString& superior, const QString& inferior) {
	if (superior == inferior)
		return true;

	if (!superior.contains('/'))
		return false;

	const QStringList& superiorList = superior.split('/', QString::SkipEmptyParts);
	const QStringList& inferiorList = inferior.split('/', QString::SkipEmptyParts);

	//a longer topic can't contain a shorter one
	if (superiorList.size() > inferiorList.size())
		return false;

	bool ok = true;
	for (int i = 0; i < superiorList.size(); ++i) {
		if (superiorList.at(i) != inferiorList.at(i)) {
			if ((superiorList.at(i) != "+") &&
			        !(superiorList.at(i) == "#" && i == superiorList.size() - 1)) {
				//if the two topics differ, and the superior's current level isn't + or #(which can be only in the last position)
				//then superior can't contain inferior
				ok = false;
				break;
			} else if (i == superiorList.size() - 1 && (superiorList.at(i) == "+" && inferiorList.at(i) == "#") ) {
				//if the two topics differ at the last level
				//and the superior's current level is + while the inferior's is #(which can be only in the last position)
				//then superior can't contain inferior
				ok = false;
				break;
			}
		}
	}
	return ok;
}

/*!
 *\brief Starts unsubscribing from the given topic, and signals to ImportFileWidget for further actions
 *
 * \param topicName the name of a topic we want to unsubscribe from
 */
void MQTTSubscriptionWidget::unsubscribeFromTopic(const QString& topicName) {
	if (topicName.isEmpty())
		return;

	QVector<QTreeWidgetItem*> children;
	findSubscriptionLeafChildren(children, ui.twSubscriptions->topLevelItem(0));

	//signals for ImportFileWidget
	emit MQTTUnsubscribeFromTopic(topicName, children);

	for (int row = 0; row < ui.twSubscriptions->topLevelItemCount(); row++)  {
		if (ui.twSubscriptions->topLevelItem(row)->text(0) == topicName) {
			ui.twSubscriptions->topLevelItem(row)->takeChildren();
			ui.twSubscriptions->takeTopLevelItem(row);
		}
	}
}

/*!
 *\brief We search in twSubscriptions for topics that can be represented using + wildcards, then merge them.
 *		 We do this until there are no topics to merge
 */
void MQTTSubscriptionWidget::manageCommonLevelSubscriptions() {
	bool foundEqual = false;

	do {
		foundEqual = false;
		QMap<QString, QVector<QString>> equalTopicsMap;
		QVector<QString> equalTopics;

		//compare the subscriptions present in the TreeWidget
		for (int i = 0; i < ui.twSubscriptions->topLevelItemCount() - 1; ++i) {
			for (int j = i + 1; j < ui.twSubscriptions->topLevelItemCount(); ++j) {
				QString commonTopic = checkCommonLevel(ui.twSubscriptions->topLevelItem(i)->text(0), ui.twSubscriptions->topLevelItem(j)->text(0));

				//if there is a common topic for the 2 compared topics, we add them to the map (using the common topic as key)
				if (!commonTopic.isEmpty()) {
					if (!equalTopicsMap[commonTopic].contains(ui.twSubscriptions->topLevelItem(i)->text(0)))
						equalTopicsMap[commonTopic].push_back(ui.twSubscriptions->topLevelItem(i)->text(0));

					if (!equalTopicsMap[commonTopic].contains(ui.twSubscriptions->topLevelItem(j)->text(0)))
						equalTopicsMap[commonTopic].push_back(ui.twSubscriptions->topLevelItem(j)->text(0));
				}
			}
		}

		if (!equalTopicsMap.isEmpty()) {
			DEBUG("Manage common topics");

			QVector<QString> commonTopics;
			QMapIterator<QString, QVector<QString>> topics(equalTopicsMap);

			//check for every map entry, if the found topics can be merged or not
			while (topics.hasNext()) {
				topics.next();

				int level = commonLevelIndex(topics.value().last(), topics.value().first());
				QStringList commonList = topics.value().first().split('/', QString::SkipEmptyParts);
				QTreeWidgetItem* currentItem = nullptr;

				//search the corresponding item to the common topics first level(root)
				for (int i = 0; i < ui.twTopics->topLevelItemCount(); ++i) {
					if (ui.twTopics->topLevelItem(i)->text(0) == commonList.first()) {
						currentItem = ui.twTopics->topLevelItem(i);
						break;
					}
				}

				if (!currentItem)
					break;

				//calculate the number of topics the new + wildcard could replace
				int childCount = checkCommonChildCount(1, level, commonList, currentItem);
				if (childCount > 0) {
					//if the number of topics found and the calculated number of topics is equal, the topics can be merged
					if (topics.value().size() == childCount) {
						QDEBUG("Found common topic to manage: " << topics.key());
						foundEqual = true;
						commonTopics.push_back(topics.key());
					}
				}
			}

			if (foundEqual) {
				//if there are more common topics, the topics of which can be merged, we choose the one which has the lowest level new '+' wildcard
				int lowestLevel = INT_MAX;
				int topicIdx = -1;
				for (int i = 0; i < commonTopics.size(); ++i) {
					int level = commonLevelIndex(equalTopicsMap[commonTopics[i]].first(), commonTopics[i]);
					if (level < lowestLevel) {
						topicIdx = i;
						lowestLevel = level;
					}
				}
				QDEBUG("Manage: " << commonTopics[topicIdx]);
				if (topicIdx != -1)
					equalTopics.append(equalTopicsMap[commonTopics[topicIdx]]);

				//Add the common topic ("merging")
				QString commonTopic;
				commonTopic = checkCommonLevel(equalTopics.first(), equalTopics.last());
				QStringList nameList;
				nameList.append(commonTopic);
				auto* newTopic = new QTreeWidgetItem(nameList);
				ui.twSubscriptions->addTopLevelItem(newTopic);

				if (m_parent == MQTTParentWidget::ImportFileWidget)
					emit makeSubscription(commonTopic, static_cast<quint8> (ui.cbQos->currentText().toUInt()));

				//remove the "merged" topics
				for (const auto& topic : equalTopics) {
					for (int j = 0; j < ui.twSubscriptions->topLevelItemCount(); ++j) {
						if (ui.twSubscriptions->topLevelItem(j)->text(0) == topic) {
							newTopic->addChild(ui.twSubscriptions->takeTopLevelItem(j));

							if (m_parent == MQTTParentWidget::ImportFileWidget)
								unsubscribeFromTopic(topic);

							break;
						}
					}
				}

				//remove any subscription that the new subscription contains
				for (int i = 0; i < ui.twSubscriptions->topLevelItemCount(); ++i) {
					if (checkTopicContains(commonTopic, ui.twSubscriptions->topLevelItem(i)->text(0)) &&
					        commonTopic != ui.twSubscriptions->topLevelItem(i)->text(0) ) {
						if (m_parent == MQTTParentWidget::ImportFileWidget)
							unsubscribeFromTopic(ui.twSubscriptions->topLevelItem(i)->text(0));
						else {
							ui.twSubscriptions->topLevelItem(i)->takeChildren();
							ui.twSubscriptions->takeTopLevelItem(i);
						}
						i--;
					}
				}

				if (m_parent == MQTTParentWidget::LiveDataDock)
					emit makeSubscription(commonTopic, static_cast<quint8> (ui.cbQos->currentText().toUInt()));
			}
		}
	} while (foundEqual);
}

/*!
 *\brief Fills twSubscriptions with the subscriptions made by the client
 */
void MQTTSubscriptionWidget::updateSubscriptionTree(const QVector<QString>& mqttSubscriptions) {
	DEBUG("ImportFileWidget::updateSubscriptionTree()");
	ui.twSubscriptions->clear();

	for (const auto& sub : mqttSubscriptions) {
		QStringList name;
		name.append(sub);

		bool found = false;
		for (int j = 0; j < ui.twSubscriptions->topLevelItemCount(); ++j) {
			if (ui.twSubscriptions->topLevelItem(j)->text(0) == sub) {
				found = true;
				break;
			}
		}

		if (!found) {
			//Add the subscription to the tree widget
			auto* newItem = new QTreeWidgetItem(name);
			ui.twSubscriptions->addTopLevelItem(newItem);
			name.clear();
			name = sub.split('/', QString::SkipEmptyParts);

			//find the corresponding "root" item in twTopics
			QTreeWidgetItem* topic = nullptr;
			for (int j = 0; j < ui.twTopics->topLevelItemCount(); ++j) {
				if (ui.twTopics->topLevelItem(j)->text(0) == name[0]) {
					topic = ui.twTopics->topLevelItem(j);
					break;
				}
			}

			//restore the children of the subscription
			if (topic != nullptr && topic->childCount() > 0)
				restoreSubscriptionChildren(topic, newItem, name, 1);
		}
	}
	m_searching = false;
}

/*!
 *\brief Adds to a # wildcard containing topic, every topic present in twTopics that the former topic contains
 *
 * \param topic pointer to the TreeWidgetItem which was selected before subscribing
 * \param subscription pointer to the TreeWidgetItem which represents the new subscirption,
 *		  we add all of the children to this item
 */
void MQTTSubscriptionWidget::addSubscriptionChildren(QTreeWidgetItem* topic, QTreeWidgetItem* subscription) {
	//if the topic doesn't have any children we don't do anything
	if (topic->childCount() <= 0)
		return;

	for (int i = 0; i < topic->childCount(); ++i) {
		QTreeWidgetItem* temp = topic->child(i);
		QString name;
		//if it has children, then we add it as a # wildcrad containing topic
		if (topic->child(i)->childCount() > 0) {
			name.append(temp->text(0) + "/#");
			while (temp->parent() != nullptr) {
				temp = temp->parent();
				name.prepend(temp->text(0) + '/');
			}
		}

		//if not then we simply add the topic itself
		else {
			name.append(temp->text(0));
			while (temp->parent() != nullptr) {
				temp = temp->parent();
				name.prepend(temp->text(0) + '/');
			}
		}

		QStringList nameList;
		nameList.append(name);
		auto* childItem = new QTreeWidgetItem(nameList);
		subscription->addChild(childItem);
		//we use the function recursively on the given item
		addSubscriptionChildren(topic->child(i), childItem);
	}
}

/*!
 *\brief Restores the children of a top level item in twSubscriptions if it contains wildcards
 *
 * \param topic pointer to a top level item in twTopics which represents the root of the subscription topic
 * \param subscription pointer to a top level item in twSubscriptions, this is the item whose children will be restored
 * \param list QStringList containing the levels of the subscription topic
 * \param level the level's number which is being investigated
 */
void MQTTSubscriptionWidget::restoreSubscriptionChildren(QTreeWidgetItem * topic, QTreeWidgetItem * subscription, const QStringList& list, int level) {
	if (list[level] != "+" && list[level] != "#" && level < list.size() - 1) {
		for (int i = 0; i < topic->childCount(); ++i) {
			//if the current level isn't + or # wildcard we recursively continue with the next level
			if (topic->child(i)->text(0) == list[level]) {
				restoreSubscriptionChildren(topic->child(i), subscription, list, level + 1);
				break;
			}
		}
	} else if (list[level] == "+") {
		for (int i = 0; i < topic->childCount(); ++i) {
			//determine the name of the topic, contained by the subscription
			QString name;
			name.append(topic->child(i)->text(0));
			for (int j = level + 1; j < list.size(); ++j)
				name.append('/' + list[j]);

			QTreeWidgetItem* temp = topic->child(i);
			while (temp->parent() != nullptr) {
				temp = temp->parent();
				name.prepend(temp->text(0) + '/');
			}

			//Add the topic as child of the subscription
			QStringList nameList;
			nameList.append(name);
			auto* newItem = new QTreeWidgetItem(nameList);
			subscription->addChild(newItem);
			//Continue adding children recursively to the new item
			restoreSubscriptionChildren(topic->child(i), newItem, list, level + 1);
		}
	} else if (list[level] == "#") {
		//add the children of the # wildcard containing subscription
		addSubscriptionChildren(topic, subscription);
	}
}

/*!
 *\brief Returns the amount of topics that the '+' wildcard will replace in the level position
 *
 * \param levelIdx the level currently being investigated
 * \param level the level where the new + wildcard will be placed
 * \param commonList the topic name split into levels
 * \param currentItem pointer to a TreeWidgetItem which represents the parent of the level
 *		  represented by levelIdx
 * \return returns the childCount, or -1 if some topics already represented by + wildcard have different
 *		   amount of children
 */
int MQTTSubscriptionWidget::checkCommonChildCount(int levelIdx, int level, QStringList& commonList, QTreeWidgetItem* currentItem) {
	//we recursively check the number of children, until we get to level-1
	if (levelIdx < level - 1) {
		if (commonList[levelIdx] != "+") {
			for (int j = 0; j < currentItem->childCount(); ++j) {
				if (currentItem->child(j)->text(0) == commonList[levelIdx]) {
					//if the level isn't represented by + wildcard we simply return the amount of children of the corresponding item, recursively
					return checkCommonChildCount(levelIdx + 1, level, commonList, currentItem->child(j));
				}
			}
		} else {
			int childCount = -1;
			bool ok = true;

			//otherwise we check if every + wildcard represented topic has the same number of children, recursively
			for (int j = 0; j < currentItem->childCount(); ++j) {
				int temp = checkCommonChildCount(levelIdx + 1, level, commonList, currentItem->child(j));
				if ((j > 0) && (temp != childCount)) {
					ok = false;
					break;
				}
				childCount = temp;
			}

			//if yes we return this number, otherwise -1
			if (ok)
				return childCount;
			else
				return -1;
		}
	} else if (levelIdx == level - 1) {
		if (commonList[levelIdx] != "+") {
			for (int j = 0; j < currentItem->childCount(); ++j) {
				if (currentItem->child(j)->text(0) == commonList[levelIdx]) {
					//if the level isn't represented by + wildcard we simply return the amount of children of the corresponding item
					return currentItem->child(j)->childCount();
				}
			}
		} else {
			int childCount = -1;
			bool ok = true;

			//otherwise we check if every + wildcard represented topic has the same number of children
			for (int j = 0; j < currentItem->childCount(); ++j) {
				if ((j > 0) && (currentItem->child(j)->childCount() != childCount)) {
					ok = false;
					break;
				}
				childCount = currentItem->child(j)->childCount();
			}

			//if yes we return this number, otherwise -1
			if (ok)
				return childCount;
			else
				return -1;
		}

	} else if (level == 1 && levelIdx == 1)
		return currentItem->childCount();

	return -1;
}


/*!
 *\brief Returns the index of level where the two topic names differ, if there is a common topic for them
 *
 * \param first the name of a topic
 * \param second the name of a topic
 * \return The index of the unequal level, if there is a common topic, otherwise -1
 */
int MQTTSubscriptionWidget::commonLevelIndex(const QString& first, const QString& second) {
	QStringList firstList = first.split('/', QString::SkipEmptyParts);
	QStringList secondtList = second.split('/', QString::SkipEmptyParts);
	QString commonTopic;
	int differIndex = -1;

	if (!firstList.isEmpty()) {
		//the two topics have to be the same size and can't be identic
		if (firstList.size() == secondtList.size() && (first != second))	{

			//the index where they differ
			for (int i = 0; i < firstList.size(); ++i) {
				if (firstList.at(i) != secondtList.at(i)) {
					differIndex = i;
					break;
				}
			}

			//they can differ at only one level
			bool differ = false;
			if (differIndex > 0) {
				for (int j = differIndex + 1; j < firstList.size(); ++j) {
					if (firstList.at(j) != secondtList.at(j)) {
						differ = true;
						break;
					}
				}
			} else
				differ = true;

			if (!differ) {
				for (int i = 0; i < firstList.size(); ++i) {
					if (i != differIndex)
						commonTopic.append(firstList.at(i));
					else
						commonTopic.append('+');

					if (i != firstList.size() - 1)
						commonTopic.append('/');
				}
			}
		}
	}

	//if there is a common topic we return the differIndex
	if (!commonTopic.isEmpty())
		return differIndex;
	else
		return -1;
}

/*!
 *\brief Returns the '+' wildcard containing topic name, which includes the given topic names
 *
 * \param first the name of a topic
 * \param second the name of a topic
 * \return The name of the common topic, if it exists, otherwise ""
 */
QString MQTTSubscriptionWidget::checkCommonLevel(const QString& first, const QString& second) {
	const QStringList& firstList = first.split('/', QString::SkipEmptyParts);
	if (firstList.isEmpty())
		return QString();

	const QStringList& secondtList = second.split('/', QString::SkipEmptyParts);
	QString commonTopic;

	//the two topics have to be the same size and can't be identic
	if (firstList.size() == secondtList.size() && (first != second))	{

		//the index where they differ
		int differIndex = -1;
		for (int i = 0; i < firstList.size(); ++i) {
			if (firstList.at(i) != secondtList.at(i)) {
				differIndex = i;
				break;
			}
		}

		//they can differ at only one level
		bool differ = false;
		if (differIndex > 0) {
			for (int j = differIndex + 1; j < firstList.size(); ++j) {
				if (firstList.at(j) != secondtList.at(j)) {
					differ = true;
					break;
				}
			}
		} else
			differ = true;

		if (!differ) {
			for (int i = 0; i < firstList.size(); ++i) {
				if (i != differIndex)
					commonTopic.append(firstList.at(i));
				else {
					//we put '+' wildcard at the level where they differ
					commonTopic.append('+');
				}

				if (i != firstList.size() - 1)
					commonTopic.append('/');
			}
		}
	}

// 	qDebug() << "Common topic for " << first << " and " << second << " is: " << commonTopic;
	return commonTopic;
}

/************** SLOTS **************************************************************/

/*!
 *\brief When a leaf topic is double clicked in the topics tree widget we subscribe on that
 */
void MQTTSubscriptionWidget::mqttAvailableTopicDoubleClicked(QTreeWidgetItem* item, int column) {
	Q_UNUSED(column)
	// Only for leaf topics
	if (item->childCount() == 0)
		mqttSubscribe();
}

/*!
 *\brief When a leaf subscription is double clicked in the topics tree widget we unsubscribe
 */
void MQTTSubscriptionWidget::mqttSubscribedTopicDoubleClicked(QTreeWidgetItem* item, int column) {
	Q_UNUSED(column)
	// Only for leaf subscriptions
	if (item->childCount() == 0)
		mqttUnsubscribe();
}

/*!
 *\brief called when the subscribe button is pressed
 * subscribes to the topic represented by the current item of twTopics
 */
void MQTTSubscriptionWidget::mqttSubscribe() {
	QTreeWidgetItem* item = ui.twTopics->currentItem();
	if (!item)
		return; //should never happen

	//determine the topic name that the current item represents
	QTreeWidgetItem* tempItem = item;
	QString name = item->text(0);
	if (item->childCount() != 0)
		name.append("/#");

	while (tempItem->parent()) {
		tempItem = tempItem->parent();
		name.prepend(tempItem->text(0) + '/');
	}

	//check if the subscription already exists
	const QList<QTreeWidgetItem*>& topLevelList = ui.twSubscriptions->findItems(name, Qt::MatchExactly);
	if (topLevelList.isEmpty() || topLevelList.first()->parent() != nullptr) {
		QDEBUG("Subscribe to: " << name);
		bool foundSuperior = false;

		for (int i = 0; i < ui.twSubscriptions->topLevelItemCount(); ++i) {
			//if the new subscirptions contains an already existing one, we remove the inferior one
			if (checkTopicContains(name, ui.twSubscriptions->topLevelItem(i)->text(0))
			        && name != ui.twSubscriptions->topLevelItem(i)->text(0)) {
				if (m_parent == MQTTParentWidget::ImportFileWidget)
					unsubscribeFromTopic(ui.twSubscriptions->topLevelItem(i)->text(0));
				else {
					ui.twSubscriptions->topLevelItem(i)->takeChildren();
					ui.twSubscriptions->takeTopLevelItem(i);
				}
				--i;
				continue;
			}

			//if there is a subscription containing the new one we set foundSuperior true
			if (checkTopicContains(ui.twSubscriptions->topLevelItem(i)->text(0), name)
			        && name != ui.twSubscriptions->topLevelItem(i)->text(0)) {
				foundSuperior = true;
				QDEBUG("Can't continue subscribing. Found superior for " << name << " : " << ui.twSubscriptions->topLevelItem(i)->text(0));
				break;
			}
		}

		//if there wasn't a superior subscription we can subscribe to the new topic
		if (!foundSuperior) {
			QStringList toplevelName;
			toplevelName.push_back(name);
			auto* newTopLevelItem = new QTreeWidgetItem(toplevelName);
			ui.twSubscriptions->addTopLevelItem(newTopLevelItem);

			if (name.endsWith('#')) {
				//adding every topic that the subscription contains to twSubscriptions
				addSubscriptionChildren(item, newTopLevelItem);
			}

			emit makeSubscription(name, static_cast<quint8>(ui.cbQos->currentText().toUInt()));

			if (name.endsWith('#')) {
				//if an already existing subscription contains a topic that the new subscription also contains
				//we decompose the already existing subscription
				//by unsubscribing from its topics, that are present in the new subscription as well
				const QStringList nameList = name.split('/', QString::SkipEmptyParts);
				const QString& root = nameList.first();
				QVector<QTreeWidgetItem*> children;
				for (int i = 0; i < ui.twSubscriptions->topLevelItemCount(); ++i) {
					if (ui.twSubscriptions->topLevelItem(i)->text(0).startsWith(root)
					        && name != ui.twSubscriptions->topLevelItem(i)->text(0)) {
						children.clear();
						//get the "leaf" children of the inspected subscription
						findSubscriptionLeafChildren(children, ui.twSubscriptions->topLevelItem(i));
						for (const auto& child : children) {
							if (checkTopicContains(name, child->text(0))) {
								//if the new subscription contains a topic, we unsubscribe from it
								if (m_parent == MQTTParentWidget::ImportFileWidget) {
									ui.twSubscriptions->setCurrentItem(child);
									mqttUnsubscribe();
									--i;
								} else {
									QTreeWidgetItem* unsubscribeItem = child;
									while (unsubscribeItem->parent() != nullptr) {
										for (int i = 0; i < unsubscribeItem->parent()->childCount(); ++i) {
											const QString& childText = unsubscribeItem->parent()->child(i)->text(0);
											if (unsubscribeItem->text(0) != childText) {
												//add topic as subscription
												quint8 qos = static_cast<quint8>(ui.cbQos->currentText().toUInt());
												emit addBeforeRemoveSubscription(childText, qos);
												//also add it to twSubscriptions
												ui.twSubscriptions->addTopLevelItem(unsubscribeItem->parent()->takeChild(i));
												--i;
											} else {
												//before we remove the topic, we reparent it to the new subscription
												//so no data is lost
												emit reparentTopic(unsubscribeItem->text(0), name);
											}
										}
										unsubscribeItem = unsubscribeItem->parent();
									}

									qDebug() << "Remove: " << unsubscribeItem->text(0);
									emit removeMQTTSubscription(unsubscribeItem->text(0));

									ui.twSubscriptions->takeTopLevelItem(ui.twSubscriptions->indexOfTopLevelItem(unsubscribeItem));
								}
							}
						}
					}
				}
			}


			//implementalj es ird at liveDataDock addsubscription!!!!!
			manageCommonLevelSubscriptions();
			updateSubscriptionCompleter();

			emit enableWill(true);
		} else
			QMessageBox::warning(this, i18n("Warning"), i18n("You already subscribed to a topic containing this one"));
	} else
		QMessageBox::warning(this, i18n("Warning"), i18n("You already subscribed to this topic"));
}

/*!
 *\brief Updates the completer for leSubscriptions
 */
void MQTTSubscriptionWidget::updateSubscriptionCompleter() {
	QStringList subscriptionList;
	for (int i = 0; i < ui.twSubscriptions->topLevelItemCount(); ++i)
		subscriptionList.append(ui.twSubscriptions->topLevelItem(i)->text(0));

	if (!subscriptionList.isEmpty()) {
		m_subscriptionCompleter = new QCompleter(subscriptionList, this);
		m_subscriptionCompleter->setCompletionMode(QCompleter::PopupCompletion);
		m_subscriptionCompleter->setCaseSensitivity(Qt::CaseSensitive);
		ui.leSubscriptions->setCompleter(m_subscriptionCompleter);
	} else
		ui.leSubscriptions->setCompleter(nullptr);
}


/*!
 *\brief called when the unsubscribe button is pressed
 * unsubscribes from the topic represented by the current item of twSubscription
 */
void MQTTSubscriptionWidget::mqttUnsubscribe() {
	QTreeWidgetItem* unsubscribeItem = ui.twSubscriptions->currentItem();
	if (!unsubscribeItem)
		return; //should never happen

	QDEBUG("Unsubscribe from: " << unsubscribeItem->text(0));
	//if it is a top level item, meaning a topic that we really subscribed to(not one that belongs to a subscription)
	//we can simply unsubscribe from it
	if (unsubscribeItem->parent() == nullptr) {
		if (m_parent == MQTTParentWidget::ImportFileWidget)
			unsubscribeFromTopic(unsubscribeItem->text(0));
		else {
			emit removeMQTTSubscription(unsubscribeItem->text(0));
			ui.twSubscriptions->takeTopLevelItem(ui.twSubscriptions->indexOfTopLevelItem(unsubscribeItem));
		}
	}

	//otherwise we remove the selected item, but subscribe to every other topic, that was contained by
	//the selected item's parent subscription(top level item of twSubscriptions)
	else {
		while (unsubscribeItem->parent() != nullptr) {
			for (int i = 0; i < unsubscribeItem->parent()->childCount(); ++i) {
				const QString& childText = unsubscribeItem->parent()->child(i)->text(0);
				if (unsubscribeItem->text(0) != childText) {
					quint8 qos = static_cast<quint8>(ui.cbQos->currentText().toUInt());
					if (m_parent == MQTTParentWidget::ImportFileWidget)
						emit makeSubscription(childText, qos);
					else
						emit addBeforeRemoveSubscription(childText, qos);

					ui.twSubscriptions->addTopLevelItem(unsubscribeItem->parent()->takeChild(i));
					--i;
				}
			}
			unsubscribeItem = unsubscribeItem->parent();
		}

		if (m_parent == MQTTParentWidget::ImportFileWidget)
			unsubscribeFromTopic(unsubscribeItem->text(0));
		else {
			emit removeMQTTSubscription(unsubscribeItem->text(0));
			ui.twSubscriptions->takeTopLevelItem(ui.twSubscriptions->indexOfTopLevelItem(unsubscribeItem));
		}

		//check if any common topics were subscribed, if possible merge them
		manageCommonLevelSubscriptions();
	}
	updateSubscriptionCompleter();

	if (ui.twSubscriptions->topLevelItemCount() <= 0)
		emit enableWill(false);
}

/*!
 *\brief called when a new topic is added to the tree(twTopics)
 * appends the topic's root to the topicList if it isn't in the list already
 * then sets the completer for leTopics
 */
void MQTTSubscriptionWidget::setTopicCompleter(const QString& topic) {
	if (!m_searching) {
		const QStringList& list = topic.split('/', QString::SkipEmptyParts);
		QString tempTopic;
		if (!list.isEmpty())
			tempTopic = list.at(0);
		else
			tempTopic = topic;

		if (!m_topicList.contains(tempTopic)) {
			m_topicList.append(tempTopic);
			m_topicCompleter = new QCompleter(m_topicList, this);
			m_topicCompleter->setCompletionMode(QCompleter::PopupCompletion);
			m_topicCompleter->setCaseSensitivity(Qt::CaseSensitive);
			ui.leTopics->setCompleter(m_topicCompleter);
		}
	}
}

/*!
 *\brief called when leTopics' text is changed
 *		 if the rootName can be found in twTopics, then we scroll it to the top of the tree widget
 *
 * \param rootName the current text of leTopics
 */
void MQTTSubscriptionWidget::scrollToTopicTreeItem(const QString& rootName) {
	m_searching = true;
	m_searchTimer->start();

	int topItemIdx = -1;
	for (int i = 0; i < ui.twTopics->topLevelItemCount(); ++i)
		if (ui.twTopics->topLevelItem(i)->text(0) == rootName) {
			topItemIdx = i;
			break;
		}

	if (topItemIdx >= 0)
		ui.twTopics->scrollToItem(ui.twTopics->topLevelItem(topItemIdx),
		                          QAbstractItemView::ScrollHint::PositionAtTop);
}

/*!
 *\brief called when leSubscriptions' text is changed
 *		 if the rootName can be found in twSubscriptions, then we scroll it to the top of the tree widget
 *
 * \param rootName the current text of leSubscriptions
 */
void MQTTSubscriptionWidget::scrollToSubsriptionTreeItem(const QString& rootName) {
	int topItemIdx = -1;
	for (int i = 0; i < ui.twSubscriptions->topLevelItemCount(); ++i)
		if (ui.twSubscriptions->topLevelItem(i)->text(0) == rootName) {
			topItemIdx = i;
			break;
		}

	if (topItemIdx >= 0)
		ui.twSubscriptions->scrollToItem(ui.twSubscriptions->topLevelItem(topItemIdx),
										QAbstractItemView::ScrollHint::PositionAtTop);
}

/*!
 *\brief called when 10 seconds passed since the last time the user searched for a certain root in twTopics
 * enables updating the completer for le
 */
void MQTTSubscriptionWidget::topicTimeout() {
	m_searching = false;
	m_searchTimer->stop();
}

void MQTTSubscriptionWidget::clearWidgets() {
	ui.twTopics->clear();
	ui.twSubscriptions->clear();
	ui.twTopics->headerItem()->setText(0, i18n("Available"));
}

void MQTTSubscriptionWidget::onDisconnect() {
	m_searchTimer->stop();
	m_searching = false;
	delete m_topicCompleter;
	delete m_subscriptionCompleter;
}
