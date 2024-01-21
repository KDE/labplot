/*
	SPDX-FileCopyrightText: 2011 SCHUTZ Sacha
	SPDX-FileCopyrightText: 2020-2023 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: MIT
*/

#ifndef QJSONMODEL_H
#define QJSONMODEL_H

#include <QAbstractItemModel>
#include <QIcon>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>

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
	QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;
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

Q_SIGNALS:
	void error(const QString&);
};

#endif // QJSONMODEL_H
