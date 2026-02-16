/*
	File                 : ScriptWorksheetElement.cpp
	Project              : LabPlot
	Description          : Worksheet element representing a button that executes a script
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2026 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "ScriptWorksheetElement.h"
#include "ScriptWorksheetElementPrivate.h"
#include "Worksheet.h"
#include "backend/lib/XmlStreamReader.h"
#include "backend/lib/commandtemplates.h"
#include "backend/script/Script.h"

#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QIcon>
#include <QMenu>
#include <QPainter>
#include <QPushButton>

#include <KConfig>
#include <KConfigGroup>
#include <KLocalizedString>

/**
 * \class ScriptWorksheetElement
 * \brief Worksheet element representing a clickable button that executes a script.
 *
 * The button can be added to \c Worksheet or to \c CartesianPlot and is aligned relative to the specified position.
 * The position can be either specified by providing the x- and y- coordinates
 * in parent's coordinate system, or by specifying one of the predefined position
 * flags (\c HorizontalPosition, \c VerticalPosition).
 * \ingroup worksheet
 */

ScriptWorksheetElement::ScriptWorksheetElement(const QString& name)
	: WorksheetElement(name, AspectType::ScriptWorksheetElement) {
	m_private = new ScriptWorksheetElementPrivate(this);
	init();
}

void ScriptWorksheetElement::init() {
	auto* d = static_cast<ScriptWorksheetElementPrivate*>(m_private);

	KConfig config;
	KConfigGroup group = config.group(QStringLiteral("ScriptWorksheetElement"));

	// Load saved defaults
	d->text = group.readEntry(QStringLiteral("text"), d->text);
	d->width = group.readEntry(QStringLiteral("width"), d->width);
	d->height = group.readEntry(QStringLiteral("height"), d->height);
	d->backgroundColor = group.readEntry(QStringLiteral("backgroundColor"), d->backgroundColor);
	d->borderColor = group.readEntry(QStringLiteral("borderColor"), d->borderColor);
	d->borderWidth = group.readEntry(QStringLiteral("borderWidth"), d->borderWidth);
	d->font = group.readEntry(QStringLiteral("font"), d->font);
	d->textColor = group.readEntry(QStringLiteral("textColor"), d->textColor);

	// geometry
	m_position.point.setX(group.readEntry(QStringLiteral("PositionXValue"), 0.));
	m_position.point.setY(group.readEntry(QStringLiteral("PositionYValue"), 0.));
	m_position.horizontalPosition =
		(WorksheetElement::HorizontalPosition)group.readEntry(QStringLiteral("PositionX"), (int)WorksheetElement::HorizontalPosition::Center);
	m_position.verticalPosition =
		(WorksheetElement::VerticalPosition)group.readEntry(QStringLiteral("PositionY"), (int)WorksheetElement::VerticalPosition::Center);
	m_horizontalAlignment =
		(WorksheetElement::HorizontalAlignment)group.readEntry(QStringLiteral("HorizontalAlignment"), (int)WorksheetElement::HorizontalAlignment::Center);
	m_verticalAlignment =
		(WorksheetElement::VerticalAlignment)group.readEntry(QStringLiteral("VerticalAlignment"), (int)WorksheetElement::VerticalAlignment::Center);
	
	d->updateButton();
}

// no need to delete the d-pointer here - it inherits from QGraphicsItem
// and is deleted during the cleanup in QGraphicsScene
ScriptWorksheetElement::~ScriptWorksheetElement() = default;

void ScriptWorksheetElement::setParentGraphicsItem(QGraphicsItem* item) {
	auto* d = static_cast<ScriptWorksheetElementPrivate*>(m_private);
	d->setParentItem(item);
	d->updatePosition();
}

void ScriptWorksheetElement::retransform() {
	auto* d = static_cast<ScriptWorksheetElementPrivate*>(m_private);
	d->retransform();
}

void ScriptWorksheetElement::handleResize(double /*horizontalRatio*/, double /*verticalRatio*/, bool /*pageResize*/) {
	// For now, keep fixed size - could implement scaling if needed
}

QGraphicsItem* ScriptWorksheetElement::graphicsItem() const {
	return m_private;
}

/*!
	Returns an icon to be used in the project explorer.
*/
QIcon ScriptWorksheetElement::icon() const {
	return QIcon::fromTheme(QStringLiteral("code-context"));
}

/* ============================ getter methods ================= */
QString ScriptWorksheetElement::text() const {
	auto* d = static_cast<const ScriptWorksheetElementPrivate*>(m_private);
	return d->text;
}

int ScriptWorksheetElement::width() const {
	auto* d = static_cast<const ScriptWorksheetElementPrivate*>(m_private);
	return d->width;
}

int ScriptWorksheetElement::height() const {
	auto* d = static_cast<const ScriptWorksheetElementPrivate*>(m_private);
	return d->height;
}

QColor ScriptWorksheetElement::backgroundColor() const {
	auto* d = static_cast<const ScriptWorksheetElementPrivate*>(m_private);
	return d->backgroundColor;
}

QColor ScriptWorksheetElement::borderColor() const {
	auto* d = static_cast<const ScriptWorksheetElementPrivate*>(m_private);
	return d->borderColor;
}

double ScriptWorksheetElement::borderWidth() const {
	auto* d = static_cast<const ScriptWorksheetElementPrivate*>(m_private);
	return d->borderWidth;
}

QFont ScriptWorksheetElement::font() const {
	auto* d = static_cast<const ScriptWorksheetElementPrivate*>(m_private);
	return d->font;
}

QColor ScriptWorksheetElement::textColor() const {
	auto* d = static_cast<const ScriptWorksheetElementPrivate*>(m_private);
	return d->textColor;
}

Script* ScriptWorksheetElement::script() const {
	auto* d = static_cast<const ScriptWorksheetElementPrivate*>(m_private);
	return d->script;
}

/* ============================ setter methods and undo commands ================= */
STD_SETTER_CMD_IMPL_F_S(ScriptWorksheetElement, SetText, QString, text, updateButton)
void ScriptWorksheetElement::setText(const QString& text) {
	auto* d = static_cast<ScriptWorksheetElementPrivate*>(m_private);
	if (text != d->text)
		exec(new ScriptWorksheetElementSetTextCmd(d, text, ki18n("%1: set text")));
}

STD_SETTER_CMD_IMPL_F_S(ScriptWorksheetElement, SetWidth, int, width, updateButton)
void ScriptWorksheetElement::setWidth(int width) {
	auto* d = static_cast<ScriptWorksheetElementPrivate*>(m_private);
	if (width != d->width)
		exec(new ScriptWorksheetElementSetWidthCmd(d, width, ki18n("%1: set width")));
}

STD_SETTER_CMD_IMPL_F_S(ScriptWorksheetElement, SetHeight, int, height, updateButton)
void ScriptWorksheetElement::setHeight(int height) {
	auto* d = static_cast<ScriptWorksheetElementPrivate*>(m_private);
	if (height != d->height)
		exec(new ScriptWorksheetElementSetHeightCmd(d, height, ki18n("%1: set height")));
}

STD_SETTER_CMD_IMPL_F_S(ScriptWorksheetElement, SetBackgroundColor, QColor, backgroundColor, updateButton)
void ScriptWorksheetElement::setBackgroundColor(const QColor& color) {
	auto* d = static_cast<ScriptWorksheetElementPrivate*>(m_private);
	if (color != d->backgroundColor)
		exec(new ScriptWorksheetElementSetBackgroundColorCmd(d, color, ki18n("%1: set background color")));
}

STD_SETTER_CMD_IMPL_F_S(ScriptWorksheetElement, SetBorderColor, QColor, borderColor, updateButton)
void ScriptWorksheetElement::setBorderColor(const QColor& color) {
	auto* d = static_cast<ScriptWorksheetElementPrivate*>(m_private);
	if (color != d->borderColor)
		exec(new ScriptWorksheetElementSetBorderColorCmd(d, color, ki18n("%1: set border color")));
}

STD_SETTER_CMD_IMPL_F_S(ScriptWorksheetElement, SetBorderWidth, double, borderWidth, updateButton)
void ScriptWorksheetElement::setBorderWidth(double width) {
	auto* d = static_cast<ScriptWorksheetElementPrivate*>(m_private);
	if (width != d->borderWidth)
		exec(new ScriptWorksheetElementSetBorderWidthCmd(d, width, ki18n("%1: set border width")));
}

STD_SETTER_CMD_IMPL_F_S(ScriptWorksheetElement, SetFont, QFont, font, updateButton)
void ScriptWorksheetElement::setFont(const QFont& font) {
	auto* d = static_cast<ScriptWorksheetElementPrivate*>(m_private);
	if (font != d->font)
		exec(new ScriptWorksheetElementSetFontCmd(d, font, ki18n("%1: set font")));
}

STD_SETTER_CMD_IMPL_F_S(ScriptWorksheetElement, SetTextColor, QColor, textColor, updateButton)
void ScriptWorksheetElement::setTextColor(const QColor& color) {
	auto* d = static_cast<ScriptWorksheetElementPrivate*>(m_private);
	if (color != d->textColor)
		exec(new ScriptWorksheetElementSetTextColorCmd(d, color, ki18n("%1: set text color")));
}

void ScriptWorksheetElement::setScript(Script* script) {
	auto* d = static_cast<ScriptWorksheetElementPrivate*>(m_private);
	if (script != d->script) {
		d->script = script;
		Q_EMIT scriptChanged(script);
	}
}

void ScriptWorksheetElement::executeScript() {
	auto* d = static_cast<ScriptWorksheetElementPrivate*>(m_private);
	if (d->script)
		d->script->runScript();
}

//##############################################################################
//#################################  SLOTS  ####################################
//##############################################################################

//##############################################################################
//######  SLOTs for changes triggered via QActions in the context menu  #######
//##############################################################################

//##############################################################################
//####################### Private implementation ###############################
//##############################################################################
ScriptWorksheetElementPrivate::ScriptWorksheetElementPrivate(ScriptWorksheetElement* owner)
	: QGraphicsProxyWidget()
	, q(owner) {
	setFlag(QGraphicsItem::ItemIsSelectable);
	setFlag(QGraphicsItem::ItemIsMovable);
	setFlag(QGraphicsItem::ItemSendsGeometryChanges);
	
	// Create the actual QPushButton widget
	button = new QPushButton();
	button->setText(text);
	button->setFixedSize(width, height);
	
	// Set the button as the widget for this proxy
	setWidget(button);
	
	// Connect button click to script execution
	connect(button, &QPushButton::clicked, this, &ScriptWorksheetElementPrivate::buttonClicked);
}

void ScriptWorksheetElementPrivate::buttonClicked() {
	q->executeScript();
}

void ScriptWorksheetElementPrivate::retransform() {
	updatePosition();
}

void ScriptWorksheetElementPrivate::updatePosition() {
	QRectF rect(0, 0, width, height);
	QPointF position = q->relativePosToParentPos(q->position());
	position = q->align(position, rect, q->horizontalAlignment(), q->verticalAlignment(), true);
	setPos(position);
}

void ScriptWorksheetElementPrivate::updateButton() {
	if (!button)
		return;
	
	button->setText(text);
	button->setFixedSize(width, height);
	button->setFont(font);
	
	// Apply styling via stylesheet
	QString styleSheet = QString("QPushButton { "
		"background-color: %1; "
		"color: %2; "
		"border: %3px solid %4; "
		"}")
		.arg(backgroundColor.name())
		.arg(textColor.name())
		.arg(borderWidth)
		.arg(borderColor.name());
	
	button->setStyleSheet(styleSheet);
	
	updatePosition();
}

QRectF ScriptWorksheetElementPrivate::boundingRect() const {
	return QRectF(0, 0, width, height);
}

void ScriptWorksheetElementPrivate::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) {
	// Let the proxy widget handle the painting
	QGraphicsProxyWidget::paint(painter, option, widget);
}

//##############################################################################
//##################  Serialization/Deserialization  ###########################
//##############################################################################
//! Save as XML
void ScriptWorksheetElement::save(QXmlStreamWriter* writer) const {
	auto* d = static_cast<const ScriptWorksheetElementPrivate*>(m_private);

	writer->writeStartElement(QStringLiteral("scriptWorksheetElement"));
	writeBasicAttributes(writer);
	writeCommentElement(writer);

	// geometry
	writer->writeStartElement(QStringLiteral("geometry"));
	WRITE_QCOLOR(d->backgroundColor);
	WRITE_QCOLOR(d->borderColor);
	writer->writeAttribute(QStringLiteral("borderWidth"), QString::number(d->borderWidth));
	WRITE_QFONT(d->font);
	WRITE_QCOLOR(d->textColor);
	writer->writeAttribute(QStringLiteral("text"), d->text);
	writer->writeAttribute(QStringLiteral("width"), QString::number(d->width));
	writer->writeAttribute(QStringLiteral("height"), QString::number(d->height));
	writer->writeEndElement();

	// script reference
	if (d->script) {
		writer->writeStartElement(QStringLiteral("script"));
		writer->writeAttribute(QStringLiteral("path"), d->script->path());
		writer->writeEndElement();
	}

	WorksheetElement::save(writer);

	writer->writeEndElement(); // close "scriptWorksheetElement" section
}

//! Load from XML
bool ScriptWorksheetElement::load(XmlStreamReader* reader, bool preview) {
	auto* d = static_cast<ScriptWorksheetElementPrivate*>(m_private);

	if (!readBasicAttributes(reader))
		return false;

	QXmlStreamAttributes attribs;
	QString str;
	QString scriptPath;

	while (!reader->atEnd()) {
		reader->readNext();
		if (reader->isEndElement() && reader->name() == QLatin1String("scriptWorksheetElement"))
			break;

		if (!reader->isStartElement())
			continue;

		if (reader->name() == QLatin1String("comment")) {
			if (!readCommentElement(reader))
				return false;
		} else if (!preview && reader->name() == QLatin1String("geometry")) {
			attribs = reader->attributes();

			READ_QCOLOR(d->backgroundColor);
			READ_QCOLOR(d->borderColor);
			READ_DOUBLE_VALUE("borderWidth", borderWidth);
			READ_QFONT(d->font);
			READ_QCOLOR(d->textColor);
			READ_STRING_VALUE("text", text);
			READ_INT_VALUE("width", width, int);
			READ_INT_VALUE("height", height, int);
		} else if (!preview && reader->name() == QLatin1String("script")) {
			attribs = reader->attributes();
			scriptPath = attribs.value(QStringLiteral("path")).toString();
		} else if (reader->name() == QLatin1String("worksheetElement")) {
			if (!WorksheetElement::load(reader, preview))
				return false;
		} else { // unknown element
			reader->raiseUnknownElementWarning();
			if (!reader->skipToEndElement())
				return false;
		}
	}

	// Resolve script reference after loading
	if (!scriptPath.isEmpty() && !preview) {
		auto* project = this->project();
		if (project) {
			auto* aspect = project->child<AbstractAspect>(scriptPath, AbstractAspect::ChildIndexFlag::Recursive);
			if (aspect && aspect->type() == AspectType::Script)
				setScript(static_cast<Script*>(aspect));
		}
	}

	if (!preview) {
		d->updateButton();
		retransform();
	}

	return true;
}

//! Save theme settings
void ScriptWorksheetElement::saveThemeConfig(const KConfig& config) {
	KConfigGroup group = config.group(QStringLiteral("ScriptWorksheetElement"));
	// Save current settings as defaults
	// (Currently no theme-specific settings)
}

//! Load theme settings
void ScriptWorksheetElement::loadThemeConfig(const KConfig& config) {
	auto* d = static_cast<ScriptWorksheetElementPrivate*>(m_private);

	KConfigGroup group = config.group(QStringLiteral("ScriptWorksheetElement"));
	// Load theme settings if needed
	// (Currently no theme-specific settings)

	d->updateButton();
}
