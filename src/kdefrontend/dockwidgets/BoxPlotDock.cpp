/***************************************************************************
    File                 : BoxPlotDock.cpp
    Project              : LabPlot
    Description          : Dock widget for the reference line on the plot
    --------------------------------------------------------------------
    Copyright            : (C) 2020 Alexander Semke (alexander.semke@web.de)
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

#include "BoxPlotDock.h"
#include "backend/core/AspectTreeModel.h"
#include "backend/core/Project.h"
#include "backend/worksheet/Worksheet.h"
#include "backend/worksheet/plots/cartesian/BoxPlot.h"
#include "commonfrontend/widgets/TreeViewComboBox.h"

#include "kdefrontend/TemplateHandler.h"
#include "kdefrontend/GuiTools.h"

#include <QFileDialog>
#include <QImageReader>

#include <KLocalizedString>
#include <KConfig>

BoxPlotDock::BoxPlotDock(QWidget* parent) : BaseDock(parent), cbDataColumn(new TreeViewComboBox) {
	ui.setupUi(this);
	m_leName = ui.leName;
	m_leComment = ui.leComment;

	// Tab "General"
	auto* gridLayout = qobject_cast<QGridLayout*>(ui.tabGeneral->layout());
	gridLayout->addWidget(cbDataColumn, 4, 2, 1, 1);

	ui.cbWhiskersType->addItem(i18n("min/max"));
	ui.cbWhiskersType->addItem(i18n("1.5 IQR"));
	ui.cbWhiskersType->addItem(i18n("1 stddev"));
	ui.cbWhiskersType->addItem(i18n("1/99 percentiles"));
	ui.cbWhiskersType->addItem(i18n("2/98 percentiles"));

	ui.cbOrientation->addItem(i18n("Horizontal"));
	ui.cbOrientation->addItem(i18n("Vertical"));

	//"Box"-tab
	//filling
	ui.cbFillingType->addItem(i18n("Color"));
	ui.cbFillingType->addItem(i18n("Image"));
	ui.cbFillingType->addItem(i18n("Pattern"));

	ui.cbFillingColorStyle->addItem(i18n("Single Color"));
	ui.cbFillingColorStyle->addItem(i18n("Horizontal Gradient"));
	ui.cbFillingColorStyle->addItem(i18n("Vertical Gradient"));
	ui.cbFillingColorStyle->addItem(i18n("Diag. Gradient (From Top Left)"));
	ui.cbFillingColorStyle->addItem(i18n("Diag. Gradient (From Bottom Left)"));
	ui.cbFillingColorStyle->addItem(i18n("Radial Gradient"));

	ui.cbFillingImageStyle->addItem(i18n("Scaled and Cropped"));
	ui.cbFillingImageStyle->addItem(i18n("Scaled"));
	ui.cbFillingImageStyle->addItem(i18n("Scaled, Keep Proportions"));
	ui.cbFillingImageStyle->addItem(i18n("Centered"));
	ui.cbFillingImageStyle->addItem(i18n("Tiled"));
	ui.cbFillingImageStyle->addItem(i18n("Center Tiled"));
	GuiTools::updateBrushStyles(ui.cbFillingBrushStyle, Qt::SolidPattern);

	ui.cbFillingColorStyle->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLengthWithIcon);
	ui.bFillingOpen->setIcon( QIcon::fromTheme("document-open") );

	//box border
	GuiTools::updatePenStyles(ui.cbBorderStyle, Qt::black);

	//adjust layouts in the tabs
	for (int i = 0; i < ui.tabWidget->count(); ++i) {
		auto* layout = dynamic_cast<QGridLayout*>(ui.tabWidget->widget(i)->layout());
		if (!layout)
			continue;

		layout->setContentsMargins(2,2,2,2);
		layout->setHorizontalSpacing(2);
		layout->setVerticalSpacing(2);
	}

	//Validators

	//set the current locale
	updateLocale();

	//SLOTS
	//General
	connect(ui.leName, &QLineEdit::textChanged, this, &BoxPlotDock::nameChanged);
	connect(ui.leComment, &QLineEdit::textChanged, this, &BoxPlotDock::commentChanged);
	connect(cbDataColumn, &TreeViewComboBox::currentModelIndexChanged, this, &BoxPlotDock::dataColumnChanged);

	//box filling
	connect(ui.chkFillingEnabled, &QCheckBox::stateChanged, this, &BoxPlotDock::fillingEnabledChanged);
	connect(ui.cbFillingType, QOverload<int>::of(&QComboBox::currentIndexChanged),
			 this, &BoxPlotDock::boxFillingTypeChanged);
	connect(ui.cbFillingColorStyle, QOverload<int>::of(&QComboBox::currentIndexChanged),
			 this, &BoxPlotDock::boxFillingColorStyleChanged);
	connect(ui.cbFillingImageStyle, QOverload<int>::of(&QComboBox::currentIndexChanged),
			 this, &BoxPlotDock::boxFillingImageStyleChanged);
	connect(ui.cbFillingBrushStyle, QOverload<int>::of(&QComboBox::currentIndexChanged),
			 this, &BoxPlotDock::boxFillingBrushStyleChanged);
	connect(ui.bFillingOpen, &QPushButton::clicked, this, &BoxPlotDock::selectFile);
	connect(ui.leFillingFileName, &QLineEdit::returnPressed, this, &BoxPlotDock::fileNameChanged);
	connect(ui.leFillingFileName, &QLineEdit::textChanged, this, &BoxPlotDock::fileNameChanged);
	connect(ui.kcbFillingFirstColor, &KColorButton::changed, this, &BoxPlotDock::boxFillingFirstColorChanged);
	connect(ui.kcbFillingSecondColor, &KColorButton::changed, this, &BoxPlotDock::boxFillingSecondColorChanged);
	connect(ui.sbFillingOpacity, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged),
			this, &BoxPlotDock::boxFillingOpacityChanged);

	//box border
	connect(ui.cbBorderStyle, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &BoxPlotDock::borderStyleChanged);
	connect(ui.kcbBorderColor, &KColorButton::changed, this, &BoxPlotDock::borderColorChanged);
	connect(ui.sbBorderWidth, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &BoxPlotDock::borderWidthChanged);
	connect(ui.sbBorderOpacity, QOverload<int>::of(&QSpinBox::valueChanged), this, &BoxPlotDock::borderOpacityChanged);

	//whiskers

	//template handler
	auto* frame = new QFrame(this);
	auto* layout = new QHBoxLayout(frame);
	layout->setContentsMargins(0, 11, 0, 11);

	auto* templateHandler = new TemplateHandler(this, TemplateHandler::ClassName::Worksheet);
	layout->addWidget(templateHandler);
	connect(templateHandler, &TemplateHandler::loadConfigRequested, this, &BoxPlotDock::loadConfigFromTemplate);
	connect(templateHandler, &TemplateHandler::saveConfigRequested, this, &BoxPlotDock::saveConfigAsTemplate);
	connect(templateHandler, &TemplateHandler::info, this, &BoxPlotDock::info);

	ui.verticalLayout->addWidget(frame);
}

void BoxPlotDock::setBoxPlots(QList<BoxPlot*> list) {
	const Lock lock(m_initializing);
	m_boxPlots = list;
	m_boxPlot = list.first();
	m_aspect = list.first();
	Q_ASSERT(m_boxPlot);
	m_aspectTreeModel = new AspectTreeModel(m_boxPlot->project());
	setModel();

	//if there is more then one point in the list, disable the comment and name widgets in "general"
	if (list.size() == 1) {
		ui.lName->setEnabled(true);
		ui.leName->setEnabled(true);
		ui.lComment->setEnabled(true);
		ui.leComment->setEnabled(true);
		ui.leName->setText(m_boxPlot->name());
		ui.leComment->setText(m_boxPlot->comment());

		ui.lDataColumn->setEnabled(true);
		cbDataColumn->setEnabled(true);
		this->setModelIndexFromColumn(cbDataColumn, m_boxPlot->dataColumn());
	} else {
		ui.lName->setEnabled(false);
		ui.leName->setEnabled(false);
		ui.lComment->setEnabled(false);
		ui.leComment->setEnabled(false);
		ui.leName->setText(QString());
		ui.leComment->setText(QString());

		ui.lDataColumn->setEnabled(false);
		cbDataColumn->setEnabled(false);
		cbDataColumn->setCurrentModelIndex(QModelIndex());
	}
	ui.leName->setStyleSheet("");
	ui.leName->setToolTip("");

	//show the properties of the first box plot
	KConfig config(QString(), KConfig::SimpleConfig);
	loadConfig(config);

	//SIGNALs/SLOTs
	//general
	connect(m_boxPlot, &AbstractAspect::aspectDescriptionChanged,this, &BoxPlotDock::plotDescriptionChanged);
	connect(m_boxPlot, &BoxPlot::visibilityChanged, this, &BoxPlotDock::plotVisibilityChanged);
	connect(m_boxPlot, &BoxPlot::dataColumnChanged, this, &BoxPlotDock::plotDataColumnChanged);

	//box filling
	connect(m_boxPlot, &BoxPlot::fillingEnabledChanged, this, &BoxPlotDock::plotFillingEnabledChanged);
	connect(m_boxPlot, &BoxPlot::fillingTypeChanged, this, &BoxPlotDock::plotFillingTypeChanged);
	connect(m_boxPlot, &BoxPlot::fillingColorStyleChanged, this, &BoxPlotDock::plotFillingColorStyleChanged);
	connect(m_boxPlot, &BoxPlot::fillingImageStyleChanged, this, &BoxPlotDock::plotFillingImageStyleChanged);
	connect(m_boxPlot, &BoxPlot::fillingBrushStyleChanged, this, &BoxPlotDock::plotFillingBrushStyleChanged);
	connect(m_boxPlot, &BoxPlot::fillingFirstColorChanged, this, &BoxPlotDock::plotFillingFirstColorChanged);
	connect(m_boxPlot, &BoxPlot::fillingSecondColorChanged, this, &BoxPlotDock::plotFillingSecondColorChanged);
	connect(m_boxPlot, &BoxPlot::fillingFileNameChanged, this, &BoxPlotDock::plotFillingFileNameChanged);
	connect(m_boxPlot, &BoxPlot::fillingOpacityChanged, this, &BoxPlotDock::plotFillingOpacityChanged);

	//box border
	connect(m_boxPlot, &BoxPlot::borderPenChanged, this, &BoxPlotDock::plotBorderPenChanged);
	connect(m_boxPlot, &BoxPlot::borderOpacityChanged, this, &BoxPlotDock::plotBorderOpacityChanged);
}

void BoxPlotDock::setModel() {
	m_aspectTreeModel->enablePlottableColumnsOnly(true);
	m_aspectTreeModel->enableShowPlotDesignation(true);

	QList<AspectType> list{AspectType::Folder, AspectType::Workbook, AspectType::Datapicker,
	                       AspectType::DatapickerCurve, AspectType::Spreadsheet, AspectType::LiveDataSource,
	                       AspectType::Column, AspectType::Worksheet, AspectType::CartesianPlot,
	                       AspectType::XYFitCurve, AspectType::XYSmoothCurve, AspectType::CantorWorksheet};

	cbDataColumn->setTopLevelClasses(list);

	list = {AspectType::Column};
	m_aspectTreeModel->setSelectableAspects(list);

	cbDataColumn->setModel(m_aspectTreeModel);
}

/*
 * updates the locale in the widgets. called when the application settins are changed.
 */
void BoxPlotDock::updateLocale() {
	SET_NUMBER_LOCALE
	ui.sbBorderWidth->setLocale(numberLocale);

// 	Lock lock(m_initializing);
// 	ui.lePosition->setText(numberLocale.toString(m_boxPlot->position()));
}

void BoxPlotDock::setModelIndexFromColumn(TreeViewComboBox* cb, const AbstractColumn* column) {
	if (column)
		cb->setCurrentModelIndex(m_aspectTreeModel->modelIndexOfAspect(column));
	else
		cb->setCurrentModelIndex(QModelIndex());
}

//**********************************************************
//*** SLOTs for changes triggered in BoxPlotDock *****
//**********************************************************
void BoxPlotDock::dataColumnChanged(const QModelIndex& index) {
	if (m_initializing)
		return;

	auto aspect = static_cast<AbstractAspect*>(index.internalPointer());
	AbstractColumn* column(nullptr);
	if (aspect) {
		column = dynamic_cast<AbstractColumn*>(aspect);
		Q_ASSERT(column);
	}

	for (auto* boxPlot : m_boxPlots)
		boxPlot->setDataColumn(column);
}

void BoxPlotDock::visibilityChanged(bool state) {
	if (m_initializing)
		return;

	for (auto* boxPlot : m_boxPlots)
		boxPlot->setVisible(state);
}

//"Box"-tab
//box filling
void BoxPlotDock::fillingEnabledChanged(int state) {
	ui.cbFillingType->setEnabled(state);
	ui.cbFillingColorStyle->setEnabled(state);
	ui.cbFillingBrushStyle->setEnabled(state);
	ui.cbFillingImageStyle->setEnabled(state);
	ui.kcbFillingFirstColor->setEnabled(state);
	ui.kcbFillingSecondColor->setEnabled(state);
	ui.leFillingFileName->setEnabled(state);
	ui.bFillingOpen->setEnabled(state);
	ui.sbFillingOpacity->setEnabled(state);

	if (m_initializing)
		return;

	for (auto* boxPlot : m_boxPlots)
		boxPlot->setFillingEnabled(state);
}

void BoxPlotDock::boxFillingTypeChanged(int index) {
	if (index == -1)
		return;

	auto type = (PlotArea::BackgroundType)index;

	if (type == PlotArea::BackgroundType::Color) {
		ui.lFillingColorStyle->show();
		ui.cbFillingColorStyle->show();
		ui.lFillingImageStyle->hide();
		ui.cbFillingImageStyle->hide();
		ui.lFillingBrushStyle->hide();
		ui.cbFillingBrushStyle->hide();

		ui.lFillingFileName->hide();
		ui.leFillingFileName->hide();
		ui.bFillingOpen->hide();

		ui.lFillingFirstColor->show();
		ui.kcbFillingFirstColor->show();

		auto style = (PlotArea::BackgroundColorStyle)ui.cbFillingColorStyle->currentIndex();
		if (style == PlotArea::BackgroundColorStyle::SingleColor) {
			ui.lFillingFirstColor->setText(i18n("Color:"));
			ui.lFillingSecondColor->hide();
			ui.kcbFillingSecondColor->hide();
		} else {
			ui.lFillingFirstColor->setText(i18n("First color:"));
			ui.lFillingSecondColor->show();
			ui.kcbFillingSecondColor->show();
		}
	} else if (type == PlotArea::BackgroundType::Image) {
		ui.lFillingFirstColor->hide();
		ui.kcbFillingFirstColor->hide();
		ui.lFillingSecondColor->hide();
		ui.kcbFillingSecondColor->hide();

		ui.lFillingColorStyle->hide();
		ui.cbFillingColorStyle->hide();
		ui.lFillingImageStyle->show();
		ui.cbFillingImageStyle->show();
		ui.lFillingBrushStyle->hide();
		ui.cbFillingBrushStyle->hide();
		ui.lFillingFileName->show();
		ui.leFillingFileName->show();
		ui.bFillingOpen->show();
	} else if (type == PlotArea::BackgroundType::Pattern) {
		ui.lFillingFirstColor->setText(i18n("Color:"));
		ui.lFillingFirstColor->show();
		ui.kcbFillingFirstColor->show();
		ui.lFillingSecondColor->hide();
		ui.kcbFillingSecondColor->hide();

		ui.lFillingColorStyle->hide();
		ui.cbFillingColorStyle->hide();
		ui.lFillingImageStyle->hide();
		ui.cbFillingImageStyle->hide();
		ui.lFillingBrushStyle->show();
		ui.cbFillingBrushStyle->show();
		ui.lFillingFileName->hide();
		ui.leFillingFileName->hide();
		ui.bFillingOpen->hide();
	}

	if (m_initializing)
		return;

	for (auto* boxPlot : m_boxPlots)
		boxPlot->setFillingType(type);
}

void BoxPlotDock::boxFillingColorStyleChanged(int index) {
	if (index == -1)
		return;

	auto style = (PlotArea::BackgroundColorStyle)index;

	if (style == PlotArea::BackgroundColorStyle::SingleColor) {
		ui.lFillingFirstColor->setText(i18n("Color:"));
		ui.lFillingSecondColor->hide();
		ui.kcbFillingSecondColor->hide();
	} else {
		ui.lFillingFirstColor->setText(i18n("First color:"));
		ui.lFillingSecondColor->show();
		ui.kcbFillingSecondColor->show();
	}

	if (m_initializing)
		return;

	for (auto* boxPlot : m_boxPlots)
		boxPlot->setFillingColorStyle(style);
}

void BoxPlotDock::boxFillingImageStyleChanged(int index) {
	if (m_initializing)
		return;

	auto style = (PlotArea::BackgroundImageStyle)index;
	for (auto* boxPlot : m_boxPlots)
		boxPlot->setFillingImageStyle(style);
}

void BoxPlotDock::boxFillingBrushStyleChanged(int index) {
	if (m_initializing)
		return;

	auto style = (Qt::BrushStyle)index;
	for (auto* boxPlot : m_boxPlots)
		boxPlot->setFillingBrushStyle(style);
}

void BoxPlotDock::boxFillingFirstColorChanged(const QColor& c) {
	if (m_initializing)
		return;

	for (auto* boxPlot : m_boxPlots)
		boxPlot->setFillingFirstColor(c);
}

void BoxPlotDock::boxFillingSecondColorChanged(const QColor& c) {
	if (m_initializing)
		return;

	for (auto* boxPlot : m_boxPlots)
		boxPlot->setFillingSecondColor(c);
}

/*!
	opens a file dialog and lets the user select the image file.
*/
void BoxPlotDock::selectFile() {
	KConfigGroup conf(KSharedConfig::openConfig(), "BoxPlotDock");
	QString dir = conf.readEntry("LastImageDir", "");

	QString formats;
	for (const QByteArray& format : QImageReader::supportedImageFormats()) {
		QString f = "*." + QString(format.constData());
		if (f == QLatin1String("*.svg"))
			continue;
		formats.isEmpty() ? formats += f : formats += ' ' + f;
	}

	QString path = QFileDialog::getOpenFileName(this, i18n("Select the image file"), dir, i18n("Images (%1)", formats));
	if (path.isEmpty())
		return; //cancel was clicked in the file-dialog

	int pos = path.lastIndexOf(QLatin1String("/"));
	if (pos != -1) {
		QString newDir = path.left(pos);
		if (newDir != dir)
			conf.writeEntry("LastImageDir", newDir);
	}

	ui.leFillingFileName->setText( path );

	for (auto* boxPlot : m_boxPlots)
		boxPlot->setFillingFileName(path);
}

void BoxPlotDock::fileNameChanged() {
	if (m_initializing)
		return;

	QString fileName = ui.leFillingFileName->text();
	for (auto* boxPlot : m_boxPlots)
		boxPlot->setFillingFileName(fileName);
}

void BoxPlotDock::boxFillingOpacityChanged(int value) {
	if (m_initializing)
		return;

	float opacity = (float)value/100;
	for (auto* boxPlot : m_boxPlots)
		boxPlot->setFillingOpacity(opacity);
}

//box border
void BoxPlotDock::borderStyleChanged(int index) {
	if (m_initializing)
		return;

	auto penStyle = Qt::PenStyle(index);
	QPen pen;
	for (auto* boxPlot : m_boxPlots) {
		pen = boxPlot->borderPen();
		pen.setStyle(penStyle);
		boxPlot->setBorderPen(pen);
	}
}

void BoxPlotDock::borderColorChanged(const QColor& color) {
	if (m_initializing)
		return;

	QPen pen;
	for (auto* boxPlot : m_boxPlots) {
		pen = boxPlot->borderPen();
		pen.setColor(color);
		boxPlot->setBorderPen(pen);
	}

	m_initializing = true;
	GuiTools::updatePenStyles(ui.cbBorderStyle, color);
	m_initializing = false;
}

void BoxPlotDock::borderWidthChanged(double value) {
	if (m_initializing)
		return;

	QPen pen;
	for (auto* boxPlot : m_boxPlots) {
		pen = boxPlot->borderPen();
		pen.setWidthF( Worksheet::convertToSceneUnits(value, Worksheet::Unit::Point) );
		boxPlot->setBorderPen(pen);
	}
}

void BoxPlotDock::borderOpacityChanged(int value) {
	if (m_initializing)
		return;

	qreal opacity = (float)value/100.;
	for (auto* boxPlot : m_boxPlots)
		boxPlot->setBorderOpacity(opacity);
}

//*************************************************************
//******* SLOTs for changes triggered in BoxPlot ********
//*************************************************************
//general
void BoxPlotDock::plotDescriptionChanged(const AbstractAspect* aspect) {
	if (m_boxPlot != aspect)
		return;

	m_initializing = true;
	if (aspect->name() != ui.leName->text())
		ui.leName->setText(aspect->name());
	else if (aspect->comment() != ui.leComment->text())
		ui.leComment->setText(aspect->comment());

	m_initializing = false;
}

void BoxPlotDock::plotDataColumnChanged(const AbstractColumn* column) {
	Lock lock(m_initializing);
	this->setModelIndexFromColumn(cbDataColumn, column);
}
void BoxPlotDock::plotVisibilityChanged(bool on) {
	Lock lock(m_initializing);
	ui.chkVisible->setChecked(on);
}

//box filling
//Filling
void BoxPlotDock::plotFillingEnabledChanged(bool status) {
	Lock lock(m_initializing);
	ui.chkFillingEnabled->setChecked(status);
}
void BoxPlotDock::plotFillingTypeChanged(PlotArea::BackgroundType type) {
	Lock lock(m_initializing);
	ui.cbFillingType->setCurrentIndex(static_cast<int>(type));
}
void BoxPlotDock::plotFillingColorStyleChanged(PlotArea::BackgroundColorStyle style) {
	Lock lock(m_initializing);
	ui.cbFillingColorStyle->setCurrentIndex(static_cast<int>(style));
}
void BoxPlotDock::plotFillingImageStyleChanged(PlotArea::BackgroundImageStyle style) {
	Lock lock(m_initializing);
	ui.cbFillingImageStyle->setCurrentIndex(static_cast<int>(style));
}
void BoxPlotDock::plotFillingBrushStyleChanged(Qt::BrushStyle style) {
	Lock lock(m_initializing);
	ui.cbFillingBrushStyle->setCurrentIndex(style);
}
void BoxPlotDock::plotFillingFirstColorChanged(QColor& color) {
	Lock lock(m_initializing);
	ui.kcbFillingFirstColor->setColor(color);
}
void BoxPlotDock::plotFillingSecondColorChanged(QColor& color) {
	Lock lock(m_initializing);
	ui.kcbFillingSecondColor->setColor(color);
}
void BoxPlotDock::plotFillingFileNameChanged(QString& filename) {
	Lock lock(m_initializing);
	ui.leFillingFileName->setText(filename);
}
void BoxPlotDock::plotFillingOpacityChanged(double opacity) {
	Lock lock(m_initializing);
	ui.sbFillingOpacity->setValue( round(opacity*100.0) );
}

//box border
void BoxPlotDock::plotBorderPenChanged(QPen& pen) {
	Lock lock(m_initializing);
	if (ui.cbBorderStyle->currentIndex() != pen.style())
		ui.cbBorderStyle->setCurrentIndex(pen.style());
	if (ui.kcbBorderColor->color() != pen.color())
		ui.kcbBorderColor->setColor(pen.color());
	if (ui.sbBorderWidth->value() != pen.widthF())
		ui.sbBorderWidth->setValue(Worksheet::convertFromSceneUnits(pen.widthF(), Worksheet::Unit::Point));
}

void BoxPlotDock::plotBorderOpacityChanged(float value) {
	Lock lock(m_initializing);
	float v = (float)value*100.;
	ui.sbBorderOpacity->setValue(v);
}

//**********************************************************
//******************** SETTINGS ****************************
//**********************************************************
void BoxPlotDock::loadConfig(KConfig& config) {
	KConfigGroup group = config.group(QLatin1String("BoxPlot"));

	//box filling
	ui.chkFillingEnabled->setChecked( group.readEntry("FillingEnabled", m_boxPlot->fillingEnabled()) );
	ui.cbFillingType->setCurrentIndex( group.readEntry("FillingType", (int) m_boxPlot->fillingType()) );
	ui.cbFillingColorStyle->setCurrentIndex( group.readEntry("FillingColorStyle", (int) m_boxPlot->fillingColorStyle()) );
	ui.cbFillingImageStyle->setCurrentIndex( group.readEntry("FillingImageStyle", (int) m_boxPlot->fillingImageStyle()) );
	ui.cbFillingBrushStyle->setCurrentIndex( group.readEntry("FillingBrushStyle", (int) m_boxPlot->fillingBrushStyle()) );
	ui.leFillingFileName->setText( group.readEntry("FillingFileName", m_boxPlot->fillingFileName()) );
	ui.kcbFillingFirstColor->setColor( group.readEntry("FillingFirstColor", m_boxPlot->fillingFirstColor()) );
	ui.kcbFillingSecondColor->setColor( group.readEntry("FillingSecondColor", m_boxPlot->fillingSecondColor()) );
	ui.sbFillingOpacity->setValue( round(group.readEntry("FillingOpacity", m_boxPlot->fillingOpacity())*100.0) );
	boxFillingTypeChanged(ui.cbFillingType->currentIndex());

	//box border
	ui.kcbBorderColor->setColor( group.readEntry("BorderColor", m_boxPlot->borderPen().color()) );
	ui.cbBorderStyle->setCurrentIndex( group.readEntry("BorderStyle", (int) m_boxPlot->borderPen().style()) );
	ui.sbBorderWidth->setValue( Worksheet::convertFromSceneUnits(group.readEntry("BorderWidth", m_boxPlot->borderPen().widthF()), Worksheet::Unit::Point) );
	ui.sbBorderOpacity->setValue( group.readEntry("BorderOpacity", m_boxPlot->borderOpacity())*100 );
}

void BoxPlotDock::loadConfigFromTemplate(KConfig& config) {
	//extract the name of the template from the file name
	QString name;
	int index = config.name().lastIndexOf(QLatin1String("/"));
	if (index != -1)
		name = config.name().right(config.name().size() - index - 1);
	else
		name = config.name();

	int size = m_boxPlots.size();
	if (size > 1)
		m_boxPlot->beginMacro(i18n("%1 xy-curves: template \"%2\" loaded", size, name));
	else
		m_boxPlot->beginMacro(i18n("%1: template \"%2\" loaded", m_boxPlot->name(), name));

	this->loadConfig(config);

	m_boxPlot->endMacro();
}

void BoxPlotDock::saveConfigAsTemplate(KConfig& config) {
	KConfigGroup group = config.group("BoxPlot");

	//box filling
	group.writeEntry("FillingEnabled", ui.chkFillingEnabled->isChecked());
	group.writeEntry("FillingType", ui.cbFillingType->currentIndex());
	group.writeEntry("FillingColorStyle", ui.cbFillingColorStyle->currentIndex());
	group.writeEntry("FillingImageStyle", ui.cbFillingImageStyle->currentIndex());
	group.writeEntry("FillingBrushStyle", ui.cbFillingBrushStyle->currentIndex());
	group.writeEntry("FillingFileName", ui.leFillingFileName->text());
	group.writeEntry("FillingFirstColor", ui.kcbFillingFirstColor->color());
	group.writeEntry("FillingSecondColor", ui.kcbFillingSecondColor->color());
	group.writeEntry("FillingOpacity", ui.sbFillingOpacity->value()/100.0);

	//box border
	group.writeEntry("BorderStyle", ui.cbBorderStyle->currentIndex());
	group.writeEntry("BorderColor", ui.kcbBorderColor->color());
	group.writeEntry("BorderWidth", Worksheet::convertToSceneUnits(ui.sbBorderWidth->value(), Worksheet::Unit::Point));
	group.writeEntry("BorderOpacity", ui.sbBorderOpacity->value()/100.0);

	//whiskers

	config.sync();
}
