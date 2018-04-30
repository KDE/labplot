/***************************************************************************
    File                 : WorksheetDock.cpp
    Project              : LabPlot
    Description          : widget for worksheet properties
    --------------------------------------------------------------------
    Copyright            : (C) 2010-2016 by Alexander Semke (alexander.semke@web.de)
    Copyright            : (C) 2012-2013 by Stefan Gerlach (stefan.gerlach@uni-konstanz.de)

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

#include "WorksheetDock.h"
#include "kdefrontend/GuiTools.h"
#include "kdefrontend/ThemeHandler.h"
#include "kdefrontend/TemplateHandler.h"

#include <QCompleter>
#include <QDirModel>
#include <QFileDialog>
#include <QImageReader>
#include <QPrinter>

#include <KConfig>
#include <KConfigGroup>
#include <KLocalizedString>

// a couple of standard sizes in mm, taken from qprinter.cpp
const int numOfPaperSizes = 30;
const float qt_paperSizes[numOfPaperSizes][2] = {
	{210, 297}, // A4
	{176, 250}, // B5
	{215.9f, 279.4f}, // Letter
	{215.9f, 355.6f}, // Legal
	{190.5f, 254}, // Executive
	{841, 1189}, // A0
	{594, 841}, // A1
	{420, 594}, // A2
	{297, 420}, // A3
	{148, 210}, // A5
	{105, 148}, // A6
	{74, 105}, // A7
	{52, 74}, // A8
	{37, 52}, // A8
	{1000, 1414}, // B0
	{707, 1000}, // B1
	{31, 44}, // B10
	{500, 707}, // B2
	{353, 500}, // B3
	{250, 353}, // B4
	{125, 176}, // B6
	{88, 125}, // B7
	{62, 88}, // B8
	{33, 62}, // B9
	{163, 229}, // C5E
	{105, 241}, // US Common
	{110, 220}, // DLE
	{210, 330}, // Folio
	{431.8f, 279.4f}, // Ledger
	{279.4f, 431.8f} // Tabloid
};

/*!
  \class WorksheetDock
  \brief  Provides a widget for editing the properties of the worksheets currently selected in the project explorer.

  \ingroup kdefrontend
*/

WorksheetDock::WorksheetDock(QWidget *parent): QWidget(parent), m_worksheet(0) {
	ui.setupUi(this);

	//Background-tab
	ui.cbBackgroundColorStyle->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLengthWithIcon);
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

	//SLOTs

	//General
	connect(ui.leName, &QLineEdit::textChanged, this, &WorksheetDock::nameChanged);
	connect(ui.leComment, &QLineEdit::textChanged, this, &WorksheetDock::commentChanged);
	connect( ui.cbSize, SIGNAL(currentIndexChanged(int)), this, SLOT(sizeChanged(int)) );
	connect( ui.sbWidth, SIGNAL(valueChanged(double)), this, SLOT(sizeChanged()) );
	connect( ui.sbHeight, SIGNAL(valueChanged(double)), this, SLOT(sizeChanged()) );
	connect( ui.cbOrientation, SIGNAL(currentIndexChanged(int)), this, SLOT(orientationChanged(int)) );

	//Background
	connect( ui.cbBackgroundType, SIGNAL(currentIndexChanged(int)), this, SLOT(backgroundTypeChanged(int)) );
	connect( ui.cbBackgroundColorStyle, SIGNAL(currentIndexChanged(int)), this, SLOT(backgroundColorStyleChanged(int)) );
	connect( ui.cbBackgroundImageStyle, SIGNAL(currentIndexChanged(int)), this, SLOT(backgroundImageStyleChanged(int)) );
	connect( ui.cbBackgroundBrushStyle, SIGNAL(currentIndexChanged(int)), this, SLOT(backgroundBrushStyleChanged(int)) );
	connect( ui.bOpen, SIGNAL(clicked(bool)), this, SLOT(selectFile()));
	connect( ui.leBackgroundFileName, SIGNAL(returnPressed()), this, SLOT(fileNameChanged()) );
	connect( ui.leBackgroundFileName, SIGNAL(textChanged(const QString&)), this, SLOT(fileNameChanged()) );
	connect( ui.kcbBackgroundFirstColor, SIGNAL(changed(QColor)), this, SLOT(backgroundFirstColorChanged(QColor)) );
	connect( ui.kcbBackgroundSecondColor, SIGNAL(changed(QColor)), this, SLOT(backgroundSecondColorChanged(QColor)) );
	connect( ui.sbBackgroundOpacity, SIGNAL(valueChanged(int)), this, SLOT(backgroundOpacityChanged(int)) );

	//Layout
	connect( ui.chScaleContent, SIGNAL(clicked(bool)), this, SLOT(scaleContentChanged(bool)) );
	connect( ui.sbLayoutTopMargin, SIGNAL(valueChanged(double)), this, SLOT(layoutTopMarginChanged(double)) );
	connect( ui.sbLayoutBottomMargin, SIGNAL(valueChanged(double)), this, SLOT(layoutBottomMarginChanged(double)) );
	connect( ui.sbLayoutLeftMargin, SIGNAL(valueChanged(double)), this, SLOT(layoutLeftMarginChanged(double)) );
	connect( ui.sbLayoutRightMargin, SIGNAL(valueChanged(double)), this, SLOT(layoutRightMarginChanged(double)) );
	connect( ui.sbLayoutHorizontalSpacing, SIGNAL(valueChanged(double)), this, SLOT(layoutHorizontalSpacingChanged(double)) );
	connect( ui.sbLayoutVerticalSpacing, SIGNAL(valueChanged(double)), this, SLOT(layoutVerticalSpacingChanged(double)) );
	connect( ui.sbLayoutRowCount, SIGNAL(valueChanged(int)), this, SLOT(layoutRowCountChanged(int)) );
	connect( ui.sbLayoutColumnCount, SIGNAL(valueChanged(int)), this, SLOT(layoutColumnCountChanged(int)) );

	//theme and template handlers
	QFrame* frame = new QFrame(this);
	QHBoxLayout* layout = new QHBoxLayout(frame);

	m_themeHandler = new ThemeHandler(this);
	layout->addWidget(m_themeHandler);
	connect(m_themeHandler, SIGNAL(loadThemeRequested(QString)), this, SLOT(loadTheme(QString)));
	connect(m_themeHandler, SIGNAL(info(QString)), this, SIGNAL(info(QString)));

	TemplateHandler* templateHandler = new TemplateHandler(this, TemplateHandler::Worksheet);
	layout->addWidget(templateHandler);
	connect(templateHandler, SIGNAL(loadConfigRequested(KConfig&)), this, SLOT(loadConfigFromTemplate(KConfig&)));
	connect(templateHandler, SIGNAL(saveConfigRequested(KConfig&)), this, SLOT(saveConfigAsTemplate(KConfig&)));
	connect(templateHandler, SIGNAL(info(QString)), this, SIGNAL(info(QString)));

	ui.verticalLayout->addWidget(frame);

	this->retranslateUi();
}

void WorksheetDock::setWorksheets(QList<Worksheet*> list) {
	m_initializing = true;
	m_worksheetList = list;
	m_worksheet = list.first();

	//if there are more then one worksheet in the list, disable the name and comment field in the tab "general"
	if (list.size()==1) {
		ui.lName->setEnabled(true);
		ui.leName->setEnabled(true);
		ui.lComment->setEnabled(true);
		ui.leComment->setEnabled(true);

		ui.leName->setText(m_worksheet->name());
		ui.leComment->setText(m_worksheet->comment());
	} else {
		ui.lName->setEnabled(false);
		ui.leName->setEnabled(false);
		ui.lComment->setEnabled(false);
		ui.leComment->setEnabled(false);

		ui.leName->setText("");
		ui.leComment->setText("");
	}

	//show the properties of the first worksheet
	this->load();
	this->worksheetLayoutChanged(m_worksheet->layout());

	m_themeHandler->setCurrentTheme(m_worksheet->theme());

	connect(m_worksheet, SIGNAL(aspectDescriptionChanged(const AbstractAspect*)),this, SLOT(worksheetDescriptionChanged(const AbstractAspect*)));
	connect(m_worksheet, SIGNAL(pageRectChanged(QRectF)),this, SLOT(worksheetPageRectChanged(QRectF)));
	connect(m_worksheet,SIGNAL(scaleContentChanged(bool)),this,SLOT(worksheetScaleContentChanged(bool)));

	connect(m_worksheet,SIGNAL(backgroundTypeChanged(PlotArea::BackgroundType)),this,SLOT(worksheetBackgroundTypeChanged(PlotArea::BackgroundType)));
	connect(m_worksheet,SIGNAL(backgroundColorStyleChanged(PlotArea::BackgroundColorStyle)),this,SLOT(worksheetBackgroundColorStyleChanged(PlotArea::BackgroundColorStyle)));
	connect(m_worksheet,SIGNAL(backgroundImageStyleChanged(PlotArea::BackgroundImageStyle)),this,SLOT(worksheetBackgroundImageStyleChanged(PlotArea::BackgroundImageStyle)));
	connect(m_worksheet,SIGNAL(backgroundBrushStyleChanged(Qt::BrushStyle)),this,SLOT(worksheetBackgroundBrushStyleChanged(Qt::BrushStyle)));
	connect(m_worksheet,SIGNAL(backgroundFirstColorChanged(QColor)),this,SLOT(worksheetBackgroundFirstColorChanged(QColor)));
	connect(m_worksheet,SIGNAL(backgroundSecondColorChanged(QColor)),this,SLOT(worksheetBackgroundSecondColorChanged(QColor)));
	connect(m_worksheet,SIGNAL(backgroundFileNameChanged(QString)),this,SLOT(worksheetBackgroundFileNameChanged(QString)));
	connect(m_worksheet,SIGNAL(backgroundOpacityChanged(float)),this,SLOT(worksheetBackgroundOpacityChanged(float)));

	connect(m_worksheet,SIGNAL(layoutChanged(Worksheet::Layout)),this,SLOT(worksheetLayoutChanged(Worksheet::Layout)));
	connect(m_worksheet,SIGNAL(layoutTopMarginChanged(float)),this,SLOT(worksheetLayoutTopMarginChanged(float)));
	connect(m_worksheet,SIGNAL(layoutBottomMarginChanged(float)),this,SLOT(worksheetLayoutBottomMarginChanged(float)));
	connect(m_worksheet,SIGNAL(layoutLeftMarginChanged(float)),this,SLOT(worksheetLayoutLeftMarginChanged(float)));
	connect(m_worksheet,SIGNAL(layoutRightMarginChanged(float)),this,SLOT(worksheetLayoutRightMarginChanged(float)));
	connect(m_worksheet,SIGNAL(layoutVerticalSpacingChanged(float)),this,SLOT(worksheetLayoutVerticalSpacingChanged(float)));
	connect(m_worksheet,SIGNAL(layoutHorizontalSpacingChanged(float)),this,SLOT(worksheetLayoutHorizontalSpacingChanged(float)));
	connect(m_worksheet,SIGNAL(layoutRowCountChanged(int)),this,SLOT(worksheetLayoutRowCountChanged(int)));
	connect(m_worksheet,SIGNAL(layoutColumnCountChanged(int)),this,SLOT(worksheetLayoutColumnCountChanged(int)));

	connect(m_worksheet,SIGNAL(themeChanged(QString)),m_themeHandler,SLOT(setCurrentTheme(QString)));

	m_initializing = false;
}

/*!
	Checks whether the size is one of the QPrinter::PaperSize and
	updates Size and Orientation checkbox when width/height changes.
*/
void WorksheetDock::updatePaperSize() {
	if (m_worksheet->useViewSize()) {
		ui.cbSize->setCurrentIndex(0);
		return;
	}

	int i=0;

	//In UI we use cm, so we need to convert to mm first before we check with qt_paperSizes
	float w=(float)ui.sbWidth->value()*10;
	float h=(float)ui.sbHeight->value()*10;

	//check the portrait-orientation first
	while ( i<numOfPaperSizes && !(w==qt_paperSizes[i][0] && h==qt_paperSizes[i][1]) )
		i++;

	if (i!=numOfPaperSizes) {
		ui.cbOrientation->setCurrentIndex(0);  //a QPrinter::PaperSize  in portrait-orientation was found
	} else {
		//check for the landscape-orientation
		i=0;
		while ( i<numOfPaperSizes && !(w==qt_paperSizes[i][1] && h==qt_paperSizes[i][0]) )
			i++;

		if (i!=numOfPaperSizes)
			ui.cbOrientation->setCurrentIndex(1); //a QPrinter::PaperSize  in landscape-orientation was found
	}

	//determine the position of the QPrinter::PaperSize in the combobox
	for (int index = 0; index < numOfPaperSizes+1; ++index) {
		if (ui.cbSize->itemData(index+2).toInt() == i) {
			ui.cbSize->setCurrentIndex(index+2);
			break;
		}
	}
}

//*************************************************************
//****** SLOTs for changes triggered in WorksheetDock *********
//*************************************************************
void WorksheetDock::retranslateUi() {
	m_initializing = true;

	//Geometry
	ui.cbOrientation->clear();
	ui.cbOrientation->addItem(i18n("portrait"));
	ui.cbOrientation->addItem(i18n("landscape"));

	ui.cbSize->clear();
	ui.cbSize->addItem(i18n("view size"));
	ui.cbSize->addItem(QString("A0"), QPrinter::A0);
	ui.cbSize->addItem(QString("A1"), QPrinter::A1);
	ui.cbSize->addItem(QString("A2"), QPrinter::A2);
	ui.cbSize->addItem(QString("A3"), QPrinter::A3);
	ui.cbSize->addItem(QString("A4"), QPrinter::A4);
	ui.cbSize->addItem(QString("A5"), QPrinter::A5);
	ui.cbSize->addItem(QString("A6"), QPrinter::A6);
	ui.cbSize->addItem(QString("A7"), QPrinter::A7);
	ui.cbSize->addItem(QString("A8"), QPrinter::A8);
	ui.cbSize->addItem(QString("A9"), QPrinter::A9);
	ui.cbSize->addItem(QString("B0"), QPrinter::B0);
	ui.cbSize->addItem(QString("B1"), QPrinter::B1);
	ui.cbSize->addItem(QString("B2"), QPrinter::B2);
	ui.cbSize->addItem(QString("B3"), QPrinter::B3);
	ui.cbSize->addItem(QString("B4"), QPrinter::B4);
	ui.cbSize->addItem(QString("B5"), QPrinter::B5);
	ui.cbSize->addItem(QString("B6"), QPrinter::B6);
	ui.cbSize->addItem(QString("B7"), QPrinter::B7);
	ui.cbSize->addItem(QString("B8"), QPrinter::B8);
	ui.cbSize->addItem(QString("B9"), QPrinter::B9);
	ui.cbSize->addItem(QString("B10"), QPrinter::B10);
	ui.cbSize->addItem(QString("C5E"), QPrinter::C5E);
	ui.cbSize->addItem(QString("DLE"), QPrinter::DLE);
	ui.cbSize->addItem(i18n("Executive"), QPrinter::Executive);
	ui.cbSize->addItem(i18n("Folio"), QPrinter::Folio);
	ui.cbSize->addItem(i18n("Ledger"), QPrinter::Ledger);
	ui.cbSize->addItem(i18n("Legal"), QPrinter::Legal);
	ui.cbSize->addItem(i18n("Letter"), QPrinter::Letter);
	ui.cbSize->addItem(i18n("Tabloid"), QPrinter::Tabloid);
	ui.cbSize->addItem(i18n("US Common #10 Envelope"), QPrinter::Comm10E);
	ui.cbSize->addItem(i18n("Custom"), QPrinter::Custom);
	ui.cbSize->insertSeparator(1);

	//Background
	ui.cbBackgroundType->clear();
	ui.cbBackgroundType->addItem(i18n("color"));
	ui.cbBackgroundType->addItem(i18n("image"));
	ui.cbBackgroundType->addItem(i18n("pattern"));

	ui.cbBackgroundColorStyle->clear();
	ui.cbBackgroundColorStyle->addItem(i18n("single color"));
	ui.cbBackgroundColorStyle->addItem(i18n("horizontal gradient"));
	ui.cbBackgroundColorStyle->addItem(i18n("vertical gradient"));
	ui.cbBackgroundColorStyle->addItem(i18n("diag. gradient (from top left)"));
	ui.cbBackgroundColorStyle->addItem(i18n("diag. gradient (from bottom left)"));
	ui.cbBackgroundColorStyle->addItem(i18n("radial gradient"));

	ui.cbBackgroundImageStyle->clear();
	ui.cbBackgroundImageStyle->addItem(i18n("scaled and cropped"));
	ui.cbBackgroundImageStyle->addItem(i18n("scaled"));
	ui.cbBackgroundImageStyle->addItem(i18n("scaled, keep proportions"));
	ui.cbBackgroundImageStyle->addItem(i18n("centered"));
	ui.cbBackgroundImageStyle->addItem(i18n("tiled"));
	ui.cbBackgroundImageStyle->addItem(i18n("center tiled"));
	GuiTools::updateBrushStyles(ui.cbBackgroundBrushStyle, Qt::SolidPattern);

	m_initializing = false;
}

// "General"-tab
void WorksheetDock::nameChanged() {
	if (m_initializing)
		return;

	m_worksheet->setName(ui.leName->text());
}

void WorksheetDock::commentChanged() {
	if (m_initializing)
		return;

	m_worksheet->setComment(ui.leComment->text());
}

void WorksheetDock::scaleContentChanged(bool scaled) {
	if (m_initializing)
		return;

	for (auto* worksheet : m_worksheetList)
		worksheet->setScaleContent(scaled);
}

void WorksheetDock::sizeChanged(int i) {
	int index = ui.cbSize->itemData(i).toInt();

	if (index==QPrinter::Custom) {
		ui.sbWidth->setEnabled(true);
		ui.sbHeight->setEnabled(true);
		ui.lOrientation->hide();
		ui.cbOrientation->hide();
	} else {
		ui.sbWidth->setEnabled(false);
		ui.sbHeight->setEnabled(false);
		if (i==0) { //no orientation available when using the complete view size (first item in the combox is selected)
			ui.lOrientation->hide();
			ui.cbOrientation->hide();
		} else {
			ui.lOrientation->show();
			ui.cbOrientation->show();
		}
	}

	if (m_initializing)
		return;

	if (i==0) {
		//use the complete view size (first item in the combox is selected)
		for (auto* worksheet : m_worksheetList)
			worksheet->setUseViewSize(true);
	} else if (index==QPrinter::Custom) {
		if (m_worksheet->useViewSize()) {
			for (auto* worksheet : m_worksheetList)
				worksheet->setUseViewSize(false);
		}
	} else {
		//determine the width and the height of the to be used predefined layout
		float w, h;
		if (ui.cbOrientation->currentIndex() == 0) {
			w=qt_paperSizes[index][0];
			h=qt_paperSizes[index][1];
		} else {
			w=qt_paperSizes[index][1];
			h=qt_paperSizes[index][0];
		}

		m_initializing = true;
		//w and h from qt_paperSizes above are in mm, in UI we show everything in cm
		ui.sbWidth->setValue(w/10);
		ui.sbHeight->setValue(h/10);
		m_initializing=false;

		w = Worksheet::convertToSceneUnits(w, Worksheet::Millimeter);
		h = Worksheet::convertToSceneUnits(h, Worksheet::Millimeter);
		for (auto* worksheet : m_worksheetList) {
			worksheet->setUseViewSize(false);
			worksheet->setPageRect(QRect(0,0,w,h));
		}
	}
}

void WorksheetDock::sizeChanged() {
	if (m_initializing)
		return;

	int w = Worksheet::convertToSceneUnits(ui.sbWidth->value(), Worksheet::Centimeter);
	int h = Worksheet::convertToSceneUnits(ui.sbHeight->value(), Worksheet::Centimeter);
	for (auto* worksheet : m_worksheetList)
		worksheet->setPageRect(QRect(0,0,w,h));
}

void WorksheetDock::orientationChanged(int index) {
	Q_UNUSED(index);
	if (m_initializing)
		return;

	this->sizeChanged(ui.cbSize->currentIndex());
}

// "Background"-tab
void WorksheetDock::backgroundTypeChanged(int index) {
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
		} else {
			ui.lBackgroundFirstColor->setText(i18n("First color"));
			ui.lBackgroundSecondColor->show();
			ui.kcbBackgroundSecondColor->show();
		}
	} else if (type == PlotArea::Image) {
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
		ui.leBackgroundFileName->show();
		ui.bOpen->show();
	} else if (type == PlotArea::Pattern) {
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
		ui.leBackgroundFileName->hide();
		ui.bOpen->hide();
	}

	if (m_initializing)
		return;

	for (auto* worksheet : m_worksheetList)
		worksheet->setBackgroundType(type);
}

void WorksheetDock::backgroundColorStyleChanged(int index) {
	PlotArea::BackgroundColorStyle style = (PlotArea::BackgroundColorStyle)index;

	if (style == PlotArea::SingleColor) {
		ui.lBackgroundFirstColor->setText(i18n("Color"));
		ui.lBackgroundSecondColor->hide();
		ui.kcbBackgroundSecondColor->hide();
	} else {
		ui.lBackgroundFirstColor->setText(i18n("First color"));
		ui.lBackgroundSecondColor->show();
		ui.kcbBackgroundSecondColor->show();
	}

	if (m_initializing)
		return;

	int size = m_worksheetList.size();
	if (size>1) {
		m_worksheet->beginMacro(i18n("%1 worksheets: background color style changed", size));
		for (auto* w : m_worksheetList)
			w->setBackgroundColorStyle(style);
		m_worksheet->endMacro();
	} else
		m_worksheet->setBackgroundColorStyle(style);
}

void WorksheetDock::backgroundImageStyleChanged(int index) {
	if (m_initializing)
		return;

	PlotArea::BackgroundImageStyle style = (PlotArea::BackgroundImageStyle)index;
	for (auto* worksheet : m_worksheetList)
		worksheet->setBackgroundImageStyle(style);
}

void WorksheetDock::backgroundBrushStyleChanged(int index) {
	if (m_initializing)
		return;

	Qt::BrushStyle style = (Qt::BrushStyle)index;
	for (auto* worksheet : m_worksheetList)
		worksheet->setBackgroundBrushStyle(style);
}

void WorksheetDock::backgroundFirstColorChanged(const QColor& c) {
	if (m_initializing)
		return;

	for (auto* worksheet : m_worksheetList)
		worksheet->setBackgroundFirstColor(c);
}

void WorksheetDock::backgroundSecondColorChanged(const QColor& c) {
	if (m_initializing)
		return;

	for (auto* worksheet : m_worksheetList)
		worksheet->setBackgroundSecondColor(c);
}

void WorksheetDock::backgroundOpacityChanged(int value) {
	if (m_initializing)
		return;

	float opacity = (float)value/100;
	for (auto* worksheet : m_worksheetList)
		worksheet->setBackgroundOpacity(opacity);
}

//"Layout"-tab
void WorksheetDock::layoutTopMarginChanged(double margin) {
	if (m_initializing)
		return;

	for (auto* worksheet : m_worksheetList)
		worksheet->setLayoutTopMargin(Worksheet::convertToSceneUnits(margin, Worksheet::Centimeter));
}

void WorksheetDock::layoutBottomMarginChanged(double margin) {
	if (m_initializing)
		return;

	for (auto* worksheet : m_worksheetList)
		worksheet->setLayoutBottomMargin(Worksheet::convertToSceneUnits(margin, Worksheet::Centimeter));
}

void WorksheetDock::layoutLeftMarginChanged(double margin) {
	if (m_initializing)
		return;

	for (auto* worksheet : m_worksheetList)
		worksheet->setLayoutLeftMargin(Worksheet::convertToSceneUnits(margin, Worksheet::Centimeter));
}

void WorksheetDock::layoutRightMarginChanged(double margin) {
	if (m_initializing)
		return;

	for (auto* worksheet : m_worksheetList)
		worksheet->setLayoutRightMargin(Worksheet::convertToSceneUnits(margin, Worksheet::Centimeter));
}

void WorksheetDock::layoutHorizontalSpacingChanged(double spacing) {
	if (m_initializing)
		return;

	for (auto* worksheet : m_worksheetList)
		worksheet->setLayoutHorizontalSpacing(Worksheet::convertToSceneUnits(spacing, Worksheet::Centimeter));
}

void WorksheetDock::layoutVerticalSpacingChanged(double spacing) {
	if (m_initializing)
		return;

	for (auto* worksheet : m_worksheetList)
		worksheet->setLayoutVerticalSpacing(Worksheet::convertToSceneUnits(spacing, Worksheet::Centimeter));
}

void WorksheetDock::layoutRowCountChanged(int count) {
	if (m_initializing)
		return;

	for (auto* worksheet : m_worksheetList)
		worksheet->setLayoutRowCount(count);
}

void WorksheetDock::layoutColumnCountChanged(int count) {
	if (m_initializing)
		return;

	for (auto* worksheet : m_worksheetList)
		worksheet->setLayoutColumnCount(count);
}

/*!
	opens a file dialog and lets the user select the image file.
*/
void WorksheetDock::selectFile() {
	KConfigGroup conf(KSharedConfig::openConfig(), "WorksheetDock");
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
	if (pos!=-1) {
		QString newDir = path.left(pos);
		if (newDir!=dir)
			conf.writeEntry("LastImageDir", newDir);
	}

	ui.leBackgroundFileName->setText( path );

	for (auto* worksheet : m_worksheetList)
		worksheet->setBackgroundFileName(path);
}

void WorksheetDock::fileNameChanged() {
	if (m_initializing)
		return;

	QString fileName = ui.leBackgroundFileName->text();
	if (!fileName.isEmpty() && !QFile::exists(fileName))
		ui.leBackgroundFileName->setStyleSheet("QLineEdit{background:red;}");
	else
		ui.leBackgroundFileName->setStyleSheet("");

	for (auto* worksheet : m_worksheetList)
		worksheet->setBackgroundFileName(fileName);
}

//*************************************************************
//******** SLOTs for changes triggered in Worksheet ***********
//*************************************************************
void WorksheetDock::worksheetDescriptionChanged(const AbstractAspect* aspect) {
	if (m_worksheet != aspect)
		return;

	m_initializing = true;
	if (aspect->name() != ui.leName->text())
		ui.leName->setText(aspect->name());
	else if (aspect->comment() != ui.leComment->text())
		ui.leComment->setText(aspect->comment());
	m_initializing = false;
}

void WorksheetDock::worksheetScaleContentChanged(bool scaled) {
	m_initializing = true;
	ui.chScaleContent->setChecked(scaled);
	m_initializing = false;
}

void WorksheetDock::worksheetPageRectChanged(const QRectF& rect) {
	m_initializing = true;
	ui.sbWidth->setValue(Worksheet::convertFromSceneUnits(rect.width(), Worksheet::Centimeter));
	ui.sbHeight->setValue(Worksheet::convertFromSceneUnits(rect.height(), Worksheet::Centimeter));
	updatePaperSize();
	m_initializing = false;
}

void WorksheetDock::worksheetBackgroundTypeChanged(PlotArea::BackgroundType type) {
	m_initializing = true;
	ui.cbBackgroundType->setCurrentIndex(type);
	m_initializing = false;
}

void WorksheetDock::worksheetBackgroundColorStyleChanged(PlotArea::BackgroundColorStyle style) {
	m_initializing = true;
	ui.cbBackgroundColorStyle->setCurrentIndex(style);
	m_initializing = false;
}

void WorksheetDock::worksheetBackgroundImageStyleChanged(PlotArea::BackgroundImageStyle style) {
	m_initializing = true;
	ui.cbBackgroundImageStyle->setCurrentIndex(style);
	m_initializing = false;
}

void WorksheetDock::worksheetBackgroundBrushStyleChanged(Qt::BrushStyle style) {
	m_initializing = true;
	ui.cbBackgroundBrushStyle->setCurrentIndex(style);
	m_initializing = false;
}

void WorksheetDock::worksheetBackgroundFirstColorChanged(const QColor& color) {
	m_initializing = true;
	ui.kcbBackgroundFirstColor->setColor(color);
	m_initializing = false;
}

void WorksheetDock::worksheetBackgroundSecondColorChanged(const QColor& color) {
	m_initializing = true;
	ui.kcbBackgroundSecondColor->setColor(color);
	m_initializing = false;
}

void WorksheetDock::worksheetBackgroundFileNameChanged(const QString& name) {
	m_initializing = true;
	ui.leBackgroundFileName->setText(name);
	m_initializing = false;
}

void WorksheetDock::worksheetBackgroundOpacityChanged(float opacity) {
	m_initializing = true;
	ui.sbBackgroundOpacity->setValue( qRound(opacity*100.0) );
	m_initializing = false;
}

void WorksheetDock::worksheetLayoutChanged(Worksheet::Layout layout) {
	bool b = (layout != Worksheet::NoLayout);
	ui.sbLayoutTopMargin->setEnabled(b);
	ui.sbLayoutBottomMargin->setEnabled(b);
	ui.sbLayoutLeftMargin->setEnabled(b);
	ui.sbLayoutRightMargin->setEnabled(b);
	ui.sbLayoutHorizontalSpacing->setEnabled(b);
	ui.sbLayoutVerticalSpacing->setEnabled(b);
	ui.sbLayoutRowCount->setEnabled(b);
	ui.sbLayoutColumnCount->setEnabled(b);

	if (b) {
		bool grid = (layout == Worksheet::GridLayout);
		ui.lGrid->setVisible(grid);
		ui.lRowCount->setVisible(grid);
		ui.sbLayoutRowCount->setVisible(grid);
		ui.lColumnCount->setVisible(grid);
		ui.sbLayoutColumnCount->setVisible(grid);
	} else {
		ui.lGrid->setVisible(true);
		ui.lRowCount->setVisible(true);
		ui.sbLayoutRowCount->setVisible(true);
		ui.lColumnCount->setVisible(true);
		ui.sbLayoutColumnCount->setVisible(true);
	}
}

void WorksheetDock::worksheetLayoutTopMarginChanged(float value) {
	m_initializing = true;
	ui.sbLayoutTopMargin->setValue(Worksheet::convertFromSceneUnits(value, Worksheet::Centimeter));
	m_initializing = false;
}

void WorksheetDock::worksheetLayoutBottomMarginChanged(float value) {
	m_initializing = true;
	ui.sbLayoutBottomMargin->setValue(Worksheet::convertFromSceneUnits(value, Worksheet::Centimeter));
	m_initializing = false;
}

void WorksheetDock::worksheetLayoutLeftMarginChanged(float value) {
	m_initializing = true;
	ui.sbLayoutLeftMargin->setValue(Worksheet::convertFromSceneUnits(value, Worksheet::Centimeter));
	m_initializing = false;
}

void WorksheetDock::worksheetLayoutRightMarginChanged(float value) {
	m_initializing = true;
	ui.sbLayoutRightMargin->setValue(Worksheet::convertFromSceneUnits(value, Worksheet::Centimeter));
	m_initializing = false;
}

void WorksheetDock::worksheetLayoutVerticalSpacingChanged(float value) {
	m_initializing = true;
	ui.sbLayoutVerticalSpacing->setValue(Worksheet::convertFromSceneUnits(value, Worksheet::Centimeter));
	m_initializing = false;
}

void WorksheetDock::worksheetLayoutHorizontalSpacingChanged(float value) {
	m_initializing = true;
	ui.sbLayoutHorizontalSpacing->setValue(Worksheet::convertFromSceneUnits(value, Worksheet::Centimeter));
	m_initializing = false;
}

void WorksheetDock::worksheetLayoutRowCountChanged(int value) {
	m_initializing = true;
	ui.sbLayoutRowCount->setValue(value);
	m_initializing = false;
}

void WorksheetDock::worksheetLayoutColumnCountChanged(int value) {
	m_initializing = true;
	ui.sbLayoutColumnCount->setValue(value);
	m_initializing = false;
}

//*************************************************************
//******************** SETTINGS *******************************
//*************************************************************
void WorksheetDock::load() {
	// Geometry
	ui.chScaleContent->setChecked(m_worksheet->scaleContent());
	ui.sbWidth->setValue(Worksheet::convertFromSceneUnits( m_worksheet->pageRect().width(), Worksheet::Centimeter) );
	ui.sbHeight->setValue(Worksheet::convertFromSceneUnits( m_worksheet->pageRect().height(), Worksheet::Centimeter) );
	updatePaperSize();

	// Background-tab
	ui.cbBackgroundType->setCurrentIndex( (int) m_worksheet->backgroundType() );
	ui.cbBackgroundColorStyle->setCurrentIndex( (int) m_worksheet->backgroundColorStyle() );
	ui.cbBackgroundImageStyle->setCurrentIndex( (int) m_worksheet->backgroundImageStyle() );
	ui.cbBackgroundBrushStyle->setCurrentIndex( (int) m_worksheet->backgroundBrushStyle() );
	ui.leBackgroundFileName->setText( m_worksheet->backgroundFileName() );
	ui.kcbBackgroundFirstColor->setColor( m_worksheet->backgroundFirstColor() );
	ui.kcbBackgroundSecondColor->setColor( m_worksheet->backgroundSecondColor() );
	ui.sbBackgroundOpacity->setValue( qRound(m_worksheet->backgroundOpacity()*100) );

	//highlight the text field for the background image red if an image is used and cannot be found
	if (!m_worksheet->backgroundFileName().isEmpty() && !QFile::exists(m_worksheet->backgroundFileName()))
		ui.leBackgroundFileName->setStyleSheet("QLineEdit{background:red;}");
	else
		ui.leBackgroundFileName->setStyleSheet("");

	// Layout
	ui.sbLayoutTopMargin->setValue( Worksheet::convertFromSceneUnits(m_worksheet->layoutTopMargin(), Worksheet::Centimeter) );
	ui.sbLayoutBottomMargin->setValue( Worksheet::convertFromSceneUnits(m_worksheet->layoutBottomMargin(), Worksheet::Centimeter) );
	ui.sbLayoutLeftMargin->setValue( Worksheet::convertFromSceneUnits(m_worksheet->layoutLeftMargin(), Worksheet::Centimeter) );
	ui.sbLayoutRightMargin->setValue( Worksheet::convertFromSceneUnits(m_worksheet->layoutRightMargin(), Worksheet::Centimeter) );
	ui.sbLayoutHorizontalSpacing->setValue( Worksheet::convertFromSceneUnits(m_worksheet->layoutHorizontalSpacing(), Worksheet::Centimeter) );
	ui.sbLayoutVerticalSpacing->setValue( Worksheet::convertFromSceneUnits(m_worksheet->layoutVerticalSpacing(), Worksheet::Centimeter) );

	ui.sbLayoutRowCount->setValue( m_worksheet->layoutRowCount() );
	ui.sbLayoutColumnCount->setValue( m_worksheet->layoutColumnCount() );
}

void WorksheetDock::loadConfigFromTemplate(KConfig& config) {
	//extract the name of the template from the file name
	QString name;
	int index = config.name().lastIndexOf(QDir::separator());
	if (index!=-1)
		name = config.name().right(config.name().size() - index - 1);
	else
		name = config.name();

	int size = m_worksheetList.size();
	if (size>1)
		m_worksheet->beginMacro(i18n("%1 worksheets: template \"%2\" loaded", size, name));
	else
		m_worksheet->beginMacro(i18n("%1: template \"%2\" loaded", m_worksheet->name(), name));

	this->loadConfig(config);
	m_worksheet->endMacro();
}

void WorksheetDock::loadConfig(KConfig& config) {
	KConfigGroup group = config.group( "Worksheet" );

	// Geometry
	ui.chScaleContent->setChecked(group.readEntry("ScaleContent", false));
	ui.sbWidth->setValue(Worksheet::convertFromSceneUnits(group.readEntry("Width", m_worksheet->pageRect().width()), Worksheet::Centimeter));
	ui.sbHeight->setValue(Worksheet::convertFromSceneUnits(group.readEntry("Height", m_worksheet->pageRect().height()), Worksheet::Centimeter));
	updatePaperSize();

	// Background-tab
	ui.cbBackgroundType->setCurrentIndex( group.readEntry("BackgroundType", (int) m_worksheet->backgroundType()) );
	ui.cbBackgroundColorStyle->setCurrentIndex( group.readEntry("BackgroundColorStyle", (int) m_worksheet->backgroundColorStyle()) );
	ui.cbBackgroundImageStyle->setCurrentIndex( group.readEntry("BackgroundImageStyle", (int) m_worksheet->backgroundImageStyle()) );
	ui.cbBackgroundBrushStyle->setCurrentIndex( group.readEntry("BackgroundBrushStyle", (int) m_worksheet->backgroundBrushStyle()) );
	ui.leBackgroundFileName->setText( group.readEntry("BackgroundFileName", m_worksheet->backgroundFileName()) );
	ui.kcbBackgroundFirstColor->setColor( group.readEntry("BackgroundFirstColor", m_worksheet->backgroundFirstColor()) );
	ui.kcbBackgroundSecondColor->setColor( group.readEntry("BackgroundSecondColor", m_worksheet->backgroundSecondColor()) );
	ui.sbBackgroundOpacity->setValue( qRound(group.readEntry("BackgroundOpacity", m_worksheet->backgroundOpacity())*100) );

	// Layout
	ui.sbLayoutTopMargin->setValue(group.readEntry("LayoutTopMargin", Worksheet::convertFromSceneUnits(m_worksheet->layoutTopMargin(), Worksheet::Centimeter)) );
	ui.sbLayoutBottomMargin->setValue(group.readEntry("LayoutBottomMargin", Worksheet::convertFromSceneUnits(m_worksheet->layoutBottomMargin(), Worksheet::Centimeter)) );
	ui.sbLayoutLeftMargin->setValue(group.readEntry("LayoutLeftMargin", Worksheet::convertFromSceneUnits(m_worksheet->layoutLeftMargin(), Worksheet::Centimeter)) );
	ui.sbLayoutRightMargin->setValue(group.readEntry("LayoutRightMargin", Worksheet::convertFromSceneUnits(m_worksheet->layoutRightMargin(), Worksheet::Centimeter)) );
	ui.sbLayoutHorizontalSpacing->setValue(group.readEntry("LayoutHorizontalSpacing", Worksheet::convertFromSceneUnits(m_worksheet->layoutHorizontalSpacing(), Worksheet::Centimeter)) );
	ui.sbLayoutVerticalSpacing->setValue(group.readEntry("LayoutVerticalSpacing", Worksheet::convertFromSceneUnits(m_worksheet->layoutVerticalSpacing(), Worksheet::Centimeter)) );

	ui.sbLayoutRowCount->setValue(group.readEntry("LayoutRowCount", m_worksheet->layoutRowCount()));
	ui.sbLayoutColumnCount->setValue(group.readEntry("LayoutColumnCount", m_worksheet->layoutColumnCount()));
}

void WorksheetDock::saveConfigAsTemplate(KConfig& config) {
	KConfigGroup group = config.group( "Worksheet" );

	//General
	group.writeEntry("ScaleContent",ui.chScaleContent->isChecked());
	group.writeEntry("UseViewSize",ui.cbSize->currentIndex()==0);
	group.writeEntry("Width",Worksheet::convertToSceneUnits(ui.sbWidth->value(), Worksheet::Centimeter));
	group.writeEntry("Height",Worksheet::convertToSceneUnits(ui.sbHeight->value(), Worksheet::Centimeter));

	//Background
	group.writeEntry("BackgroundType",ui.cbBackgroundType->currentIndex());
	group.writeEntry("BackgroundColorStyle", ui.cbBackgroundColorStyle->currentIndex());
	group.writeEntry("BackgroundImageStyle", ui.cbBackgroundImageStyle->currentIndex());
	group.writeEntry("BackgroundBrushStyle", ui.cbBackgroundBrushStyle->currentIndex());
	group.writeEntry("BackgroundFileName", ui.leBackgroundFileName->text());
	group.writeEntry("BackgroundFirstColor", ui.kcbBackgroundFirstColor->color());
	group.writeEntry("BackgroundSecondColor", ui.kcbBackgroundSecondColor->color());
	group.writeEntry("BackgroundOpacity", ui.sbBackgroundOpacity->value()/100.0);

	//Layout
	group.writeEntry("LayoutTopMargin",Worksheet::convertToSceneUnits(ui.sbLayoutTopMargin->value(), Worksheet::Centimeter));
	group.writeEntry("LayoutBottomMargin",Worksheet::convertToSceneUnits(ui.sbLayoutBottomMargin->value(), Worksheet::Centimeter));
	group.writeEntry("LayoutLeftMargin",Worksheet::convertToSceneUnits(ui.sbLayoutLeftMargin->value(), Worksheet::Centimeter));
	group.writeEntry("LayoutRightMargin",Worksheet::convertToSceneUnits(ui.sbLayoutRightMargin->value(), Worksheet::Centimeter));
	group.writeEntry("LayoutVerticalSpacing",Worksheet::convertToSceneUnits(ui.sbLayoutVerticalSpacing->value(), Worksheet::Centimeter));
	group.writeEntry("LayoutHorizontalSpacing",Worksheet::convertToSceneUnits(ui.sbLayoutHorizontalSpacing->value(), Worksheet::Centimeter));
	group.writeEntry("LayoutRowCount", ui.sbLayoutRowCount->value());
	group.writeEntry("LayoutColumnCount", ui.sbLayoutColumnCount->value());

	config.sync();
}

void WorksheetDock::loadTheme(const QString& theme) {
	for (auto* worksheet : m_worksheetList)
		worksheet->setTheme(theme);
}
