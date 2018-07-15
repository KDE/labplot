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

#include "backend/worksheet/plots/cartesian/Histogram.h"
#include "ui_histogramdock.h"

class Histogram;
class TreeViewComboBox;
class AspectTreeModel;
class Column;

class HistogramDock : public QWidget {
	Q_OBJECT

public:
	explicit HistogramDock(QWidget*);
	~HistogramDock();

	void setCurves(QList<Histogram*>);

private:
	QStringList dateStrings;
	QStringList timeStrings;

	TreeViewComboBox* cbDataColumn;
	TreeViewComboBox* cbValuesColumn;

	void updateValuesFormatWidgets(const AbstractColumn::ColumnMode);
	void showValuesColumnFormat(const Column*);

	void load();
	void loadConfig(KConfig&);

protected:
	Ui::HistogramDock ui;
	QList<Histogram*> m_curvesList;
	Histogram* m_curve;
	AspectTreeModel* m_aspectTreeModel;
	bool m_initializing;

	virtual void setModel();
	void setModelIndexFromColumn(TreeViewComboBox*, const AbstractColumn*);

private slots:
	void init();
	void retranslateUi();

	//SLOTs for changes triggered in HistogramDock

	//General-Tab
	void nameChanged();
	void commentChanged();
	void dataColumnChanged(const QModelIndex&);
	void visibilityChanged(bool);
	void typeChanged(int);
	void orientationChanged(int);
	void binningMethodChanged(int);
	void binCountChanged(int);
	void binWidthChanged();

	//Lines-Tab
	void lineTypeChanged(int);
	void lineStyleChanged(int);
	void lineColorChanged(const QColor&);
	void lineWidthChanged(double);
	void lineOpacityChanged(int);

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
	void fillingEnabledChanged(int);
  	void fillingTypeChanged(int);
	void fillingColorStyleChanged(int);
	void fillingImageStyleChanged(int);
	void fillingBrushStyleChanged(int);
	void fillingFirstColorChanged(const QColor&);
	void fillingSecondColorChanged(const QColor&);
	void selectFile();
	void fileNameChanged();
	void fillingOpacityChanged(int);

	//SLOTs for changes triggered in Histogram
	//General-Tab
	void curveDescriptionChanged(const AbstractAspect*);
	void curveDataColumnChanged(const AbstractColumn*);
	void curveTypeChanged(Histogram::HistogramType);
	void curveOrientationChanged(Histogram::HistogramOrientation);
	void curveBinningMethodChanged(Histogram::BinningMethod);
	void curveBinCountChanged(int);
	void curveBinWidthChanged(float);
	void curveVisibilityChanged(bool);

	//Line-tab
	void curveLineTypeChanged(Histogram::LineType);
	void curveLinePenChanged(const QPen&);
	void curveLineOpacityChanged(qreal);

	//Values-Tab
	void curveValuesTypeChanged(Histogram::ValuesType);
	void curveValuesColumnChanged(const AbstractColumn*);
	void curveValuesPositionChanged(Histogram::ValuesPosition);
	void curveValuesDistanceChanged(qreal);
	void curveValuesOpacityChanged(qreal);
	void curveValuesRotationAngleChanged(qreal);
	void curveValuesPrefixChanged(const QString&);
	void curveValuesSuffixChanged(const QString&);
	void curveValuesFontChanged(QFont);
	void curveValuesColorChanged(QColor);

	//Filling-Tab
	void curveFillingEnabledChanged(bool);
	void curveFillingTypeChanged(PlotArea::BackgroundType);
	void curveFillingColorStyleChanged(PlotArea::BackgroundColorStyle);
	void curveFillingImageStyleChanged(PlotArea::BackgroundImageStyle);
	void curveFillingBrushStyleChanged(Qt::BrushStyle);
	void curveFillingFirstColorChanged(QColor&);
	void curveFillingSecondColorChanged(QColor&);
	void curveFillingFileNameChanged(QString&);
	void curveFillingOpacityChanged(float);

	//load and save
	void loadConfigFromTemplate(KConfig&);
	void saveConfigAsTemplate(KConfig&);

signals:
	void info(const QString&);
};

#endif
