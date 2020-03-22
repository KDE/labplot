/***************************************************************************
    File                 : TextLabel.cpp
    Project              : LabPlot
    Description          : Text label supporting reach text and latex formatting
    --------------------------------------------------------------------
    Copyright            : (C) 2009 Tilman Benkert (thzs@gmx.net)
    Copyright            : (C) 2012-2018 Alexander Semke (alexander.semke@web.de)
    Copyright            : (C) 2019 by Stefan Gerlach (stefan.gerlach@uni.kn)
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

#include "TextLabel.h"
#include "Worksheet.h"
#include "TextLabelPrivate.h"
#include "backend/lib/commandtemplates.h"
#include "backend/lib/XmlStreamReader.h"

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
	: WorksheetElement(name, AspectType::TextLabel), d_ptr(new TextLabelPrivate(this)), m_type(type) {

	init();
}

TextLabel::TextLabel(const QString &name, TextLabelPrivate *dd, Type type)
	: WorksheetElement(name, AspectType::TextLabel), d_ptr(dd), m_type(type) {

	init();
}

TextLabel::Type TextLabel::type() const {
	return m_type;
}

void TextLabel::init() {
	Q_D(TextLabel);

	QString groupName;
	switch (m_type) {
	case General:
		groupName = "TextLabel";
		break;
	case PlotTitle:
		groupName = "PlotTitle";
		break;
	case AxisTitle:
		groupName = "AxisTitle";
		break;
	case PlotLegendTitle:
		groupName = "PlotLegendTitle";
		break;
	}

	const KConfig config;
	DEBUG("	config has group \"" << groupName.toStdString() << "\": " << config.hasGroup(groupName));
	// group is always valid if you call config.group(..;
	KConfigGroup group;
	if (config.hasGroup(groupName))
		group = config.group(groupName);

	// non-default settings
	d->staticText.setTextFormat(Qt::RichText);
	// explicitly set no wrap mode for text label to avoid unnecessary line breaks
	QTextOption textOption;
	textOption.setWrapMode(QTextOption::NoWrap);
	d->staticText.setTextOption(textOption);

	if (m_type == PlotTitle || m_type == PlotLegendTitle) {
		d->position.verticalPosition = WorksheetElement::vPositionTop;
		d->verticalAlignment = WorksheetElement::vAlignBottom;
	} else if (m_type == AxisTitle) {
		d->position.horizontalPosition = WorksheetElement::hPositionCustom;
		d->position.verticalPosition = WorksheetElement::vPositionCustom;
	}

	// read settings from config if group exists
	if (group.isValid()) {
		//properties common to all types
		d->textWrapper.teXUsed = group.readEntry("TeXUsed", d->textWrapper.teXUsed);
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
		d->horizontalAlignment = (WorksheetElement::HorizontalAlignment) group.readEntry("HorizontalAlignment", (int)d->horizontalAlignment);
		d->verticalAlignment = (WorksheetElement::VerticalAlignment) group.readEntry("VerticalAlignment", (int)d->verticalAlignment);
	}

	DEBUG("CHECK: default/run time image resolution: " << d->teXImageResolution << '/' << QApplication::desktop()->physicalDpiX());

	connect(&d->teXImageFutureWatcher, &QFutureWatcher<QImage>::finished, this, &TextLabel::updateTeXImage);
}

//no need to delete the d-pointer here - it inherits from QGraphicsItem
//and is deleted during the cleanup in QGraphicsScene
TextLabel::~TextLabel() = default;

QGraphicsItem* TextLabel::graphicsItem() const {
	return d_ptr;
}

void TextLabel::setParentGraphicsItem(QGraphicsItem* item) {
	Q_D(TextLabel);
	d->setParentItem(item);
	d->updatePosition();
}

void TextLabel::retransform() {
	Q_D(TextLabel);
	d->retransform();
}

void TextLabel::handleResize(double horizontalRatio, double verticalRatio, bool pageResize) {
	DEBUG("TextLabel::handleResize()");
	Q_UNUSED(pageResize);
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
QIcon TextLabel::icon() const{
	return QIcon::fromTheme("draw-text");
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
CLASS_SHARED_D_READER_IMPL(TextLabel, TextLabel::TextWrapper, text, textWrapper)
CLASS_SHARED_D_READER_IMPL(TextLabel, QColor, fontColor, fontColor);
CLASS_SHARED_D_READER_IMPL(TextLabel, QColor, backgroundColor, backgroundColor);
CLASS_SHARED_D_READER_IMPL(TextLabel, QFont, teXFont, teXFont);
CLASS_SHARED_D_READER_IMPL(TextLabel, TextLabel::PositionWrapper, position, position);
BASIC_SHARED_D_READER_IMPL(TextLabel, WorksheetElement::HorizontalAlignment, horizontalAlignment, horizontalAlignment);
BASIC_SHARED_D_READER_IMPL(TextLabel, WorksheetElement::VerticalAlignment, verticalAlignment, verticalAlignment);
BASIC_SHARED_D_READER_IMPL(TextLabel, qreal, rotationAngle, rotationAngle);

BASIC_SHARED_D_READER_IMPL(TextLabel, TextLabel::BorderShape, borderShape, borderShape)
CLASS_SHARED_D_READER_IMPL(TextLabel, QPen, borderPen, borderPen)
BASIC_SHARED_D_READER_IMPL(TextLabel, qreal, borderOpacity, borderOpacity)

/* ============================ setter methods and undo commands ================= */
STD_SETTER_CMD_IMPL_F_S(TextLabel, SetText, TextLabel::TextWrapper, textWrapper, updateText);
void TextLabel::setText(const TextWrapper &textWrapper) {
	Q_D(TextLabel);
	if ( (textWrapper.text != d->textWrapper.text) || (textWrapper.teXUsed != d->textWrapper.teXUsed) )
		exec(new TextLabelSetTextCmd(d, textWrapper, ki18n("%1: set label text")));
}

STD_SETTER_CMD_IMPL_F_S(TextLabel, SetTeXFont, QFont, teXFont, updateText);
void TextLabel::setTeXFont(const QFont& font) {
	Q_D(TextLabel);
	if (font != d->teXFont)
		exec(new TextLabelSetTeXFontCmd(d, font, ki18n("%1: set TeX main font")));
}

STD_SETTER_CMD_IMPL_F_S(TextLabel, SetTeXFontColor, QColor, fontColor, updateText);
void TextLabel::setFontColor(const QColor color) {
	Q_D(TextLabel);
	if (color != d->fontColor)
		exec(new TextLabelSetTeXFontColorCmd(d, color, ki18n("%1: set font color")));
}

STD_SETTER_CMD_IMPL_F_S(TextLabel, SetTeXBackgroundColor, QColor, backgroundColor, updateText);
void TextLabel::setBackgroundColor(const QColor color) {
	Q_D(TextLabel);
	if (color != d->backgroundColor)
		exec(new TextLabelSetTeXBackgroundColorCmd(d, color, ki18n("%1: set background color")));
}

STD_SETTER_CMD_IMPL_F_S(TextLabel, SetPosition, TextLabel::PositionWrapper, position, retransform);
void TextLabel::setPosition(const PositionWrapper& pos) {
	Q_D(TextLabel);
	if (pos.point != d->position.point || pos.horizontalPosition != d->position.horizontalPosition || pos.verticalPosition != d->position.verticalPosition)
		exec(new TextLabelSetPositionCmd(d, pos, ki18n("%1: set position")));

}

/*!
	sets the position without undo/redo-stuff
*/
void TextLabel::setPosition(QPointF point) {
	Q_D(TextLabel);
	if (point != d->position.point) {
		d->position.point = point;
		retransform();
	}
}

/*!
 * position is set to invalid if the parent item is not drawn on the scene
 * (e.g. axis is not drawn because it's outside plot ranges -> don't draw axis' title label)
 */
void TextLabel::setPositionInvalid(bool invalid) {
	Q_D(TextLabel);
	if (invalid != d->positionInvalid)
		d->positionInvalid = invalid;
}

STD_SETTER_CMD_IMPL_F_S(TextLabel, SetRotationAngle, qreal, rotationAngle, recalcShapeAndBoundingRect);
void TextLabel::setRotationAngle(qreal angle) {
	Q_D(TextLabel);
	if (angle != d->rotationAngle)
		exec(new TextLabelSetRotationAngleCmd(d, angle, ki18n("%1: set rotation angle")));
}

STD_SETTER_CMD_IMPL_F_S(TextLabel, SetHorizontalAlignment, TextLabel::HorizontalAlignment, horizontalAlignment, retransform);
void TextLabel::setHorizontalAlignment(const WorksheetElement::HorizontalAlignment hAlign) {
	Q_D(TextLabel);
	if (hAlign != d->horizontalAlignment)
		exec(new TextLabelSetHorizontalAlignmentCmd(d, hAlign, ki18n("%1: set horizontal alignment")));
}

STD_SETTER_CMD_IMPL_F_S(TextLabel, SetVerticalAlignment, WorksheetElement::VerticalAlignment, verticalAlignment, retransform);
void TextLabel::setVerticalAlignment(const TextLabel::VerticalAlignment vAlign) {
	Q_D(TextLabel);
	if (vAlign != d->verticalAlignment)
		exec(new TextLabelSetVerticalAlignmentCmd(d, vAlign, ki18n("%1: set vertical alignment")));
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
STD_SWAP_METHOD_SETTER_CMD_IMPL_F(TextLabel, SetVisible, bool, swapVisible, retransform);
void TextLabel::setVisible(bool on) {
	Q_D(TextLabel);
	exec(new TextLabelSetVisibleCmd(d, on, on ? ki18n("%1: set visible") : ki18n("%1: set invisible")));
}

bool TextLabel::isVisible() const {
	Q_D(const TextLabel);
	return d->isVisible();
}

void TextLabel::setPrinting(bool on) {
	Q_D(TextLabel);
	d->m_printing = on;
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
TextLabelPrivate::TextLabelPrivate(TextLabel* owner) : q(owner) {
	setFlag(QGraphicsItem::ItemIsSelectable);
	setFlag(QGraphicsItem::ItemIsMovable);
	setFlag(QGraphicsItem::ItemSendsGeometryChanges);
	setFlag(QGraphicsItem::ItemIsFocusable);
	setAcceptHoverEvents(true);
}

QString TextLabelPrivate::name() const {
	return q->name();
}

/*!
	calculates the position and the bounding box of the label. Called on geometry or text changes.
 */
void TextLabelPrivate::retransform() {
	if (suppressRetransform)
		return;

	if (position.horizontalPosition != WorksheetElement::hPositionCustom
	        || position.verticalPosition != WorksheetElement::vPositionCustom)
		updatePosition();

	double x = position.point.x();
	double y = position.point.y();

	//determine the size of the label in scene units.
	double w, h;
	if (textWrapper.teXUsed) {
		//image size is in pixel, convert to scene units
		w = teXImage.width()*teXImageScaleFactor;
		h = teXImage.height()*teXImageScaleFactor;
	}
	else {
		//size is in points, convert to scene units
		w = staticText.size().width()*scaleFactor;
		h = staticText.size().height()*scaleFactor;
	}

	//depending on the alignment, calculate the new GraphicsItem's position in parent's coordinate system
	QPointF itemPos;
	switch (horizontalAlignment) {
	case WorksheetElement::hAlignLeft:
		itemPos.setX(x - w/2);
		break;
	case WorksheetElement::hAlignCenter:
		itemPos.setX(x);
		break;
	case WorksheetElement::hAlignRight:
		itemPos.setX(x + w/2);
		break;
	}

	switch (verticalAlignment) {
	case WorksheetElement::vAlignTop:
		itemPos.setY(y - h/2);
		break;
	case WorksheetElement::vAlignCenter:
		itemPos.setY(y);
		break;
	case WorksheetElement::vAlignBottom:
		itemPos.setY(y + h/2);
		break;
	}

	suppressItemChangeEvent = true;
	setPos(itemPos);
	suppressItemChangeEvent = false;

	boundingRectangle.setX(-w/2);
	boundingRectangle.setY(-h/2);
	boundingRectangle.setWidth(w);
	boundingRectangle.setHeight(h);

	updateBorder();
}

/*!
	calculates the position of the label, when the position relative to the parent was specified (left, right, etc.)
*/
void TextLabelPrivate::updatePosition() {
	//determine the parent item
	QRectF parentRect;
	QGraphicsItem* parent = parentItem();
	if (parent) {
		parentRect = parent->boundingRect();
	} else {
		if (!scene())
			return;

		parentRect = scene()->sceneRect();
	}

	if (position.horizontalPosition != WorksheetElement::hPositionCustom) {
		if (position.horizontalPosition == WorksheetElement::hPositionLeft)
			position.point.setX( parentRect.x() );
		else if (position.horizontalPosition == WorksheetElement::hPositionCenter)
			position.point.setX( parentRect.x() + parentRect.width()/2 );
		else if (position.horizontalPosition == WorksheetElement::hPositionRight)
			position.point.setX( parentRect.x() + parentRect.width() );
	}

	if (position.verticalPosition != WorksheetElement::vPositionCustom) {
		if (position.verticalPosition == WorksheetElement::vPositionTop)
			position.point.setY( parentRect.y() );
		else if (position.verticalPosition == WorksheetElement::vPositionCenter)
			position.point.setY( parentRect.y() + parentRect.height()/2 );
		else if (position.verticalPosition == WorksheetElement::vPositionBottom)
			position.point.setY( parentRect.y() + parentRect.height() );
	}

	emit q->positionChanged(position);
}

/*!
	updates the static text.
 */
void TextLabelPrivate::updateText() {
	if (suppressRetransform)
		return;

	if (textWrapper.teXUsed) {
		TeXRenderer::Formatting format;
		format.fontColor = fontColor;
		format.backgroundColor = backgroundColor;
		format.fontSize = teXFont.pointSize();
		format.fontFamily = teXFont.family();
		format.dpi = teXImageResolution;
		QFuture<QImage> future = QtConcurrent::run(TeXRenderer::renderImageLaTeX, textWrapper.text, &teXRenderSuccessful, format);
		teXImageFutureWatcher.setFuture(future);

		//don't need to call retransorm() here since it is done in updateTeXImage
		//when the asynchronous rendering of the image is finished.
	} else {
		staticText.setText(textWrapper.text);

		//the size of the label was most probably changed.
		//call retransform() to recalculate the position and the bounding box of the label
		retransform();
	}
}

void TextLabelPrivate::updateTeXImage() {
	teXImage = teXImageFutureWatcher.result();
	retransform();
	DEBUG("teXRenderSuccessful =" << teXRenderSuccessful);
	emit q->teXImageUpdated(teXRenderSuccessful);
}

void TextLabelPrivate::updateBorder() {
	borderShapePath = QPainterPath();
	switch (borderShape) {
	case (TextLabel::NoBorder):
		break;
	case (TextLabel::BorderShape::Rect): {
		borderShapePath.addRect(boundingRectangle);
		break;
	}
	case (TextLabel::BorderShape::Ellipse): {
		const double xs = boundingRectangle.x();
		const double ys = boundingRectangle.y();
		const double w = boundingRectangle.width();
		const double h = boundingRectangle.height();
		borderShapePath.addEllipse(xs  - 0.1 * w, ys - 0.1 * h, 1.2 * w,  1.2 * h);
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
		break;
	}
	}

	recalcShapeAndBoundingRect();
}

bool TextLabelPrivate::swapVisible(bool on) {
	bool oldValue = isVisible();
	setVisible(on);
	emit q->changed();
	emit q->visibleChanged(on);
	return oldValue;
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
	if (borderShape != TextLabel::NoBorder) {
		labelShape.addPath(WorksheetElement::shapeFromPath(borderShapePath, borderPen));
		transformedBoundingRectangle = matrix.mapRect(labelShape.boundingRect());
	} else {
		labelShape.addRect(boundingRectangle);
		transformedBoundingRectangle = matrix.mapRect(boundingRectangle);
	}

	labelShape = matrix.map(labelShape);
}

void TextLabelPrivate::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) {
	Q_UNUSED(option)
	Q_UNUSED(widget)

	if (positionInvalid)
		return;

	if (textWrapper.text.isEmpty())
		return;

	painter->save();

	//draw the text
	painter->rotate(-rotationAngle);

	if (textWrapper.teXUsed) {
		if (boundingRect().width() != 0.0 &&  boundingRect().height() != 0.0)
			painter->drawImage(boundingRect(), teXImage);
	} else {
		// don't set fontColor to pen, because the color
		// is already in the html code
		//painter->setPen(fontColor);
		painter->scale(scaleFactor, scaleFactor);
		float w = staticText.size().width();
		float h = staticText.size().height();
		painter->drawStaticText(QPoint(-w/2,-h/2), staticText);
	}
	painter->restore();

	//draw the border
	if (borderShape != TextLabel::NoBorder) {
		painter->save();
		painter->rotate(-rotationAngle);
		painter->setPen(borderPen);
		painter->setOpacity(borderOpacity);
		painter->drawPath(borderShapePath);
		painter->restore();
	}

	if (m_hovered && !isSelected() && !m_printing) {
		painter->setPen(QPen(QApplication::palette().color(QPalette::Shadow), 2, Qt::SolidLine));
		painter->drawPath(labelShape);
	}

	if (isSelected() && !m_printing) {
		painter->setPen(QPen(QApplication::palette().color(QPalette::Highlight), 2, Qt::SolidLine));
		painter->drawPath(labelShape);
	}
}

QVariant TextLabelPrivate::itemChange(GraphicsItemChange change, const QVariant &value) {
	if (suppressItemChangeEvent)
		return value;

	if (change == QGraphicsItem::ItemPositionChange) {
		//convert item's center point in parent's coordinates
		TextLabel::PositionWrapper tempPosition;
		tempPosition.point = positionFromItemPosition(value.toPointF());
		tempPosition.horizontalPosition = WorksheetElement::hPositionCustom;
		tempPosition.verticalPosition = WorksheetElement::vPositionCustom;

		//emit the signals in order to notify the UI.
		//we don't set the position related member variables during the mouse movements.
		//this is done on mouse release events only.
		emit q->positionChanged(tempPosition);
	}

	return QGraphicsItem::itemChange(change, value);
}

void TextLabelPrivate::mouseReleaseEvent(QGraphicsSceneMouseEvent* event) {
	//convert position of the item in parent coordinates to label's position
	QPointF point = positionFromItemPosition(pos());
	if (point != position.point) {
		//position was changed -> set the position related member variables
		suppressRetransform = true;
		TextLabel::PositionWrapper tempPosition;
		tempPosition.point = point;
		tempPosition.horizontalPosition = TextLabel::hPositionCustom;
		tempPosition.verticalPosition = TextLabel::vPositionCustom;
		q->setPosition(tempPosition);
		suppressRetransform = false;
	}

	QGraphicsItem::mouseReleaseEvent(event);
}

void TextLabelPrivate::keyPressEvent(QKeyEvent* event) {
	if (event->key() == Qt::Key_Left || event->key() == Qt::Key_Right
		|| event->key() == Qt::Key_Up ||event->key() == Qt::Key_Down) {
		const int delta = 5;
		QPointF point = positionFromItemPosition(pos());
		WorksheetElement::PositionWrapper tempPosition;

		if (event->key() == Qt::Key_Left) {
			point.setX(point.x() - delta);
			tempPosition.horizontalPosition = WorksheetElement::hPositionCustom;
			tempPosition.verticalPosition = position.verticalPosition;
		} else if (event->key() == Qt::Key_Right) {
			point.setX(point.x() + delta);
			tempPosition.horizontalPosition = WorksheetElement::hPositionCustom;
			tempPosition.verticalPosition = position.verticalPosition;
		} else if (event->key() == Qt::Key_Up) {
			point.setY(point.y() - delta);
			tempPosition.horizontalPosition = position.horizontalPosition;
			tempPosition.verticalPosition = WorksheetElement::vPositionCustom;
		} else if (event->key() == Qt::Key_Down) {
			point.setY(point.y() + delta);
			tempPosition.horizontalPosition = position.horizontalPosition;
			tempPosition.verticalPosition = WorksheetElement::vPositionCustom;
		}

		tempPosition.point = point;
		q->setPosition(tempPosition);
	}

	QGraphicsItem::keyPressEvent(event);
}

/*!
 *	converts label's position to GraphicsItem's position.
 */
QPointF TextLabelPrivate::positionFromItemPosition(QPointF itemPos) {
	double x = itemPos.x();
	double y = itemPos.y();
	double w, h;
	QPointF tmpPosition;
	if (textWrapper.teXUsed) {
		w = teXImage.width()*scaleFactor;
		h = teXImage.height()*scaleFactor;
	} else {
		w = staticText.size().width()*scaleFactor;
		h = staticText.size().height()*scaleFactor;
	}

	//depending on the alignment, calculate the new position
	switch (horizontalAlignment) {
	case WorksheetElement::hAlignLeft:
		tmpPosition.setX(x + w/2);
		break;
	case WorksheetElement::hAlignCenter:
		tmpPosition.setX(x);
		break;
	case WorksheetElement::hAlignRight:
		tmpPosition.setX(x - w/2);
		break;
	}

	switch (verticalAlignment) {
	case WorksheetElement::vAlignTop:
		tmpPosition.setY(y + h/2);
		break;
	case WorksheetElement::vAlignCenter:
		tmpPosition.setY(y);
		break;
	case WorksheetElement::vAlignBottom:
		tmpPosition.setY(y - h/2);
		break;
	}

	return tmpPosition;
}

void TextLabelPrivate::contextMenuEvent(QGraphicsSceneContextMenuEvent* event) {
	q->createContextMenu()->exec(event->screenPos());
}

void TextLabelPrivate::hoverEnterEvent(QGraphicsSceneHoverEvent*) {
	if (!isSelected()) {
		m_hovered = true;
		emit q->hovered();
		update();
	}
}

void TextLabelPrivate::hoverLeaveEvent(QGraphicsSceneHoverEvent*) {
	if (m_hovered) {
		m_hovered = false;
		emit q->unhovered();
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

	//geometry
	writer->writeStartElement( "geometry" );
	writer->writeAttribute( "x", QString::number(d->position.point.x()) );
	writer->writeAttribute( "y", QString::number(d->position.point.y()) );
	writer->writeAttribute( "horizontalPosition", QString::number(d->position.horizontalPosition) );
	writer->writeAttribute( "verticalPosition", QString::number(d->position.verticalPosition) );
	writer->writeAttribute( "horizontalAlignment", QString::number(d->horizontalAlignment) );
	writer->writeAttribute( "verticalAlignment", QString::number(d->verticalAlignment) );
	writer->writeAttribute( "rotationAngle", QString::number(d->rotationAngle) );
	writer->writeAttribute( "visible", QString::number(d->isVisible()) );
	writer->writeEndElement();

	writer->writeStartElement( "text" );
	writer->writeCharacters( d->textWrapper.text );
	writer->writeEndElement();

	writer->writeStartElement( "format" );
	writer->writeAttribute( "teXUsed", QString::number(d->textWrapper.teXUsed) );
	WRITE_QFONT(d->teXFont);
	writer->writeAttribute( "fontColor_r", QString::number(d->fontColor.red()) );
	writer->writeAttribute( "fontColor_g", QString::number(d->fontColor.green()) );
	writer->writeAttribute( "fontColor_b", QString::number(d->fontColor.blue()) );
	writer->writeEndElement();

	//border
	writer->writeStartElement("border");
	writer->writeAttribute("borderShape", QString::number(d->borderShape));
	WRITE_QPEN(d->borderPen);
	writer->writeAttribute("borderOpacity", QString::number(d->borderOpacity));
	writer->writeEndElement();

	if (d->textWrapper.teXUsed) {
		writer->writeStartElement("teXImage");
		QByteArray ba;
		QBuffer buffer(&ba);
		buffer.open(QIODevice::WriteOnly);
		d->teXImage.save(&buffer, "PNG");
		writer->writeCharacters(ba.toBase64());
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
			attribs = reader->attributes();

			str = attribs.value("x").toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.subs("x").toString());
			else
				d->position.point.setX(str.toDouble());

			str = attribs.value("y").toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.subs("y").toString());
			else
				d->position.point.setY(str.toDouble());

			READ_INT_VALUE("horizontalPosition", position.horizontalPosition, WorksheetElement::HorizontalPosition);
			READ_INT_VALUE("verticalPosition", position.verticalPosition, WorksheetElement::VerticalPosition);
			READ_INT_VALUE("horizontalAlignment", horizontalAlignment, WorksheetElement::HorizontalAlignment);
			READ_INT_VALUE("verticalAlignment", verticalAlignment, WorksheetElement::VerticalAlignment);
			READ_DOUBLE_VALUE("rotationAngle", rotationAngle);

			str = attribs.value("visible").toString();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.subs("visible").toString());
			else
				d->setVisible(str.toInt());
		} else if (!preview && reader->name() == "text") {
			d->textWrapper.text = reader->readElementText();
		} else if (!preview && reader->name() == "format") {
			attribs = reader->attributes();

			READ_INT_VALUE("teXUsed", textWrapper.teXUsed, bool);
			READ_QFONT(d->teXFont);

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
		} else if (!preview && reader->name() == "teXImage") {
			reader->readNext();
			QString content = reader->text().toString().trimmed();
			QByteArray ba = QByteArray::fromBase64(content.toLatin1());
			teXImageFound = d->teXImage.loadFromData(ba);
		} else { // unknown element
			reader->raiseWarning(i18n("unknown element '%1'", reader->name().toString()));
			if (!reader->skipToEndElement()) return false;
		}
	}

	if (preview)
		return true;

	//in case we use latex and the image was stored (older versions of LabPlot didn't save the image)and loaded,
	//we just need to retransform.
	//otherwise, we set the static text and retransform in updateText()
	if ( !(d->textWrapper.teXUsed && teXImageFound) )
		d->updateText();
	else
		retransform();

	return true;
}

//##############################################################################
//#########################  Theme management ##################################
//##############################################################################
void TextLabel::loadThemeConfig(const KConfig& config) {
	Q_D(TextLabel);

	KConfigGroup group = config.group("Label");
	d->fontColor = group.readEntry("FontColor", QColor(Qt::white)); // used when it's latex text
	d->backgroundColor = group.readEntry("BackgroundColor", QColor(Qt::black)); // used when it's latex text

	if (!d->textWrapper.teXUsed) {
		// TODO: Replace QTextEdit by QTextDocument, because this does not contain the graphical stuff
		// to set the color in a html text, a qTextEdit must be used
		QTextEdit te;
		te.setHtml(d->textWrapper.text);
		te.selectAll();
		te.setTextColor(d->fontColor);
		//te.setTextBackgroundColor(backgroundColor); // for plain text no background color supported, due to bug https://bugreports.qt.io/browse/QTBUG-25420

		// update the text. also in the Widget to which is connected
		TextWrapper wrapper(te.toHtml(), false, true);
		setText(wrapper);
	} else
		setText(d->textWrapper.text);

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
