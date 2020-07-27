/***************************************************************************
    File             : XYFourierTransformCurveDock.h
    Project          : LabPlot
    --------------------------------------------------------------------
    Copyright        : (C) 2016 Stefan Gerlach (stefan.gerlach@uni.kn)
    Description      : widget for editing properties of Fourier transform curves

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

#ifndef XYFOURIERTRANSFORMCURVEDOCK_H
#define XYFOURIERTRANSFORMCURVEDOCK_H

#include "kdefrontend/dockwidgets/XYCurveDock.h"
#include "backend/worksheet/plots/cartesian/XYFourierTransformCurve.h"
#include "ui_xyfouriertransformcurvedockgeneraltab.h"

class TreeViewComboBox;

class XYFourierTransformCurveDock: public XYCurveDock {
	Q_OBJECT

public:
	explicit XYFourierTransformCurveDock(QWidget *parent);
	void setCurves(QList<XYCurve*>);
	void setupGeneral() override;

private:
	void initGeneralTab() override;
	void showTransformResult();

	Ui::XYFourierTransformCurveDockGeneralTab uiGeneralTab;
	TreeViewComboBox* cbXDataColumn{nullptr};
	TreeViewComboBox* cbYDataColumn{nullptr};

	XYFourierTransformCurve* m_transformCurve{nullptr};
	XYFourierTransformCurve::TransformData m_transformData;

protected:
	void setModel() override;

private slots:
	//SLOTs for changes triggered in XYFourierTransformCurveDock
	//general tab
	void xDataColumnChanged(const QModelIndex&);
	void yDataColumnChanged(const QModelIndex&);
	void autoRangeChanged();
	void xRangeMinChanged();
	void xRangeMaxChanged();
	void windowTypeChanged();
	void typeChanged();
	void twoSidedChanged();
	void shiftedChanged();
	void xScaleChanged();

//	void showOptions();
	void recalculateClicked();

	void enableRecalculate() const;

	//SLOTs for changes triggered in XYCurve
	//General-Tab
	void curveDescriptionChanged(const AbstractAspect*);
	void curveXDataColumnChanged(const AbstractColumn*);
	void curveYDataColumnChanged(const AbstractColumn*);
	void curveTransformDataChanged(const XYFourierTransformCurve::TransformData&);
	void dataChanged();
	void curveVisibilityChanged(bool);
};

#endif
