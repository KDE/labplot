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
	, recorder(this)
	, m_plot(0)
	, m_initializing(false) {
	ui.setupUi(this);

	retranslateUi();

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
	recorder.connect(ui.leName, SIGNAL(returnPressed()), SLOT(onNameChanged()));
	recorder.connect(ui.leComment, SIGNAL(returnPressed()), SLOT(onCommentChanged()));
	recorder.connect(ui.chkVisible, SIGNAL(stateChanged(int)), SLOT(onVisibilityChanged(int)));
	recorder.connect(ui.sbLeft, SIGNAL(valueChanged(double)), SLOT(onGeometryChanged()));
	recorder.connect(ui.sbTop, SIGNAL(valueChanged(double)), SLOT(onGeometryChanged()));
	recorder.connect(ui.sbWidth, SIGNAL(valueChanged(double)), SLOT(onGeometryChanged()));
	recorder.connect(ui.sbHeight, SIGNAL(valueChanged(double)), SLOT(onGeometryChanged()));

	//Background
	recorder.connect(ui.cbBackgroundType, SIGNAL(currentIndexChanged(int)), SLOT(onBackgroundTypeChanged(int)));
	recorder.connect(ui.cbBackgroundColorStyle, SIGNAL(currentIndexChanged(int)), SLOT(onBackgroundColorStyleChanged(int)));
	recorder.connect(ui.cbBackgroundImageStyle, SIGNAL(currentIndexChanged(int)), SLOT(onBackgroundImageStyleChanged(int)));
	recorder.connect(ui.cbBackgroundBrushStyle, SIGNAL(currentIndexChanged(int)), SLOT(onBackgroundBrushStyleChanged(int)));
	recorder.connect(ui.bBackgroundOpen, SIGNAL(clicked(bool)), SLOT(onBackgroundSelectFile()));
	recorder.connect(ui.kleBackgroundFileName, SIGNAL(returnPressed()), SLOT(onBackgroundFileNameChanged()));
	recorder.connect(ui.kleBackgroundFileName, SIGNAL(clearButtonClicked()), SLOT(onBackgroundFileNameChanged()));
	recorder.connect(ui.kcbBackgroundFirstColor, SIGNAL(changed(const QColor&)), SLOT(onBackgroundFirstColorChanged(const QColor&)));
	recorder.connect(ui.kcbBackgroundSecondColor, SIGNAL(changed(const QColor&)), SLOT(onBackgroundSecondColorChanged(const QColor&)));
	recorder.connect(ui.sbBackgroundOpacity, SIGNAL(valueChanged(int)), SLOT(onBackgroundOpacityChanged(int)));

	// Light
	recorder.connect(ui.sbLightIntensity, SIGNAL(valueChanged(double)), SLOT(onIntensityChanged(double)));

	recorder.connect(ui.kcbLightAmbientColor, SIGNAL(changed(const QColor&)), SLOT(onAmbientChanged(const QColor&)));
	recorder.connect(ui.kcbLightDiffuseColor, SIGNAL(changed(const QColor&)), SLOT(onDiffuseChanged(const QColor&)));
	recorder.connect(ui.kcbLightSpecularColor, SIGNAL(changed(const QColor&)), SLOT(onSpecularChanged(const QColor&)));

	recorder.connect(ui.sbLightElevation, SIGNAL(valueChanged(int)), SLOT(onElevationChanged(int)));
	recorder.connect(ui.sbLightAzimuth, SIGNAL(valueChanged(int)), SLOT(onAzimuthChanged(int)));
	recorder.connect(ui.sbLightConeAngle, SIGNAL(valueChanged(int)), SLOT(onConeAngleChanged(int)));

	//Template handler
	TemplateHandler* templateHandler = new TemplateHandler(this, TemplateHandler::Worksheet);
	ui.verticalLayout->addWidget(templateHandler, 0, 0);
	templateHandler->show();
	recorder.connect(templateHandler, SIGNAL(loadConfigRequested(KConfig&)), SLOT(loadConfigFromTemplate(KConfig&)));
	recorder.connect(templateHandler, SIGNAL(saveConfigRequested(KConfig&)), SLOT(saveConfigAsTemplate(KConfig&)));

	// TODO: Uncomment later
	//connect(templateHandler, SIGNAL(info(QString)), this, SIGNAL(info(QString)));
}

void Plot3DDock::setPlots(const QList<Plot3D*>& plots){
	m_plotsList = plots;
	Q_ASSERT(m_plotsList.size());
	m_plot = m_plotsList.first();

	{
	const SignalBlocker blocker(recorder.children());
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
	}

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
		connect(w, SIGNAL(layoutChanged(Worksheet::Layout)), SLOT(onLayoutChanged(Worksheet::Layout)));
	}

	//SIGNALs/SLOTs
	//general
	connect(m_plot, SIGNAL(aspectDescriptionChanged(const AbstractAspect*)), SLOT(descriptionChanged(const AbstractAspect*)));

	//background
	connect(m_plot, SIGNAL(backgroundTypeChanged(PlotArea::BackgroundType)), SLOT(backgroundTypeChanged(PlotArea::BackgroundType)));
	connect(m_plot, SIGNAL(backgroundColorStyleChanged(PlotArea::BackgroundColorStyle)), SLOT(backgroundColorStyleChanged(PlotArea::BackgroundColorStyle)));
	connect(m_plot, SIGNAL(backgroundImageStyleChanged(PlotArea::BackgroundImageStyle)), SLOT(backgroundImageStyleChanged(PlotArea::BackgroundImageStyle)));
	connect(m_plot, SIGNAL(backgroundBrushStyleChanged(Qt::BrushStyle)), SLOT(backgroundBrushStyleChanged(Qt::BrushStyle)));
	connect(m_plot, SIGNAL(backgroundFirstColorChanged(const QColor&)), SLOT(backgroundFirstColorChanged(const QColor&)));
	connect(m_plot, SIGNAL(backgroundSecondColorChanged(const QColor&)), SLOT(backgroundSecondColorChanged(const QColor&)));
	connect(m_plot, SIGNAL(backgroundFileNameChanged(const QString&)), SLOT(backgroundFileNameChanged(const QString&)));
	connect(m_plot, SIGNAL(backgroundOpacityChanged(float)), SLOT(backgroundOpacityChanged(float)));

	//light
	connect(m_plot, SIGNAL(intensityChanged(double)), SLOT(intensityChanged(double)));

	connect(m_plot, SIGNAL(ambientChanged(const QColor&)), SLOT(ambientChanged(const QColor&)));
	connect(m_plot, SIGNAL(diffuseChanged(const QColor&)), SLOT(diffuseChanged(const QColor&)));
	connect(m_plot, SIGNAL(specularChanged(const QColor&)), SLOT(specularChanged(const QColor&)));

	connect(m_plot, SIGNAL(elevationChanged(double)), SLOT(elevationChanged(double)));
	connect(m_plot, SIGNAL(azimuthChanged(double)), SLOT(azimuthChanged(double)));
	connect(m_plot, SIGNAL(coneAngleChanged(double)), SLOT(coneAngleChanged(double)));
}

//*************************************************************
//****** SLOTs for changes triggered in Plot3DDock *********
//*************************************************************
void Plot3DDock::retranslateUi(){
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
void Plot3DDock::onNameChanged(){
	const Lock lock(m_initializing);
	m_plot->setName(ui.leName->text());
}

void Plot3DDock::onCommentChanged(){
	const Lock lock(m_initializing);
	m_plot->setComment(ui.leComment->text());
}

void Plot3DDock::onVisibilityChanged(int state){
	const Lock lock(m_initializing);

	const bool b = (state==Qt::Checked);
	foreach(Plot3D* plot, m_plotsList)
		plot->setVisible(b);
}

void Plot3DDock::onGeometryChanged(){
	const float x = Worksheet::convertToSceneUnits(ui.sbLeft->value(), Worksheet::Centimeter);
	const float y = Worksheet::convertToSceneUnits(ui.sbTop->value(), Worksheet::Centimeter);
	const float w = Worksheet::convertToSceneUnits(ui.sbWidth->value(), Worksheet::Centimeter);
	const float h = Worksheet::convertToSceneUnits(ui.sbHeight->value(), Worksheet::Centimeter);

	const QRectF rect(x,y,w,h);
	const Lock lock(m_initializing);
	m_plot->setRect(rect);
}

/*!
	Called when the layout in the worksheet gets changed.
	Enables/disables the geometry widgets if the layout was deactivated/activated.
	Shows the new geometry values of the first plot if the layout was activated.
 */
void Plot3DDock::onLayoutChanged(Worksheet::Layout layout){
	bool b = (layout == Worksheet::NoLayout);
	ui.sbTop->setEnabled(b);
	ui.sbLeft->setEnabled(b);
	ui.sbWidth->setEnabled(b);
	ui.sbHeight->setEnabled(b);
	if (!b)
		rectChanged(m_plot->rect());
}

// "Background"-tab
void Plot3DDock::onBackgroundTypeChanged(int index){
	PlotArea::BackgroundType type = (PlotArea::BackgroundType)index;

	const Lock lock(m_initializing);
	foreach(Plot3D* plot, m_plotsList)
		plot->setBackgroundType(type);

	backgroundTypeChanged(type);
}

void Plot3DDock::onBackgroundColorStyleChanged(int index){
	PlotArea::BackgroundColorStyle style = (PlotArea::BackgroundColorStyle)index;

	if (style == PlotArea::SingleColor){
		ui.lBackgroundFirstColor->setText(i18n("Color"));
		hideItem(ui.lBackgroundSecondColor, ui.kcbBackgroundSecondColor);
	}else{
		ui.lBackgroundFirstColor->setText(i18n("First Color"));
		showItem(ui.lBackgroundSecondColor, ui.kcbBackgroundSecondColor);
	}

	const Lock lock(m_initializing);
	foreach(Plot3D* plot, m_plotsList)
		plot->setBackgroundColorStyle(style);
}

void Plot3DDock::onBackgroundImageStyleChanged(int index){
	const Lock lock(m_initializing);

	PlotArea::BackgroundImageStyle style = (PlotArea::BackgroundImageStyle)index;
	foreach(Plot3D* plot, m_plotsList)
		plot->setBackgroundImageStyle(style);
}

void Plot3DDock::onBackgroundBrushStyleChanged(int index){
	const Lock lock(m_initializing);

	Qt::BrushStyle style = (Qt::BrushStyle)index;
	foreach(Plot3D* plot, m_plotsList){
		plot->setBackgroundBrushStyle(style);
	}
}

void Plot3DDock::onBackgroundFirstColorChanged(const QColor& c){
	const Lock lock(m_initializing);

	foreach(Plot3D* plot, m_plotsList){
		plot->setBackgroundFirstColor(c);
	}
}

void Plot3DDock::onBackgroundSecondColorChanged(const QColor& c){
	const Lock lock(m_initializing);

	foreach(Plot3D* plot, m_plotsList){
		plot->setBackgroundSecondColor(c);
	}
}

void Plot3DDock::onBackgroundOpacityChanged(int value){
	const Lock lock(m_initializing);

	float opacity = (float)value/100;
	foreach(Plot3D* plot, m_plotsList)
		plot->setBackgroundOpacity(opacity);
}

void Plot3DDock::onBackgroundSelectFile() {
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

	const Lock lock(m_initializing);
	foreach(Plot3D* plot, m_plotsList)
		plot->setBackgroundFileName(path);
}

void Plot3DDock::onBackgroundFileNameChanged(){
	if (m_initializing)
		return;

	const Lock lock(m_initializing);
	QString fileName = ui.kleBackgroundFileName->text();
	foreach(Plot3D* plot, m_plotsList)
		plot->setBackgroundFileName(fileName);
}

// Light

void Plot3DDock::onIntensityChanged(double value) {
	const Lock lock(m_initializing);
	m_plot->setIntensity(value);
}

void Plot3DDock::onAmbientChanged(const QColor& color) {
	const Lock lock(m_initializing);
	m_plot->setAmbient(color);
}

void Plot3DDock::onDiffuseChanged(const QColor& color) {
	const Lock lock(m_initializing);
	m_plot->setDiffuse(color);
}

void Plot3DDock::onSpecularChanged(const QColor& color) {
	const Lock lock(m_initializing);
	m_plot->setSpecular(color);
}

void Plot3DDock::onElevationChanged(int elevation) {
	const Lock lock(m_initializing);
	m_plot->setElevation(elevation);
}

void Plot3DDock::onAzimuthChanged(int azimuth) {
	const Lock lock(m_initializing);
	m_plot->setAzimuth(azimuth);
}

void Plot3DDock::onConeAngleChanged(int angle) {
	const Lock lock(m_initializing);
	m_plot->setConeAngle(angle);
}


//*************************************************************
//******** SLOTs for changes triggered in Plot3D **************
//*************************************************************
// "General"-tab
void Plot3DDock::descriptionChanged(const AbstractAspect* aspect) {
	if (m_plot != aspect)
		return;

	if (m_initializing)
		return;

	if (aspect->name() != ui.leName->text()) {
		ui.leName->setText(aspect->name());
	} else if (aspect->comment() != ui.leComment->text()) {
		ui.leComment->setText(aspect->comment());
	}
}


void Plot3DDock::rectChanged(const QRectF& rect){
	if (m_initializing)
		return;
	ui.sbLeft->setValue(Worksheet::convertFromSceneUnits(rect.x(), Worksheet::Centimeter));
	ui.sbTop->setValue(Worksheet::convertFromSceneUnits(rect.y(), Worksheet::Centimeter));
	ui.sbWidth->setValue(Worksheet::convertFromSceneUnits(rect.width(), Worksheet::Centimeter));
	ui.sbHeight->setValue(Worksheet::convertFromSceneUnits(rect.height(), Worksheet::Centimeter));
}

void Plot3DDock::visibleChanged(bool on){
	if (m_initializing)
		return;
	ui.chkVisible->setChecked(on);
}

// "Background"-tab
void Plot3DDock::backgroundTypeChanged(PlotArea::BackgroundType type) {
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
	ui.cbBackgroundType->setCurrentIndex(type);
}

void Plot3DDock::backgroundColorStyleChanged(PlotArea::BackgroundColorStyle style) {
	if (m_initializing)
		return;
	ui.cbBackgroundColorStyle->setCurrentIndex(style);
}

void Plot3DDock::backgroundImageStyleChanged(PlotArea::BackgroundImageStyle style) {
	if (m_initializing)
		return;
	ui.cbBackgroundImageStyle->setCurrentIndex(style);
}

void Plot3DDock::backgroundBrushStyleChanged(Qt::BrushStyle style) {
	if (m_initializing)
		return;
	ui.cbBackgroundBrushStyle->setCurrentIndex(style);
}

void Plot3DDock::backgroundFirstColorChanged(const QColor& color) {
	if (m_initializing)
		return;
	ui.kcbBackgroundFirstColor->setColor(color);
}

void Plot3DDock::backgroundSecondColorChanged(const QColor& color) {
	if (m_initializing)
		return;
	ui.kcbBackgroundSecondColor->setColor(color);
}

void Plot3DDock::backgroundFileNameChanged(const QString& name) {
	if (m_initializing)
		return;
	ui.kleBackgroundFileName->setText(name);
}

void Plot3DDock::backgroundOpacityChanged(float opacity) {
	if (m_initializing)
		return;
	ui.sbBackgroundOpacity->setValue( round(opacity*100.0) );
}

// Light
void Plot3DDock::intensityChanged(double val) {
	if (m_initializing)
		return;
	ui.sbLightIntensity->setValue(val);
}

void Plot3DDock::ambientChanged(const QColor& color) {
	if (m_initializing)
		return;
	ui.kcbLightAmbientColor->setColor(color);
}

void Plot3DDock::diffuseChanged(const QColor& color) {
	if (m_initializing)
		return;
	ui.kcbLightDiffuseColor->setColor(color);
}

void Plot3DDock::specularChanged(const QColor& color) {
	if (m_initializing)
		return;
	ui.kcbLightSpecularColor->setColor(color);
}

void Plot3DDock::elevationChanged(double val) {
	if (m_initializing)
		return;
	ui.sbLightElevation->setValue(val);
}

void Plot3DDock::azimuthChanged(double val) {
	if (m_initializing)
		return;
	ui.sbLightAzimuth->setValue(val);
}

void Plot3DDock::coneAngleChanged(double val) {
	if (m_initializing)
		return;
	ui.sbLightConeAngle->setValue(val);
}

//*************************************************************
//******************** SETTINGS *******************************
//*************************************************************
void Plot3DDock::load(){
	//General
	visibleChanged(m_plot->isVisible());
	rectChanged(m_plot->rect());
	//TODO:
	//ui.cbType->setCurrentIndex((int)m_plot->visualizationType())

	// General
	ui.leName->setText(m_plot->name());
	ui.leComment->setText(m_plot->comment());

	//Background
	backgroundTypeChanged(m_plot->backgroundType());
	backgroundColorStyleChanged(m_plot->backgroundColorStyle());
	backgroundImageStyleChanged(m_plot->backgroundImageStyle());
	backgroundBrushStyleChanged(m_plot->backgroundBrushStyle());
	backgroundFileNameChanged(m_plot->backgroundFileName());
	backgroundFirstColorChanged(m_plot->backgroundFirstColor());
	backgroundSecondColorChanged(m_plot->backgroundSecondColor());
	backgroundOpacityChanged(m_plot->backgroundOpacity());
	//Light
	intensityChanged(m_plot->intensity());
	ambientChanged(m_plot->ambient());
	diffuseChanged(m_plot->diffuse());
	specularChanged(m_plot->specular());

	elevationChanged(m_plot->elevation());
	azimuthChanged(m_plot->azimuth());
	coneAngleChanged(m_plot->coneAngle());
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
	backgroundOpacityChanged(group.readEntry("BackgroundOpacity", m_plot->backgroundOpacity()));

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