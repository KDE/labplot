/***************************************************************************
    File                 : DatapickerImage.cpp
    Project              : LabPlot
    Description          : Worksheet for Datapicker
    --------------------------------------------------------------------
    Copyright            : (C) 2015 by Ankit Wagadre (wagadre.ankit@gmail.com)
    Copyright            : (C) 2015-2021 by Alexander Semke (alexander.semke@web.de)

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

#include "DatapickerImage.h"
#include "DatapickerImagePrivate.h"
#include "backend/datapicker/ImageEditor.h"
#include "backend/lib/commandtemplates.h"
#include "backend/lib/XmlStreamReader.h"
#include "backend/datapicker/DatapickerPoint.h"
#include "backend/datapicker/Segments.h"
#include "backend/worksheet/plots/cartesian/Symbol.h"
#include "backend/worksheet/Worksheet.h"
#include "commonfrontend/datapicker/DatapickerImageView.h"
#include "kdefrontend/worksheet/ExportWorksheetDialog.h"
#include "backend/lib/trace.h"

#include <QDesktopWidget>
#include <QGraphicsScene>
#include <QMenu>
#include <QPrinter>
#include <QPrintDialog>
#include <QPrintPreviewDialog>

#include <KConfig>
#include <KConfigGroup>
#include <KLocalizedString>

/**
 * \class DatapickerImage
 * \brief container to open image/plot.
 *
 * Top-level container for DatapickerPoint.
 *
 * * \ingroup datapicker
 */
DatapickerImage::DatapickerImage(const QString& name, bool loading) :
	AbstractPart(name, AspectType::DatapickerImage),
	foregroundBins( new int[ImageEditor::colorAttributeMax(ColorAttributes::Foreground) + 1]),
	hueBins( new int[ImageEditor::colorAttributeMax(ColorAttributes::Hue) + 1]),
	saturationBins( new int[ImageEditor::colorAttributeMax(ColorAttributes::Saturation) + 1]),
	valueBins( new int[ImageEditor::colorAttributeMax(ColorAttributes::Value) + 1]),
	intensityBins( new int[ImageEditor::colorAttributeMax(ColorAttributes::Intensity) + 1]),
	d(new DatapickerImagePrivate(this)),
	m_segments(new Segments(this)) {

	if (!loading)
		init();
	else {
		d->symbol = new Symbol(QString());
		addChild(d->symbol);
		d->symbol->setHidden(true);
		connect(d->symbol, &Symbol::updateRequested, [=]{d->retransform();});
		connect(d->symbol, &Symbol::updatePixmapRequested, [=]{d->retransform();});
	}
}

DatapickerImage::~DatapickerImage() {
	delete [] hueBins;
	delete [] saturationBins;
	delete [] valueBins;
	delete [] intensityBins;
	delete [] foregroundBins;
	delete m_segments;
	delete d;
}

void DatapickerImage::init() {
	KConfig config;
	KConfigGroup group = config.group("DatapickerImage");

	//general properties
	d->fileName = group.readEntry("FileName", QString());
	d->rotationAngle = group.readEntry("RotationAngle", 0.0);
	d->minSegmentLength = group.readEntry("MinSegmentLength", 30);
	d->pointSeparation = group.readEntry("PointSeparation", 30);
	d->axisPoints.type = (DatapickerImage::GraphType) group.readEntry("GraphType", static_cast<int>(DatapickerImage::GraphType::Cartesian));
	d->axisPoints.ternaryScale = group.readEntry("TernaryScale", 1);

	//edit image settings
	d->plotImageType = DatapickerImage::PlotImageType::OriginalImage;
	d->settings.foregroundThresholdHigh = group.readEntry("ForegroundThresholdHigh", 90);
	d->settings.foregroundThresholdLow = group.readEntry("ForegroundThresholdLow", 30);
	d->settings.hueThresholdHigh = group.readEntry("HueThresholdHigh", 360);
	d->settings.hueThresholdLow = group.readEntry("HueThresholdLow", 0);
	d->settings.intensityThresholdHigh = group.readEntry("IntensityThresholdHigh", 100);
	d->settings.intensityThresholdLow = group.readEntry("IntensityThresholdLow", 20);
	d->settings.saturationThresholdHigh = group.readEntry("SaturationThresholdHigh", 100);
	d->settings.saturationThresholdLow = group.readEntry("SaturationThresholdLow", 30);
	d->settings.valueThresholdHigh = group.readEntry("ValueThresholdHigh", 90);
	d->settings.valueThresholdLow = group.readEntry("ValueThresholdLow", 30);

	// reference point symbol properties
	d->symbol = new Symbol(QString());
	addChild(d->symbol);
	d->symbol->setHidden(true);
	connect(d->symbol, &Symbol::updateRequested, [=]{d->retransform();});
	connect(d->symbol, &Symbol::updatePixmapRequested, [=]{d->retransform();});
	d->symbol->init(group);
	d->pointVisibility = group.readEntry("PointVisibility", true);
}

/*!
    Returns an icon to be used in the project explorer.
*/
QIcon DatapickerImage::icon() const {
	return QIcon::fromTheme("image-x-generic");
}

/*!
    Return a new context menu
*/
QMenu* DatapickerImage::createContextMenu() {
	QMenu* menu = new QMenu(nullptr);
	emit requestProjectContextMenu(menu);
	return menu;
}

void DatapickerImage::createContextMenu(QMenu* menu) {
	emit requestProjectContextMenu(menu);
}

//! Construct a primary view on me.
/**
 * This method may be called multiple times during the life time of an Aspect, or it might not get
 * called at all. Aspects must not depend on the existence of a view for their operation.
 */
QWidget* DatapickerImage::view() const {
	if (!m_partView) {
		m_view = new DatapickerImageView(const_cast<DatapickerImage *>(this));
		m_partView = m_view;
		connect(m_view, &DatapickerImageView::statusInfo, this, &DatapickerImage::statusInfo);
	}
	return m_partView;
}

bool DatapickerImage::exportView() const {
	auto* dlg = new ExportWorksheetDialog(m_view);
	dlg->setFileName(name());
	bool ret;
	if ( (ret = (dlg->exec() == QDialog::Accepted)) ) {
		const QString path = dlg->path();
		const WorksheetView::ExportFormat format = dlg->exportFormat();
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
	if ( (ret = (dlg->exec() == QDialog::Accepted)) )
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
		emit childAspectSelectedInView(this);
	else
		emit childAspectDeselectedInView(this);
}

void DatapickerImage::setSegmentsHoverEvent(const bool on) {
	m_segments->setAcceptHoverEvents(on);
}

QGraphicsScene* DatapickerImage::scene() const {
	return d->m_scene;
}

QRectF DatapickerImage::pageRect() const {
	return d->m_scene->sceneRect();
}

void DatapickerImage::setPlotImageType(const DatapickerImage::PlotImageType type) {
	d->plotImageType = type;
	if (d->plotImageType == DatapickerImage::PlotImageType::ProcessedImage)
		d->discretize();

	emit requestUpdate();
}

DatapickerImage::PlotImageType DatapickerImage::plotImageType() {
	return d->plotImageType;
}

/* =============================== getter methods for background options ================================= */
BASIC_D_READER_IMPL(DatapickerImage, QString, fileName, fileName)
BASIC_D_READER_IMPL(DatapickerImage, DatapickerImage::ReferencePoints, axisPoints, axisPoints)
BASIC_D_READER_IMPL(DatapickerImage, DatapickerImage::EditorSettings, settings, settings)
BASIC_D_READER_IMPL(DatapickerImage, float, rotationAngle, rotationAngle)
BASIC_D_READER_IMPL(DatapickerImage, DatapickerImage::PointsType, plotPointsType, plotPointsType)
BASIC_D_READER_IMPL(DatapickerImage, int, pointSeparation, pointSeparation)
BASIC_D_READER_IMPL(DatapickerImage, int, minSegmentLength, minSegmentLength)

//symbols
Symbol* DatapickerImage::symbol() const {
// 	Q_D(const DatapickerImage);
	return d->symbol;
}

BASIC_D_READER_IMPL(DatapickerImage, bool, pointVisibility, pointVisibility)
/* ============================ setter methods and undo commands  for background options  ================= */
STD_SETTER_CMD_IMPL_F_S(DatapickerImage, SetFileName, QString, fileName, updateFileName)
void DatapickerImage::setFileName(const QString& fileName) {
	if (fileName!= d->fileName) {
		beginMacro(i18n("%1: upload new image", name()));
		exec(new DatapickerImageSetFileNameCmd(d, fileName, ki18n("%1: upload image")));
		endMacro();
	}
}

STD_SETTER_CMD_IMPL_S(DatapickerImage, SetRotationAngle, float, rotationAngle)
void DatapickerImage::setRotationAngle(float angle) {
	if (angle != d->rotationAngle)
		exec(new DatapickerImageSetRotationAngleCmd(d, angle, ki18n("%1: set rotation angle")));
}

STD_SETTER_CMD_IMPL_S(DatapickerImage, SetAxisPoints, DatapickerImage::ReferencePoints, axisPoints)
void DatapickerImage::setAxisPoints(const DatapickerImage::ReferencePoints& points) {
	if (memcmp(&points, &d->axisPoints, sizeof(points)) != 0)
		exec(new DatapickerImageSetAxisPointsCmd(d, points, ki18n("%1: set Axis points")));
}

STD_SETTER_CMD_IMPL_F_S(DatapickerImage, SetSettings, DatapickerImage::EditorSettings, settings, discretize)
void DatapickerImage::setSettings(const DatapickerImage::EditorSettings& editorSettings) {
	if (memcmp(&editorSettings, &d->settings, sizeof(editorSettings)) != 0)
		exec(new DatapickerImageSetSettingsCmd(d, editorSettings, ki18n("%1: set editor settings")));
}

STD_SETTER_CMD_IMPL_F_S(DatapickerImage, SetMinSegmentLength, int, minSegmentLength, makeSegments)
void DatapickerImage::setminSegmentLength(const int value) {
	if (d->minSegmentLength != value)
		exec(new DatapickerImageSetMinSegmentLengthCmd(d, value, ki18n("%1: set minimum segment length")));        ;
}

STD_SETTER_CMD_IMPL_F_S(DatapickerImage, SetPointVisibility, bool, pointVisibility, retransform)
void DatapickerImage::setPointVisibility(const bool on) {
	if (on != d->pointVisibility)
		exec(new DatapickerImageSetPointVisibilityCmd(d, on, on ? ki18n("%1: set visible") : ki18n("%1: set invisible")));
}

void DatapickerImage::setPrinting(bool on) const {
	auto points = parentAspect()->children<DatapickerPoint>(ChildIndexFlag::Recursive | ChildIndexFlag::IncludeHidden);
	for (auto* point : points)
		point->setPrinting(on);
}

void DatapickerImage::setPlotPointsType(const PointsType pointsType) {
	if (d->plotPointsType == pointsType)
		return;

	d->plotPointsType = pointsType;

	if (pointsType == DatapickerImage::PointsType::AxisPoints) {
		//clear image
		auto points = children<DatapickerPoint>(ChildIndexFlag::IncludeHidden);
		if (!points.isEmpty()) {
			beginMacro(i18n("%1: remove all axis points", name()));

			for (auto* point : points)
				point->remove();
			endMacro();
		}
		m_segments->setSegmentsVisible(false);
	} else if (pointsType == DatapickerImage::PointsType::CurvePoints)
		m_segments->setSegmentsVisible(false);
	else if (pointsType == DatapickerImage::PointsType::SegmentPoints) {
		d->makeSegments();
		m_segments->setSegmentsVisible(true);
	}
}

void DatapickerImage::setPointSeparation(const int value) {
	d->pointSeparation = value;
}

//##############################################################################
//######################  Private implementation ###############################
//##############################################################################
DatapickerImagePrivate::DatapickerImagePrivate(DatapickerImage *owner) : q(owner),
	pageRect(0, 0, 1000, 1000),
	m_scene(new QGraphicsScene(pageRect)) {
}

QString DatapickerImagePrivate::name() const {
	return q->name();
}

void DatapickerImagePrivate::retransform() {
	auto points = q->children<DatapickerPoint>(AbstractAspect::ChildIndexFlag::IncludeHidden);
	for (auto* point : points)
		point->retransform();
}

bool DatapickerImagePrivate::uploadImage(const QString& address) {
	bool rc = q->originalPlotImage.load(address);
	if (rc) {
		//convert the image to 32bit-format if this is not the case yet
		QImage::Format format = q->originalPlotImage.format();
		if (format != QImage::Format_RGB32 && format != QImage::Format_ARGB32 && format != QImage::Format_ARGB32_Premultiplied)
			q->originalPlotImage = q->originalPlotImage.convertToFormat(QImage::Format_RGB32);

		q->processedPlotImage = q->originalPlotImage;
		q->background = ImageEditor::findBackgroundColor(&q->originalPlotImage);
		//upload Histogram
		ImageEditor::uploadHistogram(q->intensityBins, &q->originalPlotImage, q->background, DatapickerImage::ColorAttributes::Intensity);
		ImageEditor::uploadHistogram(q->foregroundBins, &q->originalPlotImage, q->background, DatapickerImage::ColorAttributes::Foreground);
		ImageEditor::uploadHistogram(q->hueBins, &q->originalPlotImage, q->background, DatapickerImage::ColorAttributes::Hue);
		ImageEditor::uploadHistogram(q->saturationBins, &q->originalPlotImage, q->background, DatapickerImage::ColorAttributes::Saturation);
		ImageEditor::uploadHistogram(q->valueBins, &q->originalPlotImage, q->background, DatapickerImage::ColorAttributes::Value);
		discretize();

		//resize the screen
		double w = Worksheet::convertToSceneUnits(q->originalPlotImage.width(), Worksheet::Unit::Inch)/QApplication::desktop()->physicalDpiX();
		double h = Worksheet::convertToSceneUnits(q->originalPlotImage.height(), Worksheet::Unit::Inch)/QApplication::desktop()->physicalDpiX();
		m_scene->setSceneRect(0, 0, w, h);
		q->isLoaded = true;
	}
	return rc;
}

void DatapickerImagePrivate::discretize() {
	PERFTRACE("DatapickerImagePrivate::discretize()");
	if (plotImageType != DatapickerImage::PlotImageType::ProcessedImage)
		return;

	ImageEditor::discretize(&q->processedPlotImage, &q->originalPlotImage, settings, q->background);

	if (plotPointsType != DatapickerImage::PointsType::SegmentPoints)
		emit q->requestUpdate();
	else
		makeSegments();
}

void DatapickerImagePrivate::makeSegments() {
	if (plotPointsType != DatapickerImage::PointsType::SegmentPoints)
		return;

	PERFTRACE("DatapickerImagePrivate::makeSegments()");
	q->m_segments->makeSegments(q->processedPlotImage);
	q->m_segments->setSegmentsVisible(true);
	emit q->requestUpdate();
}

DatapickerImagePrivate::~DatapickerImagePrivate() {
	delete m_scene;
}

void DatapickerImagePrivate::updateFileName() {
	WAIT_CURSOR;
	q->isLoaded = false;
	const QString& address = fileName.trimmed();

	if (!address.isEmpty()) {
		if (uploadImage(address))
			fileName = address;
	} else {
		//hide segments if they are visible
		q->m_segments->setSegmentsVisible(false);
	}

	auto points = q->parentAspect()->children<DatapickerPoint>(AbstractAspect::ChildIndexFlag::Recursive | AbstractAspect::ChildIndexFlag::IncludeHidden);
	if (!points.isEmpty()) {
		for (auto* point : points)
			point->remove();
	}

	emit q->requestUpdate();
	emit q->requestUpdateActions();
	RESET_CURSOR;
}

//##############################################################################
//##################  Serialization/Deserialization  ###########################
//##############################################################################

//! Save as XML
void DatapickerImage::save(QXmlStreamWriter* writer) const {
	writer->writeStartElement( "datapickerImage" );
	writeBasicAttributes(writer);

	//general properties
	writer->writeStartElement( "general" );
	writer->writeAttribute( "fileName", d->fileName );
	writer->writeAttribute( "plotPointsType", QString::number(static_cast<int>(d->plotPointsType)) );
	writer->writeAttribute( "pointVisibility", QString::number(d->pointVisibility) );
	writer->writeEndElement();

	writer->writeStartElement( "axisPoint" );
	writer->writeAttribute( "graphType", QString::number(static_cast<int>(d->axisPoints.type)) );
	writer->writeAttribute( "ternaryScale", QString::number(d->axisPoints.ternaryScale) );
	writer->writeAttribute( "axisPointLogicalX1", QString::number(d->axisPoints.logicalPos[0].x()) );
	writer->writeAttribute( "axisPointLogicalY1", QString::number(d->axisPoints.logicalPos[0].y()) );
	writer->writeAttribute( "axisPointLogicalX2", QString::number(d->axisPoints.logicalPos[1].x()) );
	writer->writeAttribute( "axisPointLogicalY2", QString::number(d->axisPoints.logicalPos[1].y()) );
	writer->writeAttribute( "axisPointLogicalX3", QString::number(d->axisPoints.logicalPos[2].x()) );
	writer->writeAttribute( "axisPointLogicalY3", QString::number(d->axisPoints.logicalPos[2].y()) );
	writer->writeAttribute( "axisPointLogicalZ1", QString::number(d->axisPoints.logicalPos[0].z()) );
	writer->writeAttribute( "axisPointLogicalZ2", QString::number(d->axisPoints.logicalPos[1].z()) );
	writer->writeAttribute( "axisPointLogicalZ3", QString::number(d->axisPoints.logicalPos[2].z()) );
	writer->writeAttribute( "axisPointSceneX1", QString::number(d->axisPoints.scenePos[0].x()) );
	writer->writeAttribute( "axisPointSceneY1", QString::number(d->axisPoints.scenePos[0].y()) );
	writer->writeAttribute( "axisPointSceneX2", QString::number(d->axisPoints.scenePos[1].x()) );
	writer->writeAttribute( "axisPointSceneY2", QString::number(d->axisPoints.scenePos[1].y()) );
	writer->writeAttribute( "axisPointSceneX3", QString::number(d->axisPoints.scenePos[2].x()) );
	writer->writeAttribute( "axisPointSceneY3", QString::number(d->axisPoints.scenePos[2].y()) );
	writer->writeEndElement();

	//editor and segment settings
	writer->writeStartElement( "editorSettings" );
	writer->writeAttribute( "plotImageType", QString::number(static_cast<int>(d->plotImageType)) );
	writer->writeAttribute( "rotationAngle", QString::number(d->rotationAngle) );
	writer->writeAttribute( "minSegmentLength", QString::number(d->minSegmentLength) );
	writer->writeAttribute( "pointSeparation", QString::number(d->pointSeparation) );
	writer->writeAttribute( "foregroundThresholdHigh", QString::number(d->settings.foregroundThresholdHigh) );
	writer->writeAttribute( "foregroundThresholdLow", QString::number(d->settings.foregroundThresholdLow) );
	writer->writeAttribute( "hueThresholdHigh", QString::number(d->settings.hueThresholdHigh) );
	writer->writeAttribute( "hueThresholdLow", QString::number(d->settings.hueThresholdLow) );
	writer->writeAttribute( "intensityThresholdHigh", QString::number(d->settings.intensityThresholdHigh) );
	writer->writeAttribute( "intensityThresholdLow", QString::number(d->settings.intensityThresholdLow) );
	writer->writeAttribute( "saturationThresholdHigh", QString::number(d->settings.saturationThresholdHigh) );
	writer->writeAttribute( "saturationThresholdLow", QString::number(d->settings.saturationThresholdLow) );
	writer->writeAttribute( "valueThresholdHigh", QString::number(d->settings.valueThresholdHigh) );
	writer->writeAttribute( "valueThresholdLow", QString::number(d->settings.valueThresholdLow) );
	writer->writeEndElement();

	//Symbols
	d->symbol->save(writer);

	//serialize all children
	for (auto* child : children<AbstractAspect>(ChildIndexFlag::IncludeHidden))
		child->save(writer);

	writer->writeEndElement();
}

//! Load from XML
bool DatapickerImage::load(XmlStreamReader* reader, bool preview) {
	if (!readBasicAttributes(reader))
		return false;

	KLocalizedString attributeWarning = ki18n("Attribute '%1' missing or empty, default value is used");
	QXmlStreamAttributes attribs;
	QString str;

	while (!reader->atEnd()) {
		reader->readNext();
		if (reader->isEndElement() && reader->name() == "datapickerImage")
			break;

		if (!reader->isStartElement())
			continue;

		if (!preview && reader->name() == "general") {
			attribs = reader->attributes();

			str = attribs.value("fileName").toString();
			d->fileName = str;

			READ_INT_VALUE("plotPointsType", plotPointsType, DatapickerImage::PointsType);
			READ_INT_VALUE("pointVisibility", pointVisibility, bool);
		} else if (!preview && reader->name() == "axisPoint") {
			attribs = reader->attributes();

			READ_INT_VALUE("graphType", axisPoints.type, DatapickerImage::GraphType);
			READ_INT_VALUE("ternaryScale", axisPoints.ternaryScale, int);

			str = attribs.value("axisPointLogicalX1").toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.subs("axisPointLogicalX1").toString());
			else
				d->axisPoints.logicalPos[0].setX(str.toDouble());

			str = attribs.value("axisPointLogicalY1").toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.subs("axisPointLogicalY1").toString());
			else
				d->axisPoints.logicalPos[0].setY(str.toDouble());

			str = attribs.value("axisPointLogicalZ1").toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.subs("axisPointLogicalZ1").toString());
			else
				d->axisPoints.logicalPos[0].setZ(str.toDouble());

			str = attribs.value("axisPointLogicalX2").toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.subs("axisPointLogicalX2").toString());
			else
				d->axisPoints.logicalPos[1].setX(str.toDouble());

			str = attribs.value("axisPointLogicalY2").toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.subs("axisPointLogicalY2").toString());
			else
				d->axisPoints.logicalPos[1].setY(str.toDouble());

			str = attribs.value("axisPointLogicalZ2").toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.subs("axisPointLogicalZ2").toString());
			else
				d->axisPoints.logicalPos[1].setZ(str.toDouble());

			str = attribs.value("axisPointLogicalX3").toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.subs("axisPointLogicalX3").toString());
			else
				d->axisPoints.logicalPos[2].setX(str.toDouble());

			str = attribs.value("axisPointLogicalY3").toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.subs("axisPointLogicalY3").toString());
			else
				d->axisPoints.logicalPos[2].setY(str.toDouble());

			str = attribs.value("axisPointLogicalZ3").toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.subs("axisPointLogicalZ3").toString());
			else
				d->axisPoints.logicalPos[2].setZ(str.toDouble());

			str = attribs.value("axisPointSceneX1").toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.subs("axisPointSceneX1").toString());
			else
				d->axisPoints.scenePos[0].setX(str.toDouble());

			str = attribs.value("axisPointSceneY1").toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.subs("axisPointSceneY1").toString());
			else
				d->axisPoints.scenePos[0].setY(str.toDouble());

			str = attribs.value("axisPointSceneX2").toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.subs("axisPointSceneX2").toString());
			else
				d->axisPoints.scenePos[1].setX(str.toDouble());

			str = attribs.value("axisPointSceneY2").toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.subs("axisPointSceneY2").toString());
			else
				d->axisPoints.scenePos[1].setY(str.toDouble());

			str = attribs.value("axisPointSceneX3").toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.subs("axisPointSceneX3").toString());
			else
				d->axisPoints.scenePos[2].setX(str.toDouble());

			str = attribs.value("axisPointSceneY3").toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.subs("axisPointSceneY3").toString());
			else
				d->axisPoints.scenePos[2].setY(str.toDouble());

		} else if (!preview && reader->name() == "editorSettings") {
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
		} else if (!preview && reader->name() == "symbolProperties") {
			//old serialization that was used before the switch to Symbol::load().
			//in the old serialization the symbol properties and "point visibility" where saved
			//under "symbolProperties".
			attribs = reader->attributes();

			str = attribs.value("pointRotationAngle").toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.subs("pointRotationAngle").toString());
			else
				d->symbol->setRotationAngle(str.toDouble());

			str = attribs.value("pointOpacity").toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.subs("pointOpacity").toString());
			else
				d->symbol->setOpacity(str.toDouble());

			str = attribs.value("pointSize").toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.subs("pointSize").toString());
			else
				d->symbol->setSize(str.toDouble());

			str = attribs.value("pointStyle").toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.subs("pointStyle").toString());
			else
				d->symbol->setStyle(static_cast<Symbol::Style>(str.toInt()));

			//brush
			QBrush brush;
			str = attribs.value("brush_style").toString();
			if(str.isEmpty())
				reader->raiseWarning(attributeWarning.subs("brush_style").toString());
			else
				brush.setStyle( static_cast<Qt::BrushStyle>(str.toInt()) );

			QColor color;
			str = attribs.value("brush_color_r").toString();
			if(str.isEmpty())
				reader->raiseWarning(attributeWarning.subs("brush_color_r").toString());
			else
				color.setRed(str.toInt());

			str = attribs.value("brush_color_g").toString();
			if(str.isEmpty())
				reader->raiseWarning(attributeWarning.subs("brush_color_g").toString());
			else
				color.setGreen(str.toInt());

			str = attribs.value("brush_color_b").toString();
			if(str.isEmpty())
				reader->raiseWarning(attributeWarning.subs("brush_color_b").toString());
			else
				color.setBlue(str.toInt());

			brush.setColor(color);
			d->symbol->setBrush(brush);

			//pen
			QPen pen;
			str = attribs.value("style").toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.subs("style").toString());
			else
				pen.setStyle( static_cast<Qt::PenStyle>(str.toInt()) );

			str = attribs.value("color_r").toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.subs("color_r").toString());
			else
				color.setRed( str.toInt() );

			str = attribs.value("color_g").toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.subs("color_g").toString());
			else
				color.setGreen( str.toInt() );

			str = attribs.value("color_b").toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.subs("color_b").toString());
			else
				color.setBlue( str.toInt() );

			pen.setColor(color);

			str = attribs.value("width").toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.subs("width").toString());
			else
				pen.setWidthF( str.toDouble() );

			d->symbol->setPen(pen);

			READ_INT_VALUE("pointVisibility", pointVisibility, bool);
		} else if (!preview && reader->name() == "symbols") {
			d->symbol->load(reader, preview);
		} else if (reader->name() == "datapickerPoint") {
			auto* datapickerPoint = new DatapickerPoint(QString());
			datapickerPoint->setHidden(true);
			if (!datapickerPoint->load(reader, preview)) {
				delete datapickerPoint;
				return false;
			} else
				addChild(datapickerPoint);
		} else { // unknown element
			reader->raiseWarning(i18n("unknown element '%1'", reader->name().toString()));
			if (!reader->skipToEndElement()) return false;
		}
	}

	d->uploadImage(d->fileName);
	d->retransform();
	return true;
}
