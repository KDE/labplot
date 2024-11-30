/*
	File             : WorksheetElementDock.h
	Project          : LabPlot
	Description      : WorksheetElement dock widget. All docks of elements based on the worksheet element shall derive from this dock
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2024 Martin Marmsoler <martin.marmsoler@gmail.com>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef WORKSHEETELEMENTDOCK_H
#define WORKSHEETELEMENTDOCK_H

#include "BaseDock.h"

class KComboBox;
class NumberSpinBox;
class QCheckBox;
class QSpinBox;
class WorksheetElement;

class WorksheetElementDock : public BaseDock {
	Q_OBJECT

public:
    explicit WorksheetElementDock(QWidget* parent);
    void setWorksheetElementWidgets(KComboBox* cbPositionX,
        KComboBox* cbPositionY,
        NumberSpinBox* sbPositionX,
        NumberSpinBox* sbPositionY,
        KComboBox* cbHorizontalAlignment,
        KComboBox* cbVerticalAlignment,
        NumberSpinBox* sbPositionXLogical,
        NumberSpinBox* sbPositionYLogical,
        QSpinBox* sbRotation,
        QCheckBox* chbLock,
        QCheckBox* chbBindLogicalPos);
    template<typename T>
    void setAspects(QList<T*> aspects) {
        m_elements.clear();
        if (!BaseDock::setAspects(aspects)) {
            m_element = nullptr;
            return; // Nothing to do
        }

        m_element = aspects.first();
        for (auto* aspect : aspects) {
			if (aspect->inherits(AspectType::WorksheetElement)) {
				m_elements.append(static_cast<WorksheetElement*>(aspect));
            } else
				assert(false); // Should never happen
		}

        connect(m_element, &WorksheetElement::coordinateSystemIndexChanged, this, &WorksheetElementDock::updatePlotRangeList);
        connect(m_element, &WorksheetElement::plotRangeListChanged, this, &WorksheetElementDock::updatePlotRangeList);
        connect(m_element, &WorksheetElement::visibleChanged, this, &WorksheetElementDock::aspectVisibleChanged);
        auto* plot = dynamic_cast<Plot*>(m_element);
        if (plot)
            connect(plot, &Plot::legendVisibleChanged, this, &WorksheetElementDock::aspectLegendVisibleChanged);


        connect(m_element, &WorksheetElement::lockChanged, this, &WorksheetElementDock::elementLockChanged);
        connect(m_element, &WorksheetElement::positionChanged, this, &WorksheetElementDock::elementPositionChanged);

        connect(m_element, &WorksheetElement::positionLogicalChanged, this, &WorksheetElementDock::elementPositionLogicalChanged);
        connect(m_element, &WorksheetElement::coordinateBindingEnabledChanged, this, &WorksheetElementDock::elementCoordinateBindingEnabledChanged);
        connect(m_element, &WorksheetElement::horizontalAlignmentChanged, this, &WorksheetElementDock::elementHorizontalAlignmentChanged);
        connect(m_element, &WorksheetElement::verticalAlignmentChanged, this, &WorksheetElementDock::elementVerticalAlignmentChanged);
        connect(m_element, &WorksheetElement::rotationAngleChanged, this, &WorksheetElementDock::elementRotationAngleChanged);
    }

// dock -> element
protected:
    void positionXChanged(int);
	void positionYChanged(int);
	void customPositionXChanged(double);
	void customPositionYChanged(double);
	void horizontalAlignmentChanged(int);
	void verticalAlignmentChanged(int);

	void positionXLogicalChanged(double);
	void positionXLogicalDateTimeChanged(qint64);
	void positionYLogicalChanged(double);

	void rotationChanged(int);
    void lockChanged(bool);
	void bindingChanged(bool checked);

// element -> dock
protected:
    void elementPositionChanged(const WorksheetElement::PositionWrapper& position);
    void elementHorizontalAlignmentChanged(WorksheetElement::HorizontalAlignment index);
    void elementVerticalAlignmentChanged(WorksheetElement::VerticalAlignment index);
    void elementPositionLogicalChanged(QPointF pos);
    void elementRotationAngleChanged(qreal angle);
    void elementLockChanged(bool on);
    void elementCoordinateBindingEnabledChanged(bool enabled);

private:
    KComboBox* cbPositionX {nullptr};
    KComboBox* cbPositionY {nullptr};
    NumberSpinBox* sbPositionX {nullptr};
    NumberSpinBox* sbPositionY {nullptr};
    KComboBox* cbHorizontalAlignment {nullptr};
    KComboBox* cbVerticalAlignment {nullptr};
    NumberSpinBox* sbPositionXLogical {nullptr};
    NumberSpinBox* sbPositionYLogical {nullptr};
    QSpinBox* sbRotation {nullptr};
    QCheckBox* chbLock {nullptr};
    QCheckBox* chbBindLogicalPos {nullptr};

private:
    WorksheetElement* m_element;
    QList<WorksheetElement*> m_elements;

};

#endif // WORKSHEETELEMENTDOCK_H
