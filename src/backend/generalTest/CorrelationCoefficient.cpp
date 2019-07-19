/***************************************************************************
    File                 : CorrelationCoefficient.cpp
    Project              : LabPlot
    Description          : Finding Correlation Coefficient on data provided
    --------------------------------------------------------------------
    Copyright            : (C) 2019 Devanshu Agarwal(agarwaldevanshu8@gmail.com)

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

#include "CorrelationCoefficient.h"
#include "GeneralTest.h"
#include "kdefrontend/generalTest/CorrelationCoefficientView.h"
#include "backend/spreadsheet/Spreadsheet.h"
#include "backend/core/column/Column.h"
#include "backend/lib/macros.h"

#include <QVector>
#include <QStandardItemModel>
#include <QLocale>
#include <QLabel>
#include <QVBoxLayout>
#include <QWidget>
#include <QtMath>
#include <QQueue>

#include <KLocalizedString>

#include <gsl/gsl_cdf.h>
#include <gsl/gsl_math.h>

extern "C" {
#include "backend/nsl/nsl_stats.h"
}

CorrelationCoefficient::CorrelationCoefficient(const QString &name) : GeneralTest (name, AspectType::CorrelationCoefficient) {
}

CorrelationCoefficient::~CorrelationCoefficient() {
}

void CorrelationCoefficient::performTest(Test test, bool categoricalVariable) {
    m_statsTable = "";
    m_tooltips.clear();
    for (int i = 0; i < 10; i++)
        m_resultLine[i]->clear();

    switch (test) {
    case CorrelationCoefficient::Test::Pearson: {
        m_currTestName = "<h2>" + i18n("Pearson's r Correlation Test") + "</h2>";
        performPearson(categoricalVariable);
        break;
    }
    case CorrelationCoefficient::Test::Kendall:
        m_currTestName = "<h2>" + i18n("Kendall's Correlation Test") + "</h2>";
        performKendall();
        break;
    case CorrelationCoefficient::Test::Spearman: {
        m_currTestName = "<h2>" + i18n("Spearman Correlation Test") + "</h2>";
        performSpearman();
        break;
    }
    }

    emit changed();
}


double CorrelationCoefficient::correlationValue() {
    return m_correlationValue;
}


/***************************************************************************************************************************
 *                                        Private Implementations
 * ************************************************************************************************************************/

/*********************************************Pearson r ******************************************************************/
void CorrelationCoefficient::performPearson(bool categoricalVariable) {
    Q_UNUSED(categoricalVariable);

}

/***********************************************Kendall ******************************************************************/
void CorrelationCoefficient::performKendall() {

}

/***********************************************Spearman ******************************************************************/
void CorrelationCoefficient::performSpearman() {

}

// Virtual functions
QWidget* CorrelationCoefficient::view() const {
    if (!m_partView) {
        m_view = new CorrelationCoefficientView(const_cast<CorrelationCoefficient*>(this));
        m_partView = m_view;
    }
    return m_partView;
}
