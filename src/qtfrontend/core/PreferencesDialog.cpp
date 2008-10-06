/***************************************************************************
    File                 : PreferencesDialog.cpp
    Project              : SciDAVis
    --------------------------------------------------------------------
    Copyright            : (C) 2006 by Ion Vasilief, Tilman Benkert
    Email (use @ for *)  : ion_vasilief*yahoo.fr, thzs*gmx.net
    Description          : Preferences dialog

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
#include "PreferencesDialog.h"
#include "ApplicationWindow.h"
#include "lib/ColorButton.h"
#include "lib/ColorBox.h"
#include "graph/Graph.h"
#include "graph/Layer.h"
#include "matrix/Matrix.h"

#include <QLocale>
#include <QPushButton>
#include <QLabel>
#include <QGridLayout>
#include <QGroupBox>
#include <QFont>
#include <QFontDialog>
#include <QColorDialog>
#include <QTabWidget>
#include <QStackedWidget>
#include <QWidget>
#include <QComboBox>
#include <QSpinBox>
#include <QRadioButton>
#include <QStyleFactory>
#include <QRegExp>
#include <QMessageBox>
#include <QTranslator>
#include <QApplication>
#include <QDir>
#include <QPixmap>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QListWidget>
#include <QFontMetrics>

PreferencesDialog::PreferencesDialog( QWidget* parent, Qt::WFlags fl )
    : QDialog( parent, fl )
{
	// get current values from app window
	ApplicationWindow *app = (ApplicationWindow *)parentWidget();
	plot3DColors = app->plot3DColors;
	plot3DTitleFont = app->plot3DTitleFont;
	plot3DNumbersFont = app->plot3DNumbersFont;
	plot3DAxesFont = app->plot3DAxesFont;
	textFont = app->tableTextFont;
	headerFont = app->tableHeaderFont;
	appFont = app->appFont;
	axesFont = app->plotAxesFont;
	numbersFont = app->plotNumbersFont;
	legendFont = app->plotLegendFont;
	titleFont = app->plotTitleFont;

	// create the GUI
	generalDialog = new QStackedWidget();
	itemsList = new QListWidget();
	itemsList->setSpacing(10);
	itemsList->setAlternatingRowColors( true );

	initAppPage();
	initTablesPage();
	initPlotsPage();
	initPlots3DPage();
	initFittingPage();

	generalDialog->addWidget(appTabWidget);
	generalDialog->addWidget(tables);
	generalDialog->addWidget(plotsTabWidget);
	generalDialog->addWidget(plots3D);
	generalDialog->addWidget(fitPage);

	QVBoxLayout * rightLayout = new QVBoxLayout();
	lblPageHeader = new QLabel();
	QFont fnt = this->font();
	fnt.setPointSize(fnt.pointSize() + 3);
	fnt.setBold(true);
	lblPageHeader->setFont(fnt);
	lblPageHeader->setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);

	QPalette pal = lblPageHeader->palette();
	pal.setColor( QPalette::Window, app->panelsColor );
	lblPageHeader->setPalette(pal);
	lblPageHeader->setAutoFillBackground( true );

	rightLayout->setSpacing(10);
	rightLayout->addWidget( lblPageHeader );
	rightLayout->addWidget( generalDialog );

	QHBoxLayout * topLayout = new QHBoxLayout();
	topLayout->setSpacing(5);
	topLayout->setMargin(5);
	topLayout->addWidget( itemsList );
	topLayout->addLayout( rightLayout );

	QHBoxLayout * bottomButtons = new QHBoxLayout();
	bottomButtons->addStretch();
	buttonApply = new QPushButton();
	buttonApply->setAutoDefault( true );
	bottomButtons->addWidget( buttonApply );

	buttonOk = new QPushButton();
	buttonOk->setAutoDefault( true );
	buttonOk->setDefault( true );
	bottomButtons->addWidget( buttonOk );

	buttonCancel = new QPushButton();
	buttonCancel->setAutoDefault( true );
	bottomButtons->addWidget( buttonCancel );

	QVBoxLayout * mainLayout = new QVBoxLayout( this );
	mainLayout->addLayout(topLayout);
	mainLayout->addLayout(bottomButtons);

	languageChange();

	// signals and slots connections
	connect( itemsList, SIGNAL(currentRowChanged(int)), this, SLOT(setCurrentPage(int)));
	connect( buttonOk, SIGNAL( clicked() ), this, SLOT( accept() ) );
	connect( buttonApply, SIGNAL( clicked() ), this, SLOT( apply() ) );
	connect( buttonCancel, SIGNAL( clicked() ), this, SLOT( reject() ) );
	connect( buttonBackground, SIGNAL( clicked() ), this, SLOT( pickBgColor() ) );
	connect( buttonText, SIGNAL( clicked() ), this, SLOT( pickTextColor() ) );
	connect( buttonHeader, SIGNAL( clicked() ), this, SLOT( pickHeaderColor() ) );
	connect( buttonTextFont, SIGNAL( clicked() ), this, SLOT( pickTextFont() ) );
	connect( buttonHeaderFont, SIGNAL( clicked() ), this, SLOT( pickHeaderFont() ) );

	setCurrentPage(0);
}

void PreferencesDialog::setCurrentPage(int index)
{
	generalDialog->setCurrentIndex(index);
	if(itemsList->currentItem())
		lblPageHeader->setText(itemsList->currentItem()->text());
}

void PreferencesDialog::initTablesPage()
{
	ApplicationWindow *app = (ApplicationWindow *)parentWidget();
	tables = new QWidget();

	QHBoxLayout * topLayout = new QHBoxLayout();
	topLayout->setSpacing(5);

	lblSeparator = new QLabel();
	topLayout->addWidget( lblSeparator );
	boxSeparator = new QComboBox();
	boxSeparator->setEditable( true );
	topLayout->addWidget( boxSeparator );

	QString help = tr("The column separator can be customized. \nThe following special codes can be used:\n\\t for a TAB character \n\\s for a SPACE");
	help += "\n"+tr("The separator must not contain the following characters: \n0-9eE.+-");

	boxSeparator->setWhatsThis(help);
	boxSeparator->setToolTip(help);
	lblSeparator->setWhatsThis(help);
	lblSeparator->setToolTip(help);

	groupBoxTableCol = new QGroupBox();
	QGridLayout * colorsLayout = new QGridLayout(groupBoxTableCol);

	lblTableBackground = new QLabel();
	colorsLayout->addWidget( lblTableBackground, 0, 0 );
	buttonBackground= new ColorButton();
	buttonBackground->setColor(app->tableBkgdColor);
	colorsLayout->addWidget( buttonBackground, 0, 1 );

	lblTextColor = new QLabel();
	colorsLayout->addWidget( lblTextColor, 1, 0 );
	buttonText= new ColorButton();
	buttonText->setColor(app->tableTextColor);
	colorsLayout->addWidget( buttonText, 1, 1 );

	lblHeaderColor = new QLabel();
	colorsLayout->addWidget( lblHeaderColor, 2, 0 );
	buttonHeader= new ColorButton();
	buttonHeader->setColor(app->tableHeaderColor);
	colorsLayout->addWidget( buttonHeader, 2, 1 );

	groupBoxTableFonts = new QGroupBox();
	QHBoxLayout * bottomLayout = new QHBoxLayout( groupBoxTableFonts );

	buttonTextFont= new QPushButton();
	bottomLayout->addWidget( buttonTextFont );
	buttonHeaderFont= new QPushButton();
	bottomLayout->addWidget( buttonHeaderFont );

	boxTableComments = new QCheckBox();
	boxTableComments->setChecked(app->m_show_table_comments);

	QVBoxLayout * tablesPageLayout = new QVBoxLayout( tables );
	tablesPageLayout->addWidget(boxTableComments);
	tablesPageLayout->addLayout(topLayout,1);
	tablesPageLayout->addWidget(groupBoxTableCol);
	tablesPageLayout->addWidget(groupBoxTableFonts);
	tablesPageLayout->addStretch();
}

void PreferencesDialog::initPlotsPage()
{
	ApplicationWindow *app = (ApplicationWindow *)parentWidget();

	plotsTabWidget = new QTabWidget();
	plotOptions = new QWidget();

	QVBoxLayout * optionsTabLayout = new QVBoxLayout( plotOptions );
	optionsTabLayout->setSpacing(5);

	QGroupBox * groupBoxOptions = new QGroupBox();
	optionsTabLayout->addWidget( groupBoxOptions );

	QGridLayout * optionsLayout = new QGridLayout( groupBoxOptions );

	boxAutoscaling = new QCheckBox();
	boxAutoscaling->setChecked(app->autoscale2DPlots);
	optionsLayout->addWidget( boxAutoscaling, 0, 0 );

	boxScaleFonts = new QCheckBox();
	boxScaleFonts->setChecked(app->autoScaleFonts);
	optionsLayout->addWidget( boxScaleFonts, 0, 1 );

	boxTitle = new QCheckBox();
	boxTitle->setChecked(app->titleOn);
	optionsLayout->addWidget( boxTitle, 1, 0 );

	boxAllAxes = new QCheckBox();
	boxAllAxes->setChecked (app->allAxesOn);
	optionsLayout->addWidget( boxAllAxes, 1, 1 );

	boxAntialiasing = new QCheckBox();
	boxAntialiasing->setChecked(app->antialiasing2DPlots);
	optionsLayout->addWidget( boxAntialiasing, 2, 0 );

	boxBackbones= new QCheckBox();
	boxBackbones->setChecked(app->drawBackbones);
	optionsLayout->addWidget( boxBackbones, 2, 1 );

	boxFrame = new QCheckBox();
	boxFrame->setChecked(app->canvasFrameOn);
	optionsLayout->addWidget( boxFrame, 3, 0 );

	labelFrameWidth = new QLabel();
	optionsLayout->addWidget( labelFrameWidth, 4, 0 );
	boxFrameWidth= new QSpinBox();
	optionsLayout->addWidget( boxFrameWidth, 4, 1 );
	boxFrameWidth->setRange(1, 100);
	boxFrameWidth->setValue(app->canvasFrameWidth);
	if (!app->canvasFrameOn)
	{
		labelFrameWidth->hide();
		boxFrameWidth->hide();
	}

	lblAxesLineWidth = new QLabel();
	optionsLayout->addWidget( lblAxesLineWidth, 5, 0 );
	boxLineWidth= new QSpinBox();
	boxLineWidth->setRange(0, 100);
	boxLineWidth->setValue(app->axesLineWidth);
	optionsLayout->addWidget( boxLineWidth, 5, 1 );

	lblMargin = new QLabel();
	optionsLayout->addWidget( lblMargin, 6, 0 );
	boxMargin= new QSpinBox();
	boxMargin->setRange(0, 1000);
	boxMargin->setSingleStep(5);
	boxMargin->setValue(app->defaultPlotMargin);
	optionsLayout->addWidget( boxMargin, 6, 1 );

	optionsLayout->setRowStretch( 7, 1 );

	boxResize = new QCheckBox();
	boxResize->setChecked(!app->autoResizeLayers);
	if(boxResize->isChecked())
		boxScaleFonts->setEnabled(false);

	optionsTabLayout->addWidget( boxResize );

	plotsTabWidget->addTab( plotOptions, QString() );

	initCurvesPage();

	plotsTabWidget->addTab( curves, QString() );

	plotTicks = new QWidget();
	QVBoxLayout * plotTicksLayout = new QVBoxLayout( plotTicks );

	QGroupBox * ticksGroupBox = new QGroupBox();
	QGridLayout * ticksLayout = new QGridLayout( ticksGroupBox );
	plotTicksLayout->addWidget( ticksGroupBox );

	lblMajTicks = new QLabel();
	ticksLayout->addWidget( lblMajTicks, 0, 0 );
	boxMajTicks = new QComboBox();
	ticksLayout->addWidget( boxMajTicks, 0, 1 );

	lblMajTicksLength = new QLabel();
	ticksLayout->addWidget( lblMajTicksLength, 0, 2 );
	boxMajTicksLength = new QSpinBox();
	boxMajTicksLength->setRange(0, 100);
	boxMajTicksLength->setValue(app->majTicksLength);
	ticksLayout->addWidget( boxMajTicksLength, 0, 3 );

	lblMinTicks = new QLabel();
	ticksLayout->addWidget( lblMinTicks, 1, 0 );
	boxMinTicks = new QComboBox();
	ticksLayout->addWidget( boxMinTicks, 1, 1 );

	lblMinTicksLength = new QLabel();
	ticksLayout->addWidget( lblMinTicksLength, 1, 2 );
	boxMinTicksLength = new QSpinBox();
	boxMinTicksLength->setRange(0, 100);
	boxMinTicksLength->setValue(app->minTicksLength);
	ticksLayout->addWidget( boxMinTicksLength, 1, 3 );

	ticksLayout->setRowStretch( 4, 1 );

	plotsTabWidget->addTab( plotTicks, QString() );

	plotFonts = new QWidget();
	QVBoxLayout * plotFontsLayout = new QVBoxLayout( plotFonts );

	QGroupBox * groupBox2DFonts = new QGroupBox();
	plotFontsLayout->addWidget( groupBox2DFonts );
	QVBoxLayout * fontsLayout = new QVBoxLayout( groupBox2DFonts );
	buttonTitleFont= new QPushButton();
	fontsLayout->addWidget( buttonTitleFont );
	buttonLegendFont= new QPushButton();
	fontsLayout->addWidget( buttonLegendFont );
	buttonAxesFont= new QPushButton();
	fontsLayout->addWidget( buttonAxesFont );
	buttonNumbersFont= new QPushButton();
	fontsLayout->addWidget( buttonNumbersFont );
	fontsLayout->addStretch();

	plotsTabWidget->addTab( plotFonts, QString() );

	plotPrint = new QWidget();
	QVBoxLayout *printLayout = new QVBoxLayout( plotPrint );

	boxScaleLayersOnPrint = new QCheckBox();
	boxScaleLayersOnPrint->setChecked(app->m_scale_plots_on_print);
	printLayout->addWidget( boxScaleLayersOnPrint );

	boxPrintCropmarks = new QCheckBox();
	boxPrintCropmarks->setChecked(app->m_print_cropmarks);
	printLayout->addWidget( boxPrintCropmarks );
	printLayout->addStretch();
	plotsTabWidget->addTab(plotPrint, QString());

	connect( boxResize, SIGNAL( clicked() ), this, SLOT( enableScaleFonts() ) );
	connect( boxFrame, SIGNAL( toggled(bool) ), this, SLOT( showFrameWidth(bool) ) );
	connect( buttonAxesFont, SIGNAL( clicked() ), this, SLOT( pickAxesFont() ) );
	connect( buttonNumbersFont, SIGNAL( clicked() ), this, SLOT( pickNumbersFont() ) );
	connect( buttonLegendFont, SIGNAL( clicked() ), this, SLOT( pickLegendFont() ) );
	connect( buttonTitleFont, SIGNAL( clicked() ), this, SLOT( pickTitleFont() ) );
}

void PreferencesDialog::enableScaleFonts()
{
	if(boxResize->isChecked())
		boxScaleFonts->setEnabled(false);
	else
		boxScaleFonts->setEnabled(true);
}

void PreferencesDialog::showFrameWidth(bool ok)
{
	if (!ok)
	{
		boxFrameWidth->hide();
		labelFrameWidth->hide();
	}
	else
	{
		boxFrameWidth->show();
		labelFrameWidth->show();
	}
}

void PreferencesDialog::initPlots3DPage()
{
	ApplicationWindow *app = (ApplicationWindow *)parentWidget();
	plots3D = new QWidget();

	QGroupBox * topBox = new QGroupBox();
	QGridLayout * topLayout = new QGridLayout( topBox );
	topLayout->setSpacing(5);

	lblResolution = new QLabel();
	topLayout->addWidget( lblResolution, 0, 0 );
	boxResolution = new QSpinBox();
	boxResolution->setRange(1, 100);
	boxResolution->setValue(app->plot3DResolution);
	topLayout->addWidget( boxResolution, 0, 1 );

	boxShowLegend = new QCheckBox();
	boxShowLegend->setChecked(app->showPlot3DLegend);
	topLayout->addWidget( boxShowLegend, 1, 0 );

	boxShowProjection = new QCheckBox();
	boxShowProjection->setChecked(app->showPlot3DProjection);
	topLayout->addWidget( boxShowProjection, 1, 1 );

	boxSmoothMesh = new QCheckBox();
	boxSmoothMesh->setChecked(app->smooth3DMesh);
	topLayout->addWidget( boxSmoothMesh, 2, 0 );

	boxOrthogonal = new QCheckBox();
	boxOrthogonal->setChecked(app->orthogonal3DPlots);
	topLayout->addWidget( boxOrthogonal, 2, 1 );

	boxAutoscale3DPlots = new QCheckBox();
	boxAutoscale3DPlots->setChecked(app->autoscale3DPlots);
	topLayout->addWidget( boxAutoscale3DPlots, 3, 0 );

	groupBox3DCol = new QGroupBox();
	QGridLayout * middleLayout = new QGridLayout( groupBox3DCol );

	btnFromColor = new QPushButton();
	middleLayout->addWidget( btnFromColor, 0, 0 );
	btnLabels = new QPushButton();
	middleLayout->addWidget( btnLabels, 0, 1 );
	btnMesh = new QPushButton();
	middleLayout->addWidget( btnMesh, 0, 2 );
	btnGrid = new QPushButton();
	middleLayout->addWidget( btnGrid, 0, 3 );
	btnToColor = new QPushButton();
	middleLayout->addWidget( btnToColor, 1, 0 );
	btnNumbers = new QPushButton();
	middleLayout->addWidget( btnNumbers, 1, 1 );
	btnAxes = new QPushButton();
	middleLayout->addWidget( btnAxes, 1, 2 );
	btnBackground3D = new QPushButton();
	middleLayout->addWidget( btnBackground3D, 1, 3 );

	groupBox3DFonts = new QGroupBox();
	QHBoxLayout * bottomLayout = new QHBoxLayout( groupBox3DFonts );

	btnTitleFnt = new QPushButton();
	bottomLayout->addWidget( btnTitleFnt );
	btnLabelsFnt = new QPushButton();
	bottomLayout->addWidget( btnLabelsFnt );
	btnNumFnt = new QPushButton();
	bottomLayout->addWidget( btnNumFnt );

	QVBoxLayout * plots3DPageLayout = new QVBoxLayout( plots3D );
	plots3DPageLayout->addWidget(topBox);
	plots3DPageLayout->addWidget(groupBox3DCol);
	plots3DPageLayout->addWidget(groupBox3DFonts);
	plots3DPageLayout->addStretch();

	connect( btnAxes, SIGNAL( clicked() ), this, SLOT(pick3DAxesColor() ) );
	connect( btnLabels, SIGNAL( clicked() ), this, SLOT(pick3DLabelsColor() ) );
	connect( btnNumbers, SIGNAL( clicked() ), this, SLOT(pick3DNumbersColor() ) );
	connect( btnBackground3D, SIGNAL( clicked() ), this, SLOT(pick3DBackgroundColor()));
	connect( btnFromColor, SIGNAL( clicked() ), this, SLOT(pickDataMinColor() ) );
	connect( btnToColor, SIGNAL( clicked() ), this, SLOT(pickDataMaxColor() ) );
	connect( btnGrid, SIGNAL( clicked() ), this, SLOT(pickGridColor() ) );
	connect( btnMesh, SIGNAL( clicked() ), this, SLOT(pickMeshColor() ) );

	connect( btnNumFnt, SIGNAL( clicked() ), this, SLOT(pick3DNumbersFont() ) );
	connect( btnTitleFnt, SIGNAL( clicked() ), this, SLOT(pick3DTitleFont() ) );
	connect( btnLabelsFnt, SIGNAL( clicked() ), this, SLOT(pick3DAxesFont() ) );
}

void PreferencesDialog::initAppPage()
{
	ApplicationWindow *app = (ApplicationWindow *)parentWidget();

	appTabWidget = new QTabWidget(generalDialog);

	application = new QWidget();
	QVBoxLayout * applicationLayout = new QVBoxLayout( application );
	QGroupBox * groupBoxApp = new QGroupBox();
	applicationLayout->addWidget(groupBoxApp);
	QGridLayout * topBoxLayout = new QGridLayout( groupBoxApp );

	lblLanguage = new QLabel();
	topBoxLayout->addWidget( lblLanguage, 0, 0 );
	boxLanguage = new QComboBox();
	insertLanguagesList();
	topBoxLayout->addWidget( boxLanguage, 0, 1 );

	lblStyle = new QLabel();
	topBoxLayout->addWidget( lblStyle, 1, 0 );
	boxStyle = new QComboBox();
	topBoxLayout->addWidget( boxStyle, 1, 1 );
	QStringList styles = QStyleFactory::keys();
	styles.sort();
	boxStyle->addItems(styles);
	boxStyle->setCurrentIndex(boxStyle->findText(app->appStyle,Qt::MatchWildcard));

	lblFonts = new QLabel();
	topBoxLayout->addWidget( lblFonts, 2, 0 );
	fontsBtn= new QPushButton();
	topBoxLayout->addWidget( fontsBtn, 2, 1 );

	lblScriptingLanguage = new QLabel();
	topBoxLayout->addWidget( lblScriptingLanguage, 3, 0 );
	boxScriptingLanguage = new QComboBox();
	QStringList llist = AbstractScriptingEngine::engineNames();
	boxScriptingLanguage->insertStringList(llist);
	boxScriptingLanguage->setCurrentItem(llist.indexOf(app->defaultScriptingLang));
	topBoxLayout->addWidget( boxScriptingLanguage, 3, 1 );

    boxSave= new QCheckBox();
	boxSave->setChecked(app->autoSave);
	topBoxLayout->addWidget( boxSave, 4, 0 );

	boxMinutes = new QSpinBox();
	boxMinutes->setRange(1,100);
	boxMinutes->setValue(app->autoSaveTime);
	boxMinutes->setEnabled(app->autoSave);
	topBoxLayout->addWidget( boxMinutes, 4, 1 );

	boxSearchUpdates = new QCheckBox();
	boxSearchUpdates->setChecked(app->autoSearchUpdates);
	topBoxLayout->addWidget( boxSearchUpdates, 5, 0, 1, 2 );

	topBoxLayout->setRowStretch( 6, 1 );

	appTabWidget->addTab( application, QString() );

	initConfirmationsPage();

	appTabWidget->addTab( confirm, QString() );

	appColors = new QWidget();
	QVBoxLayout * appColorsLayout = new QVBoxLayout( appColors );
	QGroupBox * groupBoxAppCol = new QGroupBox();
	appColorsLayout->addWidget( groupBoxAppCol );
	QGridLayout * colorsBoxLayout = new QGridLayout( groupBoxAppCol );

	lblWorkspace = new QLabel();
	colorsBoxLayout->addWidget( lblWorkspace, 0, 0 );
	btnWorkspace = new ColorButton();
	btnWorkspace->setColor(app->workspaceColor);
	colorsBoxLayout->addWidget( btnWorkspace, 0, 1 );

	lblPanels = new QLabel();
	colorsBoxLayout->addWidget( lblPanels, 1, 0 );
	btnPanels = new ColorButton();
	colorsBoxLayout->addWidget( btnPanels, 1, 1 );
	btnPanels->setColor(app->panelsColor);

	lblPanelsText = new QLabel();
	colorsBoxLayout->addWidget( lblPanelsText, 2, 0 );
	btnPanelsText = new ColorButton();
	colorsBoxLayout->addWidget( btnPanelsText, 2, 1 );
	btnPanelsText->setColor(app->panelsTextColor);

	colorsBoxLayout->setRowStretch( 3, 1 );

	appTabWidget->addTab( appColors, QString() );

	numericFormatPage = new QWidget();
	QVBoxLayout *numLayout = new QVBoxLayout( numericFormatPage );
	QGroupBox *numericFormatBox = new QGroupBox();
	numLayout->addWidget( numericFormatBox );
	QGridLayout *numericFormatLayout = new QGridLayout( numericFormatBox );

	lblAppPrecision = new QLabel();
	numericFormatLayout->addWidget(lblAppPrecision, 0, 0);
	boxAppPrecision = new QSpinBox();
	boxAppPrecision->setRange(0, 16);
	boxAppPrecision->setValue(app->m_decimal_digits);
	numericFormatLayout->addWidget(boxAppPrecision, 0, 1);

    lblDecimalSeparator = new QLabel();
    numericFormatLayout->addWidget(lblDecimalSeparator, 1, 0 );
	boxDecimalSeparator = new QComboBox();

	numericFormatLayout->addWidget(boxDecimalSeparator, 1, 1);

	boxUseGroupSeparator = new QCheckBox();
	boxUseGroupSeparator->setChecked(!(QLocale().numberOptions() & QLocale::OmitGroupSeparator));

	numericFormatLayout->addWidget(boxUseGroupSeparator, 2, 0);

	boxSeparatorPreview = new QLabel();
	boxSeparatorPreview->setFrameStyle(QFrame::Panel | QFrame::Sunken);
	numericFormatLayout->addWidget(boxSeparatorPreview, 2, 1);

    boxUpdateSeparators = new QCheckBox();
    boxUpdateSeparators->setChecked(true);
    numericFormatLayout->addWidget(boxUpdateSeparators, 3, 0);
	numericFormatLayout->setRowStretch(4, 1);

	appTabWidget->addTab( numericFormatPage, QString() );

	connect( boxLanguage, SIGNAL( activated(int) ), this, SLOT( switchToLanguage(int) ) );
	connect( fontsBtn, SIGNAL( clicked() ), this, SLOT( pickApplicationFont() ) );
	connect( boxSave, SIGNAL( toggled(bool) ), boxMinutes, SLOT( setEnabled(bool) ) );
	connect( btnWorkspace, SIGNAL( clicked() ), this, SLOT( pickWorkspaceColor() ) );
	connect( btnPanels, SIGNAL( clicked() ), this, SLOT( pickPanelsColor() ) );
	connect( btnPanelsText, SIGNAL( clicked() ), this, SLOT( pickPanelsTextColor() ) );
	connect( boxUseGroupSeparator, SIGNAL( toggled(bool) ), this, SLOT(updateDecSepPreview()) );
	connect( boxDecimalSeparator, SIGNAL( currentIndexChanged(int) ), this, SLOT(updateDecSepPreview()) );
	connect( boxAppPrecision, SIGNAL( valueChanged(int) ), this, SLOT(updateDecSepPreview()) );
}

void PreferencesDialog::initFittingPage()
{
	ApplicationWindow *app = (ApplicationWindow *)parentWidget();
	fitPage = new QWidget();

	groupBoxFittingCurve = new QGroupBox();
	QGridLayout * fittingCurveLayout = new QGridLayout(groupBoxFittingCurve);
	fittingCurveLayout->setSpacing(5);

	generatePointsBtn = new QRadioButton();
	generatePointsBtn->setChecked(app->generateUniformFitPoints);
	fittingCurveLayout->addWidget(generatePointsBtn, 0, 0);

	lblPoints = new QLabel();
	fittingCurveLayout->addWidget(lblPoints, 0, 1);
	generatePointsBox = new QSpinBox();
	generatePointsBox->setRange(0, 1000000);
	generatePointsBox->setSingleStep(10);
	generatePointsBox->setValue(app->fitPoints);
	fittingCurveLayout->addWidget(generatePointsBox, 0, 2);

    linearFit2PointsBox = new QCheckBox();
    linearFit2PointsBox->setChecked(app->m_2_linear_fit_points);
    fittingCurveLayout->addWidget(linearFit2PointsBox, 0, 3);

	showPointsBox(!app->generateUniformFitPoints);
	
	samePointsBtn = new QRadioButton();
	samePointsBtn->setChecked(!app->generateUniformFitPoints);
	fittingCurveLayout->addWidget(samePointsBtn, 1, 0);

	groupBoxMultiPeak = new QGroupBox();
	groupBoxMultiPeak->setCheckable(true);
	groupBoxMultiPeak->setChecked(app->generatePeakCurves);

	QHBoxLayout * multiPeakLayout = new QHBoxLayout(groupBoxMultiPeak);

	lblPeaksColor = new QLabel();
	multiPeakLayout->addWidget(lblPeaksColor);
	boxPeaksColor = new ColorBox(0);
	boxPeaksColor->setCurrentItem(app->peakCurvesColor);
	multiPeakLayout->addWidget(boxPeaksColor);

	groupBoxFitParameters = new QGroupBox();
	QGridLayout * fitParamsLayout = new QGridLayout(groupBoxFitParameters);

	lblPrecision = new QLabel();
	fitParamsLayout->addWidget(lblPrecision, 0, 0);
	boxPrecision = new QSpinBox();
	fitParamsLayout->addWidget(boxPrecision, 0, 1);
	boxPrecision->setValue(app->fit_output_precision);

	logBox = new QCheckBox();
	logBox->setChecked(app->writeFitResultsToLog);
	fitParamsLayout->addWidget(logBox, 1, 0);

	plotLabelBox = new QCheckBox();
	plotLabelBox->setChecked(app->pasteFitResultsToPlot);
	fitParamsLayout->addWidget(plotLabelBox, 2, 0);

	scaleErrorsBox = new QCheckBox();
	fitParamsLayout->addWidget(scaleErrorsBox, 3, 0);
	scaleErrorsBox->setChecked(app->fit_scale_errors);

	QVBoxLayout* fitPageLayout = new QVBoxLayout(fitPage);
	fitPageLayout->addWidget(groupBoxFittingCurve);
	fitPageLayout->addWidget(groupBoxMultiPeak);
	fitPageLayout->addWidget(groupBoxFitParameters);
	fitPageLayout->addStretch();

	connect(samePointsBtn, SIGNAL(toggled(bool)), this, SLOT(showPointsBox(bool)));
	connect(generatePointsBtn, SIGNAL(toggled(bool)), this, SLOT(showPointsBox(bool)));
}


void PreferencesDialog::initCurvesPage()
{
	ApplicationWindow *app = (ApplicationWindow *)parentWidget();

	curves = new QWidget();

	QGroupBox * curvesGroupBox = new QGroupBox();
	QGridLayout * curvesBoxLayout = new QGridLayout( curvesGroupBox );

	lblCurveStyle = new QLabel();
	curvesBoxLayout->addWidget( lblCurveStyle, 0, 0 );
	boxCurveStyle = new QComboBox();
	curvesBoxLayout->addWidget( boxCurveStyle, 0, 1 );

	lblLineWidth = new QLabel();
	curvesBoxLayout->addWidget( lblLineWidth, 1, 0 );
	boxCurveLineWidth = new QSpinBox();
	boxCurveLineWidth->setRange(1,100);
	boxCurveLineWidth->setValue(app->defaultCurveLineWidth);
	curvesBoxLayout->addWidget( boxCurveLineWidth, 1, 1 );

	lblSymbSize = new QLabel();
	curvesBoxLayout->addWidget( lblSymbSize, 2, 0 );
	boxSymbolSize = new QSpinBox();
	boxSymbolSize->setRange(1,100);
	boxSymbolSize->setValue(app->defaultSymbolSize/2);
	curvesBoxLayout->addWidget( boxSymbolSize, 2, 1 );

	curvesBoxLayout->setRowStretch( 3, 1 );

	QHBoxLayout * curvesPageLayout = new QHBoxLayout( curves );
	curvesPageLayout->addWidget( curvesGroupBox );
}

void PreferencesDialog::initConfirmationsPage()
{
	ApplicationWindow *app = (ApplicationWindow *)parentWidget();
	confirm = new QWidget();

	groupBoxConfirm = new QGroupBox();
	QVBoxLayout * layout = new QVBoxLayout( groupBoxConfirm );

	boxFolders = new QCheckBox();
	boxFolders->setChecked(app->confirmCloseFolder);
	layout->addWidget( boxFolders );

	boxTables = new QCheckBox();
	boxTables->setChecked(app->confirmCloseTable);
	layout->addWidget( boxTables );

	boxMatrices = new QCheckBox();
	boxMatrices->setChecked(app->confirmCloseMatrix);
	layout->addWidget( boxMatrices );

	boxPlots2D = new QCheckBox();
	boxPlots2D->setChecked(app->confirmClosePlot2D);
	layout->addWidget( boxPlots2D );

	boxPlots3D = new QCheckBox();
	boxPlots3D->setChecked(app->confirmClosePlot3D);
	layout->addWidget( boxPlots3D );

	boxNotes = new QCheckBox();
	boxNotes->setChecked(app->confirmCloseNotes);
	layout->addWidget( boxNotes );

	layout->addStretch();

	QHBoxLayout * confirmPageLayout = new QHBoxLayout( confirm );
	confirmPageLayout->addWidget(groupBoxConfirm);
}

void PreferencesDialog::languageChange()
{
	setWindowTitle( tr( "Preferences" ) );
	ApplicationWindow *app = (ApplicationWindow *)parentWidget();

	// pages list
	itemsList->clear();
	itemsList->addItem( tr( "General" ) );
	itemsList->addItem( tr( "Tables" ) );
	itemsList->addItem( tr( "2D Plots" ) );
	itemsList->addItem( tr( "3D Plots" ) );
	itemsList->addItem( tr( "Fitting" ) );
	itemsList->setCurrentRow(0);
	itemsList->item(0)->setIcon(QIcon(QPixmap(":/general.xpm")));
	itemsList->item(1)->setIcon(QIcon(QPixmap(":/configTable.xpm")));
	itemsList->item(2)->setIcon(QIcon(QPixmap(":/config_curves.xpm")));
	itemsList->item(3)->setIcon(QIcon(QPixmap(":/3dplots.xpm")));
	itemsList->item(4)->setIcon(QIcon(QPixmap(":/fit.xpm")));
	itemsList->setIconSize(QSize(32,32));
	// calculate a sensible width for the items list
	// (default QListWidget size is 256 which looks too big)
	QFontMetrics fm(itemsList->font());
	int width = 32,i;
	for(i=0 ; i<itemsList->count() ; i++)
		if( fm.width(itemsList->item(i)->text()) > width)
			width = fm.width(itemsList->item(i)->text());
	itemsList->setMaximumWidth( itemsList->iconSize().width() + width + 50 );
	// resize the list to the maximum width
	itemsList->resize(itemsList->maximumWidth(),itemsList->height());

	//plots 2D page
	plotsTabWidget->setTabText(plotsTabWidget->indexOf(plotOptions), tr("Options"));
	plotsTabWidget->setTabText(plotsTabWidget->indexOf(curves), tr("Curves"));
	plotsTabWidget->setTabText(plotsTabWidget->indexOf(plotTicks), tr("Ticks"));
	plotsTabWidget->setTabText(plotsTabWidget->indexOf(plotFonts), tr("Fonts"));

	boxResize->setText(tr("Do not &resize layers when window size changes"));

	lblMinTicksLength->setText(tr("Length"));

	lblAxesLineWidth->setText(tr("Axes linewidth" ));
	lblMajTicksLength->setText(tr("Length" ));
	lblMajTicks->setText(tr("Major Ticks" ));
	lblMinTicks->setText(tr("Minor Ticks" ));

	lblMargin->setText(tr("Margin" ));
	labelFrameWidth->setText(tr("Frame width" ));
	boxBackbones->setText(tr("Axes &backbones"));
	boxFrame->setText(tr("Canvas Fra&me"));
	boxAllAxes->setText(tr("Sho&w all axes"));
	boxTitle->setText(tr("Show &Title"));
	boxScaleFonts->setText(tr("Scale &Fonts"));
	boxAutoscaling->setText(tr("Auto&scaling"));
	boxAntialiasing->setText(tr("Antia&liasing"));

	boxMajTicks->clear();
	boxMajTicks->addItem(tr("None"));
	boxMajTicks->addItem(tr("Out"));
	boxMajTicks->addItem(tr("In & Out"));
	boxMajTicks->addItem(tr("In"));

	boxMinTicks->clear();
	boxMinTicks->addItem(tr("None"));
	boxMinTicks->addItem(tr("Out"));
	boxMinTicks->addItem(tr("In & Out"));
	boxMinTicks->addItem(tr("In"));

	boxMajTicks->setCurrentIndex(app->majTicksStyle);
	boxMinTicks->setCurrentIndex(app->minTicksStyle);

	plotsTabWidget->setTabText(plotsTabWidget->indexOf(plotPrint), tr("Print"));
	boxPrintCropmarks->setText(tr("Print Crop &Marks"));
	boxScaleLayersOnPrint->setText(tr("&Scale layers to paper size"));

	//confirmations page
	groupBoxConfirm->setTitle(tr("Prompt on closing"));
	boxFolders->setText(tr("Folders"));
	boxTables->setText(tr("Tables"));
	boxPlots3D->setText(tr("3D Plots"));
	boxPlots2D->setText(tr("2D Plots"));
	boxMatrices->setText(tr("Matrices"));
	boxNotes->setText(tr("&Notes"));

	buttonOk->setText( tr( "&OK" ) );
	buttonCancel->setText( tr( "&Cancel" ) );
	buttonApply->setText( tr( "&Apply" ) );
	buttonTextFont->setText( tr( "&Text Font" ) );
	buttonHeaderFont->setText( tr( "&Labels Font" ) );
	buttonAxesFont->setText( tr( "A&xes Labels" ) );
	buttonNumbersFont->setText( tr( "Axes &Numbers" ) );
	buttonLegendFont->setText( tr( "&Legend" ) );
	buttonTitleFont->setText( tr( "T&itle" ) );

	//application page
	appTabWidget->setTabText(appTabWidget->indexOf(application), tr("Application"));
	appTabWidget->setTabText(appTabWidget->indexOf(confirm), tr("Confirmations"));
	appTabWidget->setTabText(appTabWidget->indexOf(appColors), tr("Colors"));
	appTabWidget->setTabText(appTabWidget->indexOf(numericFormatPage), tr("Numeric Format"));

	lblLanguage->setText(tr("Language"));
	lblStyle->setText(tr("Style"));
	lblFonts->setText(tr("Main Font"));
	fontsBtn->setText(tr("Choose &font"));
	lblWorkspace->setText(tr("Workspace"));
	lblPanelsText->setText(tr("Panels text"));
	lblPanels->setText(tr("Panels"));
	boxSave->setText(tr("Save every"));
	boxSearchUpdates->setText(tr("Check for new versions at startup"));
	boxMinutes->setSuffix(tr(" minutes"));
	lblScriptingLanguage->setText(tr("Default scripting language"));

    boxUpdateSeparators->setText(tr("Update separators in Tables/Matrices"));
    boxUseGroupSeparator->setText(tr("Use group separator","option: use separator every 3 digits"));
	lblAppPrecision->setText(tr("Number of Decimal Digits"));
	lblDecimalSeparator->setText(tr("Decimal Separators"));
	boxDecimalSeparator->clear();
	boxDecimalSeparator->addItem(tr("default") + " (" + QLocale::system().toString(1000.0, 'f', 1) +")");
	boxDecimalSeparator->addItem(QLocale::c().toString(1000.0, 'f', 1));
	boxDecimalSeparator->addItem(QLocale(QLocale::German).toString(1000.0, 'f', 1));
	boxDecimalSeparator->addItem(QLocale(QLocale::French).toString(1000.0, 'f', 1));

    if (QLocale().name() == QLocale::c().name())
        boxDecimalSeparator->setCurrentIndex(1);
    else if (QLocale().name() == QLocale(QLocale::German).name())
        boxDecimalSeparator->setCurrentIndex(2);
    else if (QLocale().name() == QLocale(QLocale::French).name())
        boxDecimalSeparator->setCurrentIndex(3);

	boxSeparatorPreview->setText(tr("Preview:","preview of the decimal separator") + " " + QLocale().toString(1000.1234567890123456, 'f', boxAppPrecision->value()) );

	//tables page
	boxTableComments->setText(tr("&Display Comments in Header"));
	groupBoxTableCol->setTitle(tr("Colors"));
	lblSeparator->setText(tr("Default Column Separator"));
	boxSeparator->clear();
	boxSeparator->addItem(tr("TAB"));
	boxSeparator->addItem(tr("SPACE"));
	boxSeparator->addItem(";" + tr("TAB"));
	boxSeparator->addItem("," + tr("TAB"));
	boxSeparator->addItem(";" + tr("SPACE"));
	boxSeparator->addItem("," + tr("SPACE"));
	boxSeparator->addItem(";");
	boxSeparator->addItem(",");
	setColumnSeparator(app->columnSeparator);

	lblTableBackground->setText(tr( "Background" ));
	lblTextColor->setText(tr( "Text" ));
	lblHeaderColor->setText(tr("Labels"));
	groupBoxTableFonts->setTitle(tr("Fonts"));

	//curves page
	lblCurveStyle->setText(tr( "Default curve style" ));
	lblLineWidth->setText(tr( "Line width" ));
	lblSymbSize->setText(tr( "Symbol size" ));

	boxCurveStyle->clear();
	boxCurveStyle->addItem( QPixmap(":/lPlot.xpm"), tr( " Line" ) );
	boxCurveStyle->addItem( QPixmap(":/pPlot.xpm"), tr( " Scatter" ) );
	boxCurveStyle->addItem( QPixmap(":/lpPlot.xpm"), tr( " Line + Symbol" ) );
	boxCurveStyle->addItem( QPixmap(":/dropLines.xpm"), tr( " Vertical drop lines" ) );
	boxCurveStyle->addItem( QPixmap(":/spline.xpm"), tr( " Spline" ) );
	boxCurveStyle->addItem( QPixmap(":/vert_steps.xpm"), tr( " Vertical steps" ) );
	boxCurveStyle->addItem( QPixmap(":/hor_steps.xpm"), tr( " Horizontal steps" ) );
	boxCurveStyle->addItem( QPixmap(":/area.xpm"), tr( " Area" ) );
	boxCurveStyle->addItem( QPixmap(":/vertBars.xpm"), tr( " Vertical Bars" ) );
	boxCurveStyle->addItem( QPixmap(":/hBars.xpm"), tr( " Horizontal Bars" ) );

	int style = app->defaultCurveStyle;
	if (style == Layer::Line)
		boxCurveStyle->setCurrentItem(0);
	else if (style == Layer::Scatter)
		boxCurveStyle->setCurrentItem(1);
	else if (style == Layer::LineSymbols)
		boxCurveStyle->setCurrentItem(2);
	else if (style == Layer::VerticalDropLines)
		boxCurveStyle->setCurrentItem(3);
	else if (style == Layer::Spline)
		boxCurveStyle->setCurrentItem(4);
	else if (style == Layer::VerticalSteps)
		boxCurveStyle->setCurrentItem(5);
	else if (style == Layer::HorizontalSteps)
		boxCurveStyle->setCurrentItem(6);
	else if (style == Layer::Area)
		boxCurveStyle->setCurrentItem(7);
	else if (style == Layer::VerticalBars)
		boxCurveStyle->setCurrentItem(8);
	else if (style == Layer::HorizontalBars)
		boxCurveStyle->setCurrentItem(9);

	//plots 3D
	lblResolution->setText(tr("Resolution"));
	boxResolution->setSpecialValueText( "1 " + tr("(all data shown)") );
	boxShowLegend->setText(tr( "&Show Legend" ));
	boxShowProjection->setText(tr( "Show &Projection" ));
	btnFromColor->setText( tr( "&Data Max" ) );
	boxSmoothMesh->setText(tr( "Smoot&h Line" ));
	boxOrthogonal->setText(tr( "O&rthogonal" ));
	btnLabels->setText( tr( "Lab&els" ) );
	btnMesh->setText( tr( "Mesh &Line" ) );
	btnGrid->setText( tr( "&Grid" ) );
	btnToColor->setText( tr( "Data &Min" ) );
	btnNumbers->setText( tr( "&Numbers" ) );
	btnAxes->setText( tr( "A&xes" ) );
	btnBackground3D->setText( tr( "&Background" ) );
	groupBox3DCol->setTitle(tr("Colors" ));
	groupBox3DFonts->setTitle(tr("Fonts" ));
	btnTitleFnt->setText( tr( "&Title" ) );
	btnLabelsFnt->setText( tr( "&Axes Labels" ) );
	btnNumFnt->setText( tr( "&Numbers" ) );
	boxAutoscale3DPlots->setText( tr( "Autosca&ling" ) );

	//Fitting page
	groupBoxFittingCurve->setTitle(tr("Generated Fit Curve"));
	generatePointsBtn->setText(tr("Uniform X Function"));
	lblPoints->setText( tr("Points") );
	samePointsBtn->setText( tr( "Same X as Fitting Data" ) );
	linearFit2PointsBox->setText( tr( "2 points for linear fits" ) );

	groupBoxMultiPeak->setTitle(tr("Display Peak Curves for Multi-peak Fits"));

	groupBoxFitParameters->setTitle(tr("Parameters Output"));
	lblPrecision->setText(tr("Significant Digits"));
	logBox->setText(tr("Write Parameters to Result Log"));
	plotLabelBox->setText(tr("Paste Parameters to Plot"));
	scaleErrorsBox->setText(tr("Scale Errors with sqrt(Chi^2/doF)"));
	groupBoxMultiPeak->setTitle(tr("Display Peak Curves for Multi-peak Fits"));
	lblPeaksColor->setText(tr("Peaks Color"));
}

void PreferencesDialog::accept()
{
	apply();
	close();
}

void PreferencesDialog::apply()
{
	ApplicationWindow *app = (ApplicationWindow *)parentWidget();
	if (!app)
		return;

	// tables page
	QString sep = boxSeparator->currentText();
	sep.replace(tr("TAB"), "\t", false);
	sep.replace("\\t", "\t");
	sep.replace(tr("SPACE"), " ");
	sep.replace("\\s", " ");

	if (sep.contains(QRegExp("[0-9.eE+-]"))!=0)
	{
		QMessageBox::warning(0, tr("Import options error"),
				tr("The separator must not contain the following characters: 0-9eE.+-"));
		return;
	}

	app->columnSeparator = sep;
	app->customizeTables(buttonBackground->color(), buttonText->color(),
			buttonHeader->color(), textFont, headerFont, boxTableComments->isChecked());
	// 2D plots page: options tab
	app->titleOn=boxTitle->isChecked();
	app->allAxesOn = boxAllAxes->isChecked();
	app->canvasFrameOn=boxFrame->isChecked();
	app->canvasFrameWidth = boxFrameWidth->value();
	app->drawBackbones = boxBackbones->isChecked();
	app->axesLineWidth = boxLineWidth->value();
	app->defaultPlotMargin = boxMargin->value();
	app->setGraphDefaultSettings(boxAutoscaling->isChecked(),boxScaleFonts->isChecked(),
								boxResize->isChecked(), boxAntialiasing->isChecked());
	// 2D plots page: curves tab
	app->defaultCurveStyle = curveStyle();
	app->defaultCurveLineWidth = boxCurveLineWidth->value();
	app->defaultSymbolSize = 2*boxSymbolSize->value() + 1;
	// 2D plots page: ticks tab
	app->majTicksLength = boxMajTicksLength->value();
	app->minTicksLength = boxMinTicksLength->value();
	app->majTicksStyle = boxMajTicks->currentItem();
	app->minTicksStyle = boxMinTicks->currentItem();
	// 2D plots page: fonts tab
	app->plotAxesFont=axesFont;
	app->plotNumbersFont=numbersFont;
	app->plotLegendFont=legendFont;
	app->plotTitleFont=titleFont;
	// 2D plots page: print tab
	app->m_print_cropmarks = boxPrintCropmarks->isChecked();
	app->m_scale_plots_on_print = boxScaleLayersOnPrint->isChecked();
	QWidgetList *windows = app->windowsList();
	foreach(QWidget *w, *windows)
	{
		if (w->inherits("Graph"))
		{
			((Graph*)w)->setScaleLayersOnPrint(boxScaleLayersOnPrint->isChecked());
			((Graph*)w)->printCropmarks(boxPrintCropmarks->isChecked());
		}
	}
	delete windows;
	// general page: application tab
	app->changeAppFont(appFont);
	setFont(appFont);
	app->changeAppStyle(boxStyle->currentText());
	app->autoSearchUpdates = boxSearchUpdates->isChecked();
	app->setSaveSettings(boxSave->isChecked(), boxMinutes->value());
	app->defaultScriptingLang = boxScriptingLanguage->currentText();

	// general page: numeric format tab
	app->m_decimal_digits = boxAppPrecision->value();
    QLocale locale;
    switch (boxDecimalSeparator->currentIndex())
	{
        case 0:
            locale = QLocale::system();
        break;
        case 1:
            locale = QLocale::c();
        break;
        case 2:
            locale = QLocale(QLocale::German);
        break;
        case 3:
            locale = QLocale(QLocale::French);
        break;
    }

    if(boxUseGroupSeparator->isChecked())
		locale.setNumberOptions(locale.numberOptions() & ~QLocale::OmitGroupSeparator);
	else
		locale.setNumberOptions(locale.numberOptions() | QLocale::OmitGroupSeparator);

    if (QLocale() != locale){
        QLocale::setDefault(locale);

        QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
        if (boxUpdateSeparators->isChecked()){
            QList<QWidget*> *lst = app->windowsList();
            foreach(QWidget *w, *lst){
					// TODO
					/*
                if(w->inherits("Table"))
                    ((Table *)w)->updateDecimalSeparators();
                else */ if(w->inherits("Matrix"))
                    ((Matrix *)w)->updateDecimalSeparators();
            }
            delete lst;
        }
        QApplication::restoreOverrideCursor();
    }

	// general page: confirmations tab
	app->confirmCloseFolder = boxFolders->isChecked();
	app->updateConfirmOptions(boxTables->isChecked(), boxMatrices->isChecked(),
			boxPlots2D->isChecked(), boxPlots3D->isChecked(),
			boxNotes->isChecked());
	// general page: colors tab
	app->setAppColors(btnWorkspace->color(), btnPanels->color(), btnPanelsText->color());
	// 3D plots page
	app->plot3DColors = plot3DColors;
	app->showPlot3DLegend = boxShowLegend->isChecked();
	app->showPlot3DProjection = boxShowProjection->isChecked();
	app->plot3DResolution = boxResolution->value();
	app->plot3DTitleFont = plot3DTitleFont;
	app->plot3DNumbersFont = plot3DNumbersFont;
	app->plot3DAxesFont = plot3DAxesFont;
	app->orthogonal3DPlots = boxOrthogonal->isChecked();
	app->smooth3DMesh = boxSmoothMesh->isChecked();
	app->autoscale3DPlots = boxAutoscale3DPlots->isChecked();
	app->setPlot3DOptions();

	// fitting page
	app->fit_output_precision = boxPrecision->value();
	app->pasteFitResultsToPlot = plotLabelBox->isChecked();
	app->writeFitResultsToLog = logBox->isChecked();
	app->fitPoints = generatePointsBox->value();
	app->generateUniformFitPoints = generatePointsBtn->isChecked();
	app->generatePeakCurves = groupBoxMultiPeak->isChecked();
	app->peakCurvesColor = boxPeaksColor->currentIndex();
	app->fit_scale_errors = scaleErrorsBox->isChecked();
	app->m_2_linear_fit_points = linearFit2PointsBox->isChecked();
	app->saveSettings();

	// calculate a sensible width for the items list
	// (default QListWidget size is 256 which looks too big)
	QFontMetrics fm(itemsList->font());
	int width = 32,i;
	for(i=0 ; i<itemsList->count() ; i++)
		if( fm.width(itemsList->item(i)->text()) > width)
			width = fm.width(itemsList->item(i)->text());
	itemsList->setMaximumWidth( itemsList->iconSize().width() + width + 50 );
	// resize the list to the maximum width
	itemsList->resize(itemsList->maximumWidth(),itemsList->height());
}

int PreferencesDialog::curveStyle()
{
	int style = 0;
	switch (boxCurveStyle->currentItem())
	{
		case 0:
			style = Layer::Line;
			break;
		case 1:
			style = Layer::Scatter;
			break;
		case 2:
			style = Layer::LineSymbols;
			break;
		case 3:
			style = Layer::VerticalDropLines;
			break;
		case 4:
			style = Layer::Spline;
			break;
		case 5:
			style = Layer::VerticalSteps;
			break;
		case 6:
			style = Layer::HorizontalSteps;
			break;
		case 7:
			style = Layer::Area;
			break;
		case 8:
			style = Layer::VerticalBars;
			break;
		case 9:
			style = Layer::HorizontalBars;
			break;
	}
	return style;
}

void PreferencesDialog::pickBgColor()
{
	QColor c = QColorDialog::getColor(buttonBackground->color(), this);
	if ( !c.isValid() || c == buttonBackground->color())
		return;

	buttonBackground->setColor(c);
}

void PreferencesDialog::pickTextColor()
{
	QColor c = QColorDialog::getColor(buttonText->color(), this);
	if ( !c.isValid() || c == buttonText->color())
		return;

	buttonText->setColor(c);
}

void PreferencesDialog::pickHeaderColor()
{
	QColor c = QColorDialog::getColor(buttonHeader->color(), this);
	if ( !c.isValid() || c == buttonHeader->color())
		return;

	buttonHeader->setColor(c);
}

void PreferencesDialog::pickTextFont()
{
	bool ok;
	QFont font = QFontDialog::getFont(&ok,textFont,this);
	if ( ok ) {
		textFont = font;
	} else {
		return;
	}
}

void PreferencesDialog::pickHeaderFont()
{
	bool ok;
	QFont font = QFontDialog::getFont(&ok,headerFont,this);
	if ( ok ) {
		headerFont = font;
	} else {
		return;
	}
}

void PreferencesDialog::pickLegendFont()
{
	bool ok;
	QFont font = QFontDialog::getFont(&ok,legendFont,this);
	if ( ok ) {
		legendFont = font;
	} else {
		return;
	}
}

void PreferencesDialog::pickAxesFont()
{
	bool ok;
	QFont font = QFontDialog::getFont(&ok,axesFont,this);
	if ( ok ) {
		axesFont = font;
	} else {
		return;
	}
}

void PreferencesDialog::pickNumbersFont()
{
	bool ok;
	QFont font = QFontDialog::getFont(&ok,numbersFont,this);
	if ( ok ) {
		numbersFont = font;
	} else {
		return;
	}
}

void PreferencesDialog::pickTitleFont()
{
	bool ok;
	QFont font = QFontDialog::getFont(&ok,titleFont,this);
	if ( ok )
		titleFont = font;
	else
		return;
}

void PreferencesDialog::pickApplicationFont()
{
	bool ok;
	QFont font = QFontDialog::getFont(&ok,appFont,this);
	if ( ok )
		appFont = font;
	else
		return;
	fontsBtn->setFont(appFont);
}

void PreferencesDialog::pickPanelsTextColor()
{
	QColor c = QColorDialog::getColor(btnPanelsText->color(), this);
	if ( !c.isValid() || c == btnPanelsText->color())
		return;

	btnPanelsText->setColor(c);
}

void PreferencesDialog::pickPanelsColor()
{
	QColor c = QColorDialog::getColor(btnPanels->color(), this);
	if ( !c.isValid() || c == btnPanels->color())
		return;

	btnPanels->setColor(c);
}

void PreferencesDialog::pickWorkspaceColor()
{
	QColor c = QColorDialog::getColor(btnWorkspace->color(), this);
	if ( !c.isValid() || c == btnWorkspace->color())
		return;

	btnWorkspace->setColor(c);
}

void PreferencesDialog::pickDataMaxColor()
{
	QColor c = QColorDialog::getColor(QColor(plot3DColors[0]), this );
	if ( !c.isValid() )
		return;

	plot3DColors[0] = c.name();
}

void PreferencesDialog::pickDataMinColor()
{
	QColor c = QColorDialog::getColor(QColor(plot3DColors[4]), this );
	if ( !c.isValid() )
		return;

	plot3DColors[4] = c.name();
}

void PreferencesDialog::pick3DBackgroundColor()
{
	QColor c = QColorDialog::getColor(QColor(plot3DColors[7]), this );
	if ( !c.isValid() )
		return;

	plot3DColors[7] = c.name();
}

void PreferencesDialog::pickMeshColor()
{
	QColor c = QColorDialog::getColor(QColor(plot3DColors[2]), this );
	if ( !c.isValid() )
		return;

	plot3DColors[2] = c.name();
}

void PreferencesDialog::pickGridColor()
{
	QColor c = QColorDialog::getColor(QColor(plot3DColors[3]), this );
	if ( !c.isValid() )
		return;

	plot3DColors[3] = c.name();
}

void PreferencesDialog::pick3DAxesColor()
{
	QColor c = QColorDialog::getColor(QColor(plot3DColors[6]), this );
	if ( !c.isValid() )
		return;

	plot3DColors[6] = c.name();
}

void PreferencesDialog::pick3DNumbersColor()
{
	QColor c = QColorDialog::getColor(QColor(plot3DColors[5]), this );
	if ( !c.isValid() )
		return;

	plot3DColors[5] = c.name();
}

void PreferencesDialog::pick3DLabelsColor()
{
	QColor c = QColorDialog::getColor(QColor(plot3DColors[1]), this );
	if ( !c.isValid() )
		return;

	plot3DColors[1] = c.name();
}

void PreferencesDialog::pick3DTitleFont()
{
	bool ok;
	QFont font = QFontDialog::getFont(&ok, plot3DTitleFont,this);
	if ( ok )
		plot3DTitleFont = font;
	else
		return;
}

void PreferencesDialog::pick3DNumbersFont()
{
	bool ok;
	QFont font = QFontDialog::getFont(&ok, plot3DNumbersFont,this);
	if ( ok )
		plot3DNumbersFont = font;
	else
		return;
}

void PreferencesDialog::pick3DAxesFont()
{
	bool ok;
	QFont font = QFontDialog::getFont(&ok, plot3DAxesFont,this);
	if ( ok )
		plot3DAxesFont = font;
	else
		return;
}

void PreferencesDialog::setColumnSeparator(const QString& sep)
{
	if (sep=="\t")
		boxSeparator->setCurrentIndex(0);
	else if (sep==" ")
		boxSeparator->setCurrentIndex(1);
	else if (sep==";\t")
		boxSeparator->setCurrentIndex(2);
	else if (sep==",\t")
		boxSeparator->setCurrentIndex(3);
	else if (sep=="; ")
		boxSeparator->setCurrentIndex(4);
	else if (sep==", ")
		boxSeparator->setCurrentIndex(5);
	else if (sep==";")
		boxSeparator->setCurrentIndex(6);
	else if (sep==",")
		boxSeparator->setCurrentIndex(7);
	else
	{
		QString separator = sep;
		boxSeparator->setEditText(separator.replace(" ","\\s").replace("\t","\\t"));
	}
}

void PreferencesDialog::switchToLanguage(int param)
{
	ApplicationWindow *app = (ApplicationWindow *)parentWidget();
	app->switchToLanguage(param);
	languageChange();
}

void PreferencesDialog::insertLanguagesList()
{
	ApplicationWindow *app = (ApplicationWindow *)parentWidget();
	QString qmPath = qApp->applicationDirPath() + "/translations";
	QDir dir(qmPath);
	QStringList locales = app->locales;
	QStringList languages;
	int lang = 0;
	for (int i=0; i < (int)locales.size(); i++)
	{
		if (locales[i] == "en")
			languages.push_back("English");
		else
		{
			QTranslator translator;
			translator.load("scidavis_"+locales[i], qmPath);

			QString language = translator.translate("ApplicationWindow", "English", "translate this to the language of the translation file, NOT to the meaning of English!");
			if (!language.isEmpty())
				languages.push_back(language);
			else
				languages.push_back(locales[i]);
		}

		if (locales[i] == app->appLanguage)
			lang = i;
	}
	boxLanguage->addItems(languages);
	boxLanguage->setCurrentIndex(lang);
}


void PreferencesDialog::showPointsBox(bool)
{
	if (generatePointsBtn->isChecked())
	{
		lblPoints->show();
		generatePointsBox->show();
		linearFit2PointsBox->show();
	}
	else
	{
		lblPoints->hide();
		generatePointsBox->hide();
		linearFit2PointsBox->hide();
	}
}

void PreferencesDialog::updateDecSepPreview()
{
    QLocale locale;
    switch (boxDecimalSeparator->currentIndex())
	{
        case 0:
            locale = QLocale::system();
        break;
        case 1:
            locale = QLocale::c();
        break;
        case 2:
            locale = QLocale(QLocale::German);
        break;
        case 3:
            locale = QLocale(QLocale::French);
        break;
    }

    if(boxUseGroupSeparator->isChecked())
		locale.setNumberOptions(locale.numberOptions() & ~QLocale::OmitGroupSeparator);
	else
		locale.setNumberOptions(locale.numberOptions() | QLocale::OmitGroupSeparator);

	boxSeparatorPreview->setText(tr("Preview:","preview of the decimal separator") + " " + locale.toString(1000.1234567890123456, 'f', boxAppPrecision->value()) );
}
