/***************************************************************************
    File                 : LabelWidget.cc
    Project              : LabPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2008-2017 Alexander Semke (alexander.semke@web.de)
    Copyright            : (C) 2012-2017 Stefan Gerlach (stefan.gerlach@uni-konstanz.de)
    Description          : label settings widget

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
#include "LabelWidget.h"
#include "backend/worksheet/Worksheet.h"
#include "backend/worksheet/plots/cartesian/Axis.h"
#include "tools/TeXRenderer.h"

#include <QMenu>
#include <QWidgetAction>
#include <QSplitter>

#include <KConfigGroup>
#include <KSharedConfig>
#include <KCharSelect>
#include <KLocalizedString>
#ifdef HAVE_KF5_SYNTAX_HIGHLIGHTING
#include <KF5/KSyntaxHighlighting/SyntaxHighlighter>
#include <KF5/KSyntaxHighlighting/Definition>
#include <KF5/KSyntaxHighlighting/Theme>
#endif

/*!
	\class LabelWidget
 	\brief Widget for editing the properties of a TextLabel object, mostly used in an appropriate dock widget.

	In order the properties of the label to be shown, \c loadConfig() has to be called with the correspondig KConfigGroup
	(settings for a label in *Plot, Axis etc. or for an independent label on the worksheet).

	\ingroup kdefrontend
 */
LabelWidget::LabelWidget(QWidget* parent) : QWidget(parent),
	m_label(nullptr),
	m_initializing(false),
	m_dateTimeMenu(new QMenu(this)),
	m_teXEnabled(false) {

	ui.setupUi(this);

	m_dateTimeMenu->setSeparatorsCollapsible(false); //we don't want the first separator to be removed

	ui.kcbFontColor->setColor(Qt::black); // default color

	//Icons
	ui.tbFontBold->setIcon( QIcon::fromTheme(QLatin1String("format-text-bold")) );
	ui.tbFontItalic->setIcon( QIcon::fromTheme(QLatin1String("format-text-italic")) );
	ui.tbFontUnderline->setIcon( QIcon::fromTheme(QLatin1String("format-text-underline")) );
	ui.tbFontStrikeOut->setIcon( QIcon::fromTheme(QLatin1String("format-text-strikethrough")) );
	ui.tbFontSuperScript->setIcon( QIcon::fromTheme(QLatin1String("format-text-superscript")) );
	ui.tbFontSubScript->setIcon( QIcon::fromTheme(QLatin1String("format-text-subscript")) );
	ui.tbSymbols->setIcon( QIcon::fromTheme(QLatin1String("labplot-format-text-symbol")) );
	ui.tbDateTime->setIcon( QIcon::fromTheme(QLatin1String("chronometer")) );
	ui.tbTexUsed->setIcon( QIcon::fromTheme(QLatin1String("labplot-TeX-logo")) );

	//Positioning and alignment
	ui.cbPositionX->addItem(i18n("Left"));
	ui.cbPositionX->addItem(i18n("Center"));
	ui.cbPositionX->addItem(i18n("Right"));
	ui.cbPositionX->addItem(i18n("Custom"));

	ui.cbPositionY->addItem(i18n("Top"));
	ui.cbPositionY->addItem(i18n("Center"));
	ui.cbPositionY->addItem(i18n("Bottom"));
	ui.cbPositionY->addItem(i18n("Custom"));

	ui.cbHorizontalAlignment->addItem(i18n("Left"));
	ui.cbHorizontalAlignment->addItem(i18n("Center"));
	ui.cbHorizontalAlignment->addItem(i18n("Right"));

	ui.cbVerticalAlignment->addItem(i18n("Top"));
	ui.cbVerticalAlignment->addItem(i18n("Center"));
	ui.cbVerticalAlignment->addItem(i18n("Bottom"));

	//check whether the used latex compiler is available.
	//Following logic is implemented (s.a. LabelWidget::teXUsedChanged()):
	//1. in case latex was used to generate the text label in the stored project
	//and no latex is available on the target system, latex button is toggled and
	//the user still can switch to the non-latex mode.
	//2. in case the label was in the non-latex mode and no latex is available,
	//deactivate the latex button so the user cannot switch to this mode.
	m_teXEnabled = TeXRenderer::enabled();

#ifdef HAVE_KF5_SYNTAX_HIGHLIGHTING
	m_highlighter = new KSyntaxHighlighting::SyntaxHighlighter(ui.teLabel->document());
	m_highlighter->setDefinition(m_repository.definitionForName(QLatin1String("LaTeX")));
	m_highlighter->setTheme(  (palette().color(QPalette::Base).lightness() < 128)
								? m_repository.defaultTheme(KSyntaxHighlighting::Repository::DarkTheme)
								: m_repository.defaultTheme(KSyntaxHighlighting::Repository::LightTheme) );
#endif

	//SLOTS
	// text properties
	connect(ui.tbTexUsed, SIGNAL(clicked(bool)), this, SLOT(teXUsedChanged(bool)) );
	connect(ui.teLabel, SIGNAL(textChanged()), this, SLOT(textChanged()));
	connect(ui.teLabel, SIGNAL(currentCharFormatChanged(QTextCharFormat)),
	        this, SLOT(charFormatChanged(QTextCharFormat)));
	connect(ui.kcbFontColor, SIGNAL(changed(QColor)), this, SLOT(fontColorChanged(QColor)));
	connect(ui.kcbBackgroundColor, SIGNAL(changed(QColor)), this, SLOT(backgroundColorChanged(QColor)));
	connect(ui.tbFontBold, SIGNAL(clicked(bool)), this, SLOT(fontBoldChanged(bool)));
	connect(ui.tbFontItalic, SIGNAL(clicked(bool)), this, SLOT(fontItalicChanged(bool)));
	connect(ui.tbFontUnderline, SIGNAL(clicked(bool)), this, SLOT(fontUnderlineChanged(bool)));
	connect(ui.tbFontStrikeOut, SIGNAL(clicked(bool)), this, SLOT(fontStrikeOutChanged(bool)));
	connect(ui.tbFontSuperScript, SIGNAL(clicked(bool)), this, SLOT(fontSuperScriptChanged(bool)));
	connect(ui.tbFontSubScript, SIGNAL(clicked(bool)), this, SLOT(fontSubScriptChanged(bool)));
	connect(ui.tbSymbols, SIGNAL(clicked(bool)), this, SLOT(charMenu()));
	connect(ui.tbDateTime, SIGNAL(clicked(bool)), this, SLOT(dateTimeMenu()));
	connect(m_dateTimeMenu, SIGNAL(triggered(QAction*)), this, SLOT(insertDateTime(QAction*)) );
	connect(ui.kfontRequester, SIGNAL(fontSelected(QFont)), this, SLOT(fontChanged(QFont)));
	connect(ui.kfontRequesterTeX, SIGNAL(fontSelected(QFont)), this, SLOT(teXFontChanged(QFont)));
	connect(ui.sbFontSize, SIGNAL(valueChanged(int)), this, SLOT(fontSizeChanged(int)) );

	// geometry
	connect( ui.cbPositionX, SIGNAL(currentIndexChanged(int)), this, SLOT(positionXChanged(int)) );
	connect( ui.cbPositionY, SIGNAL(currentIndexChanged(int)), this, SLOT(positionYChanged(int)) );
	connect( ui.sbPositionX, SIGNAL(valueChanged(double)), this, SLOT(customPositionXChanged(double)) );
	connect( ui.sbPositionY, SIGNAL(valueChanged(double)), this, SLOT(customPositionYChanged(double)) );
	connect( ui.cbHorizontalAlignment, SIGNAL(currentIndexChanged(int)), this, SLOT(horizontalAlignmentChanged(int)) );
	connect( ui.cbVerticalAlignment, SIGNAL(currentIndexChanged(int)), this, SLOT(verticalAlignmentChanged(int)) );
	connect( ui.sbRotation, SIGNAL(valueChanged(int)), this, SLOT(rotationChanged(int)) );
	connect( ui.sbOffsetX, SIGNAL(valueChanged(double)), this, SLOT(offsetXChanged(double)) );
	connect( ui.sbOffsetY, SIGNAL(valueChanged(double)), this, SLOT(offsetYChanged(double)) );

	connect( ui.chbVisible, SIGNAL(clicked(bool)), this, SLOT(visibilityChanged(bool)) );

	//TODO: https://bugreports.qt.io/browse/QTBUG-25420
	ui.tbFontUnderline->hide();
	ui.tbFontStrikeOut->hide();
}

void LabelWidget::setLabels(QList<TextLabel*> labels) {
	m_labelsList = labels;
	m_label = labels.first();

	ui.lOffsetX->hide();
	ui.lOffsetY->hide();

	ui.sbOffsetX->hide();
	ui.sbOffsetY->hide();

	this->load();
	initConnections();
}

void LabelWidget::setAxes(QList<Axis*> axes) {
	m_labelsList.clear();
	for (auto* axis : axes) {
		m_labelsList.append(axis->title());
		connect(axis, SIGNAL(titleOffsetXChanged(qreal)), this, SLOT(labelOffsetxChanged(qreal)) );
		connect(axis, SIGNAL(titleOffsetYChanged(qreal)), this, SLOT(labelOffsetyChanged(qreal)) );
		connect(axis->title(), SIGNAL(rotationAngleChanged(qreal)), this, SLOT(labelRotationAngleChanged(qreal)) );
	}

	m_axesList = axes;
	m_label = m_labelsList.first();

	this->load();
	initConnections();
}

void LabelWidget::initConnections() const {
	connect( m_label, SIGNAL(textWrapperChanged(TextLabel::TextWrapper)),
	         this, SLOT(labelTextWrapperChanged(TextLabel::TextWrapper)) );
	connect( m_label, SIGNAL(teXImageUpdated(bool)), this, SLOT(labelTeXImageUpdated(bool)) );
	connect( m_label, SIGNAL(teXFontChanged(QFont)),
	         this, SLOT(labelTeXFontChanged(QFont)) );
	connect( m_label, SIGNAL(teXFontColorChanged(QColor)),
	         this, SLOT(labelTeXFontColorChanged(QColor)) );
	connect( m_label, SIGNAL(positionChanged(TextLabel::PositionWrapper)),
	         this, SLOT(labelPositionChanged(TextLabel::PositionWrapper)) );
	connect( m_label, SIGNAL(horizontalAlignmentChanged(TextLabel::HorizontalAlignment)),
	         this, SLOT(labelHorizontalAlignmentChanged(TextLabel::HorizontalAlignment)) );
	connect( m_label, SIGNAL(verticalAlignmentChanged(TextLabel::VerticalAlignment)),
	         this, SLOT(labelVerticalAlignmentChanged(TextLabel::VerticalAlignment)) );
	connect( m_label, SIGNAL(rotationAngleChanged(qreal)), this, SLOT(labelRotationAngleChanged(qreal)) );
	connect( m_label, SIGNAL(visibleChanged(bool)), this, SLOT(labelVisibleChanged(bool)) );
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
void LabelWidget::setNoGeometryMode(const bool b) {
	ui.lGeometry->setVisible(!b);
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
	ui.lOffsetX->setVisible(!b);
	ui.lOffsetY->setVisible(!b);
	ui.sbOffsetX->setVisible(!b);
	ui.sbOffsetY->setVisible(!b);
	ui.lRotation->setVisible(!b);
	ui.sbRotation->setVisible(!b);
}

//**********************************************************
//****** SLOTs for changes triggered in LabelWidget ********
//**********************************************************

// text formating slots

void LabelWidget::textChanged() {
	if (m_initializing)
		return;

	if (ui.tbTexUsed->isChecked()) {
		QString text=ui.teLabel->toPlainText();
		TextLabel::TextWrapper wrapper(text, true);

		for (auto* label : m_labelsList)
			label->setText(wrapper);
	} else {
		//save an empty string instead of a html-string with empty body, if no text available in QTextEdit
		QString text;
		if (ui.teLabel->toPlainText() == "")
			text = "";
		else
			text = ui.teLabel->toHtml();

		TextLabel::TextWrapper wrapper(text, false);
		for (auto* label : m_labelsList)
			label->setText(wrapper);
	}
}

void LabelWidget::charFormatChanged(const QTextCharFormat& format) {
	if(ui.tbTexUsed->isChecked())
		return;

	// update button state
	ui.tbFontBold->setChecked(ui.teLabel->fontWeight()==QFont::Bold);
	ui.tbFontItalic->setChecked(ui.teLabel->fontItalic());
	ui.tbFontUnderline->setChecked(ui.teLabel->fontUnderline());
	ui.tbFontStrikeOut->setChecked(format.fontStrikeOut());
	ui.tbFontSuperScript->setChecked(format.verticalAlignment() == QTextCharFormat::AlignSuperScript);
	ui.tbFontSubScript->setChecked(format.verticalAlignment() == QTextCharFormat::AlignSubScript);

	//font and colors
	ui.kcbFontColor->setColor(format.foreground().color());
	ui.kcbBackgroundColor->setColor(format.background().color());
	ui.kfontRequester->setFont(format.font());
}

void LabelWidget::teXUsedChanged(bool checked) {
	//hide text editing elements if TeX-option is used
	ui.tbFontBold->setVisible(!checked);
	ui.tbFontItalic->setVisible(!checked);

	//TODO: https://bugreports.qt.io/browse/QTBUG-25420
// 	ui.tbFontUnderline->setVisible(!checked);
// 	ui.tbFontStrikeOut->setVisible(!checked);

	ui.tbFontSubScript->setVisible(!checked);
	ui.tbFontSuperScript->setVisible(!checked);
	ui.tbSymbols->setVisible(!checked);

	ui.lFont->setVisible(!checked);
	ui.kfontRequester->setVisible(!checked);

	if (checked) {
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
		ui.kcbFontColor->setColor(m_label->teXFontColor());
		ui.kcbBackgroundColor->setColor(m_label->teXBackgroundColor());
	} else {
#ifdef HAVE_KF5_SYNTAX_HIGHLIGHTING
		m_highlighter->setDocument(nullptr);
#endif
		ui.lFontTeX->setVisible(false);
		ui.kfontRequesterTeX->setVisible(false);
		ui.lFontSize->setVisible(false);
		ui.sbFontSize->setVisible(false);

		//when switching to the text mode, set the background color to white just for the case the latex code provided by the user
		//in the TeX-mode is not valid and the background was set to red (s.a. LabelWidget::labelTeXImageUpdated())
		ui.teLabel->setStyleSheet(QLatin1String(""));
	}

	//no latex is available and the user switched to the text mode,
	//deactivate the button since it shouldn't be possible anymore to switch to the TeX-mode
	if (!m_teXEnabled && !checked) {
		ui.tbTexUsed->setEnabled(false);
		ui.tbTexUsed->setToolTip(i18n("LaTeX typesetting not possible. Please check the settings."));
	} else {
		ui.tbTexUsed->setEnabled(true);
		ui.tbTexUsed->setToolTip(QLatin1String(""));
	}

	if (m_initializing)
		return;

	QString text = checked ? ui.teLabel->toPlainText() : ui.teLabel->toHtml();
	TextLabel::TextWrapper wrapper(text, checked);
	for (auto* label : m_labelsList)
		label->setText(wrapper);
}

void LabelWidget::fontColorChanged(const QColor& color) {
	if (m_initializing)
		return;

	ui.teLabel->setTextColor(color);
	for (auto* label : m_labelsList)
		label->setTeXFontColor(color);
}

void LabelWidget::backgroundColorChanged(const QColor& color) {
	if (m_initializing)
		return;

	ui.teLabel->setTextBackgroundColor(color);
	for (auto* label : m_labelsList)
		label->setTeXBackgroundColor(color);
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

	if (checked)
		ui.teLabel->setFontWeight(QFont::Bold);
	else
		ui.teLabel->setFontWeight(QFont::Normal);
}

void LabelWidget::fontItalicChanged(bool checked) {
	if (m_initializing)
		return;

	ui.teLabel->setFontItalic(checked);
}

void LabelWidget::fontUnderlineChanged(bool checked) {
	if (m_initializing)
		return;

	ui.teLabel->setFontUnderline(checked);
}

void LabelWidget::fontStrikeOutChanged(bool checked) {
	if (m_initializing)
		return;

	QTextCharFormat format = ui.teLabel->currentCharFormat();
	format.setFontStrikeOut(checked);
	ui.teLabel->setCurrentCharFormat(format);
}

void LabelWidget::fontSuperScriptChanged(bool checked) {
	if (m_initializing)
		return;

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

	// underline and strike-out not included
	ui.teLabel->setFontFamily(font.family());
	ui.teLabel->setFontPointSize(font.pointSize());
	ui.teLabel->setFontItalic(font.italic());
	ui.teLabel->setFontWeight(font.weight());
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
	selection.setCurrentFont(ui.teLabel->currentFont());
	connect(&selection, SIGNAL(charSelected(QChar)), this, SLOT(insertChar(QChar)));
	connect(&selection, SIGNAL(charSelected(QChar)), &menu, SLOT(close()));

	QWidgetAction *widgetAction = new QWidgetAction(this);
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

	QDate date = QDate::currentDate();
	m_dateTimeMenu->addSeparator()->setText(i18n("Date"));
	m_dateTimeMenu->addAction( date.toString(Qt::TextDate) );
	m_dateTimeMenu->addAction( date.toString(Qt::ISODate) );
	m_dateTimeMenu->addAction( date.toString(Qt::TextDate) );
	m_dateTimeMenu->addAction( date.toString(Qt::SystemLocaleShortDate) );
	m_dateTimeMenu->addAction( date.toString(Qt::SystemLocaleLongDate) );

	QDateTime time = QDateTime::currentDateTime();
	m_dateTimeMenu->addSeparator()->setText(i18n("Date and Time"));
	m_dateTimeMenu->addAction( time.toString(Qt::TextDate) );
	m_dateTimeMenu->addAction( time.toString(Qt::ISODate) );
	m_dateTimeMenu->addAction( time.toString(Qt::TextDate) );
	m_dateTimeMenu->addAction( time.toString(Qt::SystemLocaleShortDate) );
	m_dateTimeMenu->addAction( time.toString(Qt::SystemLocaleLongDate) );

	m_dateTimeMenu->exec( mapToGlobal(ui.tbDateTime->rect().bottomLeft()));
}

void LabelWidget::insertDateTime(QAction* action) {
	ui.teLabel->insertPlainText( action->text().remove('&') );
}

// geometry slots

/*!
    called when label's current horizontal position relative to its parent (left, center, right, custom ) is changed.
*/
void LabelWidget::positionXChanged(int index) {
	//Enable/disable the spinbox for the x- oordinates if the "custom position"-item is selected/deselected
	if (index == ui.cbPositionX->count()-1 )
		ui.sbPositionX->setEnabled(true);
	else
		ui.sbPositionX->setEnabled(false);

	if (m_initializing)
		return;

	TextLabel::PositionWrapper position = m_label->position();
	position.horizontalPosition = TextLabel::HorizontalPosition(index);
	for (auto* label : m_labelsList)
		label->setPosition(position);
}

/*!
    called when label's current horizontal position relative to its parent (top, center, bottom, custom ) is changed.
*/
void LabelWidget::positionYChanged(int index) {
	//Enable/disable the spinbox for the y-coordinates if the "custom position"-item is selected/deselected
	if (index == ui.cbPositionY->count()-1 )
		ui.sbPositionY->setEnabled(true);
	else
		ui.sbPositionY->setEnabled(false);

	if (m_initializing)
		return;

	TextLabel::PositionWrapper position = m_label->position();
	position.verticalPosition = TextLabel::VerticalPosition(index);
	for (auto* label : m_labelsList)
		label->setPosition(position);
}

void LabelWidget::customPositionXChanged(double value) {
	if (m_initializing)
		return;

	TextLabel::PositionWrapper position = m_label->position();
	position.point.setX(Worksheet::convertToSceneUnits(value, Worksheet::Centimeter));
	for (auto* label : m_labelsList)
		label->setPosition(position);
}

void LabelWidget::customPositionYChanged(double value) {
	if (m_initializing)
		return;

	TextLabel::PositionWrapper position = m_label->position();
	position.point.setY(Worksheet::convertToSceneUnits(value, Worksheet::Centimeter));
	for (auto* label : m_labelsList)
		label->setPosition(position);
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
		axis->setTitleOffsetX( Worksheet::convertToSceneUnits(value, Worksheet::Point) );
}

void LabelWidget::offsetYChanged(double value) {
	if (m_initializing)
		return;

	for (auto* axis : m_axesList)
		axis->setTitleOffsetY( Worksheet::convertToSceneUnits(value, Worksheet::Point) );
}

void LabelWidget::visibilityChanged(bool state) {
	if (m_initializing)
		return;

	for (auto* label : m_labelsList)
		label->setVisible(state);
}

//*********************************************************
//****** SLOTs for changes triggered in TextLabel *********
//*********************************************************
void LabelWidget::labelTextWrapperChanged(const TextLabel::TextWrapper& text) {
	m_initializing = true;

	//save and restore the current cursor position after changing the text
	QTextCursor cursor = ui.teLabel->textCursor();
	int position = cursor.position();
	if (text.teXUsed)
		ui.teLabel->setText(text.text);
	else
		ui.teLabel->setHtml(text.text);
	cursor.movePosition(QTextCursor::Start);
	cursor.movePosition(QTextCursor::Right, QTextCursor::MoveAnchor, position);
	ui.teLabel->setTextCursor(cursor);

	ui.tbTexUsed->setChecked(text.teXUsed);
	this->teXUsedChanged(text.teXUsed);
	m_initializing = false;
}

/*!
 * \brief Highlights the text field red if wrong latex syntax was used (null image was produced)
 * or something else went wrong during rendering (\sa ExpressionTextEdit::validateExpression())
 */
void LabelWidget::labelTeXImageUpdated(bool valid) {
	if (!valid)
		ui.teLabel->setStyleSheet(QLatin1String("QTextEdit{background: red;}"));
	else
		ui.teLabel->setStyleSheet(QLatin1String(""));
}

void LabelWidget::labelTeXFontChanged(const QFont& font) {
	m_initializing = true;
	ui.kfontRequesterTeX->setFont(font);
	ui.sbFontSize->setValue(font.pointSize());
	m_initializing = false;
}

void LabelWidget::labelTeXFontColorChanged(const QColor color) {
	m_initializing = true;
	ui.kcbFontColor->setColor(color);
	m_initializing = false;
}

void LabelWidget::labelPositionChanged(const TextLabel::PositionWrapper& position) {
	m_initializing = true;
	ui.sbPositionX->setValue( Worksheet::convertFromSceneUnits(position.point.x(), Worksheet::Centimeter) );
	ui.sbPositionY->setValue( Worksheet::convertFromSceneUnits(position.point.y(), Worksheet::Centimeter) );
	ui.cbPositionX->setCurrentIndex( position.horizontalPosition );
	ui.cbPositionY->setCurrentIndex( position.verticalPosition );
	m_initializing = false;
}

void LabelWidget::labelHorizontalAlignmentChanged(TextLabel::HorizontalAlignment index) {
	m_initializing = true;
	ui.cbHorizontalAlignment->setCurrentIndex(index);
	m_initializing = false;
}

void LabelWidget::labelVerticalAlignmentChanged(TextLabel::VerticalAlignment index) {
	m_initializing = true;
	ui.cbVerticalAlignment->setCurrentIndex(index);
	m_initializing = false;
}

void LabelWidget::labelOffsetxChanged(qreal offset) {
	m_initializing = true;
	ui.sbOffsetX->setValue(Worksheet::convertFromSceneUnits(offset, Worksheet::Point));
	m_initializing = false;
}

void LabelWidget::labelOffsetyChanged(qreal offset) {
	m_initializing = true;
	ui.sbOffsetY->setValue(Worksheet::convertFromSceneUnits(offset, Worksheet::Point));
	m_initializing = false;
}

void LabelWidget::labelRotationAngleChanged(qreal angle) {
	m_initializing = true;
	ui.sbRotation->setValue(angle);
	m_initializing = false;
}

void LabelWidget::labelVisibleChanged(bool on) {
	m_initializing = true;
	ui.chbVisible->setChecked(on);
	m_initializing = false;
}

//**********************************************************
//******************** SETTINGS ****************************
//**********************************************************
void LabelWidget::load() {
	if(!m_label)
		return;

	m_initializing = true;

	ui.chbVisible->setChecked(m_label->isVisible());

	//Text/TeX
	ui.tbTexUsed->setChecked( (bool) m_label->text().teXUsed );
	if (m_label->text().teXUsed)
		ui.teLabel->setText(m_label->text().text);
	else
		ui.teLabel->setHtml(m_label->text().text);

	this->teXUsedChanged(m_label->text().teXUsed);
	ui.kfontRequesterTeX->setFont(m_label->teXFont());
	ui.sbFontSize->setValue( m_label->teXFont().pointSize() );

	//move the cursor to the end and set the focus to the text editor
	QTextCursor cursor = ui.teLabel->textCursor();
	cursor.movePosition(QTextCursor::End);
	ui.teLabel->setTextCursor(cursor);
	ui.teLabel->setFocus();

	// Geometry
	ui.cbPositionX->setCurrentIndex( (int) m_label->position().horizontalPosition );
	positionXChanged(ui.cbPositionX->currentIndex());
	ui.sbPositionX->setValue( Worksheet::convertFromSceneUnits(m_label->position().point.x(),Worksheet::Centimeter) );
	ui.cbPositionY->setCurrentIndex( (int) m_label->position().verticalPosition );
	positionYChanged(ui.cbPositionY->currentIndex());
	ui.sbPositionY->setValue( Worksheet::convertFromSceneUnits(m_label->position().point.y(),Worksheet::Centimeter) );

	if (!m_axesList.isEmpty()) {
		ui.sbOffsetX->setValue( Worksheet::convertFromSceneUnits(m_axesList.first()->titleOffsetX(), Worksheet::Point) );
		ui.sbOffsetY->setValue( Worksheet::convertFromSceneUnits(m_axesList.first()->titleOffsetY(), Worksheet::Point) );
	}
	ui.cbHorizontalAlignment->setCurrentIndex( (int) m_label->horizontalAlignment() );
	ui.cbVerticalAlignment->setCurrentIndex( (int) m_label->verticalAlignment() );
	ui.sbRotation->setValue( m_label->rotationAngle() );

	m_initializing = false;
}

void LabelWidget::loadConfig(KConfigGroup& group) {
	if(!m_label)
		return;

	m_initializing = true;

	//TeX
	ui.tbTexUsed->setChecked(group.readEntry("TeXUsed", (bool) m_label->text().teXUsed));
	this->teXUsedChanged(m_label->text().teXUsed);
	ui.sbFontSize->setValue( group.readEntry("TeXFontSize", m_label->teXFont().pointSize()) );
	ui.kfontRequesterTeX->setFont(group.readEntry("TeXFont", m_label->teXFont()));

	// Geometry
	ui.cbPositionX->setCurrentIndex( group.readEntry("PositionX", (int) m_label->position().horizontalPosition ) );
	ui.sbPositionX->setValue( Worksheet::convertFromSceneUnits(group.readEntry("PositionXValue", m_label->position().point.x()),Worksheet::Centimeter) );
	ui.cbPositionY->setCurrentIndex( group.readEntry("PositionY", (int) m_label->position().verticalPosition ) );
	ui.sbPositionY->setValue( Worksheet::convertFromSceneUnits(group.readEntry("PositionYValue", m_label->position().point.y()),Worksheet::Centimeter) );

	if (!m_axesList.isEmpty()) {
		ui.sbOffsetX->setValue( Worksheet::convertFromSceneUnits(group.readEntry("OffsetX", m_axesList.first()->titleOffsetX()), Worksheet::Point) );
		ui.sbOffsetY->setValue( Worksheet::convertFromSceneUnits(group.readEntry("OffsetY", m_axesList.first()->titleOffsetY()), Worksheet::Point) );
	}
	ui.cbHorizontalAlignment->setCurrentIndex( group.readEntry("HorizontalAlignment", (int) m_label->horizontalAlignment()) );
	ui.cbVerticalAlignment->setCurrentIndex( group.readEntry("VerticalAlignment", (int) m_label->verticalAlignment()) );
	ui.sbRotation->setValue( group.readEntry("Rotation", m_label->rotationAngle()) );

	m_initializing = false;
}

void LabelWidget::saveConfig(KConfigGroup& group) {
	//TeX
	group.writeEntry("TeXUsed", ui.tbTexUsed->isChecked());
	group.writeEntry("TeXFontColor", ui.kcbFontColor->color());
	group.writeEntry("TeXBackgroundColor", ui.kcbBackgroundColor->color());
	group.writeEntry("TeXFont", ui.kfontRequesterTeX->font());

	// Geometry
	group.writeEntry("PositionX", ui.cbPositionX->currentIndex());
	group.writeEntry("PositionXValue", Worksheet::convertToSceneUnits(ui.sbPositionX->value(),Worksheet::Centimeter) );
	group.writeEntry("PositionY", ui.cbPositionY->currentIndex());
	group.writeEntry("PositionYValue",  Worksheet::convertToSceneUnits(ui.sbPositionY->value(),Worksheet::Centimeter) );

	if (!m_axesList.isEmpty()) {
		group.writeEntry("OffsetX",  Worksheet::convertToSceneUnits(ui.sbOffsetX->value(), Worksheet::Point) );
		group.writeEntry("OffsetY",  Worksheet::convertToSceneUnits(ui.sbOffsetY->value(), Worksheet::Point) );
	}
	group.writeEntry("HorizontalAlignment", ui.cbHorizontalAlignment->currentIndex());
	group.writeEntry("VerticalAlignment", ui.cbVerticalAlignment->currentIndex());
	group.writeEntry("Rotation", ui.sbRotation->value());
}
