/***************************************************************************
    File                 : ImageWidget.h
    Project              : LabPlot
    Description          : widget for datapicker properties
    --------------------------------------------------------------------
    Copyright            : (C) 2015 by Ankit Wagadre (wagadre.ankit@gmail.com)
    Copyright            : (C) 2015 Alexander Semke (alexander.semke@web.de)

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


#ifndef DATAPICKERCURVEWIDGET_H
#define DATAPICKERCURVEWIDGET_H

#include "ui_datapickercurvewidget.h"
#include "backend/datapicker/DatapickerCurve.h"


class DatapickerCurveWidget : public QWidget {
	Q_OBJECT

public:
	explicit DatapickerCurveWidget(QWidget*);
	~DatapickerCurveWidget() override;

	void setCurves(QList<DatapickerCurve*>);
	void load();

private:
	Ui::DatapickerCurveWidget ui;
	void init();
	void initConnections();
	void hideErrorBarWidgets(bool);

	DatapickerCurve* m_curve{nullptr};
	QList<DatapickerCurve*> m_curveList;
	bool m_initializing;
	bool m_suppressTypeChange{false};

private slots:
	//SLOTs for changes triggered in DatapickerCurveDock
	void nameChanged();
	void commentChanged();
	void updateSymbolWidgets();
	void xErrorTypeChanged(int);
	void yErrorTypeChanged(int);
	void styleChanged(int);
	void sizeChanged(double);
	void rotationChanged(int);
	void opacityChanged(int);
	void fillingStyleChanged(int);
	void fillingColorChanged(const QColor&);
	void borderStyleChanged(int);
	void borderColorChanged(const QColor&);
	void borderWidthChanged(double);
	void visibilityChanged(bool);
	void errorBarFillingStyleChanged(int);
	void errorBarFillingColorChanged(const QColor&);
	void errorBarSizeChanged(double);

	//SLOTs for changes triggered in DatapickerCurve
	void curveDescriptionChanged(const AbstractAspect*);
	void curveErrorsChanged(DatapickerCurve::Errors);
	void symbolStyleChanged(Symbol::Style);
	void symbolSizeChanged(qreal);
	void symbolRotationAngleChanged(qreal);
	void symbolOpacityChanged(qreal);
	void symbolBrushChanged(const QBrush&);
	void symbolPenChanged(const QPen&);
	void symbolVisibleChanged(bool);
	void symbolErrorBarSizeChanged(qreal);
	void symbolErrorBarBrushChanged(const QBrush&);
};

#endif // DATAPICKERCURVEWIDGET_H
