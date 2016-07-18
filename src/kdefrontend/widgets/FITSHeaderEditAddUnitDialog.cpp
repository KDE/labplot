/***************************************************************************
File                 : FITSHeaderEditAddUnitDialog.cpp
Project              : LabPlot
Description          : Widget for adding or modifying FITS header keyword units
--------------------------------------------------------------------
Copyright            : (C) 2016 by Fabian Kristof (fkristofszabolcs@gmail.com)
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
#include "FITSHeaderEditAddUnitDialog.h"
#include "backend/datasources/filters/FITSFilter.h"

FITSHeaderEditAddUnitDialog::FITSHeaderEditAddUnitDialog(const QString& unit, QWidget *parent) :
    KDialog(parent) {
    QWidget* mainWidget = new QWidget(this);
    ui.setupUi(mainWidget);
    setMainWidget( mainWidget );

    setWindowTitle(i18n("Specify the new unit"));
    setButtons( KDialog::Ok | KDialog::Cancel );
    setButtonText(KDialog::Ok, i18n("&Add unit"));
    KCompletion* keyCompletion = new KCompletion;
    keyCompletion->setItems(FITSFilter::units());
    if (!unit.isEmpty()) {
        ui.kleUnit->setText(unit);
    }
    ui.kleUnit->setCompletionObject(keyCompletion);
    ui.kleUnit->setAutoDeleteCompletionObject(true);
    connect(this, SIGNAL(okClicked()), this, SLOT(addUnit()));
}

FITSHeaderEditAddUnitDialog::~FITSHeaderEditAddUnitDialog() {
}

QString FITSHeaderEditAddUnitDialog::unit() const {
    return m_unit;
}

void FITSHeaderEditAddUnitDialog::addUnit() {
    if (ui.kleUnit->text().contains(QLatin1Char('('))) {
        m_unit = ui.kleUnit->text().left(ui.kleUnit->text().indexOf(QLatin1Char('('))-1);
    } else {
        m_unit = ui.kleUnit->text();
    }
}
