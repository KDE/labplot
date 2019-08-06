/***************************************************************************
    File                 : MyTextEdit.h
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

    void extractToolTips(QString& text, bool insert = true);
protected slots:
    bool event(QEvent *e);

private:
    AvlIntervalTree m_avlIntervalTree{};
};

#endif // MYTEXTEDIT_H
