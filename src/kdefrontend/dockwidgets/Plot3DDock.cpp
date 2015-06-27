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
#include "backend/core/AbstractAspect.h"
#include "backend/core/AbstractColumn.h"
#include "backend/core/AspectTreeModel.h"
#include "backend/core/Project.h"
#include "backend/worksheet/plots/3d/Plot3D.h"
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

Plot3DDock::Plot3DDock(QWidget* parent) : QWidget(parent){
	ui.setupUi(this);

	//TODO: remove this later - the initialization of the dock widget will be done in setPlots() later
	hideDataSource();
	hideFileUrl();
	hideTriangleInfo();
	onShowAxes(ui.showAxes->isChecked());
	//######

	this->retranslateUi();

	//General-tab
	ui.cbDataSource->insertItem(Plot3D::DataSource_File, i18n("File"));
	ui.cbDataSource->insertItem(Plot3D::DataSource_Spreadsheet, i18n("Spreadsheet"));
	ui.cbDataSource->insertItem(Plot3D::DataSource_Matrix, i18n("Matrix"));
	ui.cbDataSource->insertItem(Plot3D::DataSource_Empty, i18n("Demo"));
	ui.cbDataSource->setCurrentIndex(Plot3D::DataSource_File);

	ui.cbType->insertItem(Plot3D::VisualizationType_Triangles, i18n("Triangles"));
	ui.cbType->setCurrentIndex(Plot3D::VisualizationType_Triangles);
	onVisualizationTypeChanged(ui.cbType->currentIndex());


	//Spreadsheet data source
	QList<const char*>  list;
	list<<"Folder"<<"Workbook"<<"Spreadsheet"<<"FileDataSource"<<"Column";

	const QVector<TreeViewComboBox*> treeViews(QVector<TreeViewComboBox*>()
			<< ui.cbXCoordinate << ui.cbYCoordinate << ui.cbZCoordinate
			<< ui.cbNode1 << ui.cbNode2 << ui.cbNode3);

	foreach(TreeViewComboBox* view, treeViews){
		view->setTopLevelClasses(list);
	}

	list.clear();
	list<<"Column";

	foreach(TreeViewComboBox* view, treeViews){
		view->setSelectableClasses(list);
		connect(view, SIGNAL(currentModelIndexChanged(const QModelIndex&)), this, SLOT(onTreeViewIndexChanged(const QModelIndex&)));
	}

	//Matrix data source
	list.clear();
	list<<"Folder"<<"Workbook"<<"Matrix";
	ui.cbMatrix->setTopLevelClasses(list);

	list.clear();
	list<<"Matrix";
	ui.cbMatrix->setSelectableClasses(list);

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
	connect(ui.leName, SIGNAL(returnPressed()), this, SLOT(nameChanged()));
	connect(ui.cbDataSource, SIGNAL(currentIndexChanged(int)), this, SLOT(onDataSourceChanged(int)));
	connect(ui.cbType, SIGNAL(currentIndexChanged(int)), this, SLOT(onVisualizationTypeChanged(int)));
	connect(ui.cbFileRequester, SIGNAL(urlSelected(const KUrl&)), this, SLOT(onFileChanged(const KUrl&)));
	connect(ui.showAxes, SIGNAL(toggled(bool)), this, SLOT(onShowAxes(bool)));

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
	connect(templateHandler, SIGNAL(info(QString)), this, SIGNAL(info(QString)));
}

void Plot3DDock::setPlots(const QList<Plot3D*>& plots){
	m_initializing = true;
	m_plotsList = plots;
	Q_ASSERT(m_plotsList.size());
	m_plot = m_plotsList.first();

	QAbstractItemModel *aspectTreeModel = new AspectTreeModel(m_plot->project());

	const QVector<TreeViewComboBox*> treeViews(QVector<TreeViewComboBox*>()
			<< ui.cbXCoordinate << ui.cbYCoordinate << ui.cbZCoordinate
			<< ui.cbNode1 << ui.cbNode2 << ui.cbNode3 << ui.cbMatrix);

	foreach(TreeViewComboBox* cb, treeViews)
		cb->setModel(aspectTreeModel);

// 	if (ui.cbType->currentIndex() != -1)
// 		onVisualizationTypeChanged(ui.cbType->currentIndex());

	//show the properties of the first plot
	this->load();

	//update active widgets
	backgroundTypeChanged(ui.cbBackgroundType->currentIndex());

	//SIGNALs/SLOTs
	//general

	//background
	connect(m_plot,SIGNAL(backgroundTypeChanged(PlotArea::BackgroundType)),this,SLOT(plotBackgroundTypeChanged(PlotArea::BackgroundType)));
	connect(m_plot,SIGNAL(backgroundColorStyleChanged(PlotArea::BackgroundColorStyle)),this,SLOT(plotBackgroundColorStyleChanged(PlotArea::BackgroundColorStyle)));
	connect(m_plot,SIGNAL(backgroundImageStyleChanged(PlotArea::BackgroundImageStyle)),this,SLOT(plotBackgroundImageStyleChanged(PlotArea::BackgroundImageStyle)));
	connect(m_plot,SIGNAL(backgroundBrushStyleChanged(Qt::BrushStyle)),this,SLOT(plotBackgroundBrushStyleChanged(Qt::BrushStyle)));
	connect(m_plot,SIGNAL(backgroundFirstColorChanged(QColor)),this,SLOT(plotBackgroundFirstColorChanged(QColor)));
	connect(m_plot,SIGNAL(backgroundSecondColorChanged(QColor)),this,SLOT(plotBackgroundSecondColorChanged(QColor)));
	connect(m_plot,SIGNAL(backgroundFileNameChanged(QString)),this,SLOT(plotBackgroundFileNameChanged(QString)));
	connect(m_plot,SIGNAL(backgroundOpacityChanged(float)),this,SLOT(plotBackgroundOpacityChanged(float)));

	//light


	m_initializing = false;
}

//TODO:
AbstractColumn* Plot3DDock::getColumn(const QModelIndex& index) const{
	AbstractAspect* aspect = static_cast<AbstractAspect*>(index.internalPointer());
	return aspect ? dynamic_cast<AbstractColumn*>(aspect) : 0;
}

//*************************************************************
//****** SLOTs for changes triggered in Plot3DDock *********
//*************************************************************
void Plot3DDock::retranslateUi(){
	m_initializing = true;

	//general

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

	m_initializing = false;
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

void Plot3DDock::onTreeViewIndexChanged(const QModelIndex& index){
	qDebug() << Q_FUNC_INFO;
	AbstractColumn* column = getColumn(index);
	Q_ASSERT(column);

	foreach(Plot3D* plot, m_plotsList){
		plot->setDataSource(Plot3D::DataSource_Spreadsheet);
		QObject *senderW = sender();
		if(senderW == ui.cbXCoordinate)
			plot->setXColumn(column);
		else if(senderW  == ui.cbYCoordinate)
			plot->setYColumn(column);
		else if(senderW  == ui.cbZCoordinate)
			plot->setZColumn(column);
		else if(senderW  == ui.cbNode1)
			plot->setNodeColumn(0, column);
		else if(senderW  == ui.cbNode2)
			plot->setNodeColumn(1, column);
		else if(senderW == ui.cbNode3)
			plot->setNodeColumn(2, column);
		plot->retransform();
	}
}

void Plot3DDock::onVisualizationTypeChanged(int index){
	qDebug() << Q_FUNC_INFO << index;
	foreach(Plot3D* plot, m_plotsList){
		plot->setVisualizationType(static_cast<Plot3D::VisualizationType>(index));
		plot->retransform();
	}

	if (index == Plot3D::VisualizationType_Triangles){
		hideDataSource(false);
		onDataSourceChanged(ui.cbDataSource->currentIndex());
	}else{
		hideDataSource();
		hideFileUrl();
		hideTriangleInfo();
	}
}

void Plot3DDock::onShowAxes(bool show){
	foreach(Plot3D* plot, m_plotsList){
		plot->setShowAxes(show);
		plot->retransform();
	}
}

void Plot3DDock::onFileChanged(const KUrl& path){
	if (!path.isLocalFile())
		return;

	foreach(Plot3D* plot, m_plotsList)
		plot->setFile(path);
}

void Plot3DDock::onDataSourceChanged(int index){
	qDebug() << Q_FUNC_INFO << index;
	Plot3D::DataSource type = (Plot3D::DataSource)index;
	hideFileUrl(index != Plot3D::DataSource_File);
	hideTriangleInfo(index != Plot3D::DataSource_Spreadsheet);

	bool b = (type==Plot3D::DataSource_Matrix);
	ui.labelMatrix->setVisible(b);
	ui.cbMatrix->setVisible(b);

	foreach(Plot3D* plot, m_plotsList)
		plot->setDataSource(type);
}

void Plot3DDock::hideDataSource(bool hide){
	ui.labelSource->setVisible(!hide);
	ui.cbDataSource->setVisible(!hide);
}

void Plot3DDock::hideFileUrl(bool hide){
	ui.labelFile->setVisible(!hide);
	ui.cbFileRequester->setVisible(!hide);
}

void Plot3DDock::hideTriangleInfo(bool hide){
	const QVector<QWidget*> widgets(QVector<QWidget*>()
			<< ui.labelX << ui.labelY << ui.labelZ
			<< ui.cbXCoordinate << ui.cbYCoordinate << ui.cbZCoordinate
			<< ui.labelNode1 << ui.labelNode2 << ui.labelNode3
			<< ui.cbNode1 << ui.cbNode2 << ui.cbNode3);

	foreach(QWidget* w, widgets){
		w->setVisible(!hide);
	}
}

// "Background"-tab
void Plot3DDock::backgroundTypeChanged(int index){
	qDebug()<<"Plot3DDock::backgroundTypeChanged " << index;
	PlotArea::BackgroundType type = (PlotArea::BackgroundType)index;

	if (type == PlotArea::Color){
		ui.lBackgroundColorStyle->show();
		ui.cbBackgroundColorStyle->show();
		ui.lBackgroundImageStyle->hide();
		ui.cbBackgroundImageStyle->hide();
		ui.lBackgroundBrushStyle->hide();
		ui.cbBackgroundBrushStyle->hide();

		ui.lBackgroundFileName->hide();
		ui.kleBackgroundFileName->hide();
		ui.bBackgroundOpen->hide();

		ui.lBackgroundFirstColor->show();
		ui.kcbBackgroundFirstColor->show();

		PlotArea::BackgroundColorStyle style =
			(PlotArea::BackgroundColorStyle) ui.cbBackgroundColorStyle->currentIndex();
		if (style == PlotArea::SingleColor){
			ui.lBackgroundFirstColor->setText(i18n("Color"));
			ui.lBackgroundSecondColor->hide();
			ui.kcbBackgroundSecondColor->hide();
		}else{
			ui.lBackgroundFirstColor->setText(i18n("First Color"));
			ui.lBackgroundSecondColor->show();
			ui.kcbBackgroundSecondColor->show();
		}
	}else if(type == PlotArea::Image){
		ui.lBackgroundFirstColor->hide();
		ui.kcbBackgroundFirstColor->hide();
		ui.lBackgroundSecondColor->hide();
		ui.kcbBackgroundSecondColor->hide();

		ui.lBackgroundColorStyle->hide();
		ui.cbBackgroundColorStyle->hide();
		ui.lBackgroundImageStyle->show();
		ui.cbBackgroundImageStyle->show();
		ui.lBackgroundBrushStyle->hide();
		ui.cbBackgroundBrushStyle->hide();
		ui.lBackgroundFileName->show();
		ui.kleBackgroundFileName->show();
		ui.bBackgroundOpen->show();
	}else if(type == PlotArea::Pattern){
		ui.lBackgroundFirstColor->setText(i18n("Color"));
		ui.lBackgroundFirstColor->show();
		ui.kcbBackgroundFirstColor->show();
		ui.lBackgroundSecondColor->hide();
		ui.kcbBackgroundSecondColor->hide();

		ui.lBackgroundColorStyle->hide();
		ui.cbBackgroundColorStyle->hide();
		ui.lBackgroundImageStyle->hide();
		ui.cbBackgroundImageStyle->hide();
		ui.lBackgroundBrushStyle->show();
		ui.cbBackgroundBrushStyle->show();
		ui.lBackgroundFileName->hide();
		ui.kleBackgroundFileName->hide();
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
		ui.lBackgroundSecondColor->hide();
		ui.kcbBackgroundSecondColor->hide();
	}else{
		ui.lBackgroundFirstColor->setText(i18n("First Color"));
		ui.lBackgroundSecondColor->show();
		ui.kcbBackgroundSecondColor->show();
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

	m_initializing = true;
	if (aspect->name() != ui.leName->text()) {
		ui.leName->setText(aspect->name());
	} else if (aspect->comment() != ui.leComment->text()) {
		ui.leComment->setText(aspect->comment());
	}
	m_initializing = false;
}

// "Background"-tab
void Plot3DDock::plotBackgroundTypeChanged(PlotArea::BackgroundType type) {
	m_initializing = true;
	ui.cbBackgroundType->setCurrentIndex(type);
	m_initializing = false;
}

void Plot3DDock::plotBackgroundColorStyleChanged(PlotArea::BackgroundColorStyle style) {
	m_initializing = true;
	ui.cbBackgroundColorStyle->setCurrentIndex(style);
	m_initializing = false;
}

void Plot3DDock::plotBackgroundImageStyleChanged(PlotArea::BackgroundImageStyle style) {
	m_initializing = true;
	ui.cbBackgroundImageStyle->setCurrentIndex(style);
	m_initializing = false;
}

void Plot3DDock::plotBackgroundBrushStyleChanged(Qt::BrushStyle style) {
	m_initializing = true;
	ui.cbBackgroundBrushStyle->setCurrentIndex(style);
	m_initializing = false;
}

void Plot3DDock::plotBackgroundFirstColorChanged(const QColor& color) {
	m_initializing = true;
	ui.kcbBackgroundFirstColor->setColor(color);
	m_initializing = false;
}

void Plot3DDock::plotBackgroundSecondColorChanged(const QColor& color) {
	m_initializing = true;
	ui.kcbBackgroundSecondColor->setColor(color);
	m_initializing = false;
}

void Plot3DDock::plotBackgroundFileNameChanged(const QString& name) {
	m_initializing = true;
	ui.kleBackgroundFileName->setText(name);
	m_initializing = false;
}

void Plot3DDock::plotBackgroundOpacityChanged(float opacity) {
	m_initializing = true;
	ui.sbBackgroundOpacity->setValue( round(opacity*100.0) );
	m_initializing = false;
}


//*************************************************************
//******************** SETTINGS *******************************
//*************************************************************
void Plot3DDock::load(){
	//General

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
