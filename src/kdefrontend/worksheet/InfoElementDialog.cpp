#include "InfoElementDialog.h"
#include "ui_infoelementdialog.h"

#include <QDialogButtonBox>
#include <QPushButton>

#include "backend/worksheet/plots/cartesian/CartesianCoordinateSystem.h"
#include "backend/worksheet/plots/cartesian/CartesianPlot.h"
#include "backend/worksheet/plots/cartesian/XYCurve.h"

InfoElementDialog::InfoElementDialog(QWidget* parent) :
	QDialog(parent),
	ui(new Ui::InfoElementDialog) {
	ui->setupUi(this);

	connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &InfoElementDialog::createElement);
	connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &InfoElementDialog::removePlot); // reset m_plot
	connect(ui->lst_useCurve, &QListWidget::currentItemChanged, this, &InfoElementDialog::updateSelectedCurveLabel);
	connect(ui->sb_Pos, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &InfoElementDialog::validateSettings);
}

InfoElementDialog::~InfoElementDialog() {
	delete ui;
}

void InfoElementDialog::setPlot(CartesianPlot* plot) {
	if (m_plot) // when in a new plot a marker should be created without creating marker in old plot
		removePlot();

	m_plot = plot;
	connect(m_plot, &CartesianPlot::aspectAboutToBeRemoved, this, &InfoElementDialog::removePlot);
	connect(m_plot, &CartesianPlot::curveRemoved, this, &InfoElementDialog::updateSettings);
	connect(m_plot, &CartesianPlot::curveAdded, this, &InfoElementDialog::updateSettings);
	connect(m_plot, &CartesianPlot::xMinChanged, this, &InfoElementDialog::updateSettings);
	connect(m_plot, &CartesianPlot::xMaxChanged, this, &InfoElementDialog::updateSettings);

	updateSettings();


    setWindowTitle(i18n("Add Info Element in plot: ") + m_plot->name());
}

void InfoElementDialog::setActiveCurve(const XYCurve* curve, double pos) {
	auto items = ui->lst_useCurve->findItems(curve->name(), Qt::MatchExactly);

    for (auto item : items) {
		if (item->text() == curve->name())
			ui->lst_useCurve->setCurrentItem(item);
	}

	ui->sb_Pos->setValue(pos);
}

/*!
 * \brief InfoElementDialog::updateSettings
 * Called each time a setting in the cartesianplot is changed. Not each setting is handled differently.
 * This would be to complex for this simple dialog
 */
void InfoElementDialog::updateSettings() {

	if (!m_plot) // why this ca occur?
		return;
	ui->lst_useCurve->clear();

	auto curves = m_plot->children<const XYCurve>();
	for (auto curve: curves)
		ui->lst_useCurve->addItem(curve->name());

	ui->lst_useCurve->setCurrentRow(0);
	if (ui->lst_useCurve->count())
		ui->l_selectedCurve->setText(ui->lst_useCurve->currentItem()->text());
	else
		ui->l_selectedCurve->setText("");

	// TODO: set position in the mid of the screen as default
	CartesianCoordinateSystem* cSystem = dynamic_cast<CartesianCoordinateSystem*>(m_plot->coordinateSystem());
	if (cSystem) {
		//TODO: Range?
		double xMinScene = cSystem->mapLogicalToScene(QPointF(m_plot->xRange().start(), m_plot->yRange().start())).x();
		double xMaxScene = cSystem->mapLogicalToScene(QPointF(m_plot->xRange().end(), m_plot->yRange().start())).x();

		double midLogical = cSystem->mapSceneToLogical(QPointF((xMinScene + xMaxScene)/2, 0)).x();

		ui->sb_Pos->setMinimum(m_plot->xRange().start());
		ui->sb_Pos->setMaximum(m_plot->xRange().end());
		ui->sb_Pos->setValue(midLogical);
	} else
		ui->sb_Pos->setValue(0);

	validateSettings();
}

void InfoElementDialog::createElement() {
	if (m_plot) {
		double pos = ui->sb_Pos->value();

		QString curveName = ui->lst_useCurve->currentItem()->text();
		for (auto curve: m_plot->children<const XYCurve>()) {
			if (curveName == curve->name())
				m_plot->addInfoElement(curve, pos);
		}
		m_plot = nullptr;
	}
}

void InfoElementDialog::updateSelectedCurveLabel(QListWidgetItem *item) {
	// when listwidget is cleared, there is anymore an item, but the signal is raised anyway
	if (!item)
		return;
	ui->l_selectedCurve->setText(item->text());
}

/*!
 * \brief InfoElementDialog::removePlot
 * Called when dialog is closed or when the plot was deleted before creating marker
 */
void InfoElementDialog::removePlot() {

	ui->lst_useCurve->clear();
	ui->l_selectedCurve->setText("");

	// remove all connections
	disconnect(m_plot, nullptr, this, nullptr);
	m_plot = nullptr;

	validateSettings();
}

void InfoElementDialog::validateSettings() {

	if (ui->lst_useCurve->count() > 0 && m_plot) {
		ui->buttonBox->button(QDialogButtonBox::StandardButton::Ok)->setEnabled(true);
		return;
	}

	ui->buttonBox->button(QDialogButtonBox::StandardButton::Ok)->setEnabled(false);
}
