/***************************************************************************
    File                 : SymbolWidget.h
    Project              : LabPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2021 Alexander Semke (alexander.semke@web.de)
    Description          : symbol settings widget

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
	void load();
	void loadConfig(KConfigGroup&);
	void saveConfig(KConfigGroup&);

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
