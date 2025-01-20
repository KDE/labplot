/*
	File             : BaseDock.h
	Project          : LabPlot
	Description      : Base dock widget
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2019 Martin Marmsoler <martin.marmsoler@gmail.com>
	SPDX-FileCopyrightText: 2019-2025 Alexander Semke <alexander.semke@web.de>
	SPDX-FileCopyrightText: 2020-2024 Stefan Gerlach <stefan.gerlach@uni.kn>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef BASEDOCK
#define BASEDOCK

#include "backend/core/AspectTreeModel.h"
#include "backend/worksheet/Worksheet.h"
#include "backend/worksheet/plots/cartesian/Plot.h"

#include <QWidget>

class AbstractAspect;
class ResizableTextEdit;
class TimedLineEdit;
class QCheckBox;
class QComboBox;
class QDoubleSpinBox;
class QLineEdit;

class BaseDock : public QWidget {
	Q_OBJECT

public:
	explicit BaseDock(QWidget* parent);
	void setPlotRangeCombobox(QComboBox*);
	~BaseDock();

	enum class Units { Metric, Imperial };

	virtual void retranslateUi() = 0;
	virtual void updateLocale(){};
	virtual void updateUnits(){};
	static void spinBoxCalculateMinMax(QDoubleSpinBox* spinbox, Range<double> range, double newValue = NAN);

	template<typename T>
	void setAspects(QList<T*> aspects) {
		if (m_aspect)
			disconnect(m_aspect, nullptr, this, nullptr);

		m_aspects.clear();
		if (aspects.length() == 0) {
			m_aspect = nullptr;
			return;
		}

		m_aspect = aspects.first();
		connect(m_aspect, &AbstractAspect::childAspectAboutToBeRemoved, this, &BaseDock::disconnectAspect);
		connect(m_aspect, &AbstractAspect::aspectDescriptionChanged, this, &BaseDock::aspectDescriptionChanged);

		auto* wse = dynamic_cast<WorksheetElement*>(m_aspect);
		if (wse) {
			connect(wse, &WorksheetElement::coordinateSystemIndexChanged, this, &BaseDock::updatePlotRangeList);
			connect(wse, &WorksheetElement::plotRangeListChanged, this, &BaseDock::updatePlotRangeList);
			connect(wse, &WorksheetElement::visibleChanged, this, &BaseDock::aspectVisibleChanged);
			auto* plot = dynamic_cast<Plot*>(wse);
			if (plot)
				connect(plot, &Plot::legendVisibleChanged, this, &BaseDock::aspectLegendVisibleChanged);
		}

		for (auto* aspect : aspects) {
			if (aspect->inherits(AspectType::AbstractAspect))
				m_aspects.append(static_cast<AbstractAspect*>(aspect));
		}

		// delete the potentially available model, will be re-created if needed for the newly set aspects
		delete m_aspectModel;
		m_aspectModel = nullptr;

		updateNameDescriptionWidgets();
	}

	void disconnectAspect(const AbstractAspect* a) {
		disconnect(a, nullptr, this, nullptr);
		m_aspect = nullptr;
	}

	const AbstractAspect* aspect() {
		return m_aspect;
	}

	void setBaseWidgets(TimedLineEdit* nameLabel, ResizableTextEdit* commentLabel, double commentHeightFactorNameLabel = 1.2);
	void setVisibilityWidgets(QCheckBox* visible, QCheckBox* legendVisible = nullptr);

	AspectTreeModel* aspectModel();

protected:
	bool m_initializing{false};
	Units m_units{Units::Metric};
	Worksheet::Unit m_worksheetUnit{Worksheet::Unit::Centimeter};

	// round value in spinboxes to 0.1 cm/in
	static double roundValue(double value) {
		return std::round(10. * value) / 10.;
	}
	static double roundSceneValue(double value, Units units = Units::Metric) {
		if (units == Units::Metric)
			return Worksheet::convertToSceneUnits(std::round(10. * Worksheet::convertFromSceneUnits(value, Worksheet::Unit::Centimeter)) / 10.,
				Worksheet::Unit::Centimeter);
		else
			return Worksheet::convertToSceneUnits(std::round(10. * Worksheet::convertFromSceneUnits(value, Worksheet::Unit::Inch)) / 10.,
				Worksheet::Unit::Inch);
	}

	virtual void updatePlotRangeList(); // used in worksheet element docks

private:
	AbstractAspect* m_aspect{nullptr};
	QList<AbstractAspect*> m_aspects;
	AspectTreeModel* m_aspectModel{nullptr};
	void updateNameDescriptionWidgets();
	QComboBox* m_cbPlotRangeList{nullptr};
	TimedLineEdit* m_leName{nullptr};
	ResizableTextEdit* m_teComment{nullptr};
	QCheckBox* m_chkVisible{nullptr};
	QCheckBox* m_chkLegendVisible{nullptr};

protected Q_SLOTS:
	// SLOTs for changes triggered in the dock widget
	void nameChanged();
	void commentChanged();
	void plotRangeChanged(int index);
	void visibleChanged(bool);
	void legendVisibleChanged(bool);

	// SLOTs for changes triggered in the aspect
	void aspectDescriptionChanged(const AbstractAspect*);
	void aspectVisibleChanged(bool);
	void aspectLegendVisibleChanged(bool);

private:
	bool m_suppressPlotRetransform{false};

	friend class RetransformTest;
	friend class MultiRangeTest3;
};

#endif
