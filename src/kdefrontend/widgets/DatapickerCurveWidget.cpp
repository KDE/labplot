/***************************************************************************
    File                 : ImageWidget.cpp
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

#include "DatapickerCurveWidget.h"
#include "backend/datapicker/DatapickerPoint.h"
#include "kdefrontend/widgets/DatapickerPointWidget.h"

#include <QHBoxLayout>

DatapickerCurveWidget::DatapickerCurveWidget(QWidget *parent) : QWidget(parent), m_suppressTypeChange(false) {
    ui.setupUi(this);

    QHBoxLayout* hboxLayout = new QHBoxLayout(ui.tSymbols);
    datapickerPointWidget = new DatapickerPointWidget(ui.tSymbols);
    hboxLayout->addWidget(datapickerPointWidget);
    datapickerPointWidget->hidePositionWidgets();

    ui.cbXErrorType->addItem(i18n("No Error"));
    ui.cbXErrorType->addItem(i18n("symmetric"));
    ui.cbXErrorType->addItem(i18n("asymmetric"));

    ui.cbYErrorType->addItem(i18n("No Error"));
    ui.cbYErrorType->addItem(i18n("symmetric"));
    ui.cbYErrorType->addItem(i18n("asymmetric"));

	connect( ui.leName, SIGNAL(returnPressed()), this, SLOT(nameChanged()) );
	connect( ui.leComment, SIGNAL(returnPressed()), this, SLOT(commentChanged()) );
    connect( ui.cbXErrorType, SIGNAL(currentIndexChanged(int)), this, SLOT(xErrorTypeChanged(int)) );
    connect( ui.cbYErrorType, SIGNAL(currentIndexChanged(int)), this, SLOT(yErrorTypeChanged(int)) );
}

DatapickerCurveWidget::~DatapickerCurveWidget(){
}

void DatapickerCurveWidget::setCurves(QList<DatapickerCurve*> list) {
    if (list.isEmpty())
        return;

    m_curveList = list;
    m_curve = list.first();

	if (list.size()==1){
		ui.lName->setEnabled(true);
		ui.leName->setEnabled(true);
		ui.lComment->setEnabled(true);
		ui.leComment->setEnabled(true);
		ui.leName->setText(m_curve->name());
		ui.leComment->setText(m_curve->comment());
	}else{
		ui.lName->setEnabled(false);
		ui.leName->setEnabled(false);
		ui.lComment->setEnabled(false);
		ui.leComment->setEnabled(false);
		ui.leName->setText("");
		ui.leComment->setText("");
	}

    load();
    initConnections();
    updateDatapickerPointList();
}

void DatapickerCurveWidget::initConnections() {
	connect( m_curve, SIGNAL(aspectDescriptionChanged(const AbstractAspect*)),this, SLOT(curveDescriptionChanged(const AbstractAspect*)));
    connect( m_curve, SIGNAL(aspectRemoved(const AbstractAspect*,const AbstractAspect*,const AbstractAspect*)),
             this, SLOT(updateDatapickerPointList()) );
    connect( m_curve, SIGNAL(aspectAdded(const AbstractAspect*)), this, SLOT(updateDatapickerPointList()) );
    connect( m_curve, SIGNAL(curveErrorTypesChanged(DatapickerCurve::Errors)), this, SLOT(curveErrorsChanged(DatapickerCurve::Errors)) );
}

//*************************************************************
//**** SLOTs for changes triggered in DatapickerCurveWidget ***
//*************************************************************
//"General"-tab
void DatapickerCurveWidget::nameChanged(){
    if (m_initializing)
        return;

    m_curve->setName(ui.leName->text());
}

void DatapickerCurveWidget::commentChanged(){
    if (m_initializing)
        return;

    m_curve->setComment(ui.leComment->text());
}

void DatapickerCurveWidget::xErrorTypeChanged(int index) {
    if (m_initializing || m_suppressTypeChange)
        return;

    DatapickerCurve::Errors errors = m_curve->curveErrorTypes();
    errors.x = DatapickerCurve::ErrorType(index);

    foreach(DatapickerCurve* curve, m_curveList)
        curve->setCurveErrorTypes(errors);
}

void DatapickerCurveWidget::yErrorTypeChanged(int index) {
    if (m_initializing || m_suppressTypeChange)
        return;

    DatapickerCurve::Errors errors = m_curve->curveErrorTypes();
    errors.y = DatapickerCurve::ErrorType(index);

    foreach(DatapickerCurve* curve, m_curveList)
        curve->setCurveErrorTypes(errors);
}

void DatapickerCurveWidget::updateDatapickerPointList() {
    QList<DatapickerPoint*> pointsList = m_curve->children<DatapickerPoint>(AbstractAspect::IncludeHidden);
    datapickerPointWidget->setDatapickerPoints(pointsList);
    if (pointsList.isEmpty()) {
        ui.cbXErrorType->setEnabled(true);
        ui.cbYErrorType->setEnabled(true);
        m_suppressTypeChange = false;
    } else {
        ui.cbXErrorType->setEnabled(false);
        ui.cbYErrorType->setEnabled(false);
        m_suppressTypeChange = true;
    }
}

//*************************************************************
//******** SLOTs for changes triggered in DatapickerCurve *****
//*************************************************************
void DatapickerCurveWidget::curveDescriptionChanged(const AbstractAspect* aspect) {
	if (m_curve != aspect)
		return;

	m_initializing = true;
	if (aspect->name() != ui.leName->text()) {
		ui.leName->setText(aspect->name());
	} else if (aspect->comment() != ui.leComment->text()) {
		ui.leComment->setText(aspect->comment());
	}
	m_initializing = false;
}

void DatapickerCurveWidget::curveErrorsChanged(DatapickerCurve::Errors errors){
    m_initializing = true;
    ui.cbXErrorType->setCurrentIndex((int) errors.x);
    ui.cbYErrorType->setCurrentIndex((int) errors.y);
    m_initializing = false;
}

//**********************************************************
//******************** SETTINGS ****************************
//**********************************************************
void DatapickerCurveWidget::load() {
    if(m_curve == NULL)
        return;

    m_initializing = true;
    ui.cbXErrorType->setCurrentIndex((int) m_curve->curveErrorTypes().x);
    ui.cbYErrorType->setCurrentIndex((int) m_curve->curveErrorTypes().y);
    m_initializing = false;
}
