/*
	File                 : TextLabel.cpp
	Project              : LabPlot
	Description          : Text label supporting reach text and latex formatting
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2009 Tilman Benkert <thzs@gmx.net>
	SPDX-FileCopyrightText: 2012-2023 Alexander Semke <alexander.semke@web.de>
	SPDX-FileCopyrightText: 2019-2022 Stefan Gerlach <stefan.gerlach@uni.kn>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "TextLabel.h"
#include "TextLabelPrivate.h"
#include "Worksheet.h"
#include "backend/core/Project.h"
#include "backend/core/Settings.h"
#include "backend/lib/XmlStreamReader.h"
#include "backend/lib/commandtemplates.h"
#include "backend/worksheet/plots/PlotArea.h"
#include "backend/worksheet/plots/cartesian/CartesianCoordinateSystem.h"
#include "backend/worksheet/plots/cartesian/CartesianPlot.h"
#include "kdefrontend/GuiTools.h"

#include <QApplication>
#include <QBuffer>
#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QKeyEvent>
#include <QMenu>
#include <QPainter>
#include <QTextCharFormat>
#include <QTextCursor>
#include <QTextDocument>
#include <QtConcurrent/QtConcurrentRun>

#include <KConfig>
#include <KConfigGroup>
#include <KLocalizedString>
#include <QIcon>

#ifdef HAVE_DISCOUNT
extern "C" {
#include <mkdio.h>
}
#endif

class ScaledTextItem : public QGraphicsTextItem {
public:
	explicit ScaledTextItem(QGraphicsItem* parent = nullptr)
		: QGraphicsTextItem(parent) {
	}

protected:
	void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override {
#if DEBUG_TEXTLABEL_BOUNDING_RECT
		painter->setPen(QColor(Qt::GlobalColor::green));
		painter->drawRect(boundingRect());
#endif

#if DEBUG_TEXTLABEL_BOUNDING_RECT
		painter->setPen(QColor(Qt::GlobalColor::black));
		painter->drawRect(QRectF(-5, -5, 10, 10));
#endif
		QGraphicsTextItem::paint(painter, option, widget);
	}

private:
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
	: WorksheetElement(name, new TextLabelPrivate(this), AspectType::TextLabel)
	, m_type(type) {
	init();
}

TextLabel::TextLabel(const QString& name, TextLabelPrivate* dd, Type type)
	: WorksheetElement(name, dd, AspectType::TextLabel)
	, m_type(type) {
	init();
}

TextLabel::TextLabel(const QString& name, CartesianPlot* plot, Type type)
	: WorksheetElement(name, new TextLabelPrivate(this), AspectType::TextLabel)
	, m_type(type) {
	m_plot = plot;
	cSystem = dynamic_cast<const CartesianCoordinateSystem*>(m_plot->coordinateSystem(m_cSystemIndex));
	init();
}

void TextLabel::init() {
	QString groupName;
	switch (m_type) {
	case Type::General:
		groupName = QStringLiteral("TextLabel");
		break;
	case Type::PlotTitle:
		groupName = QStringLiteral("PlotTitle");
		break;
	case Type::AxisTitle:
		groupName = QStringLiteral("AxisTitle");
		break;
	case Type::PlotLegendTitle:
		groupName = QStringLiteral("PlotLegendTitle");
		break;
	case Type::InfoElementLabel:
		groupName = QStringLiteral("InfoElementLabel");
	}

	const KConfig config;
	DEBUG("	config has group \"" << STDSTRING(groupName) << "\": " << config.hasGroup(groupName));
	// group is always valid if you call config.group(..;
	KConfigGroup group;
	if (config.hasGroup(groupName))
		group = config.group(groupName);

	Q_D(TextLabel);
	d->position.point = QPointF(0, 0);
	if (m_type == Type::PlotTitle || m_type == Type::PlotLegendTitle) {
		d->position.verticalPosition = WorksheetElement::VerticalPosition::Top;
		d->position.horizontalPosition = WorksheetElement::HorizontalPosition::Center;
		d->verticalAlignment = WorksheetElement::VerticalAlignment::Top;
	} else if (m_type == Type::AxisTitle) {
		d->position.horizontalPosition = WorksheetElement::HorizontalPosition::Center;
		d->position.verticalPosition = WorksheetElement::VerticalPosition::Center;
	}

	KConfigGroup conf = Settings::group(QLatin1String("Settings_Worksheet"));
	const auto& engine = conf.readEntry(QLatin1String("LaTeXEngine"), "");
	if (engine == QLatin1String("lualatex"))
		d->teXFont.setFamily(QLatin1String("Latin Modern Roman"));

	// read settings from config if group exists
	if (group.isValid()) {
		// properties common to all types
		d->textWrapper.mode = static_cast<TextLabel::Mode>(group.readEntry("Mode", static_cast<int>(d->textWrapper.mode)));
		d->teXFont.setFamily(group.readEntry("TeXFontFamily", d->teXFont.family()));
		d->teXFont.setPointSize(group.readEntry("TeXFontSize", d->teXFont.pointSize()));
		d->fontColor = group.readEntry("TeXFontColor", d->fontColor);
		d->backgroundColor = group.readEntry("TeXBackgroundColor", d->backgroundColor);
		d->setRotation(group.readEntry("Rotation", d->rotation()));

		// border
		d->borderShape = (TextLabel::BorderShape)group.readEntry("BorderShape", (int)d->borderShape);
		d->borderPen = QPen(group.readEntry("BorderColor", d->borderPen.color()),
							group.readEntry("BorderWidth", d->borderPen.width()),
							(Qt::PenStyle)group.readEntry("BorderStyle", (int)(d->borderPen.style())));
		d->borderOpacity = group.readEntry("BorderOpacity", d->borderOpacity);

		// position and alignment relevant properties
		d->position.point.setX(group.readEntry("PositionXValue", 0.));
		d->position.point.setY(group.readEntry("PositionYValue", 0.));
		d->position.horizontalPosition = (HorizontalPosition)group.readEntry("PositionX", (int)d->position.horizontalPosition);
		d->position.verticalPosition = (VerticalPosition)group.readEntry("PositionY", (int)d->position.verticalPosition);
		d->horizontalAlignment = (WorksheetElement::HorizontalAlignment)group.readEntry("HorizontalAlignment", static_cast<int>(d->horizontalAlignment));
		d->verticalAlignment = (WorksheetElement::VerticalAlignment)group.readEntry("VerticalAlignment", static_cast<int>(d->verticalAlignment));
		if (cSystem && cSystem->isValid())
			d->positionLogical = cSystem->mapSceneToLogical(d->position.point, AbstractCoordinateSystem::MappingFlag::SuppressPageClipping);
	}
	d->updatePosition();

	DEBUG(Q_FUNC_INFO << ", default/run time image resolution: " << d->teXImageResolution << '/' << QApplication::primaryScreen()->physicalDotsPerInchX());

	connect(&d->teXImageFutureWatcher, &QFutureWatcher<QByteArray>::finished, this, &TextLabel::updateTeXImage);
}

// no need to delete the d-pointer here - it inherits from QGraphicsItem
// and is deleted during the cleanup in QGraphicsScene
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

	double ratio = 0;
	if (horizontalRatio > 1.0 || verticalRatio > 1.0)
		ratio = std::max(horizontalRatio, verticalRatio);
	else
		ratio = std::min(horizontalRatio, verticalRatio);

	Q_D(TextLabel);
	d->teXFont.setPointSizeF(d->teXFont.pointSizeF() * ratio);
	d->updateText();

	// TODO: doesn't seem to work
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
	switch (text().mode) {
	case Mode::Markdown:
		return QIcon::fromTheme(QLatin1String("text-x-markdown"));
		break;
	case Mode::LaTeX:
		return QIcon::fromTheme(QLatin1String("text-x-tex"));
		break;
	case Mode::Text:
	default:
		return QIcon::fromTheme(QLatin1String("draw-text"));
		// return QIcon::fromTheme(QLatin1String("text-x-plain"));
	}
}

QMenu* TextLabel::createContextMenu() {
	QMenu* menu = WorksheetElement::createContextMenu();
	QAction* firstAction = menu->actions().at(1); // skip the first action because of the "title-action"

	if (!visibilityAction) {
		visibilityAction = new QAction(i18n("Visible"), this);
		visibilityAction->setCheckable(true);
		connect(visibilityAction, &QAction::triggered, this, &TextLabel::changeVisibility);
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
void TextLabel::setText(const TextWrapper& textWrapper) {
	Q_D(TextLabel);
	// DEBUG("********************\n" << Q_FUNC_INFO << ", old/new mode = " << (int)d->textWrapper.mode << " " << (int)textWrapper.mode)
	// DEBUG("\nOLD TEXT = " << STDSTRING(d->textWrapper.text) << '\n')
	// DEBUG("NEW TEXT = " << STDSTRING(textWrapper.text) << '\n')

	// QDEBUG("COLORS: color =" << d->fontColor << ", background color =" << d->backgroundColor)

	if ((textWrapper.text != d->textWrapper.text) || (textWrapper.mode != d->textWrapper.mode)
		|| ((d->textWrapper.allowPlaceholder || textWrapper.allowPlaceholder) && (textWrapper.textPlaceholder != d->textWrapper.textPlaceholder))
		|| textWrapper.allowPlaceholder != d->textWrapper.allowPlaceholder) {
		bool oldEmpty = d->textWrapper.text.isEmpty();
		if (textWrapper.mode == TextLabel::Mode::Text && !textWrapper.text.isEmpty()) {
			// DEBUG("SET TEXTWRAPPER")
			TextWrapper tw = textWrapper;

			QTextEdit pte(d->textWrapper.text); // te with previous text
			// restore formatting when text changes or switching back to text mode
			if (d->textWrapper.mode != TextLabel::Mode::Text || oldEmpty || pte.toPlainText().isEmpty()) {
				// DEBUG("RESTORE FORMATTING")
				QTextEdit te(d->textWrapper.text);
				te.selectAll();
				te.setText(textWrapper.text);
				te.selectAll();
				te.setTextColor(d->fontColor);
				te.setTextBackgroundColor(d->backgroundColor);

				tw.text = te.toHtml();
				// DEBUG("\nTW TEXT = " << STDSTRING(tw.text) << std::endl)
			}

			exec(new TextLabelSetTextCmd(d, tw, ki18n("%1: set label text")));
		} else
			exec(new TextLabelSetTextCmd(d, textWrapper, ki18n("%1: set label text")));
		// If previously the text was empty, the bounding rect is zero
		// therefore the alignment did not work properly.
		// If text is added, the bounding rectangle is updated
		// and then the position must be changed to consider alignment
		if (oldEmpty)
			d->updatePosition();
	}

	// DEBUG(Q_FUNC_INFO << " DONE\n***********************")
}

STD_SETTER_CMD_IMPL_F_S(TextLabel, SetPlaceholderText, TextLabel::TextWrapper, textWrapper, updateText)
void TextLabel::setPlaceholderText(const TextWrapper& textWrapper) {
	Q_D(TextLabel);
	if ((textWrapper.textPlaceholder != d->textWrapper.textPlaceholder) || (textWrapper.mode != d->textWrapper.mode))
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

// Border
STD_SETTER_CMD_IMPL_F_S(TextLabel, SetBorderShape, TextLabel::BorderShape, borderShape, updateBorder)
void TextLabel::setBorderShape(TextLabel::BorderShape shape) {
	Q_D(TextLabel);
	if (shape != d->borderShape)
		exec(new TextLabelSetBorderShapeCmd(d, shape, ki18n("%1: set border shape")));
}

STD_SETTER_CMD_IMPL_F_S(TextLabel, SetBorderPen, QPen, borderPen, update)
void TextLabel::setBorderPen(const QPen& pen) {
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

// misc

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
	return d->m_gluePointsTransformed.length();
}

void TextLabel::updateTeXImage() {
	Q_D(TextLabel);
	d->updateTeXImage();
}

// ##############################################################################
// ####################### Private implementation ###############################
// ##############################################################################
TextLabelPrivate::TextLabelPrivate(TextLabel* owner)
	: WorksheetElementPrivate(owner)
	, q(owner) {
	setFlag(QGraphicsItem::ItemIsSelectable);
	setFlag(QGraphicsItem::ItemIsMovable);
	setFlag(QGraphicsItem::ItemSendsGeometryChanges);
	setFlag(QGraphicsItem::ItemIsFocusable);
	setAcceptHoverEvents(true);

	// scaling:
	// we need to scale from the font size specified in points to scene units.
	// furthermore, we create the tex-image in a higher resolution than usual desktop resolution
	//  -> take this into account
	// m_textItem is only used for the normal text not for latex. So the scale is only needed for the
	// normal text, for latex the generated image will be shown directly
	m_textItem = new ScaledTextItem(this);
	m_textItem->setScale(Worksheet::convertToSceneUnits(1, Worksheet::Unit::Point));
	m_textItem->setTextInteractionFlags(Qt::NoTextInteraction);
}

/*!
 * \brief TextLabelPrivate::size
 * \return Size and position of the TextLabel in scene coords
 */
QRectF TextLabelPrivate::size() {
	double w, h;
	if (textWrapper.mode == TextLabel::Mode::LaTeX) {
		// image size is in pixel, convert to scene units
		w = teXImage.width() * teXImageScaleFactor;
		h = teXImage.height() * teXImageScaleFactor;
	} else {
		// size is in points, convert to scene units
		//  TODO: the shift and scaling is just a workaround to avoid the big bounding box
		//  s.a. TextLabelPrivate::updateBoundingRect()

		// double xShift = 23., yScale = 0.8;
		// better scaling for multiline Markdown
		// if (textWrapper.mode == TextLabel::Mode::Markdown && textWrapper.text.contains(QLatin1Char('\n')))
		//	yScale = 0.95;
		// see updateBoundingRect()
		w = m_textItem->boundingRect().width() * m_textItem->scale();
		h = m_textItem->boundingRect().height() * m_textItem->scale();
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
	if (m_gluePointsTransformed.isEmpty())
		return boundingRectangle.center();

	if (m_gluePointsTransformed.length() == 1)
		return mapParentToPlotArea(mapToParent(m_gluePointsTransformed.at(0).point));

	QPointF point = mapParentToPlotArea(mapToParent(m_gluePointsTransformed.at(0).point));
	QPointF nearestPoint = point;
	double distance2 = pow(point.x() - scenePoint.x(), 2) + pow(point.y() - scenePoint.y(), 2);
	// assumption, more than one point available
	for (int i = 1; i < m_gluePointsTransformed.length(); i++) {
		point = mapParentToPlotArea(mapToParent(m_gluePointsTransformed.at(i).point));
		double distance2_temp = pow(point.x() - scenePoint.x(), 2) + pow(point.y() - scenePoint.y(), 2);
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

	if (m_gluePointsTransformed.isEmpty() || index > m_gluePointsTransformed.length()) {
		pos = boundingRectangle.center();
		name = QLatin1String("center");
	} else if (index < 0) {
		pos = m_gluePointsTransformed.at(0).point;
		name = m_gluePointsTransformed.at(0).name;
	} else {
		pos = m_gluePointsTransformed.at(index).point;
		name = m_gluePointsTransformed.at(index).name;
	}

	return {mapParentToPlotArea(mapToParent(pos)), name};
}

/*!
	calculates the position and the bounding box of the label. Called on geometry or text changes.
 */
void TextLabelPrivate::retransform() {
	const bool suppress = suppressRetransform || q->isLoading();
	trackRetransformCalled(suppress);
	if (suppress)
		return;

	updatePosition(); // needed, because CartesianPlot calls retransform if some operations are done
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

	if (q->m_type == TextLabel::Type::AxisTitle) {
		// In an axis element, the label is part of the bounding rect of the axis
		// so it is not possible to align with the bounding rect
		QPointF p = position.point;
		suppressItemChangeEvent = true;
		setPos(p);
		suppressItemChangeEvent = false;

		Q_EMIT q->positionChanged(position);

		// the position in scene coordinates was changed, calculate the position in logical coordinates
		if (q->cSystem) {
			if (!coordinateBindingEnabled) {
				QPointF pos = q->align(p, boundingRectangle, horizontalAlignment, verticalAlignment, false);
				positionLogical = q->cSystem->mapSceneToLogical(pos, AbstractCoordinateSystem::MappingFlag::SuppressPageClipping);
			}
			Q_EMIT q->positionLogicalChanged(positionLogical);
		}
	} else
		WorksheetElementPrivate::updatePosition();
}

/*!
	updates the static text.
 */
void TextLabelPrivate::updateText() {
	// DEBUG(Q_FUNC_INFO)
	if (suppressRetransform)
		return;

	switch (textWrapper.mode) {
	case TextLabel::Mode::Text: {
		// DEBUG(Q_FUNC_INFO << ", TEXT = " << STDSTRING(textWrapper.text))

		// use colors when text not empty and not already defined in text
		if (!textWrapper.text.isEmpty() && !textWrapper.text.contains(QLatin1String(" color:"))) {
			QTextEdit te(textWrapper.text);
			te.selectAll();
			te.setTextColor(fontColor);

			// don't set background color (not used if not inside text)
			textWrapper.text = te.toHtml();
		}

		m_textItem->show();
		m_textItem->setHtml(textWrapper.text);

		// the size of the label was most probably changed,
		// recalculate the bounding box of the label
		updateBoundingRect();
		break;
	}
	case TextLabel::Mode::LaTeX: {
		m_textItem->hide();
		TeXRenderer::Formatting format;
		format.fontColor = fontColor;
		format.backgroundColor = backgroundColor;
		format.fontSize = teXFont.pointSize();
		format.fontFamily = teXFont.family();
		format.dpi = teXImageResolution;
		QFuture<QByteArray> future = QtConcurrent::run(TeXRenderer::renderImageLaTeX, textWrapper.text, &teXRenderResult, format);
		teXImageFutureWatcher.setFuture(future);

		// don't need to call retransform() here since it is done in updateTeXImage
		// when the asynchronous rendering of the image is finished.
		break;
	}
	case TextLabel::Mode::Markdown: {
#ifdef HAVE_DISCOUNT
		auto mdCharArray = textWrapper.text.toUtf8();
#ifdef HAVE_DISCOUNT3
		MMIOT* mdHandle = mkd_string(mdCharArray.data(), mdCharArray.size() + 1, nullptr);

		mkd_flag_t* v3flags = mkd_flags();
		mkd_set_flag_num(v3flags, MKD_LATEX);
		mkd_set_flag_num(v3flags, MKD_FENCEDCODE);
		mkd_set_flag_num(v3flags, MKD_GITHUBTAGS);

		if (!mkd_compile(mdHandle, v3flags)) {
#else
		MMIOT* mdHandle = mkd_string(mdCharArray.data(), mdCharArray.size() + 1, 0);

		unsigned int flags = MKD_LATEX | MKD_FENCEDCODE | MKD_GITHUBTAGS;
		if (!mkd_compile(mdHandle, flags)) {
#endif
			DEBUG(Q_FUNC_INFO << ", Failed to compile the markdown document");
			mkd_cleanup(mdHandle);
			return;
		}
		char* htmlDocument;
		int htmlSize = mkd_document(mdHandle, &htmlDocument);
		QString html = QString::fromUtf8(htmlDocument, htmlSize);

		mkd_cleanup(mdHandle);

		// use QTextEdit to add other global properties (font size and colors)
		QTextEdit te;
		te.setHtml(html);
		te.selectAll();
		te.setTextColor(fontColor);
		te.setFontPointSize(teXFont.pointSize());
		te.setTextBackgroundColor(backgroundColor);

		m_textItem->setHtml(te.toHtml());
		m_textItem->show();

		updateBoundingRect();
#endif
	}
	}
}

void TextLabelPrivate::updateBoundingRect() {
	// DEBUG(Q_FUNC_INFO)
	// determine the size of the label in scene units.
	double w, h;
	if (textWrapper.mode == TextLabel::Mode::LaTeX) {
		// image size is in pixel, convert to scene units.
		// the image is scaled so we have a good image quality when the worksheet was zoomed,
		// for the bounding rect we need to scale back since it's scaled again in paint() when drawing the rect
		w = teXImage.width() * teXImageScaleFactor / zoomFactor;
		h = teXImage.height() * teXImageScaleFactor / zoomFactor;
	} else {
		// size is in points, convert to scene units
		// QDEBUG(" BOUNDING RECT = " << m_textItem->boundingRect())
		//  TODO: the shift and scaling is just a workaround to avoid the big bounding box
		//  s.a. TextLabelPrivate::size()

		// double xShift = 23., yScale = 0.8;
		//  better scaling for multiline Markdown
		// if (textWrapper.mode == TextLabel::Mode::Markdown && textWrapper.text.contains(QLatin1Char('\n')))
		//	yScale = 0.95;
		w = m_textItem->boundingRect().width() * m_textItem->scale(); // - xShift;
		h = m_textItem->boundingRect().height() * m_textItem->scale(); // * yScale;
		m_textItem->setPos(QPointF(-w / 2, -h / 2));
	}

	// DEBUG(Q_FUNC_INFO << ", scale factor = " << scaleFactor << ", w/h = " << w << " / " << h)
	boundingRectangle.setX(-w / 2);
	boundingRectangle.setY(-h / 2);
	boundingRectangle.setWidth(w);
	boundingRectangle.setHeight(h);

	updateBorder();
}

void TextLabelPrivate::updateTeXImage() {
	if (zoomFactor == -1.0) {
		// the view was not zoomed after the label was added so the zoom factor is not set yet.
		// determine the current zoom factor in the view and use it
		auto* worksheet = static_cast<const Worksheet*>(q->parent(AspectType::Worksheet));
		if (!worksheet)
			return;
		zoomFactor = worksheet->zoomFactor();
	}
	teXPdfData = teXImageFutureWatcher.result();
	teXImage = GuiTools::imageFromPDFData(teXPdfData, zoomFactor);
	updateBoundingRect();
	DEBUG(Q_FUNC_INFO << ", TeX renderer successful = " << teXRenderResult.successful);
	Q_EMIT q->teXImageUpdated(teXRenderResult);
}

void TextLabelPrivate::updateBorder() {
	using GluePoint = TextLabel::GluePoint;

	borderShapePath = QPainterPath();
	switch (borderShape) {
	case (TextLabel::BorderShape::NoBorder): {
		m_gluePoints.clear();
		m_gluePoints.append(GluePoint(QPointF(boundingRectangle.x() + boundingRectangle.width() / 2, boundingRectangle.y()), QStringLiteral("top")));
		m_gluePoints.append(GluePoint(QPointF(boundingRectangle.x() + boundingRectangle.width(), boundingRectangle.y() + boundingRectangle.height() / 2),
									  QStringLiteral("right")));
		m_gluePoints.append(GluePoint(QPointF(boundingRectangle.x() + boundingRectangle.width() / 2, boundingRectangle.y() + boundingRectangle.height()),
									  QStringLiteral("bottom")));
		m_gluePoints.append(GluePoint(QPointF(boundingRectangle.x(), boundingRectangle.y() + boundingRectangle.height() / 2), QStringLiteral("left")));
		break;
	}
	case (TextLabel::BorderShape::Rect): {
		borderShapePath.addRect(boundingRectangle);

		m_gluePoints.clear();
		m_gluePoints.append(GluePoint(QPointF(boundingRectangle.x() + boundingRectangle.width() / 2, boundingRectangle.y()), QStringLiteral("top")));
		m_gluePoints.append(GluePoint(QPointF(boundingRectangle.x() + boundingRectangle.width(), boundingRectangle.y() + boundingRectangle.height() / 2),
									  QStringLiteral("right")));
		m_gluePoints.append(GluePoint(QPointF(boundingRectangle.x() + boundingRectangle.width() / 2, boundingRectangle.y() + boundingRectangle.height()),
									  QStringLiteral("bottom")));
		m_gluePoints.append(GluePoint(QPointF(boundingRectangle.x(), boundingRectangle.y() + boundingRectangle.height() / 2), QStringLiteral("left")));
		break;
	}
	case (TextLabel::BorderShape::Ellipse): {
		const double xs = boundingRectangle.x();
		const double ys = boundingRectangle.y();
		const double w = boundingRectangle.width();
		const double h = boundingRectangle.height();
		const QRectF ellipseRect(xs - 0.1 * w, ys - 0.1 * h, 1.2 * w, 1.2 * h);
		borderShapePath.addEllipse(ellipseRect);

		m_gluePoints.clear();
		m_gluePoints.append(GluePoint(QPointF(ellipseRect.x() + ellipseRect.width() / 2, ellipseRect.y()), QStringLiteral("top")));
		m_gluePoints.append(GluePoint(QPointF(ellipseRect.x() + ellipseRect.width(), ellipseRect.y() + ellipseRect.height() / 2), QStringLiteral("right")));
		m_gluePoints.append(GluePoint(QPointF(ellipseRect.x() + ellipseRect.width() / 2, ellipseRect.y() + ellipseRect.height()), QStringLiteral("bottom")));
		m_gluePoints.append(GluePoint(QPointF(ellipseRect.x(), ellipseRect.y() + ellipseRect.height() / 2), QStringLiteral("left")));
		break;
	}
	case (TextLabel::BorderShape::RoundSideRect): {
		const double xs = boundingRectangle.x();
		const double ys = boundingRectangle.y();
		const double w = boundingRectangle.width();
		const double h = boundingRectangle.height();
		borderShapePath.moveTo(xs, ys);
		borderShapePath.lineTo(xs + w, ys);
		borderShapePath.quadTo(xs + w + h / 2, ys + h / 2, xs + w, ys + h);
		borderShapePath.lineTo(xs, ys + h);
		borderShapePath.quadTo(xs - h / 2, ys + h / 2, xs, ys);

		m_gluePoints.clear();
		m_gluePoints.append(GluePoint(QPointF(xs + w / 2, ys), QStringLiteral("top")));
		m_gluePoints.append(GluePoint(QPointF(xs + w + h / 2, ys + h / 2), QStringLiteral("right")));
		m_gluePoints.append(GluePoint(QPointF(xs + w / 2, ys + h), QStringLiteral("bottom")));
		m_gluePoints.append(GluePoint(QPointF(xs - h / 2, ys + h / 2), QStringLiteral("left")));
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
		m_gluePoints.append(GluePoint(QPointF(xs + w / 2, ys), QStringLiteral("top")));
		m_gluePoints.append(GluePoint(QPointF(xs + w, ys + h / 2), QStringLiteral("right")));
		m_gluePoints.append(GluePoint(QPointF(xs + w / 2, ys + h), QStringLiteral("bottom")));
		m_gluePoints.append(GluePoint(QPointF(xs, ys + h / 2), QStringLiteral("left")));
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
		m_gluePoints.append(GluePoint(QPointF(xs + w / 2, ys - 0.3 * h), QStringLiteral("top")));
		m_gluePoints.append(GluePoint(QPointF(xs + w + 0.3 * h, ys + h / 2), QStringLiteral("right")));
		m_gluePoints.append(GluePoint(QPointF(xs + w / 2, ys + h + 0.3 * h), QStringLiteral("bottom")));
		m_gluePoints.append(GluePoint(QPointF(xs - 0.3 * h, ys + h / 2), QStringLiteral("left")));
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
		m_gluePoints.append(GluePoint(QPointF(xs + w / 2, ys - 0.2 * h), QStringLiteral("top")));
		m_gluePoints.append(GluePoint(QPointF(xs + w + 0.2 * h, ys + h / 2), QStringLiteral("right")));
		m_gluePoints.append(GluePoint(QPointF(xs + w / 2, ys + h + 0.2 * h), QStringLiteral("bottom")));
		m_gluePoints.append(GluePoint(QPointF(xs - 0.2 * h, ys + h / 2), QStringLiteral("left")));
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
		m_gluePoints.append(GluePoint(QPointF(xs + w / 2, ys - 0.1 * h), QStringLiteral("top")));
		m_gluePoints.append(GluePoint(QPointF(xs + w + 0.15 * h, ys + h / 2), QStringLiteral("right")));
		m_gluePoints.append(GluePoint(QPointF(xs + w / 2, ys + h), QStringLiteral("bottom")));
		m_gluePoints.append(GluePoint(QPointF(xs, ys + h / 2), QStringLiteral("left")));
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
		m_gluePoints.append(GluePoint(QPointF(xs + w / 2, ys - h * 0.2), QStringLiteral("top")));
		m_gluePoints.append(GluePoint(QPointF(xs + w, ys + h / 2), QStringLiteral("right")));
		m_gluePoints.append(GluePoint(QPointF(xs + w / 2, ys + h), QStringLiteral("bottom")));
		m_gluePoints.append(GluePoint(QPointF(xs, ys + h / 2), QStringLiteral("left")));
		break;
	}
	case (TextLabel::BorderShape::DownPointingRectangle): {
		const double xs = boundingRectangle.x();
		const double ys = boundingRectangle.y();
		const double w = boundingRectangle.width();
		const double h = boundingRectangle.height();
		borderShapePath.moveTo(xs + h * 0.2, ys);
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
		m_gluePoints.append(GluePoint(QPointF(xs + w / 2, ys), QStringLiteral("top")));
		m_gluePoints.append(GluePoint(QPointF(xs + w, ys + h / 2), QStringLiteral("right")));
		m_gluePoints.append(GluePoint(QPointF(xs + w / 2, ys + h + 0.2 * h), QStringLiteral("bottom")));
		m_gluePoints.append(GluePoint(QPointF(xs, ys + h / 2), QStringLiteral("left")));
		break;
	}
	case (TextLabel::BorderShape::LeftPointingRectangle): {
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
		borderShapePath.lineTo(xs, ys + h / 2 + 0.2 * h);
		borderShapePath.lineTo(xs - 0.2 * h, ys + h / 2);
		borderShapePath.lineTo(xs, ys + h / 2 - 0.2 * h);
		borderShapePath.lineTo(xs, ys + 0.2 * h);
		borderShapePath.quadTo(xs, ys, xs + 0.2 * h, ys);

		m_gluePoints.clear();
		m_gluePoints.append(GluePoint(QPointF(xs + w / 2, ys), QStringLiteral("top")));
		m_gluePoints.append(GluePoint(QPointF(xs + w, ys + h / 2), QStringLiteral("right")));
		m_gluePoints.append(GluePoint(QPointF(xs + w / 2, ys + h), QStringLiteral("bottom")));
		m_gluePoints.append(GluePoint(QPointF(xs - 0.2 * h, ys + h / 2), QStringLiteral("left")));
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
		m_gluePoints.append(GluePoint(QPointF(xs + w / 2, ys), QStringLiteral("top")));
		m_gluePoints.append(GluePoint(QPointF(xs + w + 0.2 * h, ys + h / 2), QStringLiteral("right")));
		m_gluePoints.append(GluePoint(QPointF(xs + w / 2, ys + h), QStringLiteral("bottom")));
		m_gluePoints.append(GluePoint(QPointF(xs, ys + h / 2), QStringLiteral("left")));
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

	QTransform matrix;
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
	m_gluePointsTransformed.clear();
	for (auto& gPoint : m_gluePoints)
		m_gluePointsTransformed.append(TextLabel::GluePoint(matrix.map(gPoint.point), gPoint.name));

	Q_EMIT q->changed();
}

void TextLabelPrivate::paint(QPainter* painter, const QStyleOptionGraphicsItem* /*option*/, QWidget*) {
	if (positionInvalid || textWrapper.text.isEmpty())
		return;

	// draw the text
	painter->save();
	switch (textWrapper.mode) {
	case TextLabel::Mode::LaTeX: {
		painter->setRenderHint(QPainter::SmoothPixmapTransform);
		if (boundingRectangle.width() != 0.0 && boundingRectangle.height() != 0.0)
			painter->drawImage(boundingRectangle, teXImage);
		break;
	}
	case TextLabel::Mode::Text:
	case TextLabel::Mode::Markdown: {
		// nothing to do here, the painting is done in the ScaledTextItem child
		break;
	}
	}
	painter->restore();

	// draw the border
	if (borderShape != TextLabel::BorderShape::NoBorder) {
		painter->save();
		painter->setPen(borderPen);
		painter->setOpacity(borderOpacity);

		painter->drawPath(borderShapePath);
		painter->restore();
	}

	// TODO: move the handling of m_hovered and the logic below
	// to draw the selectiong/hover box to WorksheetElementPrivate
	// so there is no need to duplicate this code in Plot, Label, Image, etc.
	const bool selected = isSelected();
	const bool hovered = (m_hovered && !selected);
	if ((hovered || selected) && !q->isPrinting()) {
		static double penWidth = 2.;
		const QRectF& br = boundingRect();
		const qreal width = br.width();
		const qreal height = br.height();
		const QRectF rect = QRectF(-width / 2 + penWidth / 2, -height / 2 + penWidth / 2, width - penWidth, height - penWidth);

		if (hovered)
			painter->setPen(QPen(QApplication::palette().color(QPalette::Shadow), penWidth));
		else
			painter->setPen(QPen(QApplication::palette().color(QPalette::Highlight), penWidth));

		painter->drawRect(rect);
	}

#if DEBUG_TEXTLABEL_BOUNDING_RECT
	painter->setPen(QColor(Qt::GlobalColor::red));
	painter->drawRect(boundingRect());
#endif

#if DEBUG_TEXTLABEL_GLUEPOINTS
	// just for debugging
	painter->setPen(QColor(Qt::GlobalColor::red));
	QRectF gluePointRect(0, 0, 10, 10);
	for (int i = 0; i < m_gluePointsTransformed.length(); i++) {
		gluePointRect.moveTo(m_gluePointsTransformed[i].point.x() - gluePointRect.width() / 2,
							 m_gluePointsTransformed[i].point.y() - gluePointRect.height() / 2);
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
	// don't show any context menu if the label is hidden which is the case
	// for example for axis and plot title labels. For such objects the context menu
	// of their parents, i.e. of axis and plot, is used.
	if (!q->hidden())
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

// ##############################################################################
// ##################  Serialization/Deserialization  ###########################
// ##############################################################################
//! Save as XML
void TextLabel::save(QXmlStreamWriter* writer) const {
	Q_D(const TextLabel);

	writer->writeStartElement(QStringLiteral("textLabel"));
	writeBasicAttributes(writer);
	writeCommentElement(writer);

	writer->writeStartElement(QStringLiteral("geometry"));
	WorksheetElement::save(writer);
	writer->writeEndElement();

	writer->writeStartElement(QStringLiteral("text"));
	writer->writeCharacters(d->textWrapper.text);
	writer->writeEndElement();

	if (!d->textWrapper.textPlaceholder.isEmpty()) {
		writer->writeStartElement(QStringLiteral("textPlaceholder"));
		writer->writeCharacters(d->textWrapper.textPlaceholder);
		writer->writeEndElement();
	}

	writer->writeStartElement(QStringLiteral("format"));
	writer->writeAttribute(QStringLiteral("placeholder"), QString::number(d->textWrapper.allowPlaceholder));
	writer->writeAttribute(QStringLiteral("mode"), QString::number(static_cast<int>(d->textWrapper.mode)));
	WRITE_QFONT(d->teXFont);
	WRITE_QCOLOR2(d->fontColor, "fontColor");
	WRITE_QCOLOR2(d->backgroundColor, "backgroundColor");
	writer->writeEndElement();

	// border
	writer->writeStartElement(QStringLiteral("border"));
	writer->writeAttribute(QStringLiteral("borderShape"), QString::number(static_cast<int>(d->borderShape)));
	WRITE_QPEN(d->borderPen);
	writer->writeAttribute(QStringLiteral("borderOpacity"), QString::number(d->borderOpacity));
	writer->writeEndElement();

	if (d->textWrapper.mode == TextLabel::Mode::LaTeX) {
		writer->writeStartElement(QStringLiteral("teXPdfData"));
		writer->writeCharacters(QLatin1String(d->teXPdfData.toBase64()));
		writer->writeEndElement();
	}

	writer->writeEndElement(); // close "textLabel" section
}

//! Load from XML
bool TextLabel::load(XmlStreamReader* reader, bool preview) {
	if (!readBasicAttributes(reader))
		return false;

	Q_D(TextLabel);
	QXmlStreamAttributes attribs;
	QString str;

	while (!reader->atEnd()) {
		reader->readNext();
		if (reader->isEndElement() && reader->name() == QLatin1String("textLabel"))
			break;

		if (!reader->isStartElement())
			continue;

		if (!preview && reader->name() == QLatin1String("comment")) {
			if (!readCommentElement(reader))
				return false;
		} else if (!preview && reader->name() == QLatin1String("geometry")) {
			WorksheetElement::load(reader, preview);
		} else if (!preview && reader->name() == QLatin1String("text"))
			d->textWrapper.text = reader->readElementText();
		else if (!preview && reader->name() == QLatin1String("textPlaceholder"))
			d->textWrapper.textPlaceholder = reader->readElementText();
		else if (!preview && reader->name() == QLatin1String("format")) {
			attribs = reader->attributes();

			if (Project::xmlVersion() < 4) {
				str = attribs.value(QStringLiteral("teXUsed")).toString();
				d->textWrapper.mode = static_cast<TextLabel::Mode>(str.toInt());
			} else
				READ_INT_VALUE("mode", textWrapper.mode, TextLabel::Mode);

			str = attribs.value(QStringLiteral("placeholder")).toString();
			if (!str.isEmpty())
				d->textWrapper.allowPlaceholder = str.toInt();

			READ_QFONT(d->teXFont);
			READ_QCOLOR2(d->fontColor, "fontColor");
			READ_QCOLOR2(d->backgroundColor, "backgroundColor");
		} else if (!preview && reader->name() == QLatin1String("border")) {
			attribs = reader->attributes();
			READ_INT_VALUE("borderShape", borderShape, BorderShape);
			READ_QPEN(d->borderPen);
			READ_DOUBLE_VALUE("borderOpacity", borderOpacity);
		} else if (!preview && reader->name() == QLatin1String("teXPdfData")) {
			reader->readNext();
			QString content = reader->text().toString().trimmed();
			d->teXPdfData = QByteArray::fromBase64(content.toLatin1());
			d->teXImage = GuiTools::imageFromPDFData(d->teXPdfData);
		} else { // unknown element
			reader->raiseUnknownElementWarning();
			if (!reader->skipToEndElement())
				return false;
		}
	}

	if (preview)
		return true;

	// in case we use latex and the image was stored (older versions of LabPlot didn't save the image)and loaded,
	// we just need to call updateBoundingRect() to calculate the new rect.
	// otherwise, we set the static text and call updateBoundingRect() in updateText()
	if (!(d->textWrapper.mode == TextLabel::Mode::LaTeX && !d->teXPdfData.isEmpty()))
		d->updateText();
	else {
		d->m_textItem->hide();
		d->zoomFactor = 1.0; // on load the view is not zoomed yet
		d->updateBoundingRect();
	}

	return true;
}

// ##############################################################################
// #########################  Theme management ##################################
// ##############################################################################
void TextLabel::loadThemeConfig(const KConfig& config) {
	DEBUG(Q_FUNC_INFO << ", label = " << STDSTRING(name()))
	Q_D(TextLabel);

	KConfigGroup group = config.group("Label");
	// TODO: dark mode support?
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
		// call updateText() to re-render the LaTeX-image with the new text colors
		d->updateText();
	}

	// otherwise when changing theme while the textlabel dock is visible, the
	// color comboboxes do not change the color
	backgroundColorChanged(d->backgroundColor);
	fontColorChanged(d->fontColor);

	group = config.group("CartesianPlot");
	QPen pen = this->borderPen();
	pen.setColor(group.readEntry("BorderColor", pen.color()));
	pen.setStyle((Qt::PenStyle)(group.readEntry("BorderStyle", (int)pen.style())));
	pen.setWidthF(group.readEntry("BorderWidth", pen.widthF()));
	this->setBorderPen(pen);
	this->setBorderOpacity(group.readEntry("BorderOpacity", this->borderOpacity()));
}

void TextLabel::saveThemeConfig(const KConfig& config) {
	KConfigGroup group = config.group("Label");
	// TODO
	// 	group.writeEntry("TeXFontColor", (QColor) this->fontColor());
}
