/***************************************************************************
    File                 : HistogramDock.h
    Project              : LabPlot
    Description          : widget for histogram plot properties
    --------------------------------------------------------------------
    Copyright            : (C) 2016 Anu Mittal (anu22mittal@gmail.com)

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

#ifndef HISTOGRAMDOCK_H
#define HISTOGRAMDOCK_H

#include "backend/worksheet/Worksheet.h"
#include "backend/worksheet/plots/cartesian/Histogram.h"
#include "ui_histogramdock.h"
#include "ui_histogramdockgeneraltab.h"
#include <QList>
#include <KConfig>
#include <KLocalizedString>

class Histogram;
class TreeViewComboBox;
class AspectTreeModel;
class Column;
class KUrlCompletion;

class HistogramDock : public QWidget {
    //Q_OBJECT
  
public:
    explicit HistogramDock(QWidget*);
    ~HistogramDock();
	
	void setCurves(QList<Histogram*>);
	virtual void setupGeneral();
private:
	Ui::HistogramDockGeneralTab uiGeneralTab;
	KUrlCompletion* m_completion;
	QStringList dateStrings;
	QStringList timeStrings;

	TreeViewComboBox* cbXColumn;
	TreeViewComboBox* cbYColumn;
	TreeViewComboBox* cbValuesColumn;
	TreeViewComboBox* cbXErrorPlusColumn;
	TreeViewComboBox* cbXErrorMinusColumn;
	TreeViewComboBox* cbYErrorPlusColumn;
	TreeViewComboBox* cbYErrorMinusColumn;

	virtual void initGeneralTab();
	void updateValuesFormatWidgets(const AbstractColumn::ColumnMode);
	void showValuesColumnFormat(const Column*);

	void load();
	void loadConfig(KConfig&);

protected:
	bool m_initializing;
	Ui::HistogramDock ui;
	QList<Histogram*> m_curvesList;
	Histogram* m_curve;
	AspectTreeModel* m_aspectTreeModel;

	void initTabs();
	virtual void setModel();
	void setModelIndexFromColumn(TreeViewComboBox*, const AbstractColumn*);

private slots:
	void init();
	void retranslateUi();

	//SLOTs for changes triggered in HistogramDock
	void nameChanged();
	void commentChanged();
	void xColumnChanged(const QModelIndex&);
	void yColumnChanged(const QModelIndex&);
	void visibilityChanged(bool);

	//Line-Tab
	void lineTypeChanged(int);
	void lineSkipGapsChanged(bool);
	void lineInterpolationPointsCountChanged(int);
  	void lineStyleChanged(int);
	void lineColorChanged(const QColor&);
	void lineWidthChanged(double);
	void lineOpacityChanged(int);

	void dropLineTypeChanged(int);
  	void dropLineStyleChanged(int);
	void dropLineColorChanged(const QColor&);
	void dropLineWidthChanged(double);
	void dropLineOpacityChanged(int);

	//Symbol-tab
  	void symbolsStyleChanged(int);
	void symbolsSizeChanged(double);
	void symbolsRotationChanged(int);
	void symbolsOpacityChanged(int);
	void symbolsFillingStyleChanged(int);
	void symbolsFillingColorChanged(const QColor&);
	void symbolsBorderStyleChanged(int);
	void symbolsBorderColorChanged(const QColor&);
	void symbolsBorderWidthChanged(double);

	//Values-Tab
	void valuesTypeChanged(int);
	void valuesColumnChanged(const QModelIndex&);
	void valuesPositionChanged(int);
	void valuesDistanceChanged(double);
	void valuesRotationChanged(int);
	void valuesOpacityChanged(int);
	void valuesPrefixChanged();
	void valuesSuffixChanged();
	void valuesFontChanged(const QFont&);
	void valuesColorChanged(const QColor&);

	//Filling-tab
	void fillingPositionChanged(int);
  	void fillingTypeChanged(int);
	void fillingColorStyleChanged(int);
	void fillingImageStyleChanged(int);
	void fillingBrushStyleChanged(int);
	void fillingFirstColorChanged(const QColor&);
	void fillingSecondColorChanged(const QColor&);
	void selectFile();
	void fileNameChanged();
	void fillingOpacityChanged(int);

	//"Error bars"-Tab
	void xErrorTypeChanged(int) const;
	void yErrorTypeChanged(int) const;
	void xErrorPlusColumnChanged(const QModelIndex&) const;
	void xErrorMinusColumnChanged(const QModelIndex&) const;
	void yErrorPlusColumnChanged(const QModelIndex&) const;
	void yErrorMinusColumnChanged(const QModelIndex&) const;
	void errorBarsTypeChanged(int) const;
	void errorBarsCapSizeChanged(double) const;
  	void errorBarsStyleChanged(int) const;
	void errorBarsColorChanged(const QColor&);
	void errorBarsWidthChanged(double) const;
	void errorBarsOpacityChanged(int) const;

	//SLOTs for changes triggered in Histogram
	//General-Tab
	void curveDescriptionChanged(const AbstractAspect*);
	void curveXColumnChanged(const AbstractColumn*);
	void curveYColumnChanged(const AbstractColumn*);
	void curveVisibilityChanged(bool);

	//Line-Tab
	void curveLineTypeChanged(Histogram::LineType);
	void curveLineSkipGapsChanged(bool);
	void curveLineInterpolationPointsCountChanged(int);
	void curveLinePenChanged(const QPen&);
	void curveLineOpacityChanged(qreal);
	void curveDropLineTypeChanged(Histogram::DropLineType);
	void curveDropLinePenChanged(const QPen&);
	void curveDropLineOpacityChanged(qreal);

	//Symbol-Tab
	void curveSymbolsStyleChanged(Symbol::Style);
	void curveSymbolsSizeChanged(qreal);
	void curveSymbolsRotationAngleChanged(qreal);
	void curveSymbolsOpacityChanged(qreal);
	void curveSymbolsBrushChanged(QBrush);
	void curveSymbolsPenChanged(const QPen&);

	//Values-Tab
	void curveValuesTypeChanged(Histogram::ValuesType);
	void curveValuesColumnChanged(const AbstractColumn*);
	void curveValuesPositionChanged(Histogram::ValuesPosition);
	void curveValuesDistanceChanged(qreal);
	void curveValuesOpacityChanged(qreal);
	void curveValuesRotationAngleChanged(qreal);
	void curveValuesPrefixChanged(QString);
	void curveValuesSuffixChanged(QString);
	void curveValuesFontChanged(QFont);
	void curveValuesColorChanged(QColor);

	//Filling-Tab
	void curveFillingPositionChanged(Histogram::FillingPosition);
	void curveFillingTypeChanged(PlotArea::BackgroundType);
	void curveFillingColorStyleChanged(PlotArea::BackgroundColorStyle);
	void curveFillingImageStyleChanged(PlotArea::BackgroundImageStyle);
	void curveFillingBrushStyleChanged(Qt::BrushStyle);
	void curveFillingFirstColorChanged(QColor&);
	void curveFillingSecondColorChanged(QColor&);
	void curveFillingFileNameChanged(QString&);
	void curveFillingOpacityChanged(float);

	//"Error bars"-Tab
	void curveXErrorTypeChanged(Histogram::ErrorType);
	void curveXErrorPlusColumnChanged(const AbstractColumn*);
	void curveXErrorMinusColumnChanged(const AbstractColumn*);
	void curveYErrorTypeChanged(Histogram::ErrorType);
	void curveYErrorPlusColumnChanged(const AbstractColumn*);
	void curveYErrorMinusColumnChanged(const AbstractColumn*);
	void curveErrorBarsCapSizeChanged(qreal);
	void curveErrorBarsTypeChanged(Histogram::ErrorBarsType);
	void curveErrorBarsPenChanged(QPen);
	void curveErrorBarsOpacityChanged(qreal);

	//load and save
	void loadConfigFromTemplate(KConfig&);
	void saveConfigAsTemplate(KConfig&);

signals:
	void info(const QString&);
};

#endif
