/***************************************************************************
	File                 : DateTimeSpinBox.h
	Project              : LabPlot
	Description          : widget for setting datetimes with a spinbox
	--------------------------------------------------------------------
	Copyright            : (C) 2019 Martin Marmsoler (martin.marmsoler@gmail.com)

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

#ifndef DATETIMESPINBOX_H
#define DATETIMESPINBOX_H

#include <QAbstractSpinBox>

class QRegularExpressionValidator;


// Assumption: Month has always 30 days
class DateTimeSpinBox: public QAbstractSpinBox
{

	Q_OBJECT;
private:
    enum Type {
	year,
	month,
	day,
	hour,
	minute,
	second,
	millisecond
    };

public:
        DateTimeSpinBox(QWidget* parent);
	void keyPressEvent(QKeyEvent *event) override;
	void stepBy(int steps) override;
	QAbstractSpinBox::StepEnabled stepEnabled() const override;
	bool increaseValue(Type type, int step);
	bool changeValue(qint64& thisType, Type nextTypeType, int step);
	Type determineType(int cursorPos) const;
	void writeValue();
	void setValue(qint64 increment);
	qint64 value();
	void getValue();
	void setCursorPosition(Type type);
	bool valid();
private:
	QRegularExpressionValidator *m_regularExpressionValidator;
	qint64 m_year{0}, m_month{0}, m_day{0}, m_hour{0}, m_minute{0}, m_second{0}, m_millisecond{0};
signals:
	void valueChanged();
};

#endif // DATETIMESPINBOX_H
