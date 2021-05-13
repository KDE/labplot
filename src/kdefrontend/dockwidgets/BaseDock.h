/***************************************************************************
    File             : BaseDock.h
    Project          : LabPlot
    Description      : Base dock widget
    --------------------------------------------------------------------
    Copyright         : (C) 2019 Martin Marmsoler (martin.marmsoler@gmail.com)
    Copyright         : (C) 2019-2020 Alexander Semke (alexander.semke@web.de)
    Copyright         : (C) 2020-2021 Stefan Gerlach (stefan.gerlach@uni.kn)
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

#ifndef BASEDOCK
#define BASEDOCK

#include "backend/worksheet/Worksheet.h"

#include <QWidget>
#include <QLineEdit>

class AbstractAspect;
class QComboBox;
class QDoubleSpinBox;

struct Lock {
	inline explicit Lock(bool& variable)
		: variable(variable = true) {
	}

	inline ~Lock() {
		variable = false;
	}

private:
	bool& variable;
};


class BaseDock : public QWidget {
	Q_OBJECT

public:
	explicit BaseDock(QWidget* parent);
	~BaseDock();

	enum class Units {Metric, Imperial};

	virtual void updateLocale() {};
	virtual void updateUnits() {};
	virtual void updatePlotRanges() const {};	// used in worksheet element docks
    static void spinBoxCalculateMinMax(QDoubleSpinBox* spinbox, Range<double> range, double newValue=NAN);

protected:
	bool m_initializing{false};
	QLineEdit* m_leName{nullptr};
	QLineEdit* m_leComment{nullptr};
	AbstractAspect* m_aspect{nullptr};
	QList<AbstractAspect*> m_aspects;
	Units m_units{Units::Metric};
	Worksheet::Unit m_worksheetUnit{Worksheet::Unit::Centimeter};
	void updatePlotRangeList(QComboBox*) const;	// used in worksheet element docks

protected slots:
	void nameChanged();
	void commentChanged();
	void plotRangeChanged(int index);	// used in worksheet element docks
};

#endif
