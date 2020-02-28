/***************************************************************************
    File                 : filter.h
    Project              : AbstractColumn
    --------------------------------------------------------------------
    Copyright            : (C) 2017-2020 Stefan Gerlach (stefan.gerlach@uni.kn)
    Description          : Conversion filter header.
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
#ifndef FILTER_H
#define FILTER_H

#include "backend/core/datatypes/SimpleCopyThroughFilter.h"
#include "backend/core/datatypes/String2DoubleFilter.h"
#include "backend/core/datatypes/Double2StringFilter.h"
#include "backend/core/datatypes/Double2IntegerFilter.h"
#include "backend/core/datatypes/Double2DateTimeFilter.h"
#include "backend/core/datatypes/Double2MonthFilter.h"
#include "backend/core/datatypes/Double2DayOfWeekFilter.h"
#include "backend/core/datatypes/Integer2DoubleFilter.h"
#include "backend/core/datatypes/Integer2BigIntFilter.h"
#include "backend/core/datatypes/Integer2StringFilter.h"
#include "backend/core/datatypes/Integer2DateTimeFilter.h"
#include "backend/core/datatypes/Integer2MonthFilter.h"
#include "backend/core/datatypes/Integer2DayOfWeekFilter.h"
#include "backend/core/datatypes/String2IntegerFilter.h"
#include "backend/core/datatypes/String2DateTimeFilter.h"
#include "backend/core/datatypes/DateTime2StringFilter.h"
#include "backend/core/datatypes/String2MonthFilter.h"
#include "backend/core/datatypes/String2DayOfWeekFilter.h"
#include "backend/core/datatypes/DateTime2DoubleFilter.h"
#include "backend/core/datatypes/DateTime2IntegerFilter.h"
#include "backend/core/datatypes/DayOfWeek2DoubleFilter.h"
#include "backend/core/datatypes/DayOfWeek2IntegerFilter.h"
#include "backend/core/datatypes/Month2DoubleFilter.h"
#include "backend/core/datatypes/Month2IntegerFilter.h"

#endif // ifndef FILTER_H
