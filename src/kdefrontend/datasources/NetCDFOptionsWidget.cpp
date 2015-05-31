/***************************************************************************
File                 : NetCDFOptionsWidget.cpp
Project              : LabPlot
Description          : widget providing options for the import of NetCDF data
--------------------------------------------------------------------
Copyright            : (C) 2015 Stefan Gerlach (stefan.gerlach@uni.kn)
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
#include "NetCDFOptionsWidget.h"

 /*!
	\class NetCDFOptionsWidget
	\brief Widget providing options for the import of NetCDF data

	\ingroup kdefrontend
 */

NetCDFOptionsWidget::NetCDFOptionsWidget(QWidget* parent) : QWidget(parent) {
	ui.setupUi(this);
}

NetCDFOptionsWidget::~NetCDFOptionsWidget() {
}
