/***************************************************************************
    File                 : CorrelationCoefficientView.h
    Project              : LabPlot
    Description          : View class for Correlation Coefficient'
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

#ifndef CORRELATIONCOEFFICIENTVIEW_H
#define CORRELATIONCOEFFICIENTVIEW_H

#include "GeneralTestView.h"
#include "backend/core/AbstractColumn.h"
#include "backend/lib/IntervalAttribute.h"

class CorrelationCoefficient;

class CorrelationCoefficientView : public GeneralTestView {
	Q_OBJECT

public:
	explicit CorrelationCoefficientView(CorrelationCoefficient*);
	~CorrelationCoefficientView() override;

private:
	CorrelationCoefficient* m_CorrelationCoefficient;

public slots:
private slots:
};

#endif  // CORRELATIONCOEFFICIENTVIEW_H
