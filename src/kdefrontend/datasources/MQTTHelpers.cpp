/***************************************************************************
    File                 : ImportFileWidget.cpp
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

#include "MQTTHelpers.h"

#include <QTreeWidgetItem>
#include <QStringList>

/*!
 *\brief Adds to a # wildcard containing topic, every topic present in twTopics that the former topic contains
 *
 * \param topic pointer to the TreeWidgetItem which was selected before subscribing
 * \param subscription pointer to the TreeWidgetItem which represents the new subscirption,
 *		  we add all of the children to this item
 */
void MQTTHelpers::addSubscriptionChildren(QTreeWidgetItem* topic, QTreeWidgetItem* subscription) {
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
		QTreeWidgetItem* childItem = new QTreeWidgetItem(nameList);
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
void MQTTHelpers::restoreSubscriptionChildren(QTreeWidgetItem * topic, QTreeWidgetItem * subscription, const QStringList& list, int level) {
	if (list[level] != '+' && list[level] != "#" && level < list.size() - 1) {
		for (int i = 0; i < topic->childCount(); ++i) {
			//if the current level isn't + or # wildcard we recursively continue with the next level
			if (topic->child(i)->text(0) == list[level]) {
				restoreSubscriptionChildren(topic->child(i), subscription, list, level + 1);
				break;
			}
		}
	} else if (list[level] == '+') {
		for (int i = 0; i < topic->childCount(); ++i) {
			//determine the name of the topic, contained by the subscription
			QString name;
			name.append(topic->child(i)->text(0));
			for (int j = level + 1; j < list.size(); ++j) {
				name.append('/' + list[j]);
			}
			QTreeWidgetItem* temp = topic->child(i);
			while (temp->parent() != nullptr) {
				temp = temp->parent();
				name.prepend(temp->text(0) + '/');
			}

			//Add the topic as child of the subscription
			QStringList nameList;
			nameList.append(name);
			QTreeWidgetItem* newItem = new QTreeWidgetItem(nameList);
			subscription->addChild(newItem);
			//Continue adding children recursively to the new item
			restoreSubscriptionChildren(topic->child(i), newItem, list, level + 1);
		}
	} else if (list[level] == "#") {
		//add the children of the # wildcard containing subscription
		MQTTHelpers::addSubscriptionChildren(topic, subscription);
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
int MQTTHelpers::checkCommonChildCount(int levelIdx, int level, QStringList& commonList, QTreeWidgetItem* currentItem) {
	//we recursively check the number of children, until we get to level-1
	if (levelIdx < level - 1) {
		if (commonList[levelIdx] != '+') {
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
		if (commonList[levelIdx] != '+') {
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
int MQTTHelpers::commonLevelIndex(const QString& first, const QString& second) {
	QStringList firstList = first.split('/', QString::SkipEmptyParts);
	QStringList secondtList = second.split('/', QString::SkipEmptyParts);
	QString commonTopic = "";
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
			}
			else
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
QString MQTTHelpers::checkCommonLevel(const QString& first, const QString& second) {
	const QStringList& firstList = first.split('/', QString::SkipEmptyParts);
	if (firstList.isEmpty())
		return QString();

	const QStringList& secondtList = second.split('/', QString::SkipEmptyParts);
	QString commonTopic = "";

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
				if (i != differIndex) {
					commonTopic.append(firstList.at(i));
				} else {
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


/*!
 *\brief Fills the children vector, with the root item's (twSubscriptions) leaf children (meaning no wildcard containing topics)
 *
 * \param children vector of TreeWidgetItem pointers
 * \param root pointer to a TreeWidgetItem of twSubscriptions
 */
void MQTTHelpers::findSubscriptionLeafChildren(QVector<QTreeWidgetItem *>& children, QTreeWidgetItem* root) {
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
bool MQTTHelpers::checkTopicContains(const QString& superior, const QString& inferior) {
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
			if ((superiorList.at(i) != '+') &&
					!(superiorList.at(i) == "#" && i == superiorList.size() - 1)) {
				//if the two topics differ, and the superior's current level isn't + or #(which can be only in the last position)
				//then superior can't contain inferior
				ok = false;
				break;
			} else if (i == superiorList.size() - 1 && (superiorList.at(i) == '+' && inferiorList.at(i) == "#") ) {
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
