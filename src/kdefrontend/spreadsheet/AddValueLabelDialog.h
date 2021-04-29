/***************************************************************************
    File                 : AddValueLabelDialog.h
    Project              : LabPlot
    Description          : Dialog to add a new the value label
    --------------------------------------------------------------------
    Copyright            : (C) 2021 by Alexander Semke (alexander.semke@web.de)

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
#ifndef ADDVALUELABELDIALOG_H
#define ADDVALUELABELDIALOG_H

#include "backend/core/AbstractColumn.h"
#include <QDialog>
class QLineEdit;

class AddValueLabelDialog : public QDialog {
	Q_OBJECT

public:
	explicit AddValueLabelDialog(QWidget* parent = nullptr, AbstractColumn::ColumnMode = AbstractColumn::ColumnMode::Numeric);
	~AddValueLabelDialog() override;

	double value() const;
	int valueInt() const;
	qint64 valueBigInt() const;
	QString valueText() const;
	QDateTime valueDateTime() const;

	QString label() const;

private:
	QLineEdit* leValue;
	QLineEdit* leLabel;
	AbstractColumn::ColumnMode m_mode;
};

#endif
