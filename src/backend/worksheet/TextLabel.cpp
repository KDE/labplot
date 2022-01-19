/*
    File                 : TextLabel.cpp
    Project              : LabPlot
    Description          : Text label supporting reach text and latex formatting
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2009 Tilman Benkert <thzs@gmx.net>
    SPDX-FileCopyrightText: 2012-2021 Alexander Semke <alexander.semke@web.de>
    SPDX-FileCopyrightText: 2019 Stefan Gerlach <stefan.gerlach@uni.kn>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "TextLabel.h"
#include "Worksheet.h"
#include "TextLabelPrivate.h"
#include "backend/lib/commandtemplates.h"
#include "backend/lib/XmlStreamReader.h"
#include "backend/worksheet/plots/PlotArea.h"
#include "backend/worksheet/plots/cartesian/CartesianCoordinateSystem.h"
#include "backend/worksheet/plots/cartesian/CartesianPlot.h"
#include "backend/core/Project.h"
#include "kdefrontend/GuiTools.h"

#include <QApplication>
#include <QBuffer>
#include <QDesktopWidget>
#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QKeyEvent>
#include <QMenu>
#include <QPainter>
#include <QTextCharFormat>
#include <QTextDocument>
#include <QTextCursor>
#include <QtConcurrent/QtConcurrentRun>

#include <QIcon>
#include <KConfig>
#include <KConfigGroup>
#include <KLocalizedString>

#ifdef HAVE_DISCOUNT
extern "C" {
#include <mkdio.h>
}
#endif

class ScaledTextItem : public QGraphicsTextItem {
public:
	ScaledTextItem(QGraphicsItem* parent = nullptr) : QGraphicsTextItem(parent) {

	}

	void setScaleFactor(double scaleFactor) {
		m_scaleFactor = scaleFactor;
	}

	void setRotationAngle(double rotationAngle) {
		m_rotationAngle = rotationAngle;
	}

protected:
	void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override {
		painter->rotate(-m_rotationAngle);
		painter->scale(m_scaleFactor, m_scaleFactor);
		painter->translate(QPointF(-boundingRect().width()/2, -boundingRect().height()/2));
		QGraphicsTextItem::paint(painter, option, widget);
	}

private:
	double m_scaleFactor = 1.;
	double m_rotationAngle = 0.;
};


/**
 * \class TextLabel
 * \brief A label supporting rendering of html- and tex-formatted texts.
 *
 * The label is aligned relative to the specified position.
 * The position can be either specified by providing the x- and y- coordinates
 * in parent's coordinate system, or by specifying one of the predefined position
 * flags (\c HorizontalPosition, \c VerticalPosition).
 */

TextLabel::TextLabel(const QString& name, Type type)
	: WorksheetElement(name, new TextLabelPrivate(this), AspectType::TextLabel), m_type(type) {

	init();
}

TextLabel::TextLabel(const QString &name, TextLabelPrivate *dd, Type type)
	: WorksheetElement(name, dd, AspectType::TextLabel), m_type(type) {

	init();
}

TextLabel::TextLabel(const QString &name, CartesianPlot* plot, Type type):
	WorksheetElement(name, new TextLabelPrivate(this), AspectType::TextLabel),
	m_type(type) {

	m_plot = plot;
	cSystem = dynamic_cast<const CartesianCoordinateSystem*>(m_plot->coordinateSystem(m_cSystemIndex));
	init();
}

void TextLabel::init() {
	Q_D(TextLabel);

	QString groupName;
	switch (m_type) {
	case Type::General:
		groupName = "TextLabel";
		break;
	case Type::PlotTitle:
		groupName = "PlotTitle";
		break;
	case Type::AxisTitle:
		groupName = "AxisTitle";
		break;
	case Type::PlotLegendTitle:
		groupName = "PlotLegendTitle";
		break;
	case Type::InfoElementLabel:
		groupName = "InfoElementLabel";
	}

	const KConfig config;
	DEBUG("	config has group \"" << STDSTRING(groupName) << "\": " << config.hasGroup(groupName));
	// group is always valid if you call config.group(..;
	KConfigGroup group;
	if (config.hasGroup(groupName))
		group = config.group(groupName);

	d->position.point = QPointF(0, 0);
	if (m_type == Type::PlotTitle || m_type == Type::PlotLegendTitle) {
		d->position.verticalPosition = WorksheetElement::VerticalPosition::Top;
		d->position.horizontalPosition = WorksheetElement::HorizontalPosition::Center;
		d->verticalAlignment = WorksheetElement::VerticalAlignment::Top;
	} else if (m_type == Type::AxisTitle) {
		d->position.horizontalPosition = WorksheetElement::HorizontalPosition::Center;
		d->position.verticalPosition = WorksheetElement::VerticalPosition::Center;
	}

	// read settings from config if group exists
	if (group.isValid()) {
		//properties common to all types
		d->textWrapper.mode = static_cast<TextLabel::Mode>(group.readEntry("Mode", static_cast<int>(d->textWrapper.mode)));
		d->teXFont.setFamily(group.readEntry("TeXFontFamily", d->teXFont.family()));
		d->teXFont.setPointSize(group.readEntry("TeXFontSize", d->teXFont.pointSize()));
		d->fontColor = group.readEntry("TeXFontColor", d->fontColor);
		d->backgroundColor = group.readEntry("TeXBackgroundColor", d->backgroundColor);
		d->rotationAngle = group.readEntry("Rotation", d->rotationAngle);

		//border
		d->borderShape = (TextLabel::BorderShape)group.readEntry("BorderShape", (int)d->borderShape);
		d->borderPen = QPen(group.readEntry("BorderColor", d->borderPen.color()),
		                    group.readEntry("BorderWidth", d->borderPen.width()),
		                    (Qt::PenStyle) group.readEntry("BorderStyle", (int)(d->borderPen.style())));
		d->borderOpacity = group.readEntry("BorderOpacity", d->borderOpacity);

		//position and alignment relevant properties
		d->position.point.setX( group.readEntry("PositionXValue", d->position.point.x()) );
		d->position.point.setY( group.readEntry("PositionYValue", d->position.point.y()) );
		d->position.horizontalPosition = (HorizontalPosition) group.readEntry("PositionX", (int)d->position.horizontalPosition);
		d->position.verticalPosition = (VerticalPosition) group.readEntry("PositionY", (int)d->position.verticalPosition);
		d->horizontalAlignment = (WorksheetElement::HorizontalAlignment) group.readEntry("HorizontalAlignment", static_cast<int>(d->horizontalAlignment));
		d->verticalAlignment = (WorksheetElement::VerticalAlignment) group.readEntry("VerticalAlignment", static_cast<int>(d->verticalAlignment));
		if (cSystem)
			d->positionLogical = cSystem->mapSceneToLogical(d->position.point, AbstractCoordinateSystem::MappingFlag::SuppressPageClipping);
	}

	DEBUG("CHECK: default/run time image resolution: " << d->teXImageResolution << '/' << QApplication::desktop()->physicalDpiX());

	connect(&d->teXImageFutureWatcher, &QFutureWatcher<QByteArray>::finished, this, &TextLabel::updateTeXImage);
}

//no need to delete the d-pointer here - it inherits from QGraphicsItem
//and is deleted during the cleanup in QGraphicsScene
TextLabel::~TextLabel() = default;

QGraphicsItem* TextLabel::graphicsItem() const {
	return d_ptr;
}

/*!
 * \brief TextLabel::setParentGraphicsItem
 * Sets the parent graphicsitem, needed for binding to coord
 * \param item parent graphicsitem
 */
void TextLabel::setParentGraphicsItem(QGraphicsItem* item) {
	Q_D(TextLabel);
	d->setParentItem(item);
}

void TextLabel::setZoomFactor(double factor) {
	Q_D(TextLabel);
	d->setZoomFactor(factor);
}

void TextLabel::retransform() {
	Q_D(TextLabel);
	d->retransform();
}

void TextLabel::handleResize(double horizontalRatio, double verticalRatio, bool /*pageResize*/) {
	DEBUG(Q_FUNC_INFO);
	Q_D(TextLabel);

	double ratio = 0;
	if (horizontalRatio > 1.0 || verticalRatio > 1.0)
		ratio = qMax(horizontalRatio, verticalRatio);
	else
		ratio = qMin(horizontalRatio, verticalRatio);

	d->teXFont.setPointSizeF(d->teXFont.pointSizeF() * ratio);
	d->updateText();

	//TODO: doesn't seem to work
	QTextDocument doc;
	doc.setHtml(d->textWrapper.text);
	QTextCursor cursor(&doc);
	cursor.select(QTextCursor::Document);
	QTextCharFormat fmt = cursor.charFormat();
	QFont font = fmt.font();
	font.setPointSizeF(font.pointSizeF() * ratio);
	fmt.setFont(font);
	cursor.setCharFormat(fmt);
}

/*!
	Returns an icon to be used in the project explorer.
*/
QIcon TextLabel::icon() const {
	return  QIcon::fromTheme("draw-text");
}

QMenu* TextLabel::createContextMenu() {
	QMenu* menu = WorksheetElement::createContextMenu();
	QAction* firstAction = menu->actions().at(1); //skip the first action because of the "title-action"

	if (!visibilityAction) {
		visibilityAction = new QAction(i18n("Visible"), this);
		visibilityAction->setCheckable(true);
		connect(visibilityAction, &QAction::triggered, this, &TextLabel::visibilityChanged);
	}

	visibilityAction->setChecked(isVisible());
	menu->insertAction(firstAction, visibilityAction);
	menu->insertSeparator(firstAction);

	return menu;
}

/* ============================ getter methods ================= */
BASIC_SHARED_D_READER_IMPL(TextLabel, TextLabel::TextWrapper, text, textWrapper)
BASIC_SHARED_D_READER_IMPL(TextLabel, QColor, fontColor, fontColor)
BASIC_SHARED_D_READER_IMPL(TextLabel, QColor, backgroundColor, backgroundColor)
BASIC_SHARED_D_READER_IMPL(TextLabel, QFont, teXFont, teXFont)
BASIC_SHARED_D_READER_IMPL(TextLabel, TextLabel::BorderShape, borderShape, borderShape)
BASIC_SHARED_D_READER_IMPL(TextLabel, QPen, borderPen, borderPen)
BASIC_SHARED_D_READER_IMPL(TextLabel, qreal, borderOpacity, borderOpacity)

/* ============================ setter methods and undo commands ================= */
STD_SETTER_CMD_IMPL_F_S(TextLabel, SetText, TextLabel::TextWrapper, textWrapper, updateText)
void TextLabel::setText(const TextWrapper &textWrapper) {
	Q_D(TextLabel);
	//DEBUG(Q_FUNC_INFO << ", text = " << STDSTRING(textWrapper.text) << std::endl)
	//DEBUG(Q_FUNC_INFO << ", old/new mode = " << (int)d->textWrapper.mode << " " << (int)textWrapper.mode)

	if ( (textWrapper.text != d->textWrapper.text) || (textWrapper.mode != d->textWrapper.mode)
	        || ((d->textWrapper.allowPlaceholder || textWrapper.allowPlaceholder) && (textWrapper.textPlaceholder != d->textWrapper.textPlaceholder)) ||
			textWrapper.allowPlaceholder != d->textWrapper.allowPlaceholder) {
		bool changePos = d->textWrapper.text.isEmpty();
		if (textWrapper.mode == TextLabel::Mode::Text) {
			//QDEBUG("\n" << Q_FUNC_INFO << ", OLD TEXT =" << d->textWrapper.text << ", font color =" << d->fontColor)
			//DEBUG("\n" << Q_FUNC_INFO << ", NEW TEXT = " << STDSTRING(textWrapper.text) << std::endl)

			TextWrapper tw = textWrapper;
			if (d->textWrapper.mode != TextLabel::Mode::Text) {	// restore text formatting
				QTextEdit te(d->textWrapper.text);
				te.selectAll();
				te.setText(textWrapper.text);
				te.selectAll();
				te.setTextColor(d->fontColor);

				tw.text = te.toHtml();
			}

			exec(new TextLabelSetTextCmd(d, tw, ki18n("%1: set label text")));
		} else
			exec(new TextLabelSetTextCmd(d, textWrapper, ki18n("%1: set label text")));
		// If previously the text was empty, the bounding rect is zero
		// therefore the alignment did not work properly.
		// If text is added, the bounding rectangle is updated
		// and then the position must be changed to consider alignment
		if (changePos)
			d->updatePosition();
	}
}

STD_SETTER_CMD_IMPL_F_S(TextLabel, SetPlaceholderText, TextLabel::TextWrapper, textWrapper, updateText)
void TextLabel::setPlaceholderText(const TextWrapper &textWrapper) {
	Q_D(TextLabel);
	if ( (textWrapper.textPlaceholder != d->textWrapper.textPlaceholder) || (textWrapper.mode != d->textWrapper.mode) )
		exec(new TextLabelSetPlaceholderTextCmd(d, textWrapper, ki18n("%1: set label placeholdertext")));
}

STD_SETTER_CMD_IMPL_F_S(TextLabel, SetTeXFont, QFont, teXFont, updateText)
void TextLabel::setTeXFont(const QFont& font) {
	Q_D(TextLabel);
	if (font != d->teXFont)
		exec(new TextLabelSetTeXFontCmd(d, font, ki18n("%1: set TeX main font")));
}

STD_SETTER_CMD_IMPL_F_S(TextLabel, SetTeXFontColor, QColor, fontColor, updateText)
void TextLabel::setFontColor(const QColor color) {
	Q_D(TextLabel);
	if (color != d->fontColor)
		exec(new TextLabelSetTeXFontColorCmd(d, color, ki18n("%1: set font color")));
}

STD_SETTER_CMD_IMPL_F_S(TextLabel, SetTeXBackgroundColor, QColor, backgroundColor, updateText)
void TextLabel::setBackgroundColor(const QColor color) {
	QDEBUG(Q_FUNC_INFO << ", color = " << color)
	Q_D(TextLabel);
	if (color != d->backgroundColor)
		exec(new TextLabelSetTeXBackgroundColorCmd(d, color, ki18n("%1: set background color")));
}

//Border
STD_SETTER_CMD_IMPL_F_S(TextLabel, SetBorderShape, TextLabel::BorderShape, borderShape, updateBorder)
void TextLabel::setBorderShape(TextLabel::BorderShape shape) {
	Q_D(TextLabel);
	if (shape != d->borderShape)
		exec(new TextLabelSetBorderShapeCmd(d, shape, ki18n("%1: set border shape")));
}

STD_SETTER_CMD_IMPL_F_S(TextLabel, SetBorderPen, QPen, borderPen, update)
void TextLabel::setBorderPen(const QPen &pen) {
	Q_D(TextLabel);
	if (pen != d->borderPen)
		exec(new TextLabelSetBorderPenCmd(d, pen, ki18n("%1: set border")));
}

STD_SETTER_CMD_IMPL_F_S(TextLabel, SetBorderOpacity, qreal, borderOpacity, update)
void TextLabel::setBorderOpacity(qreal opacity) {
	Q_D(TextLabel);
	if (opacity != d->borderOpacity)
		exec(new TextLabelSetBorderOpacityCmd(d, opacity, ki18n("%1: set border opacity")));
}

//misc

QRectF TextLabel::size() {
	Q_D(TextLabel);
	return d->size();
}

QPointF TextLabel::findNearestGluePoint(QPointF scenePoint) {
	Q_D(TextLabel);
	return d->findNearestGluePoint(scenePoint);
}

TextLabel::GluePoint TextLabel::gluePointAt(int index) {
	Q_D(TextLabel);
	return d->gluePointAt(index);
}

int TextLabel::gluePointCount() {
	Q_D(const TextLabel);
	return d->m_gluePoints.length();
}

void TextLabel::updateTeXImage() {
	Q_D(TextLabel);
	d->updateTeXImage();
}

//##############################################################################
//######  SLOTs for changes triggered via QActions in the context menu  ########
//##############################################################################
void TextLabel::visibilityChanged() {
	Q_D(const TextLabel);
	this->setVisible(!d->isVisible());
}

//##############################################################################
//####################### Private implementation ###############################
//##############################################################################
TextLabelPrivate::TextLabelPrivate(TextLabel* owner) : WorksheetElementPrivate(owner), q(owner) {
	setFlag(QGraphicsItem::ItemIsSelectable);
	setFlag(QGraphicsItem::ItemIsMovable);
	setFlag(QGraphicsItem::ItemSendsGeometryChanges);
	setFlag(QGraphicsItem::ItemIsFocusable);
	setAcceptHoverEvents(true);

	m_textItem = new ScaledTextItem(this);
	m_textItem->setScaleFactor(scaleFactor);
	m_textItem->setTextInteractionFlags(Qt::NoTextInteraction);
}

/*!
 * \brief TextLabelPrivate::size
 * \return Size and position of the TextLabel in scene coords
 */
QRectF TextLabelPrivate::size() {
	double w, h;
	if (textWrapper.mode == TextLabel::Mode::LaTeX) {
		//image size is in pixel, convert to scene units
		w = teXImage.width()*teXImageScaleFactor;
		h = teXImage.height()*teXImageScaleFactor;
	} else {
		//size is in points, convert to scene units
		w = m_textItem->boundingRect().width()*scaleFactor;
		h = m_textItem->boundingRect().height()*scaleFactor;
	}
	qreal x = position.point.x();
	qreal y = position.point.y();
	return {x, y, w, h};
}

/*!
 * \brief TextLabelPrivate::findNearestGluePoint
 * Finds the glue point, which is nearest to @param point in scene coords.
 * \param point reference position
 * \return Nearest point to @param point
 */
QPointF TextLabelPrivate::findNearestGluePoint(QPointF scenePoint) {
	if (m_gluePoints.isEmpty())
		return boundingRectangle.center();

	if (m_gluePoints.length() == 1)
		return mapParentToPlotArea(mapToParent(m_gluePoints.at(0).point));

	QPointF point = mapParentToPlotArea(mapToParent(m_gluePoints.at(0).point));
	QPointF nearestPoint = point;
	double distance2 = pow(point.x()-scenePoint.x(), 2) + pow(point.y()-scenePoint.y(), 2);
	// assumption, more than one point available
	for (int i = 1; i < m_gluePoints.length(); i++) {
		point = mapParentToPlotArea(mapToParent(m_gluePoints.at(i).point));
		double distance2_temp = pow(point.x()-scenePoint.x(), 2) + pow(point.y()-scenePoint.y(), 2);
		if (distance2_temp < distance2) {
			nearestPoint = point;
			distance2 = distance2_temp;
		}
	}

	return nearestPoint;
}

/*!
 * \brief TextLabelPrivate::gluePointAt
 * Returns the gluePoint at a specific index.
 * \param index
 * \return Returns gluepoint at index \param index
 */
TextLabel::GluePoint TextLabelPrivate::gluePointAt(int index) {
	QPointF pos;
	QString name;

	if (m_gluePoints.isEmpty() || index > m_gluePoints.length()) {
		pos = boundingRectangle.center();
		name = "center";
	} else if (index < 0) {
		pos = m_gluePoints.at(0).point;
		name = m_gluePoints.at(0).name;
	} else {
		pos = m_gluePoints.at(index).point;
		name = m_gluePoints.at(index).name;
	}

	return TextLabel::GluePoint(mapParentToPlotArea(mapToParent(pos)), name);
}

/*!
	calculates the position and the bounding box of the label. Called on geometry or text changes.
 */
void TextLabelPrivate::retransform() {
	if (suppressRetransform || q->isLoading())
		return;

	updatePosition();
	updateBorder();

	Q_EMIT q->changed();
}

void TextLabelPrivate::setZoomFactor(double factor) {
	zoomFactor = factor;

	if (textWrapper.mode == TextLabel::Mode::LaTeX) {
		teXImage = GuiTools::imageFromPDFData(teXPdfData, zoomFactor);
		retransform();
	}
}

/*!
	calculates the position of the label, when the position relative to the parent was specified (left, right, etc.)
*/
void TextLabelPrivate::updatePosition() {
	if (q->isLoading())
		return;

	QPointF p;
	if (q->m_type == TextLabel::Type::AxisTitle) {
		// In an axis element, the label is part of the bounding rect of the axis
		// so it is not possible to align with the bounding rect
		p = position.point;
	} else if(coordinateBindingEnabled && q->cSystem) {
		//the position in logical coordinates was changed, calculate the position in scene coordinates
		bool visible;
		p = q->cSystem->mapLogicalToScene(positionLogical, visible, AbstractCoordinateSystem::MappingFlag::SuppressPageClipping);
		p = q->align(p, boundingRectangle, horizontalAlignment, verticalAlignment, true);
		position.point = q->parentPosToRelativePos(p, boundingRectangle, position,
												horizontalAlignment, verticalAlignment);
	} else
		p = q->relativePosToParentPos(boundingRectangle, position,
									horizontalAlignment, verticalAlignment);

	suppressItemChangeEvent = true;
	setPos(p);
	suppressItemChangeEvent = false;

	Q_EMIT q->positionChanged(position);

	//the position in scene coordinates was changed, calculate the position in logical coordinates
	if (q->cSystem) {
		if (!coordinateBindingEnabled)
			positionLogical = q->cSystem->mapSceneToLogical(position.point, AbstractCoordinateSystem::MappingFlag::SuppressPageClipping);
		Q_EMIT q->positionLogicalChanged(positionLogical);
	}
}

/*!
	updates the static text.
 */
void TextLabelPrivate::updateText() {
	if (suppressRetransform)
		return;

	switch (textWrapper.mode) {
	case TextLabel::Mode::LaTeX: {
		m_textItem->hide();
		TeXRenderer::Formatting format;
		format.fontColor = fontColor;
		format.backgroundColor = backgroundColor;
		format.fontSize = teXFont.pointSize();
		format.fontFamily = teXFont.family();
		format.dpi = teXImageResolution;
		QFuture<QByteArray> future = QtConcurrent::run(TeXRenderer::renderImageLaTeX, textWrapper.text, &teXRenderSuccessful, format);
		teXImageFutureWatcher.setFuture(future);

		//don't need to call retransorm() here since it is done in updateTeXImage
		//when the asynchronous rendering of the image is finished.
		break;
	}
	case TextLabel::Mode::Text: {
		m_textItem->show();
		m_textItem->setHtml(textWrapper.text);

		//the size of the label was most probably changed,
		//recalculate the bounding box of the label
		updateBoundingRect();
		break;
	}
	case TextLabel::Mode::Markdown: {
#ifdef HAVE_DISCOUNT
		QByteArray mdCharArray = textWrapper.text.toUtf8();
		MMIOT* mdHandle = mkd_string(mdCharArray.data(), mdCharArray.size()+1, 0);
		if(!mkd_compile(mdHandle, MKD_LATEX | MKD_FENCEDCODE | MKD_GITHUBTAGS))
		{
			DEBUG("Failed to compile the markdown document");
			mkd_cleanup(mdHandle);
			return;
		}
		char *htmlDocument;
		int htmlSize = mkd_document(mdHandle, &htmlDocument);
		QString html = QString::fromUtf8(htmlDocument, htmlSize);

		mkd_cleanup(mdHandle);

		m_textItem->show();
		m_textItem->setHtml(html);
		updateBoundingRect();
#endif
	}
	}
}

void TextLabelPrivate::updateBoundingRect() {
	//determine the size of the label in scene units.
	double w, h;
	if (textWrapper.mode == TextLabel::Mode::LaTeX) {
		//image size is in pixel, convert to scene units
		w = teXImage.width()*teXImageScaleFactor;
		h = teXImage.height()*teXImageScaleFactor;
	} else {
		//size is in points, convert to scene units
		w = m_textItem->boundingRect().width()*scaleFactor;
		h = m_textItem->boundingRect().height()*scaleFactor;
	}

	boundingRectangle.setX(-w/2);
	boundingRectangle.setY(-h/2);
	boundingRectangle.setWidth(w);
	boundingRectangle.setHeight(h);

	updateBorder();
}

void TextLabelPrivate::updateTeXImage() {
	teXPdfData = teXImageFutureWatcher.result();
	teXImage = GuiTools::imageFromPDFData(teXPdfData, zoomFactor);
	updateBoundingRect();
	DEBUG(Q_FUNC_INFO << ", TeX renderer successful = " << teXRenderSuccessful);
	Q_EMIT q->teXImageUpdated(teXRenderSuccessful);
}

void TextLabelPrivate::updateBorder() {
	using GluePoint = TextLabel::GluePoint;

	borderShapePath = QPainterPath();
	switch (borderShape) {
	case (TextLabel::BorderShape::NoBorder): {
		m_gluePoints.clear();
		m_gluePoints.append(GluePoint(QPointF(boundingRectangle.x()+boundingRectangle.width()/2, boundingRectangle.y()), "top"));
		m_gluePoints.append(GluePoint(QPointF(boundingRectangle.x()+boundingRectangle.width(), boundingRectangle.y()+boundingRectangle.height()/2), "right"));
		m_gluePoints.append(GluePoint(QPointF(boundingRectangle.x()+boundingRectangle.width()/2, boundingRectangle.y()+boundingRectangle.height()), "bottom"));
		m_gluePoints.append(GluePoint(QPointF(boundingRectangle.x(), boundingRectangle.y()+boundingRectangle.height()/2), "left"));
		break;
	}
	case (TextLabel::BorderShape::Rect): {
		borderShapePath.addRect(boundingRectangle);

		m_gluePoints.clear();
		m_gluePoints.append(GluePoint(QPointF(boundingRectangle.x() + boundingRectangle.width()/2, boundingRectangle.y()), "top"));
		m_gluePoints.append(GluePoint(QPointF(boundingRectangle.x() + boundingRectangle.width(), boundingRectangle.y() + boundingRectangle.height()/2), "right"));
		m_gluePoints.append(GluePoint(QPointF(boundingRectangle.x() + boundingRectangle.width()/2, boundingRectangle.y() + boundingRectangle.height()), "bottom"));
		m_gluePoints.append(GluePoint(QPointF(boundingRectangle.x(), boundingRectangle.y() + boundingRectangle.height()/2), "left"));
		break;
	}
	case (TextLabel::BorderShape::Ellipse): {
		const double xs = boundingRectangle.x();
		const double ys = boundingRectangle.y();
		const double w = boundingRectangle.width();
		const double h = boundingRectangle.height();
		const QRectF ellipseRect(xs  - 0.1 * w, ys - 0.1 * h, 1.2 * w,  1.2 * h);
		borderShapePath.addEllipse(ellipseRect);

		m_gluePoints.clear();
		m_gluePoints.append(GluePoint(QPointF(ellipseRect.x() + ellipseRect.width()/2, ellipseRect.y()), "top"));
		m_gluePoints.append(GluePoint(QPointF(ellipseRect.x() + ellipseRect.width(), ellipseRect.y() + ellipseRect.height()/2), "right"));
		m_gluePoints.append(GluePoint(QPointF(ellipseRect.x() + ellipseRect.width()/2, ellipseRect.y() + ellipseRect.height()), "bottom"));
		m_gluePoints.append(GluePoint(QPointF(ellipseRect.x(), ellipseRect.y() + ellipseRect.height()/2), "left"));
		break;
	}
	case (TextLabel::BorderShape::RoundSideRect): {
		const double xs = boundingRectangle.x();
		const double ys = boundingRectangle.y();
		const double w = boundingRectangle.width();
		const double h = boundingRectangle.height();
		borderShapePath.moveTo(xs, ys);
		borderShapePath.lineTo(xs + w, ys);
		borderShapePath.quadTo(xs + w + h/2, ys + h/2, xs + w, ys + h);
		borderShapePath.lineTo(xs, ys + h);
		borderShapePath.quadTo(xs - h/2, ys + h/2, xs, ys);

		m_gluePoints.clear();
		m_gluePoints.append(GluePoint(QPointF(xs + w/2, ys), "top"));
		m_gluePoints.append(GluePoint(QPointF(xs + w + h/2, ys + h/2), "right"));
		m_gluePoints.append(GluePoint(QPointF(xs + w/2, ys + h), "bottom"));
		m_gluePoints.append(GluePoint(QPointF(xs - h/2, ys + h/2), "left"));
		break;
	}
	case (TextLabel::BorderShape::RoundCornerRect): {
		const double xs = boundingRectangle.x();
		const double ys = boundingRectangle.y();
		const double w = boundingRectangle.width();
		const double h = boundingRectangle.height();
		borderShapePath.moveTo(xs + h * 0.2, ys);
		borderShapePath.lineTo(xs + w - h * 0.2, ys);
		borderShapePath.quadTo(xs + w, ys, xs + w, ys + h * 0.2);
		borderShapePath.lineTo(xs + w, ys + h - 0.2 * h);
		borderShapePath.quadTo(xs + w, ys + h, xs + w - 0.2 * h, ys + h);
		borderShapePath.lineTo(xs + 0.2 * h, ys + h);
		borderShapePath.quadTo(xs, ys + h, xs, ys + h - 0.2 * h);
		borderShapePath.lineTo(xs, ys + 0.2 * h);
		borderShapePath.quadTo(xs, ys, xs + 0.2 * h, ys);

		m_gluePoints.clear();
		m_gluePoints.append(GluePoint(QPointF(xs+w/2,ys), "top"));
		m_gluePoints.append(GluePoint(QPointF(xs+w,ys+h/2), "right"));
		m_gluePoints.append(GluePoint(QPointF(xs+w/2,ys+h), "bottom"));
		m_gluePoints.append(GluePoint(QPointF(xs,ys+h/2), "left"));
		break;
	}
	case (TextLabel::BorderShape::InwardsRoundCornerRect): {
		const double xs = boundingRectangle.x();
		const double ys = boundingRectangle.y();
		const double w = boundingRectangle.width();
		const double h = boundingRectangle.height();
		borderShapePath.moveTo(xs, ys - 0.3 * h);
		borderShapePath.lineTo(xs + w, ys - 0.3 * h);
		borderShapePath.quadTo(xs + w, ys, xs + w + 0.3 * h, ys);
		borderShapePath.lineTo(xs + w + 0.3 * h, ys + h);
		borderShapePath.quadTo(xs + w, ys + h, xs + w, ys + h + 0.3 * h);
		borderShapePath.lineTo(xs, ys + h + 0.3 * h);
		borderShapePath.quadTo(xs, ys + h, xs - 0.3 * h, ys + h);
		borderShapePath.lineTo(xs - 0.3 * h, ys);
		borderShapePath.quadTo(xs, ys, xs, ys - 0.3 * h);

		m_gluePoints.clear();
		m_gluePoints.append(GluePoint(QPointF(xs + w / 2, ys - 0.3 * h), "top"));
		m_gluePoints.append(GluePoint(QPointF(xs + w + 0.3 * h, ys + h / 2), "right"));
		m_gluePoints.append(GluePoint(QPointF(xs + w / 2, ys + h + 0.3 * h), "bottom"));
		m_gluePoints.append(GluePoint(QPointF(xs - 0.3 * h, ys + h / 2), "left"));
		break;
	}
	case (TextLabel::BorderShape::DentedBorderRect): {
		const double xs = boundingRectangle.x();
		const double ys = boundingRectangle.y();
		const double w = boundingRectangle.width();
		const double h = boundingRectangle.height();
		borderShapePath.moveTo(xs - 0.2 * h, ys - 0.2 * h);
		borderShapePath.quadTo(xs + w / 2, ys, xs + w + 0.2 * h, ys - 0.2 * h);
		borderShapePath.quadTo(xs + w, ys + h / 2, xs + w + 0.2 * h, ys + h + 0.2 * h);
		borderShapePath.quadTo(xs + w / 2, ys + h, xs - 0.2 * h, ys + h + 0.2 * h);
		borderShapePath.quadTo(xs, ys + h / 2, xs - 0.2 * h, ys - 0.2 * h);

		m_gluePoints.clear();
		m_gluePoints.append(GluePoint(QPointF(xs + w / 2, ys - 0.2 * h), "top"));
		m_gluePoints.append(GluePoint(QPointF(xs + w + 0.2 * h, ys + h / 2), "right"));
		m_gluePoints.append(GluePoint(QPointF(xs + w / 2, ys + h + 0.2 * h), "bottom"));
		m_gluePoints.append(GluePoint(QPointF(xs - 0.2 * h, ys + h / 2), "left"));
		break;
	}
	case (TextLabel::BorderShape::Cuboid): {
		const double xs = boundingRectangle.x();
		const double ys = boundingRectangle.y();
		const double w = boundingRectangle.width();
		const double h = boundingRectangle.height();
		borderShapePath.moveTo(xs, ys);
		borderShapePath.lineTo(xs + w, ys);
		borderShapePath.lineTo(xs + w, ys + h);
		borderShapePath.lineTo(xs, ys + h);
		borderShapePath.lineTo(xs, ys);
		borderShapePath.lineTo(xs + 0.3 * h, ys - 0.2 * h);
		borderShapePath.lineTo(xs + w + 0.3 * h, ys - 0.2 * h);
		borderShapePath.lineTo(xs + w, ys);
		borderShapePath.moveTo(xs + w, ys + h);
		borderShapePath.lineTo(xs + w + 0.3 * h, ys + h - 0.2 * h);
		borderShapePath.lineTo(xs + w + 0.3 * h, ys - 0.2 * h);

		m_gluePoints.clear();
		m_gluePoints.append(GluePoint(QPointF(xs + w / 2, ys - 0.1 * h), "top"));
		m_gluePoints.append(GluePoint(QPointF(xs + w + 0.15 * h, ys + h / 2), "right"));
		m_gluePoints.append(GluePoint(QPointF(xs + w / 2, ys + h), "bottom"));
		m_gluePoints.append(GluePoint(QPointF(xs, ys + h / 2), "left"));
		break;
	}
	case (TextLabel::BorderShape::UpPointingRectangle): {
		const double xs = boundingRectangle.x();
		const double ys = boundingRectangle.y();
		const double w = boundingRectangle.width();
		const double h = boundingRectangle.height();
		borderShapePath.moveTo(xs + h * 0.2, ys);
		borderShapePath.lineTo(xs + w / 2 - 0.2 * h, ys);
		borderShapePath.lineTo(xs + w / 2, ys - 0.2 * h);
		borderShapePath.lineTo(xs + w / 2 + 0.2 * h, ys);
		borderShapePath.lineTo(xs + w - h * 0.2, ys);
		borderShapePath.quadTo(xs + w, ys, xs + w, ys + h * 0.2);
		borderShapePath.lineTo(xs + w, ys + h - 0.2 * h);
		borderShapePath.quadTo(xs + w, ys + h, xs + w - 0.2 * h, ys + h);
		borderShapePath.lineTo(xs + 0.2 * h, ys + h);
		borderShapePath.quadTo(xs, ys + h, xs, ys + h - 0.2 * h);
		borderShapePath.lineTo(xs, ys + 0.2 * h);
		borderShapePath.quadTo(xs, ys, xs + 0.2 * h, ys);

		m_gluePoints.clear();
		m_gluePoints.append(GluePoint(QPointF(xs + w/2, ys - h * 0.2), "top"));
		m_gluePoints.append(GluePoint(QPointF(xs + w, ys + h/2), "right"));
		m_gluePoints.append(GluePoint(QPointF(xs + w/2, ys + h), "bottom"));
		m_gluePoints.append(GluePoint(QPointF(xs, ys + h/2), "left"));
		break;
	}
	case (TextLabel::BorderShape::DownPointingRectangle): {
		const double xs = boundingRectangle.x();
		const double ys = boundingRectangle.y();
		const double w = boundingRectangle.width();
		const double h = boundingRectangle.height();
		borderShapePath.moveTo(xs +h * 0.2, ys);
		borderShapePath.lineTo(xs + w - h * 0.2, ys);
		borderShapePath.quadTo(xs + w, ys, xs + w, ys + h * 0.2);
		borderShapePath.lineTo(xs + w, ys + h - 0.2 * h);
		borderShapePath.quadTo(xs + w, ys + h, xs + w - 0.2 * h, ys + h);
		borderShapePath.lineTo(xs + w / 2 + 0.2 * h, ys + h);
		borderShapePath.lineTo(xs + w / 2, ys + h + 0.2 * h);
		borderShapePath.lineTo(xs + w / 2 - 0.2 * h, ys + h);
		borderShapePath.lineTo(xs + 0.2 * h, ys + h);
		borderShapePath.quadTo(xs, ys + h, xs, ys + h - 0.2 * h);
		borderShapePath.lineTo(xs, ys + 0.2 * h);
		borderShapePath.quadTo(xs, ys, xs + 0.2 * h, ys);

		m_gluePoints.clear();
		m_gluePoints.append(GluePoint(QPointF(xs + w/2, ys), "top"));
		m_gluePoints.append(GluePoint(QPointF(xs + w, ys + h/2), "right"));
		m_gluePoints.append(GluePoint(QPointF(xs + w/2, ys + h  + 0.2 * h), "bottom"));
		m_gluePoints.append(GluePoint(QPointF(xs, ys + h/2), "left"));
		break;
	}
	case (TextLabel::BorderShape::LeftPointingRectangle): {
		const double xs = boundingRectangle.x();
		const double ys = boundingRectangle.y();
		const double w = boundingRectangle.width();
		const double h = boundingRectangle.height();
		borderShapePath.moveTo(xs + h*0.2, ys);
		borderShapePath.lineTo(xs + w - h * 0.2, ys);
		borderShapePath.quadTo(xs + w, ys, xs + w, ys + h * 0.2);
		borderShapePath.lineTo(xs + w,  ys + h - 0.2 * h);
		borderShapePath.quadTo(xs + w, ys + h, xs + w - 0.2 * h, ys + h);
		borderShapePath.lineTo(xs + 0.2 * h, ys + h);
		borderShapePath.quadTo(xs, ys + h, xs, ys + h - 0.2 * h);
		borderShapePath.lineTo(xs, ys + h / 2 + 0.2 * h);
		borderShapePath.lineTo(xs - 0.2 * h, ys + h / 2);
		borderShapePath.lineTo(xs, ys + h / 2 - 0.2 * h);
		borderShapePath.lineTo(xs, ys + 0.2 * h);
		borderShapePath.quadTo(xs, ys, xs + 0.2 * h, ys);

		m_gluePoints.clear();
		m_gluePoints.append(GluePoint(QPointF(xs + w/2, ys), "top"));
		m_gluePoints.append(GluePoint(QPointF(xs + w, ys + h/2), "right"));
		m_gluePoints.append(GluePoint(QPointF(xs + w/2, ys + h), "bottom"));
		m_gluePoints.append(GluePoint(QPointF(xs - 0.2 * h, ys + h/2), "left"));
		break;
	}
	case (TextLabel::BorderShape::RightPointingRectangle): {
		const double xs = boundingRectangle.x();
		const double ys = boundingRectangle.y();
		const double w = boundingRectangle.width();
		const double h = boundingRectangle.height();
		borderShapePath.moveTo(xs + h * 0.2, ys);
		borderShapePath.lineTo(xs + w - h * 0.2, ys);
		borderShapePath.quadTo(xs + w, ys, xs + w, ys + h * 0.2);
		borderShapePath.lineTo(xs + w, ys + h / 2 - 0.2 * h);
		borderShapePath.lineTo(xs + w + 0.2 * h, ys + h / 2);
		borderShapePath.lineTo(xs + w, ys + h / 2 + 0.2 * h);
		borderShapePath.lineTo(xs + w, ys + h - 0.2 * h);
		borderShapePath.quadTo(xs + w, ys + h, xs + w - 0.2 * h, ys + h);
		borderShapePath.lineTo(xs + 0.2 * h, ys + h);
		borderShapePath.quadTo(xs, ys + h, xs, ys + h - 0.2 * h);
		borderShapePath.lineTo(xs, ys + 0.2 * h);
		borderShapePath.quadTo(xs, ys, xs + 0.2 * h, ys);

		m_gluePoints.clear();
		m_gluePoints.append(GluePoint(QPointF(xs + w/2, ys), "top"));
		m_gluePoints.append(GluePoint(QPointF(xs + w + 0.2 * h, ys + h/2), "right"));
		m_gluePoints.append(GluePoint(QPointF(xs + w/2, ys + h), "bottom"));
		m_gluePoints.append(GluePoint(QPointF(xs, ys + h/2), "left"));
		break;
	}
	}

	recalcShapeAndBoundingRect();
}

/*!
	Returns the outer bounds of the item as a rectangle.
 */
QRectF TextLabelPrivate::boundingRect() const {
	return transformedBoundingRectangle;
}

/*!
	Returns the shape of this item as a QPainterPath in local coordinates.
*/
QPainterPath TextLabelPrivate::shape() const {
	return labelShape;
}

/*!
  recalculates the outer bounds and the shape of the label.
*/
void TextLabelPrivate::recalcShapeAndBoundingRect() {
	prepareGeometryChange();

	QMatrix matrix;
	matrix.rotate(-rotationAngle);
	labelShape = QPainterPath();
	if (borderShape != TextLabel::BorderShape::NoBorder) {
		labelShape.addPath(WorksheetElement::shapeFromPath(borderShapePath, borderPen));
		transformedBoundingRectangle = matrix.mapRect(labelShape.boundingRect());
	} else {
		labelShape.addRect(boundingRectangle);
		transformedBoundingRectangle = matrix.mapRect(boundingRectangle);
	}

	labelShape = matrix.map(labelShape);

	// rotate gluePoints
	for (auto gPoint : m_gluePoints)
		gPoint.point = matrix.map(gPoint.point);

	m_textItem->setRotationAngle(rotationAngle);

	Q_EMIT q->changed();
}

void TextLabelPrivate::paint(QPainter* painter, const QStyleOptionGraphicsItem* /*option*/, QWidget*) {
	if (positionInvalid || textWrapper.text.isEmpty())
		return;

	//draw the text
	painter->save();
	switch (textWrapper.mode) {
	case TextLabel::Mode::LaTeX: {
		painter->rotate(-rotationAngle);
		if (boundingRect().width() != 0.0 &&  boundingRect().height() != 0.0)
			painter->drawImage(boundingRect(), teXImage);
		break;
	}
	case TextLabel::Mode::Text:
	case TextLabel::Mode::Markdown: {
		//nothing to do here, the painting is done in the ScaledTextItem child
		break;
	}
	}
	painter->restore();

	//draw the border
	if (borderShape != TextLabel::BorderShape::NoBorder) {
		painter->save();
		painter->setPen(borderPen);
		painter->setOpacity(borderOpacity);

		painter->rotate(-rotationAngle);
		painter->drawPath(borderShapePath);
		painter->restore();
	}

	if (m_hovered && !isSelected() && !q->isPrinting()) {
		painter->setPen(QPen(QApplication::palette().color(QPalette::Shadow), 2, Qt::SolidLine));
		painter->drawPath(labelShape);
	}

	if (isSelected() && !q->isPrinting()) {
		painter->setPen(QPen(QApplication::palette().color(QPalette::Highlight), 2, Qt::SolidLine));
		painter->drawPath(labelShape);
	}

#define DEBUG_TEXTLABEL_GLUEPOINTS 0
#if DEBUG_TEXTLABEL_GLUEPOINTS
	// just for debugging
	painter->setPen(QColor(Qt::GlobalColor::red));
	QRectF gluePointRect(0,0,10,10);
	for (int i=0; i < m_gluePoints.length(); i++) {
		gluePointRect.moveTo(m_gluePoints[i].point.x() - gluePointRect.width()/2, m_gluePoints[i].point.y() - gluePointRect.height()/2);
		painter->fillRect(gluePointRect, QColor(Qt::GlobalColor::red));
	}
#endif
}

/*
void TextLabelPrivate::mouseMoveEvent(QGraphicsSceneMouseEvent* event) {
	if(coordinateBinding)
		positionLogical = q->cSystem->mapSceneToLogical(mapParentToPlotArea(pos()), AbstractCoordinateSystem::MappingFlag::SuppressPageClipping);
	return QGraphicsItem::mouseMoveEvent(event);
}*/

void TextLabelPrivate::contextMenuEvent(QGraphicsSceneContextMenuEvent* event) {
	q->createContextMenu()->exec(event->screenPos());
}

void TextLabelPrivate::hoverEnterEvent(QGraphicsSceneHoverEvent*) {
	if (!isSelected()) {
		m_hovered = true;
		Q_EMIT q->hovered();
		update();
	}
}

void TextLabelPrivate::hoverLeaveEvent(QGraphicsSceneHoverEvent*) {
	if (m_hovered) {
		m_hovered = false;
		Q_EMIT q->unhovered();
		update();
	}
}

//##############################################################################
//##################  Serialization/Deserialization  ###########################
//##############################################################################
//! Save as XML
void TextLabel::save(QXmlStreamWriter* writer) const {
	Q_D(const TextLabel);

	writer->writeStartElement( "textLabel" );
	writeBasicAttributes(writer);
	writeCommentElement(writer);

	writer->writeStartElement("geometry");
	WorksheetElement::save(writer);
	writer->writeEndElement();

	writer->writeStartElement( "text" );
	writer->writeCharacters( d->textWrapper.text );
	writer->writeEndElement();

	if (!d->textWrapper.textPlaceholder.isEmpty()) {
		writer->writeStartElement( "textPlaceholder");
		writer->writeCharacters( d->textWrapper.textPlaceholder);
		writer->writeEndElement();
	}

	writer->writeStartElement( "format" );
	writer->writeAttribute( "placeholder", QString::number(d->textWrapper.allowPlaceholder) );
	writer->writeAttribute( "mode", QString::number(static_cast<int>(d->textWrapper.mode)) );
	WRITE_QFONT(d->teXFont);
	writer->writeAttribute( "fontColor_r", QString::number(d->fontColor.red()) );
	writer->writeAttribute( "fontColor_g", QString::number(d->fontColor.green()) );
	writer->writeAttribute( "fontColor_b", QString::number(d->fontColor.blue()) );
	writer->writeEndElement();

	//border
	writer->writeStartElement("border");
	writer->writeAttribute("borderShape", QString::number(static_cast<int>(d->borderShape)));
	WRITE_QPEN(d->borderPen);
	writer->writeAttribute("borderOpacity", QString::number(d->borderOpacity));
	writer->writeEndElement();

	if (d->textWrapper.mode ==  TextLabel::Mode::LaTeX) {
		writer->writeStartElement("teXPdfData");
		writer->writeCharacters(d->teXPdfData.toBase64());
		writer->writeEndElement();
	}

	writer->writeEndElement(); // close "textLabel" section
}

//! Load from XML
bool TextLabel::load(XmlStreamReader* reader, bool preview) {
	if (!readBasicAttributes(reader))
		return false;

	Q_D(TextLabel);
	KLocalizedString attributeWarning = ki18n("Attribute '%1' missing or empty, default value is used");
	QXmlStreamAttributes attribs;
	QString str;
	bool teXImageFound = false;

	while (!reader->atEnd()) {
		reader->readNext();
		if (reader->isEndElement() && reader->name() == "textLabel")
			break;

		if (!reader->isStartElement())
			continue;

		if (!preview && reader->name() == "comment") {
			if (!readCommentElement(reader)) return false;
		} else if (!preview && reader->name() == "geometry") {
			WorksheetElement::load(reader, preview);
		} else if (!preview && reader->name() == "text")
			d->textWrapper.text = reader->readElementText();
		else if (!preview && reader->name() == "textPlaceholder")
			d->textWrapper.textPlaceholder = reader->readElementText();
		else if (!preview && reader->name() == "format") {
			attribs = reader->attributes();

			if (project()->xmlVersion() < 4) {
				str = attribs.value("teXUsed").toString();
				d->textWrapper.mode = static_cast<TextLabel::Mode>(str.toInt());
			} else
				READ_INT_VALUE("mode", textWrapper.mode, TextLabel::Mode);

			READ_QFONT(d->teXFont);

			str = attribs.value("placeholder").toString();
			if(!str.isEmpty())
				d->textWrapper.allowPlaceholder = str.toInt();

			str = attribs.value("fontColor_r").toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.subs("fontColor_r").toString());
			else
				d->fontColor.setRed( str.toInt() );

			str = attribs.value("fontColor_g").toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.subs("fontColor_g").toString());
			else
				d->fontColor.setGreen( str.toInt() );

			str = attribs.value("fontColor_b").toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.subs("fontColor_b").toString());
			else
				d->fontColor.setBlue( str.toInt() );
		} else if (!preview && reader->name() == "border") {
			attribs = reader->attributes();
			READ_INT_VALUE("borderShape", borderShape, BorderShape);
			READ_QPEN(d->borderPen);
			READ_DOUBLE_VALUE("borderOpacity", borderOpacity);
		} else if (!preview && reader->name() == "teXPdfData") {
			reader->readNext();
			QString content = reader->text().toString().trimmed();
			d->teXPdfData = QByteArray::fromBase64(content.toLatin1());
			d->teXImage = GuiTools::imageFromPDFData(d->teXPdfData);
			teXImageFound = true;
		} else { // unknown element
			reader->raiseWarning(i18n("unknown element '%1'", reader->name().toString()));
			if (!reader->skipToEndElement()) return false;
		}
	}

	if (preview)
		return true;

	//in case we use latex and the image was stored (older versions of LabPlot didn't save the image)and loaded,
	//we just need to call updateBoundingRect() to calculate the new rect.
	//otherwise, we set the static text and call updateBoundingRect() in updateText()
	if ( !(d->textWrapper.mode == TextLabel::Mode::LaTeX && teXImageFound) )
		d->updateText();
	else
		d->updateBoundingRect();

	return true;
}

//##############################################################################
//#########################  Theme management ##################################
//##############################################################################
void TextLabel::loadThemeConfig(const KConfig& config) {
	DEBUG(Q_FUNC_INFO << ", label = " << STDSTRING(name()))
	Q_D(TextLabel);

	KConfigGroup group = config.group("Label");
	//TODO: dark mode support?
	d->fontColor = group.readEntry("FontColor", QColor(Qt::black)); // used when it's latex text
	d->backgroundColor = group.readEntry("BackgroundColor", QColor(Qt::white)); // used when it's latex text

	if (d->textWrapper.mode == TextLabel::Mode::Text && !d->textWrapper.text.isEmpty()) {
		// TODO: Replace QTextEdit by QTextDocument, because this does not contain the graphical stuff.
		// To set the color in a html text, a QTextEdit must be used
		QTextEdit te;
		te.setHtml(d->textWrapper.text);
		te.selectAll();
		te.setTextColor(d->fontColor);
		te.setTextBackgroundColor(d->backgroundColor); // for plain text no background color supported, due to bug https://bugreports.qt.io/browse/QTBUG-25420

		TextWrapper wrapper(te.toHtml(), TextLabel::Mode::Text, true);
		te.setHtml(d->textWrapper.textPlaceholder);
		te.selectAll();
		te.setTextColor(d->fontColor);
		te.setTextBackgroundColor(d->backgroundColor); // for plain text no background color supported, due to bug https://bugreports.qt.io/browse/QTBUG-25420
		wrapper.textPlaceholder = te.toHtml();
		wrapper.allowPlaceholder = d->textWrapper.allowPlaceholder;

		// update the text. also in the Widget to which is connected

		setText(wrapper);
	} else if (d->textWrapper.mode == TextLabel::Mode::LaTeX) {
		//call updateText() to re-render the LaTeX-image with the new text colors
		d->updateText();
	}

	// otherwise when changing theme while the textlabel dock is visible, the
	// color comboboxes do not change the color
	backgroundColorChanged(d->backgroundColor);
	fontColorChanged(d->fontColor);

	group = config.group("CartesianPlot");
	QPen pen = this->borderPen();
	pen.setColor(group.readEntry("BorderColor", pen.color()));
	pen.setStyle((Qt::PenStyle)(group.readEntry("BorderStyle", (int) pen.style())));
	pen.setWidthF(group.readEntry("BorderWidth", pen.widthF()));
	this->setBorderPen(pen);
	this->setBorderOpacity(group.readEntry("BorderOpacity", this->borderOpacity()));
}

void TextLabel::saveThemeConfig(const KConfig& config) {
	KConfigGroup group = config.group("Label");
	//TODO
// 	group.writeEntry("TeXFontColor", (QColor) this->fontColor());
}
