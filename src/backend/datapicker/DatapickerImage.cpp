/*
	File                 : DatapickerImage.cpp
	Project              : LabPlot
	Description          : Worksheet for Datapicker
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2015 Ankit Wagadre <wagadre.ankit@gmail.com>
	SPDX-FileCopyrightText: 2015-2022 Alexander Semke <alexander.semke@web.de>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "DatapickerImage.h"
#include "DatapickerImagePrivate.h"
#include "backend/core/Project.h"
#include "backend/datapicker/DatapickerPoint.h"
#include "backend/datapicker/ImageEditor.h"
#include "backend/datapicker/Segments.h"
#include "backend/lib/XmlStreamReader.h"
#include "backend/lib/commandtemplates.h"
#include "backend/lib/trace.h"
#include "frontend/GuiTools.h"
#include "frontend/datapicker/DatapickerImageView.h"
#include "frontend/worksheet/ExportWorksheetDialog.h"

#include <QBuffer>
#include <QDir>
#include <QFileInfo>
#include <QGraphicsScene>
#include <QMenu>
#include <QPrintDialog>
#include <QPrintPreviewDialog>
#include <QPrinter>
#include <QScreen>

#include <KConfig>
#include <KConfigGroup>

void DatapickerImage::ReferencePoints::clearPoints() {
	scenePos[0] = QPointF(std::nan("0"), std::nan("0"));
	scenePos[1] = QPointF(std::nan("0"), std::nan("0"));
	scenePos[2] = QPointF(std::nan("0"), std::nan("0"));
}

/**
 * \class DatapickerImage
 * \brief container to open image/plot.
 *
 * Top-level container for DatapickerPoint.
 *
 * * \ingroup datapicker
 */
DatapickerImage::DatapickerImage(const QString& name, bool loading)
	: AbstractPart(name, AspectType::DatapickerImage)
	, foregroundBins(new int[ImageEditor::colorAttributeMax(ColorAttributes::Foreground) + 1])
	, hueBins(new int[ImageEditor::colorAttributeMax(ColorAttributes::Hue) + 1])
	, saturationBins(new int[ImageEditor::colorAttributeMax(ColorAttributes::Saturation) + 1])
	, valueBins(new int[ImageEditor::colorAttributeMax(ColorAttributes::Value) + 1])
	, intensityBins(new int[ImageEditor::colorAttributeMax(ColorAttributes::Intensity) + 1])
	, d_ptr(new DatapickerImagePrivate(this))
	, m_segments(new Segments(this)) {
	Q_D(DatapickerImage);
	connect(this, &AbstractAspect::childAspectAdded, this, &DatapickerImage::childAdded);
	connect(this, &AbstractAspect::childAspectAboutToBeRemoved, this, &DatapickerImage::childRemoved);
	if (!loading)
		init();
	else {
		d->symbol = new Symbol(QString());
		addChild(d->symbol);
		d->symbol->setHidden(true);
		connect(d->symbol, &Symbol::updateRequested, [=] {
			d->retransform();
		});
		connect(d->symbol, &Symbol::updatePixmapRequested, [=] {
			d->retransform();
		});
	}
}

DatapickerImage::~DatapickerImage() {
	delete[] hueBins;
	delete[] saturationBins;
	delete[] valueBins;
	delete[] intensityBins;
	delete[] foregroundBins;
	delete m_segments;
	delete d_ptr;
}

void DatapickerImage::init() {
	Q_D(DatapickerImage);
	KConfig config;
	KConfigGroup group = config.group(QStringLiteral("DatapickerImage"));

	// general properties
	d->fileName = group.readEntry(QStringLiteral("FileName"), QString());
	d->rotationAngle = group.readEntry(QStringLiteral("RotationAngle"), 0.0);
	d->minSegmentLength = group.readEntry(QStringLiteral("MinSegmentLength"), 30);
	d->pointSeparation = group.readEntry(QStringLiteral("PointSeparation"), 30);
	d->axisPoints.type = static_cast<GraphType>(group.readEntry(QStringLiteral("GraphType"), static_cast<int>(GraphType::Linear)));
	d->axisPoints.ternaryScale = group.readEntry(QStringLiteral("TernaryScale"), 1);

	// edit image settings
	d->plotImageType = DatapickerImage::PlotImageType::OriginalImage;
	d->settings.foregroundThresholdHigh = group.readEntry(QStringLiteral("ForegroundThresholdHigh"), d->settings.foregroundThresholdHigh);
	d->settings.foregroundThresholdLow = group.readEntry(QStringLiteral("ForegroundThresholdLow"), d->settings.foregroundThresholdLow);
	d->settings.hueThresholdHigh = group.readEntry(QStringLiteral("HueThresholdHigh"), d->settings.hueThresholdHigh);
	d->settings.hueThresholdLow = group.readEntry(QStringLiteral("HueThresholdLow"), d->settings.hueThresholdLow);
	d->settings.intensityThresholdHigh = group.readEntry(QStringLiteral("IntensityThresholdHigh"), d->settings.intensityThresholdHigh);
	d->settings.intensityThresholdLow = group.readEntry(QStringLiteral("IntensityThresholdLow"), d->settings.intensityThresholdLow);
	d->settings.saturationThresholdHigh = group.readEntry(QStringLiteral("SaturationThresholdHigh"), d->settings.saturationThresholdHigh);
	d->settings.saturationThresholdLow = group.readEntry(QStringLiteral("SaturationThresholdLow"), d->settings.saturationThresholdLow);
	d->settings.valueThresholdHigh = group.readEntry(QStringLiteral("ValueThresholdHigh"), d->settings.valueThresholdHigh);
	d->settings.valueThresholdLow = group.readEntry(QStringLiteral("ValueThresholdLow"), d->settings.valueThresholdLow);

	// reference point symbol properties
	d->symbol = new Symbol(QString());
	addChild(d->symbol);
	d->symbol->setHidden(true);
	connect(d->symbol, &Symbol::updateRequested, [=] {
		d->retransform();
	});
	connect(d->symbol, &Symbol::updatePixmapRequested, [=] {
		d->retransform();
	});
	d->symbol->init(group);

	d->pointVisibility = group.readEntry(QStringLiteral("PointVisibility"), true);
}

/*!
	Returns an icon to be used in the project explorer.
*/
QIcon DatapickerImage::icon() const {
	return QIcon::fromTheme(QStringLiteral("image-x-generic"));
}

/*!
	Return a new context menu
*/
QMenu* DatapickerImage::createContextMenu() {
	QMenu* menu = new QMenu(nullptr);
	Q_EMIT requestProjectContextMenu(menu);
	return menu;
}

void DatapickerImage::createContextMenu(QMenu* menu) {
	Q_EMIT requestProjectContextMenu(menu);
}

//! Construct a primary view on me.
/**
 * This method may be called multiple times during the life time of an Aspect, or it might not get
 * called at all. Aspects must not depend on the existence of a view for their operation.
 */
QWidget* DatapickerImage::view() const {
	if (!m_partView) {
		m_view = new DatapickerImageView(const_cast<DatapickerImage*>(this));
		m_partView = m_view;
		connect(m_view, &DatapickerImageView::statusInfo, this, &DatapickerImage::statusInfo);
	}
	return m_partView;
}

bool DatapickerImage::exportView() const {
	auto* dlg = new ExportWorksheetDialog(m_view);
	dlg->setProjectFileName(const_cast<DatapickerImage*>(this)->project()->fileName());
	dlg->setFileName(name());
	bool ret;
	if ((ret = (dlg->exec() == QDialog::Accepted))) {
		const QString path = dlg->path();
		const auto format = dlg->exportFormat();
		const int resolution = dlg->exportResolution();

		WAIT_CURSOR;
		m_view->exportToFile(path, format, resolution);
		RESET_CURSOR;
	}
	delete dlg;
	return ret;
}

bool DatapickerImage::printView() {
	QPrinter printer;
	auto* dlg = new QPrintDialog(&printer, m_view);
	bool ret;
	dlg->setWindowTitle(i18nc("@title:window", "Print Datapicker Image"));
	if ((ret = (dlg->exec() == QDialog::Accepted)))
		m_view->print(&printer);

	delete dlg;
	return ret;
}

bool DatapickerImage::printPreview() const {
	auto* dlg = new QPrintPreviewDialog(m_view);
	connect(dlg, &QPrintPreviewDialog::paintRequested, m_view, &DatapickerImageView::print);
	return dlg->exec();
}

/*!
	Selects or deselects the Datapicker/DatapickerImage in the project explorer.
	This function is called in \c DatapickerImageView.
	The DatapickerImage gets deselected if there are selected items in the view,
	and selected if there are no selected items in the view.
*/
void DatapickerImage::setSelectedInView(const bool b) {
	if (b)
		Q_EMIT childAspectSelectedInView(this);
	else
		Q_EMIT childAspectDeselectedInView(this);
}

void DatapickerImage::setSegmentsHoverEvent(const bool on) {
	m_segments->setAcceptHoverEvents(on);
}

QGraphicsScene* DatapickerImage::scene() const {
	Q_D(const DatapickerImage);
	return d->m_scene;
}

QRectF DatapickerImage::pageRect() const {
	Q_D(const DatapickerImage);
	return d->m_scene->sceneRect();
}

void DatapickerImage::setPlotImageType(const DatapickerImage::PlotImageType type) {
	Q_D(DatapickerImage);
	d->plotImageType = type;
	if (d->plotImageType == DatapickerImage::PlotImageType::ProcessedImage)
		d->discretize();

	Q_EMIT requestUpdate();
}

int DatapickerImage::currentSelectedReferencePoint() const {
	return m_currentRefPoint;
}

DatapickerImage::PlotImageType DatapickerImage::plotImageType() const {
	Q_D(const DatapickerImage);
	return d->plotImageType;
}

class DatapickerImageSetOriginalImageCmd : public QUndoCommand {
public:
	DatapickerImageSetOriginalImageCmd(DatapickerImage::Private* target,
									   const QImage& newImage,
									   const QString& filename,
									   bool embedded,
									   const KLocalizedString& description,
									   QUndoCommand* parent = nullptr)
		: QUndoCommand(parent)
		, m_img(newImage)
		, m_filename(filename)
		, m_embedded(embedded)
		, m_target(target) {
		setText(description.subs(m_target->name()).toString());
	}
	virtual void redo() override {
		const QImage tmp = m_target->q->originalPlotImage;
		const QString tmpFilename = m_target->q->fileName();
		const bool tmpEmbedded = m_target->q->embedded();

		if (m_embedded && !m_img.isNull())
			m_target->q->originalPlotImage = m_img;
		else
			m_target->q->originalPlotImage.load(m_filename);
		m_target->fileName = m_filename;
		m_target->embedded = m_embedded;

		if (tmpEmbedded)
			m_img = tmp;
		else
			m_img = QImage();
		m_filename = tmpFilename;
		m_embedded = tmpEmbedded;
		QUndoCommand::redo(); // redo all childs

		finalize();
		Q_EMIT m_target->q->fileNameChanged(m_target->fileName);
		Q_EMIT m_target->q->embeddedChanged(m_target->embedded);
	}

	virtual void undo() override {
		redo();
	}

	void finalize() {
		m_target->updateImage();
	}

private:
	QImage m_img;
	QString m_filename;
	bool m_embedded;
	DatapickerImage::Private* m_target;
};

// 1. Image from clipboard, 2. file from path (embedded or not embedded)
//     -> important to store qimage, because otherwise image is lost when doing undo
// 1. Image from clipboard, 2. image from clipboard
//     -> important to store qimage, because otherwise image is lost when doing undo
// 1. Image from path (embedded or not embedded), 2. image from clipboard
//     -> important to store qimage, because when doing redo after undo the image must be available
// 1. Image from path (not embedded), 2. image from path
//     -> storing qimage is not important
// 1. Image from path (embedded), 2. image from path
//     -> storing qimage is important because otherwise if undo and path is anymore valid image is anymore available

void DatapickerImage::setImage(const QString& fileName, bool embedded) {
	return setImage(QImage(), fileName, embedded);
}

void DatapickerImage::setImage(const QImage& image, const QString& filename, bool embedded) {
	Q_D(DatapickerImage);
	if (image != originalPlotImage || filename != fileName() || embedded != this->embedded())
		exec(new DatapickerImageSetOriginalImageCmd(d, image, filename, embedded, ki18n("%1: upload image")));
}

/* =============================== getter methods for background options ================================= */
BASIC_D_READER_IMPL(DatapickerImage, QString, fileName, fileName)
BASIC_D_READER_IMPL(DatapickerImage, bool, isRelativeFilePath, isRelativeFilePath)
BASIC_D_READER_IMPL(DatapickerImage, bool, embedded, embedded)
BASIC_D_READER_IMPL(DatapickerImage, DatapickerImage::ReferencePoints, axisPoints, axisPoints)
BASIC_D_READER_IMPL(DatapickerImage, DatapickerImage::EditorSettings, settings, settings)
BASIC_D_READER_IMPL(DatapickerImage, float, rotationAngle, rotationAngle)
BASIC_D_READER_IMPL(DatapickerImage, DatapickerImage::PointsType, plotPointsType, plotPointsType)
BASIC_D_READER_IMPL(DatapickerImage, int, pointSeparation, pointSeparation)
BASIC_D_READER_IMPL(DatapickerImage, int, minSegmentLength, minSegmentLength)

// symbols
Symbol* DatapickerImage::symbol() const {
	Q_D(const DatapickerImage);
	return d->symbol;
}

BASIC_D_READER_IMPL(DatapickerImage, bool, pointVisibility, pointVisibility)
/* ============================ setter methods and undo commands  for background options  ================= */
void DatapickerImage::setFileName(const QString& fileName) {
	setImage(fileName, embedded());
}

class DatapickerImageSetRelativeFilePathCmd : public StandardSetterCmd<DatapickerImage::Private, bool> {
public:
	DatapickerImageSetRelativeFilePathCmd(DatapickerImage::Private* target, bool newValue, const KLocalizedString& description, QUndoCommand* parent = nullptr)
		: StandardSetterCmd<DatapickerImage::Private, bool>(target, &DatapickerImage::Private::isRelativeFilePath, newValue, description, parent) {
	}
	virtual void finalize() override {
		if (m_target->q->project()) {
			QString filename;
			if (m_target->isRelativeFilePath) {
				// Calculate from absolute to relative
				QFileInfo fi(m_target->q->project()->fileName());
				filename = fi.absoluteDir().relativeFilePath(m_target->fileName);
			} else {
				// Calculate from relative to absolute
				QFileInfo fi(m_target->q->project()->fileName());
				fi.setFile(m_target->fileName);
				filename = fi.absoluteFilePath();
			}
			// setting relative is only possible if the image is not embedded!
			m_target->q->setImage(filename, false);
		}
		Q_EMIT m_target->q->relativeFilePathChanged(m_target->*m_field);
	}
};

void DatapickerImage::setRelativeFilePath(bool relative) {
	Q_D(DatapickerImage);
	if (relative != d->isRelativeFilePath) {
		beginMacro(i18n("%1: upload new image", name()));
		exec(new DatapickerImageSetRelativeFilePathCmd(d, relative, ki18n("%1: upload image")));
		endMacro();
	}
}

void DatapickerImage::setEmbedded(bool embedded) {
	Q_D(DatapickerImage);
	if (embedded != d->embedded) {
		if (embedded)
			setImage(originalPlotImage, fileName(), true);
		else
			setImage(fileName(), false);
	}
}

STD_SETTER_CMD_IMPL_S(DatapickerImage, SetRotationAngle, float, rotationAngle)
void DatapickerImage::setRotationAngle(float angle) {
	Q_D(DatapickerImage);
	if (angle != d->rotationAngle)
		exec(new DatapickerImageSetRotationAngleCmd(d, angle, ki18n("%1: set rotation angle")));
}

STD_SETTER_CMD_IMPL_S(DatapickerImage, SetAxisPoints, DatapickerImage::ReferencePoints, axisPoints)
void DatapickerImage::setAxisPoints(const DatapickerImage::ReferencePoints& points) {
	Q_D(DatapickerImage);
	if (memcmp(&points, &d->axisPoints, sizeof(points)) != 0) // valgrind: Conditional jump or move depends on uninitialised value(s)
		exec(new DatapickerImageSetAxisPointsCmd(d, points, ki18n("%1: set Axis points")));
}

STD_SETTER_CMD_IMPL_F_S(DatapickerImage, SetSettings, DatapickerImage::EditorSettings, settings, discretize)
void DatapickerImage::setSettings(const DatapickerImage::EditorSettings& editorSettings) {
	Q_D(DatapickerImage);
	if (memcmp(&editorSettings, &d->settings, sizeof(editorSettings)) != 0)
		exec(new DatapickerImageSetSettingsCmd(d, editorSettings, ki18n("%1: set editor settings")));
}

STD_SETTER_CMD_IMPL_F_S(DatapickerImage, SetMinSegmentLength, int, minSegmentLength, makeSegments)
void DatapickerImage::setminSegmentLength(const int value) {
	Q_D(DatapickerImage);
	if (d->minSegmentLength != value)
		exec(new DatapickerImageSetMinSegmentLengthCmd(d, value, ki18n("%1: set minimum segment length")));
	;
}

STD_SETTER_CMD_IMPL_F_S(DatapickerImage, SetPointVisibility, bool, pointVisibility, retransform)
void DatapickerImage::setPointVisibility(const bool on) {
	Q_D(DatapickerImage);
	if (on != d->pointVisibility)
		exec(new DatapickerImageSetPointVisibilityCmd(d, on, on ? ki18n("%1: set visible") : ki18n("%1: set invisible")));
}

void DatapickerImage::setPrinting(bool on) const {
	auto points = parentAspect()->children<DatapickerPoint>(ChildIndexFlag::Recursive | ChildIndexFlag::IncludeHidden);
	for (auto* point : points)
		point->setPrinting(on);
}

void DatapickerImage::clearReferencePoints() {
	Q_D(DatapickerImage);
	const auto& points = children<DatapickerPoint>(ChildIndexFlag::IncludeHidden);
	if (!points.isEmpty()) {
		beginMacro(i18n("%1: remove all axis points", name()));

		for (auto* point : points)
			point->remove();
		endMacro();
		d->axisPoints.clearPoints();
		Q_EMIT axisPointsRemoved();
	}
}

void DatapickerImage::setPlotPointsType(const PointsType pointsType) {
	Q_D(DatapickerImage);
	if (d->plotPointsType == pointsType)
		return;

	d->plotPointsType = pointsType;

	if (pointsType == DatapickerImage::PointsType::AxisPoints) {
		// clear image
		clearReferencePoints();
		m_segments->setSegmentsVisible(false);
	} else if (pointsType == DatapickerImage::PointsType::CurvePoints) {
		m_segments->setSegmentsVisible(false);

		// make the reference points non-interactive
		const auto& points = children<DatapickerPoint>(ChildIndexFlag::IncludeHidden);
		for (auto* point : points) {
			auto* item = point->graphicsItem();
			item->setFlag(QGraphicsItem::ItemIsSelectable, false);
			item->setFlag(QGraphicsItem::ItemIsFocusable, false);
			item->setAcceptHoverEvents(false);
			item->setAcceptedMouseButtons(Qt::NoButton);
		}
	} else if (pointsType == DatapickerImage::PointsType::SegmentPoints) {
		d->makeSegments();
		m_segments->setSegmentsVisible(true);
	}
}

void DatapickerImage::setPointSeparation(const int value) {
	Q_D(DatapickerImage);
	d->pointSeparation = value;
}

void DatapickerImage::referencePointSelected(const DatapickerPoint* point) {
	const auto& points = children<DatapickerPoint>(AbstractAspect::ChildIndexFlag::IncludeHidden);
	for (int i = 0; i < points.count(); i++) {
		if (points.at(i) == point) {
			m_currentRefPoint = i;
			Q_EMIT referencePointSelected(i);
			return;
		}
	}
	m_currentRefPoint = -1;
}

void DatapickerImage::childAdded(const AbstractAspect* child) {
	const auto* point = dynamic_cast<const DatapickerPoint*>(child);
	if (point) {
		connect(point, &DatapickerPoint::dataChanged, this, &DatapickerImage::datapickerPointChanged);
		connect(point, &DatapickerPoint::pointSelected, this, QOverload<const DatapickerPoint*>::of(&DatapickerImage::referencePointSelected));
	}
}

void DatapickerImage::childRemoved(const AbstractAspect* child) {
	const auto* point = dynamic_cast<const DatapickerPoint*>(child);
	if (point)
		disconnect(point, nullptr, this, nullptr);
}

bool DatapickerImage::addChild(AbstractAspect* child) {
	if (dynamic_cast<DatapickerPoint*>(child) && children<DatapickerPoint>(AbstractAspect::ChildIndexFlag::IncludeHidden).count() >= 3)
		return false;

	return AbstractAspect::addChild(child);
}

void DatapickerImage::datapickerPointChanged(const DatapickerPoint* point) {
	const auto index = indexOfChild<DatapickerPoint>(point, AbstractAspect::ChildIndexFlag::IncludeHidden);
	assert(index < 3);
	if (index >= 0 && index < 3) {
		auto axisPoints = this->axisPoints();
		axisPoints.scenePos[index].setX(point->position().x());
		axisPoints.scenePos[index].setY(point->position().y());
		setAxisPoints(axisPoints);
	}
}

// ##############################################################################
// ######################  Private implementation ###############################
// ##############################################################################
DatapickerImagePrivate::DatapickerImagePrivate(DatapickerImage* owner)
	: q(owner)
	, pageRect(0, 0, 1000, 1000)
	, m_scene(new QGraphicsScene(pageRect)) {
}

QString DatapickerImagePrivate::name() const {
	return q->name();
}

void DatapickerImagePrivate::retransform() {
	if (q->isLoading())
		return;

	const auto& points = q->children<DatapickerPoint>(AbstractAspect::ChildIndexFlag::IncludeHidden);
	for (auto* point : points)
		point->retransform();
}

bool DatapickerImagePrivate::uploadImage() {
	const bool rc = !q->originalPlotImage.isNull();

	if (rc) {
		// convert the image to 32bit-format if this is not the case yet
		QImage::Format format = q->originalPlotImage.format();
		if (format != QImage::Format_RGB32 && format != QImage::Format_ARGB32 && format != QImage::Format_ARGB32_Premultiplied)
			q->originalPlotImage = q->originalPlotImage.convertToFormat(QImage::Format_RGB32);

		q->processedPlotImage = q->originalPlotImage;
		q->background = ImageEditor::findBackgroundColor(&q->originalPlotImage);
		// upload Histogram
		ImageEditor::uploadHistogram(q->intensityBins, &q->originalPlotImage, q->background, DatapickerImage::ColorAttributes::Intensity);
		ImageEditor::uploadHistogram(q->foregroundBins, &q->originalPlotImage, q->background, DatapickerImage::ColorAttributes::Foreground);
		ImageEditor::uploadHistogram(q->hueBins, &q->originalPlotImage, q->background, DatapickerImage::ColorAttributes::Hue);
		ImageEditor::uploadHistogram(q->saturationBins, &q->originalPlotImage, q->background, DatapickerImage::ColorAttributes::Saturation);
		ImageEditor::uploadHistogram(q->valueBins, &q->originalPlotImage, q->background, DatapickerImage::ColorAttributes::Value);
		discretize();

		// resize the screen
		const auto dpi = (m_scene && !m_scene->views().isEmpty()) ? GuiTools::dpi(m_scene->views().first()) : GuiTools::dpi(nullptr);
		double w = Worksheet::convertToSceneUnits(q->originalPlotImage.width(), Worksheet::Unit::Inch) / dpi.first;
		double h = Worksheet::convertToSceneUnits(q->originalPlotImage.height(), Worksheet::Unit::Inch) / dpi.second;
		m_scene->setSceneRect(0, 0, w, h);
		q->isLoaded = true;
	}
	return rc;
}

void DatapickerImagePrivate::discretize() {
	PERFTRACE(QLatin1String(Q_FUNC_INFO));
	if (plotImageType != DatapickerImage::PlotImageType::ProcessedImage)
		return;

	ImageEditor::discretize(&q->processedPlotImage, &q->originalPlotImage, settings, q->background);

	if (plotPointsType != DatapickerImage::PointsType::SegmentPoints)
		Q_EMIT q->requestUpdate();
	else
		makeSegments();
}

void DatapickerImagePrivate::makeSegments() {
	if (plotPointsType != DatapickerImage::PointsType::SegmentPoints)
		return;

	PERFTRACE(QLatin1String(Q_FUNC_INFO));
	q->m_segments->makeSegments(q->processedPlotImage);
	q->m_segments->setSegmentsVisible(true);
	Q_EMIT q->requestUpdate();
}

DatapickerImagePrivate::~DatapickerImagePrivate() {
	delete m_scene;
}

void DatapickerImagePrivate::updateImage() {
	WAIT_CURSOR;
	q->isLoaded = false;

	if (q->originalPlotImage.isNull()) {
		// hide segments if they are visible
		q->m_segments->setSegmentsVisible(false);
	} else
		uploadImage();

	auto points = q->parentAspect()->children<DatapickerPoint>(AbstractAspect::ChildIndexFlag::Recursive | AbstractAspect::ChildIndexFlag::IncludeHidden);
	if (!points.isEmpty()) {
		for (auto* point : points)
			point->remove();
	}

	Q_EMIT q->requestUpdate();
	Q_EMIT q->requestUpdateActions();
	RESET_CURSOR;
}

// ##############################################################################
// ##################  Serialization/Deserialization  ###########################
// ##############################################################################

//! Save as XML
void DatapickerImage::save(QXmlStreamWriter* writer) const {
	Q_D(const DatapickerImage);
	writer->writeStartElement(QStringLiteral("datapickerImage"));
	writeBasicAttributes(writer);

	// general properties
	writer->writeStartElement(QStringLiteral("general"));
	writer->writeAttribute(QStringLiteral("embedded"), QString::number(d->embedded));
	writer->writeAttribute(QStringLiteral("relativePath"), QString::number(d->isRelativeFilePath));
	writer->writeAttribute(QStringLiteral("fileName"), d->fileName);
	writer->writeAttribute(QStringLiteral("plotPointsType"), QString::number(static_cast<int>(d->plotPointsType)));
	writer->writeAttribute(QStringLiteral("pointVisibility"), QString::number(d->pointVisibility));
	writer->writeEndElement();

	// image data
	if (d->embedded && !originalPlotImage.isNull()) {
		writer->writeStartElement(QStringLiteral("data"));
		QByteArray data;
		QBuffer buffer(&data);
		buffer.open(QIODevice::WriteOnly);
		originalPlotImage.save(&buffer, "PNG");
		writer->writeCharacters(QLatin1String(data.toBase64()));
		writer->writeEndElement();
	}

	writer->writeStartElement(QStringLiteral("axisPoint"));
	writer->writeAttribute(QStringLiteral("graphType"), QString::number(static_cast<int>(d->axisPoints.type)));
	writer->writeAttribute(QStringLiteral("ternaryScale"), QString::number(d->axisPoints.ternaryScale));
	writer->writeAttribute(QStringLiteral("axisPointLogicalX1"), QString::number(d->axisPoints.logicalPos[0].x()));
	writer->writeAttribute(QStringLiteral("axisPointLogicalY1"), QString::number(d->axisPoints.logicalPos[0].y()));
	writer->writeAttribute(QStringLiteral("axisPointLogicalX2"), QString::number(d->axisPoints.logicalPos[1].x()));
	writer->writeAttribute(QStringLiteral("axisPointLogicalY2"), QString::number(d->axisPoints.logicalPos[1].y()));
	writer->writeAttribute(QStringLiteral("axisPointLogicalX3"), QString::number(d->axisPoints.logicalPos[2].x()));
	writer->writeAttribute(QStringLiteral("axisPointLogicalY3"), QString::number(d->axisPoints.logicalPos[2].y()));
	writer->writeAttribute(QStringLiteral("axisPointLogicalZ1"), QString::number(d->axisPoints.logicalPos[0].z()));
	writer->writeAttribute(QStringLiteral("axisPointLogicalZ2"), QString::number(d->axisPoints.logicalPos[1].z()));
	writer->writeAttribute(QStringLiteral("axisPointLogicalZ3"), QString::number(d->axisPoints.logicalPos[2].z()));
	writer->writeAttribute(QStringLiteral("axisPointSceneX1"), QString::number(d->axisPoints.scenePos[0].x()));
	writer->writeAttribute(QStringLiteral("axisPointSceneY1"), QString::number(d->axisPoints.scenePos[0].y()));
	writer->writeAttribute(QStringLiteral("axisPointSceneX2"), QString::number(d->axisPoints.scenePos[1].x()));
	writer->writeAttribute(QStringLiteral("axisPointSceneY2"), QString::number(d->axisPoints.scenePos[1].y()));
	writer->writeAttribute(QStringLiteral("axisPointSceneX3"), QString::number(d->axisPoints.scenePos[2].x()));
	writer->writeAttribute(QStringLiteral("axisPointSceneY3"), QString::number(d->axisPoints.scenePos[2].y()));
	writer->writeEndElement();

	// editor and segment settings
	writer->writeStartElement(QStringLiteral("editorSettings"));
	writer->writeAttribute(QStringLiteral("plotImageType"), QString::number(static_cast<int>(d->plotImageType)));
	writer->writeAttribute(QStringLiteral("rotationAngle"), QString::number(d->rotationAngle));
	writer->writeAttribute(QStringLiteral("minSegmentLength"), QString::number(d->minSegmentLength));
	writer->writeAttribute(QStringLiteral("pointSeparation"), QString::number(d->pointSeparation));
	writer->writeAttribute(QStringLiteral("foregroundThresholdHigh"), QString::number(d->settings.foregroundThresholdHigh));
	writer->writeAttribute(QStringLiteral("foregroundThresholdLow"), QString::number(d->settings.foregroundThresholdLow));
	writer->writeAttribute(QStringLiteral("hueThresholdHigh"), QString::number(d->settings.hueThresholdHigh));
	writer->writeAttribute(QStringLiteral("hueThresholdLow"), QString::number(d->settings.hueThresholdLow));
	writer->writeAttribute(QStringLiteral("intensityThresholdHigh"), QString::number(d->settings.intensityThresholdHigh));
	writer->writeAttribute(QStringLiteral("intensityThresholdLow"), QString::number(d->settings.intensityThresholdLow));
	writer->writeAttribute(QStringLiteral("saturationThresholdHigh"), QString::number(d->settings.saturationThresholdHigh));
	writer->writeAttribute(QStringLiteral("saturationThresholdLow"), QString::number(d->settings.saturationThresholdLow));
	writer->writeAttribute(QStringLiteral("valueThresholdHigh"), QString::number(d->settings.valueThresholdHigh));
	writer->writeAttribute(QStringLiteral("valueThresholdLow"), QString::number(d->settings.valueThresholdLow));
	writer->writeEndElement();

	// Symbols
	d->symbol->save(writer);

	// serialize all children
	for (auto* child : children<AbstractAspect>(ChildIndexFlag::IncludeHidden))
		child->save(writer);

	writer->writeEndElement();
}

//! Load from XML
bool DatapickerImage::load(XmlStreamReader* reader, bool preview) {
	if (!readBasicAttributes(reader))
		return false;

	Q_D(DatapickerImage);
	QXmlStreamAttributes attribs;
	QString str;

	while (!reader->atEnd()) {
		reader->readNext();
		if (reader->isEndElement() && reader->name() == QLatin1String("datapickerImage"))
			break;

		if (!reader->isStartElement())
			continue;

		if (!preview && reader->name() == QLatin1String("general")) {
			attribs = reader->attributes();

			READ_INT_VALUE("embedded", embedded, bool);
			READ_INT_VALUE("relativePath", isRelativeFilePath, bool);
			str = attribs.value(QStringLiteral("fileName")).toString();
			d->fileName = str;

			READ_INT_VALUE("plotPointsType", plotPointsType, DatapickerImage::PointsType);
			READ_INT_VALUE("pointVisibility", pointVisibility, bool);
		} else if (reader->name() == QLatin1String("data")) {
			QByteArray ba = QByteArray::fromBase64(reader->readElementText().toLatin1());
			if (!originalPlotImage.loadFromData(ba))
				reader->raiseWarning(i18n("Failed to read image data"));
		} else if (!preview && reader->name() == QLatin1String("axisPoint")) {
			attribs = reader->attributes();
			READ_INT_VALUE_DIRECT("graphType", d->axisPoints.type, GraphType);
			READ_INT_VALUE("ternaryScale", axisPoints.ternaryScale, int);

			str = attribs.value(QStringLiteral("axisPointLogicalX1")).toString();
			if (str.isEmpty())
				reader->raiseMissingAttributeWarning(QStringLiteral("axisPointLogicalX1"));
			else
				d->axisPoints.logicalPos[0].setX(str.toDouble());

			str = attribs.value(QStringLiteral("axisPointLogicalY1")).toString();
			if (str.isEmpty())
				reader->raiseMissingAttributeWarning(QStringLiteral("axisPointLogicalY1"));
			else
				d->axisPoints.logicalPos[0].setY(str.toDouble());

			str = attribs.value(QStringLiteral("axisPointLogicalZ1")).toString();
			if (str.isEmpty())
				reader->raiseMissingAttributeWarning(QStringLiteral("axisPointLogicalZ1"));
			else
				d->axisPoints.logicalPos[0].setZ(str.toDouble());

			str = attribs.value(QStringLiteral("axisPointLogicalX2")).toString();
			if (str.isEmpty())
				reader->raiseMissingAttributeWarning(QStringLiteral("axisPointLogicalX2"));
			else
				d->axisPoints.logicalPos[1].setX(str.toDouble());

			str = attribs.value(QStringLiteral("axisPointLogicalY2")).toString();
			if (str.isEmpty())
				reader->raiseMissingAttributeWarning(QStringLiteral("axisPointLogicalY2"));
			else
				d->axisPoints.logicalPos[1].setY(str.toDouble());

			str = attribs.value(QStringLiteral("axisPointLogicalZ2")).toString();
			if (str.isEmpty())
				reader->raiseMissingAttributeWarning(QStringLiteral("axisPointLogicalZ2"));
			else
				d->axisPoints.logicalPos[1].setZ(str.toDouble());

			str = attribs.value(QStringLiteral("axisPointLogicalX3")).toString();
			if (str.isEmpty())
				reader->raiseMissingAttributeWarning(QStringLiteral("axisPointLogicalX3"));
			else
				d->axisPoints.logicalPos[2].setX(str.toDouble());

			str = attribs.value(QStringLiteral("axisPointLogicalY3")).toString();
			if (str.isEmpty())
				reader->raiseMissingAttributeWarning(QStringLiteral("axisPointLogicalY3"));
			else
				d->axisPoints.logicalPos[2].setY(str.toDouble());

			str = attribs.value(QStringLiteral("axisPointLogicalZ3")).toString();
			if (str.isEmpty())
				reader->raiseMissingAttributeWarning(QStringLiteral("axisPointLogicalZ3"));
			else
				d->axisPoints.logicalPos[2].setZ(str.toDouble());

			str = attribs.value(QStringLiteral("axisPointSceneX1")).toString();
			if (str.isEmpty())
				reader->raiseMissingAttributeWarning(QStringLiteral("axisPointSceneX1"));
			else
				d->axisPoints.scenePos[0].setX(str.toDouble());

			str = attribs.value(QStringLiteral("axisPointSceneY1")).toString();
			if (str.isEmpty())
				reader->raiseMissingAttributeWarning(QStringLiteral("axisPointSceneY1"));
			else
				d->axisPoints.scenePos[0].setY(str.toDouble());

			str = attribs.value(QStringLiteral("axisPointSceneX2")).toString();
			if (str.isEmpty())
				reader->raiseMissingAttributeWarning(QStringLiteral("axisPointSceneX2"));
			else
				d->axisPoints.scenePos[1].setX(str.toDouble());

			str = attribs.value(QStringLiteral("axisPointSceneY2")).toString();
			if (str.isEmpty())
				reader->raiseMissingAttributeWarning(QStringLiteral("axisPointSceneY2"));
			else
				d->axisPoints.scenePos[1].setY(str.toDouble());

			str = attribs.value(QStringLiteral("axisPointSceneX3")).toString();
			if (str.isEmpty())
				reader->raiseMissingAttributeWarning(QStringLiteral("axisPointSceneX3"));
			else
				d->axisPoints.scenePos[2].setX(str.toDouble());

			str = attribs.value(QStringLiteral("axisPointSceneY3")).toString();
			if (str.isEmpty())
				reader->raiseMissingAttributeWarning(QStringLiteral("axisPointSceneY3"));
			else
				d->axisPoints.scenePos[2].setY(str.toDouble());

		} else if (!preview && reader->name() == QLatin1String("editorSettings")) {
			attribs = reader->attributes();

			READ_INT_VALUE("plotImageType", plotImageType, DatapickerImage::PlotImageType);
			READ_DOUBLE_VALUE("rotationAngle", rotationAngle);
			READ_INT_VALUE("minSegmentLength", minSegmentLength, int);
			READ_INT_VALUE("pointSeparation", pointSeparation, int);
			READ_INT_VALUE("foregroundThresholdHigh", settings.foregroundThresholdHigh, int);
			READ_INT_VALUE("foregroundThresholdLow", settings.foregroundThresholdLow, int);
			READ_INT_VALUE("hueThresholdHigh", settings.hueThresholdHigh, int);
			READ_INT_VALUE("hueThresholdLow", settings.hueThresholdLow, int);
			READ_INT_VALUE("intensityThresholdHigh", settings.intensityThresholdHigh, int);
			READ_INT_VALUE("intensityThresholdLow", settings.intensityThresholdLow, int);
			READ_INT_VALUE("saturationThresholdHigh", settings.saturationThresholdHigh, int);
			READ_INT_VALUE("saturationThresholdLow", settings.saturationThresholdLow, int);
			READ_INT_VALUE("valueThresholdHigh", settings.valueThresholdHigh, int);
			READ_INT_VALUE("valueThresholdLow", settings.valueThresholdLow, int);
		} else if (!preview && reader->name() == QLatin1String("symbolProperties")) {
			// old serialization that was used before the switch to Symbol::load().
			// in the old serialization the symbol properties and "point visibility" where saved
			// under "symbolProperties".
			attribs = reader->attributes();

			str = attribs.value(QStringLiteral("pointRotationAngle")).toString();
			if (str.isEmpty())
				reader->raiseMissingAttributeWarning(QStringLiteral("pointRotationAngle"));
			else
				d->symbol->setRotationAngle(str.toDouble());

			str = attribs.value(QStringLiteral("pointOpacity")).toString();
			if (str.isEmpty())
				reader->raiseMissingAttributeWarning(QStringLiteral("pointOpacity"));
			else
				d->symbol->setOpacity(str.toDouble());

			str = attribs.value(QStringLiteral("pointSize")).toString();
			if (str.isEmpty())
				reader->raiseMissingAttributeWarning(QStringLiteral("pointSize"));
			else
				d->symbol->setSize(str.toDouble());

			str = attribs.value(QStringLiteral("pointStyle")).toString();
			if (str.isEmpty())
				reader->raiseMissingAttributeWarning(QStringLiteral("pointStyle"));
			else
				d->symbol->setStyle(static_cast<Symbol::Style>(str.toInt()));

			// brush
			QBrush brush;
			str = attribs.value(QStringLiteral("brush_style")).toString();
			if (str.isEmpty())
				reader->raiseMissingAttributeWarning(QStringLiteral("brush_style"));
			else
				brush.setStyle(static_cast<Qt::BrushStyle>(str.toInt()));

			QColor color;
			str = attribs.value(QStringLiteral("brush_color_r")).toString();
			if (str.isEmpty())
				reader->raiseMissingAttributeWarning(QStringLiteral("brush_color_r"));
			else
				color.setRed(str.toInt());

			str = attribs.value(QStringLiteral("brush_color_g")).toString();
			if (str.isEmpty())
				reader->raiseMissingAttributeWarning(QStringLiteral("brush_color_g"));
			else
				color.setGreen(str.toInt());

			str = attribs.value(QStringLiteral("brush_color_b")).toString();
			if (str.isEmpty())
				reader->raiseMissingAttributeWarning(QStringLiteral("brush_color_b"));
			else
				color.setBlue(str.toInt());

			brush.setColor(color);
			d->symbol->setBrush(brush);

			// pen
			QPen pen;
			str = attribs.value(QStringLiteral("style")).toString();
			if (str.isEmpty())
				reader->raiseMissingAttributeWarning(QStringLiteral("style"));
			else
				pen.setStyle(static_cast<Qt::PenStyle>(str.toInt()));

			str = attribs.value(QStringLiteral("color_r")).toString();
			if (str.isEmpty())
				reader->raiseMissingAttributeWarning(QStringLiteral("color_r"));
			else
				color.setRed(str.toInt());

			str = attribs.value(QStringLiteral("color_g")).toString();
			if (str.isEmpty())
				reader->raiseMissingAttributeWarning(QStringLiteral("color_g"));
			else
				color.setGreen(str.toInt());

			str = attribs.value(QStringLiteral("color_b")).toString();
			if (str.isEmpty())
				reader->raiseMissingAttributeWarning(QStringLiteral("color_b"));
			else
				color.setBlue(str.toInt());

			pen.setColor(color);

			str = attribs.value(QStringLiteral("width")).toString();
			if (str.isEmpty())
				reader->raiseMissingAttributeWarning(QStringLiteral("width"));
			else
				pen.setWidthF(str.toDouble());

			d->symbol->setPen(pen);

			READ_INT_VALUE("pointVisibility", pointVisibility, bool);
		} else if (!preview && reader->name() == QLatin1String("symbols")) {
			d->symbol->load(reader, preview);
		} else if (reader->name() == QLatin1String("datapickerPoint")) {
			auto* datapickerPoint = new DatapickerPoint(QString());
			if (!datapickerPoint->load(reader, preview)) {
				delete datapickerPoint;
				return false;
			} else {
				datapickerPoint->setHidden(true);
				datapickerPoint->setIsReferencePoint(true);
				addChild(datapickerPoint);
			}
		} else { // unknown element
			reader->raiseUnknownElementWarning();
			if (!reader->skipToEndElement())
				return false;
		}
	}

	// No undo redo
	if (originalPlotImage.isNull())
		originalPlotImage.load(d->fileName);
	d->uploadImage();
	d->retransform();
	return true;
}
