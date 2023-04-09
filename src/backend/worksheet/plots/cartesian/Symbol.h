/*
	File                 : Symbol.h
	Project              : LabPlot
	Description          : Symbol
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2015-2020 Alexander Semke <alexander.semke@web.de>
	SPDX-FileCopyrightText: 2021 Stefan Gerlach <stefan.gerlach@uni.kn>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef SYMBOL_H
#define SYMBOL_H

#include "backend/core/AbstractAspect.h"
#include "backend/lib/macros.h"
#include <QPainterPath>

class SymbolPrivate;
class KConfigGroup;
class QPainter;

class Symbol : public AbstractAspect {
	Q_OBJECT

public:
	// order is fixed; append new symbols at the end and add to StyleOrder in Symbol.cpp
	enum class Style {
		NoSymbols,
		Circle,
		Square,
		EquilateralTriangle,
		RightTriangle,
		Bar,
		PeakedBar,
		SkewedBar,
		Diamond,
		Lozenge,
		Tie,
		TinyTie,
		Plus,
		Boomerang,
		SmallBoomerang,
		Star4,
		Star5,
		Line,
		Cross,
		Heart,
		Lightning,
		X,
		Asterisk,
		Tri,
		XPlus,
		TallPlus,
		LatinCross,
		DotPlus,
		Hash,
		SquareX,
		SquarePlus,
		SquareHalf,
		SquareDiag,
		SquareTriangle,
		CircleHalf,
		CircleDot,
		CircleX,
		CircleTri,
		Peace,
		Flower,
		Flower2,
		Flower3,
		Flower5,
		Flower6,
		Star,
		Star3,
		Star6,
		Pentagon,
		Hexagon,
		Latin,
		David,
		Home,
		Pin,
		Female,
		Male,
		Spade,
		Club,
		SquareDot,
		TriangleDot,
		TriangleHalf,
		TriangleLine
	};
	Q_ENUM(Style)

	static int stylesCount();
	static QString styleName(Symbol::Style);
	static Symbol::Style indexToStyle(int);
	static QPainterPath stylePath(Symbol::Style);

	explicit Symbol(const QString& name);
	~Symbol() override;
	void init(const KConfigGroup&);

	void draw(QPainter*, QPointF);
	void draw(QPainter*, const QVector<QPointF>&);

	void save(QXmlStreamWriter*) const override;
	bool load(XmlStreamReader*, bool preview) override;
	void loadThemeConfig(const KConfigGroup&, const QColor&);
	void saveThemeConfig(const KConfigGroup&) const;

	BASIC_D_ACCESSOR_DECL(Symbol::Style, style, Style)
	BASIC_D_ACCESSOR_DECL(qreal, opacity, Opacity)
	BASIC_D_ACCESSOR_DECL(qreal, rotationAngle, RotationAngle)
	BASIC_D_ACCESSOR_DECL(qreal, size, Size)
	CLASS_D_ACCESSOR_DECL(QBrush, brush, Brush)
	CLASS_D_ACCESSOR_DECL(QPen, pen, Pen)

	typedef SymbolPrivate Private;

protected:
	SymbolPrivate* const d_ptr;

private:
	Q_DECLARE_PRIVATE(Symbol)

Q_SIGNALS:
	void styleChanged(Symbol::Style);
	void sizeChanged(qreal);
	void rotationAngleChanged(qreal);
	void opacityChanged(qreal);
	void brushChanged(QBrush);
	void penChanged(const QPen&);

	void updateRequested();
	void updatePixmapRequested();
};

#endif
