/***************************************************************************
    File                 : Function.h
    Project              : LabPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2008 by Alexander Semke
    Email (use @ for *)  : alexander.semke*web.de
    Description          : function class
                           
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
#ifndef FUNCTION_H
#define FUNCTION_H

#include <QString>
#include "../definitions.h"

class Function{
public:
	Function();
	~Function();

	ACCESS(QString, text, Text);
	ACCESS(float, axis1Start, Axis1Start);
	ACCESS(float, axis1End, Axis1End);
	ACCESS(float, axis1Number, Axis1Number);
	ACCESS(float, axis2Start, Axis2Start);
	ACCESS(float, axis2End, Axis2End);
	ACCESS(float, axis2Number, Axis2Number);

private:
	QString m_text;
	float m_axis1Start;
	float m_axis1End;
	float m_axis1Number;
	float m_axis2Start;
	float m_axis2End;
	float m_axis2Number;
};

#endif
