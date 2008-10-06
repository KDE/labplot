/***************************************************************************
    File                 : CurveRangeDialog.h
    Project              : SciDAVis
    --------------------------------------------------------------------
    Copyright            : (C) 2006 by Ion Vasilief, Tilman Benkert
    Email (use @ for *)  : ion_vasilief*yahoo.fr, thzs*gmx.net
    Description          : Filter options dialog

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
#ifndef CURVERANGEDIALOG_H
#define CURVERANGEDIALOG_H

#include <QDialog>

class QPushButton;
class QLabel;
class QSpinBox;
class Layer;
class DataCurve;

//! Curve range dialog
class CurveRangeDialog : public QDialog
{
    Q_OBJECT

public:
    CurveRangeDialog(QWidget* parent = 0, Qt::WFlags fl = 0 );
    ~CurveRangeDialog(){};

	QPushButton* buttonOK;
	QPushButton* buttonCancel;
	QLabel* boxName;
	QSpinBox* boxStart;
	QSpinBox* boxEnd;

public slots:
	void setCurveToModify(Layer *g, int curve);
	void accept();

private:
	DataCurve *m_curve;
	Layer *m_layer;
};

#endif
