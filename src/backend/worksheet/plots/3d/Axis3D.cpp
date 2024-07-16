#include "Axis3D.h"
#include "backend/lib/commandtemplates.h"
#include "backend/lib/macros.h"
#include "backend/worksheet/plots/3d/Axis3DPrivate.h"

class Axis3DPrivate;
Axis3D::Axis3D(QString name, Type type)
	: AbstractAspect(name, AspectType::Axis3D)
	, m_axis(new QValue3DAxis()) {
	setType(type);
}

QString Axis3D::formatToString(Format format) {
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
void Axis3D::setTitle(QString title) {
	Q_D(Axis3D);
	if (title != d->title) {
		m_axis->setTitle(title);
		d->title = title;
		Q_EMIT titleChanged(title);
	}
}
void Axis3D::setMaxRange(float max) {
	Q_D(Axis3D);
	if (max != d->maxRange) {
		m_axis->setMax(max);
		d->maxRange = max;
		Q_EMIT maxRangeChanged(max);
	}
}
void Axis3D::setMinRange(float min) {
	Q_D(Axis3D);
	if (min != d->minRange) {
		m_axis->setMin(min);
		d->minRange = min;
		Q_EMIT minRangeChanged(min);
	}
}
void Axis3D::setSegmentCount(int count) {
	Q_D(Axis3D);
	if (count != d->segmentCount) {
		m_axis->setSegmentCount(count);
		d->segmentCount = count;
		Q_EMIT segmentCountChanged(count);
	}
}
void Axis3D::setSubSegmentCount(int count) {
	Q_D(Axis3D);
	if (count != d->segmentCount) {
		m_axis->setMax(count);
		d->subSegmentCount = count;
		Q_EMIT subSegmentCountChanged(count);
	}
}
void Axis3D::setAxisFormat(Axis3D::Format format) {
	Q_D(Axis3D);
	if (format != d->axisFormat) {
		m_axis->setLabelFormat(formatToString(format));
		d->axisFormat = format;
		Q_EMIT axisFormatChanged(format);
	}
}
// #####################################################################
// ################### Private implementation ##########################
// #####################################################################
Axis3DPrivate::Axis3DPrivate(Axis3D* owner, const QString& name)
	: AbstractAspectPrivate(owner, name)
	, q(owner) {
}
