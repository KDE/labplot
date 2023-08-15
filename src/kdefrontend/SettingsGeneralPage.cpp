/*
	File                 : SettingsGeneralPage.cpp
	Project              : LabPlot
	Description          : general settings page
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2008-2020 Alexander Semke <alexander.semke@web.de>
	SPDX-FileCopyrightText: 2020-2022 Stefan Gerlach <stefan.gerlach@uni.kn>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "SettingsGeneralPage.h"
#include "backend/lib/macros.h"
#include "kdefrontend/MainWin.h" // LoadOnStart

#include "backend/core/Settings.h"

#include <KConfigGroup>
#include <KI18n/KLocalizedString>

/**
 * \brief Page for the 'General' settings of the Labplot settings dialog.
 */
SettingsGeneralPage::SettingsGeneralPage(QWidget* parent)
	: SettingsPage(parent) {
	ui.setupUi(this);
	ui.sbAutoSaveInterval->setSuffix(i18n("min."));
	retranslateUi();

	connect(ui.cbDockWindowPositionReopen, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &SettingsGeneralPage::changed);
	connect(ui.cbLoadOnStart, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &SettingsGeneralPage::changed);
	connect(ui.cbTitleBar, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &SettingsGeneralPage::changed);
	connect(ui.cbUnits, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &SettingsGeneralPage::changed);
	connect(ui.cbDecimalSeparator, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &SettingsGeneralPage::changed);
	connect(ui.chkGUMTerms, &QCheckBox::toggled, this, &SettingsGeneralPage::changed);
	connect(ui.chkOmitGroupSeparator, &QCheckBox::toggled, this, &SettingsGeneralPage::changed);
	connect(ui.chkOmitLeadingZeroInExponent, &QCheckBox::toggled, this, &SettingsGeneralPage::changed);
	connect(ui.chkIncludeTrailingZeroesAfterDot, &QCheckBox::toggled, this, &SettingsGeneralPage::changed);
	connect(ui.chkAutoSave, &QCheckBox::toggled, this, &SettingsGeneralPage::autoSaveChanged);
	connect(ui.chkCompatible, &QCheckBox::toggled, this, &SettingsGeneralPage::changed);

	loadSettings();
	autoSaveChanged(ui.chkAutoSave->isChecked());
}

/* returns decimal separator (as SettingsGeneralPage::DecimalSeparator) of given locale (default: system setting) */
SettingsGeneralPage::DecimalSeparator SettingsGeneralPage::decimalSeparator(QLocale locale) {
	DEBUG(Q_FUNC_INFO << ", LOCALE: " << STDSTRING(locale.name()) << ", " << locale.language())
	QChar decimalPoint = locale.decimalPoint();
	DEBUG(Q_FUNC_INFO << ", SEPARATING CHAR: " << STDSTRING(QString(decimalPoint)))
	if (decimalPoint == QLatin1Char('.'))
		return DecimalSeparator::Dot;
	else if (decimalPoint == QLatin1Char(','))
		return DecimalSeparator::Comma;

	return DecimalSeparator::Arabic;
}

QLocale::Language SettingsGeneralPage::decimalSeparatorLocale() const {
	int currentIndex = ui.cbDecimalSeparator->currentIndex();
	DEBUG(Q_FUNC_INFO << ", SYSTEM LOCALE: " << STDSTRING(QLocale().name()) << ':' << QLocale().language())
	DEBUG(Q_FUNC_INFO << ", SYSTEM SEPARATING CHAR: " << STDSTRING(QString(QLocale().decimalPoint())))

	QChar groupSeparator = QLocale().groupSeparator();
	switch (currentIndex) {
	case static_cast<int>(DecimalSeparator::Dot):
		if (groupSeparator == QLocale(QLocale::Language::Zarma).groupSeparator()) // \u00a0
			return QLocale::Language::Zarma; // . \u00a0
		else if (groupSeparator == QLocale(QLocale::Language::SwissGerman).groupSeparator()) // \u2019
			return QLocale::Language::SwissGerman; // . \u2019
		else
			return QLocale::Language::C; // . ,
	case static_cast<int>(DecimalSeparator::Comma):
		if (groupSeparator == QLocale(QLocale::Language::French).groupSeparator()) // \u00a0
			return QLocale::Language::French; // , \u00a0
		else if (groupSeparator == QLocale(QLocale::Language::Walser).groupSeparator()) // \u2019
			return QLocale::Language::Walser; // , \u2019
		else
			return QLocale::Language::German; // , .
	case static_cast<int>(DecimalSeparator::Arabic):
		return QLocale::Language::Arabic; // \u066b \u066c
	default: // automatic
		return QLocale::Language::AnyLanguage;
	}
}

void SettingsGeneralPage::applySettings() {
	DEBUG(Q_FUNC_INFO)
	if (!m_changed)
		return;

	KConfigGroup group = Settings::settingsGeneral();
	group.writeEntry(QLatin1String("LoadOnStart"), ui.cbLoadOnStart->currentData().toInt());
	group.writeEntry(QLatin1String("TitleBar"), ui.cbTitleBar->currentIndex());
	group.writeEntry(QLatin1String("Units"), ui.cbUnits->currentIndex());
	if (ui.cbDecimalSeparator->currentIndex() == static_cast<int>(DecimalSeparator::Automatic)) // need to overwrite previous setting
		group.writeEntry(QLatin1String("DecimalSeparatorLocale"), static_cast<int>(QLocale::Language::AnyLanguage));
	else
		group.writeEntry(QLatin1String("DecimalSeparatorLocale"), static_cast<int>(decimalSeparatorLocale()));
	QLocale::NumberOptions numberOptions{QLocale::DefaultNumberOptions};
	if (ui.chkOmitGroupSeparator->isChecked())
		numberOptions |= QLocale::OmitGroupSeparator;
	if (ui.chkOmitLeadingZeroInExponent->isChecked())
		numberOptions |= QLocale::OmitLeadingZeroInExponent;
	if (ui.chkIncludeTrailingZeroesAfterDot->isChecked())
		numberOptions |= QLocale::IncludeTrailingZeroesAfterDot;
	group.writeEntry(QLatin1String("GUMTerms"), ui.chkGUMTerms->isChecked());
	group.writeEntry(QLatin1String("NumberOptions"), static_cast<int>(numberOptions));
	group.writeEntry(QLatin1String("AutoSave"), ui.chkAutoSave->isChecked());
	group.writeEntry(QLatin1String("AutoSaveInterval"), ui.sbAutoSaveInterval->value());
	group.writeEntry(QLatin1String("CompatibleSave"), ui.chkCompatible->isChecked());
	Settings::writeDockPosBehaviour(static_cast<Settings::DockPosBehaviour>(ui.cbDockWindowPositionReopen->currentData().toInt()));
}

void SettingsGeneralPage::restoreDefaults() {
	ui.cbLoadOnStart->setCurrentIndex(ui.cbLoadOnStart->findData(static_cast<int>(MainWin::LoadOnStart::NewProject)));
	ui.cbTitleBar->setCurrentIndex(0);
	ui.cbUnits->setCurrentIndex(0);
	ui.cbDecimalSeparator->setCurrentIndex(static_cast<int>(DecimalSeparator::Automatic));
	ui.chkGUMTerms->setChecked(false);
	ui.chkOmitGroupSeparator->setChecked(true);
	ui.chkOmitLeadingZeroInExponent->setChecked(true);
	ui.chkIncludeTrailingZeroesAfterDot->setChecked(false);
	ui.chkAutoSave->setChecked(false);
	ui.sbAutoSaveInterval->setValue(5);
	ui.chkCompatible->setChecked(false);
	ui.cbDockWindowPositionReopen->setCurrentIndex(ui.cbDockWindowPositionReopen->findData(static_cast<int>(Settings::DockPosBehaviour::AboveLastActive)));
}

void SettingsGeneralPage::loadSettings() {
	const KConfigGroup group = Settings::group(QStringLiteral("Settings_General"));
	auto loadOnStart = group.readEntry(QLatin1String("LoadOnStart"), static_cast<int>(MainWin::LoadOnStart::NewProject));
	ui.cbLoadOnStart->setCurrentIndex(ui.cbLoadOnStart->findData(loadOnStart));
	ui.cbTitleBar->setCurrentIndex(group.readEntry(QLatin1String("TitleBar"), 0));
	ui.cbUnits->setCurrentIndex(group.readEntry(QLatin1String("Units"), 0));
	// must be done, because locale.language() will return the default locale if AnyLanguage is passed
	const auto l = static_cast<QLocale::Language>(group.readEntry(QLatin1String("DecimalSeparatorLocale"), static_cast<int>(QLocale::Language::AnyLanguage)));
	QLocale locale(l);
	if (l == QLocale::Language::AnyLanguage) // no or default setting
		ui.cbDecimalSeparator->setCurrentIndex(static_cast<int>(DecimalSeparator::Automatic));
	else
		ui.cbDecimalSeparator->setCurrentIndex(static_cast<int>(decimalSeparator(locale)));
	ui.chkGUMTerms->setChecked(group.readEntry<bool>(QLatin1String("GUMTerms"), false));
	QLocale::NumberOptions numberOptions{
		static_cast<QLocale::NumberOptions>(group.readEntry(QLatin1String("NumberOptions"), static_cast<int>(QLocale::DefaultNumberOptions)))};
	if (numberOptions & QLocale::OmitGroupSeparator)
		ui.chkOmitGroupSeparator->setChecked(true);
	if (numberOptions & QLocale::OmitLeadingZeroInExponent)
		ui.chkOmitLeadingZeroInExponent->setChecked(true);
	if (numberOptions & QLocale::IncludeTrailingZeroesAfterDot)
		ui.chkIncludeTrailingZeroesAfterDot->setChecked(true);
	ui.chkAutoSave->setChecked(group.readEntry<bool>(QLatin1String("AutoSave"), false));
	ui.sbAutoSaveInterval->setValue(group.readEntry(QLatin1String("AutoSaveInterval"), 0));
	ui.chkCompatible->setChecked(group.readEntry<bool>(QLatin1String("CompatibleSave"), false));

	ui.cbDockWindowPositionReopen->setCurrentIndex(ui.cbDockWindowPositionReopen->findData(static_cast<int>(Settings::readDockPosBehaviour())));
}

void SettingsGeneralPage::retranslateUi() {
	ui.cbLoadOnStart->clear();
	ui.cbLoadOnStart->addItem(i18n("Do Nothing"), static_cast<int>(MainWin::LoadOnStart::Nothing));
	ui.cbLoadOnStart->addItem(i18n("Create New Empty Project"), static_cast<int>(MainWin::LoadOnStart::NewProject));
	ui.cbLoadOnStart->addItem(i18n("Create New Project with Worksheet"), static_cast<int>(MainWin::LoadOnStart::NewProjectWorksheet));
	ui.cbLoadOnStart->addItem(i18n("Create New Project with Spreadsheet"), static_cast<int>(MainWin::LoadOnStart::NewProjectSpreadsheet));
	ui.cbLoadOnStart->addItem(i18n("Load Last Used Project"), static_cast<int>(MainWin::LoadOnStart::LastProject));
	// 	ui.cbLoadOnStart->addItem(i18n("Show Welcome Screen"));

	ui.cbDockWindowPositionReopen->setToolTip(i18n("Controls the behavior of where the dock widgets are placed after being re-opened."));
	ui.cbDockWindowPositionReopen->clear();
	ui.cbDockWindowPositionReopen->addItem(i18n("Original Position"), static_cast<int>(Settings::DockPosBehaviour::OriginalPos));
	ui.cbDockWindowPositionReopen->addItem(i18n("On top of the last active Dock Widget"), static_cast<int>(Settings::DockPosBehaviour::AboveLastActive));

	ui.cbTitleBar->clear();
	ui.cbTitleBar->addItem(i18n("Show File Path"));
	ui.cbTitleBar->addItem(i18n("Show File Name"));
	ui.cbTitleBar->addItem(i18n("Show Project Name"));

	ui.cbUnits->addItem(i18n("Metric"));
	ui.cbUnits->addItem(i18n("Imperial"));

	ui.cbDecimalSeparator->addItem(i18n("Dot (.)"));
	ui.cbDecimalSeparator->addItem(i18n("Comma (,)"));
	ui.cbDecimalSeparator->addItem(i18n("Arabic (٫)"));
	ui.cbDecimalSeparator->addItem(i18n("Automatic"));
}

void SettingsGeneralPage::changed() {
	m_changed = true;
	Q_EMIT settingsChanged();
}

void SettingsGeneralPage::autoSaveChanged(bool state) {
	ui.lAutoSaveInterval->setVisible(state);
	ui.sbAutoSaveInterval->setVisible(state);
	changed();
}
