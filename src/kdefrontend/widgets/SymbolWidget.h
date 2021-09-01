/*
    File                 : SymbolWidget.h
    Project              : LabPlot
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2021 Alexander Semke (alexander.semke@web.de)
    Description          : symbol settings widget
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef SYMBOLWIDGET_H
#define SYMBOLWIDGET_H

#include "ui_symbolwidget.h"
#include "backend/worksheet/plots/cartesian/Symbol.h"
#include <KConfigGroup>

class SymbolWidget : public QWidget {
	Q_OBJECT

public:
	explicit SymbolWidget(QWidget*);

	void setSymbols(QList<Symbol*>);
	void updateLocale();

	void load();
	void loadConfig(const KConfigGroup&);
	void saveConfig(KConfigGroup&) const;

private:
	Ui::SymbolWidget ui;
	Symbol* m_symbol{nullptr};
	QList<Symbol*> m_symbols;
	bool m_initializing{false};

signals:
	void dataChanged(bool);

private slots:
	//SLOTs for changes triggered in SymbolWidget
	void styleChanged(int);
	void sizeChanged(double);
	void rotationChanged(int);
	void opacityChanged(int);
	void fillingStyleChanged(int);
	void fillingColorChanged(const QColor&);
	void borderStyleChanged(int);
	void borderColorChanged(const QColor&);
	void borderWidthChanged(double);

	//SLOTs for changes triggered in Symbol
	void symbolStyleChanged(Symbol::Style);
	void symbolSizeChanged(qreal);
	void symbolRotationAngleChanged(qreal);
	void symbolOpacityChanged(qreal);
	void symbolBrushChanged(const QBrush&);
	void symbolPenChanged(const QPen&);
};

#endif //LABELWIDGET_H
