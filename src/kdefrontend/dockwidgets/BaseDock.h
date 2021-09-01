/*
    File             : BaseDock.h
    Project          : LabPlot
    Description      : Base dock widget
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2019 Martin Marmsoler (martin.marmsoler@gmail.com)
    SPDX-FileCopyrightText: 2019-2020 Alexander Semke (alexander.semke@web.de)
    SPDX-FileCopyrightText: 2020-2021 Stefan Gerlach (stefan.gerlach@uni.kn)
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef BASEDOCK
#define BASEDOCK

#include "backend/worksheet/Worksheet.h"

#include <QWidget>
#include <QLineEdit>

class AbstractAspect;
class ResizableTextEdit;
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
	ResizableTextEdit* m_teComment{nullptr};
	AbstractAspect* m_aspect{nullptr};
	QList<AbstractAspect*> m_aspects;
	Units m_units{Units::Metric};
	Worksheet::Unit m_worksheetUnit{Worksheet::Unit::Centimeter};
	void updatePlotRangeList(QComboBox*) const;	// used in worksheet element docks

protected slots:
	void nameChanged();
	void commentChanged();
	void aspectDescriptionChanged(const AbstractAspect*);
	void plotRangeChanged(int index);	// used in worksheet element docks
};

#endif
