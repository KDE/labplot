/*
	File                 : Value.h
	Project              : LabPlot
	Description          : Value
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2022-2023 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef VALUE_H
#define VALUE_H

#include "backend/core/AbstractAspect.h"
#include "backend/lib/macros.h"

class AbstractColumn;
class ValuePrivate;
class KConfigGroup;

class Value : public AbstractAspect {
	Q_OBJECT

public:
	enum Type { NoValues, BinEntries, CustomColumn };
	enum Position { Above, Under, Left, Right, Center };

	explicit Value(const QString& name);
	~Value() override;

	void init(const KConfigGroup&);
	void draw(QPainter*, const QVector<QPointF>&, const QVector<QString>&);

	void save(QXmlStreamWriter*) const override;
	bool load(XmlStreamReader*, bool preview) override;
	void loadThemeConfig(const KConfigGroup&, const QColor&);
	void saveThemeConfig(KConfigGroup&) const;

	BASIC_D_ACCESSOR_DECL(Type, type, Type)
	POINTER_D_ACCESSOR_DECL(const AbstractColumn, column, Column)
	QString& columnPath() const;
	void setColumnPath(const QString&);
	BASIC_D_ACCESSOR_DECL(Position, position, Position)
	BASIC_D_ACCESSOR_DECL(bool, centerPositionAvailable, centerPositionAvailable)
	BASIC_D_ACCESSOR_DECL(double, distance, Distance)
	BASIC_D_ACCESSOR_DECL(double, rotationAngle, RotationAngle)
	BASIC_D_ACCESSOR_DECL(double, opacity, Opacity)
	BASIC_D_ACCESSOR_DECL(char, numericFormat, NumericFormat)
	BASIC_D_ACCESSOR_DECL(int, precision, Precision)
	CLASS_D_ACCESSOR_DECL(QString, dateTimeFormat, DateTimeFormat)
	CLASS_D_ACCESSOR_DECL(QString, prefix, Prefix)
	CLASS_D_ACCESSOR_DECL(QString, suffix, Suffix)
	CLASS_D_ACCESSOR_DECL(QColor, color, Color)
	CLASS_D_ACCESSOR_DECL(QFont, font, Font)

	typedef ValuePrivate Private;

protected:
	ValuePrivate* const d_ptr;

private:
	Q_DECLARE_PRIVATE(Value)

private Q_SLOTS:
	void columnAboutToBeRemoved(const AbstractAspect*);

Q_SIGNALS:
	void typeChanged(Value::Type);
	void columnChanged(const AbstractColumn*);
	void positionChanged(Value::Position);
	void distanceChanged(double);
	void rotationAngleChanged(double);
	void opacityChanged(double);
	void numericFormatChanged(char);
	void precisionChanged(int);
	void dateTimeFormatChanged(QString);
	void prefixChanged(QString);
	void suffixChanged(QString);
	void fontChanged(QFont);
	void colorChanged(QColor);

	void updateRequested();
	void updatePixmapRequested();
};

#endif
