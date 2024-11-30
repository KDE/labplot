#include "WorksheetElementDock.h"

#include <KComboBox>
#include "frontend/widgets/NumberSpinBox.h"

WorksheetElementDock::WorksheetElementDock(QWidget* parent) : BaseDock(parent) {}

void WorksheetElementDock::setWorksheetElementWidgets(KComboBox* cbPositionX,
    KComboBox* cbPositionY,
    NumberSpinBox* sbPositionX,
    NumberSpinBox* sbPositionY,
    KComboBox* cbHorizontalAlignment,
    KComboBox* cbVerticalAlignment,
    NumberSpinBox* sbPositionXLogical,
    NumberSpinBox* sbPositionYLogical,
    QSpinBox* sbRotation,
    QCheckBox* chbLock,
    QCheckBox* chbBindLogicalPos) {

    this->cbPositionX = cbPositionX;
    this->cbPositionY = cbPositionY;
    this->sbPositionX = sbPositionX;
    this->sbPositionY = sbPositionY;
    this->cbHorizontalAlignment = cbHorizontalAlignment;
    this->cbVerticalAlignment = cbVerticalAlignment;
    this->sbPositionXLogical = sbPositionXLogical;
    this->sbPositionYLogical = sbPositionYLogical;
    this->sbRotation = sbRotation;
    this->chbLock = chbLock;
    this->chbBindLogicalPos = chbBindLogicalPos;

    // Positioning and alignment
    cbPositionX->addItem(i18n("Left"));
    cbPositionX->addItem(i18n("Center"));
    cbPositionX->addItem(i18n("Right"));
    cbPositionX->addItem(i18n("Relative to plot"));

    cbPositionY->addItem(i18n("Top"));
    cbPositionY->addItem(i18n("Center"));
    cbPositionY->addItem(i18n("Bottom"));
    cbPositionY->addItem(i18n("Relative to plot"));

    QString suffix;
    if (m_units == BaseDock::Units::Metric)
        suffix = QLatin1String(" cm");
    else
        suffix = QLatin1String(" in");

    sbPositionX->setSuffix(suffix);
    sbPositionY->setSuffix(suffix);

    cbHorizontalAlignment->addItem(i18n("Left"));
    cbHorizontalAlignment->addItem(i18n("Center"));
    cbHorizontalAlignment->addItem(i18n("Right"));

    cbVerticalAlignment->addItem(i18n("Top"));
    cbVerticalAlignment->addItem(i18n("Center"));
    cbVerticalAlignment->addItem(i18n("Bottom"));

    connect(cbPositionX, QOverload<int>::of(&KComboBox::currentIndexChanged), this, &WorksheetElementDock::positionXChanged);
    connect(cbPositionY, QOverload<int>::of(&KComboBox::currentIndexChanged), this, &WorksheetElementDock::positionYChanged);
    connect(sbPositionX, QOverload<double>::of(&NumberSpinBox::valueChanged), this, &WorksheetElementDock::customPositionXChanged);
    connect(sbPositionY, QOverload<double>::of(&NumberSpinBox::valueChanged), this, &WorksheetElementDock::customPositionYChanged);
    connect(cbHorizontalAlignment, QOverload<int>::of(&KComboBox::currentIndexChanged), this, &WorksheetElementDock::horizontalAlignmentChanged);
    connect(cbVerticalAlignment, QOverload<int>::of(&KComboBox::currentIndexChanged), this, &WorksheetElementDock::verticalAlignmentChanged);

    connect(sbPositionXLogical, QOverload<double>::of(&NumberSpinBox::valueChanged), this, &WorksheetElementDock::positionXLogicalChanged);
    connect(dtePositionXLogical, &UTCDateTimeEdit::mSecsSinceEpochUTCChanged, this, &WorksheetElementDock::positionXLogicalDateTimeChanged);
    connect(sbPositionYLogical, QOverload<double>::of(&NumberSpinBox::valueChanged), this, &WorksheetElementDock::positionYLogicalChanged);

    connect(sbRotation, QOverload<int>::of(&QSpinBox::valueChanged), this, &WorksheetElementDock::rotationChanged);
    connect(chbLock, &QCheckBox::clicked, this, &WorksheetElementDock::lockChanged);
    connect(chbBindLogicalPos, &QCheckBox::clicked, this, &WorksheetElementDock::bindingChanged);

    QString msg = i18n("Use logical instead of absolute coordinates to specify the position on the plot");
    chbBindLogicalPos->setToolTip(msg);
}

// ############################################################################################
// dock -> element
// ############################################################################################

// positioning using absolute coordinates
/*!
// TODO: move this into the worksheet element!
	called when element's current horizontal position relative to its parent (left, center, right, relative) is changed.
*/
void WorksheetElementDock::positionXChanged(int index) {
	CONDITIONAL_LOCK_RETURN;

	const auto pos = WorksheetElement::HorizontalPosition(index);

	if (pos == WorksheetElement::HorizontalPosition::Relative) {
		sbPositionX->setSuffix(QStringLiteral(" %"));
	} else {
		if (m_units == Units::Metric)
			sbPositionX->setSuffix(QStringLiteral(" cm"));
		else
			sbPositionX->setSuffix(QStringLiteral(" in"));
	}

	for (auto* element : m_elements)
		element->setPositionHorizontal(pos);
}

/*!
	called when element's current horizontal position relative to its parent (top, center, bottom, relative) is changed.
*/
void WorksheetElementDock::positionYChanged(int index) {
	CONDITIONAL_LOCK_RETURN;

	const auto pos = WorksheetElement::VerticalPosition(index);

	if (pos == WorksheetElement::VerticalPosition::Relative) {
		sbPositionY->setSuffix(QStringLiteral(" %"));
	} else {
		if (m_units == Units::Metric)
			sbPositionY->setSuffix(QStringLiteral(" cm"));
		else
			sbPositionY->setSuffix(QStringLiteral(" in"));
	}

	for (auto* element : m_elements)
		element->setPositionVertical(pos);
}

void WorksheetElementDock::horizontalAlignmentChanged(int index) {
	CONDITIONAL_LOCK_RETURN;

	for (auto* element : m_elements)
		element->setHorizontalAlignment(WorksheetElement::HorizontalAlignment(index));
}

void WorksheetElementDock::verticalAlignmentChanged(int index) {
	CONDITIONAL_LOCK_RETURN;

	for (auto* element : m_elements)
		element->setVerticalAlignment(WorksheetElement::VerticalAlignment(index));
}

void WorksheetElementDock::customPositionXChanged(double value) {
	CONDITIONAL_RETURN_NO_LOCK;

	for (auto* element : m_elements) {
		auto position = element->position();
		if (position.horizontalPosition == WorksheetElement::HorizontalPosition::Relative)
			position.point.setX(value / 100.);
		else
			position.point.setX(Worksheet::convertToSceneUnits(value, m_worksheetUnit));
		element->setPosition(position);
	}
}

void WorksheetElementDock::customPositionYChanged(double value) {
	CONDITIONAL_RETURN_NO_LOCK;

	for (auto* element : m_elements) {
		auto position = element->position();
		if (position.verticalPosition == WorksheetElement::VerticalPosition::Relative)
			position.point.setY(value / 100.);
		else
			position.point.setY(Worksheet::convertToSceneUnits(value, m_worksheetUnit));
		element->setPosition(position);
	}
}

// positioning using logical plot coordinates
void WorksheetElementDock::positionXLogicalChanged(double value) {
	CONDITIONAL_RETURN_NO_LOCK;

	for (auto* element : m_elements) {
		auto pos = element->positionLogical();
		pos.setX(value);
		element->setPositionLogical(pos);
	}
}

void WorksheetElementDock::positionXLogicalDateTimeChanged(qint64 value) {
	CONDITIONAL_LOCK_RETURN;

	for (auto* element : m_elements) {
		auto pos = element->positionLogical();
		pos.setX(value);
		element->setPositionLogical(pos);
	}
}

void WorksheetElementDock::positionYLogicalChanged(double value) {
	CONDITIONAL_RETURN_NO_LOCK;

	for (auto* element : m_elements) {
		auto pos = element->positionLogical();
		pos.setY(value);
		element->setPositionLogical(pos);
	}
}

void WorksheetElementDock::rotationChanged(int value) {
	CONDITIONAL_LOCK_RETURN;

	for (auto* element : m_elements)
		element->setRotationAngle(value);
}

void WorksheetElementDock::rotationChanged(int value) {
	CONDITIONAL_LOCK_RETURN;

	for (auto* element : m_elements)
		element->setRotationAngle(value);
}

void WorksheetElementDock::lockChanged(bool locked) {
	CONDITIONAL_LOCK_RETURN;
	for (auto* element : m_elements)
		element->setLock(locked);
}

/*!
 * \brief WorksheetElementDock::bindingChanged
 * Bind element to the cartesian plot coords or not
 * \param checked
 */
void WorksheetElementDock::bindingChanged(bool checked) {
	// widgets for positioning using absolute plot distances
	lPositionX->setVisible(!checked);
	cbPositionX->setVisible(!checked);
	sbPositionX->setVisible(!checked);

	lPositionY->setVisible(!checked);
	cbPositionY->setVisible(!checked);
	sbPositionY->setVisible(!checked);

	// widgets for positioning using logical plot coordinates
	const auto* plot = static_cast<const CartesianPlot*>(m_element->parent(AspectType::CartesianPlot));
	if (plot && plot->xRangeFormatDefault() == RangeT::Format::DateTime) {
		lPositionXLogicalDateTime->setVisible(checked);
		dtePositionXLogical->setVisible(checked);

		lPositionXLogical->setVisible(false);
		sbPositionXLogical->setVisible(false);
	} else {
		lPositionXLogicalDateTime->setVisible(false);
		dtePositionXLogical->setVisible(false);

		lPositionXLogical->setVisible(checked);
		sbPositionXLogical->setVisible(checked);
	}

	lPositionYLogical->setVisible(checked);
	sbPositionYLogical->setVisible(checked);

	CONDITIONAL_LOCK_RETURN;

	chbBindLogicalPos->setChecked(checked);

	for (auto* element : m_elements)
		element->setCoordinateBindingEnabled(checked);
}

// ############################################################################################
// element -> dock
// ############################################################################################

void WorksheetElementDock::elementPositionChanged(const WorksheetElement::PositionWrapper& position) {
	CONDITIONAL_LOCK_RETURN;

	cbPositionX->setCurrentIndex(static_cast<int>(position.horizontalPosition));
	cbPositionY->setCurrentIndex(static_cast<int>(position.verticalPosition));
	if (position.horizontalPosition == WorksheetElement::HorizontalPosition::Relative) {
		sbPositionX->setValue(position.point.x() * 100.);
		sbPositionX->setSuffix(QStringLiteral(" %"));
	} else
		sbPositionX->setValue(Worksheet::convertFromSceneUnits(position.point.x(), m_worksheetUnit));

	if (position.verticalPosition == WorksheetElement::VerticalPosition::Relative) {
		sbPositionY->setValue(position.point.y() * 100.);
		sbPositionY->setSuffix(QStringLiteral(" %"));
	} else
		sbPositionY->setValue(Worksheet::convertFromSceneUnits(position.point.y(), m_worksheetUnit));
}

void WorksheetElementDock::elementHorizontalAlignmentChanged(WorksheetElement::HorizontalAlignment index) {
	CONDITIONAL_LOCK_RETURN;
	cbHorizontalAlignment->setCurrentIndex(static_cast<int>(index));
}

void WorksheetElementDock::elementVerticalAlignmentChanged(WorksheetElement::VerticalAlignment index) {
	CONDITIONAL_LOCK_RETURN;
	cbVerticalAlignment->setCurrentIndex(static_cast<int>(index));
}

void WorksheetElementDock::elementPositionLogicalChanged(QPointF pos) {
	CONDITIONAL_LOCK_RETURN;
	sbPositionXLogical->setValue(pos.x());
	dtePositionXLogical->setMSecsSinceEpochUTC(pos.x());
	sbPositionYLogical->setValue(pos.y());
	// TODO: why not dtePositionYLogical->setMSecsSinceEpochUTC(pos.y());
}

void WorksheetElementDock::elementRotationAngleChanged(qreal angle) {
	CONDITIONAL_LOCK_RETURN;
	sbRotation->setValue(angle);
}

void WorksheetElementDock::elementLockChanged(bool on) {
	CONDITIONAL_LOCK_RETURN;
	chbLock->setChecked(on);
}

void WorksheetElementDock::elementCoordinateBindingEnabledChanged(bool enabled) {
	CONDITIONAL_LOCK_RETURN;
	bindingChanged(enabled);
}