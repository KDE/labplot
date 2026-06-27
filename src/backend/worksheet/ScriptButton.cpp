/*
	File                 : ScriptButton.cpp
	Project              : LabPlot
	Description          : Worksheet element representing a button that executes a script
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2026 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "ScriptButton.h"
#include "ScriptButtonPrivate.h"
#include "backend/core/Project.h"
#include "backend/lib/XmlStreamReader.h"
#include "backend/lib/commandtemplates.h"
#include "backend/script/Script.h"

#include <QPushButton>
#include <QGraphicsProxyWidget>
#include <QGraphicsSceneMouseEvent>

#include <KConfig>
#include <KConfigGroup>
#include <KLocalizedString>

/**
 * \class ScriptButton
 * \brief Worksheet element representing a clickable button that executes a script.
 *
 * \ingroup worksheet
 */

ScriptButton::ScriptButton(const QString& name) : WorksheetElement(name, new ScriptButtonPrivate(this), AspectType::ScriptButton) {
	init();
}

void ScriptButton::init() {
	Q_D(ScriptButton);

	KConfig config;
	KConfigGroup group = config.group(QStringLiteral("ScriptButton"));

	// Load saved defaults
	d->text = group.readEntry(i18n("Execute"), d->text);
	d->width = group.readEntry(QStringLiteral("width"), d->width);
	d->height = group.readEntry(QStringLiteral("height"), d->height);
	d->backgroundColor = group.readEntry(QStringLiteral("backgroundColor"), d->backgroundColor);
	d->font = group.readEntry(QStringLiteral("font"), d->font);
	d->textColor = group.readEntry(QStringLiteral("textColor"), d->textColor);

	d->borderWidth = group.readEntry(QStringLiteral("borderWidth"), d->borderWidth);
	d->borderColor = group.readEntry(QStringLiteral("borderColor"), d->borderColor);

	// geometry
	d->update();
}

// no need to delete the d-pointer here - it inherits from QGraphicsItem
// and is deleted during the cleanup in QGraphicsScene
ScriptButton::~ScriptButton() = default;

/*!
	Returns an icon to be used in the project explorer.
*/
QIcon ScriptButton::icon() const {
	return QIcon::fromTheme(QStringLiteral("code-context"));
}

QWidget* ScriptButton::widget() const {
	Q_D(const ScriptButton);
	return d->button;
}

void ScriptButton::retransform() {
	Q_D(ScriptButton);
	d->retransform();
}

void ScriptButton::handleResize(double /*horizontalRatio*/, double /*verticalRatio*/, bool /*pageResize*/) {

}

// ##############################################################################
// ##########################  getter methods  ##################################
// ##############################################################################
BASIC_SHARED_D_READER_IMPL(ScriptButton, const Script*, script, script)
BASIC_SHARED_D_READER_IMPL(ScriptButton, QString, text, text)
BASIC_SHARED_D_READER_IMPL(ScriptButton, int, width, width)
BASIC_SHARED_D_READER_IMPL(ScriptButton, int, height, height)
BASIC_SHARED_D_READER_IMPL(ScriptButton, QColor, backgroundColor, backgroundColor)
BASIC_SHARED_D_READER_IMPL(ScriptButton, QColor, textColor, textColor)
BASIC_SHARED_D_READER_IMPL(ScriptButton, QFont, font, font)

// ##############################################################################
// #################  setter methods and undo commands ##########################
// ##############################################################################
// STD_SETTER_CMD_IMPL_F_S(ScriptButton, SetScript, Script, script, update)
void ScriptButton::setScript(const Script* script) {
	Q_D(ScriptButton);
	if (script != d->script) {
		d->script = const_cast<Script*>(script);
		Q_EMIT scriptChanged(const_cast<Script*>(script));
		Q_EMIT changed();
	}
}

STD_SETTER_CMD_IMPL_F_S(ScriptButton, SetText, QString, text, update)
void ScriptButton::setText(const QString& text) {
	Q_D(ScriptButton);
	if (text != d->text)
		exec(new ScriptButtonSetTextCmd(d, text, ki18n("%1: set text")));
}

STD_SETTER_CMD_IMPL_F_S(ScriptButton, SetWidth, int, width, update)
void ScriptButton::setWidth(int width) {
	Q_D(ScriptButton);
	if (width != d->width)
		exec(new ScriptButtonSetWidthCmd(d, width, ki18n("%1: set width")));
}

STD_SETTER_CMD_IMPL_F_S(ScriptButton, SetHeight, int, height, update)
void ScriptButton::setHeight(int height) {
	Q_D(ScriptButton);
	if (height != d->height)
		exec(new ScriptButtonSetHeightCmd(d, height, ki18n("%1: set height")));
}

STD_SETTER_CMD_IMPL_F_S(ScriptButton, SetBackgroundColor, QColor, backgroundColor, update)
void ScriptButton::setBackgroundColor(const QColor& color) {
	Q_D(ScriptButton);
	if (color != d->backgroundColor)
		exec(new ScriptButtonSetBackgroundColorCmd(d, color, ki18n("%1: set background color")));
}

STD_SETTER_CMD_IMPL_F_S(ScriptButton, SetFont, QFont, font, update)
void ScriptButton::setFont(const QFont& font) {
	Q_D(ScriptButton);
	if (font != d->font)
		exec(new ScriptButtonSetFontCmd(d, font, ki18n("%1: set font")));
}

STD_SETTER_CMD_IMPL_F_S(ScriptButton, SetTextColor, QColor, textColor, update)
void ScriptButton::setTextColor(const QColor& color) {
	Q_D(ScriptButton);
	if (color != d->textColor)
		exec(new ScriptButtonSetTextColorCmd(d, color, ki18n("%1: set text color")));
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
ScriptButtonPrivate::ScriptButtonPrivate(ScriptButton* owner)
	: WorksheetElementPrivate(owner)
	, q(owner) {
	setFlag(QGraphicsItem::ItemIsSelectable);
	setFlag(QGraphicsItem::ItemIsMovable);
	setFlag(QGraphicsItem::ItemSendsGeometryChanges);
	setFlag(QGraphicsItem::ItemIsFocusable);
	setAcceptHoverEvents(true);

	button = new QPushButton();
	button->setAttribute(Qt::WA_TransparentForMouseEvents, true);
	proxy = new QGraphicsProxyWidget(this);
	proxy->setWidget(button);
	proxy->setAcceptedMouseButtons(Qt::NoButton);
	
	// Parent item handles click/drag interactions.
}

QString ScriptButtonPrivate::name() const {
	return q->name();
}

void ScriptButtonPrivate::clicked() {
	if (script)
		script->runScript();
}

void ScriptButtonPrivate::retransform() {
	const bool suppress = suppressRetransform || q->isLoading();
	trackRetransformCalled(suppress);
	if (suppress)
		return;

	m_boundingRectangle.setX(-width / 2.0);
	m_boundingRectangle.setY(-height / 2.0);
	m_boundingRectangle.setWidth(width);
	m_boundingRectangle.setHeight(height);

	if (proxy)
		proxy->setPos(m_boundingRectangle.topLeft());

	recalcShapeAndBoundingRect();
	updatePosition();
}

void ScriptButtonPrivate::recalcShapeAndBoundingRect() {
	prepareGeometryChange();
	m_shape = QPainterPath();
	m_shape.addRect(m_boundingRectangle);
}

void ScriptButtonPrivate::mouseReleaseEvent(QGraphicsSceneMouseEvent* event) {
	const bool moveStarted = m_moveStarted;
	WorksheetElementPrivate::mouseReleaseEvent(event);

	if (!moveStarted && event->button() == Qt::LeftButton)
		clicked();
}

void ScriptButtonPrivate::update() {
	if (!button)
		return;
	
	button->setText(text);
	button->setFixedSize(width, height);
	button->setFont(font);
	
	// Apply styling via stylesheet
	QString styleSheet = QLatin1String("QPushButton{background-color: %1; color: %2; border: %3px solid %4;}")
							.arg(backgroundColor.name())
							.arg(textColor.name())
							.arg(borderWidth)
							.arg(borderColor.name());
	button->setStyleSheet(styleSheet);

	retransform();
}

//##############################################################################
//##################  Serialization/Deserialization  ###########################
//##############################################################################
//! Save as XML
void ScriptButton::save(QXmlStreamWriter* writer) const {
	Q_D(const ScriptButton);

	writer->writeStartElement(QStringLiteral("scripButton"));
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

	// script
	if (d->script) {
		writer->writeStartElement(QStringLiteral("script"));
		writer->writeAttribute(QStringLiteral("path"), d->script->path());
		writer->writeEndElement();
	}

	writer->writeEndElement(); // close "scriptButton" section
}

//! Load from XML
bool ScriptButton::load(XmlStreamReader* reader, bool preview) {
	Q_D(ScriptButton);

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
		} else { // unknown element
			reader->raiseUnknownElementWarning();
			if (!reader->skipToEndElement())
				return false;
		}
	}

	// Resolve script reference after loading
	/*
	if (!scriptPath.isEmpty() && !preview) {
		auto* proj = project();
		if (proj) {
			auto* aspect = proj->child<AbstractAspect>(scriptPath, AbstractAspect::ChildIndexFlag::Recursive);
			if (aspect && aspect->type() == AspectType::Script)
				setScript(static_cast<Script*>(aspect));
		}
	}
	*/
	if (!preview) {
		d->update();
		// retransform();
	}

	return true;
}
