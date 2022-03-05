/*
    File                 : FormattingHeatmapDialog.h
    Project              : LabPlot
    Description          : Dialog for the conditional formatting according to a heatmap
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2021 Alexander Semke <alexander.semke@web.de>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "FormattingHeatmapDialog.h"
#include "backend/core/column/Column.h"
#include "backend/spreadsheet/Spreadsheet.h"
#include "kdefrontend/colormaps/ColorMapsDialog.h"
#include "tools/ColorMapsManager.h"

#include <QDialogButtonBox>
#include <QDir>
#include <QPushButton>
#include <QWindow>

#include <KLocalizedString>
#include <KSharedConfig>
#include <KWindowConfig>

/*!
	\class FormattingHeatmapDialog
	\brief Dialog for generating non-uniform random numbers.

	\ingroup kdefrontend
 */

FormattingHeatmapDialog::FormattingHeatmapDialog(Spreadsheet* s, QWidget* parent) : QDialog(parent), m_spreadsheet(s) {
	setWindowTitle(i18nc("@title:window", "Conditional Formatting - Heatmap"));

	auto* mainWidget = new QWidget(this);
	ui.setupUi(mainWidget);
	auto* layout = new QVBoxLayout(this);

	auto* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
	m_okButton = buttonBox->button(QDialogButtonBox::Ok);
	m_okButton->setDefault(true);
	m_okButton->setToolTip(i18n("Format the cells according the level and color settings"));
	m_okButton->setText(i18n("&Format"));

	connect(buttonBox->button(QDialogButtonBox::Cancel), &QPushButton::clicked, this, &FormattingHeatmapDialog::close);
	connect(buttonBox, &QDialogButtonBox::accepted, this, &FormattingHeatmapDialog::accept);
	connect(buttonBox, &QDialogButtonBox::rejected, this, &FormattingHeatmapDialog::reject);

	layout->addWidget(mainWidget);
	layout->addWidget(buttonBox);
	setLayout(layout);

	ui.bColorMap->setIcon(QIcon::fromTheme(QLatin1String("color-management")));
	ui.lColorMapPreview->setMaximumHeight(ui.bColorMap->height());

	ui.cbHightlight->addItem(i18n("Background"));
	ui.cbHightlight->addItem(i18n("Font Color"));
	ui.cbHightlight->addItem(i18n("Icon"));

	ui.leMinimum->setValidator(new QDoubleValidator(ui.leMinimum));
	ui.leMaximum->setValidator(new QDoubleValidator(ui.leMaximum));

	connect(ui.chkAutoRange, &QCheckBox::toggled, this, &FormattingHeatmapDialog::autoRangeChanged);
	connect(ui.leMaximum, &QLineEdit::textChanged, this, &FormattingHeatmapDialog::checkValues);
	connect(ui.leMinimum, &QLineEdit::textChanged, this, &FormattingHeatmapDialog::checkValues);
	connect(ui.bColorMap, &QPushButton::clicked, this, &FormattingHeatmapDialog::selectColorMap);

	//restore saved settings if available
	create(); // ensure there's a window created
	const KConfigGroup conf(KSharedConfig::openConfig(), "FormattingHeatmapDialog");
	if (conf.exists()) {
		auto state = (Qt::CheckState)(conf.readEntry("AutoRange", (int)Qt::Checked));
		ui.chkAutoRange->setCheckState(state);
		ui.cbHightlight->setCurrentIndex(conf.readEntry("Highlight", 0));

		//"Set1_3" from ColorBrewer(Qualitative) as default color map
		m_name = conf.readEntry("Colormap", "Set1_3");
		m_colors << QColor(228, 26, 28) << QColor(55, 126, 184) << QColor(77, 175, 74);

		KWindowConfig::restoreWindowSize(windowHandle(), conf);
		resize(windowHandle()->size()); // workaround for QTBUG-40584
	} else
		resize( QSize(400, 0).expandedTo(minimumSize()) );
}

FormattingHeatmapDialog::~FormattingHeatmapDialog() {
	//save current settings
	KConfigGroup conf(KSharedConfig::openConfig(), "FormattingHeatmapDialog");
	conf.writeEntry("AutoRange", (int)ui.chkAutoRange->checkState());
	conf.writeEntry("Highlight", ui.cbHightlight->currentIndex());
	KWindowConfig::saveWindowSize(windowHandle(), conf);
}

void FormattingHeatmapDialog::setColumns(const QVector<Column*>& columns) {
	m_columns = columns;

	double min = INFINITY;
	double max = -INFINITY;
	bool formatShown = false;
	for (const auto* col : qAsConst(m_columns)) {
		if (!col->isNumeric())
			continue;

		if (col->minimum() < min)
			min = col->minimum();
		if (col->maximum() > max)
			max = col->maximum();

		//show the format settings of the first column
		if (!formatShown && col->hasHeatmapFormat()) {
			const auto& format =col->heatmapFormat();

			m_name = format.name;
			m_colors = format.colors;
			ui.cbHightlight->setCurrentIndex(static_cast<int>(format.type));
			formatShown = true;
		}
	}

	QPixmap pixmap;
	ColorMapsManager::instance()->render(pixmap, m_name);
	ui.lColorMapPreview->setPixmap(pixmap);

	if (min != INFINITY)
		ui.leMinimum->setText(QString::number(min));
	else
		ui.leMinimum->setText(QString());

	if (max != -INFINITY)
		ui.leMaximum->setText(QString::number(max));
	else
		ui.leMaximum->setText(QString());
}

AbstractColumn::HeatmapFormat FormattingHeatmapDialog::format() {
	AbstractColumn::HeatmapFormat format;
	format.min = ui.leMinimum->text().toDouble();
	format.max = ui.leMaximum->text().toDouble();
	format.colors = m_colors;
	format.name = m_name;
	format.type = static_cast<AbstractColumn::Formatting>(ui.cbHightlight->currentIndex());
	return format;
}

void FormattingHeatmapDialog::autoRangeChanged(bool state) {
	ui.leMinimum->setEnabled(!state);
	ui.leMaximum->setEnabled(!state);

	checkValues();
}

void FormattingHeatmapDialog::selectColorMap() {
	auto* dlg = new ColorMapsDialog(this);
	if (dlg->exec() == QDialog::Accepted) {
		m_name = dlg->name();
		ui.lColorMapPreview->setPixmap(dlg->previewPixmap());
		m_colors = dlg->colors(); //fetch the colors _after_ the preview pixmap was fetched to get the proper colors from the color manager
		ui.lColorMapPreview->setFocus();
	}
}

void FormattingHeatmapDialog::checkValues() {
	if (ui.chkAutoRange->checkState() != Qt::Checked
		&& (ui.leMaximum->text().isEmpty() || ui.leMinimum->text().isEmpty())) {
		m_okButton->setEnabled(false);
		return;
	}

	m_okButton->setEnabled(true);
}
