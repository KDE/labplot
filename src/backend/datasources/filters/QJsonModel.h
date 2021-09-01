/*
    The MIT License (MIT)

    SPDX-FileCopyrightText: 2011 SCHUTZ Sacha

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in all
    copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
    SOFTWARE.
*/

#ifndef QJSONMODEL_H
#define QJSONMODEL_H

#include <QAbstractItemModel>
#include <QJsonDocument>
#include <QJsonValue>
#include <QJsonArray>
#include <QJsonObject>
#include <QIcon>

class QJsonModel;
class QJsonItem;

class QJsonTreeItem {
public:
	explicit QJsonTreeItem(QJsonTreeItem* parent = nullptr);
	~QJsonTreeItem();
	void appendChild(QJsonTreeItem*);
	void reserve(int);
	QJsonTreeItem* child(int row);
	QJsonTreeItem* parent();
	int childCount() const;
	int row() const;
	void setKey(const QString&);
	void setValue(const QString&);
	void setType(const QJsonValue::Type);
	void setSize(int);
	const QString& key() const;
	const QString& value() const;
	QJsonValue::Type type() const;
	int size() const;

	static QJsonTreeItem* load(const QJsonValue&, QJsonTreeItem* parent = nullptr);

private:
	QString mKey;
	QString mValue;
	QJsonValue::Type mType{QJsonValue::Undefined};
	int mSize{0};
	QList<QJsonTreeItem*> mChildren;
	QJsonTreeItem* mParent;
};


class QJsonModel : public QAbstractItemModel {
	Q_OBJECT

public:
	explicit QJsonModel(QObject* parent = nullptr);
	~QJsonModel() override;
	void clear();
	bool load(const QString& fileName);
	bool load(QIODevice*);
	bool loadJson(const QByteArray&);
	bool loadJson(const QJsonDocument&);
	QVariant data(const QModelIndex&, int role) const override;
	bool setData(const QModelIndex&, const QVariant& value, int role = Qt::EditRole) override;
	QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
	QModelIndex index(int row, int column,const QModelIndex &parent = QModelIndex()) const override;
	QModelIndex parent(const QModelIndex&) const override;
	int rowCount(const QModelIndex& parent = QModelIndex()) const override;
	int columnCount(const QModelIndex& parent = QModelIndex()) const override;
	Qt::ItemFlags flags(const QModelIndex&) const override;

// 	QJsonDocument json() const;
	QJsonDocument genJsonByIndex(const QModelIndex&) const;

private:
	QJsonValue genJson(QJsonTreeItem*) const;

	QJsonTreeItem* mHeadItem;
	QJsonTreeItem* mRootItem;
	QStringList mHeaders;
	QIcon mObjectIcon;
	QIcon mArrayIcon;
};

#endif // QJSONMODEL_H
