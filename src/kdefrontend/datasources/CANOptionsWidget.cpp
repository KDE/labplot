#include "CANOptionsWidget.h"
#include "backend/datasources/filters/CANFilter.h"
#include "ui_CANOptionsWidget.h"

#include <KConfigGroup>
#include <KSharedConfig>

CANOptionsWidget::CANOptionsWidget(QWidget* parent)
	: QWidget(parent)
	, ui(new Ui::CANOptionsWidget) {
	ui->setupUi(this);

	ui->cbImportMode->addItem(i18n("Use NAN"), (int)CANFilter::TimeHandling::ConcatNAN);
	ui->cbImportMode->addItem(i18n("Use previous value"), (int)CANFilter::TimeHandling::ConcatPrevious);
	// Not yet implemented
	// ui->cbImportMode->addItem(i18n("Separate time columns"), (int)CANFilter::TimeHandling::Separate);

	loadSettings();
}

CANOptionsWidget::~CANOptionsWidget() {
	delete ui;
}

void CANOptionsWidget::applyFilterSettings(CANFilter* filter) const {
	filter->setConvertTimeToSeconds(ui->cbConvertSeconds->isChecked());
	filter->setTimeHandlingMode(static_cast<CANFilter::TimeHandling>(ui->cbImportMode->itemData(ui->cbImportMode->currentIndex()).toInt()));

	ui->cbConvertSeconds->setVisible(false);
	if (filter->type() == AbstractFileFilter::FileType::VECTOR_BLF)
		ui->cbConvertSeconds->setVisible(true);

	saveSettings();
}

void CANOptionsWidget::saveSettings() const {
	KConfigGroup conf(KSharedConfig::openConfig(), "ImportCANOptions");

	conf.writeEntry("ConvertSeconds", ui->cbConvertSeconds->isChecked());
	conf.writeEntry("TimeHandlingMode", ui->cbImportMode->itemData(ui->cbImportMode->currentIndex()));
}

void CANOptionsWidget::loadSettings() const {
	KConfigGroup conf(KSharedConfig::openConfig(), "ImportJson");

	ui->cbConvertSeconds->setChecked(conf.readEntry("ConvertSeconds", true));
	const auto mode = conf.readEntry("TimeHandlingMode", (int)CANFilter::TimeHandling::ConcatPrevious);

	for (int i = 0; i < ui->cbImportMode->count(); i++) {
		if (mode == ui->cbImportMode->itemData(i).toInt()) {
			ui->cbImportMode->setCurrentIndex(i);
			break;
		}
	}
}
