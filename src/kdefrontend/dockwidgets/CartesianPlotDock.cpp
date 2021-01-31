/***************************************************************************
    File                 : CartesianPlotDock.cpp
    Project              : LabPlot
    Description          : widget for cartesian plot properties
    --------------------------------------------------------------------
    Copyright            : (C) 2011-2020 by Alexander Semke (alexander.semke@web.de)
    Copyright            : (C) 2012-2021 by Stefan Gerlach (stefan.gerlach@uni-konstanz.de)

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

#include "CartesianPlotDock.h"
#include "backend/worksheet/plots/PlotArea.h"
#include "backend/worksheet/plots/cartesian/Axis.h"
#include "backend/worksheet/plots/cartesian/XYCurve.h"
#include "backend/core/column/Column.h"

#include "kdefrontend/widgets/LabelWidget.h"
#include "kdefrontend/GuiTools.h"
#include "kdefrontend/TemplateHandler.h"
#include "kdefrontend/ThemeHandler.h"

#include <KMessageBox>

#include <QCompleter>
#include <QPainter>
#include <QTimer>
#include <QDir>
#include <QDirModel>
#include <QFileDialog>
#include <QImageReader>
#include <QButtonGroup>
#include <QIntValidator>

/*!
  \class CartesianPlotDock
  \brief  Provides a widget for editing the properties of the cartesian plot currently selected in the project explorer.

  \ingroup kdefrontend
*/

CartesianPlotDock::CartesianPlotDock(QWidget* parent) : BaseDock(parent) {
	ui.setupUi(this);
	m_leName = ui.leName;
	m_leComment = ui.leComment;

	//"General"-tab
	auto* rangeButtonsGroup(new QButtonGroup);
	rangeButtonsGroup->addButton(ui.rbRangeFirst);
	rangeButtonsGroup->addButton(ui.rbRangeLast);
	rangeButtonsGroup->addButton(ui.rbRangeFree);

	ui.twXRanges->setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);
	ui.twYRanges->setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);
	ui.twPlotRanges->setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);

	//"Range breaks"-tab
	ui.bAddXBreak->setIcon( QIcon::fromTheme("list-add") );
	ui.bRemoveXBreak->setIcon( QIcon::fromTheme("list-remove") );
	ui.cbXBreak->addItem("1");

	ui.bAddYBreak->setIcon( QIcon::fromTheme("list-add") );
	ui.bRemoveYBreak->setIcon( QIcon::fromTheme("list-remove") );
	ui.cbYBreak->addItem("1");

	//"Background"-tab
	ui.bOpen->setIcon( QIcon::fromTheme("document-open") );
	ui.leBackgroundFileName->setCompleter(new QCompleter(new QDirModel, this));

	//"Title"-tab
	auto* hboxLayout = new QHBoxLayout(ui.tabTitle);
	labelWidget = new LabelWidget(ui.tabTitle);
	hboxLayout->addWidget(labelWidget);
	hboxLayout->setContentsMargins(2,2,2,2);
	hboxLayout->setSpacing(2);

	//adjust layouts in the tabs
	for (int i = 0; i < ui.tabWidget->count(); ++i) {
		auto* layout = qobject_cast<QGridLayout*>(ui.tabWidget->widget(i)->layout());
		if (!layout)
			continue;

		layout->setContentsMargins(2,2,2,2);
		layout->setHorizontalSpacing(2);
		layout->setVerticalSpacing(2);
	}

	// "Cursor"-tab
	QStringList list = {i18n("NoPen"), i18n("SolidLine"), i18n("DashLine"), i18n("DotLine"), i18n("DashDotLine"), i18n("DashDotDotLine")};
	ui.cbCursorLineStyle->clear();
	for (int i = 0; i < list.count(); i++)
		ui.cbCursorLineStyle->addItem(list[i], i);

	//Validators
	ui.leRangeFirst->setValidator( new QIntValidator(ui.leRangeFirst) );
	ui.leRangeLast->setValidator( new QIntValidator(ui.leRangeLast) );
	ui.leXBreakStart->setValidator( new QDoubleValidator(ui.leXBreakStart) );
	ui.leXBreakEnd->setValidator( new QDoubleValidator(ui.leXBreakEnd) );
	ui.leYBreakStart->setValidator( new QDoubleValidator(ui.leYBreakStart) );
	ui.leYBreakEnd->setValidator( new QDoubleValidator(ui.leYBreakEnd) );

	//set the current locale
	updateLocale();

	//SIGNAL/SLOT
	//General
	connect(ui.leName, &QLineEdit::textChanged, this, &CartesianPlotDock::nameChanged);
	connect(ui.leComment, &QLineEdit::textChanged, this, &CartesianPlotDock::commentChanged);
	connect(ui.chkVisible, &QCheckBox::clicked, this, &CartesianPlotDock::visibilityChanged);
	connect(ui.sbLeft, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &CartesianPlotDock::geometryChanged);
	connect(ui.sbTop, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &CartesianPlotDock::geometryChanged);
	connect(ui.sbWidth, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &CartesianPlotDock::geometryChanged);
	connect(ui.sbHeight, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &CartesianPlotDock::geometryChanged);

	connect(ui.leRangeFirst, &QLineEdit::textChanged, this, &CartesianPlotDock::rangeFirstChanged);
	connect(ui.leRangeLast, &QLineEdit::textChanged, this, &CartesianPlotDock::rangeLastChanged);
	connect(rangeButtonsGroup, QOverload<int>::of(&QButtonGroup::buttonClicked), this, &CartesianPlotDock::rangeTypeChanged);

	//Range breaks
	connect(ui.chkXBreak, &QCheckBox::toggled, this, &CartesianPlotDock::toggleXBreak);
	connect(ui.bAddXBreak, &QPushButton::clicked, this, &CartesianPlotDock::addXBreak);
	connect(ui.bRemoveXBreak, &QPushButton::clicked, this, &CartesianPlotDock::removeXBreak);
	connect(ui.cbXBreak, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &CartesianPlotDock::currentXBreakChanged);
	connect(ui.leXBreakStart, &QLineEdit::textChanged, this, &CartesianPlotDock::xBreakStartChanged);
	connect(ui.leXBreakEnd, &QLineEdit::textChanged, this, &CartesianPlotDock::xBreakEndChanged);
	connect(ui.sbXBreakPosition, QOverload<int>::of(&QSpinBox::valueChanged), this, &CartesianPlotDock::xBreakPositionChanged);
	connect(ui.cbXBreakStyle, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &CartesianPlotDock::xBreakStyleChanged);

	connect(ui.chkYBreak, &QCheckBox::toggled, this, &CartesianPlotDock::toggleYBreak);
	connect(ui.bAddYBreak, &QPushButton::clicked, this, &CartesianPlotDock::addYBreak);
	connect(ui.bRemoveYBreak, &QPushButton::clicked, this, &CartesianPlotDock::removeYBreak);
	connect(ui.cbYBreak, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &CartesianPlotDock::currentYBreakChanged);
	connect(ui.leYBreakStart, &QLineEdit::textChanged, this, &CartesianPlotDock::yBreakStartChanged);
	connect(ui.leYBreakEnd, &QLineEdit::textChanged, this, &CartesianPlotDock::yBreakEndChanged);
	connect(ui.sbYBreakPosition, QOverload<int>::of(&QSpinBox::valueChanged), this, &CartesianPlotDock::yBreakPositionChanged);
	connect(ui.cbYBreakStyle, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &CartesianPlotDock::yBreakStyleChanged);

	//Background
	connect(ui.cbBackgroundType, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &CartesianPlotDock::backgroundTypeChanged);
	connect(ui.cbBackgroundColorStyle, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &CartesianPlotDock::backgroundColorStyleChanged);
	connect(ui.cbBackgroundImageStyle, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &CartesianPlotDock::backgroundImageStyleChanged);
	connect(ui.cbBackgroundBrushStyle, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &CartesianPlotDock::backgroundBrushStyleChanged);
	connect(ui.bOpen, &QPushButton::clicked, this, &CartesianPlotDock::selectFile);
	connect(ui.leBackgroundFileName, &QLineEdit::textChanged, this, &CartesianPlotDock::fileNameChanged);
	connect(ui.kcbBackgroundFirstColor, &KColorButton::changed, this, &CartesianPlotDock::backgroundFirstColorChanged);
	connect(ui.kcbBackgroundSecondColor, &KColorButton::changed, this, &CartesianPlotDock::backgroundSecondColorChanged);
	connect(ui.sbBackgroundOpacity, QOverload<int>::of(&QSpinBox::valueChanged), this, &CartesianPlotDock::backgroundOpacityChanged);

	//Border
	connect(ui.cbBorderStyle, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &CartesianPlotDock::borderStyleChanged);
	connect(ui.kcbBorderColor, &KColorButton::changed, this, &CartesianPlotDock::borderColorChanged);
	connect(ui.sbBorderWidth, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &CartesianPlotDock::borderWidthChanged);
	connect(ui.sbBorderCornerRadius, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &CartesianPlotDock::borderCornerRadiusChanged);
	connect(ui.sbBorderOpacity, QOverload<int>::of(&QSpinBox::valueChanged), this, &CartesianPlotDock::borderOpacityChanged);

	//Padding
	connect(ui.sbPaddingHorizontal, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &CartesianPlotDock::horizontalPaddingChanged);
	connect(ui.sbPaddingVertical, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &CartesianPlotDock::verticalPaddingChanged);
	connect(ui.sbPaddingRight, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &CartesianPlotDock::rightPaddingChanged);
	connect(ui.sbPaddingBottom, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &CartesianPlotDock::bottomPaddingChanged);
	connect(ui.cbPaddingSymmetric, &QCheckBox::toggled, this, &CartesianPlotDock::symmetricPaddingChanged);

	// Cursor
	connect(ui.sbCursorLineWidth, QOverload<int>::of(&QSpinBox::valueChanged), this, &CartesianPlotDock::cursorLineWidthChanged);
	connect(ui.kcbCursorLineColor, &KColorButton::changed, this, &CartesianPlotDock::cursorLineColorChanged);
	connect(ui.cbCursorLineStyle, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &CartesianPlotDock::cursorLineStyleChanged);

	//theme and template handlers
	auto* frame = new QFrame(this);
	auto* layout = new QHBoxLayout(frame);
	layout->setContentsMargins(0, 11, 0, 11);

	m_themeHandler = new ThemeHandler(this);
	layout->addWidget(m_themeHandler);
	connect(m_themeHandler, &ThemeHandler::loadThemeRequested, this, &CartesianPlotDock::loadTheme);
	connect(m_themeHandler, &ThemeHandler::info, this, &CartesianPlotDock::info);

	auto* templateHandler = new TemplateHandler(this, TemplateHandler::ClassName::CartesianPlot);
	layout->addWidget(templateHandler);
	connect(templateHandler, &TemplateHandler::loadConfigRequested, this, &CartesianPlotDock::loadConfigFromTemplate);
	connect(templateHandler, &TemplateHandler::saveConfigRequested, this, &CartesianPlotDock::saveConfigAsTemplate);
	connect(templateHandler, &TemplateHandler::info, this, &CartesianPlotDock::info);

	ui.verticalLayout->addWidget(frame);

	//TODO: activate the tab again once the functionality is implemented
	ui.tabWidget->removeTab(2);

	init();
}

void CartesianPlotDock::init() {
	this->retranslateUi();

	GuiTools::updatePenStyles(ui.cbCursorLineStyle, Qt::black);

	/*
	 //TODO: activate later once range breaking is implemented
	//create icons for the different styles for scale breaking
	QPainter pa;
	pa.setPen( QPen(Qt::SolidPattern, 0) );
	QPixmap pm(20, 20);
	ui.cbXBreakStyle->setIconSize( QSize(20,20) );
	ui.cbYBreakStyle->setIconSize( QSize(20,20) );

	//simple
	pm.fill(Qt::transparent);
	pa.begin( &pm );
	pa.setRenderHint(QPainter::Antialiasing);
	pa.setBrush(Qt::SolidPattern);
	pa.drawLine(3,10,8,10);
	pa.drawLine(12,10,17,10);
	pa.end();
	ui.cbXBreakStyle->setItemIcon(0, pm);
	ui.cbYBreakStyle->setItemIcon(0, pm);

	//vertical
	pm.fill(Qt::transparent);
	pa.begin( &pm );
	pa.setRenderHint(QPainter::Antialiasing);
	pa.setBrush(Qt::SolidPattern);
	pa.drawLine(3,10,8,10);
	pa.drawLine(12,10,17,10);
	pa.drawLine(8,14,8,6);
	pa.drawLine(12,14,12,6);
	pa.end();
	ui.cbXBreakStyle->setItemIcon(1, pm);
	ui.cbYBreakStyle->setItemIcon(1, pm);

	//sloped
	pm.fill(Qt::transparent);
	pa.begin( &pm );
	pa.setRenderHint(QPainter::Antialiasing);
	pa.setBrush(Qt::SolidPattern);
	pa.drawLine(3,10,8,10);
	pa.drawLine(12,10,17,10);
	pa.drawLine(6,14,10,6);
	pa.drawLine(10,14,14,6);
	pa.end();
	ui.cbXBreakStyle->setItemIcon(2, pm);
	ui.cbYBreakStyle->setItemIcon(2, pm);
	*/
}

void CartesianPlotDock::setPlots(QList<CartesianPlot*> list) {
	DEBUG(Q_FUNC_INFO)
	m_initializing = true;
	m_plotList = list;
	m_plot = list.first();
	m_aspect = list.first();

	QList<TextLabel*> labels;
	for (auto* plot : list)
		labels.append(plot->title());

	labelWidget->setLabels(labels);

	//if there is more then one plot in the list, disable the name and comment fields in the tab "general"
	if (list.size() == 1) {
		ui.lName->setEnabled(true);
		ui.leName->setEnabled(true);
		ui.lComment->setEnabled(true);
		ui.leComment->setEnabled(true);

		ui.leName->setText(m_plot->name());
		ui.leComment->setText(m_plot->comment());
	} else {
		ui.lName->setEnabled(false);
		ui.leName->setEnabled(false);
		ui.lComment->setEnabled(false);
		ui.leComment->setEnabled(false);

		ui.leName->setText(QString());
		ui.leComment->setText(QString());
	}

	symmetricPaddingChanged(m_plot->symmetricPadding());

	ui.leName->setStyleSheet("");
	ui.leName->setToolTip("");

	//show the properties of the first plot
	this->load();

	updateXRangeList();
	updateYRangeList();
	updatePlotRangeList();

	//update active widgets
	m_themeHandler->setCurrentTheme(m_plot->theme());

	//Deactivate the geometry related widgets, if the worksheet layout is active.
	//Currently, a plot can only be a child of the worksheet itself, so we only need to ask the parent aspect (=worksheet).
	//TODO redesign this, if the hierarchy will be changend in future (a plot is a child of a new object group/container or so)
	auto* w = dynamic_cast<Worksheet*>(m_plot->parentAspect());
	if (w) {
		bool b = (w->layout() == Worksheet::Layout::NoLayout);
		ui.sbTop->setEnabled(b);
		ui.sbLeft->setEnabled(b);
		ui.sbWidth->setEnabled(b);
		ui.sbHeight->setEnabled(b);
		connect(w, &Worksheet::layoutChanged, this, &CartesianPlotDock::layoutChanged);
	}

	//SIGNALs/SLOTs
	connect(m_plot, &CartesianPlot::aspectDescriptionChanged, this, &CartesianPlotDock::plotDescriptionChanged);
	connect(m_plot, &CartesianPlot::rectChanged, this, &CartesianPlotDock::plotRectChanged);
	connect(m_plot, &CartesianPlot::rangeTypeChanged, this, &CartesianPlotDock::plotRangeTypeChanged);
	connect(m_plot, &CartesianPlot::rangeFirstValuesChanged, this, &CartesianPlotDock::plotRangeFirstValuesChanged);
	connect(m_plot, &CartesianPlot::rangeLastValuesChanged, this, &CartesianPlotDock::plotRangeLastValuesChanged);
	//TODO: check if needed
	connect(m_plot, &CartesianPlot::xAutoScaleChanged, this, &CartesianPlotDock::plotXAutoScaleChanged);
	connect(m_plot, &CartesianPlot::xMinChanged, this, &CartesianPlotDock::plotXMinChanged);
	connect(m_plot, &CartesianPlot::xMaxChanged, this, &CartesianPlotDock::plotXMaxChanged);
	connect(m_plot, &CartesianPlot::xRangeChanged, this, &CartesianPlotDock::plotXRangeChanged);
	connect(m_plot, &CartesianPlot::xScaleChanged, this, &CartesianPlotDock::plotXScaleChanged);
	connect(m_plot, &CartesianPlot::xRangeFormatChanged, this, &CartesianPlotDock::plotXRangeFormatChanged);
	connect(m_plot, &CartesianPlot::yAutoScaleChanged, this, &CartesianPlotDock::plotYAutoScaleChanged);
	connect(m_plot, &CartesianPlot::yMinChanged, this, &CartesianPlotDock::plotYMinChanged);
	connect(m_plot, &CartesianPlot::yMaxChanged, this, &CartesianPlotDock::plotYMaxChanged);
	connect(m_plot, &CartesianPlot::yRangeChanged, this, &CartesianPlotDock::plotYRangeChanged);
	connect(m_plot, &CartesianPlot::yScaleChanged, this, &CartesianPlotDock::plotYScaleChanged);
	connect(m_plot, &CartesianPlot::yRangeFormatChanged, this, &CartesianPlotDock::plotYRangeFormatChanged);

	connect(m_plot, &CartesianPlot::visibleChanged, this, &CartesianPlotDock::plotVisibleChanged);

	//range breaks
	connect(m_plot, &CartesianPlot::xRangeBreakingEnabledChanged, this, &CartesianPlotDock::plotXRangeBreakingEnabledChanged);
	connect(m_plot, &CartesianPlot::xRangeBreaksChanged, this, &CartesianPlotDock::plotXRangeBreaksChanged);
	connect(m_plot, &CartesianPlot::yRangeBreakingEnabledChanged, this, &CartesianPlotDock::plotYRangeBreakingEnabledChanged);
	connect(m_plot, &CartesianPlot::yRangeBreaksChanged, this, &CartesianPlotDock::plotYRangeBreaksChanged);

	// Plot Area
	connect(m_plot->plotArea(), &PlotArea::backgroundTypeChanged, this, &CartesianPlotDock::plotBackgroundTypeChanged);
	connect(m_plot->plotArea(), &PlotArea::backgroundColorStyleChanged, this, &CartesianPlotDock::plotBackgroundColorStyleChanged);
	connect(m_plot->plotArea(), &PlotArea::backgroundImageStyleChanged, this, &CartesianPlotDock::plotBackgroundImageStyleChanged);
	connect(m_plot->plotArea(), &PlotArea::backgroundBrushStyleChanged, this, &CartesianPlotDock::plotBackgroundBrushStyleChanged);
	connect(m_plot->plotArea(), &PlotArea::backgroundFirstColorChanged, this, &CartesianPlotDock::plotBackgroundFirstColorChanged);
	connect(m_plot->plotArea(), &PlotArea::backgroundSecondColorChanged, this, &CartesianPlotDock::plotBackgroundSecondColorChanged);
	connect(m_plot->plotArea(), &PlotArea::backgroundFileNameChanged, this, &CartesianPlotDock::plotBackgroundFileNameChanged);
	connect(m_plot->plotArea(), &PlotArea::backgroundOpacityChanged, this, &CartesianPlotDock::plotBackgroundOpacityChanged);
	connect(m_plot->plotArea(), &PlotArea::borderPenChanged, this, &CartesianPlotDock::plotBorderPenChanged);
	connect(m_plot->plotArea(), &PlotArea::borderOpacityChanged, this, &CartesianPlotDock::plotBorderOpacityChanged);
	connect(m_plot, &CartesianPlot::horizontalPaddingChanged, this, &CartesianPlotDock::plotHorizontalPaddingChanged);
	connect(m_plot, &CartesianPlot::verticalPaddingChanged, this, &CartesianPlotDock::plotVerticalPaddingChanged);
	connect(m_plot, &CartesianPlot::rightPaddingChanged, this, &CartesianPlotDock::plotRightPaddingChanged);
	connect(m_plot, &CartesianPlot::bottomPaddingChanged, this, &CartesianPlotDock::plotBottomPaddingChanged);
	connect(m_plot, &CartesianPlot::symmetricPaddingChanged, this, &CartesianPlotDock::plotSymmetricPaddingChanged);

	m_initializing = false;
}

void CartesianPlotDock::activateTitleTab() {
	ui.tabWidget->setCurrentWidget(ui.tabTitle);
}

/*
 * updates the locale in the widgets. called when the application settins are changed.
 */
void CartesianPlotDock::updateLocale() {
	DEBUG(Q_FUNC_INFO)
	SET_NUMBER_LOCALE

	//update the QSpinBoxes
	ui.sbLeft->setLocale(numberLocale);
	ui.sbTop->setLocale(numberLocale);
	ui.sbWidth->setLocale(numberLocale);
	ui.sbHeight->setLocale(numberLocale);
	ui.sbBorderWidth->setLocale(numberLocale);
	ui.sbBorderCornerRadius->setLocale(numberLocale);
	ui.sbPaddingHorizontal->setLocale(numberLocale);
	ui.sbPaddingVertical->setLocale(numberLocale);
	ui.sbPaddingRight->setLocale(numberLocale);
	ui.sbPaddingBottom->setLocale(numberLocale);

	//update the QLineEdits, avoid the change events
	if (m_plot) {
		Lock lock(m_initializing);
		ui.leRangeFirst->setText( numberLocale.toString(m_plot->rangeFirstValues()) );
		ui.leRangeLast->setText( numberLocale.toString(m_plot->rangeLastValues()) );

		// x ranges
		bool isDateTime{ false };
		for (int row{0}; row < ui.twXRanges->rowCount(); row++) {
			const auto xRange{ m_plot->xRange(row) };
			DEBUG(Q_FUNC_INFO << ", x range " << row << " auto scale = " << xRange.autoScale())
			if (m_plot->xRangeFormat(row) == RangeT::Format::Numeric) {
				auto* le = qobject_cast<QLineEdit*>(ui.twXRanges->cellWidget(row, 2));
				if (le) {	// may be nullptr
					le->setText( numberLocale.toString(xRange.start()) );
					le = qobject_cast<QLineEdit*>( ui.twXRanges->cellWidget(row, 3) );
					le->setText( numberLocale.toString(xRange.end()) );
				}
			} else {
				auto* dte = qobject_cast<QDateTimeEdit*>(ui.twXRanges->cellWidget(row, 2));
				if (dte) {
					dte->setDateTime( QDateTime::fromMSecsSinceEpoch(xRange.start()) );
					dte = qobject_cast<QDateTimeEdit*>( ui.twXRanges->cellWidget(row, 3) );
					dte->setDateTime( QDateTime::fromMSecsSinceEpoch(xRange.end()) );
					isDateTime = true;
				}
			}
		}

//TODO
		if (isDateTime) {
			ui.twXRanges->resizeColumnToContents(2);
			ui.twXRanges->resizeColumnToContents(3);
		}

		// y ranges
		isDateTime = false;
		for (int row{0}; row < ui.twYRanges->rowCount(); row++) {
			const auto yRange{ m_plot->yRange(row) };
			DEBUG(Q_FUNC_INFO << ", y range " << row << " auto scale = " << yRange.autoScale())
			if (m_plot->yRangeFormat(row) == RangeT::Format::Numeric) {
				auto* le = qobject_cast<QLineEdit*>(ui.twYRanges->cellWidget(row, 2));
				QDEBUG(Q_FUNC_INFO << ", MIN SIZE = " << le->minimumSize())
				QDEBUG(Q_FUNC_INFO << ", MIN SIZE HINT = " << le->minimumSizeHint())
				QDEBUG(Q_FUNC_INFO << ", SIZE = " << le->size())
				QDEBUG(Q_FUNC_INFO << ", SIZE HINT = " << le->sizeHint())
				if (le) {	// may be nullptr
					le->setText( numberLocale.toString(yRange.start()) );
					//le->resize(le->minimumSizeHint());
					le = qobject_cast<QLineEdit*>( ui.twYRanges->cellWidget(row, 3) );
					le->setText( numberLocale.toString(yRange.end()) );
					//le->resize(le->minimumSizeHint());
				}
			} else {
				auto* dte = qobject_cast<QDateTimeEdit*>(ui.twYRanges->cellWidget(row, 2));
				if (dte) {
					dte->setDateTime( QDateTime::fromMSecsSinceEpoch(yRange.start()) );
					dte = qobject_cast<QDateTimeEdit*>( ui.twYRanges->cellWidget(row, 3) );
					dte->setDateTime( QDateTime::fromMSecsSinceEpoch(yRange.end()) );
					isDateTime = true;
				}
			}
		}
//TODO
		DEBUG(Q_FUNC_INFO << ", section size = " << ui.twYRanges->horizontalHeader()->sectionSize(2));
		DEBUG(Q_FUNC_INFO << ", min section size = " << ui.twYRanges->horizontalHeader()->minimumSectionSize());
//		ui.twYRanges->horizontalHeader()->resizeSection(2, ui.twYRanges->horizontalHeader()->minimumSectionSize());
//		ui.twYRanges->horizontalHeader()->setDefaultSectionSize(ui.twYRanges->horizontalHeader()->minimumSectionSize());
		DEBUG(Q_FUNC_INFO << ", section size = " << ui.twYRanges->horizontalHeader()->sectionSize(2));
		DEBUG(Q_FUNC_INFO << ", min section size = " << ui.twYRanges->horizontalHeader()->minimumSectionSize());
		if (isDateTime) {
			ui.twYRanges->resizeColumnToContents(2);
			DEBUG(Q_FUNC_INFO << ", section size = " << ui.twYRanges->horizontalHeader()->sectionSize(2));
			DEBUG(Q_FUNC_INFO << ", min section size = " << ui.twYRanges->horizontalHeader()->minimumSectionSize());
			ui.twYRanges->resizeColumnToContents(3);
		}
	}

	//update the title label
	labelWidget->updateLocale();
}

void CartesianPlotDock::updateUnits() {
	const KConfigGroup group = KSharedConfig::openConfig()->group(QLatin1String("Settings_General"));
	BaseDock::Units units = (BaseDock::Units)group.readEntry("Units", static_cast<int>(Units::Metric));
	if (units == m_units)
		return;

	m_units = units;
	Lock lock(m_initializing);
	QString suffix;
	if (m_units == Units::Metric) {
		//convert from imperial to metric
		m_worksheetUnit = Worksheet::Unit::Centimeter;
		suffix = QLatin1String(" cm");
		ui.sbLeft->setValue(ui.sbLeft->value()*2.54);
		ui.sbTop->setValue(ui.sbTop->value()*2.54);
		ui.sbWidth->setValue(ui.sbWidth->value()*2.54);
		ui.sbHeight->setValue(ui.sbHeight->value()*2.54);
		ui.sbBorderCornerRadius->setValue(ui.sbBorderCornerRadius->value()*2.54);
		ui.sbPaddingHorizontal->setValue(ui.sbPaddingHorizontal->value()*2.54);
		ui.sbPaddingVertical->setValue(ui.sbPaddingVertical->value()*2.54);
		ui.sbPaddingRight->setValue(ui.sbPaddingRight->value()*2.54);
		ui.sbPaddingBottom->setValue(ui.sbPaddingBottom->value()*2.54);
	} else {
		//convert from metric to imperial
		m_worksheetUnit = Worksheet::Unit::Inch;
		suffix = QLatin1String(" in");
		ui.sbLeft->setValue(ui.sbLeft->value()/2.54);
		ui.sbTop->setValue(ui.sbTop->value()/2.54);
		ui.sbWidth->setValue(ui.sbWidth->value()/2.54);
		ui.sbHeight->setValue(ui.sbHeight->value()/2.54);
		ui.sbBorderCornerRadius->setValue(ui.sbBorderCornerRadius->value()/2.54);
		ui.sbPaddingHorizontal->setValue(ui.sbPaddingHorizontal->value()/2.54);
		ui.sbPaddingVertical->setValue(ui.sbPaddingVertical->value()/2.54);
		ui.sbPaddingRight->setValue(ui.sbPaddingRight->value()/2.54);
		ui.sbPaddingBottom->setValue(ui.sbPaddingBottom->value()/2.54);
	}

	ui.sbLeft->setSuffix(suffix);
	ui.sbTop->setSuffix(suffix);
	ui.sbWidth->setSuffix(suffix);
	ui.sbHeight->setSuffix(suffix);
	ui.sbBorderCornerRadius->setSuffix(suffix);
	ui.sbPaddingHorizontal->setSuffix(suffix);
	ui.sbPaddingVertical->setSuffix(suffix);
	ui.sbPaddingRight->setSuffix(suffix);
	ui.sbPaddingBottom->setSuffix(suffix);

	labelWidget->updateUnits();
}

void CartesianPlotDock::updateXRangeList() {
	if (!m_plot)
		return;

	const int xRangeCount{ m_plot->xRangeCount() };
	DEBUG(Q_FUNC_INFO << ", x range count = " << xRangeCount)

	ui.twXRanges->setRowCount(xRangeCount);
	for (int i{0}; i < xRangeCount; i++) {
		const auto xRange{ m_plot->xRange(i) };
		const auto format{ xRange.format() };
		const auto scale { xRange.scale() };
		DEBUG(Q_FUNC_INFO << ", x range " << i << " format : " << static_cast<int>(format)
			<< " scale : " << static_cast<int>(scale) << " auto scale = " << xRange.autoScale())

		// auto scale
		QCheckBox *chk = new QCheckBox(ui.twXRanges);
		chk->setProperty("row", i);
		chk->setChecked(xRange.autoScale());
//		chk->setStyleSheet("margin-left:50%; margin-right:50%;");	// center button
		ui.twXRanges->setCellWidget(i, 0, chk);
		connect(chk, &QCheckBox::stateChanged, this, &CartesianPlotDock::autoScaleXChanged);

		// format
		QComboBox *cb = new QComboBox(ui.twXRanges);
		cb->addItem( i18n("Numeric") );
		cb->addItem( i18n("Date/Time") );
		cb->setProperty("row", i);
		cb->setCurrentIndex(static_cast<int>(format));
		ui.twXRanges->setCellWidget(i, 1, cb);
		connect(cb, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &CartesianPlotDock::xRangeFormatChanged);

		// start/end (values set in updateLocale())
		if (format == RangeT::Format::Numeric) {
			QLineEdit *le = new QLineEdit(ui.twXRanges);
			le->setValidator(new QDoubleValidator(le));
			le->setProperty("row", i);
//			le->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred);
//			le->resize(le->minimumSizeHint());
			ui.twXRanges->setCellWidget(i, 2, le);
			connect(le, &QLineEdit::textChanged, this, &CartesianPlotDock::xMinChanged);
			le = new QLineEdit(ui.twXRanges);
			le->setValidator(new QDoubleValidator(le));
			le->setProperty("row", i);
//			le->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred);
//			le->resize(le->minimumSizeHint());
			ui.twXRanges->setCellWidget(i, 3, le);
			connect(le, &QLineEdit::textChanged, this, &CartesianPlotDock::xMaxChanged);
		} else {
			QDateTimeEdit *dte = new QDateTimeEdit(ui.twXRanges);
			dte->setDisplayFormat( m_plot->xRangeDateTimeFormat(i) );
			dte->setDateTime(QDateTime::fromMSecsSinceEpoch(xRange.start()));
			ui.twXRanges->setCellWidget(i, 2, dte);
			dte->setProperty("row", i);
			connect(dte, &QDateTimeEdit::dateTimeChanged, this, &CartesianPlotDock::xMinDateTimeChanged);
			dte = new QDateTimeEdit(ui.twXRanges);
			dte->setDisplayFormat( m_plot->xRangeDateTimeFormat(i) );
			dte->setDateTime(QDateTime::fromMSecsSinceEpoch(xRange.end()));
			ui.twXRanges->setCellWidget(i, 3, dte);
			dte->setProperty("row", i);
			connect(dte, &QDateTimeEdit::dateTimeChanged, this, &CartesianPlotDock::xMaxDateTimeChanged);
		}

		// scale
		cb = new QComboBox(ui.twXRanges);
		//TODO: -> updateLocale()
		cb->addItem( i18n("linear") );
		cb->addItem( i18n("log(x)") );
		cb->addItem( i18n("log2(x)") );
		cb->addItem( i18n("ln(x)") );
		cb->addItem( i18n("log(|x|)") );
		cb->addItem( i18n("log2(|x|)") );
		cb->addItem( i18n("ln(|x|)") );
		cb->setCurrentIndex(static_cast<int>(scale));
		cb->setProperty("row", i);
		ui.twXRanges->setCellWidget(i, 4, cb);
		connect(cb, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &CartesianPlotDock::xScaleChanged);
	}
	ui.twXRanges->resizeColumnToContents(1);

	ui.tbRemoveXRange->setEnabled(xRangeCount > 1 ? true : false);

	updateLocale();	// fill values
	updatePlotRangeList();	// update x ranges used in plot ranges

	// enable/disable widgets
	for (int i{0}; i < xRangeCount; i++) {
		const bool checked{ m_plot->xRange(i).autoScale() };
		qobject_cast<QComboBox*>(ui.twXRanges->cellWidget(i, 1))->setEnabled(!checked);
		qobject_cast<QWidget*>(ui.twXRanges->cellWidget(i, 2))->setEnabled(!checked);
		qobject_cast<QWidget*>(ui.twXRanges->cellWidget(i, 3))->setEnabled(!checked);
	}
}
void CartesianPlotDock::updateYRangeList() {
	if (!m_plot)
		return;

	const int yRangeCount{ m_plot->yRangeCount() };
	DEBUG(Q_FUNC_INFO << ", y range count = " << yRangeCount)

	ui.twYRanges->setRowCount(yRangeCount);
	for (int i{0}; i < yRangeCount; i++) {
		const auto yRange{ m_plot->yRange(i) };
		const auto format{ yRange.format() };
		const auto scale { yRange.scale() };
		DEBUG(Q_FUNC_INFO << ", y range " << i << " format : " << static_cast<int>(format)
			<< " scale : " << static_cast<int>(scale) << " auto scale = " << yRange.autoScale())

		// auto scale
		QCheckBox *chk = new QCheckBox(ui.twYRanges);
		chk->setProperty("row", i);
		chk->setChecked(yRange.autoScale());
		//	chk->setStyleSheet("margin-left:50%; margin-right:50%;");	// center button
		ui.twYRanges->setCellWidget(i, 0, chk);
		connect(chk, &QCheckBox::stateChanged, this, &CartesianPlotDock::autoScaleYChanged);

		// format
		QComboBox *cb = new QComboBox(ui.twYRanges);
		cb->addItem( i18n("Numeric") );
		cb->addItem( i18n("Date/Time") );
		cb->setProperty("row", i);
		cb->setCurrentIndex(static_cast<int>(format));
		ui.twYRanges->setCellWidget(i, 1, cb);
		connect(cb, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &CartesianPlotDock::yRangeFormatChanged);

		// start/end (values set in updateLocale())
		if (format == RangeT::Format::Numeric) {
			QLineEdit *le = new QLineEdit(ui.twYRanges);
			le->setValidator(new QDoubleValidator(le));
			le->setProperty("row", i);
//TODO			le->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
			QDEBUG(Q_FUNC_INFO << ", SIZE HINT = " << le->sizeHint())
			QDEBUG(Q_FUNC_INFO << ", MIN SIZE HINT = " << le->minimumSizeHint())
			QDEBUG(Q_FUNC_INFO << ", SIZE = " << le->size())
//			le->resize(le->minimumSizeHint());
//			QDEBUG(Q_FUNC_INFO << ", resize SIZE = " << le->size())
			ui.twYRanges->setCellWidget(i, 2, le);
			connect(le, &QLineEdit::textChanged, this, &CartesianPlotDock::yMinChanged);
			le = new QLineEdit(ui.twYRanges);
			le->setValidator(new QDoubleValidator(le));
			le->setProperty("row", i);
//			le->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);
//			le->setMinimumSize(le->minimumSizeHint());
			ui.twYRanges->setCellWidget(i, 3, le);
			connect(le, &QLineEdit::textChanged, this, &CartesianPlotDock::yMaxChanged);
		} else {
			QDateTimeEdit *dte = new QDateTimeEdit(ui.twYRanges);
			dte->setDisplayFormat( m_plot->yRangeDateTimeFormat(i) );
			dte->setDateTime(QDateTime::fromMSecsSinceEpoch(m_plot->yRange(i).start()));
			ui.twYRanges->setCellWidget(i, 2, dte);
			dte->setProperty("row", i);
			connect(dte, &QDateTimeEdit::dateTimeChanged, this, &CartesianPlotDock::yMinDateTimeChanged);
			dte = new QDateTimeEdit(ui.twYRanges);
			dte->setDisplayFormat( m_plot->yRangeDateTimeFormat(i) );
			dte->setDateTime(QDateTime::fromMSecsSinceEpoch(m_plot->yRange(i).end()));
			ui.twYRanges->setCellWidget(i, 3, dte);
			dte->setProperty("row", i);
			connect(dte, &QDateTimeEdit::dateTimeChanged, this, &CartesianPlotDock::yMaxDateTimeChanged);
		}

		// scale
		cb = new QComboBox(ui.twYRanges);
		//TODO: -> updateLocale()
		cb->addItem( i18n("linear") );
		cb->addItem( i18n("log(x)") );
		cb->addItem( i18n("log2(x)") );
		cb->addItem( i18n("ln(x)") );
		cb->addItem( i18n("log(|x|)") );
		cb->addItem( i18n("log2(|x|)") );
		cb->addItem( i18n("ln(|x|)") );
		cb->setCurrentIndex(static_cast<int>(scale));
		cb->setProperty("row", i);
		ui.twYRanges->setCellWidget(i, 4, cb);
		connect(cb, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &CartesianPlotDock::yScaleChanged);
	}
//TODO	ui.twYRanges->horizontalHeader()->resizeSection(2, ui.twYRanges->horizontalHeader()->minimumSectionSize());
	DEBUG(Q_FUNC_INFO << ", section resize mode = " << ui.twYRanges->horizontalHeader()->sectionResizeMode(2));
	DEBUG(Q_FUNC_INFO << ", section size = " << ui.twYRanges->horizontalHeader()->sectionSize(2));
	DEBUG(Q_FUNC_INFO << ", min section size = " << ui.twYRanges->horizontalHeader()->minimumSectionSize());
//	DEBUG(Q_FUNC_INFO << ", min section size = " << ui.twYRanges->sizeHintForColumn());
//	ui.twYRanges->resizeColumnsToContents();
//	ui.twYRanges->setColumnWidth(2, 57);
	DEBUG(Q_FUNC_INFO << ", section size = " << ui.twYRanges->horizontalHeader()->sectionSize(2));
	DEBUG(Q_FUNC_INFO << ", min section size = " << ui.twYRanges->horizontalHeader()->minimumSectionSize());
	ui.twYRanges->resizeColumnToContents(0);
	ui.twYRanges->resizeColumnToContents(1);
	ui.twYRanges->resizeColumnToContents(4);
//	ui.twYRanges->resizeColumnsToContents();

	ui.tbRemoveYRange->setEnabled(yRangeCount > 1 ? true : false);

	updateLocale();	// fill values
	updatePlotRangeList();	// update y ranges used in plot ranges

	// enable/disable widgets
	for (int i{0}; i < yRangeCount; i++) {
		const bool checked{ m_plot->yRange(i).autoScale() };
		qobject_cast<QComboBox*>(ui.twYRanges->cellWidget(i, 1))->setEnabled(!checked);
		qobject_cast<QWidget*>(ui.twYRanges->cellWidget(i, 2))->setEnabled(!checked);
		qobject_cast<QWidget*>(ui.twYRanges->cellWidget(i, 3))->setEnabled(!checked);
	}
}

// update plot ranges in list
void CartesianPlotDock::updatePlotRangeList() {
	if (!m_plot)
		return;

	const int cSystemCount{ m_plot->coordinateSystemCount() };
	DEBUG(Q_FUNC_INFO << ", nr of cSystems = " << cSystemCount)
	ui.twPlotRanges->setRowCount(cSystemCount);
	for (int i{0}; i < cSystemCount; i++) {
		const auto* cSystem{ m_plot->coordinateSystem(i) };
		const int xIndex{ cSystem->xIndex() }, yIndex{ cSystem->yIndex() };
		const auto xRange{ m_plot->xRange(xIndex) }, yRange{ m_plot->yRange(yIndex) };

		DEBUG(Q_FUNC_INFO << ", coordinate system " << i+1 << " : xIndex = " << xIndex << ", yIndex = " << yIndex)
		DEBUG(Q_FUNC_INFO << ", x range = " << xRange.toStdString() << " auto scale = " << xRange.autoScale())
		DEBUG(Q_FUNC_INFO << ", y range = " << yRange.toStdString() << " auto scale = " << yRange.autoScale())

		QComboBox *cb = new QComboBox(ui.twPlotRanges);
		cb->setEditable(true);	// to have a line edit
		cb->lineEdit()->setReadOnly(true);
		cb->lineEdit()->setAlignment(Qt::AlignHCenter);
		if (m_plot->xRangeCount() > 1) {
			for (int index{0}; index < m_plot->xRangeCount(); index++)
				cb->addItem( QString::number(index + 1) + QLatin1String(" : ") + m_plot->xRange(index).toString() );
			cb->setCurrentIndex(xIndex);
			cb->setProperty("row", i);
			connect(cb, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &CartesianPlotDock::PlotRangeXChanged);
		} else {
			cb->addItem( xRange.toString() );
			cb->setStyleSheet("QComboBox::drop-down {border-width: 0px;}");	// hide arrow if there is only one range
		}
		ui.twPlotRanges->setCellWidget(i, 0, cb);

		cb = new QComboBox(ui.twPlotRanges);
		cb->setEditable(true);	// to have a line edit
		cb->lineEdit()->setReadOnly(true);
		cb->lineEdit()->setAlignment(Qt::AlignHCenter);
		if (m_plot->yRangeCount() > 1) {
			for (int index{0}; index < m_plot->yRangeCount(); index++)
				cb->addItem( QString::number(index + 1) + QLatin1String(" : ") + m_plot->yRange(index).toString() );
			cb->setCurrentIndex(yIndex);
			cb->setProperty("row", i);
			connect(cb, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &CartesianPlotDock::PlotRangeYChanged);
		} else {
			cb->addItem( yRange.toString() );
			cb->setStyleSheet("QComboBox::drop-down {border-width: 0px;}");	// hide arrow if there is only one range
		}
		ui.twPlotRanges->setCellWidget(i, 1, cb);
	}
	ui.twPlotRanges->resizeColumnToContents(0);
	ui.twPlotRanges->resizeColumnToContents(1);

	if (m_bgDefaultPlotRange) {
		for (auto* button : m_bgDefaultPlotRange->buttons())
			m_bgDefaultPlotRange->removeButton(button);
	} else {
		m_bgDefaultPlotRange = new QButtonGroup(this);
		connect(m_bgDefaultPlotRange, QOverload<QAbstractButton *>::of(&QButtonGroup::buttonClicked), this, &CartesianPlotDock::defaultPlotRangeChanged);
	}
	for (int i{0}; i < cSystemCount; i++) {
		QRadioButton *rb = new QRadioButton();
		if (i == m_plot->defaultCoordinateSystemIndex())
			rb->setChecked(true);
		m_bgDefaultPlotRange->addButton(rb);
		rb->setStyleSheet("margin-left:50%; margin-right:50%;");	// center button
		ui.twPlotRanges->setCellWidget(i, 2, rb);
		m_bgDefaultPlotRange->setId(rb, i);
	}

	ui.tbRemovePlotRange->setEnabled(cSystemCount > 1 ? true : false);
}

//************************************************************
//**** SLOTs for changes triggered in CartesianPlotDock ******
//************************************************************
void CartesianPlotDock::retranslateUi() {
	Lock lock(m_initializing);

	//scale breakings
	ui.cbXBreakStyle->addItem( i18n("Simple") );
	ui.cbXBreakStyle->addItem( i18n("Vertical") );
	ui.cbXBreakStyle->addItem( i18n("Sloped") );

	ui.cbYBreakStyle->addItem( i18n("Simple") );
	ui.cbYBreakStyle->addItem( i18n("Vertical") );
	ui.cbYBreakStyle->addItem( i18n("Sloped") );

	//plot area
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

	GuiTools::updatePenStyles(ui.cbBorderStyle, Qt::black);
	GuiTools::updateBrushStyles(ui.cbBackgroundBrushStyle, Qt::SolidPattern);

	QString suffix;
	if (m_units == Units::Metric)
		suffix = QLatin1String(" cm");
	else
		suffix = QLatin1String(" in");

	ui.sbLeft->setSuffix(suffix);
	ui.sbTop->setSuffix(suffix);
	ui.sbWidth->setSuffix(suffix);
	ui.sbHeight->setSuffix(suffix);
	ui.sbBorderCornerRadius->setSuffix(suffix);
	ui.sbPaddingHorizontal->setSuffix(suffix);
	ui.sbPaddingVertical->setSuffix(suffix);
	ui.sbPaddingRight->setSuffix(suffix);
	ui.sbPaddingBottom->setSuffix(suffix);
}

// "General"-tab
void CartesianPlotDock::visibilityChanged(bool state) {
	if (m_initializing)
		return;

	for (auto* plot : m_plotList)
		plot->setVisible(state);
}

void CartesianPlotDock::geometryChanged() {
	if (m_initializing)
		return;

	double x = Worksheet::convertToSceneUnits(ui.sbLeft->value(), m_worksheetUnit);
	double y = Worksheet::convertToSceneUnits(ui.sbTop->value(), m_worksheetUnit);
	double w = Worksheet::convertToSceneUnits(ui.sbWidth->value(), m_worksheetUnit);
	double h = Worksheet::convertToSceneUnits(ui.sbHeight->value(), m_worksheetUnit);

	QRectF rect(x, y, w, h);
	m_plot->setRect(rect);
}

/*!
    Called when the layout in the worksheet gets changed.
    Enables/disables the geometry widgets if the layout was deactivated/activated.
    Shows the new geometry values of the first plot if the layout was activated.
 */
void CartesianPlotDock::layoutChanged(Worksheet::Layout layout) {
	bool b = (layout == Worksheet::Layout::NoLayout);
	ui.sbTop->setEnabled(b);
	ui.sbLeft->setEnabled(b);
	ui.sbWidth->setEnabled(b);
	ui.sbHeight->setEnabled(b);
}

void CartesianPlotDock::rangeTypeChanged() {
	CartesianPlot::RangeType type;
	if (ui.rbRangeFirst->isChecked()) {
		ui.leRangeFirst->setEnabled(true);
		ui.leRangeLast->setEnabled(false);
		type = CartesianPlot::RangeType::First;
	} else if (ui.rbRangeLast->isChecked()) {
		ui.leRangeFirst->setEnabled(false);
		ui.leRangeLast->setEnabled(true);
		type = CartesianPlot::RangeType::Last;
	} else {
		ui.leRangeFirst->setEnabled(false);
		ui.leRangeLast->setEnabled(false);
		type = CartesianPlot::RangeType::Free;
	}

	if (m_initializing)
		return;

	for (auto* plot : m_plotList)
		plot->setRangeType(type);
}

void CartesianPlotDock::rangeFirstChanged(const QString& text) {
	if (m_initializing)
		return;

	const int value = text.toInt();
	for (auto* plot : m_plotList)
		plot->setRangeFirstValues(value);
}

void CartesianPlotDock::rangeLastChanged(const QString& text) {
	if (m_initializing)
		return;

	const int value = text.toInt();
	for (auto* plot : m_plotList)
		plot->setRangeLastValues(value);
}

// x/y Ranges

void CartesianPlotDock::autoScaleXChanged(int state) {
	DEBUG(Q_FUNC_INFO << ", state = " << state)
	if (m_initializing)
		return;

	bool checked = (state == Qt::Checked);
	const int xRangeIndex{ sender()->property("row").toInt() };
	DEBUG( Q_FUNC_INFO << ", x range index: " << xRangeIndex )

	autoScaleXRange(xRangeIndex, checked);
}
void CartesianPlotDock::autoScaleXRange(const int index, bool checked) {
	DEBUG(Q_FUNC_INFO << ", index = " << index << " checked = " << checked)

	if (ui.twXRanges->cellWidget(index, 1)) {
		qobject_cast<QComboBox*>(ui.twXRanges->cellWidget(index, 1))->setEnabled(!checked);
		qobject_cast<QWidget*>(ui.twXRanges->cellWidget(index, 2))->setEnabled(!checked);
		qobject_cast<QWidget*>(ui.twXRanges->cellWidget(index, 3))->setEnabled(!checked);
	}

	for (auto* plot : m_plotList) {
		plot->setAutoScaleX(index, checked);
		DEBUG(Q_FUNC_INFO << " new auto scale = " << plot->xRange(index).autoScale())
		if (checked) { // && index == plot->defaultCoordinateSystem()->xIndex()
			plot->scaleAutoX(index);
			//TODO: which yIndex?
			if (plot->autoScaleY())
				plot->scaleAutoY();
		}
	}
	updateXRangeList();	// see range changes
	updatePlotRangeList();
}

void CartesianPlotDock::autoScaleYChanged(int state) {
	DEBUG(Q_FUNC_INFO << ", state = " << state)
	if (m_initializing)
		return;

	bool checked = (state == Qt::Checked);
	const int yRangeIndex{ sender()->property("row").toInt() };
	DEBUG( Q_FUNC_INFO << ", y range " << yRangeIndex+1 )

	autoScaleYRange(yRangeIndex, checked);
}
// index - y range index
void CartesianPlotDock::autoScaleYRange(const int index, const bool checked) {
	DEBUG(Q_FUNC_INFO << ", index = " << index << ", check = " << checked)

	if (ui.twYRanges->cellWidget(index, 1)) {
		qobject_cast<QComboBox*>(ui.twYRanges->cellWidget(index, 1))->setEnabled(!checked);
		qobject_cast<QWidget*>(ui.twYRanges->cellWidget(index, 2))->setEnabled(!checked);
		qobject_cast<QWidget*>(ui.twYRanges->cellWidget(index, 3))->setEnabled(!checked);
	}

	for (auto* plot : m_plotList) {
		plot->setAutoScaleY(index, checked);
		DEBUG(Q_FUNC_INFO << " new auto scale = " << plot->yRange(index).autoScale())
		if (checked) {	// && index == plot->defaultCoordinateSystem()->yIndex()
			plot->scaleAutoY(index);
			//TODO: which xIndex?
			if (plot->autoScaleX())
				plot->scaleAutoX();
		}
	}
	updateYRangeList();	// see range changes
	updatePlotRangeList();
}

void CartesianPlotDock::xMinChanged(const QString& value) {
	DEBUG(Q_FUNC_INFO << ", value = " << value.toStdString())
	DEBUG(Q_FUNC_INFO)
	if (m_initializing)
		return;

	const Lock lock(m_initializing);
	bool ok;
	SET_NUMBER_LOCALE
	const double xMin = numberLocale.toDouble(value, &ok);
	if (ok) {
		// selected x range
		const int index{ sender()->property("row").toInt() };
		DEBUG( Q_FUNC_INFO << ", x range index: " << index )
		bool changed{false};
		for (auto* plot : m_plotList)
			if (!qFuzzyCompare(xMin, plot->xRange(index).start())) {
				plot->setXMin(index, xMin);
				changed = true;
			}

		if (changed) {
			updateYRangeList();	// plot is auto scaled
			updatePlotRangeList();
		}
	}
}
void CartesianPlotDock::yMinChanged(const QString& value) {
	DEBUG(Q_FUNC_INFO << ", value = " << value.toStdString())
	if (m_initializing)
		return;

	const Lock lock(m_initializing);
	bool ok;
	SET_NUMBER_LOCALE
	const double yMin = numberLocale.toDouble(value, &ok);
	if (ok) {
		// selected y range
		const int index{ sender()->property("row").toInt() };
		DEBUG( Q_FUNC_INFO << ", y range index: " << index )
		bool changed{false};
		for (auto* plot : m_plotList)
			if (!qFuzzyCompare(yMin, plot->yRange(index).start())) {
				plot->setYMin(index, yMin);
				changed = true;
			}

		if (changed) {
			updateXRangeList();	// plot is auto scaled
			updatePlotRangeList();
		}
	}
}

void CartesianPlotDock::xMaxChanged(const QString& value) {
	DEBUG(Q_FUNC_INFO << ", value = " << value.toStdString())
	if (m_initializing)
		return;

	const Lock lock(m_initializing);
	bool ok;
	SET_NUMBER_LOCALE
	const double xMax = numberLocale.toDouble(value, &ok);
	if (ok) {
		// selected x range
		const int index{ sender()->property("row").toInt() };
		DEBUG( Q_FUNC_INFO << ", x range index: " << index )
		bool changed{false};
		for (auto* plot : m_plotList)
			if (!qFuzzyCompare(xMax, plot->xRange(index).end())) {
				plot->setXMax(index, xMax);
				changed = true;
			}

		if (changed) {
			updateYRangeList();	// plot is auto scaled
			updatePlotRangeList();
		}
	}
}
void CartesianPlotDock::yMaxChanged(const QString& value) {
	DEBUG(Q_FUNC_INFO << ", value = " << value.toStdString())
	if (m_initializing)
		return;

	const Lock lock(m_initializing);
	bool ok;
	SET_NUMBER_LOCALE
	const double yMax = numberLocale.toDouble(value, &ok);
	if (ok) {
		// selected y range
		const int index{ sender()->property("row").toInt() };
		DEBUG( Q_FUNC_INFO << ", y range index: " << index )
		bool changed{false};
		for (auto* plot : m_plotList)
			if (!qFuzzyCompare(yMax, plot->yRange(index).end())) {
				plot->setYMax(index, yMax);
				changed = true;
			}

		if (changed) {
			updateXRangeList();	// plot is auto scaled
			updatePlotRangeList();
		}
	}
}

void CartesianPlotDock::xRangeChanged(const Range<double>& range) {
	if (m_initializing)
		return;

	// selected x range
	const int index{ sender()->property("row").toInt() };
	DEBUG( Q_FUNC_INFO << ", x range index: " << index )
	for (auto* plot : m_plotList)
			plot->setXRange(index, range);
	updatePlotRangeList();
}
void CartesianPlotDock::yRangeChanged(const Range<double>& range) {
	if (m_initializing)
		return;

	// selected y range
	const int index{ sender()->property("row").toInt() };
	DEBUG( Q_FUNC_INFO << ", y range index: " << index )
	for (auto* plot : m_plotList)
			plot->setYRange(index, range);
	updatePlotRangeList();
}

void CartesianPlotDock::xMinDateTimeChanged(const QDateTime& dateTime) {
	if (m_initializing)
		return;

	quint64 value = dateTime.toMSecsSinceEpoch();
	// selected x range
	const int index{ sender()->property("row").toInt() };
	DEBUG( Q_FUNC_INFO << ", x range index: " << index )
	for (auto* plot : m_plotList)
		plot->setXMin(index, value);
	updatePlotRangeList();
}
void CartesianPlotDock::yMinDateTimeChanged(const QDateTime& dateTime) {
	if (m_initializing)
		return;

	quint64 value = dateTime.toMSecsSinceEpoch();
	// selected y range
	const int index{ sender()->property("row").toInt() };
	DEBUG( Q_FUNC_INFO << ", y range index: " << index )
	for (auto* plot : m_plotList)
		plot->setYMin(index, value);
	updatePlotRangeList();
}

void CartesianPlotDock::xMaxDateTimeChanged(const QDateTime& dateTime) {
	if (m_initializing)
		return;

	quint64 value = dateTime.toMSecsSinceEpoch();
	// selected x range
	const int index{ sender()->property("row").toInt() };
	DEBUG( Q_FUNC_INFO << ", x range index: " << index )
	for (auto* plot : m_plotList)
		plot->setXMax(index, value);
	updatePlotRangeList();
}
void CartesianPlotDock::yMaxDateTimeChanged(const QDateTime& dateTime) {
	if (m_initializing)
		return;

	quint64 value = dateTime.toMSecsSinceEpoch();
	// selected y range
	const int index{ sender()->property("row").toInt() };
	DEBUG( Q_FUNC_INFO << ", y range index: " << index )
	for (auto* plot : m_plotList)
		plot->setYMax(index, value);
	updatePlotRangeList();
}

/*!
 *  called on scale changes (linear, log) for the x-/y-axis
 */
void CartesianPlotDock::xScaleChanged(int index) {
	if (m_initializing)
		return;

	const int xRangeIndex{ sender()->property("row").toInt() };
	DEBUG(Q_FUNC_INFO << ", x range " << xRangeIndex << " scale changed to " << index)
	const auto scale{ static_cast<RangeT::Scale>(index) };
	for (auto* plot : m_plotList)
		plot->setXRangeScale(xRangeIndex, scale);
}
void CartesianPlotDock::yScaleChanged(int index) {
	if (m_initializing)
		return;

	const int yRangeIndex{ sender()->property("row").toInt() };
	DEBUG(Q_FUNC_INFO << ", y range " << yRangeIndex << " scale changed to " << index)
	const auto scale{ static_cast<RangeT::Scale>(index) };
	for (auto* plot : m_plotList)
		plot->setYRangeScale(yRangeIndex, scale);
}

void CartesianPlotDock::xRangeFormatChanged(int index) {
	const int xRangeIndex{ sender()->property("row").toInt() };
	DEBUG(Q_FUNC_INFO << ", x range " << xRangeIndex+1 << " format = " << index)

	if (m_initializing)
		return;

	const auto format{ static_cast<RangeT::Format>(index) };
	for (auto* plot : m_plotList) {
		DEBUG(Q_FUNC_INFO << ", set format of x range " << xRangeIndex+1 << " to " << static_cast<int>(format))
		plot->setXRangeFormat(xRangeIndex, format);
	}
	updateXRangeList();
	updatePlotRangeList();
}
void CartesianPlotDock::yRangeFormatChanged(int index) {
	const int yRangeIndex{ sender()->property("row").toInt() };
	DEBUG(Q_FUNC_INFO << ", y range " << yRangeIndex+1 << " format = " << index)

	if (m_initializing)
		return;

	const auto format{ static_cast<RangeT::Format>(index) };
	for (auto* plot : m_plotList) {
		DEBUG(Q_FUNC_INFO << ", set format of y range " << yRangeIndex+1 << " to " << static_cast<int>(format))
		plot->setYRangeFormat(yRangeIndex, format);
	}
	updateYRangeList();
	updatePlotRangeList();
}


void CartesianPlotDock::addXRange() {
	if (!m_plot)
		return;

	DEBUG(Q_FUNC_INFO << ", current x range count = " << m_plot->xRangeCount())

	m_plot->addXRange();
	updateXRangeList();
}
void CartesianPlotDock::addYRange() {
	if (!m_plot)
		return;

	DEBUG(Q_FUNC_INFO << ", current y range count = " << m_plot->yRangeCount())

	m_plot->addYRange();
	updateYRangeList();
}

void CartesianPlotDock::removeXRange() {
	if (!m_plot)
		return;

	int currentRow{ ui.twXRanges->currentRow() };
	QDEBUG(Q_FUNC_INFO << ", current x range = " << currentRow)
	if (currentRow < 0 || currentRow > m_plot->xRangeCount()) {
		DEBUG(Q_FUNC_INFO << ", no current x range")
		currentRow = m_plot->xRangeCount() - 1;
	}
	QDEBUG(Q_FUNC_INFO << ", removing x range " << currentRow)

	// check plot ranges using range to remove
	const int cSystemCount{ m_plot->coordinateSystemCount() };
	DEBUG(Q_FUNC_INFO << ", nr of cSystems = " << cSystemCount)
	QString msg;
	for (int i{0}; i < cSystemCount; i++) {
		const auto* cSystem{ m_plot->coordinateSystem(i) };

		if (cSystem->xIndex() == currentRow) {
			if (msg.size() > 0)
				msg += ", ";
			msg += QString::number(i+1);
		}
	}

	if (msg.size() > 0) {
		DEBUG(Q_FUNC_INFO << ", x range used in plot range " << msg.toStdString())
		auto ret = KMessageBox::warningYesNo(this, i18n("X range %1 is used in plot range %2. ", currentRow+1, msg)
							+ i18n("Really remove it?"));
		if (ret == KMessageBox::No)
			return;
		else {
			// reset x ranges of cSystems using the range to be removed
			for (int i{0}; i < cSystemCount; i++) {
				auto* cSystem{ m_plot->coordinateSystem(i) };

				if (cSystem->xIndex() == currentRow)
					cSystem->setXIndex(0);	// first range
				else if (cSystem->xIndex() > currentRow)
					cSystem->setXIndex(cSystem->xIndex() - 1);
			}
		}
	}

	m_plot->removeXRange(currentRow);
	updateXRangeList();
}
void CartesianPlotDock::removeYRange() {
	if (!m_plot)
		return;

	int currentRow{ ui.twYRanges->currentRow() };
	QDEBUG(Q_FUNC_INFO << ", current y range = " << currentRow)
	if (currentRow < 0 || currentRow > m_plot->yRangeCount()) {
		DEBUG(Q_FUNC_INFO << ", no current y range")
		currentRow = m_plot->yRangeCount() - 1;
	}
	QDEBUG(Q_FUNC_INFO << ", removing y range " << currentRow)

	// check plot ranges using range to remove
	const int cSystemCount{ m_plot->coordinateSystemCount() };
	DEBUG(Q_FUNC_INFO << ", nr of cSystems = " << cSystemCount)
	QString msg;
	for (int i{0}; i < cSystemCount; i++) {
		const auto* cSystem{ m_plot->coordinateSystem(i) };

		if (cSystem->yIndex() == currentRow) {
			if (msg.size() > 0)
				msg += ", ";
			msg += QString::number(i+1);
		}
	}

	if (msg.size() > 0) {
		DEBUG(Q_FUNC_INFO << ", y range used in plot range " << msg.toStdString())
		auto ret = KMessageBox::warningYesNo(this, i18n("Y range %1 is used in plot range %2. ", currentRow+1, msg)
							+ i18n("Really remove it?"));
		if (ret == KMessageBox::No)
			return;
		else {
			// reset y ranges of cSystems using the range to be removed
			for (int i{0}; i < cSystemCount; i++) {
				auto* cSystem{ m_plot->coordinateSystem(i) };

				if (cSystem->yIndex() == currentRow)
					cSystem->setYIndex(0);	// first range
				else if (cSystem->yIndex() > currentRow)
					cSystem->setYIndex(cSystem->yIndex() - 1);
			}
		}
	}

	m_plot->removeYRange(currentRow);
	updateYRangeList();
}

// plot ranges

void CartesianPlotDock::addPlotRange() {
	if (!m_plot)
		return;

	m_plot->addCoordinateSystem();
	updatePlotRangeList();
}

void CartesianPlotDock::removePlotRange() {
	DEBUG(Q_FUNC_INFO)

	int currentRow{ ui.twPlotRanges->currentRow() };
	QDEBUG(Q_FUNC_INFO << ", current plot range = " << currentRow)
	if (currentRow < 0 || currentRow > m_plot->coordinateSystemCount()) {
		DEBUG(Q_FUNC_INFO << ", no current plot range")
		currentRow = m_plot->coordinateSystemCount() - 1;
	}
	QDEBUG(Q_FUNC_INFO << ", removing plot range " << currentRow)

	// check all children for cSystem usage
	for (auto* element : m_plot->children<WorksheetElement>()) {
		const int cSystemIndex{ element->coordinateSystemIndex() };
		DEBUG(Q_FUNC_INFO << ", element x index = " << cSystemIndex)
		if (cSystemIndex == currentRow) {
			DEBUG(Q_FUNC_INFO << ", WARNING: plot range used in element")
			auto ret = KMessageBox::warningYesNo(this, i18n("Plot range %1 is used by element \"%2\". ", currentRow+1, element->name())
								+ i18n("Really remove it?"));
			if (ret == KMessageBox::No)
				return;
			else
				element->setCoordinateSystemIndex(0);	// reset
		}
	}

	m_plot->removeCoordinateSystem(currentRow);
	updatePlotRangeList();
	m_plot->retransform();	// update plot and elements
}

/*
 * Called when x/y range of plot range in plot range list changes
 */
void CartesianPlotDock::PlotRangeXChanged(const int index) {
	const int plotRangeIndex{ sender()->property("row").toInt() };
	DEBUG(Q_FUNC_INFO << ", Set x range of plot range " << plotRangeIndex+1  << " to " << index+1)
	auto* cSystem{ m_plot->coordinateSystem(plotRangeIndex) };
	cSystem->setXIndex(index);

	// auto scale x range when on auto scale (now that it is used)
	if (m_plot->xRange(index).autoScale()) {
		autoScaleXRange(index, true);
		updateXRangeList();
	}

	for (auto* axis : m_plot->children<Axis>()) {
		const int cSystemIndex{ axis->coordinateSystemIndex() };
		DEBUG(Q_FUNC_INFO << ", Axis \"" << axis->name().toStdString() << "\" cSystem index = " << cSystemIndex)
		if (cSystemIndex == plotRangeIndex) {
			DEBUG(Q_FUNC_INFO << ", Plot range used in axis \"" << axis->name().toStdString() << "\" has changed")
			if (axis->autoScale() && axis->orientation() == Axis::Orientation::Horizontal) {
				DEBUG(Q_FUNC_INFO << ", set x range of axis to " << m_plot->xRange(index).toStdString())
				axis->setRange(m_plot->xRange(index));
			}
		}
	}

	m_plot->dataChanged();	// update plot
}
void CartesianPlotDock::PlotRangeYChanged(const int index) {
	const int plotRangeIndex{ sender()->property("row").toInt() };
	DEBUG(Q_FUNC_INFO << ", set y range of plot range " << plotRangeIndex+1  << " to " << index+1)
	auto* cSystem{ m_plot->coordinateSystem(plotRangeIndex) };
	cSystem->setYIndex(index);

	// auto scale y range when on auto scale (now that it is used)
	if (m_plot->yRange(index).autoScale()) {
		autoScaleYRange(index, true);
		updateYRangeList();
	}
	for (auto* axis : m_plot->children<Axis>()) {
		const int cSystemIndex{ axis->coordinateSystemIndex() };
		if (cSystemIndex == plotRangeIndex) {
			DEBUG(Q_FUNC_INFO << ", plot range used in axis \"" << axis->name().toStdString() << "\" has changed")
			if (axis->autoScale() && axis->orientation() == Axis::Orientation::Vertical) {
				DEBUG(Q_FUNC_INFO << ", set range to " << m_plot->yRange(index).toStdString())
				axis->setRange(m_plot->yRange(index));
			}
		}
	}

	m_plot->dataChanged();	// update plot
}

// "Range Breaks"-tab

// x-range breaks
void CartesianPlotDock::toggleXBreak(bool b) {
	ui.frameXBreakEdit->setEnabled(b);
	ui.leXBreakStart->setEnabled(b);
	ui.leXBreakEnd->setEnabled(b);
	ui.sbXBreakPosition->setEnabled(b);
	ui.cbXBreakStyle->setEnabled(b);

	if (m_initializing)
		return;

	for (auto* plot : m_plotList)
		plot->setXRangeBreakingEnabled(b);
}

void CartesianPlotDock::addXBreak() {
	ui.bRemoveXBreak->setVisible(true);

	CartesianPlot::RangeBreaks breaks = m_plot->xRangeBreaks();
	CartesianPlot::RangeBreak b;
	breaks.list<<b;
	breaks.lastChanged = breaks.list.size() - 1;
	for (auto* plot : m_plotList)
		plot->setXRangeBreaks(breaks);

	ui.cbXBreak->addItem(QString::number(ui.cbXBreak->count() + 1));
	ui.cbXBreak->setCurrentIndex(ui.cbXBreak->count() - 1);
}

void CartesianPlotDock::removeXBreak() {
	ui.bRemoveXBreak->setVisible(m_plot->xRangeBreaks().list.size() > 1);
	int index = ui.cbXBreak->currentIndex();
	CartesianPlot::RangeBreaks breaks = m_plot->xRangeBreaks();
	breaks.list.takeAt(index);
	breaks.lastChanged = -1;
	for (auto* plot : m_plotList)
		plot->setXRangeBreaks(breaks);

	ui.cbXBreak->clear();
	for (int i = 1; i <= breaks.list.size(); ++i)
		ui.cbXBreak->addItem(QString::number(i));

	if (index < ui.cbXBreak->count() - 1)
		ui.cbXBreak->setCurrentIndex(index);
	else
		ui.cbXBreak->setCurrentIndex(ui.cbXBreak->count() - 1);

	ui.bRemoveXBreak->setVisible(ui.cbXBreak->count() != 1);
}

void CartesianPlotDock::currentXBreakChanged(int index) {
	if (m_initializing)
		return;

	if (index == -1)
		return;

	m_initializing = true;
	SET_NUMBER_LOCALE
	const CartesianPlot::RangeBreak rangeBreak = m_plot->xRangeBreaks().list.at(index);
	QString str = qIsNaN(rangeBreak.range.start()) ? QString() : numberLocale.toString(rangeBreak.range.start());
	ui.leXBreakStart->setText(str);
	str = std::isnan(rangeBreak.range.end()) ? QString() : numberLocale.toString(rangeBreak.range.end());
	ui.leXBreakEnd->setText(str);
	ui.sbXBreakPosition->setValue(rangeBreak.position*100);
	ui.cbXBreakStyle->setCurrentIndex((int)rangeBreak.style);
	m_initializing = false;
}

void CartesianPlotDock::xBreakStartChanged() {
	if (m_initializing)
		return;

	int index = ui.cbXBreak->currentIndex();
	CartesianPlot::RangeBreaks breaks = m_plot->xRangeBreaks();
	breaks.list[index].range.start() = ui.leXBreakStart->text().toDouble();
	breaks.lastChanged = index;

	for (auto* plot : m_plotList)
		plot->setXRangeBreaks(breaks);
}

void CartesianPlotDock::xBreakEndChanged() {
	if (m_initializing)
		return;

	int index = ui.cbXBreak->currentIndex();
	CartesianPlot::RangeBreaks breaks = m_plot->xRangeBreaks();
	breaks.list[index].range.end() = ui.leXBreakEnd->text().toDouble();
	breaks.lastChanged = index;

	for (auto* plot : m_plotList)
		plot->setXRangeBreaks(breaks);
}

void CartesianPlotDock::xBreakPositionChanged(int value) {
	if (m_initializing)
		return;

	int index = ui.cbXBreak->currentIndex();
	CartesianPlot::RangeBreaks breaks = m_plot->xRangeBreaks();
	breaks.list[index].position = (double)value/100.;
	breaks.lastChanged = index;

	for (auto* plot : m_plotList)
		plot->setXRangeBreaks(breaks);
}

void CartesianPlotDock::xBreakStyleChanged(int styleIndex) {
	if (m_initializing)
		return;

	int index = ui.cbXBreak->currentIndex();
	auto style = CartesianPlot::RangeBreakStyle(styleIndex);
	CartesianPlot::RangeBreaks breaks = m_plot->xRangeBreaks();
	breaks.list[index].style = style;
	breaks.lastChanged = index;

	for (auto* plot : m_plotList)
		plot->setXRangeBreaks(breaks);
}

// y-range breaks
void CartesianPlotDock::toggleYBreak(bool b) {
	ui.frameYBreakEdit->setEnabled(b);
	ui.leYBreakStart->setEnabled(b);
	ui.leYBreakEnd->setEnabled(b);
	ui.sbYBreakPosition->setEnabled(b);
	ui.cbYBreakStyle->setEnabled(b);

	if (m_initializing)
		return;

	for (auto* plot : m_plotList)
		plot->setYRangeBreakingEnabled(b);
}

void CartesianPlotDock::addYBreak() {
	ui.bRemoveYBreak->setVisible(true);

	CartesianPlot::RangeBreaks breaks = m_plot->yRangeBreaks();
	CartesianPlot::RangeBreak b;
	breaks.list << b;
	breaks.lastChanged = breaks.list.size() - 1;
	for (auto* plot : m_plotList)
		plot->setYRangeBreaks(breaks);

	ui.cbYBreak->addItem(QString::number(ui.cbYBreak->count() + 1));
	ui.cbYBreak->setCurrentIndex(ui.cbYBreak->count()-1);
}

void CartesianPlotDock::removeYBreak() {
	ui.bRemoveYBreak->setVisible(m_plot->yRangeBreaks().list.size() > 1);
	int index = ui.cbYBreak->currentIndex();
	CartesianPlot::RangeBreaks breaks = m_plot->yRangeBreaks();
	breaks.list.takeAt(index);
	breaks.lastChanged = -1;
	for (auto* plot : m_plotList)
		plot->setYRangeBreaks(breaks);

	ui.cbYBreak->clear();
	for (int i = 1; i <= breaks.list.size(); ++i)
		ui.cbYBreak->addItem(QString::number(i));

	if (index < ui.cbYBreak->count()-1)
		ui.cbYBreak->setCurrentIndex(index);
	else
		ui.cbYBreak->setCurrentIndex(ui.cbYBreak->count()-1);

	ui.bRemoveYBreak->setVisible(ui.cbYBreak->count() != 1);
}

void CartesianPlotDock::currentYBreakChanged(int index) {
	if (m_initializing)
		return;

	if (index == -1)
		return;

	m_initializing = true;
	SET_NUMBER_LOCALE
	const CartesianPlot::RangeBreak rangeBreak = m_plot->yRangeBreaks().list.at(index);
	QString str = qIsNaN(rangeBreak.range.start()) ? QString() : numberLocale.toString(rangeBreak.range.start());
	ui.leYBreakStart->setText(str);
	str = std::isnan(rangeBreak.range.end()) ? QString() : numberLocale.toString(rangeBreak.range.end());
	ui.leYBreakEnd->setText(str);
	ui.sbYBreakPosition->setValue(rangeBreak.position * 100);
	ui.cbYBreakStyle->setCurrentIndex((int)rangeBreak.style);
	m_initializing = false;
}

void CartesianPlotDock::yBreakStartChanged() {
	if (m_initializing)
		return;

	int index = ui.cbYBreak->currentIndex();
	CartesianPlot::RangeBreaks breaks = m_plot->yRangeBreaks();
	breaks.list[index].range.start() = ui.leYBreakStart->text().toDouble();
	breaks.lastChanged = index;

	for (auto* plot : m_plotList)
		plot->setYRangeBreaks(breaks);
}

void CartesianPlotDock::yBreakEndChanged() {
	if (m_initializing)
		return;

	int index = ui.cbYBreak->currentIndex();
	CartesianPlot::RangeBreaks breaks = m_plot->yRangeBreaks();
	breaks.list[index].range.end() = ui.leYBreakEnd->text().toDouble();
	breaks.lastChanged = index;

	for (auto* plot : m_plotList)
		plot->setYRangeBreaks(breaks);
}

void CartesianPlotDock::yBreakPositionChanged(int value) {
	if (m_initializing)
		return;

	int index = ui.cbYBreak->currentIndex();
	CartesianPlot::RangeBreaks breaks = m_plot->yRangeBreaks();
	breaks.list[index].position = (float)value/100.;
	breaks.lastChanged = index;

	for (auto* plot : m_plotList)
		plot->setYRangeBreaks(breaks);
}

void CartesianPlotDock::yBreakStyleChanged(int styleIndex) {
	if (m_initializing)
		return;

	int index = ui.cbYBreak->currentIndex();
	auto style = CartesianPlot::RangeBreakStyle(styleIndex);
	CartesianPlot::RangeBreaks breaks = m_plot->yRangeBreaks();
	breaks.list[index].style = style;
	breaks.lastChanged = index;

	for (auto* plot : m_plotList)
		plot->setYRangeBreaks(breaks);
}

// "Plot area"-tab
void CartesianPlotDock::backgroundTypeChanged(int index) {
	auto type = (PlotArea::BackgroundType)index;

	if (type == PlotArea::BackgroundType::Color) {
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

		auto style = (PlotArea::BackgroundColorStyle) ui.cbBackgroundColorStyle->currentIndex();
		if (style == PlotArea::BackgroundColorStyle::SingleColor) {
			ui.lBackgroundFirstColor->setText(i18n("Color:"));
			ui.lBackgroundSecondColor->hide();
			ui.kcbBackgroundSecondColor->hide();
		} else {
			ui.lBackgroundFirstColor->setText(i18n("First color:"));
			ui.lBackgroundSecondColor->show();
			ui.kcbBackgroundSecondColor->show();
		}
	} else if (type == PlotArea::BackgroundType::Image) {
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
	} else if (type == PlotArea::BackgroundType::Pattern) {
		ui.lBackgroundFirstColor->setText(i18n("Color:"));
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

	for (auto* plot : m_plotList)
		plot->plotArea()->setBackgroundType(type);
}

void CartesianPlotDock::backgroundColorStyleChanged(int index) {
	auto style = (PlotArea::BackgroundColorStyle)index;

	if (style == PlotArea::BackgroundColorStyle::SingleColor) {
		ui.lBackgroundFirstColor->setText(i18n("Color:"));
		ui.lBackgroundSecondColor->hide();
		ui.kcbBackgroundSecondColor->hide();
	} else {
		ui.lBackgroundFirstColor->setText(i18n("First color:"));
		ui.lBackgroundSecondColor->show();
		ui.kcbBackgroundSecondColor->show();
		ui.lBackgroundBrushStyle->hide();
		ui.cbBackgroundBrushStyle->hide();
	}

	if (m_initializing)
		return;

	for (auto* plot : m_plotList)
		plot->plotArea()->setBackgroundColorStyle(style);
}

void CartesianPlotDock::backgroundImageStyleChanged(int index) {
	if (m_initializing)
		return;

	auto style = (PlotArea::BackgroundImageStyle)index;
	for (auto* plot : m_plotList)
		plot->plotArea()->setBackgroundImageStyle(style);
}

void CartesianPlotDock::backgroundBrushStyleChanged(int index) {
	if (m_initializing)
		return;

	auto style = (Qt::BrushStyle)index;
	for (auto* plot : m_plotList)
		plot->plotArea()->setBackgroundBrushStyle(style);
}

void CartesianPlotDock::backgroundFirstColorChanged(const QColor& c) {
	if (m_initializing)
		return;

	for (auto* plot : m_plotList)
		plot->plotArea()->setBackgroundFirstColor(c);
}

void CartesianPlotDock::backgroundSecondColorChanged(const QColor& c) {
	if (m_initializing)
		return;

	for (auto* plot : m_plotList)
		plot->plotArea()->setBackgroundSecondColor(c);
}

/*!
    opens a file dialog and lets the user select the image file.
*/
void CartesianPlotDock::selectFile() {
	KConfigGroup conf(KSharedConfig::openConfig(), "CartesianPlotDock");
	QString dir = conf.readEntry("LastImageDir", "");

	QString formats;
	for (const auto& format : QImageReader::supportedImageFormats()) {
		QString f = "*." + QString(format.constData());
		if (f == QLatin1String("*.svg"))
			continue;
		formats.isEmpty() ? formats += f : formats += ' ' + f;
	}

	QString path = QFileDialog::getOpenFileName(this, i18n("Select the image file"), dir, i18n("Images (%1)", formats));
	if (path.isEmpty())
		return; //cancel was clicked in the file-dialog

	const int pos = path.lastIndexOf(QLatin1String("/"));
	if (pos != -1) {
		QString newDir{path.left(pos)};
		if (newDir != dir)
			conf.writeEntry("LastImageDir", newDir);
	}

	ui.leBackgroundFileName->setText(path);

	for (auto* plot : m_plotList)
		plot->plotArea()->setBackgroundFileName(path);
}

void CartesianPlotDock::fileNameChanged() {
	if (m_initializing)
		return;

	const QString& fileName = ui.leBackgroundFileName->text();
	bool invalid = (!fileName.isEmpty() && !QFile::exists(fileName));
	GuiTools::highlight(ui.leBackgroundFileName, invalid);

	for (auto* plot : m_plotList)
		plot->plotArea()->setBackgroundFileName(fileName);
}

void CartesianPlotDock::backgroundOpacityChanged(int value) {
	if (m_initializing)
		return;

	qreal opacity = (double)value/100.;
	for (auto* plot : m_plotList)
		plot->plotArea()->setBackgroundOpacity(opacity);
}

// "Border"-tab
void CartesianPlotDock::borderStyleChanged(int index) {
	if (m_initializing)
		return;

	auto penStyle = Qt::PenStyle(index);
	QPen pen;
	for (auto* plot : m_plotList) {
		pen = plot->plotArea()->borderPen();
		pen.setStyle(penStyle);
		plot->plotArea()->setBorderPen(pen);
	}
}

void CartesianPlotDock::borderColorChanged(const QColor& color) {
	if (m_initializing)
		return;

	QPen pen;
	for (auto* plot : m_plotList) {
		pen = plot->plotArea()->borderPen();
		pen.setColor(color);
		plot->plotArea()->setBorderPen(pen);
	}

	m_initializing = true;
	GuiTools::updatePenStyles(ui.cbBorderStyle, color);
	m_initializing = false;
}

void CartesianPlotDock::borderWidthChanged(double value) {
	if (m_initializing)
		return;

	QPen pen;
	for (auto* plot : m_plotList) {
		pen = plot->plotArea()->borderPen();
		pen.setWidthF( Worksheet::convertToSceneUnits(value, Worksheet::Unit::Point) );
		plot->plotArea()->setBorderPen(pen);
	}
}

void CartesianPlotDock::borderCornerRadiusChanged(double value) {
	if (m_initializing)
		return;

	for (auto* plot : m_plotList)
		plot->plotArea()->setBorderCornerRadius(Worksheet::convertToSceneUnits(value, m_worksheetUnit));
}

void CartesianPlotDock::borderOpacityChanged(int value) {
	if (m_initializing)
		return;

	qreal opacity = (double)value/100.;
	for (auto* plot : m_plotList)
		plot->plotArea()->setBorderOpacity(opacity);
}

void CartesianPlotDock::symmetricPaddingChanged(bool checked) {
	ui.lPaddingHorizontalRight->setVisible(!checked);
	ui.sbPaddingRight->setVisible(!checked);
	ui.lPaddingVerticalDown->setVisible(!checked);
	ui.sbPaddingBottom->setVisible(!checked);

	if (checked) {
		ui.lPaddingHorizontal->setText(i18n("Horizontal:"));
		ui.lPaddingVertical->setText(i18n("Vertical:"));
	} else {
		ui.lPaddingHorizontal->setText(i18n("Left:"));
		ui.lPaddingVertical->setText(i18n("Top:"));
	}

	if (m_initializing)
		return;

	for (auto* plot : m_plotList)
		plot->setSymmetricPadding(checked);

	if (checked) {
		rightPaddingChanged(ui.sbPaddingHorizontal->value());
		bottomPaddingChanged(ui.sbPaddingVertical->value());
	}
}

void CartesianPlotDock::horizontalPaddingChanged(double value) {
	if (m_initializing)
		return;
	double padding = Worksheet::convertToSceneUnits(value, m_worksheetUnit);
	for (auto* plot : m_plotList) {
		//if symmetric padding is active we also adjust the right padding.
		//start a macro in this case to only have one single entry on the undo stack.
		//TODO: ideally this is done in CartesianPlot and is completely transparent to CartesianPlotDock.
		const bool sym = m_plot->symmetricPadding();
		if (sym)
			plot->beginMacro(i18n("%1: set horizontal padding", plot->name()));

		plot->setHorizontalPadding(padding);

		if (sym) {
			plot->setRightPadding(padding);
			plot->endMacro();
		}
	}
}

void CartesianPlotDock::rightPaddingChanged(double value) {
	if (m_initializing)
		return;
	double padding = Worksheet::convertToSceneUnits(value, m_worksheetUnit);
	for (auto* plot : m_plotList)
		plot->setRightPadding(padding);
}

void CartesianPlotDock::verticalPaddingChanged(double value) {
	if (m_initializing)
		return;

	const double padding = Worksheet::convertToSceneUnits(value, m_worksheetUnit);
	for (auto* plot : m_plotList) {
		const bool sym = m_plot->symmetricPadding();
		if (sym)
			plot->beginMacro(i18n("%1: set vertical padding", plot->name()));

		plot->setVerticalPadding(padding);

		if (sym) {
			plot->setBottomPadding(padding);
			plot->endMacro();
		}
	}
}

void CartesianPlotDock::bottomPaddingChanged(double value) {
	if (m_initializing)
		return;
	double padding = Worksheet::convertToSceneUnits(value, m_worksheetUnit);
	for (auto* plot : m_plotList)
		plot->setBottomPadding(padding);
}

void CartesianPlotDock::cursorLineWidthChanged(int width) {
	if (m_initializing)
		return;

	for (auto* plot : m_plotList) {
		QPen pen = plot->cursorPen();
		pen.setWidthF( Worksheet::convertToSceneUnits(width, Worksheet::Unit::Point) );
		plot->setCursorPen(pen);
	}
}

void CartesianPlotDock::cursorLineColorChanged(const QColor& color) {
	if (m_initializing)
		return;

	for (auto* plot : m_plotList) {
		QPen pen = plot->cursorPen();
		pen.setColor(color);
		plot->setCursorPen(pen);
	}

	m_initializing = true;
	GuiTools::updatePenStyles(ui.cbCursorLineStyle, color);
	m_initializing = false;
}

void CartesianPlotDock::cursorLineStyleChanged(int index) {
	if (m_initializing)
		return;

	if (index > 5)
		return;

	for (auto* plot : m_plotList) {
		QPen pen = plot->cursorPen();
		pen.setStyle(static_cast<Qt::PenStyle>(index));
		plot->setCursorPen(pen);
	}
}

//*************************************************************
//****** SLOTs for changes triggered in CartesianPlot *********
//*************************************************************
//general
void CartesianPlotDock::plotDescriptionChanged(const AbstractAspect* aspect) {
	if (m_plot != aspect)
		return;

	m_initializing = true;
	if (aspect->name() != ui.leName->text())
		ui.leName->setText(aspect->name());
	else if (aspect->comment() != ui.leComment->text())
		ui.leComment->setText(aspect->comment());
	m_initializing = false;
}

void CartesianPlotDock::plotRectChanged(QRectF& rect) {
	m_initializing = true;
	ui.sbLeft->setValue(Worksheet::convertFromSceneUnits(rect.x(), m_worksheetUnit));
	ui.sbTop->setValue(Worksheet::convertFromSceneUnits(rect.y(), m_worksheetUnit));
	ui.sbWidth->setValue(Worksheet::convertFromSceneUnits(rect.width(), m_worksheetUnit));
	ui.sbHeight->setValue(Worksheet::convertFromSceneUnits(rect.height(), m_worksheetUnit));
	m_initializing = false;
}

void CartesianPlotDock::plotRangeTypeChanged(CartesianPlot::RangeType type) {
	m_initializing = true;
	switch (type) {
	case CartesianPlot::RangeType::Free:
		ui.rbRangeFree->setChecked(true);
		break;
	case CartesianPlot::RangeType::First:
		ui.rbRangeFirst->setChecked(true);
		break;
	case CartesianPlot::RangeType::Last:
		ui.rbRangeLast->setChecked(true);
		break;
	}
	m_initializing = false;
}

void CartesianPlotDock::plotRangeFirstValuesChanged(int value) {
	m_initializing = true;
	SET_NUMBER_LOCALE
	ui.leRangeFirst->setText(numberLocale.toString(value));
	m_initializing = false;
}

void CartesianPlotDock::plotRangeLastValuesChanged(int value) {
	m_initializing = true;
	SET_NUMBER_LOCALE
	ui.leRangeLast->setText(numberLocale.toString(value));
	m_initializing = false;
}

// x & y ranges
void CartesianPlotDock::plotXAutoScaleChanged(bool checked) {
	DEBUG(Q_FUNC_INFO << ", checked = " << checked)
	m_initializing = true;
	//OLD: ui.chkAutoScaleX->setChecked(value);
	const int index{ m_plot->defaultCoordinateSystem()->xIndex() };
	qobject_cast<QCheckBox*>(ui.twXRanges->cellWidget(index, 0))->setChecked(checked);
	m_initializing = false;
}
void CartesianPlotDock::plotYAutoScaleChanged(bool checked) {
	DEBUG(Q_FUNC_INFO << ", checked = " << checked)
	m_initializing = true;
	//OLD: ui.chkAutoScaleY->setChecked(value);
	const int index{ m_plot->defaultCoordinateSystem()->yIndex() };
	qobject_cast<QCheckBox*>(ui.twYRanges->cellWidget(index, 0))->setChecked(checked);
	m_initializing = false;
}

void CartesianPlotDock::plotXMinChanged(double value) {
	DEBUG(Q_FUNC_INFO << ", value = " << value)
	if (m_initializing)
		return;
	const Lock lock(m_initializing);
	SET_NUMBER_LOCALE
	const int index{ m_plot ? m_plot->defaultCoordinateSystem()->xIndex() : 0 };
	auto* le = qobject_cast<QLineEdit*>(ui.twXRanges->cellWidget(index, 2));
	if (le)	// Numeric
		le->setText( numberLocale.toString(value) );
	auto* dte = qobject_cast<QDateTimeEdit*>(ui.twXRanges->cellWidget(index, 2));
	if (dte) // DateTime
		dte->setDateTime( QDateTime::fromMSecsSinceEpoch(value) );
}
void CartesianPlotDock::plotYMinChanged(double value) {
	DEBUG(Q_FUNC_INFO << ", value = " << value)
	if (m_initializing)
		return;
	const Lock lock(m_initializing);
	SET_NUMBER_LOCALE
	const int index{ m_plot ? m_plot->defaultCoordinateSystem()->yIndex() : 0 };
	auto* le = qobject_cast<QLineEdit*>(ui.twYRanges->cellWidget(index, 2));
	if (le)	// Numeric
		le->setText( numberLocale.toString(value) );
	auto* dte = qobject_cast<QDateTimeEdit*>(ui.twYRanges->cellWidget(index, 2));
	if (dte) // DateTime
		dte->setDateTime( QDateTime::fromMSecsSinceEpoch(value) );
}

void CartesianPlotDock::plotXMaxChanged(double value) {
	DEBUG(Q_FUNC_INFO << ", value = " << value)
	if (m_initializing)
		return;
	const Lock lock(m_initializing);
	SET_NUMBER_LOCALE
	const int index{ m_plot ? m_plot->defaultCoordinateSystem()->xIndex() : 0 };
	auto* le = qobject_cast<QLineEdit*>(ui.twXRanges->cellWidget(index, 3));
	if (le)	// Numeric
		le->setText(numberLocale.toString(value));
	auto* dte = qobject_cast<QDateTimeEdit*>(ui.twXRanges->cellWidget(index, 3));
	if (dte)	// DateTime
		dte->setDateTime( QDateTime::fromMSecsSinceEpoch(value) );
}
void CartesianPlotDock::plotYMaxChanged(double value) {
	DEBUG(Q_FUNC_INFO << ", value = " << value)
	if (m_initializing)
		return;
	const Lock lock(m_initializing);
	SET_NUMBER_LOCALE
	const int index{ m_plot ? m_plot->defaultCoordinateSystem()->yIndex() : 0 };
	auto* le = qobject_cast<QLineEdit*>(ui.twYRanges->cellWidget(index, 3));
	if (le)	// Numeric
		le->setText(numberLocale.toString(value));
	auto* dte = qobject_cast<QDateTimeEdit*>(ui.twYRanges->cellWidget(index, 3));
	if (dte)	// DateTime
		dte->setDateTime( QDateTime::fromMSecsSinceEpoch(value) );
}

void CartesianPlotDock::plotXRangeChanged(Range<double> range) {
	DEBUG(Q_FUNC_INFO << ", x range = " << range.toStdString())
        if (m_initializing)
                return;

	const Lock lock(m_initializing);
	SET_NUMBER_LOCALE
	const int index{ m_plot ? m_plot->defaultCoordinateSystem()->xIndex() : 0 };
	auto* le = qobject_cast<QLineEdit*>(ui.twXRanges->cellWidget(index, 2));
	if (le) {	// Numeric
		le->setText( numberLocale.toString(range.start()) );
		le = qobject_cast<QLineEdit*>(ui.twXRanges->cellWidget(index, 3));
		le->setText( numberLocale.toString(range.end()) );
	}
	auto* dte = qobject_cast<QDateTimeEdit*>(ui.twXRanges->cellWidget(index, 2));
	if (dte) {	// DateTime
		dte->setDateTime( QDateTime::fromMSecsSinceEpoch(range.start()) );
		dte = qobject_cast<QDateTimeEdit*>(ui.twXRanges->cellWidget(index, 3));
		dte->setDateTime( QDateTime::fromMSecsSinceEpoch(range.end()) );
	}
}
void CartesianPlotDock::plotYRangeChanged(Range<double> range) {
	DEBUG(Q_FUNC_INFO)
	if (m_initializing)
		return;

	const Lock lock(m_initializing);
	SET_NUMBER_LOCALE
	const int index{ m_plot ? m_plot->defaultCoordinateSystem()->yIndex() : 0 };
	auto* le = qobject_cast<QLineEdit*>(ui.twYRanges->cellWidget(index, 2));
	if (le) {	// Numeric
		le->setText( numberLocale.toString(range.start()) );
		le = qobject_cast<QLineEdit*>(ui.twYRanges->cellWidget(index, 3));
		le->setText( numberLocale.toString(range.end()) );
	}
	auto* dte = qobject_cast<QDateTimeEdit*>(ui.twYRanges->cellWidget(index, 2));
	if (dte) {	// DateTime
		dte->setDateTime( QDateTime::fromMSecsSinceEpoch(range.start()) );
		dte = qobject_cast<QDateTimeEdit*>(ui.twYRanges->cellWidget(index, 3));
		dte->setDateTime( QDateTime::fromMSecsSinceEpoch(range.end()) );
	}
}

void CartesianPlotDock::plotXScaleChanged(RangeT::Scale scale) {
	DEBUG(Q_FUNC_INFO << ", scale = " << (int)scale)
	m_initializing = true;
	const int index{ m_plot ? m_plot->defaultCoordinateSystem()->xIndex() : 0 };
	qobject_cast<QComboBox*>(ui.twXRanges->cellWidget(index, 4))->setCurrentIndex(static_cast<int>(scale));
	m_initializing = false;
}
void CartesianPlotDock::plotYScaleChanged(RangeT::Scale scale) {
	m_initializing = true;
	const int index{ m_plot ? m_plot->defaultCoordinateSystem()->yIndex() : 0 };
	qobject_cast<QComboBox*>(ui.twYRanges->cellWidget(index, 4))->setCurrentIndex(static_cast<int>(scale));
	m_initializing = false;
}

void CartesianPlotDock::plotXRangeFormatChanged(RangeT::Format format) {
	DEBUG(Q_FUNC_INFO << ", format = " << static_cast<int>(format))
	m_initializing = true;
	const int index{ m_plot ? m_plot->defaultCoordinateSystem()->xIndex() : 0 };
	qobject_cast<QComboBox*>(ui.twXRanges->cellWidget(index, 1))->setCurrentIndex(static_cast<int>(format));
	m_initializing = false;
}
void CartesianPlotDock::plotYRangeFormatChanged(RangeT::Format format) {
	m_initializing = true;
	const int index{ m_plot ? m_plot->defaultCoordinateSystem()->yIndex() : 0 };
	qobject_cast<QComboBox*>(ui.twYRanges->cellWidget(index, 1))->setCurrentIndex(static_cast<int>(format));
	m_initializing = false;
}

// plot range
void CartesianPlotDock::defaultPlotRangeChanged() {
	const int index{ m_bgDefaultPlotRange->checkedId() };
	DEBUG(Q_FUNC_INFO << ", index = " << index)
	m_plot->setDefaultCoordinateSystemIndex(index);
	updatePlotRangeList();	// changing default cSystem may change x/y-ranges when on auto scale
	m_plot->retransform();	// update plot
}

//range breaks
void CartesianPlotDock::plotXRangeBreakingEnabledChanged(bool on) {
	m_initializing = true;
	ui.chkXBreak->setChecked(on);
	m_initializing = false;
}

void CartesianPlotDock::plotXRangeBreaksChanged(const CartesianPlot::RangeBreaks& breaks) {
	Q_UNUSED(breaks);
}

void CartesianPlotDock::plotYRangeBreakingEnabledChanged(bool on) {
	m_initializing = true;
	ui.chkYBreak->setChecked(on);
	m_initializing = false;
}

void CartesianPlotDock::plotYRangeBreaksChanged(const CartesianPlot::RangeBreaks& breaks) {
	Q_UNUSED(breaks);
}

void CartesianPlotDock::plotVisibleChanged(bool on) {
	m_initializing = true;
	ui.chkVisible->setChecked(on);
	m_initializing = false;
}

//background
void CartesianPlotDock::plotBackgroundTypeChanged(PlotArea::BackgroundType type) {
	m_initializing = true;
	ui.cbBackgroundType->setCurrentIndex(static_cast<int>(type));
	m_initializing = false;
}

void CartesianPlotDock::plotBackgroundColorStyleChanged(PlotArea::BackgroundColorStyle style) {
	m_initializing = true;
	ui.cbBackgroundColorStyle->setCurrentIndex(static_cast<int>(style));
	m_initializing = false;
}

void CartesianPlotDock::plotBackgroundImageStyleChanged(PlotArea::BackgroundImageStyle style) {
	m_initializing = true;
	ui.cbBackgroundImageStyle->setCurrentIndex(static_cast<int>(style));
	m_initializing = false;
}

void CartesianPlotDock::plotBackgroundBrushStyleChanged(Qt::BrushStyle style) {
	m_initializing = true;
	ui.cbBackgroundBrushStyle->setCurrentIndex(style);
	m_initializing = false;
}

void CartesianPlotDock::plotBackgroundFirstColorChanged(QColor& color) {
	m_initializing = true;
	ui.kcbBackgroundFirstColor->setColor(color);
	m_initializing = false;
}

void CartesianPlotDock::plotBackgroundSecondColorChanged(QColor& color) {
	m_initializing = true;
	ui.kcbBackgroundSecondColor->setColor(color);
	m_initializing = false;
}

void CartesianPlotDock::plotBackgroundFileNameChanged(QString& filename) {
	m_initializing = true;
	ui.leBackgroundFileName->setText(filename);
	m_initializing = false;
}

void CartesianPlotDock::plotBackgroundOpacityChanged(float opacity) {
	m_initializing = true;
	ui.sbBackgroundOpacity->setValue( round(opacity*100.0) );
	m_initializing = false;
}

void CartesianPlotDock::plotBorderPenChanged(QPen& pen) {
	m_initializing = true;
	if (ui.cbBorderStyle->currentIndex() != pen.style())
		ui.cbBorderStyle->setCurrentIndex(pen.style());
	if (ui.kcbBorderColor->color() != pen.color())
		ui.kcbBorderColor->setColor(pen.color());
	if (ui.sbBorderWidth->value() != pen.widthF())
		ui.sbBorderWidth->setValue(Worksheet::convertFromSceneUnits(pen.widthF(), Worksheet::Unit::Point));
	m_initializing = false;
}

void CartesianPlotDock::plotBorderCornerRadiusChanged(float value) {
	m_initializing = true;
	ui.sbBorderCornerRadius->setValue(Worksheet::convertFromSceneUnits(value, m_worksheetUnit));
	m_initializing = false;
}

void CartesianPlotDock::plotBorderOpacityChanged(float value) {
	m_initializing = true;
	float v = (float)value*100.;
	ui.sbBorderOpacity->setValue(v);
	m_initializing = false;
}

void CartesianPlotDock::plotHorizontalPaddingChanged(float value) {
	m_initializing = true;
	ui.sbPaddingHorizontal->setValue(Worksheet::convertFromSceneUnits(value, m_worksheetUnit));
	m_initializing = false;
}

void CartesianPlotDock::plotVerticalPaddingChanged(float value) {
	m_initializing = true;
	ui.sbPaddingVertical->setValue(Worksheet::convertFromSceneUnits(value, m_worksheetUnit));
	m_initializing = false;
}

void CartesianPlotDock::plotRightPaddingChanged(double value) {
	m_initializing = true;
	ui.sbPaddingRight->setValue(Worksheet::convertFromSceneUnits(value, m_worksheetUnit));
	m_initializing = false;
}

void CartesianPlotDock::plotBottomPaddingChanged(double value) {
	m_initializing = true;
	ui.sbPaddingBottom->setValue(Worksheet::convertFromSceneUnits(value, m_worksheetUnit));
	m_initializing = false;
}

void CartesianPlotDock::plotSymmetricPaddingChanged(bool symmetric) {
	m_initializing = true;
	ui.cbPaddingSymmetric->setChecked(symmetric);
	m_initializing = false;
}

void CartesianPlotDock::plotCursorPenChanged(const QPen& pen) {
	m_initializing = true;
	ui.sbCursorLineWidth->setValue(Worksheet::convertFromSceneUnits(pen.widthF(), Worksheet::Unit::Point));
	ui.kcbCursorLineColor->setColor(pen.color());
	ui.cbCursorLineStyle->setCurrentIndex(pen.style());
	m_initializing = false;
}

//*************************************************************
//******************** SETTINGS *******************************
//*************************************************************
void CartesianPlotDock::loadConfigFromTemplate(KConfig& config) {
	//extract the name of the template from the file name
	QString name;
	int index = config.name().lastIndexOf(QLatin1String("/"));
	if (index != -1)
		name = config.name().right(config.name().size() - index - 1);
	else
		name = config.name();

	int size = m_plotList.size();
	if (size > 1)
		m_plot->beginMacro(i18n("%1 cartesian plots: template \"%2\" loaded", size, name));
	else
		m_plot->beginMacro(i18n("%1: template \"%2\" loaded", m_plot->name(), name));

	this->loadConfig(config);

	m_plot->endMacro();
}

void CartesianPlotDock::load() {
	//General-tab
	ui.chkVisible->setChecked(m_plot->isVisible());
	ui.sbLeft->setValue(Worksheet::convertFromSceneUnits(m_plot->rect().x(), m_worksheetUnit));
	ui.sbTop->setValue(Worksheet::convertFromSceneUnits(m_plot->rect().y(), m_worksheetUnit));
	ui.sbWidth->setValue(Worksheet::convertFromSceneUnits(m_plot->rect().width(), m_worksheetUnit));
	ui.sbHeight->setValue(Worksheet::convertFromSceneUnits(m_plot->rect().height(), m_worksheetUnit));

	switch (m_plot->rangeType()) {
	case CartesianPlot::RangeType::Free:
		ui.rbRangeFree->setChecked(true);
		break;
	case CartesianPlot::RangeType::First:
		ui.rbRangeFirst->setChecked(true);
		break;
	case CartesianPlot::RangeType::Last:
		ui.rbRangeLast->setChecked(true);
		break;
	}
	rangeTypeChanged();
	SET_NUMBER_LOCALE
	ui.leRangeFirst->setText( numberLocale.toString(m_plot->rangeFirstValues()) );
	ui.leRangeLast->setText( numberLocale.toString(m_plot->rangeLastValues()) );

	// x ranges
	for (int row{0}; row < ui.twXRanges->rowCount(); row++) {
		const auto xRange{ m_plot->xRange(row) };
		DEBUG(Q_FUNC_INFO << ", x range " << row << " auto scale = " << xRange.autoScale())

		auto* chk = qobject_cast<QCheckBox*>(ui.twXRanges->cellWidget(row, 0));
		chk->setChecked(xRange.autoScale());

		auto* cb = qobject_cast<QComboBox*>(ui.twXRanges->cellWidget(row, 1));
		cb->setCurrentIndex(static_cast<int>(xRange.format()));

		if (xRange.format() == RangeT::Format::Numeric) {
			auto* le = qobject_cast<QLineEdit*>(ui.twXRanges->cellWidget(row, 2));
			if (le) {	// may be nullptr
				le->setText( numberLocale.toString(xRange.start()) );
				le = qobject_cast<QLineEdit*>(ui.twXRanges->cellWidget(row, 3));
				le->setText( numberLocale.toString(xRange.end()) );
			}
		} else {
			auto* dte = qobject_cast<QDateTimeEdit*>(ui.twXRanges->cellWidget(row, 2));
			if (dte) {	// may be nullptr
				dte->setDisplayFormat( m_plot->xRangeDateTimeFormat(row) );
				dte->setDateTime(QDateTime::fromMSecsSinceEpoch( static_cast<qint64>(xRange.start())) );
				dte = qobject_cast<QDateTimeEdit*>(ui.twXRanges->cellWidget(row, 3));
				dte->setDisplayFormat( m_plot->xRangeDateTimeFormat(row) );
				dte->setDateTime(QDateTime::fromMSecsSinceEpoch( static_cast<qint64>(xRange.end())) );
			}
		}

		cb = qobject_cast<QComboBox*>(ui.twXRanges->cellWidget(row, 4));
		cb->setCurrentIndex(static_cast<int>(xRange.scale()));
	}

	// y ranges
	for (int row{0}; row < ui.twYRanges->rowCount(); row++) {
		const auto yRange{ m_plot->yRange(row) };
		DEBUG(Q_FUNC_INFO << ", y range " << row << " auto scale = " << yRange.autoScale())

		auto* chk = qobject_cast<QCheckBox*>(ui.twYRanges->cellWidget(row, 0));
		chk->setChecked(yRange.autoScale());

		auto* cb = qobject_cast<QComboBox*>(ui.twYRanges->cellWidget(row, 1));
		cb->setCurrentIndex(static_cast<int>(yRange.format()));

		if (yRange.format() == RangeT::Format::Numeric) {
			auto* le = qobject_cast<QLineEdit*>(ui.twYRanges->cellWidget(row, 2));
			if (le) {	// may be nullptr
				le->setText( numberLocale.toString(yRange.start()) );
				le = qobject_cast<QLineEdit*>(ui.twYRanges->cellWidget(row, 3));
				le->setText( numberLocale.toString(yRange.end()) );
			}
		} else {
			auto* dte = qobject_cast<QDateTimeEdit*>(ui.twYRanges->cellWidget(row, 2));
			if (dte) {	// may be nullptr
				dte->setDisplayFormat( m_plot->yRangeDateTimeFormat(row) );
				dte->setDateTime(QDateTime::fromMSecsSinceEpoch( static_cast<qint64>(yRange.start())) );
				dte = qobject_cast<QDateTimeEdit*>(ui.twYRanges->cellWidget(row, 3));
				dte->setDisplayFormat( m_plot->yRangeDateTimeFormat(row) );
				dte->setDateTime(QDateTime::fromMSecsSinceEpoch( static_cast<qint64>(yRange.end())) );
			}
		}

		cb = qobject_cast<QComboBox*>(ui.twYRanges->cellWidget(row, 4));
		cb->setCurrentIndex(static_cast<int>(yRange.scale()));
	}


	//Title
	labelWidget->load();

	//x-range breaks, show the first break
	ui.chkXBreak->setChecked(m_plot->xRangeBreakingEnabled());
	this->toggleXBreak(m_plot->xRangeBreakingEnabled());
	ui.bRemoveXBreak->setVisible(m_plot->xRangeBreaks().list.size() > 1);
	ui.cbXBreak->clear();
	if (!m_plot->xRangeBreaks().list.isEmpty()) {
		for (int i = 1; i <= m_plot->xRangeBreaks().list.size(); ++i)
			ui.cbXBreak->addItem(QString::number(i));
	} else
		ui.cbXBreak->addItem("1");
	ui.cbXBreak->setCurrentIndex(0);

	//y-range breaks, show the first break
	ui.chkYBreak->setChecked(m_plot->yRangeBreakingEnabled());
	this->toggleYBreak(m_plot->yRangeBreakingEnabled());
	ui.bRemoveYBreak->setVisible(m_plot->yRangeBreaks().list.size() > 1);
	ui.cbYBreak->clear();
	if (!m_plot->yRangeBreaks().list.isEmpty()) {
		for (int i = 1; i <= m_plot->yRangeBreaks().list.size(); ++i)
			ui.cbYBreak->addItem(QString::number(i));
	} else
		ui.cbYBreak->addItem("1");
	ui.cbYBreak->setCurrentIndex(0);

	//"Plot Area"-tab
	//Background
	ui.cbBackgroundType->setCurrentIndex( (int)m_plot->plotArea()->backgroundType() );
	backgroundTypeChanged(ui.cbBackgroundType->currentIndex());
	ui.cbBackgroundColorStyle->setCurrentIndex( (int) m_plot->plotArea()->backgroundColorStyle() );
	ui.cbBackgroundImageStyle->setCurrentIndex( (int) m_plot->plotArea()->backgroundImageStyle() );
	ui.cbBackgroundBrushStyle->setCurrentIndex( (int) m_plot->plotArea()->backgroundBrushStyle() );
	ui.leBackgroundFileName->setText( m_plot->plotArea()->backgroundFileName() );
	ui.kcbBackgroundFirstColor->setColor( m_plot->plotArea()->backgroundFirstColor() );
	ui.kcbBackgroundSecondColor->setColor( m_plot->plotArea()->backgroundSecondColor() );
	ui.sbBackgroundOpacity->setValue( round(m_plot->plotArea()->backgroundOpacity()*100.0) );

	//highlight the text field for the background image red if an image is used and cannot be found
	const QString& fileName = m_plot->plotArea()->backgroundFileName();
	bool invalid = (!fileName.isEmpty() && !QFile::exists(fileName));
	GuiTools::highlight(ui.leBackgroundFileName, invalid);

	//Padding
	ui.sbPaddingHorizontal->setValue( Worksheet::convertFromSceneUnits(m_plot->horizontalPadding(), m_worksheetUnit) );
	ui.sbPaddingVertical->setValue( Worksheet::convertFromSceneUnits(m_plot->verticalPadding(), m_worksheetUnit) );
	ui.sbPaddingRight->setValue(Worksheet::convertFromSceneUnits(m_plot->rightPadding(), m_worksheetUnit));
	ui.sbPaddingBottom->setValue(Worksheet::convertFromSceneUnits(m_plot->bottomPadding(), m_worksheetUnit));
	ui.cbPaddingSymmetric->setChecked(m_plot->symmetricPadding());

	//Border
	ui.kcbBorderColor->setColor( m_plot->plotArea()->borderPen().color() );
	ui.cbBorderStyle->setCurrentIndex( (int) m_plot->plotArea()->borderPen().style() );
	ui.sbBorderWidth->setValue( Worksheet::convertFromSceneUnits(m_plot->plotArea()->borderPen().widthF(), Worksheet::Unit::Point) );
	ui.sbBorderCornerRadius->setValue( Worksheet::convertFromSceneUnits(m_plot->plotArea()->borderCornerRadius(), m_worksheetUnit) );
	ui.sbBorderOpacity->setValue( round(m_plot->plotArea()->borderOpacity()*100) );
	GuiTools::updatePenStyles(ui.cbBorderStyle, ui.kcbBorderColor->color());

	// Cursor
	QPen pen = m_plot->cursorPen();
	ui.cbCursorLineStyle->setCurrentIndex(pen.style());
	ui.kcbCursorLineColor->setColor(pen.color());
	ui.sbCursorLineWidth->setValue(pen.width());
	GuiTools::updatePenStyles(ui.cbCursorLineStyle, pen.color());
}

void CartesianPlotDock::loadConfig(KConfig& config) {
	KConfigGroup group = config.group("CartesianPlot");

	//General
	//we don't load/save the settings in the general-tab, since they are not style related.
	//It doesn't make sense to load/save them in the template.
	//This data is read in CartesianPlotDock::setPlots().

	//Title
	KConfigGroup plotTitleGroup = config.group("CartesianPlotTitle");
	labelWidget->loadConfig(plotTitleGroup);

	//Scale breakings
	//TODO

	//Background-tab
	ui.cbBackgroundType->setCurrentIndex( group.readEntry("BackgroundType", (int) m_plot->plotArea()->backgroundType()) );
	ui.cbBackgroundColorStyle->setCurrentIndex( group.readEntry("BackgroundColorStyle", (int) m_plot->plotArea()->backgroundColorStyle()) );
	ui.cbBackgroundImageStyle->setCurrentIndex( group.readEntry("BackgroundImageStyle", (int) m_plot->plotArea()->backgroundImageStyle()) );
	ui.cbBackgroundBrushStyle->setCurrentIndex( group.readEntry("BackgroundBrushStyle", (int) m_plot->plotArea()->backgroundBrushStyle()) );
	ui.leBackgroundFileName->setText( group.readEntry("BackgroundFileName", m_plot->plotArea()->backgroundFileName()) );
	ui.kcbBackgroundFirstColor->setColor( group.readEntry("BackgroundFirstColor", m_plot->plotArea()->backgroundFirstColor()) );
	ui.kcbBackgroundSecondColor->setColor( group.readEntry("BackgroundSecondColor", m_plot->plotArea()->backgroundSecondColor()) );
	ui.sbBackgroundOpacity->setValue( round(group.readEntry("BackgroundOpacity", m_plot->plotArea()->backgroundOpacity())*100.0) );
	ui.sbPaddingHorizontal->setValue(Worksheet::convertFromSceneUnits(group.readEntry("HorizontalPadding", m_plot->horizontalPadding()), m_worksheetUnit));
	ui.sbPaddingVertical->setValue(Worksheet::convertFromSceneUnits(group.readEntry("VerticalPadding", m_plot->verticalPadding()), m_worksheetUnit));
	ui.sbPaddingRight->setValue(Worksheet::convertFromSceneUnits(group.readEntry("RightPadding", m_plot->rightPadding()), m_worksheetUnit));
	ui.sbPaddingBottom->setValue(Worksheet::convertFromSceneUnits(group.readEntry("BottomPadding", m_plot->bottomPadding()), m_worksheetUnit));
	ui.cbPaddingSymmetric->setChecked(group.readEntry("SymmetricPadding", m_plot->symmetricPadding()));

	//Border-tab
	ui.kcbBorderColor->setColor( group.readEntry("BorderColor", m_plot->plotArea()->borderPen().color()) );
	ui.cbBorderStyle->setCurrentIndex( group.readEntry("BorderStyle", (int) m_plot->plotArea()->borderPen().style()) );
	ui.sbBorderWidth->setValue( Worksheet::convertFromSceneUnits(group.readEntry("BorderWidth", m_plot->plotArea()->borderPen().widthF()), Worksheet::Unit::Point) );
	ui.sbBorderCornerRadius->setValue( Worksheet::convertFromSceneUnits(group.readEntry("BorderCornerRadius", m_plot->plotArea()->borderCornerRadius()), m_worksheetUnit) );
	ui.sbBorderOpacity->setValue( group.readEntry("BorderOpacity", m_plot->plotArea()->borderOpacity())*100 );

	m_initializing = true;
	GuiTools::updatePenStyles(ui.cbBorderStyle, ui.kcbBorderColor->color());
	GuiTools::updatePenStyles(ui.cbCursorLineStyle, m_plot->cursorPen().color());
	m_initializing = false;
}

void CartesianPlotDock::saveConfigAsTemplate(KConfig& config) {
	KConfigGroup group = config.group("CartesianPlot");

	//General
	//we don't load/save the settings in the general-tab, since they are not style related.
	//It doesn't make sense to load/save them in the template.

	//Title
	KConfigGroup plotTitleGroup = config.group("CartesianPlotTitle");
	labelWidget->saveConfig(plotTitleGroup);

	//Scale breakings
	//TODO

	//Background
	group.writeEntry("BackgroundType", ui.cbBackgroundType->currentIndex());
	group.writeEntry("BackgroundColorStyle", ui.cbBackgroundColorStyle->currentIndex());
	group.writeEntry("BackgroundImageStyle", ui.cbBackgroundImageStyle->currentIndex());
	group.writeEntry("BackgroundBrushStyle", ui.cbBackgroundBrushStyle->currentIndex());
	group.writeEntry("BackgroundFileName", ui.leBackgroundFileName->text());
	group.writeEntry("BackgroundFirstColor", ui.kcbBackgroundFirstColor->color());
	group.writeEntry("BackgroundSecondColor", ui.kcbBackgroundSecondColor->color());
	group.writeEntry("BackgroundOpacity", ui.sbBackgroundOpacity->value()/100.0);
	group.writeEntry("HorizontalPadding", Worksheet::convertToSceneUnits(ui.sbPaddingHorizontal->value(), m_worksheetUnit));
	group.writeEntry("VerticalPadding", Worksheet::convertToSceneUnits(ui.sbPaddingVertical->value(), m_worksheetUnit));
	group.writeEntry("RightPadding", Worksheet::convertToSceneUnits(ui.sbPaddingRight->value(), m_worksheetUnit));
	group.writeEntry("BottomPadding", Worksheet::convertToSceneUnits(ui.sbPaddingBottom->value(), m_worksheetUnit));
	group.writeEntry("SymmetricPadding", ui.cbPaddingSymmetric->isChecked());

	//Border
	group.writeEntry("BorderStyle", ui.cbBorderStyle->currentIndex());
	group.writeEntry("BorderColor", ui.kcbBorderColor->color());
	group.writeEntry("BorderWidth", Worksheet::convertToSceneUnits(ui.sbBorderWidth->value(), Worksheet::Unit::Point));
	group.writeEntry("BorderCornerRadius", Worksheet::convertToSceneUnits(ui.sbBorderCornerRadius->value(), m_worksheetUnit));
	group.writeEntry("BorderOpacity", ui.sbBorderOpacity->value()/100.0);

	config.sync();
}

void CartesianPlotDock::loadTheme(const QString& theme) {
	for (auto* plot : m_plotList)
		plot->setTheme(theme);
}

void CartesianPlotDock::saveTheme(KConfig& config) const {
	if (!m_plotList.isEmpty())
		m_plotList.at(0)->saveTheme(config);
}
