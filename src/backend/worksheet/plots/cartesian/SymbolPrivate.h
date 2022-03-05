/*
    File                 : SymbolPrivate.h
    Project              : LabPlot
    Description          : Private members of Symbol
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2021 Alexander Semke <alexander.semke@web.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef SYMBOLPRIVATE_H
#define SYMBOLPRIVATE_H

#include <QBrush>
#include <QPen>

class SymbolPrivate {
public:
	explicit SymbolPrivate(Symbol*);

	QString name() const;
	void updateSymbols();
	void updatePixmap();

	Symbol::Style style{Symbol::Style::NoSymbols};
	QBrush brush;
	QPen pen;
	qreal opacity{1.0};
	qreal rotationAngle{0.0};
	qreal size{1.0};

	Symbol* const q{nullptr};
};

#endif
