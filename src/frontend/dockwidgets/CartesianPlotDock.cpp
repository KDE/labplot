/*
	File                 : CartesianPlotDock.cpp
	Project              : LabPlot
	Description          : widget for cartesian plot properties
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2011-2025 Alexander Semke <alexander.semke@web.de>
	SPDX-FileCopyrightText: 2012-2024 Stefan Gerlach <stefan.gerlach@uni-konstanz.de>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "CartesianPlotDock.h"
#include "backend/core/AbstractColumn.h"
#include "backend/core/Project.h"
#include "backend/core/Settings.h"

#include "frontend/GuiTools.h"
#include "frontend/PlotTemplateDialog.h"
#include "frontend/TemplateHandler.h"
#include "frontend/ThemeHandler.h"
#include "frontend/colormaps/ColorMapsDialog.h"
#include "frontend/widgets/BackgroundWidget.h"
#include "frontend/widgets/LabelWidget.h"
#include "frontend/widgets/LineWidget.h"
#include "tools/ColorMapsManager.h"

#include <KIconLoader>
#include <KLocalization>
#include <KMessageBox>

#include <QButtonGroup>
#include <QFileDialog>
#include <QIntValidator>
#include <QPainter>
#include <QRadioButton>
#include <QWheelEvent>
#include <QXmlStreamWriter>

#include <gsl/gsl_const_cgs.h>

namespace {
enum TwRangesColumn { Automatic = 0, Format, Min, Max, Scale };
enum TwPlotRangesColumn { XRange, YRange, Default, Name };

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

#define CELLWIDGET(dim, rangeIndex, Column, castObject, function)                                                                                              \
	{                                                                                                                                                          \
		QTableWidget* treeWidget = nullptr;                                                                                                                    \
		switch (dim) {                                                                                                                                         \
		case Dimension::X:                                                                                                                                     \
			treeWidget = ui.twXRanges;                                                                                                                         \
			break;                                                                                                                                             \
		case Dimension::Y:                                                                                                                                     \
			treeWidget = ui.twYRanges;                                                                                                                         \
			break;                                                                                                                                             \
		}                                                                                                                                                      \
		if (rangeIndex < 0) {                                                                                                                                  \
			for (int Row = 0; Row < treeWidget->rowCount(); Row++) {                                                                                                 \
				auto obj = qobject_cast<castObject*>(treeWidget->cellWidget(Row, Column));                                                                       \
				if (obj)                                                                                                                                       \
					obj->function;                                                                                                                             \
				else                                                                                                                                           \
					DEBUG("ERROR: qobject_cast <castObject*> failed: " << __FILE__ << ":" << __LINE__ << " ( rangeIndex:" << rangeIndex                        \
																	   << ", Column: " << Column                                                               \
																	   << "). Whether the object does not exist or the cellWidget has different type");        \
			}                                                                                                                                                  \
		} else {                                                                                                                                               \
			auto obj = qobject_cast<castObject*>(treeWidget->cellWidget(rangeIndex, Column));                                                                  \
			if (obj)                                                                                                                                           \
				obj->function;                                                                                                                                 \
			else                                                                                                                                               \
				DEBUG("ERROR: qobject_cast <castObject*> failed: " << __FILE__ << ":" << __LINE__ << " (rangeIndex:" << rangeIndex << ", Column: " << Column   \
																   << "). Whether the object does not exist or the cellWidget has different type");            \
		}                                                                                                                                                      \
	}

/*!
  \class CartesianPlotDock
  \brief  Provides a widget for editing the properties of the cartesian plot currently selected in the project explorer.

  \ingroup frontend
*/

CartesianPlotDock::CartesianPlotDock(QWidget* parent)
	: BaseDock(parent) {
	ui.setupUi(this);
	setBaseWidgets(ui.leName, ui.teComment);
	setVisibilityWidgets(ui.chkVisible);

	//"General"-tab
	ui.twXRanges->setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);
	ui.twYRanges->setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);
	ui.twPlotRanges->setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);

	//"Range breaks"-tab
	ui.bAddXBreak->setIcon(QIcon::fromTheme(QStringLiteral("list-add")));
	ui.bRemoveXBreak->setIcon(QIcon::fromTheme(QStringLiteral("list-remove")));
	ui.cbXBreak->addItem(QStringLiteral("1"));

	ui.bAddYBreak->setIcon(QIcon::fromTheme(QStringLiteral("list-add")));
	ui.bRemoveYBreak->setIcon(QIcon::fromTheme(QStringLiteral("list-remove")));
	ui.cbYBreak->addItem(QStringLiteral("1"));

	ui.bColorMap->setIcon(QIcon::fromTheme(QLatin1String("color-management")));
	ui.lColorMapPreview->setMaximumHeight(ui.bColorMap->height());

	//"Background"-tab
	auto* gridLayout = static_cast<QGridLayout*>(ui.tabPlotArea->layout());
	backgroundWidget = new BackgroundWidget(ui.tabPlotArea);
	gridLayout->addWidget(backgroundWidget, 1, 0, 1, 3);

	borderLineWidget = new LineWidget(ui.tabPlotArea);
	gridLayout->addWidget(borderLineWidget, 5, 0, 1, 3);

	//"Title"-tab
	auto* hboxLayout = new QHBoxLayout(ui.tabTitle);
	labelWidget = new LabelWidget(ui.tabTitle);
	hboxLayout->addWidget(labelWidget);
	hboxLayout->setContentsMargins(0, 0, 0, 0);
	hboxLayout->setSpacing(0);

	// Layout-tab
	QString suffix;
	if (m_units == Units::Metric)
		suffix = i18n("%v cm");
	else
		suffix = i18n("%v in");

	KLocalization::setupSpinBoxFormatString(ui.sbLeft, ki18nc("@label:spinbox Suffix for the left spacing", qPrintable(suffix)));
	KLocalization::setupSpinBoxFormatString(ui.sbTop, ki18nc("@label:spinbox Suffix for the top spacing", qPrintable(suffix)));
	KLocalization::setupSpinBoxFormatString(ui.sbWidth, ki18nc("@label:spinbox Suffix for the width", qPrintable(suffix)));
	KLocalization::setupSpinBoxFormatString(ui.sbHeight, ki18nc("@label:spinbox Suffix for the height", qPrintable(suffix)));
	KLocalization::setupSpinBoxFormatString(ui.sbBorderCornerRadius, ki18nc("@label:spinbox Suffix for the border corner radius", qPrintable(suffix)));
	KLocalization::setupSpinBoxFormatString(ui.sbPaddingHorizontal, ki18nc("@label:spinbox Suffix for the horizontal padding", qPrintable(suffix)));
	KLocalization::setupSpinBoxFormatString(ui.sbPaddingVertical, ki18nc("@label:spinbox Suffix for the vertical padding", qPrintable(suffix)));
	KLocalization::setupSpinBoxFormatString(ui.sbPaddingRight, ki18nc("@label:spinbox Suffix for the right padding", qPrintable(suffix)));
	KLocalization::setupSpinBoxFormatString(ui.sbPaddingBottom, ki18nc("@label:spinbox Suffix for the bottom padding", qPrintable(suffix)));

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
	auto* vboxLayout = static_cast<QVBoxLayout*>(ui.tabCursor->layout());
	cursorLineWidget = new LineWidget(ui.tabCursor);
	vboxLayout->insertWidget(1, cursorLineWidget);

	// Validators
	ui.leRangePoints->setValidator(new QIntValidator(ui.leRangePoints));
	ui.leXBreakStart->setValidator(new QDoubleValidator(ui.leXBreakStart));
	ui.leXBreakEnd->setValidator(new QDoubleValidator(ui.leXBreakEnd));
	ui.leYBreakStart->setValidator(new QDoubleValidator(ui.leYBreakStart));
	ui.leYBreakEnd->setValidator(new QDoubleValidator(ui.leYBreakEnd));
	ui.leStackYOffset->setValidator(new QDoubleValidator(ui.leStackYOffset));

	updateLocale();
	retranslateUi();
	init();

	// SIGNAL/SLOT
	// General
	connect(ui.cbRangeType, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &CartesianPlotDock::rangeTypeChanged);
	connect(ui.cbNiceExtend, &QCheckBox::clicked, this, &CartesianPlotDock::niceExtendChanged);
	connect(ui.leRangePoints, &QLineEdit::textChanged, this, &CartesianPlotDock::rangePointsChanged);
	connect(ui.cbPlotColorMode, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &CartesianPlotDock::plotColorModeChanged);
	connect(ui.bColorMap, &QPushButton::clicked, this, &CartesianPlotDock::selectColorMap);

	// Layout
	connect(ui.sbLeft, QOverload<double>::of(&NumberSpinBox::valueChanged), this, &CartesianPlotDock::geometryChanged);
	connect(ui.sbTop, QOverload<double>::of(&NumberSpinBox::valueChanged), this, &CartesianPlotDock::geometryChanged);
	connect(ui.sbWidth, QOverload<double>::of(&NumberSpinBox::valueChanged), this, &CartesianPlotDock::geometryChanged);
	connect(ui.sbHeight, QOverload<double>::of(&NumberSpinBox::valueChanged), this, &CartesianPlotDock::geometryChanged);
	connect(ui.sbPaddingHorizontal, QOverload<double>::of(&NumberSpinBox::valueChanged), this, &CartesianPlotDock::horizontalPaddingChanged);
	connect(ui.sbPaddingVertical, QOverload<double>::of(&NumberSpinBox::valueChanged), this, &CartesianPlotDock::verticalPaddingChanged);
	connect(ui.sbPaddingRight, QOverload<double>::of(&NumberSpinBox::valueChanged), this, &CartesianPlotDock::rightPaddingChanged);
	connect(ui.sbPaddingBottom, QOverload<double>::of(&NumberSpinBox::valueChanged), this, &CartesianPlotDock::bottomPaddingChanged);
	connect(ui.cbPaddingSymmetric, &QCheckBox::toggled, this, &CartesianPlotDock::symmetricPaddingChanged);

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
	connect(ui.sbBorderCornerRadius, QOverload<double>::of(&NumberSpinBox::valueChanged), this, &CartesianPlotDock::borderCornerRadiusChanged);

	// Stacking
	connect(ui.leStackYOffset, &TimedLineEdit::textChanged, this, &CartesianPlotDock::stackYOffsetChanged);

	// theme and template handlers
	auto* frame = new QFrame(this);
	auto* layout = new QHBoxLayout(frame);
	layout->setContentsMargins(0, 0, 0, 0);

	// themes
	m_themeHandler = new ThemeHandler(this);
	layout->addWidget(m_themeHandler);
	connect(m_themeHandler, &ThemeHandler::loadThemeRequested, this, &CartesianPlotDock::loadTheme);
	connect(m_themeHandler, &ThemeHandler::info, this, &CartesianPlotDock::info);

	// templates for plot properties
	auto* templateHandler = new TemplateHandler(this, QLatin1String("CartesianPlot"));
	layout->addWidget(templateHandler);
	connect(templateHandler, &TemplateHandler::loadConfigRequested, this, &CartesianPlotDock::loadConfigFromTemplate);
	connect(templateHandler, &TemplateHandler::saveConfigRequested, this, &CartesianPlotDock::saveConfigAsTemplate);
	connect(templateHandler, &TemplateHandler::info, this, &CartesianPlotDock::info);

	// templates for plot definitions
	auto* tbExportTemplate = new QToolButton;
	int size = KIconLoader::global()->currentSize(KIconLoader::MainToolbar);
	tbExportTemplate->setIconSize(QSize(size, size));
	tbExportTemplate->setIcon(QIcon::fromTheme(QStringLiteral("document-save-as-template")));
	tbExportTemplate->setToolTip(i18n("Save current plot area definition as template"));
	connect(tbExportTemplate, &QToolButton::pressed, this, &CartesianPlotDock::exportPlotTemplate);
	layout->addWidget(tbExportTemplate);

	ui.verticalLayout->addWidget(frame);

	// TODO: activate the tab again once the functionality is implemented
	ui.tabWidget->removeTab(3);
}

void CartesianPlotDock::init() {
	// draw the icons for the border sides
	QPainter pa;
	int iconSize = 20;
	QPixmap pm(iconSize, iconSize);

	QPen pen(Qt::SolidPattern);
	const QColor& color = GuiTools::isDarkMode() ? Qt::white : Qt::black;
	pen.setColor(color);

	// left
	pm.fill(Qt::transparent);
	pa.begin(&pm);
	pa.setRenderHint(QPainter::Antialiasing); // must be set after every QPainter::begin()
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
	pa.end();
	ui.tbBorderTypeLeft->setIcon(pm);

	// top
	pm.fill(Qt::transparent);
	pa.begin(&pm);
	pa.setRenderHint(QPainter::Antialiasing);
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
	pa.setRenderHint(QPainter::Antialiasing);
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
	pa.setRenderHint(QPainter::Antialiasing);
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
	CONDITIONAL_LOCK_RETURN;
	m_plotList = list;
	m_plot = list.first();
	setAspects(list);

	QList<TextLabel*> labels;
	for (auto* plot : list)
		labels.append(plot->title());

	labelWidget->setLabels(labels);

	symmetricPaddingChanged(m_plot->symmetricPadding());

	ui.leName->setStyleSheet(QString());
	ui.leName->setToolTip(QString());

	// show the properties of the first plot
	this->load();

	// update active widgets
	m_themeHandler->setCurrentTheme(m_plot->theme());

	// Deactivate the geometry related widgets, if the worksheet layout is active.
	// activate otherwise and if the plot is a child of another plot
	auto* w = dynamic_cast<Worksheet*>(m_plot->parentAspect());
	if (w) {
		bool b = (w->layout() == Worksheet::Layout::NoLayout);
		ui.sbTop->setEnabled(b);
		ui.sbLeft->setEnabled(b);
		ui.sbWidth->setEnabled(b);
		ui.sbHeight->setEnabled(b);
		connect(w, &Worksheet::layoutChanged, this, &CartesianPlotDock::layoutChanged);
	} else {
		ui.sbTop->setEnabled(true);
		ui.sbLeft->setEnabled(true);
		ui.sbWidth->setEnabled(true);
		ui.sbHeight->setEnabled(true);
	}

	// SIGNALs/SLOTs
	connect(m_plot, &CartesianPlot::plotColorModeChanged, this, &CartesianPlotDock::plotPlotColorModeChanged);
	connect(m_plot, &CartesianPlot::plotColorMapChanged, this, &CartesianPlotDock::plotPlotColorMapChanged);
	connect(m_plot, &CartesianPlot::themeChanged, m_themeHandler, &ThemeHandler::setCurrentTheme);

	connect(m_plot, &CartesianPlot::rectChanged, this, &CartesianPlotDock::plotRectChanged);
	connect(m_plot, &CartesianPlot::rangeTypeChanged, this, &CartesianPlotDock::plotRangeTypeChanged);
	connect(m_plot, &CartesianPlot::rangeFirstValuesChanged, this, &CartesianPlotDock::plotRangeFirstValuesChanged);
	connect(m_plot, &CartesianPlot::rangeLastValuesChanged, this, &CartesianPlotDock::plotRangeLastValuesChanged);
	// TODO: check if needed
	connect(m_plot, &CartesianPlot::autoScaleChanged, this, &CartesianPlotDock::plotAutoScaleChanged);
	connect(m_plot, &CartesianPlot::minChanged, this, &CartesianPlotDock::plotMinChanged);
	connect(m_plot, &CartesianPlot::maxChanged, this, &CartesianPlotDock::plotMaxChanged);
	connect(m_plot, &CartesianPlot::rangeChanged, this, &CartesianPlotDock::plotRangeChanged);
	connect(m_plot, &CartesianPlot::scaleChanged, this, &CartesianPlotDock::plotScaleChanged);
	connect(m_plot, &CartesianPlot::rangeFormatChanged, this, &CartesianPlotDock::plotRangeFormatChanged);

	// range breaks
	// TODO: activate later once range breaking is implemented
	/*
	connect(m_plot, &CartesianPlot::xRangeBreakingEnabledChanged, this, &CartesianPlotDock::plotXRangeBreakingEnabledChanged);
	connect(m_plot, &CartesianPlot::xRangeBreaksChanged, this, &CartesianPlotDock::plotXRangeBreaksChanged);
	connect(m_plot, &CartesianPlot::yRangeBreakingEnabledChanged, this, &CartesianPlotDock::plotYRangeBreakingEnabledChanged);
	connect(m_plot, &CartesianPlot::yRangeBreaksChanged, this, &CartesianPlotDock::plotYRangeBreaksChanged);
	*/
	// Layout
	connect(m_plot, &CartesianPlot::horizontalPaddingChanged, this, &CartesianPlotDock::plotHorizontalPaddingChanged);
	connect(m_plot, &CartesianPlot::verticalPaddingChanged, this, &CartesianPlotDock::plotVerticalPaddingChanged);
	connect(m_plot, &CartesianPlot::rightPaddingChanged, this, &CartesianPlotDock::plotRightPaddingChanged);
	connect(m_plot, &CartesianPlot::bottomPaddingChanged, this, &CartesianPlotDock::plotBottomPaddingChanged);
	connect(m_plot, &CartesianPlot::symmetricPaddingChanged, this, &CartesianPlotDock::plotSymmetricPaddingChanged);

	// stacking
	connect(m_plot, &CartesianPlot::stackYOffsetChanged, this, &CartesianPlotDock::plotStackYOffsetChanged);
}

void CartesianPlotDock::activateTitleTab() {
	ui.tabWidget->setCurrentWidget(ui.tabTitle);
}

/*
 * updates the locale in the widgets. called when the application settings are changed.
 */
void CartesianPlotDock::updateLocale() {
	DEBUG(Q_FUNC_INFO)
	const auto numberLocale = QLocale();

	// update the QSpinBoxes
	ui.sbLeft->setLocale(numberLocale);
	ui.sbTop->setLocale(numberLocale);
	ui.sbWidth->setLocale(numberLocale);
	ui.sbHeight->setLocale(numberLocale);
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
		for (int row = 0; row < std::min(ui.twXRanges->rowCount(), m_plot->rangeCount(Dimension::X)); row++) {
			const auto& xRange = m_plot->range(Dimension::X, row);
			DEBUG(Q_FUNC_INFO << ", x range " << row << " auto scale = " << xRange.autoScale())
			if (m_plot->xRangeFormat(row) == RangeT::Format::Numeric) {
				// const int relPrec = xRange.relativePrecision();
				auto* sb = qobject_cast<NumberSpinBox*>(ui.twXRanges->cellWidget(row, TwRangesColumn::Min));
				sb->setLocale(numberLocale);
				sb = qobject_cast<NumberSpinBox*>(ui.twXRanges->cellWidget(row, TwRangesColumn::Max));
				sb->setLocale(numberLocale);

			} else {
				CELLWIDGET(Dimension::X, row, TwRangesColumn::Min, UTCDateTimeEdit, setMSecsSinceEpochUTC(xRange.start()));
				CELLWIDGET(Dimension::X, row, TwRangesColumn::Max, UTCDateTimeEdit, setMSecsSinceEpochUTC(xRange.end()));
				auto* dte = qobject_cast<UTCDateTimeEdit*>(ui.twXRanges->cellWidget(row, TwRangesColumn::Min));
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
		for (int row = 0; row < std::min(ui.twYRanges->rowCount(), m_plot->rangeCount(Dimension::Y)); row++) {
			const auto& yRange = m_plot->range(Dimension::Y, row);
			DEBUG(Q_FUNC_INFO << ", y range " << row << " auto scale = " << yRange.autoScale())
			if (m_plot->yRangeFormat(row) == RangeT::Format::Numeric) {
				// const int relPrec = yRange.relativePrecision();
				auto* sb = qobject_cast<NumberSpinBox*>(ui.twYRanges->cellWidget(row, TwRangesColumn::Min));
				sb->setLocale(numberLocale);
				sb = qobject_cast<NumberSpinBox*>(ui.twYRanges->cellWidget(row, TwRangesColumn::Max));
				sb->setLocale(numberLocale);
			} else {
				CELLWIDGET(Dimension::Y, row, TwRangesColumn::Min, UTCDateTimeEdit, setMSecsSinceEpochUTC(yRange.start()));
				CELLWIDGET(Dimension::Y, row, TwRangesColumn::Max, UTCDateTimeEdit, setMSecsSinceEpochUTC(yRange.end()));
				auto* dte = qobject_cast<UTCDateTimeEdit*>(ui.twYRanges->cellWidget(row, TwRangesColumn::Min));
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

	borderLineWidget->updateLocale();

	// update plot range list locale
	updatePlotRangeList();
}

void CartesianPlotDock::updateUnits() {
	const KConfigGroup group = Settings::group(QStringLiteral("Settings_General"));
	BaseDock::Units units = (BaseDock::Units)group.readEntry("Units", static_cast<int>(Units::Metric));
	if (units == m_units)
		return;

	m_units = units;
	CONDITIONAL_LOCK_RETURN;
	QString suffix;
	if (m_units == Units::Metric) {
		// convert from imperial to metric
		m_worksheetUnit = Worksheet::Unit::Centimeter;
		suffix = i18n("%v cm");
		ui.sbLeft->setValue(roundValue(ui.sbLeft->value() * GSL_CONST_CGS_INCH));
		ui.sbTop->setValue(roundValue(ui.sbTop->value() * GSL_CONST_CGS_INCH));
		ui.sbWidth->setValue(roundValue(ui.sbWidth->value() * GSL_CONST_CGS_INCH));
		ui.sbHeight->setValue(roundValue(ui.sbHeight->value() * GSL_CONST_CGS_INCH));
		ui.sbBorderCornerRadius->setValue(roundValue(ui.sbBorderCornerRadius->value() * GSL_CONST_CGS_INCH));
		ui.sbPaddingHorizontal->setValue(roundValue(ui.sbPaddingHorizontal->value() * GSL_CONST_CGS_INCH));
		ui.sbPaddingVertical->setValue(roundValue(ui.sbPaddingVertical->value() * GSL_CONST_CGS_INCH));
		ui.sbPaddingRight->setValue(roundValue(ui.sbPaddingRight->value() * GSL_CONST_CGS_INCH));
		ui.sbPaddingBottom->setValue(roundValue(ui.sbPaddingBottom->value() * GSL_CONST_CGS_INCH));
	} else {
		// convert from metric to imperial
		m_worksheetUnit = Worksheet::Unit::Inch;
		suffix = i18n("%v in");
		ui.sbLeft->setValue(roundValue(ui.sbLeft->value() / GSL_CONST_CGS_INCH));
		ui.sbTop->setValue(roundValue(ui.sbTop->value() / GSL_CONST_CGS_INCH));
		ui.sbWidth->setValue(roundValue(ui.sbWidth->value() / GSL_CONST_CGS_INCH));
		ui.sbHeight->setValue(roundValue(ui.sbHeight->value() / GSL_CONST_CGS_INCH));
		ui.sbBorderCornerRadius->setValue(roundValue(ui.sbBorderCornerRadius->value() / GSL_CONST_CGS_INCH));
		ui.sbPaddingHorizontal->setValue(roundValue(ui.sbPaddingHorizontal->value() / GSL_CONST_CGS_INCH));
		ui.sbPaddingVertical->setValue(roundValue(ui.sbPaddingVertical->value() / GSL_CONST_CGS_INCH));
		ui.sbPaddingRight->setValue(roundValue(ui.sbPaddingRight->value() / GSL_CONST_CGS_INCH));
		ui.sbPaddingBottom->setValue(roundValue(ui.sbPaddingBottom->value() / GSL_CONST_CGS_INCH));
	}

	KLocalization::setupSpinBoxFormatString(ui.sbLeft, ki18nc("@label:spinbox Suffix for the left spacing", qPrintable(suffix)));
	KLocalization::setupSpinBoxFormatString(ui.sbTop, ki18nc("@label:spinbox Suffix for the top spacing", qPrintable(suffix)));
	KLocalization::setupSpinBoxFormatString(ui.sbWidth, ki18nc("@label:spinbox Suffix for the width", qPrintable(suffix)));
	KLocalization::setupSpinBoxFormatString(ui.sbHeight, ki18nc("@label:spinbox Suffix for the height", qPrintable(suffix)));
	KLocalization::setupSpinBoxFormatString(ui.sbBorderCornerRadius, ki18nc("@label:spinbox Suffix for the border corner radius", qPrintable(suffix)));
	KLocalization::setupSpinBoxFormatString(ui.sbPaddingHorizontal, ki18nc("@label:spinbox Suffix for the horizontal padding", qPrintable(suffix)));
	KLocalization::setupSpinBoxFormatString(ui.sbPaddingVertical, ki18nc("@label:spinbox Suffix for the vertical padding", qPrintable(suffix)));
	KLocalization::setupSpinBoxFormatString(ui.sbPaddingRight, ki18nc("@label:spinbox Suffix for the right padding", qPrintable(suffix)));
	KLocalization::setupSpinBoxFormatString(ui.sbPaddingBottom, ki18nc("@label:spinbox Suffix for the bottom padding", qPrintable(suffix)));

	labelWidget->updateUnits();
}

void CartesianPlotDock::updateRangeList(const Dimension dim) {
	if (!m_plot)
		return;

	const auto dir_str = CartesianCoordinateSystem::dimensionToString(dim);

	QTableWidget* tw = nullptr;
	QLabel* l = nullptr;
	QToolButton* tb = nullptr;
	switch (dim) {
	case Dimension::X:
		tw = ui.twXRanges;
		l = ui.lXRanges;
		tb = ui.tbRemoveXRange;
		break;
	case Dimension::Y:
		tw = ui.twYRanges;
		l = ui.lYRanges;
		tb = ui.tbRemoveYRange;
		break;
	}

	tw->horizontalHeader()->setSectionResizeMode(TwRangesColumn::Automatic, QHeaderView::ResizeMode::ResizeToContents);
	tw->horizontalHeader()->setSectionResizeMode(TwRangesColumn::Format, QHeaderView::ResizeMode::ResizeToContents);
	tw->horizontalHeader()->setSectionResizeMode(TwRangesColumn::Min, QHeaderView::ResizeMode::Stretch);
	tw->horizontalHeader()->setSectionResizeMode(TwRangesColumn::Max, QHeaderView::ResizeMode::Stretch);
	tw->horizontalHeader()->setSectionResizeMode(TwRangesColumn::Scale, QHeaderView::ResizeMode::ResizeToContents);
	tw->horizontalHeader()->setStretchLastSection(false);

	const int rangeCount = m_plot->rangeCount(dim);
	DEBUG(Q_FUNC_INFO << ", " << dir_str.toStdString() << " range count = " << rangeCount)

	if (rangeCount > 1)
		l->setText(i18n("%1-Ranges:", dir_str.toUpper()));
	else
		l->setText(i18n("%1-Range:", dir_str.toUpper()));
	tw->setRowCount(rangeCount);
	for (int i = 0; i < rangeCount; i++) {
		const auto& r = m_plot->range(dim, i);
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
		connect(chk, &QCheckBox::toggled, [this, chk, dim](bool checked) {
			const int rangeIndex = chk->property("row").toInt();
			this->autoScaleChanged(dim, rangeIndex, checked);
		});

		// format
		auto* cb = new ComboBoxIgnoreWheel(tw);
		cb->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
		cb->setFrame(false);
		cb->addItem(i18n("Numeric"));
		cb->addItem(AbstractColumn::columnModeString(AbstractColumn::ColumnMode::DateTime));
		cb->setProperty("row", i);
		cb->setCurrentIndex(static_cast<int>(format));
		tw->setCellWidget(i, TwRangesColumn::Format, cb);
		connect(cb, QOverload<int>::of(&QComboBox::currentIndexChanged), [this, dim, cb](int index) {
			this->rangeFormatChanged(cb, dim, index);
		});

		// start/end (values set in updateLocale())
		if (format == RangeT::Format::Numeric) {
			auto* sb = new NumberSpinBox(tw);
			sb->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
			sb->setProperty("row", i);
			sb->setFeedback(true);
			sb->setValue(r.start());
			tw->setCellWidget(i, TwRangesColumn::Min, sb);
			connect(sb, QOverload<double>::of(&NumberSpinBox::valueChanged), [this, dim, sb](double value) {
				this->minChanged(dim, sb->property("row").toInt(), value);
			});
			sb = new NumberSpinBox(tw);
			sb->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
			sb->setProperty("row", i);
			sb->setFeedback(true);
			sb->setValue(r.end());
			tw->setCellWidget(i, TwRangesColumn::Max, sb);
			connect(sb, QOverload<double>::of(&NumberSpinBox::valueChanged), [this, dim, sb](double value) {
				this->maxChanged(dim, sb->property("row").toInt(), value);
			});
		} else {
			auto* dte = new UTCDateTimeEdit(tw);
			dte->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
			dte->setDisplayFormat(m_plot->rangeDateTimeFormat(dim, i));
			dte->setMSecsSinceEpochUTC(r.start());
			dte->setWrapping(true);
			tw->setCellWidget(i, TwRangesColumn::Min, dte);
			dte->setProperty("row", i);
			connect(dte, &UTCDateTimeEdit::mSecsSinceEpochUTCChanged, [this, dim, dte](qint64 dateTime) {
				this->minDateTimeChanged(dte, dim, dateTime);
			});

			dte = new UTCDateTimeEdit(tw);
			dte->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
			dte->setDisplayFormat(m_plot->rangeDateTimeFormat(dim, i));
			dte->setMSecsSinceEpochUTC(r.end());
			dte->setWrapping(true);
			tw->setCellWidget(i, TwRangesColumn::Max, dte);
			dte->setProperty("row", i);
			connect(dte, &UTCDateTimeEdit::mSecsSinceEpochUTCChanged, [this, dim, dte](qint64 dateTime) {
				this->maxDateTimeChanged(dte, dim, dateTime);
			});
		}

		// scale
		cb = new ComboBoxIgnoreWheel(tw);
		cb->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
		cb->setFrame(false);
		// TODO: -> updateLocale()
		for (const auto& name : RangeT::scaleNames)
			cb->addItem(name.toString());

		cb->setCurrentIndex(static_cast<int>(scale));
		cb->setProperty("row", i);
		tw->setCellWidget(i, TwRangesColumn::Scale, cb);
		connect(cb, QOverload<int>::of(&QComboBox::currentIndexChanged), [this, dim, cb](int index) {
			this->scaleChanged(cb, dim, index);
		});
	}

	// adjust the size of the table widget
	tw->resizeColumnToContents(0);
	tw->resizeColumnToContents(1);
	tw->resizeColumnToContents(2);
	tw->resizeColumnToContents(3);
	int height = tw->verticalHeader()->sectionSize(0) * tw->verticalHeader()->count();
	height +=
		tw->horizontalHeader()->height() + 2; // TODO: offset of 2 is required, otherwise too much is cut off. figure out who to get this offset from QStyle
	tw->setFixedHeight(height);

	tb->setEnabled(rangeCount > 1 ? true : false);

	if (m_updateUI) {
		updateLocale(); // fill values
		updatePlotRangeList(); // update x ranges used in plot ranges
	}

	// enable/disable widgets
	for (int i = 0; i < rangeCount; i++) {
		const bool checked{m_plot->range(dim, i).autoScale()};
		CELLWIDGET(dim, i, TwRangesColumn::Format, QComboBox, setEnabled(!checked));
		CELLWIDGET(dim, i, TwRangesColumn::Min, QWidget, setEnabled(!checked));
		CELLWIDGET(dim, i, TwRangesColumn::Max, QWidget, setEnabled(!checked));
	}
}

QString generatePlotRangeString(int rangeCount, int rangeIndex, const Range<double>& range) {
	if (rangeCount > 1)
		return QString::number(rangeIndex + 1) + QStringLiteral(" : ") + range.toLocaleString();
	return range.toLocaleString();
}

/*!
 * \brief CartesianPlotDock::updatePlotRangeListValues
 * Updates the plot range values. This is much faster than updatePlotRangeList()
 * \param dim
 * \param rangeIndex
 */
void CartesianPlotDock::updatePlotRangeListValues(const Dimension dim, int rangeIndex) {
	auto column = TwPlotRangesColumn::XRange;
	switch (dim) {
	case Dimension::X:
		break;
	case Dimension::Y:
		column = TwPlotRangesColumn::YRange;
		break;
	}

	for (auto cSystemIndex = 0; cSystemIndex < ui.twPlotRanges->rowCount(); cSystemIndex++) {
		auto* cb = dynamic_cast<QComboBox*>(ui.twPlotRanges->cellWidget(cSystemIndex, column));
		if (cb) {
			for (auto itemIndex = 0; itemIndex < cb->count(); itemIndex++) {
				const auto data = cb->itemData(itemIndex);
				if (data.isValid()) {
					bool ok = true;
					const auto rangeIndexComboBox = data.toInt(&ok);
					Q_ASSERT(ok);
					if (rangeIndexComboBox == rangeIndex)
						cb->setItemText(itemIndex, generatePlotRangeString(m_plot->rangeCount(dim), rangeIndex, m_plot->range(dim, rangeIndex)));
				}
			}
		}
	}
}

/*!
 * \brief CartesianPlotDock::updatePlotRangeList
 * update plot ranges in list by recreating all widgets. This function is really slow, use it only for non-critical workflows which are called not that often.
 * Prefer updatePlotRangeListValues()
 */
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
	ui.twPlotRanges->horizontalHeader()->setSectionResizeMode(TwPlotRangesColumn::XRange, QHeaderView::ResizeMode::Stretch);
	ui.twPlotRanges->horizontalHeader()->setSectionResizeMode(TwPlotRangesColumn::YRange, QHeaderView::ResizeMode::Stretch);
	ui.twPlotRanges->horizontalHeader()->setSectionResizeMode(TwPlotRangesColumn::Default, QHeaderView::ResizeMode::ResizeToContents);
	ui.twPlotRanges->horizontalHeader()->setSectionResizeMode(TwPlotRangesColumn::Name, QHeaderView::ResizeMode::ResizeToContents);
	ui.twPlotRanges->horizontalHeader()->setStretchLastSection(false);
	for (int i = 0; i < cSystemCount; i++) {
		auto* cSystem{m_plot->coordinateSystem(i)};
		const int xIndex{cSystem->index(Dimension::X)}, yIndex{cSystem->index(Dimension::Y)};
		const auto& xRange = m_plot->range(Dimension::X, xIndex);
		const auto& yRange = m_plot->range(Dimension::Y, yIndex);

		DEBUG(Q_FUNC_INFO << ", coordinate system " << i + 1 << " : xIndex = " << xIndex << ", yIndex = " << yIndex)
		DEBUG(Q_FUNC_INFO << ", x range = " << xRange.toStdString() << ", auto scale = " << xRange.autoScale())
		DEBUG(Q_FUNC_INFO << ", y range = " << yRange.toStdString() << ", auto scale = " << yRange.autoScale())

		// X-range
		auto* cb = new ComboBoxIgnoreWheel(ui.twPlotRanges);
		cb->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
		cb->setEditable(true); // to have a line edit
		cb->lineEdit()->setReadOnly(true);
		cb->lineEdit()->setAlignment(Qt::AlignHCenter);
		const auto xRangeCount = m_plot->rangeCount(Dimension::X);
		if (xRangeCount > 1) {
			for (int index = 0; index < xRangeCount; index++)
				cb->addItem(generatePlotRangeString(xRangeCount, index, m_plot->range(Dimension::X, index)), QVariant::fromValue(index));
			cb->setCurrentIndex(xIndex);
			cb->setProperty("row", i);
			connect(cb, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &CartesianPlotDock::PlotRangeXChanged);
		} else {
			cb->addItem(generatePlotRangeString(xRangeCount, 0, xRange), QVariant::fromValue(0));
			cb->setStyleSheet(QStringLiteral("QComboBox::drop-down {border-width: 0px;}")); // hide arrow if there is only one range
		}
		ui.twPlotRanges->setCellWidget(i, TwPlotRangesColumn::XRange, cb);

		// Y-range
		cb = new ComboBoxIgnoreWheel(ui.twPlotRanges);
		cb->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
		cb->setEditable(true); // to have a line edit
		cb->lineEdit()->setReadOnly(true);
		cb->lineEdit()->setAlignment(Qt::AlignHCenter);
		const auto yRangeCount = m_plot->rangeCount(Dimension::Y);
		if (yRangeCount > 1) {
			for (int index = 0; index < yRangeCount; index++)
				cb->addItem(generatePlotRangeString(yRangeCount, index, m_plot->range(Dimension::Y, index)));
			cb->setCurrentIndex(yIndex);
			cb->setProperty("row", i);
			connect(cb, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &CartesianPlotDock::PlotRangeYChanged);
		} else {
			cb->addItem(generatePlotRangeString(yRangeCount, 0, yRange));
			cb->setStyleSheet(QStringLiteral("QComboBox::drop-down {border-width: 0px;}")); // hide arrow if there is only one range
		}
		ui.twPlotRanges->setCellWidget(i, TwPlotRangesColumn::YRange, cb);

		// Name
		auto* le = new TimedLineEdit(ui.twPlotRanges);
		le->setText(cSystem->name());
		ui.twPlotRanges->setCellWidget(i, TwPlotRangesColumn::Name, le);
		connect(le, &TimedLineEdit::textEdited, this, [=]() {
			cSystem->setName(le->text());
		});
	}

	// adjust the size of the table widget
	ui.twPlotRanges->resizeColumnToContents(0);
	ui.twPlotRanges->resizeColumnToContents(1);
	auto* tw = ui.twPlotRanges;
	int height = tw->verticalHeader()->sectionSize(0) * tw->verticalHeader()->count();
	height +=
		tw->horizontalHeader()->height() + 2; // TODO: offset of 2 is required, otherwise too much is cut off. figure out who to get this offset from QStyle
	tw->setFixedHeight(height);

	if (m_bgDefaultPlotRange) {
		for (auto* button : m_bgDefaultPlotRange->buttons())
			m_bgDefaultPlotRange->removeButton(button);
	} else {
		m_bgDefaultPlotRange = new QButtonGroup(this);
		connect(m_bgDefaultPlotRange, QOverload<QAbstractButton*>::of(&QButtonGroup::buttonClicked), this, &CartesianPlotDock::defaultPlotRangeChanged);
	}
	for (int i = 0; i < cSystemCount; i++) {
		auto* rb = new QRadioButton();
		rb->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
		if (i == m_plot->defaultCoordinateSystemIndex())
			rb->setChecked(true);
		m_bgDefaultPlotRange->addButton(rb);
		rb->setStyleSheet(QStringLiteral("margin-left:50%; margin-right:50%;")); // center button
		ui.twPlotRanges->setCellWidget(i, TwPlotRangesColumn::Default, rb);
		m_bgDefaultPlotRange->setId(rb, i);
	}

	ui.tbRemovePlotRange->setEnabled(cSystemCount > 1 ? true : false);
}

void CartesianPlotDock::retranslateUi() {
	CONDITIONAL_LOCK_RETURN;

	// data range types
	ui.cbRangeType->clear();
	ui.cbRangeType->addItem(i18n("Free"));
	ui.cbRangeType->addItem(i18n("Last Points"));
	ui.cbRangeType->addItem(i18n("First Points"));

	// TODO: activa later once scale breaking is supported
	// scale breakings
	ui.cbXBreakStyle->addItem(i18n("Simple"));
	ui.cbXBreakStyle->addItem(i18n("Vertical"));
	ui.cbXBreakStyle->addItem(i18n("Sloped"));

	ui.cbYBreakStyle->addItem(i18n("Simple"));
	ui.cbYBreakStyle->addItem(i18n("Vertical"));
	ui.cbYBreakStyle->addItem(i18n("Sloped"));

	ui.cbPlotColorMode->clear();
	ui.cbPlotColorMode->addItem(i18n("Theme"));
	ui.cbPlotColorMode->addItem(i18n("Color Map"));

	// tooltip texts
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

	msg = i18n(
		"Defines which colors to use for plots:"
		"<ul>"
		"<li>Theme - colors are taken from the current theme apply to plot area</li>"
		"<li>Color Map - colors are taken from the specified color map</li>"
		"</ul>");
	ui.lPlotColorMode->setToolTip(msg);
	ui.cbPlotColorMode->setToolTip(msg);
}

//************************************************************
//**** SLOTs for changes triggered in CartesianPlotDock ******
//************************************************************
// "General"-tab
void CartesianPlotDock::plotColorModeChanged(int index) {
	const auto mode = static_cast<CartesianPlot::PlotColorMode>(index);
	const bool visible = (mode == CartesianPlot::PlotColorMode::ColorMap);
	ui.lColorMap->setVisible(visible);
	ui.frameColorMap->setVisible(visible);

	if (visible) {
		const auto& name = m_plot->plotColorMap();
		ui.lColorMapName->setText(name);
		ui.lColorMapPreview->setPixmap(ColorMapsManager::instance()->previewPixmap(name));
	}

	CONDITIONAL_LOCK_RETURN;
	for (auto* plot : m_plotList)
		plot->setPlotColorMode(mode);
}

void CartesianPlotDock::selectColorMap() {
	auto* dlg = new ColorMapsDialog(this);
	if (dlg->exec() == QDialog::Accepted)
		plotColorMapChanged(dlg->name());
	delete dlg;
}

void CartesianPlotDock::plotColorMapChanged(const QString& name) {
	if (m_plot->plotColorMode() == CartesianPlot::PlotColorMode::ColorMap) {
		ui.lColorMapName->setText(name);
		ui.lColorMapPreview->setPixmap(ColorMapsManager::instance()->previewPixmap(name));
	}

	CONDITIONAL_LOCK_RETURN;
	for (auto* plot : m_plotList)
		plot->setPlotColorMap(name);
}

void CartesianPlotDock::rangeTypeChanged(int index) {
	auto type = static_cast<CartesianPlot::RangeType>(index);
	if (type == CartesianPlot::RangeType::Free) {
		ui.lRangePoints->hide();
		ui.leRangePoints->hide();
	} else {
		ui.lRangePoints->show();
		ui.leRangePoints->show();

		if (type == CartesianPlot::RangeType::First)
			ui.leRangePoints->setText(QLocale().toString(m_plot->rangeFirstValues()));
		else
			ui.leRangePoints->setText(QLocale().toString(m_plot->rangeLastValues()));
	}

	CONDITIONAL_LOCK_RETURN;
	for (auto* plot : m_plotList)
		plot->setRangeType(type);
}

void CartesianPlotDock::niceExtendChanged(bool checked) {
	CONDITIONAL_LOCK_RETURN;

	for (auto* plot : m_plotList)
		plot->setNiceExtend(checked);
}

void CartesianPlotDock::rangePointsChanged(const QString& text) {
	CONDITIONAL_LOCK_RETURN;

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

void CartesianPlotDock::autoScaleChanged(const Dimension dim, const int rangeIndex, bool state) {
	DEBUG(Q_FUNC_INFO << ", state = " << state)
	DEBUG(Q_FUNC_INFO << ", range index: " << rangeIndex)
	CONDITIONAL_LOCK_RETURN;

	autoScaleRange(dim, rangeIndex, state);
}

void CartesianPlotDock::autoScaleRange(const Dimension dim, const int index, bool checked) {
	DEBUG(Q_FUNC_INFO << ", index = " << index << " checked = " << checked)

	auto* treewidget = ui.twXRanges;
	Dimension dim_other = Dimension::Y;
	switch (dim) {
	case Dimension::X:
		break;
	case Dimension::Y:
		dim_other = Dimension::X;
		treewidget = ui.twYRanges;
		break;
	}

	if (treewidget->cellWidget(index, TwRangesColumn::Format)) {
		CELLWIDGET(dim, index, TwRangesColumn::Format, QComboBox, setEnabled(!checked));
		CELLWIDGET(dim, index, TwRangesColumn::Min, QWidget, setEnabled(!checked));
		CELLWIDGET(dim, index, TwRangesColumn::Max, QWidget, setEnabled(!checked));
	}

	for (auto* plot : m_plotList) {
		bool retransform = true; // must be true, because in enableAutoScale scaleAutoX will be already called
		plot->enableAutoScale(dim, index, checked, true);
		DEBUG(Q_FUNC_INFO << " new auto scale = " << plot->range(dim, index).autoScale())
		if (checked) { // && index == plot->defaultCoordinateSystem()->index(Dimension::Y)
			retransform |= plot->scaleAuto(dim, index, true);

			for (int i = 0; i < plot->coordinateSystemCount(); i++) {
				auto cSystem = plot->coordinateSystem(i);
				if (cSystem->index(dim) == index) {
					if (plot->autoScale(dim_other, cSystem->index(dim_other)))
						retransform |= plot->scaleAuto(dim_other, cSystem->index(dim_other), false);
				}
			}
		}
		if (retransform)
			plot->WorksheetElementContainer::retransform();
	}
	updateRangeList(dim); // see range changes
}

void CartesianPlotDock::minChanged(const Dimension dim, const int index, double min) {
	DEBUG(Q_FUNC_INFO << ", value = " << min);
	CONDITIONAL_RETURN_NO_LOCK;

	// selected x/y range
	DEBUG(Q_FUNC_INFO << ", x range index: " << index)
	for (auto* plot : m_plotList)
		if (!qFuzzyCompare(min, plot->range(dim, index).start()))
			plot->setMin(dim, index, min);
}

void CartesianPlotDock::maxChanged(const Dimension dim, const int index, double max) {
	DEBUG(Q_FUNC_INFO << ", value = " << max);
	CONDITIONAL_RETURN_NO_LOCK;

	// selected x/y range
	for (auto* plot : m_plotList) {
		if (!qFuzzyCompare(max, plot->range(dim, index).end()))
			plot->setMax(dim, index, max);
	}
}

void CartesianPlotDock::minDateTimeChanged(const QObject* sender, const Dimension dim, qint64 value) {
	CONDITIONAL_LOCK_RETURN;

	// selected x range
	const int index{sender->property("row").toInt()};
	DEBUG(Q_FUNC_INFO << ", x range index: " << index)
	for (auto* plot : m_plotList)
		plot->setMin(dim, index, value);
	updatePlotRangeList();
}

void CartesianPlotDock::maxDateTimeChanged(const QObject* sender, const Dimension dim, qint64 value) {
	CONDITIONAL_LOCK_RETURN;

	// selected x range
	const int index{sender->property("row").toInt()};
	DEBUG(Q_FUNC_INFO << ", x range index: " << index)
	for (auto* plot : m_plotList)
		plot->setMax(dim, index, value);
	updatePlotRangeList();
}

/*!
 *  called on scale changes (linear, log) for the x-/y-axis
 */
void CartesianPlotDock::scaleChanged(const QObject* sender, const Dimension dim, int index) {
	CONDITIONAL_LOCK_RETURN;

	const int rangeIndex{sender->property("row").toInt()};
	DEBUG(Q_FUNC_INFO << ", range " << rangeIndex << " scale changed to " << ENUM_TO_STRING(RangeT, Scale, index))
	const auto scale{static_cast<RangeT::Scale>(index)};
	for (auto* plot : m_plotList)
		plot->setRangeScale(dim, rangeIndex, scale);
	updateRangeList(dim);
}

void CartesianPlotDock::rangeFormatChanged(const QObject* sender, const Dimension dim, int index) {
	const int rangeIndex{sender->property("row").toInt()};
	DEBUG(Q_FUNC_INFO << ", x range " << rangeIndex + 1 << " format = " << index)

	CONDITIONAL_LOCK_RETURN;

	const auto format{static_cast<RangeT::Format>(index)};
	for (auto* plot : m_plotList) {
		DEBUG(Q_FUNC_INFO << ", set format of range " << rangeIndex + 1 << " to " << static_cast<int>(format))
		plot->setRangeFormat(dim, rangeIndex, format);
	}
	updateRangeList(dim);
}

void CartesianPlotDock::addXRange() {
	if (!m_plot)
		return;

	DEBUG(Q_FUNC_INFO << ", current x range count = " << m_plot->rangeCount(Dimension::X))

	m_plot->addXRange();
	updateRangeList(Dimension::X);
}
void CartesianPlotDock::addYRange() {
	if (!m_plot)
		return;

	DEBUG(Q_FUNC_INFO << ", current y range count = " << m_plot->rangeCount(Dimension::Y))

	m_plot->addYRange();
	updateRangeList(Dimension::Y);
}

void CartesianPlotDock::removeXRange() {
	removeRange(Dimension::X);
}

void CartesianPlotDock::removeYRange() {
	removeRange(Dimension::Y);
}

void CartesianPlotDock::removeRange(const Dimension dim) {
	if (!m_plot)
		return;

	QTableWidget* treewidget = ui.twXRanges;
	switch (dim) {
	case Dimension::X:
		break;
	case Dimension::Y:
		treewidget = ui.twYRanges;
		break;
	}

	int currentRow{treewidget->currentRow()};
	QDEBUG(Q_FUNC_INFO << ", current range = " << currentRow)
	if (currentRow < 0 || currentRow > m_plot->rangeCount(dim)) {
		DEBUG(Q_FUNC_INFO << ", no current range")
		currentRow = m_plot->rangeCount(dim) - 1;
	}
	QDEBUG(Q_FUNC_INFO << ", removing range " << currentRow)

	// check plot ranges using range to remove
	const int cSystemCount{m_plot->coordinateSystemCount()};
	DEBUG(Q_FUNC_INFO << ", nr of cSystems = " << cSystemCount)
	QString msg;
	for (int i{0}; i < cSystemCount; i++) {
		const auto* cSystem{m_plot->coordinateSystem(i)};

		if (cSystem->index(dim) == currentRow) {
			if (msg.size() > 0)
				msg += QStringLiteral(", ");
			msg += QString::number(i + 1);
		}
	}

	if (msg.size() > 0) {
		DEBUG(Q_FUNC_INFO << ", range used in plot range " << STDSTRING(msg))

		auto status = KMessageBox::warningTwoActions(
			this,
			i18n("%1 range %2 is used in plot range %3. ", CartesianCoordinateSystem::dimensionToString(dim).toUpper(), currentRow + 1, msg)
				+ i18n("Really remove it?"),
			QString(),
			KStandardGuiItem::remove(),
			KStandardGuiItem::cancel());
		if (status == KMessageBox::SecondaryAction)
			return;
		else {
			// reset x ranges of cSystems using the range to be removed
			for (int i{0}; i < cSystemCount; i++) {
				auto* cSystem{m_plot->coordinateSystem(i)};

				if (cSystem->index(dim) == currentRow)
					m_plot->setCoordinateSystemRangeIndex(i, dim, 0); // first range
				else if (cSystem->index(dim) > currentRow)
					m_plot->setCoordinateSystemRangeIndex(i, dim, cSystem->index(dim) - 1);
			}
		}
	}

	m_plot->removeRange(dim, currentRow);
	updateRangeList(dim);
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

			auto status =
				KMessageBox::warningTwoActions(this,
											   i18n("Plot range %1 is used by element \"%2\". ", currentRow + 1, element->name()) + i18n("Really remove it?"),
											   QString(),
											   KStandardGuiItem::remove(),
											   KStandardGuiItem::cancel());
			if (status == KMessageBox::SecondaryAction)
				return;
			else
				element->setCoordinateSystemIndex(0); // reset
		}
	}

	m_plot->removeCoordinateSystem(currentRow);
	updatePlotRangeList();
	m_plot->retransform(); // update plot and elements
}

void CartesianPlotDock::PlotRangeChanged(const int plotRangeIndex, const Dimension dim, const int index) {
	const std::string dimStr = CartesianCoordinateSystem::dimensionToString(dim).toStdString();
	DEBUG(Q_FUNC_INFO << ", Set " << dimStr << " range of plot range " << plotRangeIndex + 1 << " to " << index + 1)
	auto* cSystem{m_plot->coordinateSystem(plotRangeIndex)};
	const auto indexOld = cSystem->index(dim);
	m_plot->setCoordinateSystemRangeIndex(plotRangeIndex, dim, index);

	m_plot->setRangeDirty(dim, index, true);
	m_plot->setRangeDirty(dim, indexOld, true);

	// auto scale x range when on auto scale (now that it is used)
	if (m_plot->range(dim, index).autoScale()) {
		autoScaleRange(dim, index, true);
		updateRangeList(dim);
	}

	for (auto* axis : m_plot->children<Axis>()) {
		const int cSystemIndex{axis->coordinateSystemIndex()};
		DEBUG(Q_FUNC_INFO << ", Axis \"" << STDSTRING(axis->name()) << "\" cSystem index = " << cSystemIndex)
		if (cSystemIndex == plotRangeIndex) {
			DEBUG(Q_FUNC_INFO << ", Plot range used in axis \"" << STDSTRING(axis->name()) << "\" has changed")
			if (axis->rangeType() == Axis::RangeType::Auto
				&& ((dim == Dimension::X && axis->orientation() == Axis::Orientation::Horizontal)
					|| (dim == Dimension::Y && axis->orientation() == Axis::Orientation::Vertical))) {
				DEBUG(Q_FUNC_INFO << ", set " << dimStr << " range of axis to " << m_plot->range(dim, index).toStdString())
				axis->setRange(m_plot->range(dim, index));
			}
		}
	}

	// Retransform axes and all other elements, because the coordinatesystem changed
	m_plot->WorksheetElementContainer::retransform();
}

/*
 * Called when x/y range of plot range in plot range list changes
 */
void CartesianPlotDock::PlotRangeXChanged(const int index) {
	PlotRangeChanged(sender()->property("row").toInt(), Dimension::X, index);
}
void CartesianPlotDock::PlotRangeYChanged(const int index) {
	PlotRangeChanged(sender()->property("row").toInt(), Dimension::Y, index);
}

// "Layout" tab
void CartesianPlotDock::geometryChanged() {
	CONDITIONAL_RETURN_NO_LOCK;

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

	CONDITIONAL_LOCK_RETURN;

	for (auto* plot : m_plotList)
		plot->setSymmetricPadding(checked);

	if (checked) {
		rightPaddingChanged(ui.sbPaddingHorizontal->value());
		bottomPaddingChanged(ui.sbPaddingVertical->value());
	}
}

void CartesianPlotDock::horizontalPaddingChanged(double value) {
	CONDITIONAL_RETURN_NO_LOCK;

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
	CONDITIONAL_RETURN_NO_LOCK;

	double padding = Worksheet::convertToSceneUnits(value, m_worksheetUnit);
	for (auto* plot : m_plotList)
		plot->setRightPadding(padding);
}

void CartesianPlotDock::verticalPaddingChanged(double value) {
	CONDITIONAL_RETURN_NO_LOCK;

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
	CONDITIONAL_RETURN_NO_LOCK;

	double padding = Worksheet::convertToSceneUnits(value, m_worksheetUnit);
	for (auto* plot : m_plotList)
		plot->setBottomPadding(padding);
}

// "Range Breaks"-tab

// x-range breaks
void CartesianPlotDock::toggleXBreak(bool b) {
	ui.frameXBreakEdit->setEnabled(b);
	ui.leXBreakStart->setEnabled(b);
	ui.leXBreakEnd->setEnabled(b);
	ui.sbXBreakPosition->setEnabled(b);
	ui.cbXBreakStyle->setEnabled(b);

	CONDITIONAL_LOCK_RETURN;

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
	CONDITIONAL_LOCK_RETURN;

	if (index == -1)
		return;

	const auto numberLocale = QLocale();
	const CartesianPlot::RangeBreak rangeBreak = m_plot->xRangeBreaks().list.at(index);
	QString str = std::isnan(rangeBreak.range.start()) ? QString() : numberLocale.toString(rangeBreak.range.start());
	ui.leXBreakStart->setText(str);
	str = std::isnan(rangeBreak.range.end()) ? QString() : numberLocale.toString(rangeBreak.range.end());
	ui.leXBreakEnd->setText(str);
	ui.sbXBreakPosition->setValue(rangeBreak.position * 100);
	ui.cbXBreakStyle->setCurrentIndex((int)rangeBreak.style);
}

void CartesianPlotDock::xBreakStartChanged() {
	CONDITIONAL_LOCK_RETURN;

	int index = ui.cbXBreak->currentIndex();
	CartesianPlot::RangeBreaks breaks = m_plot->xRangeBreaks();
	breaks.list[index].range.start() = ui.leXBreakStart->text().toDouble();
	breaks.lastChanged = index;

	for (auto* plot : m_plotList)
		plot->setXRangeBreaks(breaks);
}

void CartesianPlotDock::xBreakEndChanged() {
	CONDITIONAL_LOCK_RETURN;

	int index = ui.cbXBreak->currentIndex();
	CartesianPlot::RangeBreaks breaks = m_plot->xRangeBreaks();
	breaks.list[index].range.end() = ui.leXBreakEnd->text().toDouble();
	breaks.lastChanged = index;

	for (auto* plot : m_plotList)
		plot->setXRangeBreaks(breaks);
}

void CartesianPlotDock::xBreakPositionChanged(int value) {
	CONDITIONAL_LOCK_RETURN;

	int index = ui.cbXBreak->currentIndex();
	CartesianPlot::RangeBreaks breaks = m_plot->xRangeBreaks();
	breaks.list[index].position = (double)value / 100.;
	breaks.lastChanged = index;

	for (auto* plot : m_plotList)
		plot->setXRangeBreaks(breaks);
}

void CartesianPlotDock::xBreakStyleChanged(int styleIndex) {
	CONDITIONAL_LOCK_RETURN;

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

	CONDITIONAL_LOCK_RETURN;

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
	CONDITIONAL_LOCK_RETURN;

	if (index == -1)
		return;

	const auto numberLocale = QLocale();
	const CartesianPlot::RangeBreak rangeBreak = m_plot->yRangeBreaks().list.at(index);
	QString str = std::isnan(rangeBreak.range.start()) ? QString() : numberLocale.toString(rangeBreak.range.start());
	ui.leYBreakStart->setText(str);
	str = std::isnan(rangeBreak.range.end()) ? QString() : numberLocale.toString(rangeBreak.range.end());
	ui.leYBreakEnd->setText(str);
	ui.sbYBreakPosition->setValue(rangeBreak.position * 100);
	ui.cbYBreakStyle->setCurrentIndex((int)rangeBreak.style);
}

void CartesianPlotDock::yBreakStartChanged() {
	CONDITIONAL_LOCK_RETURN;

	int index = ui.cbYBreak->currentIndex();
	CartesianPlot::RangeBreaks breaks = m_plot->yRangeBreaks();
	breaks.list[index].range.start() = ui.leYBreakStart->text().toDouble();
	breaks.lastChanged = index;

	for (auto* plot : m_plotList)
		plot->setYRangeBreaks(breaks);
}

void CartesianPlotDock::yBreakEndChanged() {
	CONDITIONAL_LOCK_RETURN;

	int index = ui.cbYBreak->currentIndex();
	CartesianPlot::RangeBreaks breaks = m_plot->yRangeBreaks();
	breaks.list[index].range.end() = ui.leYBreakEnd->text().toDouble();
	breaks.lastChanged = index;

	for (auto* plot : m_plotList)
		plot->setYRangeBreaks(breaks);
}

void CartesianPlotDock::yBreakPositionChanged(int value) {
	CONDITIONAL_LOCK_RETURN;

	int index = ui.cbYBreak->currentIndex();
	CartesianPlot::RangeBreaks breaks = m_plot->yRangeBreaks();
	breaks.list[index].position = (float)value / 100.;
	breaks.lastChanged = index;

	for (auto* plot : m_plotList)
		plot->setYRangeBreaks(breaks);
}

void CartesianPlotDock::yBreakStyleChanged(int styleIndex) {
	CONDITIONAL_LOCK_RETURN;

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
	CONDITIONAL_LOCK_RETURN;

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

void CartesianPlotDock::borderCornerRadiusChanged(double value) {
	CONDITIONAL_RETURN_NO_LOCK;

	const double radius = Worksheet::convertToSceneUnits(value, m_worksheetUnit);
	for (auto* plot : m_plotList)
		plot->plotArea()->setBorderCornerRadius(radius);
}

// "Stack"-tab
void CartesianPlotDock::stackYOffsetChanged() {
	CONDITIONAL_RETURN_NO_LOCK;

	bool ok = false;
	double offset = QLocale().toDouble(ui.leStackYOffset->text(), &ok);
	if (!ok)
		return;

	qDebug()<<"offset im dock " << offset;
	for (auto* plot : m_plotList)
		plot->setStackYOffset(offset);
}

void CartesianPlotDock::exportPlotTemplate() {
	KConfig config;
	KConfigGroup group = config.group(QStringLiteral("PlotTemplate"));
	const QString dir = group.readEntry(QStringLiteral("ExportPath"), PlotTemplateDialog::defaultTemplateInstallPath());
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
	writer.writeDTD(QStringLiteral("<!DOCTYPE LabPlotXML>"));
	writer.writeStartElement(QStringLiteral("PlotTemplate"));
	writer.writeAttribute(QStringLiteral("xmlVersion"), QString::number(Project::currentBuildXmlVersion()));
	m_plot->save(&writer);
	writer.writeEndElement();
	writer.writeEndDocument();
}

//*************************************************************
//****** SLOTs for changes triggered in CartesianPlot *********
//*************************************************************
// general
void CartesianPlotDock::plotPlotColorModeChanged(CartesianPlot::PlotColorMode mode) {
	CONDITIONAL_LOCK_RETURN;
	ui.cbPlotColorMode->setCurrentIndex(static_cast<int>(mode));
}
void CartesianPlotDock::plotPlotColorMapChanged(const QString& name) {
	CONDITIONAL_LOCK_RETURN;
	ui.lColorMapName->setText(name);
	ui.lColorMapPreview->setPixmap(ColorMapsManager::instance()->previewPixmap(name));
}
void CartesianPlotDock::plotRangeTypeChanged(CartesianPlot::RangeType type) {
	CONDITIONAL_LOCK_RETURN;
	ui.cbRangeType->setCurrentIndex(static_cast<int>(type));
}
void CartesianPlotDock::plotRangeFirstValuesChanged(int value) {
	CONDITIONAL_LOCK_RETURN;
	ui.leRangePoints->setText(QLocale().toString(value));
}
void CartesianPlotDock::plotRangeLastValuesChanged(int value) {
	CONDITIONAL_LOCK_RETURN;
	ui.leRangePoints->setText(QLocale().toString(value));
}

// x & y ranges
void CartesianPlotDock::plotAutoScaleChanged(const Dimension dim, int index, bool checked) {
	CONDITIONAL_LOCK_RETURN;
	DEBUG(Q_FUNC_INFO << ", checked = " << checked)

	CELLWIDGET(dim, index, TwRangesColumn::Automatic, QCheckBox, setChecked(checked));
	CELLWIDGET(dim, index, TwRangesColumn::Format, QComboBox, setEnabled(!checked));
	CELLWIDGET(dim, index, TwRangesColumn::Min, QWidget, setEnabled(!checked));
	CELLWIDGET(dim, index, TwRangesColumn::Max, QWidget, setEnabled(!checked));
	CELLWIDGET(dim, index, TwRangesColumn::Scale, QComboBox, setEnabled(!checked));
}

void CartesianPlotDock::plotMinChanged(const Dimension dim, int rangeIndex, double value) {
	DEBUG(Q_FUNC_INFO << ", value = " << value)
	CONDITIONAL_LOCK_RETURN;

	CELLWIDGET(dim, rangeIndex, TwRangesColumn::Min, NumberSpinBox, setValue(value));
	CELLWIDGET(dim, rangeIndex, TwRangesColumn::Min, UTCDateTimeEdit, setMSecsSinceEpochUTC(value));

	updatePlotRangeListValues(dim, rangeIndex);
}

void CartesianPlotDock::plotMaxChanged(const Dimension dim, int rangeIndex, double value) {
	DEBUG(Q_FUNC_INFO << ", value = " << value)
	CONDITIONAL_LOCK_RETURN;

	CELLWIDGET(dim, rangeIndex, TwRangesColumn::Max, NumberSpinBox, setValue(value));
	CELLWIDGET(dim, rangeIndex, TwRangesColumn::Max, UTCDateTimeEdit, setMSecsSinceEpochUTC(value));

	updatePlotRangeListValues(dim, rangeIndex);
}

void CartesianPlotDock::plotRangeChanged(const Dimension dim, int rangeIndex, Range<double> range) {
	DEBUG(Q_FUNC_INFO << ", " << CartesianCoordinateSystem::dimensionToString(dim).toStdString() << " range = " << range.toStdString())

	// The ranges can change on multiple ways
	// - setting autoscale
	// - setting min/max
	// - but also when changing datarange type or the datarange points.
	// If the datarange type/points changes, CONDITIONAL_LOCK_RETURN locks already and then the ranges would not update.
	// To update also in those cases the cells will be updated all the time regardless of the m_initializing member state
	if (m_initializing) {
		CELLWIDGET(dim, rangeIndex, TwRangesColumn::Min, NumberSpinBox, setValue(range.start()));
		CELLWIDGET(dim, rangeIndex, TwRangesColumn::Min, UTCDateTimeEdit, setMSecsSinceEpochUTC(range.start()));
		CELLWIDGET(dim, rangeIndex, TwRangesColumn::Max, NumberSpinBox, setValue(range.end()));
		CELLWIDGET(dim, rangeIndex, TwRangesColumn::Max, UTCDateTimeEdit, setMSecsSinceEpochUTC(range.end()));
	} else {
		// Must be copied, because the Lock would otherwise be in it's own space and therefore it would not make any sense
		CONDITIONAL_LOCK_RETURN;
		CELLWIDGET(dim, rangeIndex, TwRangesColumn::Min, NumberSpinBox, setValue(range.start()));
		CELLWIDGET(dim, rangeIndex, TwRangesColumn::Min, UTCDateTimeEdit, setMSecsSinceEpochUTC(range.start()));
		CELLWIDGET(dim, rangeIndex, TwRangesColumn::Max, NumberSpinBox, setValue(range.end()));
		CELLWIDGET(dim, rangeIndex, TwRangesColumn::Max, UTCDateTimeEdit, setMSecsSinceEpochUTC(range.end()));
	}
	updatePlotRangeListValues(dim, rangeIndex);
}

void CartesianPlotDock::plotScaleChanged(const Dimension dim, int rangeIndex, RangeT::Scale scale) {
	DEBUG(Q_FUNC_INFO << ", scale = " << ENUM_TO_STRING(RangeT, Scale, scale))
	CONDITIONAL_LOCK_RETURN;
	CELLWIDGET(dim, rangeIndex, TwRangesColumn::Scale, QComboBox, setCurrentIndex(static_cast<int>(scale)));
}

void CartesianPlotDock::plotRangeFormatChanged(const Dimension dim, int rangeIndex, RangeT::Format format) {
	DEBUG(Q_FUNC_INFO << ", format = " << ENUM_TO_STRING(RangeT, Format, format))
	CONDITIONAL_LOCK_RETURN;
	CELLWIDGET(dim, rangeIndex, TwRangesColumn::Format, QComboBox, setCurrentIndex(static_cast<int>(format)));

	updatePlotRangeList();
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
	CONDITIONAL_LOCK_RETURN;
	ui.chkXBreak->setChecked(on);
}

void CartesianPlotDock::plotXRangeBreaksChanged(const CartesianPlot::RangeBreaks&) {
}

void CartesianPlotDock::plotYRangeBreakingEnabledChanged(bool on) {
	CONDITIONAL_LOCK_RETURN;
	ui.chkYBreak->setChecked(on);
}

void CartesianPlotDock::plotYRangeBreaksChanged(const CartesianPlot::RangeBreaks&) {
}

// layout
void CartesianPlotDock::plotRectChanged(QRectF& rect) {
	CONDITIONAL_LOCK_RETURN;
	ui.sbLeft->setValue(Worksheet::convertFromSceneUnits(roundSceneValue(rect.x(), m_units), m_worksheetUnit));
	ui.sbTop->setValue(Worksheet::convertFromSceneUnits(roundSceneValue(rect.y(), m_units), m_worksheetUnit));
	ui.sbWidth->setValue(Worksheet::convertFromSceneUnits(roundSceneValue(rect.width(), m_units), m_worksheetUnit));
	ui.sbHeight->setValue(Worksheet::convertFromSceneUnits(roundSceneValue(rect.height(), m_units), m_worksheetUnit));
}

void CartesianPlotDock::plotHorizontalPaddingChanged(double value) {
	CONDITIONAL_LOCK_RETURN;
	ui.sbPaddingHorizontal->setValue(Worksheet::convertFromSceneUnits(roundSceneValue(value, m_units), m_worksheetUnit));
}

void CartesianPlotDock::plotVerticalPaddingChanged(double value) {
	CONDITIONAL_LOCK_RETURN;
	ui.sbPaddingVertical->setValue(Worksheet::convertFromSceneUnits(roundSceneValue(value, m_units), m_worksheetUnit));
}

void CartesianPlotDock::plotRightPaddingChanged(double value) {
	CONDITIONAL_LOCK_RETURN;
	ui.sbPaddingRight->setValue(Worksheet::convertFromSceneUnits(roundSceneValue(value, m_units), m_worksheetUnit));
}

void CartesianPlotDock::plotBottomPaddingChanged(double value) {
	CONDITIONAL_LOCK_RETURN;
	ui.sbPaddingBottom->setValue(Worksheet::convertFromSceneUnits(roundSceneValue(value, m_units), m_worksheetUnit));
}

void CartesianPlotDock::plotSymmetricPaddingChanged(bool symmetric) {
	CONDITIONAL_LOCK_RETURN;
	ui.cbPaddingSymmetric->setChecked(symmetric);
}

// border
void CartesianPlotDock::plotBorderTypeChanged(PlotArea::BorderType type) {
	CONDITIONAL_LOCK_RETURN;
	ui.tbBorderTypeLeft->setChecked(type.testFlag(PlotArea::BorderTypeFlags::BorderLeft));
	ui.tbBorderTypeRight->setChecked(type.testFlag(PlotArea::BorderTypeFlags::BorderRight));
	ui.tbBorderTypeTop->setChecked(type.testFlag(PlotArea::BorderTypeFlags::BorderTop));
	ui.tbBorderTypeBottom->setChecked(type.testFlag(PlotArea::BorderTypeFlags::BorderBottom));
}

void CartesianPlotDock::plotBorderCornerRadiusChanged(double value) {
	CONDITIONAL_LOCK_RETURN;
	ui.sbBorderCornerRadius->setValue(Worksheet::convertFromSceneUnits(roundSceneValue(value, m_units), m_worksheetUnit));
}

// stacking
void CartesianPlotDock::plotStackYOffsetChanged(double offset) {
	CONDITIONAL_LOCK_RETURN;
	ui.leStackYOffset->setText(QLocale().toString(offset));
}

//*************************************************************
//******************** SETTINGS *******************************
//*************************************************************
void CartesianPlotDock::loadConfigFromTemplate(KConfig& config) {
	auto name = TemplateHandler::templateName(config);
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

	int index = static_cast<int>(m_plot->rangeType());
	ui.cbRangeType->setCurrentIndex(index);
	rangeTypeChanged(index);
	ui.cbNiceExtend->setChecked(m_plot->niceExtend());

	m_updateUI = false; // avoid updating twice
	updateRangeList(Dimension::X);
	m_updateUI = true;
	updateRangeList(Dimension::Y);

	index = static_cast<int>(m_plot->plotColorMode());
	ui.cbPlotColorMode->setCurrentIndex(index);
	plotColorModeChanged(index);
	plotColorMapChanged(m_plot->plotColorMap());

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
		ui.cbXBreak->addItem(QStringLiteral("1"));
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
		ui.cbYBreak->addItem(QStringLiteral("1"));
	ui.cbYBreak->setCurrentIndex(0);

	//"Plot Area"-tab
	const auto* plotArea = m_plot->plotArea();

	// Background, border and cursor Lines
	QList<Background*> backgrounds;
	QList<Line*> cursorLines;
	QList<Line*> borderLines;
	for (auto* plot : m_plotList) {
		backgrounds << plot->plotArea()->background();
		borderLines << plot->plotArea()->borderLine();
		cursorLines << plot->cursorLine();
	}

	backgroundWidget->setBackgrounds(backgrounds);
	borderLineWidget->setLines(borderLines);
	cursorLineWidget->setLines(cursorLines);

	// Layout
	ui.sbLeft->setValue(Worksheet::convertFromSceneUnits(roundSceneValue(m_plot->rect().x(), m_units), m_worksheetUnit));
	ui.sbTop->setValue(Worksheet::convertFromSceneUnits(roundSceneValue(m_plot->rect().y(), m_units), m_worksheetUnit));
	ui.sbWidth->setValue(Worksheet::convertFromSceneUnits(roundSceneValue(m_plot->rect().width(), m_units), m_worksheetUnit));
	ui.sbHeight->setValue(Worksheet::convertFromSceneUnits(roundSceneValue(m_plot->rect().height(), m_units), m_worksheetUnit));

	// Padding
	ui.sbPaddingHorizontal->setValue(Worksheet::convertFromSceneUnits(roundSceneValue(m_plot->horizontalPadding(), m_units), m_worksheetUnit));
	ui.sbPaddingVertical->setValue(Worksheet::convertFromSceneUnits(roundSceneValue(m_plot->verticalPadding(), m_units), m_worksheetUnit));
	ui.sbPaddingRight->setValue(Worksheet::convertFromSceneUnits(roundSceneValue(m_plot->rightPadding(), m_units), m_worksheetUnit));
	ui.sbPaddingBottom->setValue(Worksheet::convertFromSceneUnits(roundSceneValue(m_plot->bottomPadding(), m_units), m_worksheetUnit));
	ui.cbPaddingSymmetric->setChecked(m_plot->symmetricPadding());

	// Border
	ui.tbBorderTypeLeft->setChecked(plotArea->borderType().testFlag(PlotArea::BorderTypeFlags::BorderLeft));
	ui.tbBorderTypeRight->setChecked(plotArea->borderType().testFlag(PlotArea::BorderTypeFlags::BorderRight));
	ui.tbBorderTypeTop->setChecked(plotArea->borderType().testFlag(PlotArea::BorderTypeFlags::BorderTop));
	ui.tbBorderTypeBottom->setChecked(plotArea->borderType().testFlag(PlotArea::BorderTypeFlags::BorderBottom));
	ui.sbBorderCornerRadius->setValue(Worksheet::convertFromSceneUnits(roundSceneValue(plotArea->borderCornerRadius(), m_units), m_worksheetUnit));

	// Stacking
	ui.leStackYOffset->setText(QLocale().toString(m_plot->stackYOffset()));
}

void CartesianPlotDock::loadConfig(KConfig& config) {
	KConfigGroup group = config.group(QStringLiteral("CartesianPlot"));

	// General
	// we don't load/save the settings in the general-tab, since they are not style related.
	// This data is read in CartesianPlotDock::setPlots().
	auto index = group.readEntry(QStringLiteral("PlotColorMode"), static_cast<int>(m_plot->plotColorMode()));
	ui.cbPlotColorMode->setCurrentIndex(index);
	plotColorModeChanged(index);
	plotColorMapChanged(group.readEntry(QStringLiteral("PlotColorMap"), m_plot->plotColorMap()));

	// Title
	KConfigGroup plotTitleGroup = config.group(QStringLiteral("CartesianPlotTitle"));
	labelWidget->loadConfig(plotTitleGroup);

	// Scale breakings
	// TODO

	// Layout
	ui.sbPaddingHorizontal->setValue(
		Worksheet::convertFromSceneUnits(roundSceneValue(group.readEntry(QStringLiteral("HorizontalPadding"), m_plot->horizontalPadding()), m_units), m_worksheetUnit));
	ui.sbPaddingVertical->setValue(
		Worksheet::convertFromSceneUnits(roundSceneValue(group.readEntry(QStringLiteral("VerticalPadding"), m_plot->verticalPadding()), m_units), m_worksheetUnit));
	ui.sbPaddingRight->setValue(Worksheet::convertFromSceneUnits(roundSceneValue(group.readEntry(QStringLiteral("RightPadding"), m_plot->rightPadding()), m_units), m_worksheetUnit));
	ui.sbPaddingBottom->setValue(Worksheet::convertFromSceneUnits(roundSceneValue(group.readEntry(QStringLiteral("BottomPadding"), m_plot->bottomPadding()), m_units), m_worksheetUnit));
	ui.cbPaddingSymmetric->setChecked(group.readEntry(QStringLiteral("SymmetricPadding"), m_plot->symmetricPadding()));

	// Area
	backgroundWidget->loadConfig(group);

	const auto* plotArea = m_plot->plotArea();
	auto type = static_cast<PlotArea::BorderType>(group.readEntry(QStringLiteral("BorderType"), static_cast<int>(plotArea->borderType())));
	ui.tbBorderTypeLeft->setChecked(type.testFlag(PlotArea::BorderTypeFlags::BorderLeft));
	ui.tbBorderTypeRight->setChecked(type.testFlag(PlotArea::BorderTypeFlags::BorderRight));
	ui.tbBorderTypeTop->setChecked(type.testFlag(PlotArea::BorderTypeFlags::BorderTop));
	ui.tbBorderTypeBottom->setChecked(type.testFlag(PlotArea::BorderTypeFlags::BorderBottom));

	borderLineWidget->loadConfig(group);
	ui.sbBorderCornerRadius->setValue(
		Worksheet::convertFromSceneUnits(roundSceneValue(group.readEntry(QStringLiteral("BorderCornerRadius"), plotArea->borderCornerRadius()), m_units), m_worksheetUnit));
}

void CartesianPlotDock::saveConfigAsTemplate(KConfig& config) {
	KConfigGroup group = config.group(QStringLiteral("CartesianPlot"));

	// General
	// we don't load/save the any settings in the general-tab that are not style/appearance related.
	group.writeEntry(QStringLiteral("PlotColorMode"), static_cast<int>(m_plot->plotColorMode()));
	group.writeEntry(QStringLiteral("Theme"), m_plot->theme());
	group.writeEntry(QStringLiteral("PlotColorMap"), m_plot->plotColorMap());

	// Title
	KConfigGroup plotTitleGroup = config.group(QStringLiteral("CartesianPlotTitle"));
	labelWidget->saveConfig(plotTitleGroup);

	// Layout
	group.writeEntry(QStringLiteral("HorizontalPadding"), m_plot->horizontalPadding());
	group.writeEntry(QStringLiteral("VerticalPadding"), m_plot->verticalPadding());
	group.writeEntry(QStringLiteral("RightPadding"), m_plot->rightPadding());
	group.writeEntry(QStringLiteral("BottomPadding"), m_plot->bottomPadding());
	group.writeEntry(QStringLiteral("SymmetricPadding"), m_plot->symmetricPadding());

	// Scale breakings
	// TODO

	// Area
	backgroundWidget->saveConfig(group);
	group.writeEntry(QStringLiteral("BorderType"), static_cast<int>(m_plot->plotArea()->borderType()));
	borderLineWidget->saveConfig(group);
	group.writeEntry(QStringLiteral("BorderCornerRadius"), m_plot->plotArea()->borderCornerRadius());

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
