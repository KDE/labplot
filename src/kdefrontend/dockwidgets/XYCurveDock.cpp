/***************************************************************************
    File                 : XYCurveDock.cpp
    Project              : LabPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2010-2011 Alexander Semke (alexander.semke*web.de)
                           (replace * with @ in the email addresses)
    Description          : widget for XYCurve properties
                           
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

#include "XYCurveDock.h"
#include "kdefrontend/GuiTools.h"
#include "worksheet/XYCurve.h"
#include "worksheet/Worksheet.h"
#include "core/AspectTreeModel.h"
#include "core/column/Column.h"
#include "core/plugin/PluginManager.h"
#include "worksheet/StandardCurveSymbolFactory.h"
#include "widgets/TreeViewComboBox.h"

#include "core/AbstractFilter.h"
#include "core/datatypes/SimpleCopyThroughFilter.h"
#include "core/datatypes/Double2StringFilter.h"
#include "core/datatypes/String2DoubleFilter.h"
#include "core/datatypes/DateTime2StringFilter.h"
#include "core/datatypes/String2DateTimeFilter.h"

#include <QTextEdit>
#include <QCheckBox>
#include <QPainter>
#include <QBrush>
#include <QPen>
#include <QDebug>

/*!
  \class GuiObserver
  \brief  Provides a widget for editing the properties of the XYCurves (2D-curves) currently selected in the project explorer.
  
  If more then one curves are set, the properties of the first column are shown. The changes of the properties are applied to all curves.
  The exclusions are the name, the comment and the datasets (columns) of the curves  - these properties can only be changed if there is only one single curve.

  \ingroup kdefrontend
*/

XYCurveDock::XYCurveDock(QWidget *parent): QWidget(parent){
	ui.setupUi(this);
	
	ui.tbLoad->setIcon(KIcon("document-open"));
	ui.tbSave->setIcon(KIcon("document-save"));
	ui.tbSaveDefault->setIcon(KIcon("document-save-as"));
	ui.tbCopy->setIcon(KIcon("edit-copy"));
	ui.tbPaste->setIcon(KIcon("edit-paste"));

	// Tab "General"
	gridLayout = new QGridLayout(ui.tabGeneral);
	gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
	lName = new QLabel(ui.tabGeneral);
	lName->setObjectName(QString::fromUtf8("lName"));
	QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
	sizePolicy.setHorizontalStretch(0);
	sizePolicy.setVerticalStretch(0);
	sizePolicy.setHeightForWidth(lName->sizePolicy().hasHeightForWidth());
	lName->setSizePolicy(sizePolicy);

	gridLayout->addWidget(lName, 0, 0, 1, 1);

	leName = new QLineEdit(ui.tabGeneral);
	leName->setObjectName(QString::fromUtf8("leName"));

	gridLayout->addWidget(leName, 0, 1, 1, 1);

	lComment = new QLabel(ui.tabGeneral);
	lComment->setObjectName(QString::fromUtf8("lComment"));
	sizePolicy.setHeightForWidth(lComment->sizePolicy().hasHeightForWidth());
	lComment->setSizePolicy(sizePolicy);

	gridLayout->addWidget(lComment, 1, 0, 1, 1);

	leComment = new QLineEdit(ui.tabGeneral);
	leComment->setObjectName(QString::fromUtf8("leComment"));
	QSizePolicy sizePolicy1(QSizePolicy::Expanding, QSizePolicy::Fixed);
	sizePolicy1.setHorizontalStretch(0);
	sizePolicy1.setVerticalStretch(0);
	sizePolicy1.setHeightForWidth(leComment->sizePolicy().hasHeightForWidth());
	leComment->setSizePolicy(sizePolicy1);
	leComment->setMaximumSize(QSize(16777215, 50));

	gridLayout->addWidget(leComment, 1, 1, 1, 1);
	
	chkVisible = new QCheckBox(ui.tabGeneral);
	gridLayout->addWidget(chkVisible, 2, 0, 1, 1);
	
	lXColumn= new QLabel(ui.tabGeneral);
	gridLayout->addWidget(lXColumn, 3, 0, 1, 1);
	
	cbXColumn = new TreeViewComboBox(ui.tabGeneral);
	gridLayout->addWidget(cbXColumn, 3, 1, 1, 1);
	
	lYColumn= new QLabel(ui.tabGeneral);
	gridLayout->addWidget(lYColumn, 4, 0, 1, 1);
	
	cbYColumn = new TreeViewComboBox(ui.tabGeneral);
	gridLayout->addWidget(cbYColumn, 4, 1, 1, 1);
	
	verticalSpacer = new QSpacerItem(24, 320, QSizePolicy::Minimum, QSizePolicy::Expanding);
	gridLayout->addItem(verticalSpacer, 5, 0, 1, 1);

	
	//Tab "Values"
	cbValuesColumn = new TreeViewComboBox(ui.tabValues);
	qobject_cast<QGridLayout*>(ui.tabValues->layout())->addWidget(cbValuesColumn, 2, 2, 1, 1);
	
	
	//adjust layouts in the tabs
	QGridLayout* layout;
	for (int i=0; i<ui.tabWidget->count(); ++i){
	  layout=static_cast<QGridLayout*>(ui.tabWidget->widget(i)->layout());
	  if (!layout)
		continue;
	  
	  layout->setContentsMargins(2,2,2,2);
	  layout->setHorizontalSpacing(2);
	  layout->setVerticalSpacing(2);
	}
	
	
	//Slots
	
	//General
	connect( leName, SIGNAL(returnPressed()), this, SLOT(nameChanged()) );
	connect( leComment, SIGNAL(returnPressed()), this, SLOT(commentChanged()) );
	connect( chkVisible, SIGNAL(stateChanged(int)), this, SLOT(visibilityChanged(int)) );
	connect( cbXColumn, SIGNAL(currentModelIndexChanged(const QModelIndex&)), this, SLOT(xColumnChanged(const QModelIndex&)) );
	connect( cbYColumn, SIGNAL(currentModelIndexChanged(const QModelIndex&)), this, SLOT(yColumnChanged(const QModelIndex&)) );
	
	//Lines
	connect( ui.cbLineType, SIGNAL(currentIndexChanged(int)), this, SLOT(lineTypeChanged(int)) );
	connect( ui.sbLineInterpolationPointsCount, SIGNAL(valueChanged(int)), this, SLOT(lineInterpolationPointsCountChanged(int)) );
	connect( ui.cbLineStyle, SIGNAL(currentIndexChanged(int)), this, SLOT(lineStyleChanged(int)) );
	connect( ui.kcbLineColor, SIGNAL(changed (const QColor &)), this, SLOT(lineColorChanged(const QColor&)) );
	connect( ui.sbLineWidth, SIGNAL(valueChanged(double)), this, SLOT(lineWidthChanged(double)) );
	connect( ui.sbLineOpacity, SIGNAL(valueChanged(int)), this, SLOT(lineOpacityChanged(int)) );

	connect( ui.cbDropLineType, SIGNAL(currentIndexChanged(int)), this, SLOT(dropLineTypeChanged(int)) );
	connect( ui.cbDropLineStyle, SIGNAL(currentIndexChanged(int)), this, SLOT(dropLineStyleChanged(int)) );
	connect( ui.kcbDropLineColor, SIGNAL(changed (const QColor &)), this, SLOT(dropLineColorChanged(const QColor&)) );
	connect( ui.sbDropLineWidth, SIGNAL(valueChanged(double)), this, SLOT(dropLineWidthChanged(double)) );
	connect( ui.sbDropLineOpacity, SIGNAL(valueChanged(int)), this, SLOT(dropLineOpacityChanged(int)) );
	
	//Symbol
	connect( ui.cbSymbolStyle, SIGNAL(currentIndexChanged(int)), this, SLOT(symbolStyleChanged(int)) );
	connect( ui.sbSymbolSize, SIGNAL(valueChanged(double)), this, SLOT(symbolSizeChanged(double)) );
	connect( ui.sbSymbolRotation, SIGNAL(valueChanged(int)), this, SLOT(symbolRotationChanged(int)) );
	connect( ui.sbSymbolOpacity, SIGNAL(valueChanged(int)), this, SLOT(symbolOpacityChanged(int)) );
	
	connect( ui.cbSymbolFillingStyle, SIGNAL(currentIndexChanged(int)), this, SLOT(symbolFillingStyleChanged(int)) );
	connect( ui.kcbSymbolFillingColor, SIGNAL(changed (const QColor &)), this, SLOT(symbolFillingColorChanged(const QColor)) );

	connect( ui.cbSymbolBorderStyle, SIGNAL(currentIndexChanged(int)), this, SLOT(symbolBorderStyleChanged(int)) );
	connect( ui.kcbSymbolBorderColor, SIGNAL(changed (const QColor &)), this, SLOT(symbolBorderColorChanged(const QColor&)) );
	connect( ui.sbSymbolBorderWidth, SIGNAL(valueChanged(double)), this, SLOT(symbolBorderWidthChanged(double)) );

	//Values
	connect( ui.cbValuesType, SIGNAL(currentIndexChanged(int)), this, SLOT(valuesTypeChanged(int)) );
	connect( cbValuesColumn, SIGNAL(currentIndexChanged(int)), this, SLOT(valuesColumnChanged(int)) );
	connect( ui.cbValuesPosition, SIGNAL(currentIndexChanged(int)), this, SLOT(valuesPositionChanged(int)) );
	connect( ui.sbValuesDistance, SIGNAL(valueChanged(double)), this, SLOT(valuesDistanceChanged(double)) );
	connect( ui.sbValuesRotation, SIGNAL(valueChanged(int)), this, SLOT(valuesRotationChanged(int)) );
	connect( ui.sbValuesOpacity, SIGNAL(valueChanged(int)), this, SLOT(valuesOpacityChanged(int)) );
	
	//TODO connect( ui.cbValuesFormat, SIGNAL(currentIndexChanged(int)), this, SLOT(valuesColumnFormatChanged(int)) );
	connect( ui.leValuesPrefix, SIGNAL(returnPressed()), this, SLOT(valuesPrefixChanged()) );
	connect( ui.leValuesSuffix, SIGNAL(returnPressed()), this, SLOT(valuesSuffixChanged()) );
	connect( ui.kfrValuesFont, SIGNAL(fontSelected(const QFont& )), this, SLOT(valuesFontChanged(const QFont&)) );
	connect( ui.kcbValuesFontColor, SIGNAL(changed (const QColor &)), this, SLOT(valuesFontColorChanged(const QColor&)) );
	
	connect( ui.tbLoad, SIGNAL(clicked()), this, SLOT(loadSettings()));
	connect( ui.tbSave, SIGNAL(clicked()), this, SLOT(saveSettings()));
	connect( ui.tbSaveDefault, SIGNAL(clicked()), this, SLOT(saveDefaults()));

	retranslateUi();
	
	QTimer::singleShot(0, this, SLOT(init()));
}

void XYCurveDock::init(){
  	dateStrings<<"yyyy-MM-dd";
	dateStrings<<"yyyy/MM/dd";
	dateStrings<<"dd/MM/yyyy"; 
	dateStrings<<"dd/MM/yy";
	dateStrings<<"dd.MM.yyyy";
	dateStrings<<"dd.MM.yy";
	dateStrings<<"MM/yyyy";
	dateStrings<<"dd.MM."; 
	dateStrings<<"yyyyMMdd";

	timeStrings<<"hh";
	timeStrings<<"hh ap";
	timeStrings<<"hh:mm";
	timeStrings<<"hh:mm ap";
	timeStrings<<"hh:mm:ss";
	timeStrings<<"hh:mm:ss.zzz";
	timeStrings<<"hh:mm:ss:zzz";
	timeStrings<<"mm:ss.zzz";
	timeStrings<<"hhmmss";
	
	
  	//Line
	ui.cbLineType->addItems(XYCurve::lineTypeStrings());
	QPainter pa;
	QPixmap pm( 20, 20 );
	ui.cbLineType->setIconSize( QSize(20,20) );

	QPen pen(Qt::SolidPattern, 0);
 	pa.setPen( pen );
	
	//no line
	pm.fill(Qt::transparent);
	pa.begin( &pm );
	pa.setRenderHint(QPainter::Antialiasing);
	pa.setBrush(Qt::SolidPattern);
	pa.drawEllipse( 1,1,4,4);
	pa.drawEllipse( 15,15,4,4);
	pa.end();
	ui.cbLineType->setItemIcon(0, pm);
	
	//line
	pm.fill(Qt::transparent);
	pa.begin( &pm );
	pa.setRenderHint(QPainter::Antialiasing);
	pa.setBrush(Qt::SolidPattern);
	pa.drawEllipse( 1,1,4,4);
	pa.drawEllipse( 15,15,4,4);
	pa.drawLine(3,3,17,17);
	pa.end();
	ui.cbLineType->setItemIcon(1, pm);
	
	pm.fill(Qt::transparent);
	pa.begin( &pm );
	pa.setRenderHint(QPainter::Antialiasing);
	pa.setBrush(Qt::SolidPattern);
	pa.drawEllipse( 1,1,4,4);
	pa.drawEllipse( 15,15,4,4);
	pa.drawLine(3,3,17,3);
	pa.drawLine(17,3,17,17);
	pa.end();
	ui.cbLineType->setItemIcon(2, pm);
	
	pm.fill(Qt::transparent);
	pa.begin( &pm );
	pa.setRenderHint(QPainter::Antialiasing);
	pa.setBrush(Qt::SolidPattern);
	pa.drawEllipse( 1,1,4,4);
	pa.drawEllipse( 15,15,4,4);
	pa.drawLine(3,3,3,17);
	pa.drawLine(3,17,17,17);
	pa.end();
	ui.cbLineType->setItemIcon(3, pm);
	
	//horizontal midpoint
	pm.fill(Qt::transparent);
	pa.begin( &pm );
	pa.setRenderHint(QPainter::Antialiasing);
	pa.setBrush(Qt::SolidPattern);
	pa.drawEllipse( 1,1,4,4);
	pa.drawEllipse( 15,15,4,4);
	pa.drawLine(3,3,10,3);
	pa.drawLine(10,3,10,17);
	pa.drawLine(10,17,17,17);
	pa.end();
	ui.cbLineType->setItemIcon(4, pm);
	
	//vertical midpoint
	pm.fill(Qt::transparent);
	pa.begin( &pm );
	pa.setRenderHint(QPainter::Antialiasing);
	pa.setBrush(Qt::SolidPattern);
	pa.drawEllipse( 1,1,4,4);
	pa.drawEllipse( 15,15,4,4);
	pa.drawLine(3,3,3,10);
	pa.drawLine(3,10,17,10);
	pa.drawLine(17,10,17,17);
	pa.end();
	ui.cbLineType->setItemIcon(5, pm);
	
	//2-segments
	pm.fill(Qt::transparent);
	pa.begin( &pm );
	pa.setRenderHint(QPainter::Antialiasing);
	pa.setBrush(Qt::SolidPattern);
	pa.drawEllipse( 1,1,4,4);
	pa.drawEllipse( 8,8,4,4);
	pa.drawEllipse( 15,15,4,4);
	pa.drawLine(3,3,10,10);
	pa.end();
	ui.cbLineType->setItemIcon(6, pm);
	
	//3-segments
	pm.fill(Qt::transparent);
	pa.begin( &pm );
	pa.setRenderHint(QPainter::Antialiasing);
	pa.setBrush(Qt::SolidPattern);
	pa.drawEllipse( 1,1,4,4);
	pa.drawEllipse( 8,8,4,4);
	pa.drawEllipse( 15,15,4,4);
	pa.drawLine(3,3,17,17);
	pa.end();
	ui.cbLineType->setItemIcon(7, pm);
	
	//natural spline
	pm.fill(Qt::transparent);
	pa.begin( &pm );
	pa.setRenderHint(QPainter::Antialiasing);
	pa.setBrush(Qt::SolidPattern);
	pa.drawEllipse( 1,1,4,4);
	pa.drawEllipse( 15,15,4,4);
	pa.rotate(45);
  	pa.drawArc(2*sqrt(2),-4,17*sqrt(2),20,30*16,120*16);
	
	pa.end();
	ui.cbLineType->setItemIcon(8, pm);
	ui.cbLineType->setItemIcon(9, pm);
	ui.cbLineType->setItemIcon(10, pm);
	ui.cbLineType->setItemIcon(11, pm);
	
	m_initializing = true;
	GuiTools::updatePenStyles(ui.cbLineStyle, Qt::black);
	
	//Drop lines
	ui.cbDropLineType->addItems(XYCurve::dropLineTypeStrings());
	GuiTools::updatePenStyles(ui.cbDropLineStyle, Qt::black);
	
	//Symbols
	GuiTools::updatePenStyles(ui.cbSymbolBorderStyle, Qt::black);
	this->fillSymbolStyles();
 	GuiTools::updateBrushStyles(ui.cbSymbolFillingStyle, Qt::black);
	m_initializing = false;
	
	//Values
	ui.cbValuesType->addItems(XYCurve::valuesTypeStrings());
	ui.cbValuesPosition->addItems(XYCurve::valuesPositionStrings());
}

void XYCurveDock::setModel(AspectTreeModel* model){
	QList<const char *>  list;
	list<<"Folder"<<"Spreadsheet"<<"FileDataSource"<<"Column";
	cbXColumn->setTopLevelClasses(list);
	cbYColumn->setTopLevelClasses(list);
	cbValuesColumn->setTopLevelClasses(list);
	
 	list.clear();
	list<<"Column";
	model->setSelectableAspects(list);
	
	m_initializing=true;
  	cbXColumn->setModel(model);
	cbYColumn->setModel(model);
	cbValuesColumn->setModel(model);
	
	m_aspectTreeModel=model;
	m_initializing=false;
}

/*!
  sets the curves. The properties of the curves in the list \c list can be edited in this widget.
*/
void XYCurveDock::setCurves(QList<XYCurve*> list){
  m_initializing=true;
  m_curvesList=list;
  XYCurve* curve=list.first();
  
  //if there are more then one curve in the list, disable the tab "general"
  if (list.size()==1){
	lName->setEnabled(true);
	leName->setEnabled(true);
	lComment->setEnabled(true);
	leComment->setEnabled(true);
	lXColumn->setEnabled(true);
	cbXColumn->setEnabled(true);
	lYColumn->setEnabled(true);
	cbYColumn->setEnabled(true);
	
	leName->setText(curve->name());
	leComment->setText(curve->comment());
  }else{
	lName->setEnabled(false);
	leName->setEnabled(false);
	lComment->setEnabled(false);
	leComment->setEnabled(false);
	lXColumn->setEnabled(false);
	cbXColumn->setEnabled(false);
	lYColumn->setEnabled(false);
	cbYColumn->setEnabled(false);	
	
	leName->setText("");
	leComment->setText("");
	cbXColumn->setCurrentModelIndex(QModelIndex());
	cbYColumn->setCurrentModelIndex(QModelIndex());
	cbValuesColumn->setCurrentModelIndex(QModelIndex());
  }
  
  //show the properties of the first curve
  
  //General-tab
  chkVisible->setChecked(curve->isVisible());
  cbXColumn->setCurrentModelIndex( m_aspectTreeModel->modelIndexOfAspect(curve->xColumn()) );
  cbYColumn->setCurrentModelIndex( m_aspectTreeModel->modelIndexOfAspect(curve->yColumn()) );
  
  //Line-tab
  ui.cbLineType->setCurrentIndex( curve->lineType() );
  ui.sbLineInterpolationPointsCount->setValue( curve->lineInterpolationPointsCount() );
  ui.cbLineStyle->setCurrentIndex( curve->linePen().style() );
  ui.kcbLineColor->setColor( curve->linePen().color() );
  ui.sbLineWidth->setValue( Worksheet::convertFromSceneUnits(curve->linePen().widthF(), Worksheet::Point) );
  ui.sbLineOpacity->setValue( curve->lineOpacity()*100 );
  GuiTools::updatePenStyles(ui.cbLineStyle, curve->linePen().color() );
  
  ui.cbDropLineType->setCurrentIndex( curve->dropLineType() );
  ui.cbDropLineStyle->setCurrentIndex( curve->dropLinePen().style() );
  ui.kcbDropLineColor->setColor( curve->dropLinePen().color() );
  ui.sbDropLineWidth->setValue( Worksheet::convertFromSceneUnits(curve->dropLinePen().widthF(), Worksheet::Point) );
  ui.sbDropLineOpacity->setValue( curve->dropLineOpacity()*100 );
  GuiTools::updatePenStyles(ui.cbDropLineStyle, curve->dropLinePen().color() );
  
  
  //Symbol-tab
  ui.cbSymbolStyle->setCurrentIndex( ui.cbSymbolStyle->findText(curve->symbolTypeId()) );
  ui.sbSymbolSize->setValue( Worksheet::convertFromSceneUnits(curve->symbolSize(), Worksheet::Point) );
  ui.sbSymbolRotation->setValue( curve->symbolRotationAngle() );
  ui.sbSymbolOpacity->setValue( curve->symbolsOpacity()*100 );
  ui.cbSymbolFillingStyle->setCurrentIndex( curve->symbolsBrush().style() );
  ui.kcbSymbolFillingColor->setColor( curve->symbolsBrush().color() );

  ui.cbSymbolBorderStyle->setCurrentIndex( curve->symbolsPen().style() );
  ui.kcbSymbolBorderColor->setColor( curve->symbolsPen().color() );
  ui.sbSymbolBorderWidth->setValue( Worksheet::convertFromSceneUnits(curve->symbolsPen().widthF(), Worksheet::Point) );

  GuiTools::updatePenStyles(ui.cbSymbolBorderStyle, curve->symbolsPen().color() );
  GuiTools::updateBrushStyles(ui.cbSymbolFillingStyle, curve->symbolsBrush().color() );
  
  //Values-tab
  ui.cbValuesType->setCurrentIndex( curve->valuesType() );

  ui.cbValuesPosition->setCurrentIndex( curve->valuesPosition() );
  ui.sbValuesRotation->setValue( curve->valuesRotationAngle() );
  ui.sbValuesDistance->setValue( Worksheet::convertFromSceneUnits(curve->valuesDistance(), Worksheet::Point) );
  ui.sbValuesOpacity->setValue( curve->valuesOpacity()*100 );
  ui.leValuesPrefix->setText( curve->valuesPrefix() );
  ui.leValuesSuffix->setText( curve->valuesSuffix() );
  ui.kfrValuesFont->setFont( curve->valuesFont() );
  ui.kcbValuesFontColor->setColor( curve->valuesPen().color() );

  //TODO
  //Area filling
  //Error bars


//TODO connect the signals of the first column with the slots of this class.

  m_initializing=false;
}


/*!
	fills the ComboBox for the symbol style with all possible styles in the style factory.
*/
void XYCurveDock::fillSymbolStyles(){
	QPainter painter;
	int size=20; 	//TODO size of the icon depending on the actuall height of the combobox?
	QPixmap pm( size, size );
 	ui.cbSymbolStyle->setIconSize( QSize(size, size) );
	
  //TODO redesign the PluginManager and adjust this part here (load only symbol plugins and not everything!!!)
	foreach(QObject *plugin, PluginManager::plugins()) {
		CurveSymbolFactory *factory = qobject_cast<CurveSymbolFactory *>(plugin);
		if (factory){
		  symbolFactory=factory;
		  AbstractCurveSymbol* symbol;
		  foreach (const AbstractCurveSymbol* symbolPrototype, factory->prototypes()){
			if (symbolPrototype){
			  symbol= symbolPrototype->clone();
			  symbol->setSize(15);
			  
			  pm.fill(Qt::transparent);
			  painter.begin( &pm );
			  painter.setRenderHint(QPainter::Antialiasing);
			  painter.translate(size/2,size/2);
			  symbol->paint(&painter);
			  painter.end();
			  ui.cbSymbolStyle->addItem(QIcon(pm), symbol->id());
			}
		  }
		}
	}
}


/*!
  depending on the currently selected values column type (column mode) updates the widgets for the values column format, 
  shows/hides the allowed widgets, fills the corresponding combobox with the possible entries.
  Called when the values column was changed.
  
  synchronize this function with ColumnDock::updateFormat.
*/
void XYCurveDock::updateValuesFormatWidgets(const SciDAVis::ColumnMode columnMode){
  ui.cbValuesFormat->clear();

  switch (columnMode){
	case SciDAVis::Numeric:
	  ui.cbValuesFormat->addItem(tr("Decimal"), QVariant('f'));
	  ui.cbValuesFormat->addItem(tr("Scientific (e)"), QVariant('e'));
	  ui.cbValuesFormat->addItem(tr("Scientific (E)"), QVariant('E'));
	  ui.cbValuesFormat->addItem(tr("Automatic (e)"), QVariant('g'));
	  ui.cbValuesFormat->addItem(tr("Automatic (E)"), QVariant('G'));
	  break;
	case SciDAVis::Text:
	  ui.cbValuesFormat->addItem(tr("Text"), QVariant());
	  break;
	case SciDAVis::Month:
	  ui.cbValuesFormat->addItem(tr("Number without leading zero"), QVariant("M"));
	  ui.cbValuesFormat->addItem(tr("Number with leading zero"), QVariant("MM"));
	  ui.cbValuesFormat->addItem(tr("Abbreviated month name"), QVariant("MMM"));
	  ui.cbValuesFormat->addItem(tr("Full month name"), QVariant("MMMM"));
	  break;
	case SciDAVis::Day:
	  ui.cbValuesFormat->addItem(tr("Number without leading zero"), QVariant("d"));
	  ui.cbValuesFormat->addItem(tr("Number with leading zero"), QVariant("dd"));
	  ui.cbValuesFormat->addItem(tr("Abbreviated day name"), QVariant("ddd"));
	  ui.cbValuesFormat->addItem(tr("Full day name"), QVariant("dddd"));
	  break;
	case SciDAVis::DateTime:{
	  foreach(QString s, dateStrings)
		ui.cbValuesFormat->addItem(s, QVariant(s));
	  
	  foreach(QString s, timeStrings)
		ui.cbValuesFormat->addItem(s, QVariant(s));
	  
	  foreach(QString s1, dateStrings){
		foreach(QString s2, timeStrings)
		  ui.cbValuesFormat->addItem(s1 + " " + s2, QVariant(s1 + " " + s2));
	  }
	  
	  break;
	}
	default:
		break;
  }
  
  ui.cbValuesFormat->setCurrentIndex(0);
  
  if (columnMode == SciDAVis::Numeric){
	ui.lValuesPrecision->show();
	ui.sbValuesPrecision->show();
  }else{
	ui.lValuesPrecision->hide();
	ui.sbValuesPrecision->hide();
  }
  
  if (columnMode == SciDAVis::Text){
	ui.lValuesFormatTop->hide();
	ui.lValuesFormat->hide();
	ui.cbValuesFormat->hide();
  }else{
	ui.lValuesFormatTop->show();
	ui.lValuesFormat->show();
	ui.cbValuesFormat->show();
	ui.cbValuesFormat->setCurrentIndex(0);
  }
  
  if (columnMode == SciDAVis::DateTime){
	ui.cbValuesFormat->setEditable( true );
  }else{
	ui.cbValuesFormat->setEditable( false );
  }
}

/*!
  shows the formating properties of the column \c column. 
  Called, when a new column for the values was selected - either by changing the type of the values (none, x, y, etc.) or 
  by selecting a new custom column for the values.
*/
void XYCurveDock::showValuesColumnFormat(const Column* column){
  if (!column){
	// no valid column is available 
	// -> hide all the format properties widgets (equivalent to showing the properties of the column mode "Text")
	m_initializing = true;
	this->updateValuesFormatWidgets(SciDAVis::Text);
	m_initializing = false;
  }else{
	SciDAVis::ColumnMode columnMode = column->columnMode();
	
	//update the format widgets for the new column mode
	m_initializing = true;
	this->updateValuesFormatWidgets(columnMode);
	m_initializing = false;
	  
	 //show the actuall formating properties
	switch(columnMode) {
		case SciDAVis::Numeric:{
		  Double2StringFilter * filter = static_cast<Double2StringFilter*>(column->outputFilter());
		  ui.cbValuesFormat->setCurrentIndex(ui.cbValuesFormat->findData(filter->numericFormat()));
		  ui.sbValuesPrecision->setValue(filter->numDigits());
		  break;
		}
		case SciDAVis::Month:
		case SciDAVis::Day:
		case SciDAVis::DateTime: {
				DateTime2StringFilter * filter = static_cast<DateTime2StringFilter*>(column->outputFilter());
				ui.cbValuesFormat->setCurrentIndex(ui.cbValuesFormat->findData(filter->format()));
				break;
			}
		default:
			break;
	}
  }
}


//************************************************************
//****************** SLOTS ********************************
//************************************************************
void XYCurveDock::retranslateUi(){
	lName->setText(i18n("Name"));
	lComment->setText(i18n("Comment"));
	chkVisible->setText(i18n("Visible"));
	lXColumn->setText(i18n("x-data"));
	lYColumn->setText(i18n("y-data"));
	
	//TODO updatePenStyles, updateBrushStyles for all comboboxes
}

// "General"-tab
void XYCurveDock::nameChanged(){
  if (m_initializing)
	return;
  
  m_curvesList.first()->setName(leName->text());
}


void XYCurveDock::commentChanged(){
  if (m_initializing)
	return;
  
  m_curvesList.first()->setComment(leComment->text());
}

void XYCurveDock::xColumnChanged(const QModelIndex& index){
	Q_UNUSED(index);
	if (m_initializing)
		return;
  
	AbstractColumn* column= static_cast<AbstractColumn*>(index.internalPointer());
	foreach(XYCurve* curve, m_curvesList){
	curve->setXColumn(column);
	}
}

void XYCurveDock::yColumnChanged(const QModelIndex& index){
	Q_UNUSED(index);
	if (m_initializing)
		return;
  
	AbstractColumn* column= static_cast<AbstractColumn*>(index.internalPointer());
	foreach(XYCurve* curve, m_curvesList){
	curve->setYColumn(column);
	}
}

void XYCurveDock::visibilityChanged(int state){
  if (m_initializing)
	return;
  
  bool b;
  if (state==Qt::Checked)
	b=true;
  else
	b=false;

  foreach(XYCurve* curve, m_curvesList){
	curve->setVisible(b);
  }
}

// "Line"-tab
void XYCurveDock::lineTypeChanged(int index){
  XYCurve::LineType lineType = XYCurve::LineType(index);
  
  if ( lineType == XYCurve::NoLine){
	ui.cbLineStyle->setEnabled(false);
	ui.kcbLineColor->setEnabled(false);
	ui.sbLineWidth->setEnabled(false);
	ui.sbLineOpacity->setEnabled(false);
	ui.lLineInterpolationPointsCount->hide();
	ui.sbLineInterpolationPointsCount->hide();
  }else{
	ui.cbLineStyle->setEnabled(true);	
	ui.kcbLineColor->setEnabled(true);
	ui.sbLineWidth->setEnabled(true);
	ui.sbLineOpacity->setEnabled(true);
	
	if (lineType==XYCurve::SplineCubicNatural || lineType==XYCurve::SplineCubicPeriodic
	  || lineType==XYCurve::SplineAkimaNatural || lineType==XYCurve::SplineAkimaPeriodic){
	  ui.lLineInterpolationPointsCount->show();
	  ui.sbLineInterpolationPointsCount->show();
	}else{
	  ui.lLineInterpolationPointsCount->hide();
	  ui.sbLineInterpolationPointsCount->hide();
	 }
  }
  
  if (m_initializing)
	return;
  
  foreach(XYCurve* curve, m_curvesList){
	curve->setLineType(lineType);
  }
}


void XYCurveDock::lineInterpolationPointsCountChanged(int count){
   if (m_initializing)
	return;

  foreach(XYCurve* curve, m_curvesList){
	curve->setLineInterpolationPointsCount(count);
  }
}

void XYCurveDock::lineStyleChanged(int index){
   if (m_initializing)
	return;

  Qt::PenStyle penStyle=Qt::PenStyle(index);
  QPen pen;
  foreach(XYCurve* curve, m_curvesList){
	pen=curve->linePen();
	pen.setStyle(penStyle);
	curve->setLinePen(pen);
  }
}

void XYCurveDock::lineColorChanged(const QColor& color){
  if (m_initializing)
	return;

  QPen pen;
  foreach(XYCurve* curve, m_curvesList){
	pen=curve->linePen();
	pen.setColor(color);
	curve->setLinePen(pen);
  }  

  GuiTools::updatePenStyles(ui.cbLineStyle, color);
}

void XYCurveDock::lineWidthChanged(double value){
  if (m_initializing)
	return;
  
  QPen pen;
  foreach(XYCurve* curve, m_curvesList){
	pen=curve->linePen();
	pen.setWidthF( Worksheet::convertToSceneUnits(value, Worksheet::Point) );
	curve->setLinePen(pen);
  }  
}

void XYCurveDock::lineOpacityChanged(int value){
  if (m_initializing)
	return;
		
  qreal opacity = (float)value/100;
  foreach(XYCurve* curve, m_curvesList)
	curve->setLineOpacity(opacity);
	
}

void XYCurveDock::dropLineTypeChanged(int index){
  XYCurve::DropLineType dropLineType = XYCurve::DropLineType(index);
  
  if ( dropLineType == XYCurve::NoDropLine){
	ui.cbDropLineStyle->setEnabled(false);
	ui.kcbDropLineColor->setEnabled(false);
	ui.sbDropLineWidth->setEnabled(false);
	ui.sbDropLineOpacity->setEnabled(false);
  }else{
	ui.cbDropLineStyle->setEnabled(true);
	ui.kcbDropLineColor->setEnabled(true);
	ui.sbDropLineWidth->setEnabled(true);
	ui.sbDropLineOpacity->setEnabled(true);
  }
  
  if (m_initializing)
	return;
  
  foreach(XYCurve* curve, m_curvesList){
	curve->setDropLineType(dropLineType);
  }
}


void XYCurveDock::dropLineStyleChanged(int index){
   if (m_initializing)
	return;

  Qt::PenStyle penStyle=Qt::PenStyle(index);
  QPen pen;
  foreach(XYCurve* curve, m_curvesList){
	pen=curve->dropLinePen();
	pen.setStyle(penStyle);
	curve->setDropLinePen(pen);
  }
}

void XYCurveDock::dropLineColorChanged(const QColor& color){
  if (m_initializing)
	return;

  QPen pen;
  foreach(XYCurve* curve, m_curvesList){
	pen=curve->dropLinePen();
	pen.setColor(color);
	curve->setDropLinePen(pen);
  }  

  GuiTools::updatePenStyles(ui.cbDropLineStyle, color);
}

void XYCurveDock::dropLineWidthChanged(double value){
  if (m_initializing)
	return;
  
  QPen pen;
  foreach(XYCurve* curve, m_curvesList){
	pen=curve->dropLinePen();
	pen.setWidthF( Worksheet::convertToSceneUnits(value, Worksheet::Point) );
	curve->setDropLinePen(pen);
  }  
}

void XYCurveDock::dropLineOpacityChanged(int value){
  if (m_initializing)
	return;
		
  qreal opacity = (float)value/100;
  foreach(XYCurve* curve, m_curvesList)
	curve->setDropLineOpacity(opacity);
	
}

//"Symbol"-tab
void XYCurveDock::symbolStyleChanged(int index){
  Q_UNUSED(index);
  QString currentSymbolTypeId = ui.cbSymbolStyle->currentText();
  
  if (currentSymbolTypeId=="none"){
	ui.sbSymbolSize->setEnabled(false);
	ui.sbSymbolRotation->setEnabled(false);
	ui.sbSymbolOpacity->setEnabled(false);
	
	ui.kcbSymbolFillingColor->setEnabled(false);
	ui.lSymbolFillingStyle->setEnabled(false);
	ui.cbSymbolFillingStyle->setEnabled(false);
	
	ui.cbSymbolBorderStyle->setEnabled(false);
	ui.kcbSymbolBorderColor->setEnabled(false);
	ui.sbSymbolBorderWidth->setEnabled(false);
  }else{
	ui.sbSymbolSize->setEnabled(true);
	ui.sbSymbolRotation->setEnabled(true);
	ui.sbSymbolOpacity->setEnabled(true);
	
	//enable/disable the symbol filling options in the GUI depending on the currently selected symbol.
	if ( symbolFactory->prototype(currentSymbolTypeId)->fillingEnabled() ){
	  ui.kcbSymbolFillingColor->setEnabled(true);
	  ui.lSymbolFillingStyle->setEnabled(true);
	  ui.cbSymbolFillingStyle->setEnabled(true);
	}else{
	  ui.kcbSymbolFillingColor->setEnabled(false);
	  ui.cbSymbolFillingStyle->setEnabled(false);
	}
	
	ui.cbSymbolBorderStyle->setEnabled(true);
	ui.kcbSymbolBorderColor->setEnabled(true);
	ui.sbSymbolBorderWidth->setEnabled(true);
  }

  if (m_initializing)
	return;

  foreach(XYCurve* curve, m_curvesList){
	curve->setSymbolTypeId(currentSymbolTypeId);
  }

}


void XYCurveDock::symbolSizeChanged(double value){
  if (m_initializing)
	return;
	
  foreach(XYCurve* curve, m_curvesList){
	curve->setSymbolSize( Worksheet::convertToSceneUnits(value, Worksheet::Point) );
  }
}

void XYCurveDock::symbolRotationChanged(int value){
  if (m_initializing)
	return;
	
  foreach(XYCurve* curve, m_curvesList){
	curve->setSymbolRotationAngle(value);
  }
}

void XYCurveDock::symbolOpacityChanged(int value){
  if (m_initializing)
	return;
		
  qreal opacity = (float)value/100;
  foreach(XYCurve* curve, m_curvesList){
	curve->setSymbolsOpacity(opacity);
  }
}

void XYCurveDock::symbolFillingStyleChanged(int index){
  Qt::BrushStyle brushStyle = Qt::BrushStyle(index);
  if (brushStyle == Qt::NoBrush){
	ui.lSymbolFillingColor->setEnabled(false);
	ui.kcbSymbolFillingColor->setEnabled(false);
  }else{
	ui.lSymbolFillingColor->setEnabled(true);
	ui.kcbSymbolFillingColor->setEnabled(true);
  }
  
  if (m_initializing)
	return;

  QBrush brush;
  foreach(XYCurve* curve, m_curvesList){
	brush=curve->symbolsBrush();
	brush.setStyle(brushStyle);
	curve->setSymbolsBrush(brush);
  }
}

void XYCurveDock::symbolFillingColorChanged(const QColor& color){
  if (m_initializing)
	return;
	
  QBrush brush;
  foreach(XYCurve* curve, m_curvesList){
	brush=curve->symbolsBrush();
	brush.setColor(color);
	curve->setSymbolsBrush(brush);
  }

  GuiTools::updateBrushStyles(ui.cbSymbolFillingStyle, color );
}

void XYCurveDock::symbolBorderStyleChanged(int index){
  Qt::PenStyle penStyle=Qt::PenStyle(index);
  
  if ( penStyle == Qt::NoPen ){
	ui.kcbSymbolBorderColor->setEnabled(false);
	ui.sbSymbolBorderWidth->setEnabled(false);
  }else{
	ui.kcbSymbolBorderColor->setEnabled(true);
	ui.sbSymbolBorderWidth->setEnabled(true);
  }

  if (m_initializing)
	return;
  
  QPen pen;
  foreach(XYCurve* curve, m_curvesList){
	pen=curve->symbolsPen();
	pen.setStyle(penStyle);
	curve->setSymbolsPen(pen);
  }
}

void XYCurveDock::symbolBorderColorChanged(const QColor& color){
  if (m_initializing)
	return;
  
  QPen pen;
  foreach(XYCurve* curve, m_curvesList){
	pen=curve->symbolsPen();
	pen.setColor(color);
	curve->setSymbolsPen(pen);
  }  
  
  GuiTools::updatePenStyles(ui.cbSymbolBorderStyle, color);
}

void XYCurveDock::symbolBorderWidthChanged(double value){
  if (m_initializing)
	return;
  
  QPen pen;
  foreach(XYCurve* curve, m_curvesList){
	pen=curve->symbolsPen();
	pen.setWidthF( Worksheet::convertToSceneUnits(value, Worksheet::Point) );
	curve->setSymbolsPen(pen);
  }
}

//Values-tab

/*!
  called when the type of the values (none, x, y, (x,y) etc.) was changed.
*/
void XYCurveDock::valuesTypeChanged(int index){
  XYCurve::ValuesType valuesType = XYCurve::ValuesType(index);
  
  if (valuesType==XYCurve::NoValues){
	//no values are to paint -> deactivate all the pertinent widgets
	ui.cbValuesPosition->setEnabled(false);
	ui.lValuesColumn->hide();
	cbValuesColumn->hide();
	ui.sbValuesDistance->setEnabled(false);
	ui.sbValuesRotation->setEnabled(false);	
	ui.sbValuesOpacity->setEnabled(false);
	ui.cbValuesFormat->setEnabled(false);
	ui.cbValuesFormat->setEnabled(false);
	ui.sbValuesPrecision->setEnabled(false);
	ui.leValuesPrefix->setEnabled(false);
	ui.leValuesSuffix->setEnabled(false);
	ui.kfrValuesFont->setEnabled(false);
	ui.kcbValuesFontColor->setEnabled(false);
  }else{
	ui.cbValuesPosition->setEnabled(true);
	ui.sbValuesDistance->setEnabled(true);
	ui.sbValuesRotation->setEnabled(true);	
	ui.sbValuesOpacity->setEnabled(true);
	ui.cbValuesFormat->setEnabled(true);
	ui.sbValuesPrecision->setEnabled(true);
	ui.leValuesPrefix->setEnabled(true);
	ui.leValuesSuffix->setEnabled(true);
	ui.kfrValuesFont->setEnabled(true);
	ui.kcbValuesFontColor->setEnabled(true);	
	
	const Column* column;
	if (valuesType==XYCurve::ValuesCustomColumn){
	  ui.lValuesColumn->show();
	  cbValuesColumn->show();
	  
	  column= static_cast<Column*>(cbValuesColumn->currentModelIndex().internalPointer());
	}else{
	  ui.lValuesColumn->hide();
	  cbValuesColumn->hide();
	  
	  if (valuesType==XYCurve::ValuesY){
		column = static_cast<const Column*>(m_curvesList.first()->yColumn());
	  }else{
		column = static_cast<const Column*>(m_curvesList.first()->xColumn());
	  }
	}
	this->showValuesColumnFormat(column);
  }

	
  if (m_initializing)
	return;

  foreach(XYCurve* curve, m_curvesList){
	curve->setValuesType(valuesType);
  }
}

/*!
  called when the custom column for the values was changed.
*/
void XYCurveDock::valuesColumnChanged(int index){
	Q_UNUSED(index);
  if (m_initializing)
	return;

  Column* column= static_cast<Column*>(cbValuesColumn->currentModelIndex().internalPointer());
  this->showValuesColumnFormat(column);
  
  foreach(XYCurve* curve, m_curvesList){
	//TODO save also the format of the currently selected column for the values (precision etc.)
	curve->setValuesColumn(column);
  }
}

void XYCurveDock::valuesPositionChanged(int index){
  if (m_initializing)
	return;
	
  foreach(XYCurve* curve, m_curvesList){
	curve->setValuesPosition(XYCurve::ValuesPosition(index));
  }
}

void XYCurveDock::valuesDistanceChanged(double  value){
  if (m_initializing)
	return;
		
  foreach(XYCurve* curve, m_curvesList){
	curve->setValuesDistance( Worksheet::convertToSceneUnits(value, Worksheet::Point) );
  }
}

void XYCurveDock::valuesRotationChanged(int value){
  if (m_initializing)
	return;
		
  foreach(XYCurve* curve, m_curvesList){
	curve->setValuesRotationAngle(value);
  }
}

void XYCurveDock::valuesOpacityChanged(int value){
  if (m_initializing)
	return;
		
  qreal opacity = (float)value/100;
  foreach(XYCurve* curve, m_curvesList){
	curve->setValuesOpacity(opacity);
  }
}

void XYCurveDock::valuesPrefixChanged(){
  if (m_initializing)
	return;
		
  QString prefix = ui.leValuesPrefix->text();
  foreach(XYCurve* curve, m_curvesList){
	curve->setValuesPrefix(prefix);
  }
}

void XYCurveDock::valuesSuffixChanged(){
  if (m_initializing)
	return;
		
  QString suffix = ui.leValuesSuffix->text();
  foreach(XYCurve* curve, m_curvesList){
	curve->setValuesSuffix(suffix);
  }
}

void XYCurveDock::valuesFontChanged(const QFont& font){
  if (m_initializing)
	return;
	
  foreach(XYCurve* curve, m_curvesList){
	curve->setValuesFont(font);
  }
}

void XYCurveDock::valuesFontColorChanged(const QColor& color){
  if (m_initializing)
	return;
  
  QPen pen;
  foreach(XYCurve* curve, m_curvesList){
	pen=curve->valuesPen();
	pen.setColor(color);
	curve->setValuesPen(pen);
  }  
}

/* Settings */

void XYCurveDock::loadSettings(){
    	QString filename=QFileDialog::getOpenFileName(this, i18n("Select the file to load settings"),
			"LabPlotrc", i18n("KDE resource files (*rc)"));
    	if (filename=="")
        	return;

	KConfig config(filename, KConfig::SimpleConfig);
	KConfigGroup group = config.group( "XYCurve" );

  	//TODO
}

void XYCurveDock::saveSettings(){
     	QString filename=QFileDialog::getSaveFileName(this, i18n("Select the file to save settings"),
			"LabPlotrc", i18n("KDE resource files (*rc)"));
    	if (filename=="")
        	return;

	KConfig config(filename, KConfig::SimpleConfig );
	save(config);
	config.sync();
}

void XYCurveDock::saveDefaults(){
	KConfig config;
	save(config);
	config.sync();
}

void XYCurveDock::save(const KConfig& config){
	KConfigGroup group = config.group( "XYCurve" );

	//TODO
}

