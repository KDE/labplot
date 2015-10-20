/***************************************************************************
    File                 : ImageWidget.cpp
    Project              : LabPlot
    Description          : widget for datapicker properties
    --------------------------------------------------------------------
    Copyright            : (C) 2015 by Ankit Wagadre (wagadre.ankit@gmail.com)

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
#include "backend/datapicker/CustomItem.h"
#include "kdefrontend/widgets/CustomItemWidget.h"

#include <QGridLayout>

DatapickerCurveWidget::DatapickerCurveWidget(QWidget *parent) : QWidget(parent), m_suppressTypeChange(false) {
    ui.setupUi(this);

    QHBoxLayout* hboxLayout = new QHBoxLayout(ui.tSymbols);
    customItemWidget = new CustomItemWidget(ui.tSymbols);
    hboxLayout->addWidget(customItemWidget);
    customItemWidget->hidePositionWidgets();

    ui.cbXErrorType->addItem(i18n("No Error"));
    ui.cbXErrorType->addItem(i18n("symmetric"));
    ui.cbXErrorType->addItem(i18n("asymmetric"));

    ui.cbYErrorType->addItem(i18n("No Error"));
    ui.cbYErrorType->addItem(i18n("symmetric"));
    ui.cbYErrorType->addItem(i18n("asymmetric"));

    connect( ui.cbXErrorType, SIGNAL(currentIndexChanged(int)), this, SLOT(xErrorTypeChanged(int)) );
    connect( ui.cbYErrorType, SIGNAL(currentIndexChanged(int)), this, SLOT(yErrorTypeChanged(int)) );
}

DatapickerCurveWidget::~DatapickerCurveWidget(){
}

void DatapickerCurveWidget::setCurves(QList<DataPickerCurve*> list) {
    if (list.isEmpty())
        return;

    m_curveList = list;
    m_curve = list.first();
    load();
    initConnections();
    updateCustomItemList();
}

void DatapickerCurveWidget::initConnections() {
    connect( m_curve, SIGNAL(aspectRemoved(const AbstractAspect*,const AbstractAspect*,const AbstractAspect*)),
             this, SLOT(updateCustomItemList()) );
    connect( m_curve, SIGNAL(aspectAdded(const AbstractAspect*)), this, SLOT(updateCustomItemList()) );
    connect( m_curve, SIGNAL(curveErrorTypesChanged(DataPickerCurve::Errors)), this, SLOT(curveErrorsChanged(DataPickerCurve::Errors)) );
}

void DatapickerCurveWidget::xErrorTypeChanged(int index) {
    if (m_initializing || m_suppressTypeChange)
        return;

    DataPickerCurve::Errors errors = m_curve->curveErrorTypes();
    errors.x = DataPickerCurve::ErrorType(index);

    foreach(DataPickerCurve* curve, m_curveList)
        curve->setCurveErrorTypes(errors);
}

void DatapickerCurveWidget::yErrorTypeChanged(int index) {
    if (m_initializing || m_suppressTypeChange)
        return;

    DataPickerCurve::Errors errors = m_curve->curveErrorTypes();
    errors.y = DataPickerCurve::ErrorType(index);

    foreach(DataPickerCurve* curve, m_curveList)
        curve->setCurveErrorTypes(errors);
}

void DatapickerCurveWidget::curveErrorsChanged(DataPickerCurve::Errors errors){
    m_initializing = true;
    ui.cbXErrorType->setCurrentIndex((int) errors.x);
    ui.cbYErrorType->setCurrentIndex((int) errors.y);
    m_initializing = false;
}

void DatapickerCurveWidget::updateCustomItemList() {
    QList<CustomItem*> itemsList = m_curve->children<CustomItem>(AbstractAspect::IncludeHidden);
    customItemWidget->setCustomItems(itemsList);
    if (itemsList.isEmpty()) {
        ui.cbXErrorType->setEnabled(true);
        ui.cbYErrorType->setEnabled(true);
        m_suppressTypeChange = false;
    } else {
        ui.cbXErrorType->setEnabled(false);
        ui.cbYErrorType->setEnabled(false);
        m_suppressTypeChange = true;
    }
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
