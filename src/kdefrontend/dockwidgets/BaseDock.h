/*
	File             : BaseDock.h
	Project          : LabPlot
	Description      : Base dock widget
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2019 Martin Marmsoler <martin.marmsoler@gmail.com>
	SPDX-FileCopyrightText: 2019-2020 Alexander Semke <alexander.semke@web.de>
	SPDX-FileCopyrightText: 2020-2021 Stefan Gerlach <stefan.gerlach@uni.kn>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef BASEDOCK
#define BASEDOCK

#include "backend/worksheet/Worksheet.h"

#include <QLineEdit>
#include <QWidget>

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

	enum class Units { Metric, Imperial };

	virtual void updateLocale(){};
	virtual void updateUnits(){};
	virtual void updatePlotRanges(){}; // used in worksheet element docks
	static void spinBoxCalculateMinMax(QDoubleSpinBox* spinbox, Range<double> range, double newValue = NAN);
    template <typename T> // TODO: check if it is possible to check at compile time if the type is derived from AbstractAspect
    void setAspects(QList<T*> aspects) {
        m_aspects.clear();
        if (aspects.length() == 0) {
            m_aspect = nullptr;
            return;
        }

        m_aspect = aspects.first();
        for (auto* aspect: aspects) {
            m_aspects.append(static_cast<AbstractAspect*>(aspect));
        }
    }

protected:
	bool m_initializing{false};
	QLineEdit* m_leName{nullptr};
	ResizableTextEdit* m_teComment{nullptr};
	AbstractAspect* m_aspect{nullptr};
	QList<AbstractAspect*> m_aspects;
	Units m_units{Units::Metric};
	Worksheet::Unit m_worksheetUnit{Worksheet::Unit::Centimeter};
	void updatePlotRangeList(QComboBox*); // used in worksheet element docks

protected Q_SLOTS:
	void nameChanged();
	void commentChanged();
	void aspectDescriptionChanged(const AbstractAspect*);
	void plotRangeChanged(int index); // used in worksheet element docks

private:
	bool m_suppressPlotRetransform{false};
};

#endif
