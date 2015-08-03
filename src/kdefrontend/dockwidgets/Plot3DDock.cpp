/***************************************************************************
    File                 : Plot3DDock.h
    Project              : LabPlot
    Description          : widget for 3D plot properties
    --------------------------------------------------------------------
    Copyright            : (C) 2015 Minh Ngo (minh@fedoraproject.org)

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

#include "Plot3DDock.h"
#include "DockHelpers.h"
#include "backend/worksheet/plots/3d/Plot3D.h"
#include "backend/core/AbstractAspect.h"
#include "backend/core/AbstractColumn.h"
#include "backend/core/AspectTreeModel.h"
#include "backend/core/Project.h"
#include "commonfrontend/widgets/TreeViewComboBox.h"
#include "kdefrontend/GuiTools.h"
#include "kdefrontend/TemplateHandler.h"

#include <QDebug>
#include <QLabel>
#include <QFileDialog>
#include <QGridLayout>
#include <QSpacerItem>

#include <KComboBox>
#include <KUrl>
#include <KUrlCompletion>
#include <KUrlRequester>
#include <KLocalizedString>

#include <math.h>

using namespace DockHelpers;

Plot3DDock::Plot3DDock(QWidget* parent)
	: QWidget(parent)
	, m_initializing(false) {
	ui.setupUi(this);

	this->retranslateUi();

	//Background-tab
	ui.cbBackgroundColorStyle->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLengthWithIcon);
	ui.kleBackgroundFileName->setClearButtonShown(true);
	ui.bBackgroundOpen->setIcon( KIcon("document-open") );

	KUrlCompletion *comp = new KUrlCompletion();
	ui.kleBackgroundFileName->setCompletionObject(comp);

	GuiTools::updateBrushStyles(ui.cbBackgroundBrushStyle, Qt::SolidPattern);

	//adjust layouts in the tabs
	for (int i=0; i<ui.tabWidget->count(); ++i){
		QGridLayout* layout = dynamic_cast<QGridLayout*>(ui.tabWidget->widget(i)->layout());
		if (!layout)
			continue;

		layout->setContentsMargins(2,2,2,2);
		layout->setHorizontalSpacing(2);
		layout->setVerticalSpacing(2);
	}

	//SIGNALs/SLOTs
	//General
	connect( ui.leName, SIGNAL(returnPressed()), this, SLOT(nameChanged()) );
	connect( ui.leComment, SIGNAL(returnPressed()), this, SLOT(commentChanged()) );
	connect( ui.chkVisible, SIGNAL(stateChanged(int)), this, SLOT(visibilityChanged(int)) );
	connect( ui.sbLeft, SIGNAL(valueChanged(double)), this, SLOT(geometryChanged()) );
	connect( ui.sbTop, SIGNAL(valueChanged(double)), this, SLOT(geometryChanged()) );
	connect( ui.sbWidth, SIGNAL(valueChanged(double)), this, SLOT(geometryChanged()) );
	connect( ui.sbHeight, SIGNAL(valueChanged(double)), this, SLOT(geometryChanged()) );

	//Background
	connect( ui.cbBackgroundType, SIGNAL(currentIndexChanged(int)), this, SLOT(backgroundTypeChanged(int)) );
	connect( ui.cbBackgroundColorStyle, SIGNAL(currentIndexChanged(int)), this, SLOT(backgroundColorStyleChanged(int)) );
	connect( ui.cbBackgroundImageStyle, SIGNAL(currentIndexChanged(int)), this, SLOT(backgroundImageStyleChanged(int)) );
	connect( ui.cbBackgroundBrushStyle, SIGNAL(currentIndexChanged(int)), this, SLOT(backgroundBrushStyleChanged(int)) );
	connect( ui.bBackgroundOpen, SIGNAL(clicked(bool)), this, SLOT(backgroundSelectFile()));
	connect( ui.kleBackgroundFileName, SIGNAL(returnPressed()), this, SLOT(backgroundFileNameChanged()) );
	connect( ui.kleBackgroundFileName, SIGNAL(clearButtonClicked()), this, SLOT(backgroundFileNameChanged()) );
	connect( ui.kcbBackgroundFirstColor, SIGNAL(changed(QColor)), this, SLOT(backgroundFirstColorChanged(QColor)) );
	connect( ui.kcbBackgroundSecondColor, SIGNAL(changed(QColor)), this, SLOT(backgroundSecondColorChanged(QColor)) );
	connect( ui.sbBackgroundOpacity, SIGNAL(valueChanged(int)), this, SLOT(backgroundOpacityChanged(int)) );

	//Template handler
	TemplateHandler* templateHandler = new TemplateHandler(this, TemplateHandler::Worksheet);
	ui.verticalLayout->addWidget(templateHandler, 0, 0);
	templateHandler->show();
	connect(templateHandler, SIGNAL(loadConfigRequested(KConfig&)), this, SLOT(loadConfigFromTemplate(KConfig&)));
	connect(templateHandler, SIGNAL(saveConfigRequested(KConfig&)), this, SLOT(saveConfigAsTemplate(KConfig&)));

	// TODO: Uncomment later
	//connect(templateHandler, SIGNAL(info(QString)), this, SIGNAL(info(QString)));
}

void Plot3DDock::setPlots(const QList<Plot3D*>& plots){
	Lock lock(m_initializing);
	m_plotsList = plots;
	Q_ASSERT(m_plotsList.size());
	m_plot = m_plotsList.first();

	//if there is more then one plot in the list, disable the name and comment fields in the tab "general"
	if (m_plotsList.size()==1){
		ui.lName->setEnabled(true);
		ui.leName->setEnabled(true);
		ui.lComment->setEnabled(true);
		ui.leComment->setEnabled(true);

		ui.leName->setText(m_plot->name());
		ui.leComment->setText(m_plot->comment());
	}else{
		ui.lName->setEnabled(false);
		ui.leName->setEnabled(false);
		ui.lComment->setEnabled(false);
		ui.leComment->setEnabled(false);

		ui.leName->setText("");
		ui.leComment->setText("");
	}

	//show the properties of the first plot
	this->load();

	//update active widgets
	backgroundTypeChanged(ui.cbBackgroundType->currentIndex());

	//Deactivate the geometry related widgets, if the worksheet layout is active.
	//Currently, a plot can only be a child of the worksheet itself, so we only need to ask the parent aspect (=worksheet).
	//TODO redesign this, if the hierarchy will be changend in future (a plot is a child of a new object group/container or so)
	Worksheet* w = dynamic_cast<Worksheet*>(m_plot->parentAspect());
	if (w){
		bool b = (w->layout()==Worksheet::NoLayout);
		ui.sbTop->setEnabled(b);
		ui.sbLeft->setEnabled(b);
		ui.sbWidth->setEnabled(b);
		ui.sbHeight->setEnabled(b);
		connect(w, SIGNAL(layoutChanged(Worksheet::Layout)), this, SLOT(layoutChanged(Worksheet::Layout)));
	}

	//SIGNALs/SLOTs
	//general
	connect(m_plot, SIGNAL(aspectDescriptionChanged(const AbstractAspect*)), SLOT(plotDescriptionChanged(const AbstractAspect*)));

	//background
	connect(m_plot, SIGNAL(backgroundTypeChanged(PlotArea::BackgroundType)), SLOT(plotBackgroundTypeChanged(PlotArea::BackgroundType)));
	connect(m_plot, SIGNAL(backgroundColorStyleChanged(PlotArea::BackgroundColorStyle)), SLOT(plotBackgroundColorStyleChanged(PlotArea::BackgroundColorStyle)));
	connect(m_plot, SIGNAL(backgroundImageStyleChanged(PlotArea::BackgroundImageStyle)), SLOT(plotBackgroundImageStyleChanged(PlotArea::BackgroundImageStyle)));
	connect(m_plot, SIGNAL(backgroundBrushStyleChanged(Qt::BrushStyle)), SLOT(plotBackgroundBrushStyleChanged(Qt::BrushStyle)));
	connect(m_plot, SIGNAL(backgroundFirstColorChanged(QColor)), SLOT(plotBackgroundFirstColorChanged(QColor)));
	connect(m_plot, SIGNAL(backgroundSecondColorChanged(QColor)), SLOT(plotBackgroundSecondColorChanged(QColor)));
	connect(m_plot, SIGNAL(backgroundFileNameChanged(QString)), SLOT(plotBackgroundFileNameChanged(QString)));
	connect(m_plot, SIGNAL(backgroundOpacityChanged(float)), SLOT(plotBackgroundOpacityChanged(float)));

	//light
}

//*************************************************************
//****** SLOTs for changes triggered in Plot3DDock *********
//*************************************************************
void Plot3DDock::retranslateUi(){
	Lock lock(m_initializing);

	//general
	ui.cbXScaling->addItem( i18n("linear") );
	ui.cbXScaling->addItem( i18n("log(x)") );
	ui.cbXScaling->addItem( i18n("log2(x)") );
	ui.cbXScaling->addItem( i18n("ln(x)") );

	ui.cbYScaling->addItem( i18n("linear") );
	ui.cbYScaling->addItem( i18n("log(y)") );
	ui.cbYScaling->addItem( i18n("log2(y)") );
	ui.cbYScaling->addItem( i18n("ln(y)") );

	ui.cbZScaling->addItem( i18n("linear") );
	ui.cbZScaling->addItem( i18n("log(y)") );
	ui.cbZScaling->addItem( i18n("log2(y)") );
	ui.cbZScaling->addItem( i18n("ln(y)") );

	//background
	ui.cbBackgroundType->addItem(i18n("color"));
	ui.cbBackgroundType->addItem(i18n("image"));
	ui.cbBackgroundType->addItem(i18n("pattern"));

	ui.cbBackgroundColorStyle->addItem(i18n("single color"));
	ui.cbBackgroundColorStyle->addItem(i18n("horizontal linear gradient"));
	ui.cbBackgroundColorStyle->addItem(i18n("vertical linear gradient"));
	ui.cbBackgroundColorStyle->addItem(i18n("diagonal linear gradient (start from top left)"));
	ui.cbBackgroundColorStyle->addItem(i18n("diagonal linear gradient (start from bottom left)"));
	ui.cbBackgroundColorStyle->addItem(i18n("radial gradient"));

	ui.cbBackgroundImageStyle->addItem(i18n("scaled and cropped"));
	ui.cbBackgroundImageStyle->addItem(i18n("scaled"));
	ui.cbBackgroundImageStyle->addItem(i18n("scaled, keep proportions"));
	ui.cbBackgroundImageStyle->addItem(i18n("centered"));
	ui.cbBackgroundImageStyle->addItem(i18n("tiled"));
	ui.cbBackgroundImageStyle->addItem(i18n("center tiled"));

	//light
}

// "General"-tab
void Plot3DDock::nameChanged(){
	if (m_initializing)
		return;

	m_plot->setName(ui.leName->text());
}

void Plot3DDock::commentChanged(){
	if (m_initializing)
		return;

	m_plot->setComment(ui.leComment->text());
}

void Plot3DDock::visibilityChanged(int state){
	if (m_initializing)
		return;

	bool b = (state==Qt::Checked);
	foreach(Plot3D* plot, m_plotsList)
		plot->setVisible(b);
}

void Plot3DDock::geometryChanged(){
	if (m_initializing)
		return;

	float x = Worksheet::convertToSceneUnits(ui.sbLeft->value(), Worksheet::Centimeter);
	float y = Worksheet::convertToSceneUnits(ui.sbTop->value(), Worksheet::Centimeter);
	float w = Worksheet::convertToSceneUnits(ui.sbWidth->value(), Worksheet::Centimeter);
	float h = Worksheet::convertToSceneUnits(ui.sbHeight->value(), Worksheet::Centimeter);

	QRectF rect(x,y,w,h);
	m_plot->setRect(rect);
}

/*!
	Called when the layout in the worksheet gets changed.
	Enables/disables the geometry widgets if the layout was deactivated/activated.
	Shows the new geometry values of the first plot if the layout was activated.
 */
void Plot3DDock::layoutChanged(Worksheet::Layout layout){
	bool b = (layout == Worksheet::NoLayout);
	ui.sbTop->setEnabled(b);
	ui.sbLeft->setEnabled(b);
	ui.sbWidth->setEnabled(b);
	ui.sbHeight->setEnabled(b);
	if (!b){
		Lock lock(m_initializing);
		ui.sbLeft->setValue(Worksheet::convertFromSceneUnits(m_plot->rect().x(), Worksheet::Centimeter));
		ui.sbTop->setValue(Worksheet::convertFromSceneUnits(m_plot->rect().y(), Worksheet::Centimeter));
		ui.sbWidth->setValue(Worksheet::convertFromSceneUnits(m_plot->rect().width(), Worksheet::Centimeter));
		ui.sbHeight->setValue(Worksheet::convertFromSceneUnits(m_plot->rect().height(), Worksheet::Centimeter));
	}
}

// "Background"-tab
void Plot3DDock::backgroundTypeChanged(int index){
	qDebug()<<"Plot3DDock::backgroundTypeChanged " << index;
	PlotArea::BackgroundType type = (PlotArea::BackgroundType)index;

	if (type == PlotArea::Color){
		showItem(ui.lBackgroundColorStyle, ui.cbBackgroundColorStyle);
		hideItem(ui.lBackgroundImageStyle, ui.cbBackgroundImageStyle);
		hideItem(ui.lBackgroundBrushStyle, ui.cbBackgroundBrushStyle);
		hideItem(ui.lBackgroundFileName, ui.kleBackgroundFileName);
		ui.bBackgroundOpen->hide();

		showItem(ui.lBackgroundFirstColor, ui.kcbBackgroundFirstColor);

		PlotArea::BackgroundColorStyle style =
			(PlotArea::BackgroundColorStyle) ui.cbBackgroundColorStyle->currentIndex();
		if (style == PlotArea::SingleColor){
			ui.lBackgroundFirstColor->setText(i18n("Color"));
			hideItem(ui.lBackgroundSecondColor, ui.kcbBackgroundSecondColor);
		}else{
			ui.lBackgroundFirstColor->setText(i18n("First Color"));
			showItem(ui.lBackgroundSecondColor, ui.kcbBackgroundSecondColor);
		}
	}else if(type == PlotArea::Image){
		hideItem(ui.lBackgroundFirstColor, ui.kcbBackgroundFirstColor);
		hideItem(ui.lBackgroundSecondColor, ui.kcbBackgroundSecondColor);
		hideItem(ui.lBackgroundColorStyle, ui.cbBackgroundColorStyle);
		showItem(ui.lBackgroundImageStyle, ui.cbBackgroundImageStyle);
		hideItem(ui.lBackgroundBrushStyle, ui.cbBackgroundBrushStyle);
		showItem(ui.lBackgroundFileName, ui.kleBackgroundFileName);
		ui.bBackgroundOpen->show();
	}else if(type == PlotArea::Pattern){
		ui.lBackgroundFirstColor->setText(i18n("Color"));
		showItem(ui.lBackgroundFirstColor, ui.kcbBackgroundFirstColor);
		hideItem(ui.lBackgroundSecondColor, ui.kcbBackgroundSecondColor);

		hideItem(ui.lBackgroundColorStyle, ui.cbBackgroundColorStyle);
		hideItem(ui.lBackgroundImageStyle, ui.cbBackgroundImageStyle);
		showItem(ui.lBackgroundBrushStyle, ui.cbBackgroundBrushStyle);
		hideItem(ui.lBackgroundFileName, ui.kleBackgroundFileName);
		ui.bBackgroundOpen->hide();
	}

	if (m_initializing)
		return;

	foreach(Plot3D* plot, m_plotsList)
		plot->setBackgroundType(type);
}

void Plot3DDock::backgroundColorStyleChanged(int index){
	PlotArea::BackgroundColorStyle style = (PlotArea::BackgroundColorStyle)index;

	if (style == PlotArea::SingleColor){
		ui.lBackgroundFirstColor->setText(i18n("Color"));
		hideItem(ui.lBackgroundSecondColor, ui.kcbBackgroundSecondColor);
	}else{
		ui.lBackgroundFirstColor->setText(i18n("First Color"));
		showItem(ui.lBackgroundSecondColor, ui.kcbBackgroundSecondColor);
	}

	if (m_initializing)
		return;

	foreach(Plot3D* plot, m_plotsList)
		plot->setBackgroundColorStyle(style);
}

void Plot3DDock::backgroundImageStyleChanged(int index){
	if (m_initializing)
		return;

	PlotArea::BackgroundImageStyle style = (PlotArea::BackgroundImageStyle)index;
	foreach(Plot3D* plot, m_plotsList)
		plot->setBackgroundImageStyle(style);
}

void Plot3DDock::backgroundBrushStyleChanged(int index){
	if (m_initializing)
		return;

	Qt::BrushStyle style = (Qt::BrushStyle)index;
	foreach(Plot3D* plot, m_plotsList){
		plot->setBackgroundBrushStyle(style);
	}
}

void Plot3DDock::backgroundFirstColorChanged(const QColor& c){
	if (m_initializing)
		return;

	foreach(Plot3D* plot, m_plotsList){
		plot->setBackgroundFirstColor(c);
	}
}

void Plot3DDock::backgroundSecondColorChanged(const QColor& c){
	if (m_initializing)
		return;

	foreach(Plot3D* plot, m_plotsList){
		plot->setBackgroundSecondColor(c);
	}
}

void Plot3DDock::backgroundOpacityChanged(int value){
	if (m_initializing)
		return;

	float opacity = (float)value/100;
	foreach(Plot3D* plot, m_plotsList)
		plot->setBackgroundOpacity(opacity);
}

void Plot3DDock::backgroundSelectFile() {
	KConfigGroup conf(KSharedConfig::openConfig(), "Plot3DDock");
	QString dir = conf.readEntry("LastImageDir", "");
	QString path = QFileDialog::getOpenFileName(this, i18n("Select the image file"), dir);
	if (path.isEmpty())
		return; //cancel was clicked in the file-dialog

	int pos = path.lastIndexOf(QDir::separator());
	if (pos!=-1) {
		QString newDir = path.left(pos);
		if (newDir!=dir)
			conf.writeEntry("LastImageDir", newDir);
	}

	ui.kleBackgroundFileName->setText( path );

	foreach(Plot3D* plot, m_plotsList)
		plot->setBackgroundFileName(path);
}

void Plot3DDock::backgroundFileNameChanged(){
	if (m_initializing)
		return;

	QString fileName = ui.kleBackgroundFileName->text();
	foreach(Plot3D* plot, m_plotsList)
		plot->setBackgroundFileName(fileName);
}


//*************************************************************
//******** SLOTs for changes triggered in Plot3D **************
//*************************************************************
// "General"-tab
void Plot3DDock::plotDescriptionChanged(const AbstractAspect* aspect) {
	if (m_plot != aspect)
		return;

	Lock lock(m_initializing);
	if (aspect->name() != ui.leName->text()) {
		ui.leName->setText(aspect->name());
	} else if (aspect->comment() != ui.leComment->text()) {
		ui.leComment->setText(aspect->comment());
	}
}


void Plot3DDock::plotRectChanged(QRectF& rect){
	Lock lock(m_initializing);
	ui.sbLeft->setValue(Worksheet::convertFromSceneUnits(rect.x(), Worksheet::Centimeter));
	ui.sbTop->setValue(Worksheet::convertFromSceneUnits(rect.y(), Worksheet::Centimeter));
	ui.sbWidth->setValue(Worksheet::convertFromSceneUnits(rect.width(), Worksheet::Centimeter));
	ui.sbHeight->setValue(Worksheet::convertFromSceneUnits(rect.height(), Worksheet::Centimeter));
}

void Plot3DDock::plotVisibleChanged(bool on){
	Lock lock(m_initializing);
	ui.chkVisible->setChecked(on);
}

// "Background"-tab
void Plot3DDock::plotBackgroundTypeChanged(PlotArea::BackgroundType type) {
	Lock lock(m_initializing);
	ui.cbBackgroundType->setCurrentIndex(type);
}

void Plot3DDock::plotBackgroundColorStyleChanged(PlotArea::BackgroundColorStyle style) {
	Lock lock(m_initializing);
	ui.cbBackgroundColorStyle->setCurrentIndex(style);
}

void Plot3DDock::plotBackgroundImageStyleChanged(PlotArea::BackgroundImageStyle style) {
	Lock lock(m_initializing);
	ui.cbBackgroundImageStyle->setCurrentIndex(style);
}

void Plot3DDock::plotBackgroundBrushStyleChanged(Qt::BrushStyle style) {
	Lock lock(m_initializing);
	ui.cbBackgroundBrushStyle->setCurrentIndex(style);
}

void Plot3DDock::plotBackgroundFirstColorChanged(const QColor& color) {
	Lock lock(m_initializing);
	ui.kcbBackgroundFirstColor->setColor(color);
}

void Plot3DDock::plotBackgroundSecondColorChanged(const QColor& color) {
	Lock lock(m_initializing);
	ui.kcbBackgroundSecondColor->setColor(color);
}

void Plot3DDock::plotBackgroundFileNameChanged(const QString& name) {
	Lock lock(m_initializing);
	ui.kleBackgroundFileName->setText(name);
}

void Plot3DDock::plotBackgroundOpacityChanged(float opacity) {
	Lock lock(m_initializing);
	ui.sbBackgroundOpacity->setValue( round(opacity*100.0) );
}


//*************************************************************
//******************** SETTINGS *******************************
//*************************************************************
void Plot3DDock::load(){
	//General
	ui.chkVisible->setChecked(m_plot->isVisible());
	ui.sbLeft->setValue(Worksheet::convertFromSceneUnits(m_plot->rect().x(), Worksheet::Centimeter));
	ui.sbTop->setValue(Worksheet::convertFromSceneUnits(m_plot->rect().y(), Worksheet::Centimeter));
	ui.sbWidth->setValue(Worksheet::convertFromSceneUnits(m_plot->rect().width(), Worksheet::Centimeter));
	ui.sbHeight->setValue(Worksheet::convertFromSceneUnits(m_plot->rect().height(), Worksheet::Centimeter));

	//TODO:
	//ui.cbType->setCurrentIndex((int)m_plot->visualizationType())

	// General
	ui.leName->setText(m_plot->name());
	ui.leComment->setText(m_plot->comment());

	//Background
	ui.cbBackgroundType->setCurrentIndex( (int) m_plot->backgroundType() );
	ui.cbBackgroundColorStyle->setCurrentIndex( (int) m_plot->backgroundColorStyle() );
	ui.cbBackgroundImageStyle->setCurrentIndex( (int) m_plot->backgroundImageStyle() );
	ui.cbBackgroundBrushStyle->setCurrentIndex( (int) m_plot->backgroundBrushStyle() );
	ui.kleBackgroundFileName->setText( m_plot->backgroundFileName() );
	ui.kcbBackgroundFirstColor->setColor( m_plot->backgroundFirstColor() );
	ui.kcbBackgroundSecondColor->setColor( m_plot->backgroundSecondColor() );
	ui.sbBackgroundOpacity->setValue( round(m_plot->backgroundOpacity()*100) );

	//Light
}

void Plot3DDock::loadConfigFromTemplate(KConfig& config) {
	//extract the name of the template from the file name
	QString name;
	int index = config.name().lastIndexOf(QDir::separator());
	if (index!=-1)
		name = config.name().right(config.name().size() - index - 1);
	else
		name = config.name();

	int size = m_plotsList.size();
	if (size>1)
		m_plot->beginMacro(i18n("%1 plots: template \"%2\" loaded", size, name));
	else
		m_plot->beginMacro(i18n("%1: template \"%2\" loaded", m_plot->name(), name));

	this->loadConfig(config);
	m_plot->endMacro();
}

void Plot3DDock::loadConfig(KConfig& config){
	KConfigGroup group = config.group("Plot3D");

	//General

	//Background
	ui.cbBackgroundType->setCurrentIndex( group.readEntry("BackgroundType", (int) m_plot->backgroundType()) );
	ui.cbBackgroundColorStyle->setCurrentIndex( group.readEntry("BackgroundColorStyle", (int) m_plot->backgroundColorStyle()) );
	ui.cbBackgroundImageStyle->setCurrentIndex( group.readEntry("BackgroundImageStyle", (int) m_plot->backgroundImageStyle()) );
	ui.cbBackgroundBrushStyle->setCurrentIndex( group.readEntry("BackgroundBrushStyle", (int) m_plot->backgroundBrushStyle()) );
	ui.kleBackgroundFileName->setText( group.readEntry("BackgroundFileName", m_plot->backgroundFileName()) );
	ui.kcbBackgroundFirstColor->setColor( group.readEntry("BackgroundFirstColor", m_plot->backgroundFirstColor()) );
	ui.kcbBackgroundSecondColor->setColor( group.readEntry("BackgroundSecondColor", m_plot->backgroundSecondColor()) );
	ui.sbBackgroundOpacity->setValue( round(group.readEntry("BackgroundOpacity", m_plot->backgroundOpacity())*100) );

	//Light
}

void Plot3DDock::saveConfigAsTemplate(KConfig& config) {
	KConfigGroup group = config.group("Plot3D");

	//General

	//Background
	group.writeEntry("BackgroundType",ui.cbBackgroundType->currentIndex());
	group.writeEntry("BackgroundColorStyle", ui.cbBackgroundColorStyle->currentIndex());
	group.writeEntry("BackgroundImageStyle", ui.cbBackgroundImageStyle->currentIndex());
	group.writeEntry("BackgroundBrushStyle", ui.cbBackgroundBrushStyle->currentIndex());
	group.writeEntry("BackgroundFileName", ui.kleBackgroundFileName->text());
	group.writeEntry("BackgroundFirstColor", ui.kcbBackgroundFirstColor->color());
	group.writeEntry("BackgroundSecondColor", ui.kcbBackgroundSecondColor->color());
	group.writeEntry("BackgroundOpacity", ui.sbBackgroundOpacity->value()/100.0);

	//Light

	config.sync();
}