#include "CANOptionsWidget.h"
#include "backend/core/Settings.h"
#include "backend/datasources/filters/VectorBLFFilter.h"
#include "ui_CANOptionsWidget.h"

#include <KConfigGroup>

CANOptionsWidget::CANOptionsWidget(QWidget* parent)
	: QWidget(parent)
	, ui(new Ui::CANOptionsWidget) {
	ui->setupUi(this);

	ui->cbImportMode->addItem(i18n("Use NAN"), (int)VectorBLFFilter::TimeHandling::ConcatNAN);
	ui->cbImportMode->addItem(i18n("Use previous value"), (int)VectorBLFFilter::TimeHandling::ConcatPrevious);
	// Not yet implemented
	// ui->cbImportMode->addItem(i18n("Separate time columns"), (int)VectorBLFFilter::TimeHandling::Separate);

	loadSettings();
}

CANOptionsWidget::~CANOptionsWidget() {
	delete ui;
}

void CANOptionsWidget::applyFilterSettings(VectorBLFFilter* filter) const {
	filter->setConvertTimeToSeconds(ui->cbConvertSeconds->isChecked());
	filter->setTimeHandlingMode(static_cast<VectorBLFFilter::TimeHandling>(ui->cbImportMode->itemData(ui->cbImportMode->currentIndex()).toInt()));

	saveSettings();
}

void CANOptionsWidget::saveSettings() const {
	KConfigGroup conf = Settings::group(QStringLiteral("ImportCANOptions"));

	conf.writeEntry("ConvertSeconds", ui->cbConvertSeconds->isChecked());
	conf.writeEntry("TimeHandlingMode", ui->cbImportMode->itemData(ui->cbImportMode->currentIndex()));
}

void CANOptionsWidget::loadSettings() const {
	KConfigGroup conf = Settings::group(QStringLiteral("ImportCANOptions"));

	ui->cbConvertSeconds->setChecked(conf.readEntry("ConvertSeconds", true));
	const auto mode = conf.readEntry("TimeHandlingMode", (int)VectorBLFFilter::TimeHandling::ConcatPrevious);

	for (int i = 0; i < ui->cbImportMode->count(); i++) {
		if (mode == ui->cbImportMode->itemData(i).toInt()) {
			ui->cbImportMode->setCurrentIndex(i);
			break;
		}
	}
}
