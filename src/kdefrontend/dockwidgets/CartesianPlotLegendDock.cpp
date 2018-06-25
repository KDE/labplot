/***************************************************************************
    File                 : CartesianPlotLegendDock.cpp
    Project              : LabPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2013-2016 by Alexander Semke (alexander.semke@web.de)
    Description          : widget for cartesian plot legend properties

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

#include "CartesianPlotLegendDock.h"
#include "backend/worksheet/plots/PlotArea.h"
#include "backend/worksheet/Worksheet.h"
#include "kdefrontend/widgets/LabelWidget.h"
#include "kdefrontend/GuiTools.h"
#include "kdefrontend/TemplateHandler.h"

#include <QCompleter>
#include <QDir>
#include <QDirModel>
#include <QFileDialog>
#include <QImageReader>

#include <KLocalizedString>
#include <KSharedConfig>

/*!
  \class CartesianPlotLegendDock
  \brief  Provides a widget for editing the properties of the cartesian plot legend currently selected in the project explorer.

  \ingroup kdefrontend
*/
CartesianPlotLegendDock::CartesianPlotLegendDock(QWidget* parent) : QWidget(parent),
	m_legend(nullptr),
	labelWidget(nullptr),
	m_initializing(false) {

	ui.setupUi(this);

	//"Title"-tab
	QHBoxLayout* hboxLayout = new QHBoxLayout(ui.tabTitle);
	labelWidget = new LabelWidget(ui.tabTitle);
	labelWidget->setNoGeometryMode(true);
	hboxLayout->addWidget(labelWidget);
	hboxLayout->setContentsMargins(2,2,2,2);
	hboxLayout->setSpacing(2);

	//"Background"-tab
	ui.bOpen->setIcon( QIcon::fromTheme("document-open") );
	QCompleter* completer = new QCompleter(this);
	completer->setModel(new QDirModel);
	ui.leBackgroundFileName->setCompleter(completer);

	//adjust layouts in the tabs
	for (int i=0; i<ui.tabWidget->count(); ++i) {
		QGridLayout* layout = dynamic_cast<QGridLayout*>(ui.tabWidget->widget(i)->layout());
		if (!layout)
			continue;

		layout->setContentsMargins(2,2,2,2);
		layout->setHorizontalSpacing(2);
		layout->setVerticalSpacing(2);
	}

	//SIGNAL/SLOT

	//General
	connect(ui.leName, &QLineEdit::textChanged, this, &CartesianPlotLegendDock::nameChanged);
	connect(ui.leComment, &QLineEdit::textChanged, this, &CartesianPlotLegendDock::commentChanged);
	connect( ui.chkVisible, SIGNAL(clicked(bool)), this, SLOT(visibilityChanged(bool)) );
	connect( ui.kfrLabelFont, SIGNAL(fontSelected(QFont)), this, SLOT(labelFontChanged(QFont)) );
	connect( ui.kcbLabelColor, SIGNAL(changed(QColor)), this, SLOT(labelColorChanged(QColor)) );
	connect( ui.cbOrder, SIGNAL(currentIndexChanged(int)), this, SLOT(labelOrderChanged(int)) );
	connect( ui.sbLineSymbolWidth, SIGNAL(valueChanged(double)), this, SLOT(lineSymbolWidthChanged(double)) );

	connect( ui.cbPositionX, SIGNAL(currentIndexChanged(int)), this, SLOT(positionXChanged(int)) );
	connect( ui.cbPositionY, SIGNAL(currentIndexChanged(int)), this, SLOT(positionYChanged(int)) );
	connect( ui.sbPositionX, SIGNAL(valueChanged(double)), this, SLOT(customPositionXChanged(double)) );
	connect( ui.sbPositionY, SIGNAL(valueChanged(double)), this, SLOT(customPositionYChanged(double)) );

	//Background
	connect( ui.cbBackgroundType, SIGNAL(currentIndexChanged(int)), this, SLOT(backgroundTypeChanged(int)) );
	connect( ui.cbBackgroundColorStyle, SIGNAL(currentIndexChanged(int)), this, SLOT(backgroundColorStyleChanged(int)) );
	connect( ui.cbBackgroundImageStyle, SIGNAL(currentIndexChanged(int)), this, SLOT(backgroundImageStyleChanged(int)) );
	connect( ui.cbBackgroundBrushStyle, SIGNAL(currentIndexChanged(int)), this, SLOT(backgroundBrushStyleChanged(int)) );
	connect( ui.bOpen, SIGNAL(clicked(bool)), this, SLOT(selectFile()) );
	connect( ui.leBackgroundFileName, SIGNAL(returnPressed()), this, SLOT(fileNameChanged()) );
	connect( ui.leBackgroundFileName, SIGNAL(textChanged(QString)), this, SLOT(fileNameChanged()) );
	connect( ui.kcbBackgroundFirstColor, SIGNAL(changed(QColor)), this, SLOT(backgroundFirstColorChanged(QColor)) );
	connect( ui.kcbBackgroundSecondColor, SIGNAL(changed(QColor)), this, SLOT(backgroundSecondColorChanged(QColor)) );
	connect( ui.sbBackgroundOpacity, SIGNAL(valueChanged(int)), this, SLOT(backgroundOpacityChanged(int)) );

	//Border
	connect( ui.cbBorderStyle, SIGNAL(currentIndexChanged(int)), this, SLOT(borderStyleChanged(int)) );
	connect( ui.kcbBorderColor, SIGNAL(changed(QColor)), this, SLOT(borderColorChanged(QColor)) );
	connect( ui.sbBorderWidth, SIGNAL(valueChanged(double)), this, SLOT(borderWidthChanged(double)) );
	connect( ui.sbBorderCornerRadius, SIGNAL(valueChanged(double)), this, SLOT(borderCornerRadiusChanged(double)) );
	connect( ui.sbBorderOpacity, SIGNAL(valueChanged(int)), this, SLOT(borderOpacityChanged(int)) );

	//Layout
	connect( ui.sbLayoutTopMargin, SIGNAL(valueChanged(double)), this, SLOT(layoutTopMarginChanged(double)) );
	connect( ui.sbLayoutBottomMargin, SIGNAL(valueChanged(double)), this, SLOT(layoutBottomMarginChanged(double)) );
	connect( ui.sbLayoutLeftMargin, SIGNAL(valueChanged(double)), this, SLOT(layoutLeftMarginChanged(double)) );
	connect( ui.sbLayoutRightMargin, SIGNAL(valueChanged(double)), this, SLOT(layoutRightMarginChanged(double)) );
	connect( ui.sbLayoutHorizontalSpacing, SIGNAL(valueChanged(double)), this, SLOT(layoutHorizontalSpacingChanged(double)) );
	connect( ui.sbLayoutVerticalSpacing, SIGNAL(valueChanged(double)), this, SLOT(layoutVerticalSpacingChanged(double)) );
	connect( ui.sbLayoutColumnCount, SIGNAL(valueChanged(int)), this, SLOT(layoutColumnCountChanged(int)) );

	TemplateHandler* templateHandler = new TemplateHandler(this, TemplateHandler::CartesianPlotLegend);
	ui.verticalLayout->addWidget(templateHandler);
	templateHandler->show();
	connect(templateHandler, SIGNAL(loadConfigRequested(KConfig&)), this, SLOT(loadConfigFromTemplate(KConfig&)));
	connect(templateHandler, SIGNAL(saveConfigRequested(KConfig&)), this, SLOT(saveConfigAsTemplate(KConfig&)));
	connect(templateHandler, SIGNAL(info(QString)), this, SIGNAL(info(QString)));

	init();
}

void CartesianPlotLegendDock::init() {
	this->retranslateUi();
}

void CartesianPlotLegendDock::setLegends(QList<CartesianPlotLegend*> list) {
	m_initializing = true;
	m_legendList = list;

	m_legend=list.first();

	//if there is more then one legend in the list, disable the tab "general"
	if (list.size()==1) {
		ui.lName->setEnabled(true);
		ui.leName->setEnabled(true);
		ui.lComment->setEnabled(true);
		ui.leComment->setEnabled(true);

		ui.leName->setText(m_legend->name());
		ui.leComment->setText(m_legend->comment());
	}else{
		ui.lName->setEnabled(false);
		ui.leName->setEnabled(false);
		ui.lComment->setEnabled(false);
		ui.leComment->setEnabled(false);

		ui.leName->setText("");
		ui.leComment->setText("");
	}

	//show the properties of the first curve
	this->load();

	//on the very first start the column count shown in UI is 1.
	//if the this count for m_legend is also 1 then the slot layoutColumnCountChanged is not called
	//and we need to disable the "order" widgets here.
	ui.lOrder->setVisible(m_legend->layoutColumnCount()!=1);
	ui.cbOrder->setVisible(m_legend->layoutColumnCount()!=1);

	//legend title
	QList<TextLabel*> labels;
	for (auto* legend : list)
		labels.append(legend->title());

	labelWidget->setLabels(labels);

	//update active widgets
	backgroundTypeChanged(ui.cbBackgroundType->currentIndex());

	//SIGNALs/SLOTs
	//General
	connect( m_legend, SIGNAL(aspectDescriptionChanged(const AbstractAspect*)), this, SLOT(legendDescriptionChanged(const AbstractAspect*)) );
	connect( m_legend, SIGNAL(labelFontChanged(QFont&)), this, SLOT(legendLabelFontChanged(QFont&)) );
	connect( m_legend, SIGNAL(labelColorChanged(QColor&)), this, SLOT(legendLabelColorChanged(QColor&)) );
	connect( m_legend, SIGNAL(labelColumnMajorChanged(bool)), this, SLOT(legendLabelOrderChanged(bool)) );
	connect( m_legend, SIGNAL(positionChanged(CartesianPlotLegend::PositionWrapper)),
			 this, SLOT(legendPositionChanged(CartesianPlotLegend::PositionWrapper)) );
	connect( m_legend, SIGNAL(lineSymbolWidthChanged(float)), this, SLOT(legendLineSymbolWidthChanged(float)) );
	connect(m_legend, SIGNAL(visibilityChanged(bool)), this, SLOT(legendVisibilityChanged(bool)));

	//background
	connect( m_legend, SIGNAL(backgroundTypeChanged(PlotArea::BackgroundType)), this, SLOT(legendBackgroundTypeChanged(PlotArea::BackgroundType)) );
	connect( m_legend, SIGNAL(backgroundColorStyleChanged(PlotArea::BackgroundColorStyle)), this, SLOT(legendBackgroundColorStyleChanged(PlotArea::BackgroundColorStyle)) );
	connect( m_legend, SIGNAL(backgroundImageStyleChanged(PlotArea::BackgroundImageStyle)), this, SLOT(legendBackgroundImageStyleChanged(PlotArea::BackgroundImageStyle)) );
	connect( m_legend, SIGNAL(backgroundBrushStyleChanged(Qt::BrushStyle)), this, SLOT(legendBackgroundBrushStyleChanged(Qt::BrushStyle)) );
	connect( m_legend, SIGNAL(backgroundFirstColorChanged(QColor&)), this, SLOT(legendBackgroundFirstColorChanged(QColor&)) );
	connect( m_legend, SIGNAL(backgroundSecondColorChanged(QColor&)), this, SLOT(legendBackgroundSecondColorChanged(QColor&)) );
	connect( m_legend, SIGNAL(backgroundFileNameChanged(QString&)), this, SLOT(legendBackgroundFileNameChanged(QString&)) );
	connect( m_legend, SIGNAL(backgroundOpacityChanged(float)), this, SLOT(legendBackgroundOpacityChanged(float)) );
	connect( m_legend, SIGNAL(borderPenChanged(QPen&)), this, SLOT(legendBorderPenChanged(QPen&)) );
	connect( m_legend, SIGNAL(borderCornerRadiusChanged(float)), this, SLOT(legendBorderCornerRadiusChanged(float)) );
	connect( m_legend, SIGNAL(borderOpacityChanged(float)), this, SLOT(legendBorderOpacityChanged(float)) );

	//layout
	connect(m_legend,SIGNAL(layoutTopMarginChanged(float)),this,SLOT(legendLayoutTopMarginChanged(float)));
	connect(m_legend,SIGNAL(layoutBottomMarginChanged(float)),this,SLOT(legendLayoutBottomMarginChanged(float)));
	connect(m_legend,SIGNAL(layoutLeftMarginChanged(float)),this,SLOT(legendLayoutLeftMarginChanged(float)));
	connect(m_legend,SIGNAL(layoutRightMarginChanged(float)),this,SLOT(legendLayoutRightMarginChanged(float)));
	connect(m_legend,SIGNAL(layoutVerticalSpacingChanged(float)),this,SLOT(legendLayoutVerticalSpacingChanged(float)));
	connect(m_legend,SIGNAL(layoutHorizontalSpacingChanged(float)),this,SLOT(legendLayoutHorizontalSpacingChanged(float)));
	connect(m_legend,SIGNAL(layoutColumnCountChanged(int)),this,SLOT(legendLayoutColumnCountChanged(int)));

	m_initializing = false;
}

void CartesianPlotLegendDock::activateTitleTab() const{
	ui.tabWidget->setCurrentWidget(ui.tabTitle);
}

//************************************************************
//** SLOTs for changes triggered in CartesianPlotLegendDock **
//************************************************************
void CartesianPlotLegendDock::retranslateUi() {
	m_initializing = true;

	ui.cbBackgroundType->addItem(i18n("Color"));
	ui.cbBackgroundType->addItem(i18n("Image"));
	ui.cbBackgroundType->addItem(i18n("Pattern"));

	ui.cbBackgroundColorStyle->addItem(i18n("Single Color"));
	ui.cbBackgroundColorStyle->addItem(i18n("Horizontal Gradient"));
	ui.cbBackgroundColorStyle->addItem(i18n("Vertical Gradient"));
	ui.cbBackgroundColorStyle->addItem(i18n("Diag. Gradient (From Top Left)"));
	ui.cbBackgroundColorStyle->addItem(i18n("Diag. Gradient (From Bottom Left)"));
	ui.cbBackgroundColorStyle->addItem(i18n("Radial Gradient"));

	ui.cbBackgroundImageStyle->addItem(i18n("Scaled and Cropped"));
	ui.cbBackgroundImageStyle->addItem(i18n("Scaled"));
	ui.cbBackgroundImageStyle->addItem(i18n("Scaled, Keep Proportions"));
	ui.cbBackgroundImageStyle->addItem(i18n("Centered"));
	ui.cbBackgroundImageStyle->addItem(i18n("Tiled"));
	ui.cbBackgroundImageStyle->addItem(i18n("Center Tiled"));

	ui.cbOrder->addItem(i18n("Column Major"));
	ui.cbOrder->addItem(i18n("Row Major"));

	ui.cbPositionX->addItem(i18n("Left"));
	ui.cbPositionX->addItem(i18n("Center"));
	ui.cbPositionX->addItem(i18n("Right"));
	ui.cbPositionX->addItem(i18n("Custom"));
	ui.cbPositionY->addItem(i18n("Top"));
	ui.cbPositionY->addItem(i18n("Center"));
	ui.cbPositionY->addItem(i18n("Bottom"));
	ui.cbPositionY->addItem(i18n("Custom"));

	GuiTools::updatePenStyles(ui.cbBorderStyle, Qt::black);
	GuiTools::updateBrushStyles(ui.cbBackgroundBrushStyle, Qt::SolidPattern);

	m_initializing = false;
}

// "General"-tab
void CartesianPlotLegendDock::nameChanged() {
	if (m_initializing)
		return;

	m_legend->setName(ui.leName->text());
}

void CartesianPlotLegendDock::commentChanged() {
	if (m_initializing)
		return;

	m_legend->setComment(ui.leComment->text());
}

void CartesianPlotLegendDock::visibilityChanged(bool state) {
	if (m_initializing)
		return;

	for (auto* legend : m_legendList)
		legend->setVisible(state);
}

//General
void CartesianPlotLegendDock::labelFontChanged(const QFont& font) {
	if (m_initializing)
		return;

	QFont labelsFont = font;
	labelsFont.setPixelSize( Worksheet::convertToSceneUnits(font.pointSizeF(), Worksheet::Point) );
	for (auto* legend : m_legendList)
		legend->setLabelFont(labelsFont);
}

void CartesianPlotLegendDock::labelColorChanged(const QColor& color) {
	if (m_initializing)
		return;

	for (auto* legend : m_legendList)
		legend->setLabelColor(color);
}

void CartesianPlotLegendDock::labelOrderChanged(const int index) {
	if (m_initializing)
		return;

	bool columnMajor = (index==0);
	for (auto* legend : m_legendList)
		legend->setLabelColumnMajor(columnMajor);
}

void CartesianPlotLegendDock::lineSymbolWidthChanged(double value) {
	if (m_initializing)
		return;

	for (auto* legend : m_legendList)
		legend->setLineSymbolWidth(Worksheet::convertToSceneUnits(value, Worksheet::Centimeter));
}

/*!
	called when legend's current horizontal position relative to its parent (left, center, right, custom ) is changed.
*/
void CartesianPlotLegendDock::positionXChanged(int index) {
	//Enable/disable the spinbox for the x- oordinates if the "custom position"-item is selected/deselected
	if (index == ui.cbPositionX->count()-1 ) {
		ui.sbPositionX->setEnabled(true);
	}else{
		ui.sbPositionX->setEnabled(false);
	}

	if (m_initializing)
		return;

	CartesianPlotLegend::PositionWrapper position = m_legend->position();
	position.horizontalPosition = CartesianPlotLegend::HorizontalPosition(index);
	for (auto* legend : m_legendList)
		legend->setPosition(position);
}

/*!
	called when legend's current horizontal position relative to its parent (top, center, bottom, custom ) is changed.
*/
void CartesianPlotLegendDock::positionYChanged(int index) {
	//Enable/disable the spinbox for the y- oordinates if the "custom position"-item is selected/deselected
	if (index == ui.cbPositionY->count()-1 ) {
		ui.sbPositionY->setEnabled(true);
	}else{
		ui.sbPositionY->setEnabled(false);
	}

	if (m_initializing)
		return;

	CartesianPlotLegend::PositionWrapper position = m_legend->position();
	position.verticalPosition = CartesianPlotLegend::VerticalPosition(index);
	for (auto* legend : m_legendList)
		legend->setPosition(position);
}

void CartesianPlotLegendDock::customPositionXChanged(double value) {
	if (m_initializing)
		return;

	CartesianPlotLegend::PositionWrapper position = m_legend->position();
	position.point.setX(Worksheet::convertToSceneUnits(value, Worksheet::Centimeter));
	for (auto* legend : m_legendList)
		legend->setPosition(position);
}

void CartesianPlotLegendDock::customPositionYChanged(double value) {
	if (m_initializing)
		return;

	CartesianPlotLegend::PositionWrapper position = m_legend->position();
	position.point.setY(Worksheet::convertToSceneUnits(value, Worksheet::Centimeter));
	for (auto* legend : m_legendList)
		legend->setPosition(position);
}

// "Background"-tab
void CartesianPlotLegendDock::backgroundTypeChanged(int index) {
	PlotArea::BackgroundType type = (PlotArea::BackgroundType)index;

	if (type == PlotArea::Color) {
		ui.lBackgroundColorStyle->show();
		ui.cbBackgroundColorStyle->show();
		ui.lBackgroundImageStyle->hide();
		ui.cbBackgroundImageStyle->hide();
		ui.lBackgroundBrushStyle->hide();
		ui.cbBackgroundBrushStyle->hide();

		ui.lBackgroundFileName->hide();
		ui.leBackgroundFileName->hide();
		ui.bOpen->hide();

		ui.lBackgroundFirstColor->show();
		ui.kcbBackgroundFirstColor->show();

		PlotArea::BackgroundColorStyle style =
			(PlotArea::BackgroundColorStyle) ui.cbBackgroundColorStyle->currentIndex();
		if (style == PlotArea::SingleColor) {
			ui.lBackgroundFirstColor->setText(i18n("Color"));
			ui.lBackgroundSecondColor->hide();
			ui.kcbBackgroundSecondColor->hide();
		}else{
			ui.lBackgroundFirstColor->setText(i18n("First color"));
			ui.lBackgroundSecondColor->show();
			ui.kcbBackgroundSecondColor->show();
		}
	}else if (type == PlotArea::Image) {
		ui.lBackgroundColorStyle->hide();
		ui.cbBackgroundColorStyle->hide();
		ui.lBackgroundImageStyle->show();
		ui.cbBackgroundImageStyle->show();
		ui.lBackgroundBrushStyle->hide();
		ui.cbBackgroundBrushStyle->hide();
		ui.lBackgroundFileName->show();
		ui.leBackgroundFileName->show();
		ui.bOpen->show();

		ui.lBackgroundFirstColor->hide();
		ui.kcbBackgroundFirstColor->hide();
		ui.lBackgroundSecondColor->hide();
		ui.kcbBackgroundSecondColor->hide();
	}else if (type == PlotArea::Pattern) {
		ui.lBackgroundFirstColor->setText(i18n("Color"));
		ui.lBackgroundColorStyle->hide();
		ui.cbBackgroundColorStyle->hide();
		ui.lBackgroundImageStyle->hide();
		ui.cbBackgroundImageStyle->hide();
		ui.lBackgroundBrushStyle->show();
		ui.cbBackgroundBrushStyle->show();
		ui.lBackgroundFileName->hide();
		ui.leBackgroundFileName->hide();
		ui.bOpen->hide();

		ui.lBackgroundFirstColor->show();
		ui.kcbBackgroundFirstColor->show();
		ui.lBackgroundSecondColor->hide();
		ui.kcbBackgroundSecondColor->hide();
	}

	if (m_initializing)
		return;

	for (auto* legend : m_legendList)
		legend->setBackgroundType(type);
}

void CartesianPlotLegendDock::backgroundColorStyleChanged(int index) {
	PlotArea::BackgroundColorStyle style = (PlotArea::BackgroundColorStyle)index;

	if (style == PlotArea::SingleColor) {
		ui.lBackgroundFirstColor->setText(i18n("Color"));
		ui.lBackgroundSecondColor->hide();
		ui.kcbBackgroundSecondColor->hide();
	}else{
		ui.lBackgroundFirstColor->setText(i18n("First color"));
		ui.lBackgroundSecondColor->show();
		ui.kcbBackgroundSecondColor->show();
		ui.lBackgroundBrushStyle->hide();
		ui.cbBackgroundBrushStyle->hide();
	}

	if (m_initializing)
		return;

	for (auto* legend : m_legendList)
		legend->setBackgroundColorStyle(style);
}

void CartesianPlotLegendDock::backgroundImageStyleChanged(int index) {
	if (m_initializing)
		return;

	PlotArea::BackgroundImageStyle style = (PlotArea::BackgroundImageStyle)index;
	for (auto* legend : m_legendList)
		legend->setBackgroundImageStyle(style);
}

void CartesianPlotLegendDock::backgroundBrushStyleChanged(int index) {
	if (m_initializing)
		return;

	Qt::BrushStyle style = (Qt::BrushStyle)index;
	for (auto* legend : m_legendList)
		legend->setBackgroundBrushStyle(style);
}

void CartesianPlotLegendDock::backgroundFirstColorChanged(const QColor& c) {
	if (m_initializing)
		return;

	for (auto* legend : m_legendList)
		legend->setBackgroundFirstColor(c);
}

void CartesianPlotLegendDock::backgroundSecondColorChanged(const QColor& c) {
	if (m_initializing)
		return;

	for (auto* legend : m_legendList)
		legend->setBackgroundSecondColor(c);
}

/*!
	opens a file dialog and lets the user select the image file.
*/
void CartesianPlotLegendDock::selectFile() {
	KConfigGroup conf(KSharedConfig::openConfig(), "CartesianPlotLegendDock");
	QString dir = conf.readEntry("LastImageDir", "");

	QString formats;
	for (const QByteArray& format : QImageReader::supportedImageFormats()) {
		QString f = "*." + QString(format.constData());
		formats.isEmpty() ? formats+=f : formats+=' '+f;
	}

	QString path = QFileDialog::getOpenFileName(this, i18n("Select the image file"), dir, i18n("Images (%1)", formats));
	if (path.isEmpty())
		return; //cancel was clicked in the file-dialog

	int pos = path.lastIndexOf(QDir::separator());
	if (pos != -1) {
		QString newDir = path.left(pos);
		if (newDir != dir)
			conf.writeEntry("LastImageDir", newDir);
	}

    ui.leBackgroundFileName->setText( path );

	for (auto* legend : m_legendList)
		legend->setBackgroundFileName(path);
}

void CartesianPlotLegendDock::fileNameChanged() {
	if (m_initializing)
		return;

	QString fileName = ui.leBackgroundFileName->text();
	if (!fileName.isEmpty() && !QFile::exists(fileName))
		ui.leBackgroundFileName->setStyleSheet("QLineEdit{background:red;}");
	else
		ui.leBackgroundFileName->setStyleSheet("");

	for (auto* legend : m_legendList)
		legend->setBackgroundFileName(fileName);
}

void CartesianPlotLegendDock::backgroundOpacityChanged(int value) {
	if (m_initializing)
		return;

	float opacity = (float)value/100.;
	for (auto* legend : m_legendList)
		legend->setBackgroundOpacity(opacity);
}

// "Border"-tab
void CartesianPlotLegendDock::borderStyleChanged(int index) {
	if (m_initializing)
		return;

	Qt::PenStyle penStyle=Qt::PenStyle(index);
	QPen pen;
	for (auto* legend : m_legendList) {
		pen = legend->borderPen();
		pen.setStyle(penStyle);
		legend->setBorderPen(pen);
	}
}

void CartesianPlotLegendDock::borderColorChanged(const QColor& color) {
	if (m_initializing)
		return;

	QPen pen;
	for (auto* legend : m_legendList) {
		pen = legend->borderPen();
		pen.setColor(color);
		legend->setBorderPen(pen);
	}

	m_initializing = true;
	GuiTools::updatePenStyles(ui.cbBorderStyle, color);
	m_initializing = false;
}

void CartesianPlotLegendDock::borderWidthChanged(double value) {
	if (m_initializing)
		return;

	QPen pen;
	for (auto* legend : m_legendList) {
		pen = legend->borderPen();
		pen.setWidthF( Worksheet::convertToSceneUnits(value, Worksheet::Point) );
		legend->setBorderPen(pen);
	}
}

void CartesianPlotLegendDock::borderCornerRadiusChanged(double value) {
	if (m_initializing)
		return;

	for (auto* legend : m_legendList)
		legend->setBorderCornerRadius(Worksheet::convertToSceneUnits(value, Worksheet::Centimeter));
}

void CartesianPlotLegendDock::borderOpacityChanged(int value) {
	if (m_initializing)
		return;

	float opacity = (float)value/100.;
	for (auto* legend : m_legendList)
		legend->setBorderOpacity(opacity);
}

//Layout
void CartesianPlotLegendDock::layoutTopMarginChanged(double margin) {
	if (m_initializing)
		return;

	for (auto* legend : m_legendList)
		legend->setLayoutTopMargin(Worksheet::convertToSceneUnits(margin, Worksheet::Centimeter));
}

void CartesianPlotLegendDock::layoutBottomMarginChanged(double margin) {
	if (m_initializing)
		return;

	for (auto* legend : m_legendList)
		legend->setLayoutBottomMargin(Worksheet::convertToSceneUnits(margin, Worksheet::Centimeter));
}

void CartesianPlotLegendDock::layoutLeftMarginChanged(double margin) {
	if (m_initializing)
		return;

	for (auto* legend : m_legendList)
		legend->setLayoutLeftMargin(Worksheet::convertToSceneUnits(margin, Worksheet::Centimeter));
}

void CartesianPlotLegendDock::layoutRightMarginChanged(double margin) {
	if (m_initializing)
		return;

	for (auto* legend : m_legendList)
		legend->setLayoutRightMargin(Worksheet::convertToSceneUnits(margin, Worksheet::Centimeter));
}

void CartesianPlotLegendDock::layoutHorizontalSpacingChanged(double spacing) {
	if (m_initializing)
		return;

	for (auto* legend : m_legendList)
		legend->setLayoutHorizontalSpacing(Worksheet::convertToSceneUnits(spacing, Worksheet::Centimeter));
}

void CartesianPlotLegendDock::layoutVerticalSpacingChanged(double spacing) {
	if (m_initializing)
		return;

	for (auto* legend : m_legendList)
		legend->setLayoutVerticalSpacing(Worksheet::convertToSceneUnits(spacing, Worksheet::Centimeter));
}

void CartesianPlotLegendDock::layoutColumnCountChanged(int count) {
	ui.lOrder->setVisible(count!=1);
	ui.cbOrder->setVisible(count!=1);

	if (m_initializing)
		return;

	for (auto* legend : m_legendList)
		legend->setLayoutColumnCount(count);
}

//*************************************************************
//**** SLOTs for changes triggered in CartesianPlotLegend *****
//*************************************************************
//General
void CartesianPlotLegendDock::legendDescriptionChanged(const AbstractAspect* aspect) {
	if (m_legend != aspect)
		return;

	m_initializing = true;
	if (aspect->name() != ui.leName->text()) {
		ui.leName->setText(aspect->name());
	} else if (aspect->comment() != ui.leComment->text()) {
		ui.leComment->setText(aspect->comment());
	}
	m_initializing = false;
}

void CartesianPlotLegendDock::legendLabelFontChanged(QFont& font) {
	m_initializing = true;
	//we need to set the font size in points for KFontRequester
	QFont f(font);
	f.setPointSizeF( Worksheet::convertFromSceneUnits(f.pixelSize(), Worksheet::Point) );
	ui.kfrLabelFont->setFont(f);
	m_initializing = false;
}

void CartesianPlotLegendDock::legendLabelColorChanged(QColor& color) {
	m_initializing = true;
	ui.kcbLabelColor->setColor(color);
	m_initializing = false;
}

void CartesianPlotLegendDock::legendLabelOrderChanged(bool b) {
	m_initializing = true;
	if (b)
		ui.cbOrder->setCurrentIndex(0); //column major
	else
		ui.cbOrder->setCurrentIndex(1); //row major
	m_initializing = false;
}

void CartesianPlotLegendDock::legendLineSymbolWidthChanged(float value) {
	m_initializing = true;
	ui.sbLineSymbolWidth->setValue(Worksheet::convertFromSceneUnits(value, Worksheet::Centimeter));
	m_initializing = false;
}


void CartesianPlotLegendDock::legendPositionChanged(const CartesianPlotLegend::PositionWrapper& position) {
	m_initializing = true;
	ui.sbPositionX->setValue( Worksheet::convertFromSceneUnits(position.point.x(), Worksheet::Centimeter) );
	ui.sbPositionY->setValue( Worksheet::convertFromSceneUnits(position.point.y(), Worksheet::Centimeter) );
	ui.cbPositionX->setCurrentIndex( position.horizontalPosition );
	ui.cbPositionY->setCurrentIndex( position.verticalPosition );
	m_initializing = false;
}

void CartesianPlotLegendDock::legendVisibilityChanged(bool on) {
	m_initializing = true;
	ui.chkVisible->setChecked(on);
	m_initializing = false;
}

//Background
void CartesianPlotLegendDock::legendBackgroundTypeChanged(PlotArea::BackgroundType type) {
	m_initializing = true;
	ui.cbBackgroundType->setCurrentIndex(type);
	m_initializing = false;
}

void CartesianPlotLegendDock::legendBackgroundColorStyleChanged(PlotArea::BackgroundColorStyle style) {
	m_initializing = true;
	ui.cbBackgroundColorStyle->setCurrentIndex(style);
	m_initializing = false;
}

void CartesianPlotLegendDock::legendBackgroundImageStyleChanged(PlotArea::BackgroundImageStyle style) {
	m_initializing = true;
	ui.cbBackgroundImageStyle->setCurrentIndex(style);
	m_initializing = false;
}

void CartesianPlotLegendDock::legendBackgroundBrushStyleChanged(Qt::BrushStyle style) {
	m_initializing = true;
	ui.cbBackgroundBrushStyle->setCurrentIndex(style);
	m_initializing = false;
}

void CartesianPlotLegendDock::legendBackgroundFirstColorChanged(QColor& color) {
	m_initializing = true;
	ui.kcbBackgroundFirstColor->setColor(color);
	m_initializing = false;
}

void CartesianPlotLegendDock::legendBackgroundSecondColorChanged(QColor& color) {
	m_initializing = true;
	ui.kcbBackgroundSecondColor->setColor(color);
	m_initializing = false;
}

void CartesianPlotLegendDock::legendBackgroundFileNameChanged(QString& filename) {
	m_initializing = true;
	ui.leBackgroundFileName->setText(filename);
	m_initializing = false;
}

void CartesianPlotLegendDock::legendBackgroundOpacityChanged(float opacity) {
	m_initializing = true;
	ui.sbBackgroundOpacity->setValue( qRound(opacity*100.0) );
	m_initializing = false;
}

//Border
void CartesianPlotLegendDock::legendBorderPenChanged(QPen& pen) {
	if (m_initializing)
		return;

	m_initializing = true;
	if (ui.cbBorderStyle->currentIndex() != pen.style())
		ui.cbBorderStyle->setCurrentIndex(pen.style());
	if (ui.kcbBorderColor->color() != pen.color())
		ui.kcbBorderColor->setColor(pen.color());
	if (ui.sbBorderWidth->value() != pen.widthF())
		ui.sbBorderWidth->setValue(Worksheet::convertFromSceneUnits(pen.widthF(),Worksheet::Point));
	m_initializing = false;
}

void CartesianPlotLegendDock::legendBorderCornerRadiusChanged(float value) {
	m_initializing = true;
	ui.sbBorderCornerRadius->setValue(Worksheet::convertFromSceneUnits(value, Worksheet::Centimeter));
	m_initializing = false;
}

void CartesianPlotLegendDock::legendBorderOpacityChanged(float opacity) {
	m_initializing = true;
	ui.sbBorderOpacity->setValue( qRound(opacity*100.0) );
	m_initializing = false;
}

//Layout
void CartesianPlotLegendDock::legendLayoutTopMarginChanged(float value) {
	m_initializing = true;
	ui.sbLayoutTopMargin->setValue(Worksheet::convertFromSceneUnits(value, Worksheet::Centimeter));
	m_initializing = false;
}

void CartesianPlotLegendDock::legendLayoutBottomMarginChanged(float value) {
	m_initializing = true;
	ui.sbLayoutBottomMargin->setValue(Worksheet::convertFromSceneUnits(value, Worksheet::Centimeter));
	m_initializing = false;
}

void CartesianPlotLegendDock::legendLayoutLeftMarginChanged(float value) {
	m_initializing = true;
	ui.sbLayoutLeftMargin->setValue(Worksheet::convertFromSceneUnits(value, Worksheet::Centimeter));
	m_initializing = false;
}

void CartesianPlotLegendDock::legendLayoutRightMarginChanged(float value) {
	m_initializing = true;
	ui.sbLayoutRightMargin->setValue(Worksheet::convertFromSceneUnits(value, Worksheet::Centimeter));
	m_initializing = false;
}

void CartesianPlotLegendDock::legendLayoutVerticalSpacingChanged(float value) {
	m_initializing = true;
	ui.sbLayoutVerticalSpacing->setValue(Worksheet::convertFromSceneUnits(value, Worksheet::Centimeter));
	m_initializing = false;
}

void CartesianPlotLegendDock::legendLayoutHorizontalSpacingChanged(float value) {
	m_initializing = true;
	ui.sbLayoutHorizontalSpacing->setValue(Worksheet::convertFromSceneUnits(value, Worksheet::Centimeter));
	m_initializing = false;
}

void CartesianPlotLegendDock::legendLayoutColumnCountChanged(int value) {
	m_initializing = true;
	ui.sbLayoutColumnCount->setValue(value);
	m_initializing = false;
}

//*************************************************************
//******************** SETTINGS *******************************
//*************************************************************
void CartesianPlotLegendDock::load() {
	//General-tab
	ui.chkVisible->setChecked( m_legend->isVisible() );

	//we need to set the font size in points for KFontRequester
	QFont font = m_legend->labelFont();
	font.setPointSizeF( qRound(Worksheet::convertFromSceneUnits(font.pixelSize(), Worksheet::Point)) );
// 	qDebug()<<"font size " << font.pixelSize() << "  " << font.pointSizeF();
	ui.kfrLabelFont->setFont(font);

	ui.kcbLabelColor->setColor( m_legend->labelColor() );
	bool columnMajor = m_legend->labelColumnMajor();
	if (columnMajor)
		ui.cbOrder->setCurrentIndex(0); //column major
	else
		ui.cbOrder->setCurrentIndex(1); //row major

	ui.sbLineSymbolWidth->setValue( Worksheet::convertFromSceneUnits(m_legend->lineSymbolWidth(), Worksheet::Centimeter) );

	//Background-tab
	ui.cbBackgroundType->setCurrentIndex( (int) m_legend->backgroundType() );
	ui.cbBackgroundColorStyle->setCurrentIndex( (int) m_legend->backgroundColorStyle() );
	ui.cbBackgroundImageStyle->setCurrentIndex( (int) m_legend->backgroundImageStyle() );
	ui.cbBackgroundBrushStyle->setCurrentIndex( (int) m_legend->backgroundBrushStyle() );
	ui.leBackgroundFileName->setText( m_legend->backgroundFileName() );
	ui.kcbBackgroundFirstColor->setColor( m_legend->backgroundFirstColor() );
	ui.kcbBackgroundSecondColor->setColor( m_legend->backgroundSecondColor() );
	ui.sbBackgroundOpacity->setValue( qRound(m_legend->backgroundOpacity()*100.0) );

	//highlight the text field for the background image red if an image is used and cannot be found
	if (!m_legend->backgroundFileName().isEmpty() && !QFile::exists(m_legend->backgroundFileName()))
		ui.leBackgroundFileName->setStyleSheet("QLineEdit{background:red;}");
	else
		ui.leBackgroundFileName->setStyleSheet("");

	//Border
	ui.kcbBorderColor->setColor( m_legend->borderPen().color() );
	ui.cbBorderStyle->setCurrentIndex( (int) m_legend->borderPen().style() );
	ui.sbBorderWidth->setValue( Worksheet::convertFromSceneUnits(m_legend->borderPen().widthF(), Worksheet::Point) );
	ui.sbBorderCornerRadius->setValue( Worksheet::convertFromSceneUnits(m_legend->borderCornerRadius(), Worksheet::Centimeter) );
	ui.sbBorderOpacity->setValue( qRound(m_legend->borderOpacity()*100.0) );

	// Layout
	ui.sbLayoutTopMargin->setValue( Worksheet::convertFromSceneUnits(m_legend->layoutTopMargin(), Worksheet::Centimeter) );
	ui.sbLayoutBottomMargin->setValue( Worksheet::convertFromSceneUnits(m_legend->layoutBottomMargin(), Worksheet::Centimeter) );
	ui.sbLayoutLeftMargin->setValue( Worksheet::convertFromSceneUnits(m_legend->layoutLeftMargin(), Worksheet::Centimeter) );
	ui.sbLayoutRightMargin->setValue( Worksheet::convertFromSceneUnits(m_legend->layoutRightMargin(), Worksheet::Centimeter) );
	ui.sbLayoutHorizontalSpacing->setValue( Worksheet::convertFromSceneUnits(m_legend->layoutHorizontalSpacing(), Worksheet::Centimeter) );
	ui.sbLayoutVerticalSpacing->setValue( Worksheet::convertFromSceneUnits(m_legend->layoutVerticalSpacing(), Worksheet::Centimeter) );

	ui.sbLayoutColumnCount->setValue( m_legend->layoutColumnCount() );

	m_initializing=true;
	GuiTools::updatePenStyles(ui.cbBorderStyle, ui.kcbBorderColor->color());
	m_initializing=false;
}

void CartesianPlotLegendDock::loadConfigFromTemplate(KConfig& config) {
	//extract the name of the template from the file name
	QString name;
	int index = config.name().lastIndexOf(QDir::separator());
	if (index!=-1)
		name = config.name().right(config.name().size() - index - 1);
	else
		name = config.name();

	int size = m_legendList.size();
	if (size>1)
		m_legend->beginMacro(i18n("%1 cartesian plot legends: template \"%2\" loaded", size, name));
	else
		m_legend->beginMacro(i18n("%1: template \"%2\" loaded", m_legend->name(), name));

	this->loadConfig(config);

	m_legend->endMacro();
}

void CartesianPlotLegendDock::loadConfig(KConfig& config) {
	KConfigGroup group = config.group( "CartesianPlotLegend" );

	//General-tab
	ui.chkVisible->setChecked( group.readEntry("Visible", m_legend->isVisible()) );

	//we need to set the font size in points for KFontRequester
	QFont font = m_legend->labelFont();
	font.setPointSizeF( qRound(Worksheet::convertFromSceneUnits(font.pixelSize(), Worksheet::Point)) );
	ui.kfrLabelFont->setFont( group.readEntry("LabelFont", font) );

	ui.kcbLabelColor->setColor( group.readEntry("LabelColor", m_legend->labelColor()) );

	bool columnMajor = group.readEntry("LabelColumMajor", m_legend->labelColumnMajor());
	if (columnMajor)
		ui.cbOrder->setCurrentIndex(0); //column major
	else
		ui.cbOrder->setCurrentIndex(1); //row major

	ui.sbLineSymbolWidth->setValue(group.readEntry("LineSymbolWidth",
													Worksheet::convertFromSceneUnits(m_legend->lineSymbolWidth(), Worksheet::Centimeter)) );

	//Background-tab
	ui.cbBackgroundType->setCurrentIndex( group.readEntry("BackgroundType", (int) m_legend->backgroundType()) );
	ui.cbBackgroundColorStyle->setCurrentIndex( group.readEntry("BackgroundColorStyle", (int) m_legend->backgroundColorStyle()) );
	ui.cbBackgroundImageStyle->setCurrentIndex( group.readEntry("BackgroundImageStyle", (int) m_legend->backgroundImageStyle()) );
	ui.cbBackgroundBrushStyle->setCurrentIndex( group.readEntry("BackgroundBrushStyle", (int) m_legend->backgroundBrushStyle()) );
	ui.leBackgroundFileName->setText( group.readEntry("BackgroundFileName", m_legend->backgroundFileName()) );
	ui.kcbBackgroundFirstColor->setColor( group.readEntry("BackgroundFirstColor", m_legend->backgroundFirstColor()) );
	ui.kcbBackgroundSecondColor->setColor( group.readEntry("BackgroundSecondColor", m_legend->backgroundSecondColor()) );
	ui.sbBackgroundOpacity->setValue( qRound(group.readEntry("BackgroundOpacity", m_legend->backgroundOpacity())*100.0) );

	//Border
	ui.kcbBorderColor->setColor( group.readEntry("BorderColor", m_legend->borderPen().color()) );
	ui.cbBorderStyle->setCurrentIndex( group.readEntry("BorderStyle", (int) m_legend->borderPen().style()) );
	ui.sbBorderWidth->setValue( Worksheet::convertFromSceneUnits(group.readEntry("BorderWidth", m_legend->borderPen().widthF()), Worksheet::Point) );
	ui.sbBorderCornerRadius->setValue( Worksheet::convertFromSceneUnits(group.readEntry("BorderCornerRadius", m_legend->borderCornerRadius()), Worksheet::Centimeter) );
	ui.sbBorderOpacity->setValue( qRound(group.readEntry("BorderOpacity", m_legend->borderOpacity())*100.0) );

	// Layout
	ui.sbLayoutTopMargin->setValue(group.readEntry("LayoutTopMargin",
												   Worksheet::convertFromSceneUnits(m_legend->layoutTopMargin(), Worksheet::Centimeter)) );
	ui.sbLayoutBottomMargin->setValue(group.readEntry("LayoutBottomMargin",
													   Worksheet::convertFromSceneUnits(m_legend->layoutBottomMargin(), Worksheet::Centimeter)) );
	ui.sbLayoutLeftMargin->setValue(group.readEntry("LayoutLeftMargin",
													 Worksheet::convertFromSceneUnits(m_legend->layoutLeftMargin(), Worksheet::Centimeter)) );
	ui.sbLayoutRightMargin->setValue(group.readEntry("LayoutRightMargin",
													  Worksheet::convertFromSceneUnits(m_legend->layoutRightMargin(), Worksheet::Centimeter)) );
	ui.sbLayoutHorizontalSpacing->setValue(group.readEntry("LayoutHorizontalSpacing",
														    Worksheet::convertFromSceneUnits(m_legend->layoutHorizontalSpacing(), Worksheet::Centimeter)) );
	ui.sbLayoutVerticalSpacing->setValue(group.readEntry("LayoutVerticalSpacing",
														  Worksheet::convertFromSceneUnits(m_legend->layoutVerticalSpacing(), Worksheet::Centimeter)) );
	ui.sbLayoutColumnCount->setValue(group.readEntry("LayoutColumnCount", m_legend->layoutColumnCount()));

	//Title
	group = config.group("PlotLegend");
	labelWidget->loadConfig(group);

	m_initializing=true;
	GuiTools::updatePenStyles(ui.cbBorderStyle, ui.kcbBorderColor->color());
	m_initializing=false;
}

void CartesianPlotLegendDock::saveConfigAsTemplate(KConfig& config) {
	KConfigGroup group = config.group( "CartesianPlotLegend" );

	//General-tab
	group.writeEntry("Visible", ui.chkVisible->isChecked());
	QFont font = m_legend->labelFont();
	font.setPointSizeF( Worksheet::convertFromSceneUnits(font.pointSizeF(), Worksheet::Point) );
	group.writeEntry("LabelFont", font);
	group.writeEntry("LabelColor", ui.kcbLabelColor->color());
	group.writeEntry("LabelColumMajorOrder", ui.cbOrder->currentIndex()==0);// true for "column major", false for "row major"
	group.writeEntry("LineSymbolWidth", Worksheet::convertToSceneUnits(ui.sbLineSymbolWidth->value(), Worksheet::Centimeter));

	//Background
	group.writeEntry("BackgroundType", ui.cbBackgroundType->currentIndex());
	group.writeEntry("BackgroundColorStyle", ui.cbBackgroundColorStyle->currentIndex());
	group.writeEntry("BackgroundImageStyle", ui.cbBackgroundImageStyle->currentIndex());
	group.writeEntry("BackgroundBrushStyle", ui.cbBackgroundBrushStyle->currentIndex());
	group.writeEntry("BackgroundFileName", ui.leBackgroundFileName->text());
	group.writeEntry("BackgroundFirstColor", ui.kcbBackgroundFirstColor->color());
	group.writeEntry("BackgroundSecondColor", ui.kcbBackgroundSecondColor->color());
	group.writeEntry("BackgroundOpacity", ui.sbBackgroundOpacity->value()/100.0);

	//Border
	group.writeEntry("BorderStyle", ui.cbBorderStyle->currentIndex());
	group.writeEntry("BorderColor", ui.kcbBorderColor->color());
	group.writeEntry("BorderWidth", Worksheet::convertToSceneUnits(ui.sbBorderWidth->value(), Worksheet::Point));
	group.writeEntry("BorderCornerRadius", Worksheet::convertToSceneUnits(ui.sbBorderCornerRadius->value(), Worksheet::Centimeter));
	group.writeEntry("BorderOpacity", ui.sbBorderOpacity->value()/100.0);

	//Layout
	group.writeEntry("LayoutTopMargin",Worksheet::convertToSceneUnits(ui.sbLayoutTopMargin->value(), Worksheet::Centimeter));
	group.writeEntry("LayoutBottomMargin",Worksheet::convertToSceneUnits(ui.sbLayoutBottomMargin->value(), Worksheet::Centimeter));
	group.writeEntry("LayoutLeftMargin",Worksheet::convertToSceneUnits(ui.sbLayoutLeftMargin->value(), Worksheet::Centimeter));
	group.writeEntry("LayoutRightMargin",Worksheet::convertToSceneUnits(ui.sbLayoutRightMargin->value(), Worksheet::Centimeter));
	group.writeEntry("LayoutVerticalSpacing",Worksheet::convertToSceneUnits(ui.sbLayoutVerticalSpacing->value(), Worksheet::Centimeter));
	group.writeEntry("LayoutHorizontalSpacing",Worksheet::convertToSceneUnits(ui.sbLayoutHorizontalSpacing->value(), Worksheet::Centimeter));
	group.writeEntry("LayoutColumnCount", ui.sbLayoutColumnCount->value());

	//Title
	group = config.group("PlotLegend");
	labelWidget->saveConfig(group);

	config.sync();
}
