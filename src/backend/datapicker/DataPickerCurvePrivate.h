
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

#ifndef DATAPICKERCURVEPRIVATE_H
#define DATAPICKERCURVEPRIVATE_H

class DataPickerCurvePrivate {
	public:
        explicit DataPickerCurvePrivate() {};

        Image::Errors curveErrorTypes;
        bool visible;
		
        AbstractColumn* posXColumn;
        QString posXColumnPath;
        AbstractColumn* posYColumn;
        QString posYColumnPath;
        AbstractColumn* plusDeltaXColumn;
        QString plusDeltaXColumnPath;
        AbstractColumn* minusDeltaXColumn;
        QString minusDeltaXColumnPath;
        AbstractColumn* plusDeltaYColumn;
        QString plusDeltaYColumnPath;
        AbstractColumn* minusDeltaYColumn;
        QString minusDeltaYColumnPath;
};

#endif
