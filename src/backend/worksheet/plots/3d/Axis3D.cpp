#include "Axis3D.h"
#include "Scatter3DPlotArea.h"
#include "backend/lib/commandtemplates.h"
#include "backend/lib/macros.h"
#include "backend/worksheet/plots/3d/Axis3DPrivate.h"

class Axis3DPrivate;
Axis3D::Axis3D(QString name, Type type)
	: AbstractAspect(name, AspectType::Axis3D)
	, m_axis(new QValue3DAxis())
	, d_ptr(new Axis3DPrivate(this)) {
	setType(type);
	m_axis->setTitleVisible(true);
}

void Axis3D::setRange(float min, float max) {
	Q_D(Axis3D);
	m_axis->setRange(min, max);
	d->minRange = min;
	d->maxRange = max;
}
void Axis3D::save(QXmlStreamWriter*) const {
}
bool Axis3D::load(XmlStreamReader*, bool preview) {
	return false;
}

// ##############################################################################
// ##########################  getter methods  ##################################
// ##############################################################################

BASIC_SHARED_D_READER_IMPL(Axis3D, Axis3D::Format, axisFormat, axisFormat);
BASIC_SHARED_D_READER_IMPL(Axis3D, QString, title, title);
BASIC_SHARED_D_READER_IMPL(Axis3D, float, minRange, minRange);
BASIC_SHARED_D_READER_IMPL(Axis3D, float, maxRange, maxRange);
BASIC_SHARED_D_READER_IMPL(Axis3D, int, subSegmentCount, subSegmentCount);
BASIC_SHARED_D_READER_IMPL(Axis3D, int, segmentCount, segmentCount);
BASIC_SHARED_D_READER_IMPL(Axis3D, Axis3D::Type, type, type);

// ##############################################################################
// #################  setter methods and undo commands ##########################
// ##############################################################################
void Axis3D::setType(Axis3D::Type type) {
	Q_D(Axis3D);
	if (d->type != type)
		d->type = type;
}
STD_SETTER_CMD_IMPL_F_S(Axis3D, SetTitle, QString, title, updateTitle)
void Axis3D::setTitle(QString title) {
	Q_D(Axis3D);
	if (title != d->title)
		exec(new Axis3DSetTitleCmd(d, title, ki18n("%1: Axis3D title changed")));
}
STD_SETTER_CMD_IMPL_F_S(Axis3D, SetMaxRange, float, maxRange, updateMaxRange)
void Axis3D::setMaxRange(float max) {
	Q_D(Axis3D);
	if (max != d->maxRange)
		exec(new Axis3DSetMaxRangeCmd(d, max, ki18n("%1: Axis3D max range changed")));
}
STD_SETTER_CMD_IMPL_F_S(Axis3D, SetMinRange, float, minRange, updateMinRange)
void Axis3D::setMinRange(float min) {
	Q_D(Axis3D);
	if (min != d->minRange)
		exec(new Axis3DSetMinRangeCmd(d, min, ki18n("%1: Axis3D min range changed")));
}
STD_SETTER_CMD_IMPL_F_S(Axis3D, SetSegmentCount, int, segmentCount, updateSegmentCount)
void Axis3D::setSegmentCount(int count) {
	Q_D(Axis3D);
	if (count != d->segmentCount)
		exec(new Axis3DSetSegmentCountCmd(d, count, ki18n("%1 Axis3D segment count changed")));
}

STD_SETTER_CMD_IMPL_F_S(Axis3D, SetSubSegmentCount, int, subSegmentCount, updateSubSegmentCount)
void Axis3D::setSubSegmentCount(int count) {
	Q_D(Axis3D);
	if (count != d->segmentCount)
		exec(new Axis3DSetSubSegmentCountCmd(d, count, ki18n("%1 Axis3D sub segment count changed")));
}
STD_SETTER_CMD_IMPL_F_S(Axis3D, SetFormat, Axis3D::Format, axisFormat, updateFormat)
void Axis3D::setAxisFormat(Axis3D::Format format) {
	Q_D(Axis3D);
	if (format != d->axisFormat)
		exec(new Axis3DSetFormatCmd(d, format, ki18n("%1: Axis3D format Changed")));
}
// #####################################################################
// ################### Private implementation ##########################
// #####################################################################
Axis3DPrivate::Axis3DPrivate(Axis3D* owner)
	: q(owner)
	, axisFormat(Axis3D::Format_Decimal)
	, minRange(0)
	, maxRange(100)
	, title(QStringLiteral(""))
	, segmentCount(10)
	, subSegmentCount(10) {
}
QString Axis3DPrivate::formatToString(Axis3D::Format format) {
	switch (format) {
	case Axis3D::Format_Decimal:
		return QStringLiteral("%.2f"); // Decimal format with 2 decimal places
	case Axis3D::Format_Scientific:
		return QStringLiteral("%.1e"); // Scientific notation with 1 decimal place
	case Axis3D::Format_PowerOf10:
		return QStringLiteral("10^%.2f"); // Custom format for Power of 10 (may need special handling)
	case Axis3D::Format_PowerOf2:
		return QStringLiteral("2^%.2f"); // Custom format for Power of 2 (may need special handling)
	case Axis3D::Format_PowerOfE:
		return QStringLiteral("e^%.2f"); // Custom format for Power of E (may need special handling)
	case Axis3D::Format_MultiplierOfPi:
		return QStringLiteral("%.2fPi"); // Custom format for Pi (may need special handling)
	default:
		return QStringLiteral("%.2f"); // Default to Decimal format
	}
}
QString Axis3DPrivate::name() const {
	return q->parentAspect()->name();
}

void Axis3DPrivate::updateTitle() {
	q->m_axis->setTitle(title);
	WorksheetElement* element = static_cast<WorksheetElement*>(q->parent(AspectType::WorksheetElement));
	element->changed();
}

void Axis3DPrivate::updateMaxRange() {
	q->m_axis->setMax(maxRange);
	WorksheetElement* element = static_cast<WorksheetElement*>(q->parent(AspectType::WorksheetElement));
	element->changed();
}

void Axis3DPrivate::updateMinRange() {
	q->m_axis->setMin(minRange);
	WorksheetElement* element = static_cast<WorksheetElement*>(q->parent(AspectType::WorksheetElement));
	element->changed();
}

void Axis3DPrivate::updateSegmentCount() {
	q->m_axis->setSegmentCount(segmentCount);
	WorksheetElement* element = static_cast<WorksheetElement*>(q->parent(AspectType::WorksheetElement));
	element->changed();
}

void Axis3DPrivate::updateSubSegmentCount() {
	q->m_axis->setSubSegmentCount(subSegmentCount);
	WorksheetElement* element = static_cast<WorksheetElement*>(q->parent(AspectType::WorksheetElement));
	element->changed();
}
void Axis3DPrivate::updateFormat() {
	q->m_axis->setLabelFormat(formatToString(axisFormat));
	WorksheetElement* element = static_cast<WorksheetElement*>(q->parent(AspectType::WorksheetElement));
	element->changed();
}
