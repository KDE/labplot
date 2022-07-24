/*
	File                 : CartesianPlotDock.cpp
	Project              : LabPlot
	Description          : widget for cartesian plot properties
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2011-2022 Alexander Semke <alexander.semke@web.de>
	SPDX-FileCopyrightText: 2012-2021 Stefan Gerlach <stefan.gerlach@uni-konstanz.de>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "CartesianPlotDock.h"
#include "backend/core/Project.h"
#include "backend/core/column/Column.h"
#include "backend/worksheet/plots/cartesian/Axis.h"
#include "kdefrontend/widgets/BackgroundWidget.h"

#include "kdefrontend/GuiTools.h"
#include "kdefrontend/PlotTemplateDialog.h"
#include "kdefrontend/TemplateHandler.h"
#include "kdefrontend/ThemeHandler.h"
#include "kdefrontend/widgets/LabelWidget.h"

#include <KIconLoader>
#include <KMessageBox>

#include <QButtonGroup>
#include <QDebug>
#include <QFileDialog>
#include <QIntValidator>
#include <QPainter>
#include <QRadioButton>
#include <QWheelEvent>
#include <QXmlStreamWriter>

namespace {
enum TwRangesColumn { Automatic = 0, Format, Min, Max, Scale };

enum TwPlotRangesColumn { XRange, YRange, Default };

// https://stackoverflow.com/questions/5821802/qspinbox-inside-a-qscrollarea-how-to-prevent-spin-box-from-stealing-focus-when
class ComboBoxIgnoreWheel : public QComboBox {
public:
	explicit ComboBoxIgnoreWheel(QWidget* parent = nullptr)
		: QComboBox(parent) {
		setFocusPolicy(Qt::StrongFocus);
	}

protected:
	void wheelEvent(QWheelEvent* event) override {
		if (!hasFocus())
			event->ignore();
		else
			QComboBox::wheelEvent(event);
	}
};
}

using Direction = CartesianCoordinateSystem::Dimension;

#define CELLWIDGET(dir, rangeIndex, Column, castObject, function)                                                                                              \
	do {                                                                                                                                                       \
		QTableWidget* treewidget;                                                                                                                              \
		switch (dir) {                                                                                                                                         \
		case Direction::X:                                                                                                                                     \
			treewidget = ui.twXRanges;                                                                                                                         \
			break;                                                                                                                                             \
		case Direction::Y:                                                                                                                                     \
			treewidget = ui.twYRanges;                                                                                                                         \
			break;                                                                                                                                             \                                     \
		}                                                                                                                                                      \
		if (rangeIndex < 0) {                                                                                                                                  \
			for (int i = 0; i < treewidget->rowCount(); i++) {                                                                                                 \
				auto obj = qobject_cast<castObject*>(treewidget->cellWidget(i, Column));                                                                       \
				if (obj)                                                                                                                                       \
					obj->function;                                                                                                                             \
				else                                                                                                                                           \
					qDebug() << "ERROR: qobject_cast <castObject*> failed: " << __FILE__ << ":" << __LINE__ << " ( rangeIndex:" << rangeIndex                  \
							 << ", Column: " << Column << "). Wether the object does not exist or the cellWidget has differnt type";                           \
			}                                                                                                                                                  \
		} else {                                                                                                                                               \
			auto obj = qobject_cast<castObject*>(treewidget->cellWidget(rangeIndex, Column));                                                                  \
			if (obj)                                                                                                                                           \
				obj->function;                                                                                                                                 \
			else                                                                                                                                               \
				qDebug() << "ERROR: qobject_cast <castObject*> failed: " << __FILE__ << ":" << __LINE__ << " (rangeIndex:" << rangeIndex                       \
						 << ", Column: " << Column << "). Wether the object does not exist or the cellWidget has differnt type";                               \
		}                                                                                                                                                      \
	} while (0);

/*!
  \class CartesianPlotDock
  \brief  Provides a widget for editing the properties of the cartesian plot currently selected in the project explorer.

  \ingroup kdefrontend
*/

CartesianPlotDock::CartesianPlotDock(QWidget* parent)
	: BaseDock(parent) {
	ui.setupUi(this);
	m_leName = ui.leName;
	m_teComment = ui.teComment;
	m_teComment->setFixedHeight(2 * m_leName->height());

	//"General"-tab
	ui.twXRanges->setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);
	ui.twYRanges->setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);
	ui.twPlotRanges->setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);

	//"Range breaks"-tab
	ui.bAddXBreak->setIcon(QIcon::fromTheme("list-add"));
	ui.bRemoveXBreak->setIcon(QIcon::fromTheme("list-remove"));
	ui.cbXBreak->addItem("1");

	ui.bAddYBreak->setIcon(QIcon::fromTheme("list-add"));
	ui.bRemoveYBreak->setIcon(QIcon::fromTheme("list-remove"));
	ui.cbYBreak->addItem("1");

	//"Background"-tab
	auto* gridLayout = static_cast<QGridLayout*>(ui.tabPlotArea->layout());
	backgroundWidget = new BackgroundWidget(ui.tabPlotArea);
	gridLayout->addWidget(backgroundWidget, 1, 0, 1, 3);

	//"Title"-tab
	auto* hboxLayout = new QHBoxLayout(ui.tabTitle);
	labelWidget = new LabelWidget(ui.tabTitle);
	hboxLayout->addWidget(labelWidget);
	hboxLayout->setContentsMargins(0, 0, 0, 0);
	hboxLayout->setSpacing(0);

	// adjust layouts in the tabs
	for (int i = 0; i < ui.tabWidget->count(); ++i) {
		auto* layout = qobject_cast<QGridLayout*>(ui.tabWidget->widget(i)->layout());
		if (!layout)
			continue;

		layout->setContentsMargins(2, 2, 2, 2);
		layout->setHorizontalSpacing(2);
		layout->setVerticalSpacing(2);
	}

	// "Cursor"-tab
	QStringList list = {i18n("NoPen"), i18n("SolidLine"), i18n("DashLine"), i18n("DotLine"), i18n("DashDotLine"), i18n("DashDotDotLine")};
	ui.cbCursorLineStyle->clear();
	for (int i = 0; i < list.count(); i++)
		ui.cbCursorLineStyle->addItem(list[i], i);

	// Validators
	ui.leRangePoints->setValidator(new QIntValidator(ui.leRangePoints));
	ui.leXBreakStart->setValidator(new QDoubleValidator(ui.leXBreakStart));
	ui.leXBreakEnd->setValidator(new QDoubleValidator(ui.leXBreakEnd));
	ui.leYBreakStart->setValidator(new QDoubleValidator(ui.leYBreakStart));
	ui.leYBreakEnd->setValidator(new QDoubleValidator(ui.leYBreakEnd));

	// SIGNAL/SLOT
	// General
	connect(ui.leName, &QLineEdit::textChanged, this, &CartesianPlotDock::nameChanged);
	connect(ui.teComment, &QTextEdit::textChanged, this, &CartesianPlotDock::commentChanged);
	connect(ui.chkVisible, &QCheckBox::clicked, this, &CartesianPlotDock::visibilityChanged);
	connect(ui.sbLeft, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &CartesianPlotDock::geometryChanged);
	connect(ui.sbTop, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &CartesianPlotDock::geometryChanged);
	connect(ui.sbWidth, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &CartesianPlotDock::geometryChanged);
	connect(ui.sbHeight, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &CartesianPlotDock::geometryChanged);

	connect(ui.cbRangeType, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &CartesianPlotDock::rangeTypeChanged);
	connect(ui.cbNiceExtend, &QCheckBox::clicked, this, &CartesianPlotDock::niceExtendChanged);
	connect(ui.leRangePoints, &QLineEdit::textChanged, this, &CartesianPlotDock::rangePointsChanged);

	// Range breaks
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

	// Border
	connect(ui.tbBorderTypeLeft, &QToolButton::clicked, this, &CartesianPlotDock::borderTypeChanged);
	connect(ui.tbBorderTypeTop, &QToolButton::clicked, this, &CartesianPlotDock::borderTypeChanged);
	connect(ui.tbBorderTypeRight, &QToolButton::clicked, this, &CartesianPlotDock::borderTypeChanged);
	connect(ui.tbBorderTypeBottom, &QToolButton::clicked, this, &CartesianPlotDock::borderTypeChanged);
	connect(ui.cbBorderStyle, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &CartesianPlotDock::borderStyleChanged);
	connect(ui.kcbBorderColor, &KColorButton::changed, this, &CartesianPlotDock::borderColorChanged);
	connect(ui.sbBorderWidth, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &CartesianPlotDock::borderWidthChanged);
	connect(ui.sbBorderCornerRadius, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &CartesianPlotDock::borderCornerRadiusChanged);
	connect(ui.sbBorderOpacity, QOverload<int>::of(&QSpinBox::valueChanged), this, &CartesianPlotDock::borderOpacityChanged);

	// Padding
	connect(ui.sbPaddingHorizontal, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &CartesianPlotDock::horizontalPaddingChanged);
	connect(ui.sbPaddingVertical, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &CartesianPlotDock::verticalPaddingChanged);
	connect(ui.sbPaddingRight, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &CartesianPlotDock::rightPaddingChanged);
	connect(ui.sbPaddingBottom, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &CartesianPlotDock::bottomPaddingChanged);
	connect(ui.cbPaddingSymmetric, &QCheckBox::toggled, this, &CartesianPlotDock::symmetricPaddingChanged);

	// Cursor
	connect(ui.sbCursorLineWidth, QOverload<int>::of(&QSpinBox::valueChanged), this, &CartesianPlotDock::cursorLineWidthChanged);
	connect(ui.kcbCursorLineColor, &KColorButton::changed, this, &CartesianPlotDock::cursorLineColorChanged);
	connect(ui.cbCursorLineStyle, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &CartesianPlotDock::cursorLineStyleChanged);

	// theme and template handlers
	auto* frame = new QFrame(this);
	auto* layout = new QHBoxLayout(frame);
	layout->setContentsMargins(0, 11, 0, 11);

	// themes
	m_themeHandler = new ThemeHandler(this);
	layout->addWidget(m_themeHandler);
	connect(m_themeHandler, &ThemeHandler::loadThemeRequested, this, &CartesianPlotDock::loadTheme);
	connect(m_themeHandler, &ThemeHandler::info, this, &CartesianPlotDock::info);

	// templates for plot properties
	auto* templateHandler = new TemplateHandler(this, TemplateHandler::ClassName::CartesianPlot);
	layout->addWidget(templateHandler);
	connect(templateHandler, &TemplateHandler::loadConfigRequested, this, &CartesianPlotDock::loadConfigFromTemplate);
	connect(templateHandler, &TemplateHandler::saveConfigRequested, this, &CartesianPlotDock::saveConfigAsTemplate);
	connect(templateHandler, &TemplateHandler::info, this, &CartesianPlotDock::info);

	// templates for plot definitions
	auto* tbExportTemplate = new QToolButton;
	int size = KIconLoader::global()->currentSize(KIconLoader::MainToolbar);
	tbExportTemplate->setIconSize(QSize(size, size));
	tbExportTemplate->setIcon(QIcon::fromTheme(QLatin1String("document-save-as-template")));
	tbExportTemplate->setToolTip(i18n("Save current plot definition as template"));
	connect(tbExportTemplate, &QToolButton::pressed, this, &CartesianPlotDock::exportPlotTemplate);
	layout->addWidget(tbExportTemplate);

	ui.verticalLayout->addWidget(frame);

	// TODO: activate the tab again once the functionality is implemented
	ui.tabWidget->removeTab(2);

	init();
}

void CartesianPlotDock::init() {
	this->retranslateUi();

	GuiTools::updatePenStyles(ui.cbCursorLineStyle, Qt::black);

	// draw the icons for the border sides
	QPainter pa;
	pa.setRenderHint(QPainter::Antialiasing);
	int iconSize = 20;
	QPixmap pm(iconSize, iconSize);

	QPen pen(Qt::SolidPattern);
	const QColor& color = (palette().color(QPalette::Base).lightness() < 128) ? Qt::white : Qt::black;
	pen.setColor(color);

	// left
	pm.fill(Qt::transparent);
	pa.begin(&pm);
	pen.setStyle(Qt::SolidLine);
	pen.setWidthF(1.0);
	pa.setPen(pen);
	pa.drawLine(1, 1, 1, 19);
	pen.setStyle(Qt::DotLine);
	pen.setWidthF(0.0);
	pa.setPen(pen);
	pa.drawLine(1, 19, 19, 19);
	pa.drawLine(19, 19, 19, 1);
	pa.drawLine(19, 1, 1, 1);
	ui.tbBorderTypeLeft->setIcon(pm);

	// top
	pm.fill(Qt::transparent);
	pa.begin(&pm);
	pen.setStyle(Qt::SolidLine);
	pen.setWidthF(1.0);
	pa.setPen(pen);
	pa.drawLine(19, 1, 1, 1);
	pen.setStyle(Qt::DotLine);
	pen.setWidthF(0.0);
	pa.setPen(pen);
	pa.drawLine(1, 19, 19, 19);
	pa.drawLine(1, 1, 1, 19);
	pa.drawLine(19, 19, 19, 1);
	pa.end();
	ui.tbBorderTypeTop->setIcon(pm);

	// right
	pm.fill(Qt::transparent);
	pa.begin(&pm);
	pen.setStyle(Qt::SolidLine);
	pen.setWidthF(1.0);
	pa.setPen(pen);
	pa.drawLine(19, 19, 19, 1);
	pen.setStyle(Qt::DotLine);
	pen.setWidthF(0.0);
	pa.setPen(pen);
	pa.drawLine(1, 1, 1, 19);
	pa.drawLine(1, 19, 19, 19);
	pa.drawLine(19, 1, 1, 1);
	pa.end();
	ui.tbBorderTypeRight->setIcon(pm);

	// bottom
	pm.fill(Qt::transparent);
	pa.begin(&pm);
	pen.setStyle(Qt::SolidLine);
	pen.setWidthF(1.0);
	pa.setPen(pen);
	pa.drawLine(1, 19, 19, 19);
	pen.setStyle(Qt::DotLine);
	pen.setWidthF(0.0);
	pa.setPen(pen);
	pa.drawLine(1, 1, 1, 19);
	pa.drawLine(19, 19, 19, 1);
	pa.drawLine(19, 1, 1, 1);
	pa.end();
	ui.tbBorderTypeBottom->setIcon(pm);

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

	// if there is more then one plot in the list, disable the name and comment fields in the tab "general"
	if (list.size() == 1) {
		ui.lName->setEnabled(true);
		ui.leName->setEnabled(true);
		ui.lComment->setEnabled(true);
		ui.teComment->setEnabled(true);

		ui.leName->setText(m_plot->name());
		ui.teComment->setText(m_plot->comment());
	} else {
		ui.lName->setEnabled(false);
		ui.leName->setEnabled(false);
		ui.lComment->setEnabled(false);
		ui.teComment->setEnabled(false);

		ui.leName->setText(QString());
		ui.teComment->setText(QString());
	}

	symmetricPaddingChanged(m_plot->symmetricPadding());

	ui.leName->setStyleSheet("");
	ui.leName->setToolTip("");

	// show the properties of the first plot
	this->load();

	// set the current locale
	updateLocale();
	updatePlotRangeList();

	// update active widgets
	m_themeHandler->setCurrentTheme(m_plot->theme());

	// Deactivate the geometry related widgets, if the worksheet layout is active.
	// Currently, a plot can only be a child of the worksheet itself, so we only need to ask the parent aspect (=worksheet).
	// TODO redesign this, if the hierarchy will be changend in future (a plot is a child of a new object group/container or so)
	auto* w = dynamic_cast<Worksheet*>(m_plot->parentAspect());
	if (w) {
		bool b = (w->layout() == Worksheet::Layout::NoLayout);
		ui.sbTop->setEnabled(b);
		ui.sbLeft->setEnabled(b);
		ui.sbWidth->setEnabled(b);
		ui.sbHeight->setEnabled(b);
		connect(w, &Worksheet::layoutChanged, this, &CartesianPlotDock::layoutChanged);
	}

	// SIGNALs/SLOTs
	connect(m_plot, &CartesianPlot::aspectDescriptionChanged, this, &CartesianPlotDock::aspectDescriptionChanged);
	connect(m_plot, &CartesianPlot::rectChanged, this, &CartesianPlotDock::plotRectChanged);
	connect(m_plot, &CartesianPlot::rangeTypeChanged, this, &CartesianPlotDock::plotRangeTypeChanged);
	connect(m_plot, &CartesianPlot::rangeFirstValuesChanged, this, &CartesianPlotDock::plotRangeFirstValuesChanged);
	connect(m_plot, &CartesianPlot::rangeLastValuesChanged, this, &CartesianPlotDock::plotRangeLastValuesChanged);
	// TODO: check if needed
	connect(m_plot, &CartesianPlot::autoScaleChanged, this, &CartesianPlotDock::plotAutoScaleChanged);
	connect(m_plot, &CartesianPlot::xMinChanged, this, &CartesianPlotDock::plotXMinChanged);
	connect(m_plot, &CartesianPlot::xMaxChanged, this, &CartesianPlotDock::plotXMaxChanged);
	connect(m_plot, &CartesianPlot::rangeChanged, this, &CartesianPlotDock::plotRangeChanged);
	connect(m_plot, &CartesianPlot::scaleChanged, this, &CartesianPlotDock::plotScaleChanged);
	connect(m_plot, &CartesianPlot::rangeFormatChanged, this, &CartesianPlotDock::plotRangeFormatChanged);
	connect(m_plot, &CartesianPlot::yMinChanged, this, &CartesianPlotDock::plotYMinChanged);
	connect(m_plot, &CartesianPlot::yMaxChanged, this, &CartesianPlotDock::plotYMaxChanged);

	connect(m_plot, &CartesianPlot::visibleChanged, this, &CartesianPlotDock::plotVisibleChanged);

	// range breaks
	connect(m_plot, &CartesianPlot::xRangeBreakingEnabledChanged, this, &CartesianPlotDock::plotXRangeBreakingEnabledChanged);
	connect(m_plot, &CartesianPlot::xRangeBreaksChanged, this, &CartesianPlotDock::plotXRangeBreaksChanged);
	connect(m_plot, &CartesianPlot::yRangeBreakingEnabledChanged, this, &CartesianPlotDock::plotYRangeBreakingEnabledChanged);
	connect(m_plot, &CartesianPlot::yRangeBreaksChanged, this, &CartesianPlotDock::plotYRangeBreaksChanged);

	// Plot Area
	connect(m_plot, &CartesianPlot::horizontalPaddingChanged, this, &CartesianPlotDock::plotHorizontalPaddingChanged);
	connect(m_plot, &CartesianPlot::verticalPaddingChanged, this, &CartesianPlotDock::plotVerticalPaddingChanged);
	connect(m_plot, &CartesianPlot::rightPaddingChanged, this, &CartesianPlotDock::plotRightPaddingChanged);
	connect(m_plot, &CartesianPlot::bottomPaddingChanged, this, &CartesianPlotDock::plotBottomPaddingChanged);
	connect(m_plot, &CartesianPlot::symmetricPaddingChanged, this, &CartesianPlotDock::plotSymmetricPaddingChanged);

	connect(m_plot, &CartesianPlot::themeChanged, m_themeHandler, &ThemeHandler::setCurrentTheme);

	m_initializing = false;
}

void CartesianPlotDock::activateTitleTab() {
	ui.tabWidget->setCurrentWidget(ui.tabTitle);
}

/*
 * updates the locale in the widgets. called when the application settings are changed.
 */
void CartesianPlotDock::updateLocale() {
	DEBUG(Q_FUNC_INFO)
	SET_NUMBER_LOCALE

	// update the QSpinBoxes
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

	// update the QLineEdits, avoid the change events
	if (m_plot) {
		if (m_plot->rangeType() == CartesianPlot::RangeType::First)
			ui.leRangePoints->setText(numberLocale.toString(m_plot->rangeFirstValues()));
		else if (m_plot->rangeType() == CartesianPlot::RangeType::Last)
			ui.leRangePoints->setText(numberLocale.toString(m_plot->rangeLastValues()));

		// x ranges
		bool isDateTime = false;
		for (int row = 0; row < qMin(ui.twXRanges->rowCount(), m_plot->rangeCount(Direction::X)); row++) {
			const auto xRange{m_plot->range(Direction::X, row)};
			DEBUG(Q_FUNC_INFO << ", x range " << row << " auto scale = " << xRange.autoScale())
			if (m_plot->xRangeFormat(row) == RangeT::Format::Numeric) {
				const int relPrec = xRange.relativePrecision();
				auto* le = qobject_cast<QLineEdit*>(ui.twXRanges->cellWidget(row, TwRangesColumn::Min));
				// save cursor position
				int pos = le->cursorPosition();
				CELLWIDGET(Direction::X, row, TwRangesColumn::Min, QLineEdit, setText(numberLocale.toString(xRange.start(), 'g', relPrec)));
				le->setCursorPosition(pos);
				le = qobject_cast<QLineEdit*>(ui.twXRanges->cellWidget(row, TwRangesColumn::Max));
				pos = le->cursorPosition();
				CELLWIDGET(Direction::X, row, TwRangesColumn::Max, QLineEdit, setText(numberLocale.toString(xRange.end(), 'g', relPrec)));
				le->setCursorPosition(pos);

			} else {
				CELLWIDGET(Direction::X, row, TwRangesColumn::Min, QDateTimeEdit, setDateTime(QDateTime::fromMSecsSinceEpoch(xRange.start(), Qt::UTC)));
				CELLWIDGET(Direction::X, row, TwRangesColumn::Max, QDateTimeEdit, setDateTime(QDateTime::fromMSecsSinceEpoch(xRange.end(), Qt::UTC)));
				auto* dte = qobject_cast<QDateTimeEdit*>(ui.twXRanges->cellWidget(row, TwRangesColumn::Min));
				if (dte)
					isDateTime = true;
			}
		}

		// TODO
		if (isDateTime) {
			ui.twXRanges->resizeColumnToContents(2);
			ui.twXRanges->resizeColumnToContents(3);
		}

		// y ranges
		isDateTime = false;
		for (int row = 0; row < qMin(ui.twYRanges->rowCount(), m_plot->rangeCount(Direction::Y)); row++) {
			const auto yRange{m_plot->range(Direction::Y, row)};
			DEBUG(Q_FUNC_INFO << ", y range " << row << " auto scale = " << yRange.autoScale())
			if (m_plot->yRangeFormat(row) == RangeT::Format::Numeric) {
				const int relPrec = yRange.relativePrecision();
				auto* le = qobject_cast<QLineEdit*>(ui.twYRanges->cellWidget(row, TwRangesColumn::Min));
				// save cursor position
				int pos = le->cursorPosition();
				CELLWIDGET(Direction::Y, row, TwRangesColumn::Min, QLineEdit, setText(numberLocale.toString(yRange.start(), 'g', relPrec)));
				le->setCursorPosition(pos);
				le = qobject_cast<QLineEdit*>(ui.twYRanges->cellWidget(row, TwRangesColumn::Max));
				pos = le->cursorPosition();
				CELLWIDGET(Direction::Y, row, TwRangesColumn::Max, QLineEdit, setText(numberLocale.toString(yRange.end(), 'g', relPrec)));
				le->setCursorPosition(pos);
			} else {
				CELLWIDGET(Direction::Y, row, TwRangesColumn::Min, QDateTimeEdit, setDateTime(QDateTime::fromMSecsSinceEpoch(yRange.start(), Qt::UTC)));
				CELLWIDGET(Direction::Y, row, TwRangesColumn::Max, QDateTimeEdit, setDateTime(QDateTime::fromMSecsSinceEpoch(yRange.end(), Qt::UTC)));
				auto* dte = qobject_cast<QDateTimeEdit*>(ui.twYRanges->cellWidget(row, TwRangesColumn::Min));
				if (dte)
					isDateTime = true;
			}
		}
		if (isDateTime) {
			ui.twYRanges->resizeColumnToContents(2);
			ui.twYRanges->resizeColumnToContents(3);
		}
	}

	// update the title label
	labelWidget->updateLocale();

	// update locale plot range list
	updatePlotRangeList();
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
		// convert from imperial to metric
		m_worksheetUnit = Worksheet::Unit::Centimeter;
		suffix = QLatin1String(" cm");
		ui.sbLeft->setValue(ui.sbLeft->value() * 2.54);
		ui.sbTop->setValue(ui.sbTop->value() * 2.54);
		ui.sbWidth->setValue(ui.sbWidth->value() * 2.54);
		ui.sbHeight->setValue(ui.sbHeight->value() * 2.54);
		ui.sbBorderCornerRadius->setValue(ui.sbBorderCornerRadius->value() * 2.54);
		ui.sbPaddingHorizontal->setValue(ui.sbPaddingHorizontal->value() * 2.54);
		ui.sbPaddingVertical->setValue(ui.sbPaddingVertical->value() * 2.54);
		ui.sbPaddingRight->setValue(ui.sbPaddingRight->value() * 2.54);
		ui.sbPaddingBottom->setValue(ui.sbPaddingBottom->value() * 2.54);
	} else {
		// convert from metric to imperial
		m_worksheetUnit = Worksheet::Unit::Inch;
		suffix = QLatin1String(" in");
		ui.sbLeft->setValue(ui.sbLeft->value() / 2.54);
		ui.sbTop->setValue(ui.sbTop->value() / 2.54);
		ui.sbWidth->setValue(ui.sbWidth->value() / 2.54);
		ui.sbHeight->setValue(ui.sbHeight->value() / 2.54);
		ui.sbBorderCornerRadius->setValue(ui.sbBorderCornerRadius->value() / 2.54);
		ui.sbPaddingHorizontal->setValue(ui.sbPaddingHorizontal->value() / 2.54);
		ui.sbPaddingVertical->setValue(ui.sbPaddingVertical->value() / 2.54);
		ui.sbPaddingRight->setValue(ui.sbPaddingRight->value() / 2.54);
		ui.sbPaddingBottom->setValue(ui.sbPaddingBottom->value() / 2.54);
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

void CartesianPlotDock::updateRangeList(Direction dir) {
	if (!m_plot)
		return;

	auto dir_str = CartesianCoordinateSystem::directionToString(dir);

	QTableWidget* tw = nullptr;
	QLabel* l = nullptr;
	QToolButton* tb = nullptr;
	switch (dir) {
	case Direction::X:
		tw = ui.twXRanges;
		l = ui.lXRanges;
		tb = ui.tbRemoveXRange;
		break;
	case Direction::Y:
		tw = ui.twYRanges;
		l = ui.lYRanges;
		tb = ui.tbRemoveYRange;
		break;
	}

	const int rangeCount = m_plot->rangeCount(dir);
	DEBUG(Q_FUNC_INFO << ", " << dir_str.toStdString() << " range count = " << rangeCount)

	if (rangeCount > 1)
		l->setText(i18n("%1-Ranges:", dir_str.toUpper()));
	else
		l->setText(i18n("%1-Range:", dir_str.toUpper()));
	tw->setRowCount(rangeCount);
	for (int i = 0; i < rangeCount; i++) {
		const auto r = m_plot->range(dir, i);
		const auto format = r.format();
		const auto scale = r.scale();
		DEBUG(Q_FUNC_INFO << ", range " << i << ": format = " << ENUM_TO_STRING(RangeT, Format, format) << ", scale = " << ENUM_TO_STRING(RangeT, Scale, scale)
						  << ", auto scale = " << r.autoScale())

		// auto scale
		auto* chk = new QCheckBox(tw);
		chk->setProperty("row", i);
		chk->setChecked(r.autoScale());
		//		chk->setStyleSheet("margin-left:50%; margin-right:50%;");	// center button
		tw->setCellWidget(i, TwRangesColumn::Automatic, chk);
		connect(chk, &QCheckBox::toggled, [this, chk, dir](bool checked) {
			this->autoScaleChanged(chk, dir, checked);
		});

		// format
		auto* cb = new ComboBoxIgnoreWheel(tw);
		cb->addItem(i18n("Numeric"));
		cb->addItem(AbstractColumn::columnModeString(AbstractColumn::ColumnMode::DateTime));
		cb->setProperty("row", i);
		cb->setCurrentIndex(static_cast<int>(format));
		tw->setCellWidget(i, TwRangesColumn::Format, cb);
		connect(cb, QOverload<int>::of(&QComboBox::currentIndexChanged), [this, dir, cb](int index) {
			this->rangeFormatChanged(cb, dir, index);
		});

		// start/end (values set in updateLocale())
		if (format == RangeT::Format::Numeric) {
			auto* le = new QLineEdit(tw);
			le->setValidator(new QDoubleValidator(le));
			le->setProperty("row", i);
			tw->setCellWidget(i, TwRangesColumn::Min, le);
			connect(le, &QLineEdit::textChanged, [this, dir, le](const QString& text) {
				this->minChanged(le, dir, text);
			});
			DEBUG(Q_FUNC_INFO << ", max length = " << le->maxLength())
			le = new QLineEdit(tw);
			le->setValidator(new QDoubleValidator(le));
			le->setProperty("row", i);
			tw->setCellWidget(i, TwRangesColumn::Max, le);
			connect(le, &QLineEdit::textChanged, [this, dir, le](const QString& text) {
				this->maxChanged(le, dir, text);
			});
		} else {
			auto* dte = new QDateTimeEdit(tw);
			dte->setDisplayFormat(m_plot->rangeDateTimeFormat(dir, i));
			dte->setTimeSpec(Qt::UTC);
			dte->setDateTime(QDateTime::fromMSecsSinceEpoch(r.start(), Qt::UTC));
			dte->setWrapping(true);
			tw->setCellWidget(i, TwRangesColumn::Min, dte);
			dte->setProperty("row", i);
			connect(dte, &QDateTimeEdit::dateTimeChanged, [this, dir, dte](const QDateTime& dateTime) {
				this->minDateTimeChanged(dte, dir, dateTime);
			});
			dte = new QDateTimeEdit(tw);
			dte->setDisplayFormat(m_plot->rangeDateTimeFormat(dir, i));
			dte->setTimeSpec(Qt::UTC);
			dte->setDateTime(QDateTime::fromMSecsSinceEpoch(r.end(), Qt::UTC));
			dte->setWrapping(true);
			tw->setCellWidget(i, TwRangesColumn::Max, dte);
			dte->setProperty("row", i);
			connect(dte, &QDateTimeEdit::dateTimeChanged, [this, dir, dte](const QDateTime& dateTime) {
				this->maxDateTimeChanged(dte, dir, dateTime);
			});
		}

		// scale
		cb = new ComboBoxIgnoreWheel(tw);
		// TODO: -> updateLocale()
		for (const auto& name : RangeT::scaleNames)
			cb->addItem(name);

		cb->setCurrentIndex(static_cast<int>(scale));
		cb->setProperty("row", i);
		tw->setCellWidget(i, TwRangesColumn::Scale, cb);
		connect(cb, QOverload<int>::of(&QComboBox::currentIndexChanged), [this, dir, cb](int index) {
			this->scaleChanged(cb, dir, index);
		});
	}
	tw->resizeColumnToContents(1);

	tb->setEnabled(rangeCount > 1 ? true : false);

	if (m_updateUI) {
		updateLocale(); // fill values
		updatePlotRangeList(); // update x ranges used in plot ranges
	}

	// enable/disable widgets
	for (int i = 0; i < rangeCount; i++) {
		const bool checked{m_plot->range(dir, i).autoScale()};
		CELLWIDGET(dir, i, TwRangesColumn::Format, QComboBox, setEnabled(!checked));
		CELLWIDGET(dir, i, TwRangesColumn::Min, QWidget, setEnabled(!checked));
		CELLWIDGET(dir, i, TwRangesColumn::Max, QWidget, setEnabled(!checked));
	}
}

// update plot ranges in list
void CartesianPlotDock::updatePlotRangeList() {
	if (!m_plot)
		return;

	const int cSystemCount{m_plot->coordinateSystemCount()};
	DEBUG(Q_FUNC_INFO << ", nr of coordinate systems = " << cSystemCount)
	if (cSystemCount > 1)
		ui.lPlotRanges->setText(i18n("Plot Ranges:"));
	else
		ui.lPlotRanges->setText(i18n("Plot Range:"));
	ui.twPlotRanges->setRowCount(cSystemCount);
	for (int i = 0; i < cSystemCount; i++) {
		const auto* cSystem{m_plot->coordinateSystem(i)};
		const int xIndex{cSystem->index(Direction::X)}, yIndex{cSystem->index(Direction::Y)};
		const auto xRange{m_plot->range(Direction::X, xIndex)}, yRange{m_plot->range(Direction::Y, yIndex)};

		DEBUG(Q_FUNC_INFO << ", coordinate system " << i + 1 << " : xIndex = " << xIndex << ", yIndex = " << yIndex)
		DEBUG(Q_FUNC_INFO << ", x range = " << xRange.toStdString() << ", auto scale = " << xRange.autoScale())
		DEBUG(Q_FUNC_INFO << ", y range = " << yRange.toStdString() << ", auto scale = " << yRange.autoScale())

		auto* cb = new ComboBoxIgnoreWheel(ui.twPlotRanges);
		cb->setEditable(true); // to have a line edit
		cb->lineEdit()->setReadOnly(true);
		cb->lineEdit()->setAlignment(Qt::AlignHCenter);
		if (m_plot->rangeCount(Direction::X) > 1) {
			for (int index = 0; index < m_plot->rangeCount(Direction::X); index++)
				cb->addItem(QString::number(index + 1) + QLatin1String(" : ") + m_plot->range(Direction::X, index).toLocaleString());
			cb->setCurrentIndex(xIndex);
			cb->setProperty("row", i);
			connect(cb, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &CartesianPlotDock::PlotRangeXChanged);
		} else {
			cb->addItem(xRange.toLocaleString());
			cb->setStyleSheet("QComboBox::drop-down {border-width: 0px;}"); // hide arrow if there is only one range
		}
		ui.twPlotRanges->setCellWidget(i, TwPlotRangesColumn::XRange, cb);

		cb = new ComboBoxIgnoreWheel(ui.twPlotRanges);
		cb->setEditable(true); // to have a line edit
		cb->lineEdit()->setReadOnly(true);
		cb->lineEdit()->setAlignment(Qt::AlignHCenter);
		if (m_plot->rangeCount(Direction::Y) > 1) {
			for (int index = 0; index < m_plot->rangeCount(Direction::Y); index++)
				cb->addItem(QString::number(index + 1) + QLatin1String(" : ") + m_plot->range(Direction::Y, index).toLocaleString());
			cb->setCurrentIndex(yIndex);
			cb->setProperty("row", i);
			connect(cb, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &CartesianPlotDock::PlotRangeYChanged);
		} else {
			cb->addItem(yRange.toLocaleString());
			cb->setStyleSheet("QComboBox::drop-down {border-width: 0px;}"); // hide arrow if there is only one range
		}
		ui.twPlotRanges->setCellWidget(i, TwPlotRangesColumn::YRange, cb);
	}
	ui.twPlotRanges->resizeColumnToContents(0);
	ui.twPlotRanges->resizeColumnToContents(1);

	if (m_bgDefaultPlotRange) {
		for (auto* button : m_bgDefaultPlotRange->buttons())
			m_bgDefaultPlotRange->removeButton(button);
	} else {
		m_bgDefaultPlotRange = new QButtonGroup(this);
		connect(m_bgDefaultPlotRange, QOverload<QAbstractButton*>::of(&QButtonGroup::buttonClicked), this, &CartesianPlotDock::defaultPlotRangeChanged);
	}
	for (int i = 0; i < cSystemCount; i++) {
		auto* rb = new QRadioButton();
		if (i == m_plot->defaultCoordinateSystemIndex())
			rb->setChecked(true);
		m_bgDefaultPlotRange->addButton(rb);
		rb->setStyleSheet("margin-left:50%; margin-right:50%;"); // center button
		ui.twPlotRanges->setCellWidget(i, TwPlotRangesColumn::Default, rb);
		m_bgDefaultPlotRange->setId(rb, i);
	}

	ui.tbRemovePlotRange->setEnabled(cSystemCount > 1 ? true : false);
}

//************************************************************
//**** SLOTs for changes triggered in CartesianPlotDock ******
//************************************************************
void CartesianPlotDock::retranslateUi() {
	Lock lock(m_initializing);

	// data range types
	ui.cbRangeType->clear();
	ui.cbRangeType->addItem(i18n("Free"));
	ui.cbRangeType->addItem(i18n("Last Points"));
	ui.cbRangeType->addItem(i18n("First Points"));

	QString msg = i18n(
		"Data Range:"
		"<ul>"
		"<li>Free - full data range is plotted</li>"
		"<li>Last Points - specified number of last points is plotted</li>"
		"<li>First Points - specified number of first points is plotted</li>"
		"</ul>");
	ui.lRangeType->setToolTip(msg);
	ui.cbRangeType->setToolTip(msg);

	msg = i18n("If checked, automatically extend the plot range to nice values");
	ui.lNiceExtend->setToolTip(msg);
	ui.cbNiceExtend->setToolTip(msg);

	// scale breakings
	ui.cbXBreakStyle->addItem(i18n("Simple"));
	ui.cbXBreakStyle->addItem(i18n("Vertical"));
	ui.cbXBreakStyle->addItem(i18n("Sloped"));

	ui.cbYBreakStyle->addItem(i18n("Simple"));
	ui.cbYBreakStyle->addItem(i18n("Vertical"));
	ui.cbYBreakStyle->addItem(i18n("Sloped"));

	// plot area
	GuiTools::updatePenStyles(ui.cbBorderStyle, Qt::black);

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

void CartesianPlotDock::rangeTypeChanged(int index) {
	auto type = static_cast<CartesianPlot::RangeType>(index);
	if (type == CartesianPlot::RangeType::Free) {
		ui.lRangePoints->hide();
		ui.leRangePoints->hide();
	} else {
		ui.lRangePoints->show();
		ui.leRangePoints->show();
		SET_NUMBER_LOCALE;
		if (type == CartesianPlot::RangeType::First)
			ui.leRangePoints->setText(numberLocale.toString(m_plot->rangeFirstValues()));
		else
			ui.leRangePoints->setText(numberLocale.toString(m_plot->rangeLastValues()));
	}

	if (m_initializing)
		return;

	for (auto* plot : m_plotList)
		plot->setRangeType(type);
}

void CartesianPlotDock::niceExtendChanged(bool checked) {
	if (m_initializing)
		return;

	for (auto* plot : m_plotList)
		plot->setNiceExtend(checked);
	updatePlotRangeList();
}

void CartesianPlotDock::rangePointsChanged(const QString& text) {
	if (m_initializing)
		return;

	const int value = text.toInt();
	auto type = static_cast<CartesianPlot::RangeType>(ui.cbRangeType->currentIndex());
	if (type == CartesianPlot::RangeType::First) {
		for (auto* plot : m_plotList)
			plot->setRangeFirstValues(value);
	} else {
		for (auto* plot : m_plotList)
			plot->setRangeLastValues(value);
	}
}

void CartesianPlotDock::autoScaleChanged(const QObject* sender, const Direction dir, bool state) {
	DEBUG(Q_FUNC_INFO << ", state = " << state)
	if (m_initializing)
		return;

	Lock lock(m_initializing);
	const int rangeIndex{sender->property("row").toInt()}; // TODO: is sender still valid here?
	DEBUG(Q_FUNC_INFO << ", range index: " << rangeIndex)

	autoScaleRange(dir, rangeIndex, state);
}

void CartesianPlotDock::autoScaleRange(const Direction dir, const int index, bool checked) {
	DEBUG(Q_FUNC_INFO << ", index = " << index << " checked = " << checked)

	QTableWidget* treewidget;
	Direction dir_other;
	switch (dir) {
	case Direction::X:
		dir_other = Direction::Y;
		treewidget = ui.twXRanges;
		break;
	case Direction::Y:
		dir_other = Direction::X;
		treewidget = ui.twYRanges;
		break;
	}

	if (treewidget->cellWidget(index, TwRangesColumn::Format)) {
		CELLWIDGET(dir, index, TwRangesColumn::Format, QComboBox, setEnabled(!checked));
		CELLWIDGET(dir, index, TwRangesColumn::Min, QWidget, setEnabled(!checked));
		CELLWIDGET(dir, index, TwRangesColumn::Max, QWidget, setEnabled(!checked));
	}

	for (auto* plot : m_plotList) {
		bool retransform = true; // must be true, because in enableAutoScale scaleAutoX will be already called
		plot->enableAutoScale(dir, index, checked, true);
		DEBUG(Q_FUNC_INFO << " new auto scale = " << plot->range(dir, index).autoScale())
		if (checked) { // && index == plot->defaultCoordinateSystem()->index(Direction::Y)
			retransform |= plot->scaleAuto(dir, index, true);

			for (int i = 0; i < plot->coordinateSystemCount(); i++) {
				auto cSystem = plot->coordinateSystem(i);
				if (cSystem->index(dir) == index) {
					if (plot->autoScale(dir_other, cSystem->index(dir)))
						retransform |= plot->scaleAuto(dir_other, cSystem->index(dir_other), false);
				}
			}
		}
		if (retransform) { // TODO: not necessary to retransform all coordinatesystems. Maybe storing in a vector all system indices and then retransform only
						   // them
			plot->retransformScales();
			plot->retransform();
		} else {
			// TODO:when no object used the range, handle it differently
		}
	}
	updateRangeList(dir); // see range changes
}

void CartesianPlotDock::minChanged(const QObject* sender, const Direction dir, const QString& value) {
	DEBUG(Q_FUNC_INFO << ", value = " << STDSTRING(value))
	if (m_initializing)
		return;

	Direction dir_other;
	switch (dir) {
	case Direction::X:
		dir_other = Direction::Y;
		break;
	case Direction::Y:
		dir_other = Direction::X;
		break;
	}

	const Lock lock(m_initializing);
	bool ok;
	SET_NUMBER_LOCALE
	const double min = numberLocale.toDouble(value, &ok);
	if (ok) {
		// selected x/y range
		const int index{sender->property("row").toInt()};
		DEBUG(Q_FUNC_INFO << ", x range index: " << index)
		bool changed = false;
		for (auto* plot : m_plotList)
			if (!qFuzzyCompare(min, plot->range(dir, index).start())) {
				plot->setMin(dir, index, min);
				changed = true;
			}

		if (changed)
			updateRangeList(dir_other); // plot is auto scaled
	}
}

void CartesianPlotDock::maxChanged(const QObject* sender, const Direction dir, const QString& value) {
	DEBUG(Q_FUNC_INFO << ", value = " << STDSTRING(value))
	if (m_initializing)
		return;

	Direction dir_other;
	switch (dir) {
	case Direction::X:
		dir_other = Direction::Y;
		break;
	case Direction::Y:
		dir_other = Direction::X;
		break;
	}

	const Lock lock(m_initializing);
	bool ok;
	SET_NUMBER_LOCALE
	const double max = numberLocale.toDouble(value, &ok);
	if (ok) {
		// selected x/y range
		const int index{sender->property("row").toInt()};
		bool changed = false;
		for (auto* plot : m_plotList)
			if (!qFuzzyCompare(max, plot->range(dir, index).end())) {
				plot->setMax(dir, index, max);
				changed = true;
			}

		if (changed)
			updateRangeList(dir_other); // plot is auto scaled
	}
}

void CartesianPlotDock::rangeChanged(const QObject* sender, const Direction dir, const Range<double>& range) {
	if (m_initializing)
		return;

	// selected x range
	const int index{sender->property("row").toInt()};
	DEBUG(Q_FUNC_INFO << ", range index: " << index)
	for (auto* plot : m_plotList)
		plot->setRange(dir, index, range);
	updatePlotRangeList();
}

void CartesianPlotDock::minDateTimeChanged(const QObject* sender, const Direction dir, const QDateTime& dateTime) {
	if (m_initializing)
		return;

	quint64 value = dateTime.toMSecsSinceEpoch();
	// selected x range
	const int index{sender->property("row").toInt()};
	DEBUG(Q_FUNC_INFO << ", x range index: " << index)
	for (auto* plot : m_plotList)
		plot->setMin(dir, index, value);
	updatePlotRangeList();
}

void CartesianPlotDock::maxDateTimeChanged(const QObject* sender, const Direction dir, const QDateTime& dateTime) {
	if (m_initializing)
		return;

	quint64 value = dateTime.toMSecsSinceEpoch();
	// selected x range
	const int index{sender->property("row").toInt()};
	DEBUG(Q_FUNC_INFO << ", x range index: " << index)
	for (auto* plot : m_plotList)
		plot->setMax(dir, index, value);
	updatePlotRangeList();
}

/*!
 *  called on scale changes (linear, log) for the x-/y-axis
 */
void CartesianPlotDock::scaleChanged(const QObject* sender, Direction dir, int index) {
	if (m_initializing)
		return;

	const int rangeIndex{sender->property("row").toInt()};
	DEBUG(Q_FUNC_INFO << ", range " << rangeIndex << " scale changed to " << ENUM_TO_STRING(RangeT, Scale, index))
	const auto scale{static_cast<RangeT::Scale>(index)};
	for (auto* plot : m_plotList)
		plot->setRangeScale(dir, rangeIndex, scale);
}

void CartesianPlotDock::rangeFormatChanged(const QObject* sender, Direction dir, int index) {
	const int rangeIndex{sender->property("row").toInt()};
	DEBUG(Q_FUNC_INFO << ", x range " << rangeIndex + 1 << " format = " << index)

	if (m_initializing)
		return;

	const auto format{static_cast<RangeT::Format>(index)};
	for (auto* plot : m_plotList) {
		DEBUG(Q_FUNC_INFO << ", set format of range " << rangeIndex + 1 << " to " << static_cast<int>(format))
		plot->setRangeFormat(dir, rangeIndex, format);
	}
	updateRangeList(dir);
}

void CartesianPlotDock::addXRange() {
	if (!m_plot)
		return;

	DEBUG(Q_FUNC_INFO << ", current x range count = " << m_plot->rangeCount(Direction::X))

	m_plot->addXRange();
	updateRangeList(Direction::X);
}
void CartesianPlotDock::addYRange() {
	if (!m_plot)
		return;

	DEBUG(Q_FUNC_INFO << ", current y range count = " << m_plot->rangeCount(Direction::Y))

	m_plot->addYRange();
	updateRangeList(Direction::Y);
}

void CartesianPlotDock::removeXRange() {
	removeRange(Direction::X);
}

void CartesianPlotDock::removeYRange() {
	removeRange(Direction::Y);
}

void CartesianPlotDock::removeRange(const Direction dir) {
	if (!m_plot)
		return;

	QTableWidget* treewidget;
	switch (dir) {
	case Direction::X:
		treewidget = ui.twXRanges;
		break;
	case Direction::Y:
		treewidget = ui.twYRanges;
		break;
	}

	int currentRow{treewidget->currentRow()};
	QDEBUG(Q_FUNC_INFO << ", current range = " << currentRow)
	if (currentRow < 0 || currentRow > m_plot->rangeCount(dir)) {
		DEBUG(Q_FUNC_INFO << ", no current range")
		currentRow = m_plot->rangeCount(dir) - 1;
	}
	QDEBUG(Q_FUNC_INFO << ", removing range " << currentRow)

	// check plot ranges using range to remove
	const int cSystemCount{m_plot->coordinateSystemCount()};
	DEBUG(Q_FUNC_INFO << ", nr of cSystems = " << cSystemCount)
	QString msg;
	for (int i{0}; i < cSystemCount; i++) {
		const auto* cSystem{m_plot->coordinateSystem(i)};

		if (cSystem->index(dir) == currentRow) {
			if (msg.size() > 0)
				msg += ", ";
			msg += QString::number(i + 1);
		}
	}

	if (msg.size() > 0) {
		DEBUG(Q_FUNC_INFO << ", range used in plot range " << STDSTRING(msg))
		auto ret = KMessageBox::warningYesNo(
			this,
			i18n("%1 range %2 is used in plot range %3. ", CartesianCoordinateSystem::directionToString(dir).toUpper(), currentRow + 1, msg)
				+ i18n("Really remove it?"));
		if (ret == KMessageBox::No)
			return;
		else {
			// reset x ranges of cSystems using the range to be removed
			for (int i{0}; i < cSystemCount; i++) {
				auto* cSystem{m_plot->coordinateSystem(i)};

				if (cSystem->index(dir) == currentRow)
					cSystem->setIndex(dir, 0); // first range
				else if (cSystem->index(dir) > currentRow)
					cSystem->setIndex(dir, cSystem->index(dir) - 1);
			}
		}
	}

	m_plot->removeRange(dir, currentRow);
	updateRangeList(dir);
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

	int currentRow{ui.twPlotRanges->currentRow()};
	QDEBUG(Q_FUNC_INFO << ", current plot range = " << currentRow)
	if (currentRow < 0 || currentRow > m_plot->coordinateSystemCount()) {
		DEBUG(Q_FUNC_INFO << ", no current plot range")
		currentRow = m_plot->coordinateSystemCount() - 1;
	}
	QDEBUG(Q_FUNC_INFO << ", removing plot range " << currentRow)

	// check all children for cSystem usage
	for (auto* element : m_plot->children<WorksheetElement>()) {
		const int cSystemIndex{element->coordinateSystemIndex()};
		DEBUG(Q_FUNC_INFO << ", element x index = " << cSystemIndex)
		if (cSystemIndex == currentRow) {
			DEBUG(Q_FUNC_INFO << ", WARNING: plot range used in element")
			auto ret =
				KMessageBox::warningYesNo(this, i18n("Plot range %1 is used by element \"%2\". ", currentRow + 1, element->name()) + i18n("Really remove it?"));
			if (ret == KMessageBox::No)
				return;
			else
				element->setCoordinateSystemIndex(0); // reset
		}
	}

	m_plot->removeCoordinateSystem(currentRow);
	updatePlotRangeList();
	m_plot->retransform(); // update plot and elements
}

/*
 * Called when x/y range of plot range in plot range list changes
 */
void CartesianPlotDock::PlotRangeXChanged(const int index) {
	const int plotRangeIndex{sender()->property("row").toInt()};
	DEBUG(Q_FUNC_INFO << ", Set x range of plot range " << plotRangeIndex + 1 << " to " << index + 1)
	auto* cSystem{m_plot->coordinateSystem(plotRangeIndex)};
	cSystem->setIndex(Direction::X, index);

	// auto scale x range when on auto scale (now that it is used)
	if (m_plot->range(Direction::X, index).autoScale()) {
		autoScaleRange(Direction::X, index, true);
		updateRangeList(Direction::X);
	}

	for (auto* axis : m_plot->children<Axis>()) {
		const int cSystemIndex{axis->coordinateSystemIndex()};
		DEBUG(Q_FUNC_INFO << ", Axis \"" << STDSTRING(axis->name()) << "\" cSystem index = " << cSystemIndex)
		if (cSystemIndex == plotRangeIndex) {
			DEBUG(Q_FUNC_INFO << ", Plot range used in axis \"" << STDSTRING(axis->name()) << "\" has changed")
			if (axis->rangeType() == Axis::RangeType::Auto && axis->orientation() == Axis::Orientation::Horizontal) {
				DEBUG(Q_FUNC_INFO << ", set x range of axis to " << m_plot->range(Direction::X, index).toStdString())
				axis->setRange(m_plot->range(Direction::X, index));
			}
		}
	}

	m_plot->dataChanged(index, -1); // update plot
}
void CartesianPlotDock::PlotRangeYChanged(const int index) {
	const int plotRangeIndex{sender()->property("row").toInt()};
	DEBUG(Q_FUNC_INFO << ", set y range of plot range " << plotRangeIndex + 1 << " to " << index + 1)
	auto* cSystem{m_plot->coordinateSystem(plotRangeIndex)};
	cSystem->setIndex(Direction::Y, index);

	// auto scale y range when on auto scale (now that it is used)
	if (m_plot->range(Direction::Y, index).autoScale()) {
		autoScaleRange(Direction::Y, index, true);
		updateRangeList(Direction::Y);
	}
	for (auto* axis : m_plot->children<Axis>()) {
		const int cSystemIndex{axis->coordinateSystemIndex()};
		if (cSystemIndex == plotRangeIndex) {
			DEBUG(Q_FUNC_INFO << ", plot range used in axis \"" << STDSTRING(axis->name()) << "\" has changed")
			if (axis->rangeType() == Axis::RangeType::Auto && axis->orientation() == Axis::Orientation::Vertical) {
				DEBUG(Q_FUNC_INFO << ", set range to " << m_plot->range(Direction::Y, index).toStdString())
				axis->setRange(m_plot->range(Direction::Y, index));
			}
		}
	}

	// TODO: cha
	m_plot->dataChanged(-1, index); // update plot
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
	breaks.list << b;
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
	ui.sbXBreakPosition->setValue(rangeBreak.position * 100);
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
	breaks.list[index].position = (double)value / 100.;
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
	ui.cbYBreak->setCurrentIndex(ui.cbYBreak->count() - 1);
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

	if (index < ui.cbYBreak->count() - 1)
		ui.cbYBreak->setCurrentIndex(index);
	else
		ui.cbYBreak->setCurrentIndex(ui.cbYBreak->count() - 1);

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
	breaks.list[index].position = (float)value / 100.;
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
void CartesianPlotDock::borderTypeChanged() {
	if (m_initializing)
		return;

	auto type = m_plot->plotArea()->borderType();
	auto* tb = static_cast<QToolButton*>(QObject::sender());
	bool checked = tb->isChecked();
	if (tb == ui.tbBorderTypeLeft)
		type.setFlag(PlotArea::BorderTypeFlags::BorderLeft, checked);
	else if (tb == ui.tbBorderTypeTop)
		type.setFlag(PlotArea::BorderTypeFlags::BorderTop, checked);
	else if (tb == ui.tbBorderTypeRight)
		type.setFlag(PlotArea::BorderTypeFlags::BorderRight, checked);
	else if (tb == ui.tbBorderTypeBottom)
		type.setFlag(PlotArea::BorderTypeFlags::BorderBottom, checked);

	for (auto* plot : m_plotList)
		plot->plotArea()->setBorderType(type);
}

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
		pen.setWidthF(Worksheet::convertToSceneUnits(value, Worksheet::Unit::Point));
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

	qreal opacity = (double)value / 100.;
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
		// if symmetric padding is active we also adjust the right padding.
		// start a macro in this case to only have one single entry on the undo stack.
		// TODO: ideally this is done in CartesianPlot and is completely transparent to CartesianPlotDock.
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
		pen.setWidthF(Worksheet::convertToSceneUnits(width, Worksheet::Unit::Point));
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

void CartesianPlotDock::exportPlotTemplate() {
	KConfig config;
	KConfigGroup group = config.group(QLatin1String("PlotTemplate"));
	const QString dir = group.readEntry(QLatin1String("ExportPath"), PlotTemplateDialog::defaultTemplateInstallPath());
	QString path = QFileDialog::getSaveFileName(nullptr,
												i18nc("@title:window", "Choose Template Save File"),
												dir,
												i18n("Labplot Plot Templates (*%1)", PlotTemplateDialog::format));

	if (path.split(PlotTemplateDialog::format).count() < 2)
		path.append(PlotTemplateDialog::format); // Sometimes the format is not added to the file. Don't know why
	QFile file(path);
	if (!file.open(QIODevice::OpenModeFlag::WriteOnly)) {
		// TODO: show error message
		return;
	}
	QXmlStreamWriter writer(&file);
	writer.setAutoFormatting(true);
	writer.writeStartDocument();
	writer.writeDTD("<!DOCTYPE LabPlotXML>");
	writer.writeStartElement("PlotTemplate");
	writer.writeAttribute("xmlVersion", QString::number(Project::currentBuildXmlVersion()));
	m_plot->save(&writer);
	writer.writeEndElement();
	writer.writeEndDocument();
}

//*************************************************************
//****** SLOTs for changes triggered in CartesianPlot *********
//*************************************************************
// general
void CartesianPlotDock::plotRectChanged(QRectF& rect) {
	if (m_initializing)
		return;
	const Lock lock(m_initializing);
	ui.sbLeft->setValue(Worksheet::convertFromSceneUnits(rect.x(), m_worksheetUnit));
	ui.sbTop->setValue(Worksheet::convertFromSceneUnits(rect.y(), m_worksheetUnit));
	ui.sbWidth->setValue(Worksheet::convertFromSceneUnits(rect.width(), m_worksheetUnit));
	ui.sbHeight->setValue(Worksheet::convertFromSceneUnits(rect.height(), m_worksheetUnit));
}

void CartesianPlotDock::plotRangeTypeChanged(CartesianPlot::RangeType type) {
	if (m_initializing)
		return;
	const Lock lock(m_initializing);
	ui.cbRangeType->setCurrentIndex(static_cast<int>(type));
}
void CartesianPlotDock::plotRangeFirstValuesChanged(int value) {
	if (m_initializing)
		return;
	const Lock lock(m_initializing);
	SET_NUMBER_LOCALE
	ui.leRangePoints->setText(numberLocale.toString(value));
}
void CartesianPlotDock::plotRangeLastValuesChanged(int value) {
	if (m_initializing)
		return;
	const Lock lock(m_initializing);
	SET_NUMBER_LOCALE
	ui.leRangePoints->setText(numberLocale.toString(value));
}

// x & y ranges
void CartesianPlotDock::plotAutoScaleChanged(Direction dir, int index, bool checked) {
	if (m_initializing)
		return;
	const Lock lock(m_initializing);
	DEBUG(Q_FUNC_INFO << ", checked = " << checked)

	CELLWIDGET(dir, index, TwRangesColumn::Automatic, QCheckBox, setChecked(checked));
	CELLWIDGET(dir, index, TwRangesColumn::Format, QComboBox, setEnabled(!checked));
	CELLWIDGET(dir, index, TwRangesColumn::Min, QLineEdit, setEnabled(!checked));
	CELLWIDGET(dir, index, TwRangesColumn::Max, QLineEdit, setEnabled(!checked));
	CELLWIDGET(dir, index, TwRangesColumn::Scale, QComboBox, setEnabled(!checked));
}

void CartesianPlotDock::plotXMinChanged(int xRangeIndex, double value) {
	DEBUG(Q_FUNC_INFO << ", value = " << value)
	if (m_initializing)
		return;
	const Lock lock(m_initializing);
	SET_NUMBER_LOCALE
	CELLWIDGET(Direction::X, xRangeIndex, TwRangesColumn::Min, QLineEdit, setText(numberLocale.toString(value)));
	CELLWIDGET(Direction::X, xRangeIndex, TwRangesColumn::Min, QDateTimeEdit, setDateTime(QDateTime::fromMSecsSinceEpoch(value, Qt::UTC)));
}
void CartesianPlotDock::plotYMinChanged(int yRangeIndex, double value) {
	DEBUG(Q_FUNC_INFO << ", value = " << value)
	if (m_initializing)
		return;
	const Lock lock(m_initializing);
	SET_NUMBER_LOCALE
	CELLWIDGET(Direction::Y, yRangeIndex, TwRangesColumn::Min, QLineEdit, setText(numberLocale.toString(value)));
	CELLWIDGET(Direction::Y, yRangeIndex, TwRangesColumn::Min, QDateTimeEdit, setDateTime(QDateTime::fromMSecsSinceEpoch(value, Qt::UTC)));
}

void CartesianPlotDock::plotXMaxChanged(int xRangeIndex, double value) {
	DEBUG(Q_FUNC_INFO << ", value = " << value)
	if (m_initializing)
		return;
	const Lock lock(m_initializing);
	SET_NUMBER_LOCALE
	CELLWIDGET(Direction::X, xRangeIndex, TwRangesColumn::Max, QLineEdit, setText(numberLocale.toString(value)));
	CELLWIDGET(Direction::X, xRangeIndex, TwRangesColumn::Max, QDateTimeEdit, setDateTime(QDateTime::fromMSecsSinceEpoch(value, Qt::UTC)));
}
void CartesianPlotDock::plotYMaxChanged(int yRangeIndex, double value) {
	DEBUG(Q_FUNC_INFO << ", value = " << value)
	if (m_initializing)
		return;
	const Lock lock(m_initializing);
	SET_NUMBER_LOCALE
	CELLWIDGET(Direction::Y, yRangeIndex, TwRangesColumn::Max, QLineEdit, setText(numberLocale.toString(value)));
	CELLWIDGET(Direction::Y, yRangeIndex, TwRangesColumn::Max, QDateTimeEdit, setDateTime(QDateTime::fromMSecsSinceEpoch(value, Qt::UTC)));
}

void CartesianPlotDock::plotRangeChanged(Direction dir, int index, Range<double> range) {
	DEBUG(Q_FUNC_INFO << ", " << CartesianCoordinateSystem::directionToString(dir).toStdString() << " range = " << range.toStdString())
	if (m_initializing)
		return;

	const Lock lock(m_initializing);
	SET_NUMBER_LOCALE
	CELLWIDGET(dir, index, TwRangesColumn::Min, QLineEdit, setText(numberLocale.toString(range.start())));
	CELLWIDGET(dir, index, TwRangesColumn::Min, QDateTimeEdit, setDateTime(QDateTime::fromMSecsSinceEpoch(range.start(), Qt::UTC)));
	CELLWIDGET(dir, index, TwRangesColumn::Max, QLineEdit, setText(numberLocale.toString(range.end())));
	CELLWIDGET(dir, index, TwRangesColumn::Max, QDateTimeEdit, setDateTime(QDateTime::fromMSecsSinceEpoch(range.end(), Qt::UTC)));
}

void CartesianPlotDock::plotScaleChanged(const Direction dir, int rangeIndex, RangeT::Scale scale) {
	DEBUG(Q_FUNC_INFO << ", scale = " << ENUM_TO_STRING(RangeT, Scale, scale))
	m_initializing = true;
	CELLWIDGET(dir, rangeIndex, TwRangesColumn::Scale, QComboBox, setCurrentIndex(static_cast<int>(scale)));
	m_initializing = false;
}

void CartesianPlotDock::plotRangeFormatChanged(Direction dir, int rangeIndex, RangeT::Format format) {
	DEBUG(Q_FUNC_INFO << ", format = " << ENUM_TO_STRING(RangeT, Format, format))
	m_initializing = true;
	CELLWIDGET(dir, rangeIndex, TwRangesColumn::Format, QComboBox, setCurrentIndex(static_cast<int>(format)));
	m_initializing = false;
}

// plot range
void CartesianPlotDock::defaultPlotRangeChanged() {
	const int index{m_bgDefaultPlotRange->checkedId()};
	DEBUG(Q_FUNC_INFO << ", index = " << index)
	m_plot->setDefaultCoordinateSystemIndex(index);
	updatePlotRangeList(); // changing default cSystem may change x/y-ranges when on auto scale
	m_plot->retransform(); // update plot
}

// range breaks
void CartesianPlotDock::plotXRangeBreakingEnabledChanged(bool on) {
	m_initializing = true;
	ui.chkXBreak->setChecked(on);
	m_initializing = false;
}

void CartesianPlotDock::plotXRangeBreaksChanged(const CartesianPlot::RangeBreaks&) {
}

void CartesianPlotDock::plotYRangeBreakingEnabledChanged(bool on) {
	m_initializing = true;
	ui.chkYBreak->setChecked(on);
	m_initializing = false;
}

void CartesianPlotDock::plotYRangeBreaksChanged(const CartesianPlot::RangeBreaks&) {
}

void CartesianPlotDock::plotVisibleChanged(bool on) {
	m_initializing = true;
	ui.chkVisible->setChecked(on);
	m_initializing = false;
}

// border
void CartesianPlotDock::plotBorderTypeChanged(PlotArea::BorderType type) {
	Lock lock(m_initializing);
	ui.tbBorderTypeLeft->setChecked(type.testFlag(PlotArea::BorderTypeFlags::BorderLeft));
	ui.tbBorderTypeRight->setChecked(type.testFlag(PlotArea::BorderTypeFlags::BorderRight));
	ui.tbBorderTypeTop->setChecked(type.testFlag(PlotArea::BorderTypeFlags::BorderTop));
	ui.tbBorderTypeBottom->setChecked(type.testFlag(PlotArea::BorderTypeFlags::BorderBottom));
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

void CartesianPlotDock::plotBorderCornerRadiusChanged(double value) {
	m_initializing = true;
	ui.sbBorderCornerRadius->setValue(Worksheet::convertFromSceneUnits(value, m_worksheetUnit));
	m_initializing = false;
}

void CartesianPlotDock::plotBorderOpacityChanged(double value) {
	m_initializing = true;
	float v = (float)value * 100.;
	ui.sbBorderOpacity->setValue(v);
	m_initializing = false;
}

void CartesianPlotDock::plotHorizontalPaddingChanged(double value) {
	m_initializing = true;
	ui.sbPaddingHorizontal->setValue(Worksheet::convertFromSceneUnits(value, m_worksheetUnit));
	m_initializing = false;
}

void CartesianPlotDock::plotVerticalPaddingChanged(double value) {
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
	// extract the name of the template from the file name
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
	// General-tab
	ui.chkVisible->setChecked(m_plot->isVisible());
	ui.sbLeft->setValue(Worksheet::convertFromSceneUnits(m_plot->rect().x(), m_worksheetUnit));
	ui.sbTop->setValue(Worksheet::convertFromSceneUnits(m_plot->rect().y(), m_worksheetUnit));
	ui.sbWidth->setValue(Worksheet::convertFromSceneUnits(m_plot->rect().width(), m_worksheetUnit));
	ui.sbHeight->setValue(Worksheet::convertFromSceneUnits(m_plot->rect().height(), m_worksheetUnit));

	int index = static_cast<int>(m_plot->rangeType());
	ui.cbRangeType->setCurrentIndex(index);
	rangeTypeChanged(index);
	ui.cbNiceExtend->setChecked(m_plot->niceExtend());

	m_updateUI = false; // avoid updating twice
	updateRangeList(Direction::X);
	m_updateUI = true;
	updateRangeList(Direction::Y);

	// Title
	labelWidget->load();

	// x-range breaks, show the first break
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

	// y-range breaks, show the first break
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
	const auto* plotArea = m_plot->plotArea();

	// Background
	QList<Background*> backgrounds;
	for (auto* plot : m_plotList)
		backgrounds << plot->plotArea()->background();

	backgroundWidget->setBackgrounds(backgrounds);

	// Padding
	ui.sbPaddingHorizontal->setValue(Worksheet::convertFromSceneUnits(m_plot->horizontalPadding(), m_worksheetUnit));
	ui.sbPaddingVertical->setValue(Worksheet::convertFromSceneUnits(m_plot->verticalPadding(), m_worksheetUnit));
	ui.sbPaddingRight->setValue(Worksheet::convertFromSceneUnits(m_plot->rightPadding(), m_worksheetUnit));
	ui.sbPaddingBottom->setValue(Worksheet::convertFromSceneUnits(m_plot->bottomPadding(), m_worksheetUnit));
	ui.cbPaddingSymmetric->setChecked(m_plot->symmetricPadding());

	// Border
	ui.tbBorderTypeLeft->setChecked(plotArea->borderType().testFlag(PlotArea::BorderTypeFlags::BorderLeft));
	ui.tbBorderTypeRight->setChecked(plotArea->borderType().testFlag(PlotArea::BorderTypeFlags::BorderRight));
	ui.tbBorderTypeTop->setChecked(plotArea->borderType().testFlag(PlotArea::BorderTypeFlags::BorderTop));
	ui.tbBorderTypeBottom->setChecked(plotArea->borderType().testFlag(PlotArea::BorderTypeFlags::BorderBottom));
	ui.kcbBorderColor->setColor(plotArea->borderPen().color());
	ui.cbBorderStyle->setCurrentIndex((int)plotArea->borderPen().style());
	ui.sbBorderWidth->setValue(Worksheet::convertFromSceneUnits(plotArea->borderPen().widthF(), Worksheet::Unit::Point));
	ui.sbBorderCornerRadius->setValue(Worksheet::convertFromSceneUnits(plotArea->borderCornerRadius(), m_worksheetUnit));
	ui.sbBorderOpacity->setValue(round(plotArea->borderOpacity() * 100));
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

	// General
	// we don't load/save the settings in the general-tab, since they are not style related.
	// It doesn't make sense to load/save them in the template.
	// This data is read in CartesianPlotDock::setPlots().

	// Title
	KConfigGroup plotTitleGroup = config.group("CartesianPlotTitle");
	labelWidget->loadConfig(plotTitleGroup);

	// Scale breakings
	// TODO

	// Background-tab
	backgroundWidget->loadConfig(group);

	// Layout-tab
	ui.sbPaddingHorizontal->setValue(Worksheet::convertFromSceneUnits(group.readEntry("HorizontalPadding", m_plot->horizontalPadding()), m_worksheetUnit));
	ui.sbPaddingVertical->setValue(Worksheet::convertFromSceneUnits(group.readEntry("VerticalPadding", m_plot->verticalPadding()), m_worksheetUnit));
	ui.sbPaddingRight->setValue(Worksheet::convertFromSceneUnits(group.readEntry("RightPadding", m_plot->rightPadding()), m_worksheetUnit));
	ui.sbPaddingBottom->setValue(Worksheet::convertFromSceneUnits(group.readEntry("BottomPadding", m_plot->bottomPadding()), m_worksheetUnit));
	ui.cbPaddingSymmetric->setChecked(group.readEntry("SymmetricPadding", m_plot->symmetricPadding()));

	// Border-tab
	const auto* plotArea = m_plot->plotArea();
	auto type = static_cast<PlotArea::BorderType>(group.readEntry("BorderType", static_cast<int>(plotArea->borderType())));
	ui.tbBorderTypeLeft->setChecked(type.testFlag(PlotArea::BorderTypeFlags::BorderLeft));
	ui.tbBorderTypeRight->setChecked(type.testFlag(PlotArea::BorderTypeFlags::BorderRight));
	ui.tbBorderTypeTop->setChecked(type.testFlag(PlotArea::BorderTypeFlags::BorderTop));
	ui.tbBorderTypeBottom->setChecked(type.testFlag(PlotArea::BorderTypeFlags::BorderBottom));
	ui.kcbBorderColor->setColor(group.readEntry("BorderColor", plotArea->borderPen().color()));
	ui.cbBorderStyle->setCurrentIndex(group.readEntry("BorderStyle", (int)plotArea->borderPen().style()));
	ui.sbBorderWidth->setValue(Worksheet::convertFromSceneUnits(group.readEntry("BorderWidth", plotArea->borderPen().widthF()), Worksheet::Unit::Point));
	ui.sbBorderCornerRadius->setValue(Worksheet::convertFromSceneUnits(group.readEntry("BorderCornerRadius", plotArea->borderCornerRadius()), m_worksheetUnit));
	ui.sbBorderOpacity->setValue(group.readEntry("BorderOpacity", plotArea->borderOpacity()) * 100);

	m_initializing = true;
	GuiTools::updatePenStyles(ui.cbBorderStyle, ui.kcbBorderColor->color());
	GuiTools::updatePenStyles(ui.cbCursorLineStyle, m_plot->cursorPen().color());
	m_initializing = false;
}

void CartesianPlotDock::saveConfigAsTemplate(KConfig& config) {
	KConfigGroup group = config.group("CartesianPlot");

	// General
	// we don't load/save the settings in the general-tab, since they are not style related.
	// It doesn't make sense to load/save them in the template.

	// Title
	KConfigGroup plotTitleGroup = config.group("CartesianPlotTitle");
	labelWidget->saveConfig(plotTitleGroup);

	// Scale breakings
	// TODO

	// Background
	backgroundWidget->saveConfig(group);

	// Border
	group.writeEntry("BorderType", static_cast<int>(m_plot->plotArea()->borderType()));
	group.writeEntry("BorderStyle", ui.cbBorderStyle->currentIndex());
	group.writeEntry("BorderColor", ui.kcbBorderColor->color());
	group.writeEntry("BorderWidth", Worksheet::convertToSceneUnits(ui.sbBorderWidth->value(), Worksheet::Unit::Point));
	group.writeEntry("BorderCornerRadius", Worksheet::convertToSceneUnits(ui.sbBorderCornerRadius->value(), m_worksheetUnit));
	group.writeEntry("BorderOpacity", ui.sbBorderOpacity->value() / 100.0);

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
