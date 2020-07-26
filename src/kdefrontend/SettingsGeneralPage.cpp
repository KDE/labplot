/***************************************************************************
    File                 : SettingsGeneralPage.cpp
    Project              : LabPlot
    Description          : general settings page
    --------------------------------------------------------------------
    Copyright            : (C) 2008-2020 Alexander Semke (alexander.semke@web.de)
    Copyright            : (C) 2020 Stefan Gerlach (stefan.gerlach@uni.kn)

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

#include "SettingsGeneralPage.h"
#include "backend/lib/macros.h"

#include <KI18n/KLocalizedString>
#include <KConfigGroup>
#include <KSharedConfig>

/**
 * \brief Page for the 'General' settings of the Labplot settings dialog.
 */
SettingsGeneralPage::SettingsGeneralPage(QWidget* parent) : SettingsPage(parent) {
	ui.setupUi(this);
	ui.sbAutoSaveInterval->setSuffix(i18n("min."));
	retranslateUi();

	connect(ui.cbLoadOnStart, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &SettingsGeneralPage::changed);
	connect(ui.cbTitleBar, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &SettingsGeneralPage::changed);
	connect(ui.cbInterface, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &SettingsGeneralPage::interfaceChanged);
	connect(ui.cbMdiVisibility, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &SettingsGeneralPage::changed);
	connect(ui.cbTabPosition, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &SettingsGeneralPage::changed);
	connect(ui.cbUnits, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &SettingsGeneralPage::changed);
	connect(ui.cbDecimalSeparator, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &SettingsGeneralPage::changed);
	connect(ui.chkAutoSave, &QCheckBox::stateChanged, this, &SettingsGeneralPage::autoSaveChanged);
	connect(ui.chkMemoryInfo, &QCheckBox::stateChanged, this, &SettingsGeneralPage::changed);

	loadSettings();
	interfaceChanged(ui.cbInterface->currentIndex());
	autoSaveChanged(ui.chkAutoSave->checkState());
}

/* returns decimal separator (as SettingsGeneralPage::DecimalSeparator) of given locale (default: system setting) */
SettingsGeneralPage::DecimalSeparator SettingsGeneralPage::decimalSeparator(QLocale locale) {
	DEBUG(Q_FUNC_INFO << ", LOCALE: " << STDSTRING(locale.name()) << ',' << locale.language())
	QChar decimalPoint{locale.decimalPoint()};
	DEBUG(Q_FUNC_INFO << ", SEPARATING CHAR: " << STDSTRING(QString(decimalPoint)) )
	if (decimalPoint == QChar('.'))
		return DecimalSeparator::Dot;
	else if (decimalPoint == QChar(','))
		return DecimalSeparator::Comma;

	return DecimalSeparator::Arabic;
}

QLocale::Language SettingsGeneralPage::decimalSeparatorLocale() const {
	int currentIndex = ui.cbDecimalSeparator->currentIndex();
	DEBUG(Q_FUNC_INFO << ", SYSTEM LOCALE: " << STDSTRING(QLocale().name()) << ':' << QLocale().language())
	DEBUG(Q_FUNC_INFO << ", SYSTEM SEPARATING CHAR: " << STDSTRING(QString(QLocale().decimalPoint())) )

	QChar groupSeparator{QLocale().groupSeparator()};
	switch (currentIndex) {
	case static_cast<int>(DecimalSeparator::Dot):
		if (groupSeparator == QLocale(QLocale::Language::Zarma).groupSeparator())	// \u00a0
			return QLocale::Language::Zarma;	// . \u00a0
		else if (groupSeparator == QLocale(QLocale::Language::SwissGerman).groupSeparator())	// \u2019
			return QLocale::Language::SwissGerman;  // . \u2019
		else
			return QLocale::Language::C;	 	// . ,
	case static_cast<int>(DecimalSeparator::Comma):
		if (groupSeparator == QLocale(QLocale::Language::French).groupSeparator())	// \u00a0
			return QLocale::Language::French;       // , \u00a0
		else if (groupSeparator == QLocale(QLocale::Language::Walser).groupSeparator())	// \u2019
			return QLocale::Language::Walser;       // , \u2019
		else
			return QLocale::Language::German;       // , .
	case static_cast<int>(DecimalSeparator::Arabic):
		return QLocale::Language::Arabic;		// \u066b \u066c
	default:	// automatic
		return QLocale::Language::AnyLanguage;
	}
}

void SettingsGeneralPage::applySettings() {
	DEBUG(Q_FUNC_INFO)
	if (!m_changed)
		return;

	KConfigGroup group = KSharedConfig::openConfig()->group(QLatin1String("Settings_General"));
	group.writeEntry(QLatin1String("LoadOnStart"), ui.cbLoadOnStart->currentIndex());
	group.writeEntry(QLatin1String("TitleBar"), ui.cbTitleBar->currentIndex());
	group.writeEntry(QLatin1String("ViewMode"), ui.cbInterface->currentIndex());
	group.writeEntry(QLatin1String("TabPosition"), ui.cbTabPosition->currentIndex());
	group.writeEntry(QLatin1String("MdiWindowVisibility"), ui.cbMdiVisibility->currentIndex());
	group.writeEntry(QLatin1String("Units"), ui.cbUnits->currentIndex());
	if (ui.cbDecimalSeparator->currentIndex() == static_cast<int>(DecimalSeparator::Automatic))	// needed to overwrite previous setting
		group.writeEntry(QLatin1String("DecimalSeparatorLocale"), static_cast<int>(QLocale::Language::AnyLanguage));
	else
		group.writeEntry(QLatin1String("DecimalSeparatorLocale"), static_cast<int>(decimalSeparatorLocale()));
	group.writeEntry(QLatin1String("AutoSave"), ui.chkAutoSave->isChecked());
	group.writeEntry(QLatin1String("AutoSaveInterval"), ui.sbAutoSaveInterval->value());
	group.writeEntry(QLatin1String("ShowMemoryInfo"), ui.chkMemoryInfo->isChecked());
}

void SettingsGeneralPage::restoreDefaults() {
	ui.cbLoadOnStart->setCurrentIndex(0);
	ui.cbTitleBar->setCurrentIndex(0);
	ui.cbInterface->setCurrentIndex(0);
	ui.cbTabPosition->setCurrentIndex(0);
	ui.cbMdiVisibility->setCurrentIndex(0);
	ui.cbUnits->setCurrentIndex(0);
	ui.cbDecimalSeparator->setCurrentIndex(static_cast<int>(DecimalSeparator::Automatic));
	ui.chkAutoSave->setChecked(false);
	ui.sbAutoSaveInterval->setValue(0);
	ui.sbAutoSaveInterval->setValue(5);
	ui.chkMemoryInfo->setChecked(true);
}

void SettingsGeneralPage::loadSettings() {
	const KConfigGroup group = KSharedConfig::openConfig()->group(QLatin1String("Settings_General"));
	ui.cbLoadOnStart->setCurrentIndex(group.readEntry(QLatin1String("LoadOnStart"), 0));
	ui.cbTitleBar->setCurrentIndex(group.readEntry(QLatin1String("TitleBar"), 0));
	ui.cbInterface->setCurrentIndex(group.readEntry(QLatin1String("ViewMode"), 0));
	ui.cbTabPosition->setCurrentIndex(group.readEntry(QLatin1String("TabPosition"), 0));
	ui.cbMdiVisibility->setCurrentIndex(group.readEntry(QLatin1String("MdiWindowVisibility"), 0));
	ui.cbUnits->setCurrentIndex(group.readEntry(QLatin1String("Units"), 0));
	QLocale locale(static_cast<QLocale::Language>(group.readEntry( QLatin1String("DecimalSeparatorLocale"), static_cast<int>(QLocale::Language::AnyLanguage) )) );
	if (locale.language() == QLocale::Language::AnyLanguage) 	// no or default setting
		ui.cbDecimalSeparator->setCurrentIndex( static_cast<int>(DecimalSeparator::Automatic) );
	else
		ui.cbDecimalSeparator->setCurrentIndex( static_cast<int>(decimalSeparator(locale)) );
	ui.chkAutoSave->setChecked(group.readEntry<bool>(QLatin1String("AutoSave"), false));
	ui.sbAutoSaveInterval->setValue(group.readEntry(QLatin1String("AutoSaveInterval"), 0));
	ui.chkMemoryInfo->setChecked(group.readEntry<bool>(QLatin1String("ShowMemoryInfo"), true));
}

void SettingsGeneralPage::retranslateUi() {
	ui.cbLoadOnStart->clear();
	ui.cbLoadOnStart->addItem(i18n("Do nothing"));
	ui.cbLoadOnStart->addItem(i18n("Create new empty project"));
	ui.cbLoadOnStart->addItem(i18n("Create new project with worksheet"));
	ui.cbLoadOnStart->addItem(i18n("Load last used project"));
// 	ui.cbLoadOnStart->addItem(i18n("Show Welcome Screen"));

	ui.cbTitleBar->clear();
	ui.cbTitleBar->addItem(i18n("Show File Path"));
	ui.cbTitleBar->addItem(i18n("Show File Name"));
	ui.cbTitleBar->addItem(i18n("Show Project Name"));

	ui.cbInterface->clear();
	ui.cbInterface->addItem(i18n("Sub-window view"));
	ui.cbInterface->addItem(i18n("Tabbed view"));

	ui.cbMdiVisibility->clear();
	ui.cbMdiVisibility->addItem(i18n("Show windows of the current folder only"));
	ui.cbMdiVisibility->addItem(i18n("Show windows of the current folder and its subfolders only"));
	ui.cbMdiVisibility->addItem(i18n("Show all windows"));

	ui.cbTabPosition->clear();
	ui.cbTabPosition->addItem(i18n("Top"));
	ui.cbTabPosition->addItem(i18n("Bottom"));
	ui.cbTabPosition->addItem(i18n("Left"));
	ui.cbTabPosition->addItem(i18n("Right"));

	ui.cbUnits->addItem(i18n("Metric"));
	ui.cbUnits->addItem(i18n("Imperial"));

	ui.cbDecimalSeparator->addItem(i18n("Dot (.)"));
	ui.cbDecimalSeparator->addItem(i18n("Comma (,)"));
	ui.cbDecimalSeparator->addItem(i18n("Arabic (٫)"));
	ui.cbDecimalSeparator->addItem(i18n("Automatic"));
}

void SettingsGeneralPage::changed() {
	m_changed = true;
	emit settingsChanged();
}

void SettingsGeneralPage::interfaceChanged(int index) {
	bool tabbedView = (index == 1);
	ui.lTabPosition->setVisible(tabbedView);
	ui.cbTabPosition->setVisible(tabbedView);
	ui.lMdiVisibility->setVisible(!tabbedView);
	ui.cbMdiVisibility->setVisible(!tabbedView);
	changed();
}

void SettingsGeneralPage::autoSaveChanged(int state) {
	const bool visible = (state == Qt::Checked);
	ui.lAutoSaveInterval->setVisible(visible);
	ui.sbAutoSaveInterval->setVisible(visible);
	changed();
}
