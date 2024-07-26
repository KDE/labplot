/*
	File                 : LabelWidget.cc
	Project              : LabPlot
	Description          : label settings widget
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2008-2024 Alexander Semke <alexander.semke@web.de>
	SPDX-FileCopyrightText: 2012-2022 Stefan Gerlach <stefan.gerlach@uni-konstanz.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "LabelWidget.h"
#include "backend/core/Settings.h"
#include "backend/worksheet/Background.h"
#include "backend/worksheet/Worksheet.h"
#include "backend/worksheet/plots/PlotArea.h"
#include "backend/worksheet/plots/cartesian/Axis.h"
#include "backend/worksheet/plots/cartesian/CartesianPlot.h"
#include "backend/worksheet/plots/cartesian/CartesianPlotLegend.h"
#include "kdefrontend/GuiTools.h"
#include "kdefrontend/dockwidgets/BaseDock.h"
#include "tools/TeXRenderer.h"

#include <KCharSelect>
#include <KLocalizedString>
#ifdef HAVE_KF5_SYNTAX_HIGHLIGHTING
#include <KSyntaxHighlighting/Definition>
#include <KSyntaxHighlighting/SyntaxHighlighter>
#include <KSyntaxHighlighting/Theme>
#endif
#include <KMessageWidget>

#include <QFile>
#include <QMenu>
#include <QSettings>
#include <QSplitter>
#include <QStandardItemModel>
#include <QTextDocumentFragment>
#include <QWidgetAction>

#include <gsl/gsl_const_cgs.h>

/*!
 * Setting label property without changing the content. This is needed,
 * because the text is html formatted and therefore setting properties
 * is not that easy
 */
#define SETLABELTEXTPROPERTY(TextEditFunction, TextEditArgument)                                                                                               \
	auto cursor = ui.teLabel->textCursor();                                                                                                                    \
	int cursorAnchor = cursor.anchor(), cursorPos = cursor.position();                                                                                         \
	/* move position with right allows only positive numbers (from left to right) */                                                                           \
	if (cursorAnchor > cursorPos)                                                                                                                              \
		qSwap(cursorAnchor, cursorPos);                                                                                                                        \
	bool cursorHasSelection = cursor.hasSelection();                                                                                                           \
	if (!cursorHasSelection)                                                                                                                                   \
		ui.teLabel->selectAll();                                                                                                                               \
                                                                                                                                                               \
	ui.teLabel->TextEditFunction(TextEditArgument);                                                                                                            \
	QTextEdit te;                                                                                                                                              \
	for (auto& label : m_labelsList) {                                                                                                                         \
		TextLabel::TextWrapper w = label->text();                                                                                                              \
		if (w.allowPlaceholder)                                                                                                                                \
			te.setText(w.textPlaceholder);                                                                                                                     \
		else                                                                                                                                                   \
			te.setText(w.text);                                                                                                                                \
		if (!cursorHasSelection)                                                                                                                               \
			te.selectAll();                                                                                                                                    \
		else {                                                                                                                                                 \
			/* Set cursor as it is in the ui.teLabel*/                                                                                                         \
			auto c = te.textCursor();                                                                                                                          \
			c.setPosition(cursorAnchor);                                                                                                                       \
			c.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor, cursorPos - cursorAnchor);                                                             \
			te.setTextCursor(c);                                                                                                                               \
		}                                                                                                                                                      \
		te.TextEditFunction(TextEditArgument);                                                                                                                 \
		if (w.allowPlaceholder)                                                                                                                                \
			w.textPlaceholder = te.toHtml();                                                                                                                   \
		else                                                                                                                                                   \
			w.text = te.toHtml();                                                                                                                              \
		label->setText(w);                                                                                                                                     \
	}                                                                                                                                                          \
                                                                                                                                                               \
	if (!cursorHasSelection) {                                                                                                                                 \
		cursor.clearSelection();                                                                                                                               \
		ui.teLabel->setTextCursor(cursor);                                                                                                                     \
	}

/*!
	\class LabelWidget
	\brief Widget for editing the properties of a TextLabel object, mostly used in an appropriate dock widget.

	In order the properties of the label to be shown, \c loadConfig() has to be called with the corresponding KConfigGroup
	(settings for a label in *Plot, Axis etc. or for an independent label on the worksheet).

	\ingroup kdefrontend
 */
LabelWidget::LabelWidget(QWidget* parent)
	: BaseDock(parent)
	, m_dateTimeMenu(new QMenu(this)) {
	ui.setupUi(this);
	setBaseWidgets(ui.leName, ui.teComment);
	setVisibilityWidgets(ui.chbVisible);

	// set the minimum size of the text edit widget to one row of a QLabel
	ui.teLabel->setMinimumHeight(ui.lName->height());

	// adjust the layout margins
	if (auto* l = dynamic_cast<QGridLayout*>(layout())) {
		l->setContentsMargins(2, 2, 2, 2);
		l->setHorizontalSpacing(2);
		l->setVerticalSpacing(2);
	}

	const KConfigGroup group = Settings::group(QStringLiteral("Settings_General"));
	m_units = (BaseDock::Units)group.readEntry("Units", (int)BaseDock::Units::Metric);
	if (m_units == BaseDock::Units::Imperial)
		m_worksheetUnit = Worksheet::Unit::Inch;

	m_dateTimeMenu->setSeparatorsCollapsible(false); // we don't want the first separator to be removed

	QString msg = i18n("Use logical instead of absolute coordinates to specify the position on the plot");
	ui.chbBindLogicalPos->setToolTip(msg);

	ui.sbBorderWidth->setMinimum(0);

	// Icons
	ui.tbFontBold->setIcon(QIcon::fromTheme(QStringLiteral("format-text-bold")));
	ui.tbFontItalic->setIcon(QIcon::fromTheme(QStringLiteral("format-text-italic")));
	ui.tbFontUnderline->setIcon(QIcon::fromTheme(QStringLiteral("format-text-underline")));
	ui.tbFontStrikeOut->setIcon(QIcon::fromTheme(QStringLiteral("format-text-strikethrough")));
	ui.tbFontSuperScript->setIcon(QIcon::fromTheme(QStringLiteral("format-text-superscript")));
	ui.tbFontSubScript->setIcon(QIcon::fromTheme(QStringLiteral("format-text-subscript")));
	ui.tbAlignLeft->setIcon(QIcon::fromTheme(QStringLiteral("format-justify-left")));
	ui.tbAlignCenter->setIcon(QIcon::fromTheme(QStringLiteral("format-justify-center")));
	ui.tbAlignRight->setIcon(QIcon::fromTheme(QStringLiteral("format-justify-right")));
	ui.tbAlignJustify->setIcon(QIcon::fromTheme(QStringLiteral("format-justify-fill")));
	ui.tbSymbols->setIcon(QIcon::fromTheme(QStringLiteral("labplot-format-text-symbol")));
	ui.tbDateTime->setIcon(QIcon::fromTheme(QStringLiteral("chronometer")));

	ui.tbFontBold->setToolTip(i18n("Bold"));
	ui.tbFontItalic->setToolTip(i18n("Italic"));
	ui.tbFontUnderline->setToolTip(i18n("Underline"));
	ui.tbFontStrikeOut->setToolTip(i18n("Strike Out"));
	ui.tbFontSuperScript->setToolTip(i18n("Super Script"));
	ui.tbFontSubScript->setToolTip(i18n("Sub-Script"));
	ui.tbAlignLeft->setToolTip(i18n("Left Align"));
	ui.tbAlignCenter->setToolTip(i18n("Center"));
	ui.tbAlignRight->setToolTip(i18n("Right Align"));
	ui.tbAlignJustify->setToolTip(i18n("Justify"));
	ui.tbSymbols->setToolTip(i18n("Insert Symbol"));
	ui.tbDateTime->setToolTip(i18n("Insert Date/Time"));

	// Positioning and alignment
	ui.cbPositionX->addItem(i18n("Left"));
	ui.cbPositionX->addItem(i18n("Center"));
	ui.cbPositionX->addItem(i18n("Right"));
	ui.cbPositionX->addItem(i18n("Relative to plot"));

	ui.cbPositionY->addItem(i18n("Top"));
	ui.cbPositionY->addItem(i18n("Center"));
	ui.cbPositionY->addItem(i18n("Bottom"));
	ui.cbPositionY->addItem(i18n("Relative to plot"));

	QString suffix;
	if (m_units == BaseDock::Units::Metric)
		suffix = QLatin1String(" cm");
	else
		suffix = QLatin1String(" in");

	ui.sbPositionX->setSuffix(suffix);
	ui.sbPositionY->setSuffix(suffix);

	ui.cbHorizontalAlignment->addItem(i18n("Left"));
	ui.cbHorizontalAlignment->addItem(i18n("Center"));
	ui.cbHorizontalAlignment->addItem(i18n("Right"));

	ui.cbVerticalAlignment->addItem(i18n("Top"));
	ui.cbVerticalAlignment->addItem(i18n("Center"));
	ui.cbVerticalAlignment->addItem(i18n("Bottom"));

	ui.cbBorderShape->addItem(i18n("No Border"));
	ui.cbBorderShape->addItem(i18n("Rectangle"));
	ui.cbBorderShape->addItem(i18n("Ellipse"));
	ui.cbBorderShape->addItem(i18n("Round sided rectangle"));
	ui.cbBorderShape->addItem(i18n("Round corner rectangle"));
	ui.cbBorderShape->addItem(i18n("Inwards round corner rectangle"));
	ui.cbBorderShape->addItem(i18n("Dented border rectangle"));
	ui.cbBorderShape->addItem(i18n("Cuboid"));
	ui.cbBorderShape->addItem(i18n("Up Pointing rectangle"));
	ui.cbBorderShape->addItem(i18n("Down Pointing rectangle"));
	ui.cbBorderShape->addItem(i18n("Left Pointing rectangle"));
	ui.cbBorderShape->addItem(i18n("Right Pointing rectangle"));

	ui.cbBorderStyle->addItem(i18n("No line"));
	ui.cbBorderStyle->addItem(i18n("Solid line"));
	ui.cbBorderStyle->addItem(i18n("Dash line"));
	ui.cbBorderStyle->addItem(i18n("Dot line"));
	ui.cbBorderStyle->addItem(i18n("Dash dot line"));
	ui.cbBorderStyle->addItem(i18n("Dash dot dot line"));

	ui.kcbBackgroundColor->setAlphaChannelEnabled(true);
	ui.kcbBackgroundColor->setColor(QColor(0, 0, 0, 0)); // transparent
	ui.kcbFontColor->setAlphaChannelEnabled(true);
	ui.kcbFontColor->setColor(QColor(255, 255, 255, 255)); // black
	ui.kcbBorderColor->setAlphaChannelEnabled(true);
	ui.kcbBorderColor->setColor(QColor(255, 255, 255, 255)); // black

	// Text mode
	ui.cbMode->addItem(QIcon::fromTheme(QLatin1String("text-x-plain")), i18n("Text"));
	ui.cbMode->addItem(QIcon::fromTheme(QLatin1String("text-x-tex")), i18n("LaTeX"));
#ifdef HAVE_DISCOUNT
	ui.cbMode->addItem(QIcon::fromTheme(QLatin1String("text-x-markdown")), i18n("Markdown"));
#endif

#ifdef HAVE_DISCOUNT
	msg = i18n(
		"Text setting mode:"
		"<ul>"
		"<li>Text - text setting using rich-text formatting</li>"
		"<li>LaTeX - text setting using LaTeX, installation of LaTeX required</li>"
		"<li>Markdown - text setting using Markdown markup language</li>"
		"</ul>");
#else
	msg = i18n(
		"Text setting mode:"
		"<ul>"
		"<li>Text - text setting using rich-text formatting</li>"
		"<li>LaTeX - text setting using LaTeX, installation of LaTeX required</li>"
		"</ul>");
#endif
	ui.cbMode->setToolTip(msg);

	// check whether LaTeX is available and deactivate the item in the combobox, if not.
	// in case LaTeX was used to generate the text label in the stored project
	// and no LaTeX is available on the target system, the disabled LaTeX item is selected
	// in the combobox in load() and the user still can switch to the non-latex mode.
	m_teXEnabled = TeXRenderer::enabled();
	if (!m_teXEnabled) {
		const auto* model = qobject_cast<QStandardItemModel*>(ui.cbMode->model());
		model->item(1)->setEnabled(false);
	}

#ifdef HAVE_KF5_SYNTAX_HIGHLIGHTING
	m_highlighter = new KSyntaxHighlighting::SyntaxHighlighter(ui.teLabel->document());
	m_highlighter->setTheme(GuiTools::isDarkMode() ? m_repository.defaultTheme(KSyntaxHighlighting::Repository::DarkTheme)
												   : m_repository.defaultTheme(KSyntaxHighlighting::Repository::LightTheme));
#endif

	m_messageWidget = new KMessageWidget(this);
	m_messageWidget->setMessageType(KMessageWidget::Error);
	m_messageWidget->setWordWrap(true);
	auto* gridLayout = qobject_cast<QGridLayout*>(layout());
	gridLayout->addWidget(m_messageWidget, 6, 3);
	m_messageWidget->hide(); // will be shown later once there is a latex render result

	// SLOTS

	// text properties
	connect(ui.cbMode, QOverload<int>::of(&KComboBox::currentIndexChanged), this, &LabelWidget::modeChanged);
	connect(ui.teLabel, &ResizableTextEdit::textChanged, this, &LabelWidget::textChanged);
	connect(ui.teLabel, &ResizableTextEdit::currentCharFormatChanged, this, &LabelWidget::charFormatChanged);
	connect(ui.kcbFontColor, &KColorButton::changed, this, &LabelWidget::fontColorChanged);
	connect(ui.kcbBackgroundColor, &KColorButton::changed, this, &LabelWidget::backgroundColorChanged);
	connect(ui.tbFontBold, &QToolButton::clicked, this, &LabelWidget::fontBoldChanged);
	connect(ui.tbFontItalic, &QToolButton::clicked, this, &LabelWidget::fontItalicChanged);
	connect(ui.tbFontUnderline, &QToolButton::clicked, this, &LabelWidget::fontUnderlineChanged);
	connect(ui.tbFontStrikeOut, &QToolButton::clicked, this, &LabelWidget::fontStrikeOutChanged);
	connect(ui.tbFontSuperScript, &QToolButton::clicked, this, &LabelWidget::fontSuperScriptChanged);
	connect(ui.tbFontSubScript, &QToolButton::clicked, this, &LabelWidget::fontSubScriptChanged);
	connect(ui.tbAlignLeft, &QToolButton::clicked, this, &LabelWidget::alignLeft);
	connect(ui.tbAlignCenter, &QToolButton::clicked, this, &LabelWidget::alignCenter);
	connect(ui.tbAlignRight, &QToolButton::clicked, this, &LabelWidget::alignRight);
	connect(ui.tbAlignJustify, &QToolButton::clicked, this, &LabelWidget::alignJustify);
	connect(ui.tbSymbols, &QToolButton::clicked, this, &LabelWidget::charMenu);
	connect(ui.tbDateTime, &QToolButton::clicked, this, &LabelWidget::dateTimeMenu);
	connect(m_dateTimeMenu, &QMenu::triggered, this, &LabelWidget::insertDateTime);
	connect(ui.kfontRequester, &KFontRequester::fontSelected, this, &LabelWidget::fontChanged);
	connect(ui.kfontRequesterTeX, &KFontRequester::fontSelected, this, &LabelWidget::teXFontChanged);
	connect(ui.sbFontSize, QOverload<int>::of(&QSpinBox::valueChanged), this, &LabelWidget::fontSizeChanged);

	// geometry
	connect(ui.cbPositionX, QOverload<int>::of(&KComboBox::currentIndexChanged), this, &LabelWidget::positionXChanged);
	connect(ui.cbPositionY, QOverload<int>::of(&KComboBox::currentIndexChanged), this, &LabelWidget::positionYChanged);
	connect(ui.sbPositionX, QOverload<double>::of(&NumberSpinBox::valueChanged), this, &LabelWidget::customPositionXChanged);
	connect(ui.sbPositionY, QOverload<double>::of(&NumberSpinBox::valueChanged), this, &LabelWidget::customPositionYChanged);
	connect(ui.cbHorizontalAlignment, QOverload<int>::of(&KComboBox::currentIndexChanged), this, &LabelWidget::horizontalAlignmentChanged);
	connect(ui.cbVerticalAlignment, QOverload<int>::of(&KComboBox::currentIndexChanged), this, &LabelWidget::verticalAlignmentChanged);

	connect(ui.sbPositionXLogical, QOverload<double>::of(&NumberSpinBox::valueChanged), this, &LabelWidget::positionXLogicalChanged);
	connect(ui.dtePositionXLogical, &UTCDateTimeEdit::mSecsSinceEpochUTCChanged, this, &LabelWidget::positionXLogicalDateTimeChanged);
	connect(ui.sbPositionYLogical, QOverload<double>::of(&NumberSpinBox::valueChanged), this, &LabelWidget::positionYLogicalChanged);
	// TODO?: connect(ui.dtePositionYLogical, &UTCDateTimeEdit::mSecsSinceEpochUTCChanged, this, &LabelWidget::positionYLogicalDateTimeChanged);
	connect(ui.sbRotation, QOverload<int>::of(&QSpinBox::valueChanged), this, &LabelWidget::rotationChanged);
	connect(ui.sbOffsetX, QOverload<double>::of(&NumberSpinBox::valueChanged), this, &LabelWidget::offsetXChanged);
	connect(ui.sbOffsetY, QOverload<double>::of(&NumberSpinBox::valueChanged), this, &LabelWidget::offsetYChanged);

	connect(ui.chbLock, &QCheckBox::clicked, this, &LabelWidget::lockChanged);
	connect(ui.chbBindLogicalPos, &QCheckBox::clicked, this, &LabelWidget::bindingChanged);
	connect(ui.chbShowPlaceholderText, &QCheckBox::toggled, this, &LabelWidget::showPlaceholderTextChanged);

	// Border
	connect(ui.cbBorderShape, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &LabelWidget::borderShapeChanged);
	connect(ui.cbBorderStyle, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &LabelWidget::borderStyleChanged);
	connect(ui.kcbBorderColor, &KColorButton::changed, this, &LabelWidget::borderColorChanged);
	connect(ui.sbBorderWidth, QOverload<double>::of(&NumberSpinBox::valueChanged), this, &LabelWidget::borderWidthChanged);
	connect(ui.sbBorderOpacity, QOverload<int>::of(&QSpinBox::valueChanged), this, &LabelWidget::borderOpacityChanged);

	// TODO: https://bugreports.qt.io/browse/QTBUG-25420
	ui.tbFontUnderline->hide();
	ui.tbFontStrikeOut->hide();
}

void LabelWidget::setLabels(QList<TextLabel*> labels) {
	m_labelsList = labels;
	m_axesList.clear();
	m_label = labels.first();
	setAspects(labels);

	ui.lOffsetX->hide();
	ui.lOffsetY->hide();

	ui.sbOffsetX->hide();
	ui.sbOffsetY->hide();

	// show the text fields for name and comment if the label is not hidden (i.e. not a plot title, etc.)
	bool visible = !m_label->hidden();
	ui.lName->setVisible(visible);
	ui.leName->setVisible(visible);
	ui.lComment->setVisible(visible);
	ui.teComment->setVisible(visible);

	this->load();
	initConnections();
	updateBackground();
	updateLocale();

	// hide the option "Visible" if the label is child of a InfoElement,
	// the label is what the user identifies with the info element itself
	visible = (m_label->parentAspect()->type() != AspectType::InfoElement);
	ui.chbVisible->setVisible(visible);

	// resize the widget to take the minimal height
	layout()->activate();
	const auto s = QSize(this->width(), 0).expandedTo(minimumSize());
	if (s.height() > 0)
		resize(s);
}

void LabelWidget::setAxes(QList<Axis*> axes) {
	m_labelsList.clear();
	for (const auto* axis : axes) {
		DEBUG(Q_FUNC_INFO << ", axis TITLE = " << axis->title())
		m_labelsList.append(axis->title());
		connect(axis, &Axis::titleOffsetXChanged, this, &LabelWidget::labelOffsetXChanged);
		connect(axis, &Axis::titleOffsetYChanged, this, &LabelWidget::labelOffsetYChanged);
		connect(axis->title(), &TextLabel::rotationAngleChanged, this, &LabelWidget::labelRotationAngleChanged);
	}

	m_axesList = axes;
	m_label = m_labelsList.first();
	setAspects(m_labelsList);

	ui.lName->hide();
	ui.leName->hide();
	ui.lComment->hide();
	ui.teComment->hide();

	// hide the option "Lock" if the label is child of an axis, the label cannot be freely moved in this case
	ui.chbLock->hide();

	this->load();
	initConnections();
	updateBackground();
	updateLocale();

	// resize the widget to take the minimal height
	layout()->activate();
	const auto s = QSize(this->width(), 0).expandedTo(minimumSize());
	if (s.height() > 0)
		resize(s);
}

/*!
 * this function keeps the background color of the TextEdit in LabelWidget in sync with
 * the background color of the parent aspect. This is to avoid the situations where the
 * text wouldn't be readable anymore if the foreground color is close or equal to the
 * background color of TextEdit - e.g., a label with white foreground color on a dark worksheet
 * and with white background color in TextEdit (desktop default color).
 *
 * Called if the background color of the parent aspect has changed.
 */
void LabelWidget::updateBackground() const {
	// if latex or markdown mode is used, use the default palette from the desktop theme
	// since we have additional highlighting for latex and markdown and we don't want it to
	// collide with the modified background color of QTextEdit. Modify it only for rich-text.
	const auto mode = static_cast<TextLabel::Mode>(ui.cbMode->currentIndex());
	if (mode != TextLabel::Mode::Text) {
		ui.teLabel->setPalette(QPalette());
		return;
	}

	QColor color(Qt::white);
	const auto type = m_label->parentAspect()->type();
	if (type == AspectType::Worksheet)
		color = static_cast<const Worksheet*>(m_label->parentAspect())->background()->firstColor();
	else if (type == AspectType::CartesianPlot)
		color = static_cast<CartesianPlot*>(m_label->parentAspect())->plotArea()->background()->firstColor();
	else if (type == AspectType::CartesianPlotLegend)
		color = static_cast<const CartesianPlotLegend*>(m_label->parentAspect())->background()->firstColor();
	else if (type == AspectType::InfoElement || type == AspectType::Axis)
		color = static_cast<CartesianPlot*>(m_label->parentAspect()->parentAspect())->plotArea()->background()->firstColor();
	else
		DEBUG(Q_FUNC_INFO << ", Not handled type:" << static_cast<int>(type));

	auto p = ui.teLabel->palette();
	// QDEBUG(Q_FUNC_INFO << ", color = " << color)
	p.setColor(QPalette::Base, color);
	ui.teLabel->setPalette(p);
}

void LabelWidget::initConnections() {
	while (!m_connections.isEmpty())
		disconnect(m_connections.takeFirst());
	m_connections << connect(m_label, &TextLabel::textWrapperChanged, this, &LabelWidget::labelTextWrapperChanged);
	m_connections << connect(m_label, &TextLabel::teXImageUpdated, this, &LabelWidget::labelTeXImageUpdated);
	m_connections << connect(m_label, &TextLabel::teXFontChanged, this, &LabelWidget::labelTeXFontChanged);
	m_connections << connect(m_label, &TextLabel::fontColorChanged, this, &LabelWidget::labelFontColorChanged);
	m_connections << connect(m_label, &TextLabel::backgroundColorChanged, this, &LabelWidget::labelBackgroundColorChanged);
	m_connections << connect(m_label, &TextLabel::positionChanged, this, &LabelWidget::labelPositionChanged);

	m_connections << connect(m_label, &TextLabel::positionLogicalChanged, this, &LabelWidget::labelPositionLogicalChanged);
	m_connections << connect(m_label, &TextLabel::coordinateBindingEnabledChanged, this, &LabelWidget::labelCoordinateBindingEnabledChanged);
	m_connections << connect(m_label, &TextLabel::horizontalAlignmentChanged, this, &LabelWidget::labelHorizontalAlignmentChanged);
	m_connections << connect(m_label, &TextLabel::verticalAlignmentChanged, this, &LabelWidget::labelVerticalAlignmentChanged);
	m_connections << connect(m_label, &TextLabel::rotationAngleChanged, this, &LabelWidget::labelRotationAngleChanged);
	m_connections << connect(m_label, &TextLabel::borderShapeChanged, this, &LabelWidget::labelBorderShapeChanged);
	m_connections << connect(m_label, &TextLabel::borderPenChanged, this, &LabelWidget::labelBorderPenChanged);
	m_connections << connect(m_label, &TextLabel::borderOpacityChanged, this, &LabelWidget::labelBorderOpacityChanged);
	m_connections << connect(m_label, &TextLabel::lockChanged, this, &LabelWidget::labelLockChanged);

	if (!m_label->parentAspect()) {
		QDEBUG(Q_FUNC_INFO << ", LABEL " << m_label << " HAS NO PARENT!")
		return;
	}

	const auto type = m_label->parentAspect()->type();
	if (type == AspectType::Worksheet) {
		auto* worksheet = static_cast<const Worksheet*>(m_label->parentAspect());
		connect(worksheet->background(), &Background::firstColorChanged, this, &LabelWidget::updateBackground);
	} else if (type == AspectType::CartesianPlot) {
		auto* plotArea = static_cast<CartesianPlot*>(m_label->parentAspect())->plotArea();
		connect(plotArea->background(), &Background::firstColorChanged, this, &LabelWidget::updateBackground);
	} else if (type == AspectType::CartesianPlotLegend) {
		auto* legend = static_cast<const CartesianPlotLegend*>(m_label->parentAspect());
		connect(legend->background(), &Background::firstColorChanged, this, &LabelWidget::updateBackground);
	} else if (type == AspectType::InfoElement || type == AspectType::Axis) {
		auto* plotArea = static_cast<CartesianPlot*>(m_label->parentAspect()->parentAspect())->plotArea();
		connect(plotArea->background(), &Background::firstColorChanged, this, &LabelWidget::updateBackground);
	}
}

/*!
 * enables/disables the "fixed label"-mode, used when displaying
 * the properties of axis' title label.
 * In this mode, in the "geometry"-part only the offset (offset to the axis)
 * and the rotation of the label are available.
 */
void LabelWidget::setFixedLabelMode(const bool b) {
	ui.lPositionX->setVisible(!b);
	ui.cbPositionX->setVisible(!b);
	ui.sbPositionX->setVisible(!b);
	ui.lPositionY->setVisible(!b);
	ui.cbPositionY->setVisible(!b);
	ui.sbPositionY->setVisible(!b);
	ui.lHorizontalAlignment->setVisible(!b);
	ui.cbHorizontalAlignment->setVisible(!b);
	ui.lVerticalAlignment->setVisible(!b);
	ui.cbVerticalAlignment->setVisible(!b);
	ui.lOffsetX->setVisible(b);
	ui.lOffsetY->setVisible(b);
	ui.sbOffsetX->setVisible(b);
	ui.sbOffsetY->setVisible(b);
}

/*!
 * enables/disables all geometry relevant widgets.
 * Used when displaying legend's title label.
 */
void LabelWidget::setGeometryAvailable(const bool b) {
	ui.lGeometry->setVisible(b);
	ui.lPositionX->setVisible(b);
	ui.cbPositionX->setVisible(b);
	ui.sbPositionX->setVisible(b);
	ui.lPositionY->setVisible(b);
	ui.cbPositionY->setVisible(b);
	ui.sbPositionY->setVisible(b);
	ui.lHorizontalAlignment->setVisible(b);
	ui.cbHorizontalAlignment->setVisible(b);
	ui.lVerticalAlignment->setVisible(b);
	ui.cbVerticalAlignment->setVisible(b);
	ui.lOffsetX->setVisible(b);
	ui.lOffsetY->setVisible(b);
	ui.sbOffsetX->setVisible(b);
	ui.sbOffsetY->setVisible(b);
	ui.lRotation->setVisible(b);
	ui.sbRotation->setVisible(b);
}

/*!
 * enables/disables all border relevant widgets.
 * Used when displaying legend's title label.
 */
void LabelWidget::setBorderAvailable(bool b) {
	ui.lBorder->setVisible(b);
	ui.lBorderShape->setVisible(b);
	ui.cbBorderShape->setVisible(b);
	ui.lBorderStyle->setVisible(b);
	ui.cbBorderStyle->setVisible(b);
	ui.lBorderColor->setVisible(b);
	ui.kcbBorderColor->setVisible(b);
	ui.lBorderWidth->setVisible(b);
	ui.sbBorderWidth->setVisible(b);
	ui.lBorderOpacity->setVisible(b);
	ui.sbBorderOpacity->setVisible(b);
}

void LabelWidget::updateUnits() {
	const KConfigGroup group = Settings::group(QStringLiteral("Settings_General"));
	BaseDock::Units units = (BaseDock::Units)group.readEntry("Units", (int)BaseDock::Units::Metric);
	if (units == m_units)
		return;

	m_units = units;
	CONDITIONAL_LOCK_RETURN;
	QString suffix;
	auto xPosition = ui.cbPositionX->currentIndex();
	auto yPosition = ui.cbPositionY->currentIndex();
	if (m_units == BaseDock::Units::Metric) {
		// convert from imperial to metric
		m_worksheetUnit = Worksheet::Unit::Centimeter;
		suffix = QLatin1String(" cm");
		if (xPosition != static_cast<int>(WorksheetElement::HorizontalPosition::Relative))
			ui.sbPositionX->setValue(ui.sbPositionX->value() * GSL_CONST_CGS_INCH);
		if (yPosition != static_cast<int>(WorksheetElement::VerticalPosition::Relative))
			ui.sbPositionY->setValue(ui.sbPositionY->value() * GSL_CONST_CGS_INCH);
	} else {
		// convert from metric to imperial
		m_worksheetUnit = Worksheet::Unit::Inch;
		suffix = QLatin1String(" in");
		if (xPosition != static_cast<int>(WorksheetElement::HorizontalPosition::Relative))
			ui.sbPositionX->setValue(ui.sbPositionX->value() / GSL_CONST_CGS_INCH);
		if (yPosition != static_cast<int>(WorksheetElement::VerticalPosition::Relative))
			ui.sbPositionY->setValue(ui.sbPositionY->value() / GSL_CONST_CGS_INCH);
	}

	if (xPosition != static_cast<int>(WorksheetElement::HorizontalPosition::Relative))
		ui.sbPositionX->setSuffix(suffix);
	if (yPosition != static_cast<int>(WorksheetElement::VerticalPosition::Relative))
		ui.sbPositionY->setSuffix(suffix);
}

void LabelWidget::updateLocale() {
	const auto numberLocale = QLocale();
	ui.sbPositionX->setLocale(numberLocale);
	ui.sbPositionY->setLocale(numberLocale);
	ui.sbOffsetX->setLocale(numberLocale);
	ui.sbOffsetY->setLocale(numberLocale);
	ui.sbBorderWidth->setLocale(numberLocale);
}

//**********************************************************
//****** SLOTs for changes triggered in LabelWidget ********
//**********************************************************

// text formatting slots

void LabelWidget::textChanged() {
	// QDEBUG("############\n" << Q_FUNC_INFO << ", label text =" << m_label->text().text)
	CONDITIONAL_LOCK_RETURN;

	const auto& plainText = ui.teLabel->toPlainText();
	QTextEdit te(ui.chbShowPlaceholderText->isChecked() ? m_label->text().textPlaceholder : m_label->text().text);
	bool plainTextChanged = plainText != te.toPlainText();

	const auto mode = static_cast<TextLabel::Mode>(ui.cbMode->currentIndex());
	switch (mode) {
	case TextLabel::Mode::LaTeX:
	case TextLabel::Mode::Markdown: {
		QString text = ui.teLabel->toPlainText();
		TextLabel::TextWrapper wrapper;
		wrapper.mode = mode;

		if (!ui.chbShowPlaceholderText->isChecked()) {
			if (plainTextChanged) {
				// set text only if the plain text change. otherwise the text is changed
				// already in the setter functions
				wrapper.text = std::move(text);
				for (auto* label : m_labelsList) {
					wrapper.textPlaceholder = label->text().textPlaceholder;
					wrapper.allowPlaceholder = label->text().allowPlaceholder;
					label->setText(wrapper);
				}
			}
		} else {
			// No need to compare if plainTextChanged
			// Change it always.
			wrapper.textPlaceholder = std::move(text);
			for (auto* label : m_labelsList) {
				wrapper.allowPlaceholder = label->text().allowPlaceholder;
				wrapper.text = label->text().text;
				label->setPlaceholderText(wrapper);
			}
		}
		break;
	}
	case TextLabel::Mode::Text: {
		// QDEBUG(Q_FUNC_INFO << ", color = " << m_label->fontColor())
		// QDEBUG(Q_FUNC_INFO << ", background color = " << m_label->backgroundColor())
		// QDEBUG(Q_FUNC_INFO << ", format color = " << ui.teLabel->currentCharFormat().foreground().color())
		// QDEBUG(Q_FUNC_INFO << ", Plain TEXT = " << ui.teLabel->toPlainText() << '\n')
		// QDEBUG(Q_FUNC_INFO << ", OLD TEXT =" << m_label->text().text << '\n')
		// save an empty string instead of html with empty body if no text is in QTextEdit
		QString text;
		if (!ui.teLabel->toPlainText().isEmpty()) {
			// if the current or previous label text is empty, set the color first
			QTextEdit pte(m_label->text().text); // te with previous text
			if (m_label->text().text.isEmpty() || pte.toPlainText().isEmpty()) {
				// DEBUG("EMPTY TEXT")
				ui.teLabel->selectAll();
				ui.teLabel->setTextColor(m_label->fontColor());
				ui.teLabel->setTextBackgroundColor(m_label->backgroundColor());
				// clear the selection after setting the color
				auto tc = ui.teLabel->textCursor();
				tc.setPosition(tc.selectionEnd());
				ui.teLabel->setTextCursor(tc);
			}

			text = ui.teLabel->toHtml();
		}

		QDEBUG(Q_FUNC_INFO << ", NEW TEXT = " << text << '\n')
		TextLabel::TextWrapper wrapper(text, TextLabel::Mode::Text, true);
		// Don't set font color, because it is already in the html code
		// of the text. The font color is used to change the color for Latex text
		if (!ui.chbShowPlaceholderText->isChecked()) {
			if (plainTextChanged) {
				wrapper.text = text;
				for (auto* label : m_labelsList) {
					if (text.isEmpty()) {
						label->setFontColor(ui.kcbFontColor->color());
						label->setBackgroundColor(ui.kcbBackgroundColor->color());
					}
					wrapper.allowPlaceholder = label->text().allowPlaceholder;
					wrapper.textPlaceholder = label->text().textPlaceholder;
					label->setText(wrapper);
				}
			}
		} else {
			wrapper.textPlaceholder = std::move(text);
			for (auto* label : m_labelsList) {
				wrapper.allowPlaceholder = label->text().allowPlaceholder;
				wrapper.text = label->text().text;
				label->setPlaceholderText(wrapper);
			}
		}
	}
	}

	// background color gets lost on every text change...
	updateBackground();
	// DEBUG(Q_FUNC_INFO << " DONE\n#################################")
}

/*!
 * \brief LabelWidget::charFormatChanged
 * \param format
 * Used to update the colors, font,... in the color font widgets to show the style of the selected text
 */
void LabelWidget::charFormatChanged(const QTextCharFormat& format) {
	auto mode = static_cast<TextLabel::Mode>(ui.cbMode->currentIndex());
	if (mode != TextLabel::Mode::Text)
		return;

	CONDITIONAL_LOCK_RETURN;

	// update button state
	ui.tbFontBold->setChecked(ui.teLabel->fontWeight() == QFont::Bold);
	ui.tbFontItalic->setChecked(ui.teLabel->fontItalic());
	ui.tbFontUnderline->setChecked(ui.teLabel->fontUnderline());
	ui.tbFontStrikeOut->setChecked(format.fontStrikeOut());
	ui.tbFontSuperScript->setChecked(format.verticalAlignment() == QTextCharFormat::AlignSuperScript);
	ui.tbFontSubScript->setChecked(format.verticalAlignment() == QTextCharFormat::AlignSubScript);

	// font and colors
	QDEBUG(Q_FUNC_INFO << ", format color = " << format.foreground().color())
	QDEBUG(Q_FUNC_INFO << ", label color = " << m_label->fontColor())
	QDEBUG(Q_FUNC_INFO << ", text = " << ui.teLabel->toPlainText())
	QDEBUG(Q_FUNC_INFO << ", label text = " << m_label->text().text)

	// TEST
	if (ui.teLabel->toPlainText().isEmpty())
		return;

	// when text is empty the default color of format is black instead of the theme color!
	if (m_label->text().isHtml() && format.foreground().color().isValid() && !ui.teLabel->toPlainText().isEmpty())
		ui.kcbFontColor->setColor(format.foreground().color());
	else
		ui.kcbFontColor->setColor(m_label->fontColor());

	if (m_label->text().isHtml() && format.background().color().isValid() && !ui.teLabel->toPlainText().isEmpty())
		ui.kcbBackgroundColor->setColor(format.background().color());
	else
		ui.kcbBackgroundColor->setColor(m_label->backgroundColor());

	ui.kfontRequester->setFont(format.font());
}

// called when textlabel mode is changed
void LabelWidget::labelModeChanged(TextLabel::Mode mode) {
	CONDITIONAL_LOCK_RETURN;

	updateMode(mode);
}

// Called when the combobox changes index
void LabelWidget::modeChanged(int index) {
	const auto mode = static_cast<TextLabel::Mode>(index);
	const bool plain = (mode != TextLabel::Mode::Text);

	labelModeChanged(mode);

	CONDITIONAL_RETURN_NO_LOCK; // No lock, because multiple things are set by the feedback

	QString text = plain ? ui.teLabel->toPlainText() : ui.teLabel->toHtml();
	TextLabel::TextWrapper wrapper(text, mode, !plain);
	DEBUG(Q_FUNC_INFO << ", text = " << STDSTRING(wrapper.text))
	for (auto* label : m_labelsList)
		label->setText(wrapper);
}

void LabelWidget::fontColorChanged(const QColor& color) {
	CONDITIONAL_LOCK_RETURN;

	const auto mode = m_label->text().mode;
	if (mode == TextLabel::Mode::Text || (mode == TextLabel::Mode::LaTeX && !m_teXEnabled)) {
		SETLABELTEXTPROPERTY(setTextColor, color);
		if (!cursorHasSelection) {
			for (auto* label : m_labelsList)
				label->setFontColor(color);
		}
	} else { // LaTeX (enabled) or Markup mode
		for (auto* label : m_labelsList)
			label->setFontColor(color);
	}
}

void LabelWidget::backgroundColorChanged(const QColor& color) {
	QDEBUG(Q_FUNC_INFO << ", color = " << color)
	CONDITIONAL_LOCK_RETURN;

	// remove the transparency if it was set initially before
	auto newColor(color);
	if (color.alpha() == 0) {
		newColor.setAlpha(255);
		ui.kcbBackgroundColor->setColor(newColor);
	}

	const auto mode = m_label->text().mode;
	if (mode == TextLabel::Mode::Text || (mode == TextLabel::Mode::LaTeX && !m_teXEnabled)) {
		SETLABELTEXTPROPERTY(setTextBackgroundColor, newColor);
	} else { // LaTeX (enabled) or Markup mode
		// Latex text does not support html code. For this the backgroundColor variable is used
		// Only single color background is supported
		for (auto* label : m_labelsList)
			label->setBackgroundColor(newColor);
	}
}

void LabelWidget::fontSizeChanged(int value) {
	CONDITIONAL_LOCK_RETURN;

	QFont font = m_label->teXFont();
	font.setPointSize(value);
	for (auto* label : m_labelsList)
		label->setTeXFont(font);
}

void LabelWidget::alignLeft() {
	CONDITIONAL_LOCK_RETURN;
	SETLABELTEXTPROPERTY(setAlignment, Qt::AlignLeft);
}

void LabelWidget::alignCenter() {
	CONDITIONAL_LOCK_RETURN;
	SETLABELTEXTPROPERTY(setAlignment, Qt::AlignHCenter);
}

void LabelWidget::alignRight() {
	CONDITIONAL_LOCK_RETURN;
	SETLABELTEXTPROPERTY(setAlignment, Qt::AlignRight);
}

void LabelWidget::alignJustify() {
	CONDITIONAL_LOCK_RETURN;
	SETLABELTEXTPROPERTY(setAlignment, Qt::AlignJustify);
}

void LabelWidget::fontBoldChanged(bool checked) {
	CONDITIONAL_LOCK_RETURN;

	QFont::Weight weight;
	if (checked)
		weight = QFont::Bold;
	else
		weight = QFont::Normal;
	SETLABELTEXTPROPERTY(setFontWeight, weight);
}

void LabelWidget::fontItalicChanged(bool checked) {
	CONDITIONAL_LOCK_RETURN;

	SETLABELTEXTPROPERTY(setFontItalic, checked);
}

void LabelWidget::fontUnderlineChanged(bool checked) {
	CONDITIONAL_LOCK_RETURN;

	SETLABELTEXTPROPERTY(setFontUnderline, checked);
}

void LabelWidget::fontStrikeOutChanged(bool checked) {
	CONDITIONAL_LOCK_RETURN;

	auto format = ui.teLabel->currentCharFormat();
	format.setFontStrikeOut(checked);
	SETLABELTEXTPROPERTY(setCurrentCharFormat, format);
}

void LabelWidget::fontSuperScriptChanged(bool checked) {
	CONDITIONAL_LOCK_RETURN;

	auto format = ui.teLabel->currentCharFormat();
	if (checked)
		format.setVerticalAlignment(QTextCharFormat::AlignSuperScript);
	else
		format.setVerticalAlignment(QTextCharFormat::AlignNormal);

	SETLABELTEXTPROPERTY(setCurrentCharFormat, format);
}

void LabelWidget::fontSubScriptChanged(bool checked) {
	CONDITIONAL_LOCK_RETURN;

	auto format = ui.teLabel->currentCharFormat();
	if (checked)
		format.setVerticalAlignment(QTextCharFormat::AlignSubScript);
	else
		format.setVerticalAlignment(QTextCharFormat::AlignNormal);

	SETLABELTEXTPROPERTY(setCurrentCharFormat, format);
}

void LabelWidget::fontChanged(const QFont& font) {
	CONDITIONAL_LOCK_RETURN;

	// use mergeCurrentCharFormat(QTextCharFormat) instead of setFontFamily(font.family()), etc.
	// because this avoids textChanged() after every command
	QTextCharFormat format;
	format.setFontFamily(font.family());
#if (QT_VERSION >= QT_VERSION_CHECK(5, 13, 0))
	format.setFontFamilies({font.family()}); // see QTBUG-80475
#endif
	format.setFontPointSize(font.pointSize());
	format.setFontItalic(font.italic());
	format.setFontWeight(font.weight());
	if (font.underline())
		format.setUnderlineStyle(QTextCharFormat::UnderlineStyle::SingleUnderline);
	if (font.strikeOut())
		format.setFontStrikeOut(font.strikeOut());

	// QDEBUG(Q_FUNC_INFO << ", BEFORE:" << ui.teLabel->toHtml())
	SETLABELTEXTPROPERTY(mergeCurrentCharFormat, format);
	// QDEBUG(Q_FUNC_INFO << ", AFTER :" << ui.teLabel->toHtml())
}

void LabelWidget::teXFontChanged(const QFont& font) {
	CONDITIONAL_LOCK_RETURN;

	for (auto* label : m_labelsList)
		label->setTeXFont(font);
}

void LabelWidget::charMenu() {
	QMenu menu;
	KCharSelect selection(this, nullptr, KCharSelect::SearchLine | KCharSelect::CharacterTable | KCharSelect::BlockCombos | KCharSelect::HistoryButtons);
	QFont font = ui.teLabel->currentFont();
	// use the system default size, otherwise the symbols might be hard to read
	// if the current label font size is too small
	font.setPointSize(QFont().pointSize());
	selection.setCurrentFont(font);
	connect(&selection, &KCharSelect::charSelected, this, &LabelWidget::insertChar);
	connect(&selection, &KCharSelect::charSelected, &menu, &LabelWidget::close);

	auto* widgetAction = new QWidgetAction(this);
	widgetAction->setDefaultWidget(&selection);
	menu.addAction(widgetAction);

	QPoint pos(-menu.sizeHint().width() + ui.tbSymbols->width(), -menu.sizeHint().height());
	menu.exec(ui.tbSymbols->mapToGlobal(pos));
}

void LabelWidget::insertChar(QChar c) {
	ui.teLabel->insertPlainText(QString(c));
}

void LabelWidget::dateTimeMenu() {
	m_dateTimeMenu->clear();

	const QString configPath = QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation);
	const QString configFile = configPath + QLatin1String("/klanguageoverridesrc");
	if (!QFile::exists(configFile)) {
		QDate date = QDate::currentDate();
		m_dateTimeMenu->addSeparator()->setText(i18n("Date"));
		m_dateTimeMenu->addAction(date.toString(Qt::TextDate));
		m_dateTimeMenu->addAction(date.toString(Qt::ISODate));
		m_dateTimeMenu->addAction(QLocale::system().toString(date, QLocale::ShortFormat));
		m_dateTimeMenu->addAction(QLocale::system().toString(date, QLocale::LongFormat));
		m_dateTimeMenu->addAction(date.toString(Qt::RFC2822Date));

		QDateTime time = QDateTime::currentDateTime();
		m_dateTimeMenu->addSeparator()->setText(i18n("Date and Time"));
		m_dateTimeMenu->addAction(time.toString(Qt::TextDate));
		m_dateTimeMenu->addAction(time.toString(Qt::ISODate));
		m_dateTimeMenu->addAction(QLocale::system().toString(time, QLocale::ShortFormat));
		m_dateTimeMenu->addAction(QLocale::system().toString(time, QLocale::LongFormat));
		m_dateTimeMenu->addAction(time.toString(Qt::RFC2822Date));
	} else {
		// application language was changed:
		// determine the currently used language and use QLocale::toString()
		// to get the strings translated into the currently used language
		// TODO: why not use QLocale() ?
		QSettings settings(configFile, QSettings::IniFormat);
		settings.beginGroup(QLatin1String("Language"));
		QByteArray languageCode;
		languageCode = settings.value(qAppName(), languageCode).toByteArray();
		QLocale locale(QString::fromLatin1(languageCode.data()));

		QDate date = QDate::currentDate();
		m_dateTimeMenu->addSeparator()->setText(i18n("Date"));
		m_dateTimeMenu->addAction(locale.toString(date, QLatin1String("ddd MMM d yyyy"))); // Qt::TextDate
		m_dateTimeMenu->addAction(locale.toString(date, QLatin1String("yyyy-MM-dd"))); // Qt::ISODate
		m_dateTimeMenu->addAction(locale.system().toString(date, QLocale::ShortFormat)); // Qt::SystemLocaleShortDate
		// no LongFormat here since it would contain strings in system's language which (potentially) is not the current application language
		m_dateTimeMenu->addAction(locale.toString(date, QLatin1String("dd MMM yyyy"))); // Qt::RFC2822Date

		QDateTime time = QDateTime::currentDateTime();
		m_dateTimeMenu->addSeparator()->setText(i18n("Date and Time"));
		m_dateTimeMenu->addAction(locale.toString(time, QLatin1String("ddd MMM d hh:mm:ss yyyy"))); // Qt::TextDate
		m_dateTimeMenu->addAction(locale.toString(time, QLatin1String("yyyy-MM-ddTHH:mm:ss"))); // Qt::ISODate
		m_dateTimeMenu->addAction(locale.system().toString(time, QLocale::ShortFormat)); // Qt::SystemLocaleShortDate
		// no LongFormat here since it would contain strings in system's language which (potentially) is not the current application language

		// TODO: RFC2822 requires time zone but Qt QLocale::toString() seems to ignore TZD (time zone designator) completely,
		// which works correctly with QDateTime::toString()
		m_dateTimeMenu->addAction(locale.toString(time, QLatin1String("dd MMM yyyy hh:mm:ss"))); // Qt::RFC2822Date
	}

	m_dateTimeMenu->exec(mapToGlobal(ui.tbDateTime->rect().bottomLeft()));
}

void LabelWidget::insertDateTime(QAction* action) {
	ui.teLabel->insertPlainText(action->text().remove(QLatin1Char('&')));
}

// positioning using absolute coordinates
/*!
	called when label's current horizontal position relative to its parent (left, center, right, relative) is changed.
*/
void LabelWidget::positionXChanged(int index) {
	CONDITIONAL_LOCK_RETURN;

	auto position = m_label->position();
	auto oldHorizontalPosition = position.horizontalPosition;
	position.horizontalPosition = TextLabel::HorizontalPosition(index);
	double x = 0.;
	if (position.horizontalPosition == WorksheetElement::HorizontalPosition::Relative) {
		switch (oldHorizontalPosition) {
		case WorksheetElement::HorizontalPosition::Left:
		case WorksheetElement::HorizontalPosition::Relative:
			break;
		case WorksheetElement::HorizontalPosition::Center:
			x = 0.5;
			break;
		case WorksheetElement::HorizontalPosition::Right:
			x = 1.0;
		}
		ui.sbPositionX->setSuffix(QStringLiteral(" %"));
	} else {
		if (m_units == Units::Metric)
			ui.sbPositionX->setSuffix(QStringLiteral(" cm"));
		else
			ui.sbPositionX->setSuffix(QStringLiteral(" in"));
	}

	position.point.setX(x);
	ui.sbPositionX->setValue(100. * x);

	for (auto* label : m_labelsList)
		label->setPosition(position);
}

/*!
	called when label's current horizontal position relative to its parent (top, center, bottom, relative) is changed.
*/
void LabelWidget::positionYChanged(int index) {
	CONDITIONAL_LOCK_RETURN;

	auto position = m_label->position();
	auto oldVerticalPosition = position.verticalPosition;
	position.verticalPosition = TextLabel::VerticalPosition(index);
	double y = 0.;
	if (position.verticalPosition == WorksheetElement::VerticalPosition::Relative) {
		switch (oldVerticalPosition) {
		case WorksheetElement::VerticalPosition::Top:
		case WorksheetElement::VerticalPosition::Relative:
			break;
		case WorksheetElement::VerticalPosition::Center:
			y = 0.5;
			break;
		case WorksheetElement::VerticalPosition::Bottom:
			y = 1.0;
		}
		ui.sbPositionY->setSuffix(QStringLiteral(" %"));
	} else {
		if (m_units == Units::Metric)
			ui.sbPositionY->setSuffix(QStringLiteral(" cm"));
		else
			ui.sbPositionY->setSuffix(QStringLiteral(" in"));
	}

	position.point.setY(y);
	ui.sbPositionY->setValue(100. * y);

	for (auto* label : m_labelsList)
		label->setPosition(position);
}

void LabelWidget::horizontalAlignmentChanged(int index) {
	CONDITIONAL_LOCK_RETURN;

	for (auto* label : m_labelsList)
		label->setHorizontalAlignment(TextLabel::HorizontalAlignment(index));
}

void LabelWidget::verticalAlignmentChanged(int index) {
	CONDITIONAL_LOCK_RETURN;

	for (auto* label : m_labelsList)
		label->setVerticalAlignment(TextLabel::VerticalAlignment(index));
}

void LabelWidget::customPositionXChanged(double value) {
	CONDITIONAL_RETURN_NO_LOCK;

	for (auto* label : m_labelsList) {
		auto position = label->position();
		if (position.horizontalPosition == WorksheetElement::HorizontalPosition::Relative)
			position.point.setX(value / 100.);
		else
			position.point.setX(Worksheet::convertToSceneUnits(value, m_worksheetUnit));
		label->setPosition(position);
	}
}

void LabelWidget::customPositionYChanged(double value) {
	CONDITIONAL_RETURN_NO_LOCK;

	for (auto* label : m_labelsList) {
		auto position = label->position();
		if (position.verticalPosition == WorksheetElement::VerticalPosition::Relative)
			position.point.setY(value / 100.);
		else
			position.point.setY(Worksheet::convertToSceneUnits(value, m_worksheetUnit));
		label->setPosition(position);
	}
}

// positioning using logical plot coordinates
void LabelWidget::positionXLogicalChanged(double value) {
	CONDITIONAL_RETURN_NO_LOCK;

	for (auto* label : m_labelsList) {
		auto pos = label->positionLogical();
		pos.setX(value);
		label->setPositionLogical(pos);
	}
}

void LabelWidget::positionXLogicalDateTimeChanged(qint64 value) {
	CONDITIONAL_LOCK_RETURN;

	for (auto* label : m_labelsList) {
		auto pos = label->positionLogical();
		pos.setX(value);
		label->setPositionLogical(pos);
	}
}

void LabelWidget::positionYLogicalChanged(double value) {
	CONDITIONAL_RETURN_NO_LOCK;

	for (auto* label : m_labelsList) {
		auto pos = label->positionLogical();
		pos.setY(value);
		label->setPositionLogical(pos);
	}
}

void LabelWidget::rotationChanged(int value) {
	CONDITIONAL_LOCK_RETURN;

	for (auto* label : m_labelsList)
		label->setRotationAngle(value);
}

void LabelWidget::offsetXChanged(double value) {
	CONDITIONAL_RETURN_NO_LOCK;

	for (auto* axis : m_axesList)
		axis->setTitleOffsetX(Worksheet::convertToSceneUnits(value, Worksheet::Unit::Point));
}

void LabelWidget::offsetYChanged(double value) {
	CONDITIONAL_RETURN_NO_LOCK;

	for (auto* axis : m_axesList)
		axis->setTitleOffsetY(Worksheet::convertToSceneUnits(value, Worksheet::Unit::Point));
}

void LabelWidget::lockChanged(bool locked) {
	CONDITIONAL_LOCK_RETURN;
	for (auto* label : m_labelsList)
		label->setLock(locked);
}

// border
void LabelWidget::borderShapeChanged(int index) {
	auto shape = (TextLabel::BorderShape)index;
	bool b = (shape != TextLabel::BorderShape::NoBorder);
	ui.lBorderStyle->setVisible(b);
	ui.cbBorderStyle->setVisible(b);
	ui.lBorderWidth->setVisible(b);
	ui.sbBorderWidth->setVisible(b);
	ui.lBorderColor->setVisible(b);
	ui.kcbBorderColor->setVisible(b);
	ui.lBorderOpacity->setVisible(b);
	ui.sbBorderOpacity->setVisible(b);

	CONDITIONAL_LOCK_RETURN;

	for (auto* label : m_labelsList)
		label->setBorderShape(shape);
}

void LabelWidget::borderStyleChanged(int index) {
	CONDITIONAL_LOCK_RETURN;

	auto penStyle = Qt::PenStyle(index);
	QPen pen;
	for (auto* label : m_labelsList) {
		pen = label->borderPen();
		pen.setStyle(penStyle);
		label->setBorderPen(pen);
	}
}

void LabelWidget::borderColorChanged(const QColor& color) {
	CONDITIONAL_LOCK_RETURN;

	QPen pen;
	for (auto* label : m_labelsList) {
		pen = label->borderPen();
		pen.setColor(color);
		label->setBorderPen(pen);
	}
	GuiTools::updatePenStyles(ui.cbBorderStyle, color);
}

void LabelWidget::borderWidthChanged(double value) {
	CONDITIONAL_RETURN_NO_LOCK;

	QPen pen;
	for (auto* label : m_labelsList) {
		pen = label->borderPen();
		pen.setWidthF(Worksheet::convertToSceneUnits(value, Worksheet::Unit::Point));
		label->setBorderPen(pen);
	}
}

void LabelWidget::borderOpacityChanged(int value) {
	CONDITIONAL_LOCK_RETURN;

	qreal opacity = (float)value / 100.;
	for (auto* label : m_labelsList)
		label->setBorderOpacity(opacity);
}

/*!
 * \brief LabelWidget::bindingChanged
 * Bind TextLabel to the cartesian plot coords or not
 * \param checked
 */
void LabelWidget::bindingChanged(bool checked) {
	// widgets for positioning using absolute plot distances
	ui.lPositionX->setVisible(!checked);
	ui.cbPositionX->setVisible(!checked);
	ui.sbPositionX->setVisible(!checked);

	ui.lPositionY->setVisible(!checked);
	ui.cbPositionY->setVisible(!checked);
	ui.sbPositionY->setVisible(!checked);

	// widgets for positioning using logical plot coordinates
	const auto* plot = static_cast<const CartesianPlot*>(m_label->parent(AspectType::CartesianPlot));
	if (plot && plot->xRangeFormatDefault() == RangeT::Format::DateTime) {
		ui.lPositionXLogicalDateTime->setVisible(checked);
		ui.dtePositionXLogical->setVisible(checked);

		ui.lPositionXLogical->setVisible(false);
		ui.sbPositionXLogical->setVisible(false);
	} else {
		ui.lPositionXLogicalDateTime->setVisible(false);
		ui.dtePositionXLogical->setVisible(false);

		ui.lPositionXLogical->setVisible(checked);
		ui.sbPositionXLogical->setVisible(checked);
	}

	ui.lPositionYLogical->setVisible(checked);
	ui.sbPositionYLogical->setVisible(checked);

	CONDITIONAL_LOCK_RETURN;

	ui.chbBindLogicalPos->setChecked(checked);

	for (auto* label : m_labelsList)
		label->setCoordinateBindingEnabled(checked);
}

void LabelWidget::showPlaceholderTextChanged(bool checked) {
	CONDITIONAL_LOCK_RETURN;

	if (!checked) {
		ui.teLabel->setEnabled(false);
		if (m_label->text().mode != TextLabel::Mode::Text)
			ui.teLabel->setText(m_label->text().text);
		else
			ui.teLabel->setHtml(m_label->text().text);
	} else {
		ui.teLabel->setEnabled(true);
		if (m_label->text().mode != TextLabel::Mode::Text)
			ui.teLabel->setText(m_label->text().textPlaceholder);
		else
			ui.teLabel->setHtml(m_label->text().textPlaceholder);
	}
}

//*********************************************************
//****** SLOTs for changes triggered in TextLabel *********
//*********************************************************
void LabelWidget::labelTextWrapperChanged(const TextLabel::TextWrapper& text) {
	CONDITIONAL_LOCK_RETURN;

	// save and restore the current cursor position after changing the text
	auto cursor = ui.teLabel->textCursor();
	int position = cursor.position();
	if (!ui.chbShowPlaceholderText->isChecked()) {
		if (text.mode != TextLabel::Mode::Text)
			ui.teLabel->setText(text.text);
		else
			ui.teLabel->setHtml(text.text);
	} else {
		if (text.mode != TextLabel::Mode::Text)
			ui.teLabel->setText(text.textPlaceholder);
		else
			ui.teLabel->setHtml(text.textPlaceholder);
	}
	cursor.movePosition(QTextCursor::Start);
	cursor.movePosition(QTextCursor::Right, QTextCursor::MoveAnchor, position);
	ui.teLabel->setTextCursor(cursor);

	const int index = static_cast<int>(text.mode);
	ui.cbMode->setCurrentIndex(index);
	this->labelModeChanged(text.mode);
}

/*!
 * \brief Highlights the text field if wrong latex syntax was used (null image was produced)
 * or something else went wrong during rendering (\sa ExpressionTextEdit::validateExpression())
 */
void LabelWidget::labelTeXImageUpdated(const TeXRenderer::Result& result) {
	if (!result.successful) {
		if (ui.teLabel->styleSheet().isEmpty())
			SET_WARNING_STYLE(ui.teLabel)
		m_messageWidget->setText(result.errorMessage);
		m_messageWidget->setMaximumWidth(ui.teLabel->width());
	} else
		ui.teLabel->setStyleSheet(QString());

	m_messageWidget->setVisible(!result.successful);
}

void LabelWidget::labelTeXFontChanged(const QFont& font) {
	CONDITIONAL_LOCK_RETURN;
	ui.kfontRequesterTeX->setFont(font);
	ui.sbFontSize->setValue(font.pointSize());
}

// this function is only called when the theme is changed. Otherwise the color is coded in the html text.
// when the theme changes, the whole text should change color regardless of the color it has
void LabelWidget::labelFontColorChanged(const QColor& color) {
	Q_EMIT labelFontColorChangedSignal(color);

	CONDITIONAL_LOCK_RETURN;
	ui.kcbFontColor->setColor(color);
	ui.teLabel->selectAll();
	ui.teLabel->setTextColor(color);
}

void LabelWidget::labelPositionChanged(const TextLabel::PositionWrapper& position) {
	CONDITIONAL_LOCK_RETURN;

	ui.cbPositionX->setCurrentIndex(static_cast<int>(position.horizontalPosition));
	ui.cbPositionY->setCurrentIndex(static_cast<int>(position.verticalPosition));
	if (position.horizontalPosition == WorksheetElement::HorizontalPosition::Relative) {
		ui.sbPositionX->setValue(position.point.x() * 100.);
		ui.sbPositionX->setSuffix(QStringLiteral(" %"));
	} else
		ui.sbPositionX->setValue(Worksheet::convertFromSceneUnits(position.point.x(), m_worksheetUnit));

	if (position.verticalPosition == WorksheetElement::VerticalPosition::Relative) {
		ui.sbPositionY->setValue(position.point.y() * 100.);
		ui.sbPositionY->setSuffix(QStringLiteral(" %"));
	} else
		ui.sbPositionY->setValue(Worksheet::convertFromSceneUnits(position.point.y(), m_worksheetUnit));
}

void LabelWidget::labelHorizontalAlignmentChanged(TextLabel::HorizontalAlignment index) {
	CONDITIONAL_LOCK_RETURN;
	ui.cbHorizontalAlignment->setCurrentIndex(static_cast<int>(index));
}

void LabelWidget::labelVerticalAlignmentChanged(TextLabel::VerticalAlignment index) {
	CONDITIONAL_LOCK_RETURN;
	ui.cbVerticalAlignment->setCurrentIndex(static_cast<int>(index));
}

void LabelWidget::labelCoordinateBindingEnabledChanged(bool enabled) {
	CONDITIONAL_LOCK_RETURN;
	bindingChanged(enabled);
}

void LabelWidget::labelPositionLogicalChanged(QPointF pos) {
	CONDITIONAL_LOCK_RETURN;
	ui.sbPositionXLogical->setValue(pos.x());
	ui.dtePositionXLogical->setMSecsSinceEpochUTC(pos.x());
	ui.sbPositionYLogical->setValue(pos.y());
	// TODO: why not ui.dtePositionYLogical->setMSecsSinceEpochUTC(pos.y());
}

// this function is only called when the theme is changed. Otherwise the color is coded in the html text.
// when the theme changes, the whole text should change color regardless of the color it has
void LabelWidget::labelBackgroundColorChanged(const QColor& color) {
	QDEBUG(Q_FUNC_INFO << ", color =" << color)
	CONDITIONAL_LOCK_RETURN;
	ui.kcbBackgroundColor->setColor(color);

	auto mode = static_cast<TextLabel::Mode>(ui.cbMode->currentIndex());
	if (mode != TextLabel::Mode::Text)
		return;

	ui.teLabel->selectAll();
	ui.teLabel->setTextBackgroundColor(color);
}

void LabelWidget::labelOffsetXChanged(qreal offset) {
	CONDITIONAL_LOCK_RETURN;
	ui.sbOffsetX->setValue(Worksheet::convertFromSceneUnits(offset, Worksheet::Unit::Point));
}

void LabelWidget::labelOffsetYChanged(qreal offset) {
	CONDITIONAL_LOCK_RETURN;
	ui.sbOffsetY->setValue(Worksheet::convertFromSceneUnits(offset, Worksheet::Unit::Point));
}

void LabelWidget::labelRotationAngleChanged(qreal angle) {
	CONDITIONAL_LOCK_RETURN;
	ui.sbRotation->setValue(angle);
}

void LabelWidget::labelLockChanged(bool on) {
	CONDITIONAL_LOCK_RETURN;
	ui.chbLock->setChecked(on);
}

// border
void LabelWidget::labelBorderShapeChanged(TextLabel::BorderShape shape) {
	CONDITIONAL_LOCK_RETURN;
	ui.cbBorderShape->setCurrentIndex(static_cast<int>(shape));
}

void LabelWidget::labelBorderPenChanged(const QPen& pen) {
	CONDITIONAL_LOCK_RETURN;
	if (ui.cbBorderStyle->currentIndex() != pen.style())
		ui.cbBorderStyle->setCurrentIndex(pen.style());
	if (ui.kcbBorderColor->color() != pen.color())
		ui.kcbBorderColor->setColor(pen.color());
	// Feedback needed therefore no condition
	ui.sbBorderWidth->setValue(Worksheet::convertFromSceneUnits(pen.widthF(), Worksheet::Unit::Point));
}

void LabelWidget::labelBorderOpacityChanged(float value) {
	CONDITIONAL_LOCK_RETURN;
	const float v = (float)value * 100.;
	ui.sbBorderOpacity->setValue(v);
}

void LabelWidget::labelCartesianPlotParent(bool on) {
	CONDITIONAL_LOCK_RETURN;
	ui.chbBindLogicalPos->setVisible(on);
	if (!on)
		ui.chbBindLogicalPos->setChecked(false);
}

//**********************************************************
//******************** SETTINGS ****************************
//**********************************************************
void LabelWidget::load() {
	if (!m_label)
		return;

	CONDITIONAL_LOCK_RETURN;

	ui.chbVisible->setChecked(m_label->isVisible());
	ui.chbLock->setChecked(m_label->isLocked());

	// don't show checkbox if Placeholder feature not used
	const bool allowPlaceholder = m_label->text().allowPlaceholder;
	ui.chbShowPlaceholderText->setVisible(allowPlaceholder);
	ui.chbShowPlaceholderText->setEnabled(allowPlaceholder);
	ui.chbShowPlaceholderText->setChecked(allowPlaceholder);

	// Text
	const auto mode = m_label->text().mode;
	ui.cbMode->setCurrentIndex(static_cast<int>(mode));
	this->updateMode(mode);

	QString text;
	if (!allowPlaceholder)
		text = m_label->text().text;
	else
		text = m_label->text().textPlaceholder;

	if (mode == TextLabel::Mode::Text) {
		ui.teLabel->setHtml(text);
		ui.teLabel->selectAll(); // must be done to retrieve font
		ui.kfontRequester->setFont(ui.teLabel->currentFont());
	} else
		ui.teLabel->setText(text);

	// when text is empty the default color of format is black instead of the theme color!
	const auto& format = ui.teLabel->currentCharFormat();
	if (m_label->text().isHtml() && format.foreground().color().isValid() && !ui.teLabel->toPlainText().isEmpty())
		ui.kcbFontColor->setColor(format.foreground().color());
	else
		ui.kcbFontColor->setColor(m_label->fontColor());

	if (m_label->text().isHtml() && format.background().color().isValid() && !ui.teLabel->toPlainText().isEmpty()) {
		// The html below does not contain any information about the background color. So qt uses black which is not correct
		// "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\">\n<html><head><meta name=\"qrichtext\"
		// content=\"1\" /><style type=\"text/css\">\np, li { white-space: pre-wrap; }\n</style></head><body style=\" font-family:'Noto Sans'; font-size:10pt;
		// font-weight:400; font-style:normal;\">\n<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0;
		// text-indent:0px;\"><span style=\" color:#000000;\">1</span></p></body></html>"
		if (m_label->text().text.contains(QStringLiteral("background-color")))
			ui.kcbBackgroundColor->setColor(format.background().color());
		else
			ui.kcbBackgroundColor->setColor(QColor(Qt::GlobalColor::transparent));
	} else
		ui.kcbBackgroundColor->setColor(m_label->backgroundColor());

	ui.kfontRequesterTeX->setFont(m_label->teXFont());
	ui.sbFontSize->setValue(m_label->teXFont().pointSize());

	ui.tbFontBold->setChecked(ui.teLabel->fontWeight() == QFont::Bold);
	ui.tbFontItalic->setChecked(ui.teLabel->fontItalic());
	ui.tbFontUnderline->setChecked(ui.teLabel->fontUnderline());
	ui.tbFontStrikeOut->setChecked(format.fontStrikeOut());
	ui.tbFontSuperScript->setChecked(format.verticalAlignment() == QTextCharFormat::AlignSuperScript);
	ui.tbFontSubScript->setChecked(format.verticalAlignment() == QTextCharFormat::AlignSubScript);

	// move the cursor to the end
	auto cursor = ui.teLabel->textCursor();
	cursor.movePosition(QTextCursor::End);
	ui.teLabel->setTextCursor(cursor);
	// ui.teLabel->setFocus(); // Do not set focus, otherwise the WorksheetView is not able to catch key input events!

	// Geometry
	// widgets for positioning using absolute plot distances
	ui.cbPositionX->setCurrentIndex((int)m_label->position().horizontalPosition);
	// positionXChanged(ui.cbPositionX->currentIndex());
	if (m_label->position().horizontalPosition == WorksheetElement::HorizontalPosition::Relative) {
		ui.sbPositionX->setValue(m_label->position().point.x() * 100);
		ui.sbPositionX->setSuffix(QStringLiteral(" %"));
	} else
		ui.sbPositionX->setValue(Worksheet::convertFromSceneUnits(m_label->position().point.x(), m_worksheetUnit));
	ui.cbPositionY->setCurrentIndex((int)m_label->position().verticalPosition);
	// positionYChanged(ui.cbPositionY->currentIndex());
	if (m_label->position().verticalPosition == WorksheetElement::VerticalPosition::Relative) {
		ui.sbPositionY->setValue(m_label->position().point.y() * 100);
		ui.sbPositionY->setSuffix(QStringLiteral(" %"));
	} else
		ui.sbPositionY->setValue(Worksheet::convertFromSceneUnits(m_label->position().point.y(), m_worksheetUnit));

	ui.cbHorizontalAlignment->setCurrentIndex((int)m_label->horizontalAlignment());
	ui.cbVerticalAlignment->setCurrentIndex((int)m_label->verticalAlignment());

	// widgets for positioning using logical plot coordinates
	bool allowLogicalCoordinates = (m_label->plot() != nullptr);
	ui.chbBindLogicalPos->setVisible(allowLogicalCoordinates);

	if (allowLogicalCoordinates) {
		const auto* plot = static_cast<const CartesianPlot*>(m_label->plot());
		if (plot->xRangeFormatDefault() == RangeT::Format::Numeric) {
			ui.lPositionXLogical->show();
			ui.sbPositionXLogical->show();
			ui.lPositionXLogicalDateTime->hide();
			ui.dtePositionXLogical->hide();

			ui.sbPositionXLogical->setValue(m_label->positionLogical().x());
			ui.sbPositionYLogical->setValue(m_label->positionLogical().y());
		} else { // DateTime
			ui.lPositionXLogical->hide();
			ui.sbPositionXLogical->hide();
			ui.lPositionXLogicalDateTime->show();
			ui.dtePositionXLogical->show();

			ui.dtePositionXLogical->setDisplayFormat(plot->rangeDateTimeFormat(Dimension::X));
			ui.dtePositionXLogical->setMSecsSinceEpochUTC(m_label->positionLogical().x());
		}

		ui.chbBindLogicalPos->setChecked(m_label->coordinateBindingEnabled());
		bindingChanged(m_label->coordinateBindingEnabled());
	} else {
		ui.lPositionXLogical->hide();
		ui.sbPositionXLogical->hide();
		ui.lPositionYLogical->hide();
		ui.sbPositionYLogical->hide();
		ui.lPositionXLogicalDateTime->hide();
		ui.dtePositionXLogical->hide();
	}

	// offsets, available for axis label only
	if (!m_axesList.isEmpty()) {
		ui.sbOffsetX->setValue(Worksheet::convertFromSceneUnits(m_axesList.first()->titleOffsetX(), Worksheet::Unit::Point));
		ui.sbOffsetY->setValue(Worksheet::convertFromSceneUnits(m_axesList.first()->titleOffsetY(), Worksheet::Unit::Point));
	}
	ui.sbRotation->setValue(m_label->rotationAngle());

	// don't show if binding not enabled. example: axis titles
	// Border
	ui.cbBorderShape->setCurrentIndex(static_cast<int>(m_label->borderShape()));
	borderShapeChanged(ui.cbBorderShape->currentIndex());
	ui.kcbBorderColor->setColor(m_label->borderPen().color());
	ui.cbBorderStyle->setCurrentIndex((int)m_label->borderPen().style());
	ui.sbBorderWidth->setValue(Worksheet::convertFromSceneUnits(m_label->borderPen().widthF(), Worksheet::Unit::Point));
	ui.sbBorderOpacity->setValue(round(m_label->borderOpacity() * 100));
	GuiTools::updatePenStyles(ui.cbBorderStyle, ui.kcbBorderColor->color());
}

// General updater function to update the dock (used also in the load method)
void LabelWidget::updateMode(TextLabel::Mode mode) {
	bool plain = (mode != TextLabel::Mode::Text);

	// hide text editing elements if TeX-option is used
	ui.tbFontBold->setVisible(!plain);
	ui.tbFontItalic->setVisible(!plain);

	// TODO: https://bugreports.qt.io/browse/QTBUG-25420
	// 	ui.tbFontUnderline->setVisible(!plain);
	// 	ui.tbFontStrikeOut->setVisible(!plain);

	ui.tbFontSubScript->setVisible(!plain);
	ui.tbFontSuperScript->setVisible(!plain);

	ui.lFont->setVisible(!plain);
	ui.kfontRequester->setVisible(!plain);

	if (plain) {
		// reset all applied formattings when switching from html to tex mode
		QTextCursor cursor = ui.teLabel->textCursor();
		int position = cursor.position();
		ui.teLabel->selectAll();
		QTextCharFormat format;
		ui.teLabel->setCurrentCharFormat(format);
		cursor.movePosition(QTextCursor::Right, QTextCursor::MoveAnchor, position);
		ui.teLabel->setTextCursor(cursor);

#ifdef HAVE_KF5_SYNTAX_HIGHLIGHTING
		m_highlighter->setDocument(ui.teLabel->document());
		if (mode == TextLabel::Mode::LaTeX)
			m_highlighter->setDefinition(m_repository.definitionForName(QLatin1String("LaTeX")));
		else
			m_highlighter->setDefinition(m_repository.definitionForName(QLatin1String("Markdown")));
#endif
		KConfigGroup conf = Settings::group(QLatin1String("Settings_Worksheet"));
		QString engine = conf.readEntry(QLatin1String("LaTeXEngine"), "");
		if (engine == QLatin1String("xelatex") || engine == QLatin1String("lualatex")) {
			ui.lFontTeX->setVisible(true);
			ui.kfontRequesterTeX->setVisible(true);
			ui.lFontSize->setVisible(false);
			ui.sbFontSize->setVisible(false);
		} else {
			// changing the main font for latex and pdflatex is a cumbersome (https://latex-tutorial.com/changing-font-style/),
			// hide this option completely for these engines for now
			ui.lFontTeX->setVisible(false);
			ui.kfontRequesterTeX->setVisible(false);

			ui.lFontSize->setVisible(true);
			ui.sbFontSize->setVisible(true);
		}

		// update colors
		ui.kcbFontColor->setColor(m_label->fontColor());
		ui.kcbBackgroundColor->setColor(m_label->backgroundColor());
	} else {
#ifdef HAVE_KF5_SYNTAX_HIGHLIGHTING
		m_highlighter->setDocument(nullptr);
#endif
		ui.lFontTeX->setVisible(false);
		ui.kfontRequesterTeX->setVisible(false);
		ui.lFontSize->setVisible(false);
		ui.sbFontSize->setVisible(false);
	}

	updateBackground();

	// when switching to non-LaTeX mode, set the background color to white just for the case the latex code provided by the user
	// in the TeX-mode is not valid and the background was set to red (s.a. LabelWidget::labelTeXImageUpdated())
	if (mode != TextLabel::Mode::LaTeX) {
		ui.teLabel->setStyleSheet(QString());
		m_messageWidget->setVisible(false);
	}
}

void LabelWidget::loadConfig(KConfigGroup& group) {
	if (!m_label)
		return;

	// Text
	ui.cbMode->setCurrentIndex(group.readEntry("Mode", static_cast<int>(m_label->text().mode)));
	this->modeChanged(ui.cbMode->currentIndex());
	ui.sbFontSize->setValue(group.readEntry("TeXFontSize", m_label->teXFont().pointSize()));
	ui.kcbFontColor->setColor(group.readEntry("FontColor", m_label->fontColor()));
	ui.kcbBackgroundColor->setColor(group.readEntry("BackgroundColor", m_label->backgroundColor()));
	ui.kfontRequesterTeX->setFont(group.readEntry("TeXFont", m_label->teXFont()));

	// Geometry
	ui.cbPositionX->setCurrentIndex(group.readEntry("PositionX", (int)m_label->position().horizontalPosition));
	ui.sbPositionX->setValue(Worksheet::convertFromSceneUnits(group.readEntry("PositionXValue", m_label->position().point.x()), m_worksheetUnit));
	ui.cbPositionY->setCurrentIndex(group.readEntry("PositionY", (int)m_label->position().verticalPosition));
	ui.sbPositionY->setValue(Worksheet::convertFromSceneUnits(group.readEntry("PositionYValue", m_label->position().point.y()), m_worksheetUnit));

	if (!m_axesList.isEmpty()) {
		ui.sbOffsetX->setValue(Worksheet::convertFromSceneUnits(group.readEntry("OffsetX", m_axesList.first()->titleOffsetX()), Worksheet::Unit::Point));
		ui.sbOffsetY->setValue(Worksheet::convertFromSceneUnits(group.readEntry("OffsetY", m_axesList.first()->titleOffsetY()), Worksheet::Unit::Point));
	}
	ui.cbHorizontalAlignment->setCurrentIndex(group.readEntry("HorizontalAlignment", (int)m_label->horizontalAlignment()));
	ui.cbVerticalAlignment->setCurrentIndex(group.readEntry("VerticalAlignment", (int)m_label->verticalAlignment()));
	ui.sbRotation->setValue(group.readEntry("Rotation", m_label->rotationAngle()));

	// Border
	ui.cbBorderShape->setCurrentIndex(group.readEntry("BorderShape").toInt());
	ui.kcbBorderColor->setColor(group.readEntry("BorderColor", m_label->borderPen().color()));
	ui.cbBorderStyle->setCurrentIndex(group.readEntry("BorderStyle", (int)m_label->borderPen().style()));
	ui.sbBorderWidth->setValue(Worksheet::convertFromSceneUnits(group.readEntry("BorderWidth", m_label->borderPen().widthF()), Worksheet::Unit::Point));
	ui.sbBorderOpacity->setValue(group.readEntry("BorderOpacity", m_label->borderOpacity()) * 100);
}

void LabelWidget::saveConfig(KConfigGroup& group) {
	// Text
	group.writeEntry("Mode", ui.cbMode->currentIndex());
	group.writeEntry("FontColor", ui.kcbFontColor->color());
	group.writeEntry("BackgroundColor", ui.kcbBackgroundColor->color());
	group.writeEntry("TeXFont", ui.kfontRequesterTeX->font());

	// Geometry
	group.writeEntry("PositionX", ui.cbPositionX->currentIndex());
	group.writeEntry("PositionXValue", Worksheet::convertToSceneUnits(ui.sbPositionX->value(), m_worksheetUnit));
	group.writeEntry("PositionY", ui.cbPositionY->currentIndex());
	group.writeEntry("PositionYValue", Worksheet::convertToSceneUnits(ui.sbPositionY->value(), m_worksheetUnit));

	if (!m_axesList.isEmpty()) {
		group.writeEntry("OffsetX", Worksheet::convertToSceneUnits(ui.sbOffsetX->value(), Worksheet::Unit::Point));
		group.writeEntry("OffsetY", Worksheet::convertToSceneUnits(ui.sbOffsetY->value(), Worksheet::Unit::Point));
	}
	group.writeEntry("HorizontalAlignment", ui.cbHorizontalAlignment->currentIndex());
	group.writeEntry("VerticalAlignment", ui.cbVerticalAlignment->currentIndex());
	group.writeEntry("Rotation", ui.sbRotation->value());

	// Border
	group.writeEntry("BorderShape", ui.cbBorderShape->currentIndex());
	group.writeEntry("BorderStyle", ui.cbBorderStyle->currentIndex());
	group.writeEntry("BorderColor", ui.kcbBorderColor->color());
	group.writeEntry("BorderWidth", Worksheet::convertToSceneUnits(ui.sbBorderWidth->value(), Worksheet::Unit::Point));
	group.writeEntry("BorderOpacity", ui.sbBorderOpacity->value() / 100.0);
}
