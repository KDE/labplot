/*
    File                 : LabelWidget.cc
    Project              : LabPlot
    Description          : label settings widget
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2008-2022 Alexander Semke <alexander.semke@web.de>
    SPDX-FileCopyrightText: 2012-2022 Stefan Gerlach <stefan.gerlach@uni-konstanz.de>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "LabelWidget.h"
#include "backend/worksheet/Worksheet.h"
#include "backend/worksheet/plots/PlotArea.h"
#include "backend/worksheet/plots/cartesian/Axis.h"
#include "backend/worksheet/plots/cartesian/CartesianPlot.h"
#include "backend/worksheet/plots/cartesian/CartesianPlotLegend.h"
#include "kdefrontend/GuiTools.h"
#include "kdefrontend/dockwidgets/BaseDock.h"
#include "tools/TeXRenderer.h"

#include <QFile>
#include <QMenu>
#include <QSettings>
#include <QSplitter>
#include <QTextDocumentFragment>
#include <QStandardItemModel>
#include <QWidgetAction>

#include <KCharSelect>
#include <KLocalizedString>
#ifdef HAVE_KF5_SYNTAX_HIGHLIGHTING
#include <KSyntaxHighlighting/SyntaxHighlighter>
#include <KSyntaxHighlighting/Definition>
#include <KSyntaxHighlighting/Theme>
#endif

/*!
	\class LabelWidget
 	\brief Widget for editing the properties of a TextLabel object, mostly used in an appropriate dock widget.

	In order the properties of the label to be shown, \c loadConfig() has to be called with the corresponding KConfigGroup
	(settings for a label in *Plot, Axis etc. or for an independent label on the worksheet).

	\ingroup kdefrontend
 */
LabelWidget::LabelWidget(QWidget* parent) : QWidget(parent), m_dateTimeMenu(new QMenu(this)) {
	ui.setupUi(this);

	//set the minimum size of the text edit widget to one row of a QLineEdit
	ui.teLabel->setMinimumHeight(ui.lePositionXLogical->height());

	//adjust the layout margins
	if (auto* l = dynamic_cast<QGridLayout*>(layout())) {
		l->setContentsMargins(2,2,2,2);
		l->setHorizontalSpacing(2);
		l->setVerticalSpacing(2);
	}

	const KConfigGroup group = KSharedConfig::openConfig()->group(QLatin1String("Settings_General"));
	m_units = (BaseDock::Units)group.readEntry("Units", (int)BaseDock::Units::Metric);
	if (m_units == BaseDock::Units::Imperial)
		m_worksheetUnit = Worksheet::Unit::Inch;

	m_dateTimeMenu->setSeparatorsCollapsible(false); //we don't want the first separator to be removed

	QString msg = i18n("Use logical instead of absolute coordinates to specify the position on the plot");
	ui.lBindLogicalPos->setToolTip(msg);
	ui.chbBindLogicalPos->setToolTip(msg);

	//Icons
	ui.tbFontBold->setIcon( QIcon::fromTheme(QLatin1String("format-text-bold")) );
	ui.tbFontItalic->setIcon( QIcon::fromTheme(QLatin1String("format-text-italic")) );
	ui.tbFontUnderline->setIcon( QIcon::fromTheme(QLatin1String("format-text-underline")) );
	ui.tbFontStrikeOut->setIcon( QIcon::fromTheme(QLatin1String("format-text-strikethrough")) );
	ui.tbFontSuperScript->setIcon( QIcon::fromTheme(QLatin1String("format-text-superscript")) );
	ui.tbFontSubScript->setIcon( QIcon::fromTheme(QLatin1String("format-text-subscript")) );
	ui.tbSymbols->setIcon( QIcon::fromTheme(QLatin1String("labplot-format-text-symbol")) );
	ui.tbDateTime->setIcon( QIcon::fromTheme(QLatin1String("chronometer")) );

	ui.tbFontBold->setToolTip(i18n("Bold"));
	ui.tbFontItalic->setToolTip(i18n("Italic"));
	ui.tbFontUnderline->setToolTip(i18n("Underline"));
	ui.tbFontStrikeOut->setToolTip(i18n("Strike Out"));
	ui.tbFontSuperScript->setToolTip(i18n("Super Script"));
	ui.tbFontSubScript->setToolTip(i18n("Sub-Script"));
	ui.tbSymbols->setToolTip(i18n("Insert Symbol"));
	ui.tbDateTime->setToolTip(i18n("Insert Date/Time"));

	//Positioning and alignment
	ui.cbPositionX->addItem(i18n("Left"));
	ui.cbPositionX->addItem(i18n("Center"));
	ui.cbPositionX->addItem(i18n("Right"));

	ui.cbPositionY->addItem(i18n("Top"));
	ui.cbPositionY->addItem(i18n("Center"));
	ui.cbPositionY->addItem(i18n("Bottom"));

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
	ui.kcbBackgroundColor->setColor(QColor(0,0,0, 0)); // transparent
	ui.kcbFontColor->setAlphaChannelEnabled(true);
	ui.kcbFontColor->setColor(QColor(255,255,255, 255)); // black
	ui.kcbBorderColor->setAlphaChannelEnabled(true);
	ui.kcbBorderColor->setColor(QColor(255,255,255, 255)); // black

	//Text mode
	ui.cbMode->addItem(QIcon::fromTheme(QLatin1String("text-x-plain")), i18n("Text"));
	ui.cbMode->addItem(QIcon::fromTheme(QLatin1String("text-x-tex")), i18n("LaTeX"));
#ifdef HAVE_DISCOUNT
	ui.cbMode->addItem(QIcon::fromTheme(QLatin1String("text-x-markdown")), i18n("Markdown"));
#endif

	msg = i18n("Text setting mode:"
	"<ul>"
	"<li>Text - text setting using rich-text formatting</li>"
	"<li>LaTeX - text setting using LaTeX, installation of LaTeX required</li>"
#ifdef HAVE_DISCOUNT
	"<li>Markdown - text setting using Markdown markup language</li>"
#endif
	"</ul>");
	ui.cbMode->setToolTip(msg);

	//check whether LaTeX is available and deactivate the item in the combobox, if not.
	//in case LaTeX was used to generate the text label in the stored project
	//and no LaTeX is available on the target system, the disabled LaTeX item is selected
	//in the combobox in load() and the user still can switch to the non-latex mode.
	m_teXEnabled = TeXRenderer::enabled();
	if (!m_teXEnabled) {
		auto* model = qobject_cast<QStandardItemModel*>(ui.cbMode->model());
		model->item(1)->setEnabled(false);
	}

#ifdef HAVE_KF5_SYNTAX_HIGHLIGHTING
	m_highlighter = new KSyntaxHighlighting::SyntaxHighlighter(ui.teLabel->document());
	m_highlighter->setTheme(  (palette().color(QPalette::Base).lightness() < 128)
	                          ? m_repository.defaultTheme(KSyntaxHighlighting::Repository::DarkTheme)
	                          : m_repository.defaultTheme(KSyntaxHighlighting::Repository::LightTheme) );
#endif

	//SLOTS
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
	connect(ui.tbSymbols, &QToolButton::clicked, this, &LabelWidget::charMenu);
	connect(ui.tbDateTime, &QToolButton::clicked, this, &LabelWidget::dateTimeMenu);
	connect(m_dateTimeMenu, &QMenu::triggered, this, &LabelWidget::insertDateTime );
	connect(ui.kfontRequester, &KFontRequester::fontSelected, this, &LabelWidget::fontChanged);
	connect(ui.kfontRequesterTeX, &KFontRequester::fontSelected, this, &LabelWidget::teXFontChanged);
	connect(ui.sbFontSize, QOverload<int>::of(&QSpinBox::valueChanged), this, &LabelWidget::fontSizeChanged);

	// geometry
	connect( ui.cbPositionX, QOverload<int>::of(&KComboBox::currentIndexChanged), this, &LabelWidget::positionXChanged);
	connect( ui.cbPositionY, QOverload<int>::of(&KComboBox::currentIndexChanged), this, &LabelWidget::positionYChanged);
	connect( ui.sbPositionX, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &LabelWidget::customPositionXChanged);
	connect( ui.sbPositionY, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &LabelWidget::customPositionYChanged);
	connect( ui.cbHorizontalAlignment, QOverload<int>::of(&KComboBox::currentIndexChanged), this, &LabelWidget::horizontalAlignmentChanged);
	connect( ui.cbVerticalAlignment, QOverload<int>::of(&KComboBox::currentIndexChanged), this, &LabelWidget::verticalAlignmentChanged);

	connect(ui.lePositionXLogical, &QLineEdit::textChanged, this, &LabelWidget::positionXLogicalChanged);
	connect(ui.dtePositionXLogical, &QDateTimeEdit::dateTimeChanged, this, &LabelWidget::positionXLogicalDateTimeChanged);
	connect(ui.lePositionYLogical, &QLineEdit::textChanged, this, &LabelWidget::positionYLogicalChanged);
	connect(ui.sbRotation, QOverload<int>::of(&QSpinBox::valueChanged), this, &LabelWidget::rotationChanged);
	connect(ui.sbOffsetX, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &LabelWidget::offsetXChanged);
	connect(ui.sbOffsetY, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &LabelWidget::offsetYChanged);

	connect(ui.chbVisible, &QCheckBox::clicked, this, &LabelWidget::visibilityChanged);
	connect(ui.chbBindLogicalPos, &QCheckBox::clicked, this, &LabelWidget::bindingChanged);
	connect(ui.chbShowPlaceholderText, &QCheckBox::toggled, this, &LabelWidget::showPlaceholderTextChanged);

	//Border
	connect(ui.cbBorderShape, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &LabelWidget::borderShapeChanged);
	connect(ui.cbBorderStyle, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &LabelWidget::borderStyleChanged);
	connect(ui.kcbBorderColor, &KColorButton::changed, this, &LabelWidget::borderColorChanged);
	connect(ui.sbBorderWidth, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &LabelWidget::borderWidthChanged);
	connect(ui.sbBorderOpacity, QOverload<int>::of(&QSpinBox::valueChanged), this, &LabelWidget::borderOpacityChanged);

	//TODO: https://bugreports.qt.io/browse/QTBUG-25420
	ui.tbFontUnderline->hide();
	ui.tbFontStrikeOut->hide();
}

void LabelWidget::setLabels(QList<TextLabel*> labels) {
	m_labelsList = labels;
	m_axesList.clear();
	m_label = labels.first();

	ui.lOffsetX->hide();
	ui.lOffsetY->hide();

	ui.sbOffsetX->hide();
	ui.sbOffsetY->hide();

	this->load();
	initConnections();
	updateBackground();
	updateLocale();

	//hide the option "Visible" if the label is child of a InfoElement,
	//the label is what the user identifies with the info element itself
	bool visible = (m_label->parentAspect()->type() != AspectType::InfoElement);
	ui.chbVisible->setVisible(visible);

	//resize the widget to take the minimal height
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


	this->load();
	initConnections();
	updateBackground();
	updateLocale();

	//resize the widget to take the minimal height
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
	DEBUG(Q_FUNC_INFO)
	if (static_cast<TextLabel::Mode>(ui.cbMode->currentIndex()) != TextLabel::Mode::Text)
		return; //nothing to do

	QColor color(Qt::white);
	const auto type = m_label->parentAspect()->type();
	if (type == AspectType::Worksheet)
		color = static_cast<const Worksheet*>(m_label->parentAspect())->backgroundFirstColor();
	else if (type == AspectType::CartesianPlot)
		color = static_cast<CartesianPlot*>(m_label->parentAspect())->plotArea()->backgroundFirstColor();
	else if (type == AspectType::CartesianPlotLegend)
		color = static_cast<const CartesianPlotLegend*>(m_label->parentAspect())->backgroundFirstColor();
	else if (type == AspectType::InfoElement || type == AspectType::Axis)
		color = static_cast<CartesianPlot*>(m_label->parentAspect()->parentAspect())->plotArea()->backgroundFirstColor();
	else
		DEBUG(Q_FUNC_INFO << ", Not handled type:" << static_cast<int>(type));

	auto p = ui.teLabel->palette();
	QDEBUG(Q_FUNC_INFO << ", color = " << color)
	p.setColor(QPalette::Base, color);
	ui.teLabel->setPalette(p);
}

void LabelWidget::initConnections() const {
	connect(m_label, &TextLabel::textWrapperChanged, this, &LabelWidget::labelTextWrapperChanged);
	connect(m_label, &TextLabel::teXImageUpdated, this, &LabelWidget::labelTeXImageUpdated);
	connect(m_label, &TextLabel::teXFontChanged, this, &LabelWidget::labelTeXFontChanged);
	connect(m_label, &TextLabel::fontColorChanged, this, &LabelWidget::labelFontColorChanged);
	connect(m_label, &TextLabel::backgroundColorChanged, this, &LabelWidget::labelBackgroundColorChanged);
	connect(m_label, &TextLabel::positionChanged, this, &LabelWidget::labelPositionChanged);

	connect(m_label, &TextLabel::positionLogicalChanged, this, &LabelWidget::labelPositionLogicalChanged);
	connect(m_label, &TextLabel::coordinateBindingEnabledChanged, this, &LabelWidget::labelCoordinateBindingEnabledChanged);
	connect(m_label, &TextLabel::horizontalAlignmentChanged, this, &LabelWidget::labelHorizontalAlignmentChanged);
	connect(m_label, &TextLabel::verticalAlignmentChanged, this, &LabelWidget::labelVerticalAlignmentChanged);
	connect(m_label, &TextLabel::rotationAngleChanged, this, &LabelWidget::labelRotationAngleChanged);
	connect(m_label, &TextLabel::borderShapeChanged, this, &LabelWidget::labelBorderShapeChanged);
	connect(m_label, &TextLabel::borderPenChanged, this, &LabelWidget::labelBorderPenChanged);
	connect(m_label, &TextLabel::borderOpacityChanged, this, &LabelWidget::labelBorderOpacityChanged);
	connect(m_label, &TextLabel::visibleChanged, this, &LabelWidget::labelVisibleChanged);

	if (!m_label->parentAspect()) {
		QDEBUG(Q_FUNC_INFO << ", LABEL " <<m_label << " HAS NO PARENT!")
		return;
}
	AspectType type = m_label->parentAspect()->type();
	if (type == AspectType::Worksheet) {
		auto* worksheet = static_cast<const Worksheet*>(m_label->parentAspect());
		connect(worksheet, &Worksheet::backgroundFirstColorChanged, this, &LabelWidget::updateBackground);
	} else if (type == AspectType::CartesianPlot) {
		auto* plotArea = static_cast<CartesianPlot*>(m_label->parentAspect())->plotArea();
		connect(plotArea, &PlotArea::backgroundFirstColorChanged, this, &LabelWidget::updateBackground);
	} else if (type == AspectType::CartesianPlotLegend) {
		auto* legend = static_cast<const CartesianPlotLegend*>(m_label->parentAspect());
		connect(legend, &CartesianPlotLegend::backgroundFirstColorChanged, this, &LabelWidget::updateBackground);
	} else if (type == AspectType::Axis) {
		auto* plotArea = static_cast<CartesianPlot*>(m_label->parentAspect()->parentAspect())->plotArea();
		connect(plotArea, &PlotArea::backgroundFirstColorChanged, this, &LabelWidget::updateBackground);
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
	const KConfigGroup group = KSharedConfig::openConfig()->group(QLatin1String("Settings_General"));
	BaseDock::Units units = (BaseDock::Units)group.readEntry("Units", (int)BaseDock::Units::Metric);
	if (units == m_units)
		return;

	m_units = units;
	Lock lock(m_initializing);
	QString suffix;
	if (m_units == BaseDock::Units::Metric) {
		//convert from imperial to metric
		m_worksheetUnit = Worksheet::Unit::Centimeter;
		suffix = QLatin1String(" cm");
		ui.sbPositionX->setValue(ui.sbPositionX->value()*2.54);
		ui.sbPositionY->setValue(ui.sbPositionX->value()*2.54);
	} else {
		//convert from metric to imperial
		m_worksheetUnit = Worksheet::Unit::Inch;
		suffix = QLatin1String(" in");
		ui.sbPositionX->setValue(ui.sbPositionX->value()/2.54);
		ui.sbPositionY->setValue(ui.sbPositionY->value()/2.54);
	}

	ui.sbPositionX->setSuffix(suffix);
	ui.sbPositionY->setSuffix(suffix);
}

void LabelWidget::updateLocale() {
	SET_NUMBER_LOCALE
	ui.sbPositionX->setLocale(numberLocale);
	ui.sbPositionY->setLocale(numberLocale);
	ui.lePositionXLogical->setLocale(numberLocale);
	ui.lePositionYLogical->setLocale(numberLocale);
	ui.sbOffsetX->setLocale(numberLocale);
	ui.sbOffsetY->setLocale(numberLocale);
	ui.sbBorderWidth->setLocale(numberLocale);
}

//**********************************************************
//****** SLOTs for changes triggered in LabelWidget ********
//**********************************************************

// text formatting slots

void LabelWidget::textChanged() {
	//QDEBUG("############\n" << Q_FUNC_INFO << ", label text =" << m_label->text().text)
	if (m_initializing)
		return;

	const Lock lock(m_initializing);

	auto mode = static_cast<TextLabel::Mode>(ui.cbMode->currentIndex());
	switch (mode) {
	case TextLabel::Mode::LaTeX:
	case TextLabel::Mode::Markdown: {
		QString text = ui.teLabel->toPlainText();
		TextLabel::TextWrapper wrapper;
		wrapper.mode = mode;

		if (!ui.chbShowPlaceholderText->isChecked()) {
			wrapper.text = text;
			for (auto* label : m_labelsList) {
				wrapper.textPlaceholder = label->text().textPlaceholder;
				wrapper.allowPlaceholder = label->text().allowPlaceholder;
				label->setText(wrapper);
			}
		} else {
			wrapper.textPlaceholder = text;
			for (auto* label: m_labelsList) {
				wrapper.allowPlaceholder = label->text().allowPlaceholder;
				wrapper.text = label->text().text;
				label->setPlaceholderText(wrapper);
			}
		}
		break;
	}
	case TextLabel::Mode::Text: {
		//QDEBUG(Q_FUNC_INFO << ", color = " << m_label->fontColor())
		//QDEBUG(Q_FUNC_INFO << ", background color = " << m_label->backgroundColor())
		//QDEBUG(Q_FUNC_INFO << ", format color = " << ui.teLabel->currentCharFormat().foreground().color())
		//QDEBUG(Q_FUNC_INFO << ", Plain TEXT = " << ui.teLabel->toPlainText() << '\n')
		//QDEBUG(Q_FUNC_INFO << ", OLD TEXT =" << m_label->text().text << '\n')
		//save an empty string instead of html with empty body if no text is in QTextEdit
		QString text;
		if (!ui.teLabel->toPlainText().isEmpty()) {
			//if the current or previous label text is empty, set the color first
			QTextEdit pte(m_label->text().text);	// te with previous text
			if (m_label->text().text.isEmpty() || pte.toPlainText().isEmpty()) {
				//DEBUG("EMPTY TEXT")
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

		//QDEBUG(Q_FUNC_INFO << ", NEW TEXT = " << text << '\n')
		TextLabel::TextWrapper wrapper(text, TextLabel::Mode::Text, true);
		// Don't set font color, because it is already in the html code
		// of the text. The font color is used to change the color for Latex text
		if(!ui.chbShowPlaceholderText->isChecked()) {
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
		} else {
			wrapper.textPlaceholder = text;
			for (auto* label : m_labelsList) {
				wrapper.allowPlaceholder = label->text().allowPlaceholder;
				wrapper.text = label->text().text;
				label->setPlaceholderText(wrapper);
			}
		}
	}
	}

	//background color gets lost on every text change...
	updateBackground();
	//DEBUG(Q_FUNC_INFO << " DONE\n#################################")
}

/*!
 * \brief LabelWidget::charFormatChanged
 * \param format
 * Used to update the colors, font,... in the color font widgets to show the style of the selected text
 */
void LabelWidget::charFormatChanged(const QTextCharFormat& format) {
	if (m_initializing)
		return;

	auto mode = static_cast<TextLabel::Mode>(ui.cbMode->currentIndex());
	if (mode != TextLabel::Mode::Text)
		return;

	const Lock lock(m_initializing);

	// update button state
	ui.tbFontBold->setChecked(ui.teLabel->fontWeight() == QFont::Bold);
	ui.tbFontItalic->setChecked(ui.teLabel->fontItalic());
	ui.tbFontUnderline->setChecked(ui.teLabel->fontUnderline());
	ui.tbFontStrikeOut->setChecked(format.fontStrikeOut());
	ui.tbFontSuperScript->setChecked(format.verticalAlignment() == QTextCharFormat::AlignSuperScript);
	ui.tbFontSubScript->setChecked(format.verticalAlignment() == QTextCharFormat::AlignSubScript);

	//font and colors
	QDEBUG(Q_FUNC_INFO << ", format color = " << format.foreground().color())
	QDEBUG(Q_FUNC_INFO << ", label color = " << m_label->fontColor())
	QDEBUG(Q_FUNC_INFO << ", text = " << ui.teLabel->toPlainText())
	QDEBUG(Q_FUNC_INFO << ", label text = " << m_label->text().text)

	//TEST
	if (ui.teLabel->toPlainText().isEmpty())
		return;

	// when text is empty the default color of format is black instead of the theme color!
	if (format.foreground().color().isValid() && !ui.teLabel->toPlainText().isEmpty()) {
		ui.kcbFontColor->setColor(format.foreground().color());
	} else {
		ui.kcbFontColor->setColor(m_label->fontColor());
	}

	if (format.background().color().isValid() && !ui.teLabel->toPlainText().isEmpty())
		ui.kcbBackgroundColor->setColor(format.background().color());
	else
		ui.kcbBackgroundColor->setColor(m_label->backgroundColor());

	ui.kfontRequester->setFont(format.font());
}

// called when textlabel mode is changed
void LabelWidget::labelModeChanged(TextLabel::Mode mode) {
	if (m_initializing)
		return;
	const Lock lock(m_initializing);

	updateMode(mode);
}

// Called when the combobox changes index
void LabelWidget::modeChanged(int index) {
	auto mode = static_cast<TextLabel::Mode>(index);
	bool plain = (mode != TextLabel::Mode::Text);

	labelModeChanged(mode);

	if (m_initializing)
		return;

	QString text = plain ? ui.teLabel->toPlainText() : ui.teLabel->toHtml();
	TextLabel::TextWrapper wrapper(text, mode, !plain);
	DEBUG(Q_FUNC_INFO << ", text = " << STDSTRING(wrapper.text))
	for (auto* label : m_labelsList)
		label->setText(wrapper);
}

void LabelWidget::fontColorChanged(const QColor& color) {
	if (m_initializing)
		return;

	auto mode = m_label->text().mode;
	if ((!m_teXEnabled && mode == TextLabel::Mode::LaTeX) || mode == TextLabel::Mode::Text) {
		auto cursor = ui.teLabel->textCursor();
		bool deselect = false;
		if (!cursor.hasSelection()) {
			ui.teLabel->selectAll();
			deselect = true;
		}
		ui.teLabel->setTextColor(color);
		if (deselect) {
			cursor.clearSelection();
			ui.teLabel->setTextCursor(cursor);
		}
	} else {
		for (auto* label : m_labelsList)
			label->setFontColor(color);
	}
}

void LabelWidget::backgroundColorChanged(const QColor& color) {
	QDEBUG(Q_FUNC_INFO << ", color = " << color)
	if (m_initializing)
		return;

	auto mode = m_label->text().mode;
	DEBUG(Q_FUNC_INFO << ", tex enable = " << m_teXEnabled << ", mode = " << (int)mode)
	if ((!m_teXEnabled && mode == TextLabel::Mode::LaTeX) || mode == TextLabel::Mode::Text) {
		DEBUG(Q_FUNC_INFO << ", update background color")
		auto cursor = ui.teLabel->textCursor();
		bool deselect = false;
		if (!cursor.hasSelection()) {
			ui.teLabel->selectAll();
			deselect = true;
		}

		ui.teLabel->setTextBackgroundColor(color);
		if (deselect) {
			cursor.clearSelection();
			ui.teLabel->setTextCursor(cursor);
		}
	} else {
		// Latex text does not support html code. For this the backgroundColor variable is used
		// Only single color background is supported
		for (auto* label : m_labelsList)
			label->setBackgroundColor(color);
	}
}

void LabelWidget::fontSizeChanged(int value) {
	if (m_initializing)
		return;

	QFont font = m_label->teXFont();
	font.setPointSize(value);
	for (auto* label : m_labelsList)
		label->setTeXFont(font);
}

void LabelWidget::fontBoldChanged(bool checked) {
	if (m_initializing)
		return;

	QTextCursor c = ui.teLabel->textCursor();
	if (c.selectedText().isEmpty())
		ui.teLabel->selectAll();

	if (checked)
		ui.teLabel->setFontWeight(QFont::Bold);
	else
		ui.teLabel->setFontWeight(QFont::Normal);
}

void LabelWidget::fontItalicChanged(bool checked) {
	if (m_initializing)
		return;

	QTextCursor c = ui.teLabel->textCursor();
	if (c.selectedText().isEmpty())
		ui.teLabel->selectAll();

	ui.teLabel->setFontItalic(checked);
}

void LabelWidget::fontUnderlineChanged(bool checked) {
	if (m_initializing)
		return;

	QTextCursor c = ui.teLabel->textCursor();
	if (c.selectedText().isEmpty())
		ui.teLabel->selectAll();

	ui.teLabel->setFontUnderline(checked);
}

void LabelWidget::fontStrikeOutChanged(bool checked) {
	if (m_initializing)
		return;

	QTextCursor c = ui.teLabel->textCursor();
	if (c.selectedText().isEmpty())
		ui.teLabel->selectAll();

	QTextCharFormat format = ui.teLabel->currentCharFormat();
	format.setFontStrikeOut(checked);
	ui.teLabel->setCurrentCharFormat(format);
}

void LabelWidget::fontSuperScriptChanged(bool checked) {
	if (m_initializing)
		return;

	QTextCursor c = ui.teLabel->textCursor();
	if (c.selectedText().isEmpty())
		ui.teLabel->selectAll();

	QTextCharFormat format = ui.teLabel->currentCharFormat();
	if (checked)
		format.setVerticalAlignment(QTextCharFormat::AlignSuperScript);
	else
		format.setVerticalAlignment(QTextCharFormat::AlignNormal);

	ui.teLabel->setCurrentCharFormat(format);
}

void LabelWidget::fontSubScriptChanged(bool checked) {
	if (m_initializing)
		return;

	QTextCursor c = ui.teLabel->textCursor();
	if (c.selectedText().isEmpty())
		ui.teLabel->selectAll();

	QTextCharFormat format = ui.teLabel->currentCharFormat();
	if (checked)
		format.setVerticalAlignment(QTextCharFormat::AlignSubScript);
	else
		format.setVerticalAlignment(QTextCharFormat::AlignNormal);

	ui.teLabel->setCurrentCharFormat(format);
}

void LabelWidget::fontChanged(const QFont& font) {
	if (m_initializing)
		return;

	const auto c = ui.teLabel->textCursor();
	if (c.selectedText().isEmpty())
		ui.teLabel->selectAll();

	// use mergeCurrentCharFormat(QTextCharFormat) instead of setFontFamily(font.family()), etc.
	// because this avoids textChanged() after every command
	QTextCharFormat format;
	format.setFontFamily(font.family());
#if (QT_VERSION >= QT_VERSION_CHECK(5, 13, 0))
	format.setFontFamilies({font.family()});	// see QTBUG-80475
#endif
	format.setFontPointSize(font.pointSize());
	format.setFontItalic(font.italic());
	format.setFontWeight(font.weight());
	if (font.underline())
		format.setUnderlineStyle(QTextCharFormat::UnderlineStyle::SingleUnderline);
	if (font.strikeOut())
		format.setFontStrikeOut(font.strikeOut());

	//QDEBUG(Q_FUNC_INFO << ", BEFORE:" << ui.teLabel->toHtml())
	ui.teLabel->mergeCurrentCharFormat(format);
	//QDEBUG(Q_FUNC_INFO << ", AFTER :" << ui.teLabel->toHtml())
}

void LabelWidget::teXFontChanged(const QFont& font) {
	if (m_initializing)
		return;

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

	QPoint pos(-menu.sizeHint().width()+ui.tbSymbols->width(),-menu.sizeHint().height());
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
		m_dateTimeMenu->addAction( date.toString(Qt::TextDate) );
		m_dateTimeMenu->addAction( date.toString(Qt::ISODate) );
		m_dateTimeMenu->addAction( QLocale::system().toString(date, QLocale::ShortFormat) );
		m_dateTimeMenu->addAction( QLocale::system().toString(date, QLocale::LongFormat) );
		m_dateTimeMenu->addAction( date.toString(Qt::RFC2822Date) );

		QDateTime time = QDateTime::currentDateTime();
		m_dateTimeMenu->addSeparator()->setText(i18n("Date and Time"));
		m_dateTimeMenu->addAction( time.toString(Qt::TextDate) );
		m_dateTimeMenu->addAction( time.toString(Qt::ISODate) );
		m_dateTimeMenu->addAction( QLocale::system().toString(time, QLocale::ShortFormat) );
		m_dateTimeMenu->addAction( QLocale::system().toString(time, QLocale::LongFormat) );
		m_dateTimeMenu->addAction( time.toString(Qt::RFC2822Date) );
	} else {
		//application language was changed:
		//determine the currently used language and use QLocale::toString()
		//to get the strings translated into the currently used language
		//TODO: why not use QLocale() ?
		QSettings settings (configFile, QSettings::IniFormat);
		settings.beginGroup(QLatin1String("Language"));
		QByteArray languageCode;
		languageCode = settings.value(qAppName(), languageCode).toByteArray();
		QLocale locale(QString::fromLatin1(languageCode.data()));

		QDate date = QDate::currentDate();
		m_dateTimeMenu->addSeparator()->setText(i18n("Date"));
		m_dateTimeMenu->addAction( locale.toString(date, QLatin1String("ddd MMM d yyyy")) ); //Qt::TextDate
		m_dateTimeMenu->addAction( locale.toString(date, QLatin1String("yyyy-MM-dd")) ); //Qt::ISODate
		m_dateTimeMenu->addAction( locale.system().toString(date, QLocale::ShortFormat) ); //Qt::SystemLocaleShortDate
		//no LongFormat here since it would contain strings in system's language which (potentially) is not the current application language
		m_dateTimeMenu->addAction( locale.toString(date, QLatin1String("dd MMM yyyy")) ); //Qt::RFC2822Date

		QDateTime time = QDateTime::currentDateTime();
		m_dateTimeMenu->addSeparator()->setText(i18n("Date and Time"));
		m_dateTimeMenu->addAction( locale.toString(time, QLatin1String("ddd MMM d hh:mm:ss yyyy")) ); //Qt::TextDate
		m_dateTimeMenu->addAction( locale.toString(time, QLatin1String("yyyy-MM-ddTHH:mm:ss")) ); //Qt::ISODate
		m_dateTimeMenu->addAction( locale.system().toString(time, QLocale::ShortFormat) ); //Qt::SystemLocaleShortDate
		//no LongFormat here since it would contain strings in system's language which (potentially) is not the current application language

		//TODO: RFC2822 requires time zone but Qt QLocale::toString() seems to ignore TZD (time zone designator) completely,
		//which works correctly with QDateTime::toString()
		m_dateTimeMenu->addAction( locale.toString(time, QLatin1String("dd MMM yyyy hh:mm:ss")) ); //Qt::RFC2822Date
	}

	m_dateTimeMenu->exec( mapToGlobal(ui.tbDateTime->rect().bottomLeft()));
}

void LabelWidget::insertDateTime(QAction* action) {
	ui.teLabel->insertPlainText( action->text().remove('&') );
}

// positioning using absolute coordinates
/*!
    called when label's current horizontal position relative to its parent (left, center, right ) is changed.
*/
void LabelWidget::positionXChanged(int index) {

	if (m_initializing)
		return;

	auto horPos = TextLabel::HorizontalPosition(index);
	for (auto* label : m_labelsList) {
		auto position = label->position();
		position.horizontalPosition = horPos;
		label->setPosition(position);
	}
}

/*!
    called when label's current horizontal position relative to its parent (top, center, bottom) is changed.
*/
void LabelWidget::positionYChanged(int index) {

	if (m_initializing)
		return;

	auto verPos = TextLabel::VerticalPosition(index);
	for (auto* label : m_labelsList) {
		auto position = label->position();
		position.verticalPosition = verPos;
		label->setPosition(position);
	}
}

void LabelWidget::horizontalAlignmentChanged(int index) {
	if (m_initializing)
		return;

	for (auto* label : m_labelsList)
		label->setHorizontalAlignment(TextLabel::HorizontalAlignment(index));
}

void LabelWidget::verticalAlignmentChanged(int index) {
	if (m_initializing)
		return;

	for (auto* label : m_labelsList)
		label->setVerticalAlignment(TextLabel::VerticalAlignment(index));
}

void LabelWidget::customPositionXChanged(double value) {
	if (m_initializing)
		return;

	const double x = Worksheet::convertToSceneUnits(value, m_worksheetUnit);
	for (auto* label : m_labelsList) {
		auto position = label->position();
		position.point.setX(x);
		label->setPosition(position);
	}
}

void LabelWidget::customPositionYChanged(double value) {
	if (m_initializing)
		return;

	const double y = Worksheet::convertToSceneUnits(value, m_worksheetUnit);
	for (auto* label : m_labelsList) {
		auto position = label->position();
		position.point.setY(y);
		label->setPosition(position);
	}
}


//positioning using logical plot coordinates
void LabelWidget::positionXLogicalChanged(const QString& value) {
	if (m_initializing)
		return;

	const Lock lock(m_initializing);
	bool ok;
	SET_NUMBER_LOCALE
	const double x = numberLocale.toDouble(value, &ok);
	if (ok) {
		QPointF pos = m_label->positionLogical();
		pos.setX(x);
		for (auto* label : m_labelsList)
			label->setPositionLogical(pos);
	}
}

void LabelWidget::positionXLogicalDateTimeChanged(const QDateTime& dateTime) {
	if (m_initializing)
		return;

	quint64 x = dateTime.toMSecsSinceEpoch();
	QPointF pos = m_label->positionLogical();
	pos.setX(x);
	for (auto* label : m_labelsList)
		label->setPositionLogical(pos);
}

void LabelWidget::positionYLogicalChanged(const QString& value) {
	if (m_initializing)
		return;

	const Lock lock(m_initializing);
	bool ok;
	SET_NUMBER_LOCALE
	const double y = numberLocale.toDouble(value, &ok);
	if (ok) {
		QPointF pos = m_label->positionLogical();
		pos.setY(y);
		for (auto* label : m_labelsList)
			label->setPositionLogical(pos);
	}
}

void LabelWidget::rotationChanged(int value) {
	if (m_initializing)
		return;

	for (auto* label : m_labelsList)
		label->setRotationAngle(value);
}

void LabelWidget::offsetXChanged(double value) {
	if (m_initializing)
		return;

	for (auto* axis : m_axesList)
		axis->setTitleOffsetX( Worksheet::convertToSceneUnits(value, Worksheet::Unit::Point) );
}

void LabelWidget::offsetYChanged(double value) {
	if (m_initializing)
		return;

	for (auto* axis : m_axesList)
		axis->setTitleOffsetY( Worksheet::convertToSceneUnits(value, Worksheet::Unit::Point) );
}

void LabelWidget::visibilityChanged(bool state) {
	if (m_initializing)
		return;

	for (auto* label : m_labelsList)
		label->setVisible(state);
}

//border
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

	if (m_initializing)
		return;

	for (auto* label : m_labelsList)
		label->setBorderShape(shape);
}

void LabelWidget::borderStyleChanged(int index) {
	if (m_initializing)
		return;

	auto penStyle = Qt::PenStyle(index);
	QPen pen;
	for (auto* label : m_labelsList) {
		pen = label->borderPen();
		pen.setStyle(penStyle);
		label->setBorderPen(pen);
	}
}

void LabelWidget::borderColorChanged(const QColor& color) {
	if (m_initializing)
		return;

	QPen pen;
	for (auto* label : m_labelsList) {
		pen = label->borderPen();
		pen.setColor(color);
		label->setBorderPen(pen);
	}

	m_initializing = true;
	GuiTools::updatePenStyles(ui.cbBorderStyle, color);
	m_initializing = false;
}

void LabelWidget::borderWidthChanged(double value) {
	if (m_initializing)
		return;

	QPen pen;
	for (auto* label : m_labelsList) {
		pen = label->borderPen();
		pen.setWidthF( Worksheet::convertToSceneUnits(value, Worksheet::Unit::Point) );
		label->setBorderPen(pen);
	}
}

void LabelWidget::borderOpacityChanged(int value) {
	if (m_initializing)
		return;

	qreal opacity = (float)value/100.;
	for (auto* label : m_labelsList)
		label->setBorderOpacity(opacity);
}

/*!
 * \brief LabelWidget::bindingChanged
 * Bind TextLabel to the cartesian plot coords or not
 * \param checked
 */
void LabelWidget::bindingChanged(bool checked) {
	ui.chbBindLogicalPos->setChecked(checked);

	//widgets for positioning using absolute plot distances
	ui.lPositionX->setVisible(!checked);
	ui.cbPositionX->setVisible(!checked);
	ui.sbPositionX->setVisible(!checked);

	ui.lPositionY->setVisible(!checked);
	ui.cbPositionY->setVisible(!checked);
	ui.sbPositionY->setVisible(!checked);

	//widgets for positioning using logical plot coordinates
	const auto* plot = static_cast<const CartesianPlot*>(m_label->parent(AspectType::CartesianPlot));
	if (plot && plot->xRangeFormat() == RangeT::Format::DateTime) {
		ui.lPositionXLogicalDateTime->setVisible(checked);
		ui.dtePositionXLogical->setVisible(checked);

		ui.lPositionXLogical->setVisible(false);
		ui.lePositionXLogical->setVisible(false);
	} else {
		ui.lPositionXLogicalDateTime->setVisible(false);
		ui.dtePositionXLogical->setVisible(false);

		ui.lPositionXLogical->setVisible(checked);
		ui.lePositionXLogical->setVisible(checked);
	}

	ui.lPositionYLogical->setVisible(checked);
	ui.lePositionYLogical->setVisible(checked);

	if(m_initializing)
		return;

	for (auto* label : m_labelsList)
		label->setCoordinateBindingEnabled(checked);
}

void LabelWidget::showPlaceholderTextChanged(bool checked) {
	if(m_initializing)
		return;
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
	if (m_initializing)return;
	const Lock lock(m_initializing);

	//save and restore the current cursor position after changing the text
	auto cursor = ui.teLabel->textCursor();
	int position = cursor.position();
	if(!ui.chbShowPlaceholderText->isChecked()) {
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
void LabelWidget::labelTeXImageUpdated(bool valid) {
	if (!valid) {
		if (ui.teLabel->styleSheet().isEmpty())
			SET_WARNING_STYLE(ui.teLabel)
	} else
		ui.teLabel->setStyleSheet(QString());
}

void LabelWidget::labelTeXFontChanged(const QFont& font) {
	const Lock lock(m_initializing);
	ui.kfontRequesterTeX->setFont(font);
	ui.sbFontSize->setValue(font.pointSize());
}

// this function is only called when the theme is changed. Otherwise the color is coded in the html text.
// when the theme changes, the whole text should change color regardless of the color it has
void LabelWidget::labelFontColorChanged(const QColor& color) {
	QDEBUG(Q_FUNC_INFO << ", COLOR = " << color)
	const Lock lock(m_initializing);
	ui.kcbFontColor->setColor(color);
	ui.teLabel->selectAll();
	ui.teLabel->setTextColor(color);
}

void LabelWidget::labelPositionChanged(const TextLabel::PositionWrapper& position) {
	const Lock lock(m_initializing);
	ui.sbPositionX->setValue( Worksheet::convertFromSceneUnits(position.point.x(), m_worksheetUnit) );
	ui.sbPositionY->setValue( Worksheet::convertFromSceneUnits(position.point.y(), m_worksheetUnit) );
	ui.cbPositionX->setCurrentIndex( static_cast<int>(position.horizontalPosition) );
	ui.cbPositionY->setCurrentIndex( static_cast<int>(position.verticalPosition) );
}

void LabelWidget::labelHorizontalAlignmentChanged(TextLabel::HorizontalAlignment index) {
	m_initializing = true;
	ui.cbHorizontalAlignment->setCurrentIndex(static_cast<int>(index));
	m_initializing = false;
}

void LabelWidget::labelVerticalAlignmentChanged(TextLabel::VerticalAlignment index) {
	m_initializing = true;
	ui.cbVerticalAlignment->setCurrentIndex(static_cast<int>(index));
	m_initializing = false;
}

void LabelWidget::labelCoordinateBindingEnabledChanged(bool enabled) {
	const Lock lock(m_initializing);
	bindingChanged(enabled);
}

void LabelWidget::labelPositionLogicalChanged(QPointF pos) {
	const Lock lock(m_initializing);
	SET_NUMBER_LOCALE
	ui.lePositionXLogical->setText(numberLocale.toString(pos.x()));
	ui.dtePositionXLogical->setDateTime(QDateTime::fromMSecsSinceEpoch(pos.x()));
	ui.lePositionYLogical->setText(numberLocale.toString(pos.y()));
}

// this function is only called when the theme is changed. Otherwise the color is coded in the html text.
// when the theme changes, the whole text should change color regardless of the color it has
void LabelWidget::labelBackgroundColorChanged(const QColor& color) {
	QDEBUG(Q_FUNC_INFO << ", color =" << color)
	const Lock lock(m_initializing);
	ui.kcbBackgroundColor->setColor(color);
//	auto cursor = ui.teLabel->textCursor();
	ui.teLabel->selectAll();
	ui.teLabel->setTextBackgroundColor(color);
//	ui.teLabel->setTextCursor(cursor);	// restore cursor
}

void LabelWidget::labelOffsetXChanged(qreal offset) {
	const Lock lock(m_initializing);
	ui.sbOffsetX->setValue(Worksheet::convertFromSceneUnits(offset, Worksheet::Unit::Point));
}

void LabelWidget::labelOffsetYChanged(qreal offset) {
	const Lock lock(m_initializing);
	ui.sbOffsetY->setValue(Worksheet::convertFromSceneUnits(offset, Worksheet::Unit::Point));
}

void LabelWidget::labelRotationAngleChanged(qreal angle) {
	const Lock lock(m_initializing);
	ui.sbRotation->setValue(angle);
}

void LabelWidget::labelVisibleChanged(bool on) {
	const Lock lock(m_initializing);
	ui.chbVisible->setChecked(on);
}

//border
void LabelWidget::labelBorderShapeChanged(TextLabel::BorderShape shape) {
	const Lock lock(m_initializing);
	ui.cbBorderShape->setCurrentIndex(static_cast<int>(shape));
}

void LabelWidget::labelBorderPenChanged(const QPen& pen) {
	const Lock lock(m_initializing);
	if (ui.cbBorderStyle->currentIndex() != pen.style())
		ui.cbBorderStyle->setCurrentIndex(pen.style());
	if (ui.kcbBorderColor->color() != pen.color())
		ui.kcbBorderColor->setColor(pen.color());
	if (ui.sbBorderWidth->value() != pen.widthF())
		ui.sbBorderWidth->setValue(Worksheet::convertFromSceneUnits(pen.widthF(), Worksheet::Unit::Point));
}

void LabelWidget::labelBorderOpacityChanged(float value) {
	const Lock lock(m_initializing);
	float v = (float)value*100.;
	ui.sbBorderOpacity->setValue(v);
}

void LabelWidget::labelCartesianPlotParent(bool on) {
	const Lock lock(m_initializing);
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

	const Lock lock(m_initializing);

	ui.chbVisible->setChecked(m_label->isVisible());

	// don't show checkbox if Placeholder feature not used
	bool allowPlaceholder = m_label->text().allowPlaceholder;
	ui.chbShowPlaceholderText->setVisible(allowPlaceholder);
	ui.chbShowPlaceholderText->setEnabled(allowPlaceholder);
	ui.chbShowPlaceholderText->setChecked(allowPlaceholder);

	//Text
	const auto mode = m_label->text().mode;
	ui.cbMode->setCurrentIndex(static_cast<int>(mode));
	this->updateMode(mode);

	if(!allowPlaceholder) {
		if (mode == TextLabel::Mode::Text) {
			ui.teLabel->setHtml(m_label->text().text);
			ui.teLabel->selectAll(); // must be done to retrieve font
			ui.kfontRequester->setFont(ui.teLabel->currentFont());
		} else
			ui.teLabel->setText(m_label->text().text);

	} else {
		if (mode == TextLabel::Mode::Text) {
			ui.teLabel->setHtml(m_label->text().textPlaceholder);
			ui.teLabel->selectAll(); // must be done to retrieve font
			ui.kfontRequester->setFont(ui.teLabel->currentFont());
		} else
			ui.teLabel->setText(m_label->text().textPlaceholder);
	}

	// if the text is empty, use LabelWidget::fontColor(),
	// extract the color from the html formatted text otherwise
	auto format = ui.teLabel->currentCharFormat();
//	QDEBUG(Q_FUNC_INFO << ", format color = " << format.foreground().color())
//	QDEBUG(Q_FUNC_INFO << ", format bg color = " << format.background().color())
//	QDEBUG(Q_FUNC_INFO << ", label color = " << m_label->fontColor())
//	QDEBUG(Q_FUNC_INFO << ", label bg color = " << m_label->backgroundColor())
	if (m_label->text().text.isEmpty() || mode == TextLabel::Mode::Text) {
		ui.kcbFontColor->setColor(m_label->fontColor());
		ui.kcbBackgroundColor->setColor(m_label->backgroundColor());
	} else {
		ui.kcbFontColor->setColor(format.foreground().color());
		ui.kcbBackgroundColor->setColor(format.background().color());
	}

	ui.kfontRequesterTeX->setFont(m_label->teXFont());
	ui.sbFontSize->setValue(m_label->teXFont().pointSize());

	ui.tbFontBold->setChecked(ui.teLabel->fontWeight() == QFont::Bold);
	ui.tbFontItalic->setChecked(ui.teLabel->fontItalic());
	ui.tbFontUnderline->setChecked(ui.teLabel->fontUnderline());
	ui.tbFontStrikeOut->setChecked(format.fontStrikeOut());
	ui.tbFontSuperScript->setChecked(format.verticalAlignment() == QTextCharFormat::AlignSuperScript);
	ui.tbFontSubScript->setChecked(format.verticalAlignment() == QTextCharFormat::AlignSubScript);

	//move the cursor to the end
	QTextCursor cursor = ui.teLabel->textCursor();
	cursor.movePosition(QTextCursor::End);
	ui.teLabel->setTextCursor(cursor);
	//ui.teLabel->setFocus(); // Do not set focus, otherwise the WorksheetView is not able to catch key input events!

	// Geometry
	//widgets for positioning using absolute plot distances
	ui.cbPositionX->setCurrentIndex( (int)m_label->position().horizontalPosition );
	positionXChanged(ui.cbPositionX->currentIndex());
	ui.sbPositionX->setValue( Worksheet::convertFromSceneUnits(m_label->position().point.x(), m_worksheetUnit) );
	ui.cbPositionY->setCurrentIndex( (int)m_label->position().verticalPosition );
	positionYChanged(ui.cbPositionY->currentIndex());
	ui.sbPositionY->setValue( Worksheet::convertFromSceneUnits(m_label->position().point.y(), m_worksheetUnit) );

	ui.cbHorizontalAlignment->setCurrentIndex( (int) m_label->horizontalAlignment() );
	ui.cbVerticalAlignment->setCurrentIndex( (int) m_label->verticalAlignment() );

	//widgets for positioning using logical plot coordinates
	SET_NUMBER_LOCALE
	bool allowLogicalCoordinates = (m_label->plot() != nullptr);
	ui.lBindLogicalPos->setVisible(allowLogicalCoordinates);
	ui.chbBindLogicalPos->setVisible(allowLogicalCoordinates);

	if (allowLogicalCoordinates) {
		const auto* plot = static_cast<const CartesianPlot*>(m_label->plot());
		if (plot->xRangeFormat() == RangeT::Format::Numeric) {
			ui.lPositionXLogical->show();
			ui.lePositionXLogical->show();
			ui.lPositionXLogicalDateTime->hide();
			ui.dtePositionXLogical->hide();

			ui.lePositionXLogical->setText(numberLocale.toString(m_label->positionLogical().x()));
			ui.lePositionYLogical->setText(numberLocale.toString(m_label->positionLogical().y()));
		} else { //DateTime
			ui.lPositionXLogical->hide();
			ui.lePositionXLogical->hide();
			ui.lPositionXLogicalDateTime->show();
			ui.dtePositionXLogical->show();

			ui.dtePositionXLogical->setDisplayFormat(plot->xRangeDateTimeFormat());
			ui.dtePositionXLogical->setDateTime(QDateTime::fromMSecsSinceEpoch(m_label->positionLogical().x()));
		}

		bindingChanged(m_label->coordinateBindingEnabled());
	} else {
		ui.lPositionXLogical->hide();
		ui.lePositionXLogical->hide();
		ui.lPositionYLogical->hide();
		ui.lePositionYLogical->hide();
		ui.lPositionXLogicalDateTime->hide();
		ui.dtePositionXLogical->hide();
	}

	//offsets, available for axis label only
	if (!m_axesList.isEmpty()) {
		ui.sbOffsetX->setValue( Worksheet::convertFromSceneUnits(m_axesList.first()->titleOffsetX(), Worksheet::Unit::Point) );
		ui.sbOffsetY->setValue( Worksheet::convertFromSceneUnits(m_axesList.first()->titleOffsetY(), Worksheet::Unit::Point) );
	}
	ui.sbRotation->setValue( m_label->rotationAngle() );

	// don't show if binding not enabled. example: axis titles
	//Border
	ui.cbBorderShape->setCurrentIndex(static_cast<int>(m_label->borderShape()));
	borderShapeChanged(ui.cbBorderShape->currentIndex());
	ui.kcbBorderColor->setColor( m_label->borderPen().color() );
	ui.cbBorderStyle->setCurrentIndex( (int) m_label->borderPen().style() );
	ui.sbBorderWidth->setValue( Worksheet::convertFromSceneUnits(m_label->borderPen().widthF(), Worksheet::Unit::Point) );
	ui.sbBorderOpacity->setValue( round(m_label->borderOpacity()*100) );
	GuiTools::updatePenStyles(ui.cbBorderStyle, ui.kcbBorderColor->color());
}

// General updater function to update the dock (used also in the load method)
void LabelWidget::updateMode(TextLabel::Mode mode) {
	bool plain = (mode != TextLabel::Mode::Text);

	//hide text editing elements if TeX-option is used
	ui.tbFontBold->setVisible(!plain);
	ui.tbFontItalic->setVisible(!plain);

	//TODO: https://bugreports.qt.io/browse/QTBUG-25420
// 	ui.tbFontUnderline->setVisible(!plain);
// 	ui.tbFontStrikeOut->setVisible(!plain);

	ui.tbFontSubScript->setVisible(!plain);
	ui.tbFontSuperScript->setVisible(!plain);

	ui.lFont->setVisible(!plain);
	ui.kfontRequester->setVisible(!plain);

	if (plain) {
		//reset all applied formattings when switching from html to tex mode
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
		KConfigGroup conf(KSharedConfig::openConfig(), QLatin1String("Settings_Worksheet"));
		QString engine = conf.readEntry(QLatin1String("LaTeXEngine"), "");
		if (engine == QLatin1String("xelatex") || engine == QLatin1String("lualatex")) {
			ui.lFontTeX->setVisible(true);
			ui.kfontRequesterTeX->setVisible(true);
			ui.lFontSize->setVisible(false);
			ui.sbFontSize->setVisible(false);
		} else {
			ui.lFontTeX->setVisible(false);
			ui.kfontRequesterTeX->setVisible(false);
			ui.lFontSize->setVisible(true);
			ui.sbFontSize->setVisible(true);
		}

		//update TeX colors
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

	//when switching to non-LaTeX mode, set the background color to white just for the case the latex code provided by the user
	//in the TeX-mode is not valid and the background was set to red (s.a. LabelWidget::labelTeXImageUpdated())
	if (mode != TextLabel::Mode::LaTeX)
		ui.teLabel->setStyleSheet(QString());
}

void LabelWidget::loadConfig(KConfigGroup& group) {
	if (!m_label)
		return;

	m_initializing = true;

	//Text
	ui.cbMode->setCurrentIndex(group.readEntry("Mode", static_cast<int>(m_label->text().mode)));
	this->modeChanged(ui.cbMode->currentIndex());
	ui.sbFontSize->setValue( group.readEntry("TeXFontSize", m_label->teXFont().pointSize()) );
	ui.kcbFontColor->setColor( group.readEntry("TeXFontColor", m_label->fontColor()) );
	ui.kcbBackgroundColor->setColor( group.readEntry("TeXBackgroundColor", m_label->backgroundColor()) );
	ui.kfontRequesterTeX->setFont(group.readEntry("TeXFont", m_label->teXFont()));

	// Geometry
	ui.cbPositionX->setCurrentIndex( group.readEntry("PositionX", (int) m_label->position().horizontalPosition ) );
	ui.sbPositionX->setValue( Worksheet::convertFromSceneUnits(group.readEntry("PositionXValue", m_label->position().point.x()), m_worksheetUnit) );
	ui.cbPositionY->setCurrentIndex( group.readEntry("PositionY", (int) m_label->position().verticalPosition ) );
	ui.sbPositionY->setValue( Worksheet::convertFromSceneUnits(group.readEntry("PositionYValue", m_label->position().point.y()), m_worksheetUnit) );

	if (!m_axesList.isEmpty()) {
		ui.sbOffsetX->setValue( Worksheet::convertFromSceneUnits(group.readEntry("OffsetX", m_axesList.first()->titleOffsetX()), Worksheet::Unit::Point) );
		ui.sbOffsetY->setValue( Worksheet::convertFromSceneUnits(group.readEntry("OffsetY", m_axesList.first()->titleOffsetY()), Worksheet::Unit::Point) );
	}
	ui.cbHorizontalAlignment->setCurrentIndex( group.readEntry("HorizontalAlignment", (int) m_label->horizontalAlignment()) );
	ui.cbVerticalAlignment->setCurrentIndex( group.readEntry("VerticalAlignment", (int) m_label->verticalAlignment()) );
	ui.sbRotation->setValue( group.readEntry("Rotation", m_label->rotationAngle()) );

	//Border
	ui.cbBorderShape->setCurrentIndex(group.readEntry("BorderShape").toInt());
	ui.kcbBorderColor->setColor( group.readEntry("BorderColor", m_label->borderPen().color()) );
	ui.cbBorderStyle->setCurrentIndex( group.readEntry("BorderStyle", (int)m_label->borderPen().style()) );
	ui.sbBorderWidth->setValue( Worksheet::convertFromSceneUnits(group.readEntry("BorderWidth", m_label->borderPen().widthF()), Worksheet::Unit::Point) );
	ui.sbBorderOpacity->setValue( group.readEntry("BorderOpacity", m_label->borderOpacity())*100 );
	m_initializing = false;
}

void LabelWidget::saveConfig(KConfigGroup& group) {
	//Text
	group.writeEntry("Mode", ui.cbMode->currentIndex());
	group.writeEntry("TeXFontColor", ui.kcbFontColor->color());
	group.writeEntry("TeXBackgroundColor", ui.kcbBackgroundColor->color());
	group.writeEntry("TeXFont", ui.kfontRequesterTeX->font());

	// Geometry
	group.writeEntry("PositionX", ui.cbPositionX->currentIndex());
	group.writeEntry("PositionXValue", Worksheet::convertToSceneUnits(ui.sbPositionX->value(),m_worksheetUnit) );
	group.writeEntry("PositionY", ui.cbPositionY->currentIndex());
	group.writeEntry("PositionYValue",  Worksheet::convertToSceneUnits(ui.sbPositionY->value(),m_worksheetUnit) );

	if (!m_axesList.isEmpty()) {
		group.writeEntry("OffsetX",  Worksheet::convertToSceneUnits(ui.sbOffsetX->value(), Worksheet::Unit::Point) );
		group.writeEntry("OffsetY",  Worksheet::convertToSceneUnits(ui.sbOffsetY->value(), Worksheet::Unit::Point) );
	}
	group.writeEntry("HorizontalAlignment", ui.cbHorizontalAlignment->currentIndex());
	group.writeEntry("VerticalAlignment", ui.cbVerticalAlignment->currentIndex());
	group.writeEntry("Rotation", ui.sbRotation->value());

	//Border
	group.writeEntry("BorderShape", ui.cbBorderShape->currentIndex());
	group.writeEntry("BorderStyle", ui.cbBorderStyle->currentIndex());
	group.writeEntry("BorderColor", ui.kcbBorderColor->color());
	group.writeEntry("BorderWidth", Worksheet::convertToSceneUnits(ui.sbBorderWidth->value(), Worksheet::Unit::Point));
	group.writeEntry("BorderOpacity", ui.sbBorderOpacity->value()/100.0);
}
