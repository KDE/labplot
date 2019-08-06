#ifndef MYTEXTEDIT_H
#define MYTEXTEDIT_H

#include <QTextEdit>

struct Node {
    int low;
    int high;
    Node *left;
    Node *right;
    int height;
    QString tooltip = "";
};

class AvlIntervalTree {
public:
    AvlIntervalTree();
    ~AvlIntervalTree();

    void insert(const int &low, const int &high, const QString &tooltip);
    QString toolTip(const int &key);

private:
    Node* head = nullptr;
    int height(Node *root);
    Node* newNode(const int &l, const int &h);
    Node* leftRotate(Node *x);
    Node* rightRotate(Node *y);
    int getBalance(Node *root);
    Node* insertHelper(Node *node, const int &low, const int &high);

    QString m_currTooltip = "";
};

class MyTextEdit : public QTextEdit {
    Q_OBJECT

public:
    typedef QTextEdit inherited;
    explicit MyTextEdit(QWidget* parent = nullptr);
    //    int itemAt(const QPoint &pos);
    void setHtml(QString text);

protected slots:
    bool event(QEvent *e);

private:
    AvlIntervalTree m_avlIntervalTree{};
};

#endif // MYTEXTEDIT_H
