/*
    SPDX-FileCopyrightText: 2011 SCHUTZ Sacha
    SPDX-FileCopyrightText: 2020 Alexander Semke <alexander.semke@web.de>

    SPDX-License-Identifier: MIT
*/

#include "QJsonModel.h"
#include "backend/lib/trace.h"

#include <QFile>
#include <QMessageBox>
#include <QPainter>

#include <KLocalizedString>

QJsonTreeItem::QJsonTreeItem(QJsonTreeItem* parent) : mParent(parent) {}

QJsonTreeItem::~QJsonTreeItem() {
	qDeleteAll(mChildren);
}

void QJsonTreeItem::appendChild(QJsonTreeItem* item) {
	mChildren.append(item);
}

void QJsonTreeItem::reserve(int size) {
	mChildren.reserve(size);
}

QJsonTreeItem* QJsonTreeItem::child(int row) {
	return mChildren.value(row);
}

QJsonTreeItem* QJsonTreeItem::parent() {
	return mParent;
}

int QJsonTreeItem::childCount() const {
	return mChildren.count();
}

int QJsonTreeItem::row() const {
	if (mParent)
		return mParent->mChildren.indexOf(const_cast<QJsonTreeItem*>(this));

	return 0;
}

void QJsonTreeItem::setKey(const QString& key) {
	mKey = key;
}

void QJsonTreeItem::setValue(const QString& value) {
	mValue = value;
}

void QJsonTreeItem::setType(const QJsonValue::Type type) {
	mType = type;
}

void QJsonTreeItem::setSize(int size) {
	mSize = size;
}

const QString& QJsonTreeItem::key() const {
	return mKey;
}

const QString& QJsonTreeItem::value() const {
	return mValue;
}

QJsonValue::Type QJsonTreeItem::type() const {
	return mType;
}

int QJsonTreeItem::size() const {
	return mSize;
}

QJsonTreeItem* QJsonTreeItem::load(const QJsonValue& value, QJsonTreeItem* parent) {
	auto* rootItem = new QJsonTreeItem(parent);
// 	rootItem->setKey("root");

	if (value.isObject()) {
		const auto& object = value.toObject();

		//determine the size
		rootItem->setSize(QJsonDocument(object).toJson(QJsonDocument::Compact).size());

		//read all children
		for (const QString& key : object.keys()) {
			const QJsonValue& v = object.value(key);
			QJsonTreeItem* child = load(v,rootItem);
			child->setKey(key);
			child->setType(v.type());
			rootItem->appendChild(child);
		}
	} else if (value.isArray()) {
		const auto& array = value.toArray();
		rootItem->setSize(QJsonDocument(array).toJson(QJsonDocument::Compact).size());

		int index = 0;
		rootItem->reserve(array.count());
		for (const QJsonValue& v : array) {
			QJsonTreeItem* child = load(v, rootItem);
			child->setKey(QString::number(index));
			child->setType(v.type());
			rootItem->appendChild(child);
			++index;
		}
	} else {
		const QString& str = value.toVariant().toString();
		rootItem->setValue(str);
		rootItem->setType(value.type());
		rootItem->setSize(str.length());
	}

	return rootItem;
}

//=========================================================================

QJsonModel::QJsonModel(QObject* parent) : QAbstractItemModel(parent),
	mHeadItem(new QJsonTreeItem),
	mRootItem(new QJsonTreeItem(mHeadItem)) {

	mHeadItem->appendChild(mRootItem);
	mHeaders.append(i18n("Key"));
	mHeaders.append(i18n("Value"));
	mHeaders.append(i18n("Size in Bytes"));

	//icons
	QPainter painter;
	QPixmap pix(64, 64);

	QFont font;
	font.setPixelSize(60);

	const QColor& color = qApp->palette().color(QPalette::Text);
	painter.setPen(QPen(color));

	//draw the icon for JSON array
	pix.fill(QColor(Qt::transparent));
	painter.begin(&pix);
	painter.setFont(font);
	painter.drawText(pix.rect(), Qt::AlignCenter, QLatin1String("[ ]"));
	painter.end();
	mArrayIcon = QIcon(pix);

	//draw the icon for JSON object
	pix.fill(QColor(Qt::transparent));
	painter.begin(&pix);
	painter.setFont(font);
	painter.drawText(pix.rect(), Qt::AlignCenter, QLatin1String("{ }"));
	painter.end();
	mObjectIcon = QIcon(pix);
}

QJsonModel::~QJsonModel() {
	//delete mRootItem;
	delete mHeadItem;
}

void QJsonModel::clear() {
	beginResetModel();
	delete mHeadItem;
	mHeadItem = new QJsonTreeItem;
	mRootItem = new QJsonTreeItem(mHeadItem);
	mHeadItem->appendChild(mRootItem);
	endResetModel();
}

bool QJsonModel::load(const QString& fileName) {
	QFile file(fileName);
	bool success = false;
	if (file.open(QIODevice::ReadOnly)) {
		success = load(&file);
		file.close();
	} else
		success = false;

	return success;
}

bool QJsonModel::load(QIODevice* device) {
	return loadJson(device->readAll());
}

bool QJsonModel::loadJson(const QByteArray& json) {
	QJsonParseError error;

	const QJsonDocument& doc = QJsonDocument::fromJson(json, &error);
	if (error.error == QJsonParseError::NoError)
		return loadJson(doc);
	else {
		QMessageBox::critical(nullptr, i18n("Failed to load JSON document"),
							  i18n("Failed to load JSON document. Error: %1.", error.errorString()));
		return false;
	}

}

bool QJsonModel::loadJson(const QJsonDocument& jdoc) {
	PERFTRACE("load json document into the model");
	if (!jdoc.isNull()) {
		beginResetModel();
		delete mHeadItem;

		mHeadItem = new QJsonTreeItem;

		if (jdoc.isArray()) {
			{
			PERFTRACE("load json tree items");
			mRootItem = QJsonTreeItem::load(QJsonValue(jdoc.array()), mHeadItem);
			}
			mRootItem->setType(QJsonValue::Array);

		} else {
			mRootItem = QJsonTreeItem::load(QJsonValue(jdoc.object()), mHeadItem);
			mRootItem->setType(QJsonValue::Object);
		}

		mHeadItem->appendChild(mRootItem);

		endResetModel();
		return true;
	}

	return false;
}

QVariant QJsonModel::data(const QModelIndex& index, int role) const {
	if (!index.isValid())
		return QVariant();

	auto* item = static_cast<QJsonTreeItem*>(index.internalPointer());

	if (role == Qt::DisplayRole) {
		if (index.column() == 0)
			return item->key();
		else if (index.column() == 1) {
			//in case the value is very long, cut it so the preview tree tree view doesnt' explode
			if (item->value().length() > 200)
				return item->value().left(200) + QLatin1String(" ...");
			else
				return item->value();
		} else {
			if (item->size() != 0)
				return QString::number(item->size());
			else
				return QString();
		}
	} else if (Qt::EditRole == role) {
		if (index.column() == 1)
			return item->value();
	} else if (role == Qt::DecorationRole) {
		if (index.column() == 0) {
			if (item->type() == QJsonValue::Array)
				return mArrayIcon;
			else if (item->type() == QJsonValue::Object)
				return mObjectIcon;
		}
	}

	return QVariant();
}

bool QJsonModel::setData(const QModelIndex& index, const QVariant& value, int role) {
	if (Qt::EditRole == role) {
		if (index.column() == 1) {
			auto* item = static_cast<QJsonTreeItem*>(index.internalPointer());
			item->setValue(value.toString());
			emit dataChanged(index, index, {Qt::EditRole});
			return true;
		}
	}

	return false;
}

QVariant QJsonModel::headerData(int section, Qt::Orientation orientation, int role) const {
	if (role == Qt::DisplayRole && orientation == Qt::Horizontal)
		return mHeaders.value(section);

	return QVariant();
}

QModelIndex QJsonModel::index(int row, int column, const QModelIndex& parent) const {
	if (!hasIndex(row, column, parent))
		return QModelIndex{};

	QJsonTreeItem* parentItem;

	if (!parent.isValid())
		parentItem = mHeadItem;
	else
		parentItem = static_cast<QJsonTreeItem*>(parent.internalPointer());

	QJsonTreeItem* childItem = parentItem->child(row);
	if (childItem)
		return createIndex(row, column, childItem);

	return QModelIndex{};
}

QModelIndex QJsonModel::parent(const QModelIndex& index) const {
	if (!index.isValid())
		return QModelIndex{};

	auto* childItem = static_cast<QJsonTreeItem*>(index.internalPointer());
	QJsonTreeItem* parentItem = childItem->parent();

	if (parentItem == mHeadItem)
		return QModelIndex{};

	return createIndex(parentItem->row(), 0, parentItem);
}

int QJsonModel::rowCount(const QModelIndex& parent) const {
	QJsonTreeItem* parentItem;
	if (parent.column() > 0)
		return 0;

	if (!parent.isValid())
		parentItem = mHeadItem;
	else
		parentItem = static_cast<QJsonTreeItem*>(parent.internalPointer());

	return parentItem->childCount();
}

int QJsonModel::columnCount(const QModelIndex& /*parent*/) const {
	return 3;
}

Qt::ItemFlags QJsonModel::flags(const QModelIndex& index) const {
	if (index.column() == 1)
		return Qt::ItemIsEditable | QAbstractItemModel::flags(index);
	else
		return QAbstractItemModel::flags(index);
}
/*
QJsonDocument QJsonModel::json() const {
	auto v = genJson(mRootItem);
	QJsonDocument doc;

	if (v.isObject())
		doc = QJsonDocument(v.toObject());
	else
		doc = QJsonDocument(v.toArray());

	return doc;
}
*/
QJsonValue QJsonModel::genJson(QJsonTreeItem* item) const {
	auto type = item->type();
	const int nchild = item->childCount();

	if (QJsonValue::Object == type) {
		QJsonObject jo;
		for (int i = 0; i < nchild; ++i) {
			auto ch = item->child(i);
			auto key = ch->key();
			jo.insert(key, genJson(ch));
		}
		return  jo;
	} else if (QJsonValue::Array == type) {
		QJsonArray arr;
		for (int i = 0; i < nchild; ++i) {
			auto ch = item->child(i);
			arr.append(genJson(ch));
		}
		return arr;
	} else {
		QJsonValue va(item->value());
		return va;
	}

}

QJsonDocument QJsonModel::genJsonByIndex(const QModelIndex& index) const {
	if (!index.isValid())
		return QJsonDocument();

	auto* item = static_cast<QJsonTreeItem*>(index.internalPointer());
	return QJsonDocument::fromVariant(genJson(item).toVariant());
}
