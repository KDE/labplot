#include "backend/generalTest/MyTextEdit.h"
#include "backend/lib/macros.h"

#include <QTextEdit>
#include <QHelpEvent>
#include <QToolTip>

MyTextEdit::MyTextEdit(QWidget* parent) : QTextEdit(parent) {
	m_avlIntervalTree.insert(19, 23, "Cold");
	m_avlIntervalTree.insert(243, 254, "Interaction");
	m_avlIntervalTree.insert(187, 196, "Detergent");
	m_avlIntervalTree.insert(163, 176, "result table");
	m_avlIntervalTree.insert(33, 37, "Mean");
}

bool MyTextEdit::event(QEvent *e) {
	if (e->type() == QEvent::ToolTip) {
		QHelpEvent *helpEvent = static_cast<QHelpEvent *>(e);
//		QDEBUG("pos is " << helpEvent->pos());
		QTextCursor textCursor = this->cursorForPosition(helpEvent->pos());
//		QDEBUG("text cursor1 for event is " << textCursor.position());

		QString tooltip = m_avlIntervalTree.toolTip(textCursor.position());
//		QDEBUG("tooltip is " << tooltip);
		if (!tooltip.isEmpty())
			QToolTip::showText(helpEvent->globalPos(), tooltip);
		else
			QToolTip::hideText();
		//		int index = itemAt(helpEvent->pos());
		//		if (index != -1) {
		//			QToolTip::showText(helpEvent->globalPos(), shapeItems[index].toolTip());
		//		} else {
		//			QToolTip::hideText();
		//			e->ignore();
		//		}
		return true;
	}
	return 	inherited::event(e);
	//	return QWidget::event(e);
}

//int MyTextEdit::itemAt(const QPoint &pos) {
////	for (int i = shapeItems.size() - 1; i >= 0; --i) {
////		const ShapeItem &item = shapeItems[i];
////		if (item.path().contains(pos - item.position()))
////			return i;
////	}
//	return -1;
//}
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


