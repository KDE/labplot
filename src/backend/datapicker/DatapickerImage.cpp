/***************************************************************************
    File                 : DatapickerImage.cpp
    Project              : LabPlot
    Description          : Worksheet for Datapicker
    --------------------------------------------------------------------
    Copyright            : (C) 2015 by Ankit Wagadre (wagadre.ankit@gmail.com)
    Copyright            : (C) 2015-2016 by Alexander Semke (alexander.semke@web.de)

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
#include "backend/worksheet/Worksheet.h"
#include "commonfrontend/datapicker/DatapickerImageView.h"
#include "kdefrontend/worksheet/ExportWorksheetDialog.h"

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
DatapickerImage::DatapickerImage(const QString& name, bool loading) : AbstractPart(name),
	isLoaded(false),
	foregroundBins( new int[ImageEditor::colorAttributeMax(Foreground) + 1]),
	hueBins( new int[ImageEditor::colorAttributeMax(Hue) + 1]),
	saturationBins( new int[ImageEditor::colorAttributeMax(Saturation) + 1]),
	valueBins( new int[ImageEditor::colorAttributeMax(Value) + 1]),
	intensityBins( new int[ImageEditor::colorAttributeMax(Intensity) + 1]),
	m_magnificationWindow(nullptr),
	d(new DatapickerImagePrivate(this)),
	m_view(nullptr),
	m_segments(new Segments(this)) {

	if (!loading)
		init();
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
	KConfigGroup group = config.group( "DatapickerImage" );
	d->fileName = group.readEntry("FileName", QString());
	d->rotationAngle = group.readEntry("RotationAngle", 0.0);
	d->minSegmentLength = group.readEntry("MinSegmentLength", 30);
	d->pointSeparation = group.readEntry("PointSeparation", 30);
	d->axisPoints.type = (DatapickerImage::GraphType) group.readEntry("GraphType", (int) DatapickerImage::Cartesian);
	d->axisPoints.ternaryScale = group.readEntry("TernaryScale", 1);
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
	d->plotPointsType = (DatapickerImage::PointsType) group.readEntry("PlotPointsType", (int) DatapickerImage::AxisPoints);
	d->plotImageType = DatapickerImage::OriginalImage;
	// point properties
	d->pointStyle = (Symbol::Style)group.readEntry("PointStyle", (int)Symbol::Cross);
	d->pointSize = group.readEntry("Size", Worksheet::convertToSceneUnits(7, Worksheet::Point));
	d->pointRotationAngle = group.readEntry("Rotation", 0.0);
	d->pointOpacity = group.readEntry("Opacity", 1.0);
	d->pointBrush.setStyle( (Qt::BrushStyle)group.readEntry("FillingStyle", (int)Qt::NoBrush) );
	d->pointBrush.setColor( group.readEntry("FillingColor", QColor(Qt::black)) );
	d->pointPen.setStyle( (Qt::PenStyle)group.readEntry("BorderStyle", (int)Qt::SolidLine) );
	d->pointPen.setColor( group.readEntry("BorderColor", QColor(Qt::red)) );
	d->pointPen.setWidthF( group.readEntry("BorderWidth", Worksheet::convertToSceneUnits(1, Worksheet::Point)) );
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
	ExportWorksheetDialog* dlg = new ExportWorksheetDialog(m_view);
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
	QPrintDialog* dlg = new QPrintDialog(&printer, m_view);
	bool ret;
	dlg->setWindowTitle(i18nc("@title:window", "Print Datapicker Image"));
	if ( (ret = (dlg->exec() == QDialog::Accepted)) )
		m_view->print(&printer);

	delete dlg;
	return ret;
}

bool DatapickerImage::printPreview() const {
	QPrintPreviewDialog* dlg = new QPrintPreviewDialog(m_view);
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
	if (d->plotImageType == DatapickerImage::ProcessedImage)
		d->discretize();

	emit requestUpdate();
}

DatapickerImage::PlotImageType DatapickerImage::plotImageType() {
	return d->plotImageType;
}

void DatapickerImage::initSceneParameters() {
	setRotationAngle(0.0);
	setminSegmentLength(30);
	setPointSeparation(30);

	ReferencePoints axisPoints = d->axisPoints;
	axisPoints.ternaryScale = 1;
	axisPoints.type = DatapickerImage::Cartesian;
	setAxisPoints(axisPoints);

	EditorSettings settings;
	settings.foregroundThresholdHigh = 90;
	settings.foregroundThresholdLow = 30;
	settings.hueThresholdHigh = 360;
	settings.hueThresholdLow = 0;
	settings.intensityThresholdHigh = 100;
	settings.intensityThresholdLow = 20;
	settings.saturationThresholdHigh = 100;
	settings.saturationThresholdLow = 30;
	settings.valueThresholdHigh = 90;
	settings.valueThresholdLow = 30;
	setSettings(settings);

	DatapickerImage::PointsType plotPointsType = DatapickerImage::AxisPoints;
	setPlotPointsType(plotPointsType);
}

/* =============================== getter methods for background options ================================= */
CLASS_D_READER_IMPL(DatapickerImage, QString, fileName, fileName)
CLASS_D_READER_IMPL(DatapickerImage, DatapickerImage::ReferencePoints, axisPoints, axisPoints)
CLASS_D_READER_IMPL(DatapickerImage, DatapickerImage::EditorSettings, settings, settings)
BASIC_D_READER_IMPL(DatapickerImage, float, rotationAngle, rotationAngle)
BASIC_D_READER_IMPL(DatapickerImage, DatapickerImage::PointsType, plotPointsType, plotPointsType)
BASIC_D_READER_IMPL(DatapickerImage, int, pointSeparation, pointSeparation)
BASIC_D_READER_IMPL(DatapickerImage, int, minSegmentLength, minSegmentLength)
BASIC_D_READER_IMPL(DatapickerImage, Symbol::Style, pointStyle, pointStyle)
BASIC_D_READER_IMPL(DatapickerImage, qreal, pointOpacity, pointOpacity)
BASIC_D_READER_IMPL(DatapickerImage, qreal, pointRotationAngle, pointRotationAngle)
BASIC_D_READER_IMPL(DatapickerImage, qreal, pointSize, pointSize)
CLASS_D_READER_IMPL(DatapickerImage, QBrush, pointBrush, pointBrush)
CLASS_D_READER_IMPL(DatapickerImage, QPen, pointPen, pointPen)
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

STD_SETTER_CMD_IMPL_F_S(DatapickerImage, SetPointStyle, Symbol::Style, pointStyle, retransform)
void DatapickerImage::setPointStyle(Symbol::Style newStyle) {
	if (newStyle != d->pointStyle)
		exec(new DatapickerImageSetPointStyleCmd(d, newStyle, ki18n("%1: set point's style")));
}


STD_SETTER_CMD_IMPL_F_S(DatapickerImage, SetPointSize, qreal, pointSize, retransform)
void DatapickerImage::setPointSize(qreal value) {
	if (!qFuzzyCompare(1 + value, 1 + d->pointSize))
		exec(new DatapickerImageSetPointSizeCmd(d, value, ki18n("%1: set point's size")));
}

STD_SETTER_CMD_IMPL_F_S(DatapickerImage, SetPointRotationAngle, qreal, pointRotationAngle, retransform)
void DatapickerImage::setPointRotationAngle(qreal angle) {
	if (!qFuzzyCompare(1 + angle, 1 + d->pointRotationAngle))
		exec(new DatapickerImageSetPointRotationAngleCmd(d, angle, ki18n("%1: rotate point")));
}

STD_SETTER_CMD_IMPL_F_S(DatapickerImage, SetPointBrush, QBrush, pointBrush, retransform)
void DatapickerImage::setPointBrush(const QBrush& newBrush) {
	if (newBrush != d->pointBrush)
		exec(new DatapickerImageSetPointBrushCmd(d, newBrush, ki18n("%1: set point's filling")));
}

STD_SETTER_CMD_IMPL_F_S(DatapickerImage, SetPointPen, QPen, pointPen, retransform)
void DatapickerImage::setPointPen(const QPen &newPen) {
	if (newPen != d->pointPen)
		exec(new DatapickerImageSetPointPenCmd(d, newPen, ki18n("%1: set outline style")));
}

STD_SETTER_CMD_IMPL_F_S(DatapickerImage, SetPointOpacity, qreal, pointOpacity, retransform)
void DatapickerImage::setPointOpacity(qreal newOpacity) {
	if (newOpacity != d->pointOpacity)
		exec(new DatapickerImageSetPointOpacityCmd(d, newOpacity, ki18n("%1: set point's opacity")));
}

STD_SETTER_CMD_IMPL_F_S(DatapickerImage, SetPointVisibility, bool, pointVisibility, retransform)
void DatapickerImage::setPointVisibility(const bool on) {
	if (on != d->pointVisibility)
		exec(new DatapickerImageSetPointVisibilityCmd(d, on, on ? ki18n("%1: set visible") : ki18n("%1: set invisible")));
}

void DatapickerImage::setPrinting(bool on) const {
	QVector<DatapickerPoint*> childPoints = parentAspect()->children<DatapickerPoint>(AbstractAspect::Recursive | AbstractAspect::IncludeHidden);
	for (auto* point : childPoints)
		point->setPrinting(on);
}

void DatapickerImage::setPlotPointsType(const PointsType pointsType) {
	d->plotPointsType = pointsType;

	if (pointsType == DatapickerImage::AxisPoints) {
		//clear image
		int childCount = this->childCount<DatapickerPoint>(AbstractAspect::IncludeHidden);
		if (childCount) {
			beginMacro(i18n("%1: remove all axis points", name()));
			QVector<DatapickerPoint*> childrenPoints = children<DatapickerPoint>(AbstractAspect::IncludeHidden);
			for (auto* point : childrenPoints)
				point->remove();
			endMacro();
		}
		m_segments->setSegmentsVisible(false);
	} else if (pointsType==DatapickerImage::CurvePoints)
		m_segments->setSegmentsVisible(false);
	else if (pointsType==DatapickerImage::SegmentPoints) {
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
DatapickerImagePrivate::DatapickerImagePrivate(DatapickerImage *owner):q(owner),
	pageRect(0, 0, 1500, 1500),
	m_scene(new QGraphicsScene(pageRect)) {
}

QString DatapickerImagePrivate::name() const {
	return q->name();
}

void DatapickerImagePrivate::retransform() {
	QVector<DatapickerPoint*> childrenPoints = q->children<DatapickerPoint>(AbstractAspect::IncludeHidden);
	for (auto* point : childrenPoints)
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
		ImageEditor::uploadHistogram(q->intensityBins, &q->originalPlotImage, q->background, DatapickerImage::Intensity);
		ImageEditor::uploadHistogram(q->foregroundBins, &q->originalPlotImage, q->background, DatapickerImage::Foreground);
		ImageEditor::uploadHistogram(q->hueBins, &q->originalPlotImage, q->background, DatapickerImage::Hue);
		ImageEditor::uploadHistogram(q->saturationBins, &q->originalPlotImage, q->background, DatapickerImage::Saturation);
		ImageEditor::uploadHistogram(q->valueBins, &q->originalPlotImage, q->background, DatapickerImage::Value);
		discretize();

		//resize the screen
		double w = Worksheet::convertToSceneUnits(q->originalPlotImage.width(), Worksheet::Inch)/QApplication::desktop()->physicalDpiX();
		double h = Worksheet::convertToSceneUnits(q->originalPlotImage.height(), Worksheet::Inch)/QApplication::desktop()->physicalDpiX();
		m_scene->setSceneRect(0, 0, w, h);
		q->isLoaded = true;
	}
	return rc;
}

void DatapickerImagePrivate::discretize() {
	if (plotImageType != DatapickerImage::ProcessedImage)
		return;

	ImageEditor::discretize(&q->processedPlotImage, &q->originalPlotImage, settings, q->background);

	if (plotPointsType != DatapickerImage::SegmentPoints)
		emit q->requestUpdate();
	else
		makeSegments();
}

void DatapickerImagePrivate::makeSegments() {
	if (plotPointsType != DatapickerImage::SegmentPoints)
		return;

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

	if ( !address.isEmpty() ) {
		if (uploadImage(address)) {
			q->initSceneParameters();
			fileName = address;
		}
	} else {
		//hide segments if they are visible
		q->m_segments->setSegmentsVisible(false);
	}

	QVector<DatapickerPoint*> childPoints = q->parentAspect()->children<DatapickerPoint>(AbstractAspect::Recursive | AbstractAspect::IncludeHidden);
	if (childPoints.count()) {
		for (auto* point : childPoints)
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
	writeCommentElement(writer);
	//general properties
	writer->writeStartElement( "general" );
	writer->writeAttribute( "fileName", d->fileName );
	writer->writeAttribute( "plotPointsType", QString::number(d->plotPointsType) );
	writer->writeEndElement();

	writer->writeStartElement( "axisPoint" );
	writer->writeAttribute( "graphType", QString::number(d->axisPoints.type) );
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
	writer->writeAttribute( "plotImageType", QString::number(d->plotImageType) );
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

	//symbol properties
	writer->writeStartElement( "symbolProperties" );
	writer->writeAttribute( "pointRotationAngle", QString::number(d->pointRotationAngle) );
	writer->writeAttribute( "pointOpacity", QString::number(d->pointOpacity) );
	writer->writeAttribute( "pointSize", QString::number(d->pointSize) );
	writer->writeAttribute( "pointStyle", QString::number(d->pointStyle) );
	writer->writeAttribute( "pointVisibility", QString::number(d->pointVisibility) );
	WRITE_QBRUSH(d->pointBrush);
	WRITE_QPEN(d->pointPen);
	writer->writeEndElement();

	//serialize all children
	for (auto* child : children<AbstractAspect>(IncludeHidden))
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

		if (reader->name() == "comment") {
			if (!readCommentElement(reader)) return false;
		} else if (!preview && reader->name() == "general") {
			attribs = reader->attributes();

			str = attribs.value("fileName").toString();
			d->fileName = str;

			str = attribs.value("plotPointsType").toString();
			if(str.isEmpty())
				reader->raiseWarning(attributeWarning.subs("plotPointsType").toString());
			else
				d->plotPointsType = DatapickerImage::PointsType(str.toInt());

		} else if (!preview && reader->name() == "axisPoint") {
			attribs = reader->attributes();

			str = attribs.value("graphType").toString();
			if(str.isEmpty())
				reader->raiseWarning(attributeWarning.subs("graphType").toString());
			else
				d->axisPoints.type = DatapickerImage::GraphType(str.toInt());

			str = attribs.value("ternaryScale").toString();
			if(str.isEmpty())
				reader->raiseWarning(attributeWarning.subs("ternaryScale").toString());
			else
				d->axisPoints.ternaryScale = str.toDouble();

			str = attribs.value("axisPointLogicalX1").toString();
			if(str.isEmpty())
				reader->raiseWarning(attributeWarning.subs("axisPointLogicalX1").toString());
			else
				d->axisPoints.logicalPos[0].setX(str.toDouble());

			str = attribs.value("axisPointLogicalY1").toString();
			if(str.isEmpty())
				reader->raiseWarning(attributeWarning.subs("axisPointLogicalY1").toString());
			else
				d->axisPoints.logicalPos[0].setY(str.toDouble());

			str = attribs.value("axisPointLogicalZ1").toString();
			if(str.isEmpty())
				reader->raiseWarning(attributeWarning.subs("axisPointLogicalZ1").toString());
			else
				d->axisPoints.logicalPos[0].setZ(str.toDouble());

			str = attribs.value("axisPointLogicalX2").toString();
			if(str.isEmpty())
				reader->raiseWarning(attributeWarning.subs("axisPointLogicalX2").toString());
			else
				d->axisPoints.logicalPos[1].setX(str.toDouble());

			str = attribs.value("axisPointLogicalY2").toString();
			if(str.isEmpty())
				reader->raiseWarning(attributeWarning.subs("axisPointLogicalY2").toString());
			else
				d->axisPoints.logicalPos[1].setY(str.toDouble());

			str = attribs.value("axisPointLogicalZ2").toString();
			if(str.isEmpty())
				reader->raiseWarning(attributeWarning.subs("axisPointLogicalZ2").toString());
			else
				d->axisPoints.logicalPos[1].setZ(str.toDouble());

			str = attribs.value("axisPointLogicalX3").toString();
			if(str.isEmpty())
				reader->raiseWarning(attributeWarning.subs("axisPointLogicalX3").toString());
			else
				d->axisPoints.logicalPos[2].setX(str.toDouble());

			str = attribs.value("axisPointLogicalY3").toString();
			if(str.isEmpty())
				reader->raiseWarning(attributeWarning.subs("axisPointLogicalY3").toString());
			else
				d->axisPoints.logicalPos[2].setY(str.toDouble());

			str = attribs.value("axisPointLogicalZ3").toString();
			if(str.isEmpty())
				reader->raiseWarning(attributeWarning.subs("axisPointLogicalZ3").toString());
			else
				d->axisPoints.logicalPos[2].setZ(str.toDouble());

			str = attribs.value("axisPointSceneX1").toString();
			if(str.isEmpty())
				reader->raiseWarning(attributeWarning.subs("axisPointSceneX1").toString());
			else
				d->axisPoints.scenePos[0].setX(str.toDouble());

			str = attribs.value("axisPointSceneY1").toString();
			if(str.isEmpty())
				reader->raiseWarning(attributeWarning.subs("axisPointSceneY1").toString());
			else
				d->axisPoints.scenePos[0].setY(str.toDouble());

			str = attribs.value("axisPointSceneX2").toString();
			if(str.isEmpty())
				reader->raiseWarning(attributeWarning.subs("axisPointSceneX2").toString());
			else
				d->axisPoints.scenePos[1].setX(str.toDouble());

			str = attribs.value("axisPointSceneY2").toString();
			if(str.isEmpty())
				reader->raiseWarning(attributeWarning.subs("axisPointSceneY2").toString());
			else
				d->axisPoints.scenePos[1].setY(str.toDouble());

			str = attribs.value("axisPointSceneX3").toString();
			if(str.isEmpty())
				reader->raiseWarning(attributeWarning.subs("axisPointSceneX3").toString());
			else
				d->axisPoints.scenePos[2].setX(str.toDouble());

			str = attribs.value("axisPointSceneY3").toString();
			if(str.isEmpty())
				reader->raiseWarning(attributeWarning.subs("axisPointSceneY3").toString());
			else
				d->axisPoints.scenePos[2].setY(str.toDouble());

		} else if (!preview && reader->name() == "editorSettings") {
			attribs = reader->attributes();

			str = attribs.value("plotImageType").toString();
			if(str.isEmpty())
				reader->raiseWarning(attributeWarning.subs("plotImageType").toString());
			else
				d->plotImageType = DatapickerImage::PlotImageType(str.toInt());

			str = attribs.value("rotationAngle").toString();
			if(str.isEmpty())
				reader->raiseWarning(attributeWarning.subs("rotationAngle").toString());
			else
				d->rotationAngle = str.toFloat();

			str = attribs.value("minSegmentLength").toString();
			if(str.isEmpty())
				reader->raiseWarning(attributeWarning.subs("minSegmentLength").toString());
			else
				d->minSegmentLength = str.toInt();

			str = attribs.value("pointSeparation").toString();
			if(str.isEmpty())
				reader->raiseWarning(attributeWarning.subs("pointSeparation").toString());
			else
				d->pointSeparation = str.toInt();

			str = attribs.value("foregroundThresholdHigh").toString();
			if(str.isEmpty())
				reader->raiseWarning(attributeWarning.subs("foregroundThresholdHigh").toString());
			else
				d->settings.foregroundThresholdHigh = str.toInt();

			str = attribs.value("foregroundThresholdLow").toString();
			if(str.isEmpty())
				reader->raiseWarning(attributeWarning.subs("foregroundThresholdLow").toString());
			else
				d->settings.foregroundThresholdLow = str.toInt();

			str = attribs.value("hueThresholdHigh").toString();
			if(str.isEmpty())
				reader->raiseWarning(attributeWarning.subs("hueThresholdHigh").toString());
			else
				d->settings.hueThresholdHigh = str.toInt();

			str = attribs.value("hueThresholdLow").toString();
			if(str.isEmpty())
				reader->raiseWarning(attributeWarning.subs("hueThresholdLow").toString());
			else
				d->settings.hueThresholdLow = str.toInt();

			str = attribs.value("intensityThresholdHigh").toString();
			if(str.isEmpty())
				reader->raiseWarning(attributeWarning.subs("intensityThresholdHigh").toString());
			else
				d->settings.intensityThresholdHigh = str.toInt();

			str = attribs.value("intensityThresholdLow").toString();
			if(str.isEmpty())
				reader->raiseWarning(attributeWarning.subs("intensityThresholdLow").toString());
			else
				d->settings.intensityThresholdLow = str.toInt();

			str = attribs.value("saturationThresholdHigh").toString();
			if(str.isEmpty())
				reader->raiseWarning(attributeWarning.subs("saturationThresholdHigh").toString());
			else
				d->settings.saturationThresholdHigh = str.toInt();

			str = attribs.value("saturationThresholdLow").toString();
			if(str.isEmpty())
				reader->raiseWarning(attributeWarning.subs("saturationThresholdLow").toString());
			else
				d->settings.saturationThresholdLow = str.toInt();

			str = attribs.value("valueThresholdHigh").toString();
			if(str.isEmpty())
				reader->raiseWarning(attributeWarning.subs("valueThresholdHigh").toString());
			else
				d->settings.valueThresholdHigh = str.toInt();

			str = attribs.value("valueThresholdLow").toString();
			if(str.isEmpty())
				reader->raiseWarning(attributeWarning.subs("valueThresholdLow").toString());
			else
				d->settings.valueThresholdLow = str.toInt();

		} else if(!preview && reader->name() == "symbolProperties") {
			attribs = reader->attributes();

			str = attribs.value("pointRotationAngle").toString();
			if(str.isEmpty())
				reader->raiseWarning(attributeWarning.subs("pointRotationAngle").toString());
			else
				d->pointRotationAngle = str.toFloat();

			str = attribs.value("pointOpacity").toString();
			if(str.isEmpty())
				reader->raiseWarning(attributeWarning.subs("pointOpacity").toString());
			else
				d->pointOpacity = str.toFloat();

			str = attribs.value("pointSize").toString();
			if(str.isEmpty())
				reader->raiseWarning(attributeWarning.subs("pointSize").toString());
			else
				d->pointSize = str.toFloat();

			str = attribs.value("pointStyle").toString();
			if(str.isEmpty())
				reader->raiseWarning(attributeWarning.subs("pointStyle").toString());
			else
				d->pointStyle = (Symbol::Style)str.toInt();

			str = attribs.value("pointVisibility").toString();
			if(str.isEmpty())
				reader->raiseWarning(attributeWarning.subs("pointVisibility").toString());
			else
				d->pointVisibility = (bool)str.toInt();

			READ_QBRUSH(d->pointBrush);
			READ_QPEN(d->pointPen);
		} else if(reader->name() == "datapickerPoint") {
			DatapickerPoint* datapickerPoint = new DatapickerPoint("");
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
