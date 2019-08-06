/***************************************************************************
	File                 : MyTextEdit.cpp
	Project              : LabPlot
	Description          : Derived class of QTextEdit to add ToolTip Functionality
	--------------------------------------------------------------------
	Copyright            : (C) 2019 Devanshu Agarwal(agarwaldevanshu8@gmail.com)

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

#include "backend/generalTest/MyTextEdit.h"
#include "backend/lib/macros.h"

#include <QTextEdit>
#include <QHelpEvent>
#include <QToolTip>

MyTextEdit::MyTextEdit(QWidget* parent) : QTextEdit(parent) {
}

bool MyTextEdit::event(QEvent *e) {
	if (e->type() == QEvent::ToolTip || e->type() == QEvent::WhatsThis) {
		QHelpEvent *helpEvent = static_cast<QHelpEvent *>(e);
		QTextCursor textCursor = this->cursorForPosition(helpEvent->pos());
		QTextCursor origCursor = textCursor;

		QString tooltip = m_avlIntervalTree.toolTip(textCursor.position());

		this->setTextCursor(origCursor);

//		QDEBUG("cursor pos is " << textCursor.position());
//		QDEBUG("tooltip is " << tooltip);
		if (!tooltip.isEmpty())
			QToolTip::showText(helpEvent->globalPos(), tooltip);
		else
			QToolTip::hideText();
		return true;
	}
	return 	inherited::event(e);
}

void MyTextEdit::setHtml(QString text) {
	inherited::setHtml(text);
	QString plainText = this->toPlainText();
	extractToolTips(plainText, true);
	extractToolTips(text, false);
	return inherited::setHtml(text);
}

void MyTextEdit::extractToolTips(QString &text, bool insert) {
	QString startToolTip = "[tooltip]";
	QString endToolTip = "[/tooltip]";
	QString startData = "[data]";
	QString endData = "[/data]";
	QString startTip = "[tip]";
	QString endTip = "[/tip]";

	int i_startToolTip = 0;
	int i_endToolTip = 0;
	int i_startData = 0;
	int i_endData = 0;
	int i_startTip = 0;
	int i_endTip = 0;

	QString tip;
	QString data;
	while ((i_startToolTip = text.indexOf(startToolTip, i_startToolTip)) != -1) {
		i_endToolTip = text.indexOf(endToolTip, i_startToolTip);

		if (i_endToolTip != -1) {
			i_startData = text.indexOf(startData, i_startToolTip);
			i_endData = text.indexOf(endData, i_startToolTip);

			if (i_startData != -1 && i_endData != -1 &&
					i_startData < i_endToolTip && i_endData < i_endToolTip)
				data = text.mid(i_startData + startData.size(), i_endData - i_startData - startData.size());
			else {
				data = "";
				i_endData = i_startToolTip + startToolTip.size();
			}

			i_startTip = text.indexOf(startTip, i_endData);
			i_endTip = text.indexOf(endTip, i_endData);

			if (i_startTip != -1 && i_endTip != -1 &&
					i_startTip < i_endToolTip && i_endTip < i_endToolTip)
				tip = text.mid(i_startTip + startTip.size(), i_endTip - i_startTip - startTip.size());
			else
				tip = "";
			text.replace(i_startToolTip, i_endToolTip - i_startToolTip + endToolTip.size(), data);

			if (insert) {
//				QDEBUG("data is " << data);
//				QDEBUG("low is " << i_startToolTip);
//				QDEBUG("high is " << i_startToolTip + data.size());
				m_avlIntervalTree.insert(i_startToolTip, i_startToolTip + data.size(), tip);
			}
			i_startToolTip += data.size();
		}
	}
}

/*********************************** Implementation of AvlIntervalTree *******************************************/


AvlIntervalTree::AvlIntervalTree() {
}

AvlIntervalTree::~AvlIntervalTree() {
}

void AvlIntervalTree::insert(const int &low, const int &high, const QString &tooltip) {
	m_currTooltip = tooltip;
	Node* node = insertHelper(head, low, high);

	if (head == nullptr)
		head = node;
	return;
}

QString AvlIntervalTree::toolTip(const int &key) {
	Node* node = head;
	while (node != nullptr) {
		if (key < node->low)
			node = node->left;
		else if (key > node->high)
			node = node->right;
		else
			return node->tooltip;
	}
	return QString();
}

int AvlIntervalTree::height(struct Node* root) {
	if (root == nullptr)
		return 0;
	return root->height;
}

struct Node* AvlIntervalTree::newNode(const int &l, const int &h) {
	struct Node* node = new struct Node();
	node->low = l;
	node->high = h;
	node->left = nullptr;
	node->right = nullptr;
	node->height = 1;

	node->tooltip = m_currTooltip;
	return(node);
}

struct Node* AvlIntervalTree::rightRotate(struct Node* y) {
	struct Node* x = y->left;
	struct Node* T2 = x->right;

	x->right = y;
	y->left = T2;

	y->height = std::max(height(y->left),
						 height(y->right)) + 1;
	x->height = std::max(height(x->left),
						 height(x->right)) + 1;

	if (head == y)
		head = x;
	return x;
}

struct Node* AvlIntervalTree::leftRotate(struct Node* x) {
	struct Node* y = x->right;
	struct Node* T2 = y->left;

	y->left = x;
	x->right = T2;

	x->height = std::max(height(x->left),
						 height(x->right)) + 1;
	y->height = std::max(height(y->left),
						 height(y->right)) + 1;

	if (head == x)
		head = y;
	return y;
}

int AvlIntervalTree::getBalance(struct Node* root) {
	if (root == nullptr)
		return 0;
	return height(root->left) - height(root->right);
}

struct Node* AvlIntervalTree::insertHelper(struct Node* node, const int &low, const int &high) {
	if (node == nullptr)
		return newNode(low, high);

	if (low < node->low)
		node->left = insertHelper(node->left, low, high);
	else if (low > node->low)
		node->right = insertHelper(node->right, low, high);
	else
		return node;

	node->height = 1 + std::max(height(node->left),
								height(node->right));

	int balance = getBalance(node);

	if (balance > 1 && low < node->left->low)
		return rightRotate(node);

	if (balance < -1 && low > node->right->low)
		return leftRotate(node);

	if (balance > 1 && low > node->left->low) {
		node->left = leftRotate(node->left);
		return rightRotate(node);
	}

	if (balance < -1 && low < node->right->low) {
		node->right = rightRotate(node->right);
		return leftRotate(node);
	}

	return node;
}


