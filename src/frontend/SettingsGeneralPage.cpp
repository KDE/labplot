/*
	File                 : SettingsGeneralPage.cpp
	Project              : LabPlot
	Description          : general settings page
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2008-2025 Alexander Semke <alexander.semke@web.de>
	SPDX-FileCopyrightText: 2020-2022 Stefan Gerlach <stefan.gerlach@uni.kn>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "SettingsGeneralPage.h"
#ifdef HAVE_CANTOR_LIBS
#include <cantor/backend.h>
#endif
#include "frontend/MainWin.h" // LoadOnStart

#include <KConfigGroup>

/**
 * \brief Page for the 'General' settings of the Labplot settings dialog.
 */
SettingsGeneralPage::SettingsGeneralPage(QWidget* parent, const QLocale& locale)
	: SettingsPage(parent)
	, m_defaultSystemLocale(locale) {
	ui.setupUi(this);
	ui.sbAutoSaveInterval->setSuffix(i18n("min."));
#ifdef NDEBUG
	ui.chkDebugTrace->setVisible(false);
#endif
	retranslateUi();

	connect(ui.cbLoadOnStart, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &SettingsGeneralPage::loadOnStartChanged);
	connect(ui.cbNewProject, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &SettingsGeneralPage::newProjectChanged);
	connect(ui.cbNewProjectNotebook, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &SettingsGeneralPage::changed);
	connect(ui.cbDockWindowPositionReopen, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &SettingsGeneralPage::changed);
	connect(ui.cbLoadOnStart, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &SettingsGeneralPage::changed);
	connect(ui.cbTitleBar, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &SettingsGeneralPage::changed);
	connect(ui.cbUnits, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &SettingsGeneralPage::changed);
	connect(ui.cbNumberFormat, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &SettingsGeneralPage::changed);
	connect(ui.chkGUMTerms, &QCheckBox::toggled, this, &SettingsGeneralPage::changed);
	connect(ui.chkOmitGroupSeparator, &QCheckBox::toggled, this, &SettingsGeneralPage::changed);
	connect(ui.chkOmitLeadingZeroInExponent, &QCheckBox::toggled, this, &SettingsGeneralPage::changed);
	connect(ui.chkIncludeTrailingZeroesAfterDot, &QCheckBox::toggled, this, &SettingsGeneralPage::changed);
	connect(ui.chkUseHyphen, &QCheckBox::toggled, this, &SettingsGeneralPage::changed);
	connect(ui.chkAutoSave, &QCheckBox::toggled, this, &SettingsGeneralPage::autoSaveChanged);
	connect(ui.chkSaveDockStates, &QCheckBox::toggled, this, &SettingsGeneralPage::changed);
	connect(ui.chkSaveCalculations, &QCheckBox::toggled, this, &SettingsGeneralPage::changed);
	connect(ui.chkCompatible, &QCheckBox::toggled, this, &SettingsGeneralPage::changed);
	connect(ui.chkInfoTrace, &QCheckBox::toggled, this, &SettingsGeneralPage::changed);
	connect(ui.chkDebugTrace, &QCheckBox::toggled, this, &SettingsGeneralPage::changed);
	connect(ui.chkPerfTrace, &QCheckBox::toggled, this, &SettingsGeneralPage::changed);

#ifdef HAVE_CANTOR_LIBS
	for (auto* backend : Cantor::Backend::availableBackends()) {
		if (backend->isEnabled())
			ui.cbNewProjectNotebook->addItem(QIcon::fromTheme(backend->icon()), backend->name());
	}
#endif

	loadSettings();
	autoSaveChanged(ui.chkAutoSave->isChecked());
}

QLocale::Language SettingsGeneralPage::numberFormat() const {
	return static_cast<QLocale::Language>(ui.cbNumberFormat->currentData().toInt());
}

QList<Settings::Type> SettingsGeneralPage::applySettings() {
	QList<Settings::Type> changes;
	if (!m_changed)
		return changes;

	KConfigGroup group = Settings::settingsGeneral();
	group.writeEntry(QLatin1String("LoadOnStart"), ui.cbLoadOnStart->currentData().toInt());
	group.writeEntry(QLatin1String("NewProject"), ui.cbNewProject->currentData().toInt());
	group.writeEntry(QLatin1String("NewProjectNotebook"), ui.cbNewProjectNotebook->currentText());
	group.writeEntry(QLatin1String("TitleBar"), ui.cbTitleBar->currentIndex());

	// units
	if (ui.cbUnits->currentIndex() != group.readEntry(QLatin1String("Units"), 0)) {
		group.writeEntry(QLatin1String("Units"), ui.cbUnits->currentIndex());
		changes << Settings::Type::General_Units;
	}

	// number format
	if (ui.cbNumberFormat->currentData().toInt() != group.readEntry(QLatin1String("NumberFormat"), 0)) {
		group.writeEntry(QLatin1String("NumberFormat"), ui.cbNumberFormat->currentData().toInt());
		changes << Settings::Type::General_Number_Format;
	}

	// number options
	QLocale::NumberOptions numberOptions{QLocale::DefaultNumberOptions};
	if (ui.chkOmitGroupSeparator->isChecked())
		numberOptions |= QLocale::OmitGroupSeparator;
	if (ui.chkOmitLeadingZeroInExponent->isChecked())
		numberOptions |= QLocale::OmitLeadingZeroInExponent;
	if (ui.chkIncludeTrailingZeroesAfterDot->isChecked())
		numberOptions |= QLocale::IncludeTrailingZeroesAfterDot;

	if (static_cast<int>(numberOptions) != group.readEntry(QLatin1String("NumberOptions"), 0)) {
		group.writeEntry(QLatin1String("NumberOptions"), static_cast<int>(numberOptions));
		if (changes.indexOf(Settings::Type::General_Number_Format) == -1)
			changes << Settings::Type::General_Number_Format;
	}

	// minus/hyphen sign
	if (ui.chkUseHyphen->isChecked() != group.readEntry(QLatin1String("UseHyphen"), false)) {
		group.writeEntry(QLatin1String("UseHyphen"), ui.chkUseHyphen->isChecked());
		changes << Settings::Type::General_Number_Format;
	}

	group.writeEntry(QLatin1String("GUMTerms"), ui.chkGUMTerms->isChecked());
	group.writeEntry(QLatin1String("AutoSave"), ui.chkAutoSave->isChecked());
	group.writeEntry(QLatin1String("AutoSaveInterval"), ui.sbAutoSaveInterval->value());
	group.writeEntry(QLatin1String("SaveDockStates"), ui.chkSaveDockStates->isChecked());
	group.writeEntry(QLatin1String("SaveCalculations"), ui.chkSaveCalculations->isChecked());
	group.writeEntry(QLatin1String("CompatibleSave"), ui.chkCompatible->isChecked());
	const bool infoTraceEnabled = ui.chkInfoTrace->isChecked();
	group.writeEntry(QLatin1String("InfoTrace"), infoTraceEnabled);
	enableInfoTrace(infoTraceEnabled);
	const bool debugTraceEnabled = ui.chkDebugTrace->isChecked();
	group.writeEntry(QLatin1String("DebugTrace"), debugTraceEnabled);
	enableDebugTrace(debugTraceEnabled);
	const bool perfTraceEnabled = ui.chkPerfTrace->isChecked();
	group.writeEntry(QLatin1String("PerfTrace"), perfTraceEnabled);
	enablePerfTrace(perfTraceEnabled);

	Settings::writeDockPosBehavior(static_cast<Settings::DockPosBehavior>(ui.cbDockWindowPositionReopen->currentData().toInt()));

	changes << Settings::Type::General;
	return changes;
}

void SettingsGeneralPage::restoreDefaults() {
	ui.cbLoadOnStart->setCurrentIndex(ui.cbLoadOnStart->findData(static_cast<int>(MainWin::LoadOnStart::NewProject)));
	ui.cbNewProject->setCurrentIndex(ui.cbNewProject->findData(static_cast<int>(MainWin::NewProject::WithSpreadsheet)));
	ui.cbTitleBar->setCurrentIndex(0);
	ui.cbUnits->setCurrentIndex(0);
	ui.cbNumberFormat->setCurrentIndex(ui.cbNumberFormat->findData(static_cast<int>(QLocale::AnyLanguage)));
	ui.chkGUMTerms->setChecked(false);
	ui.chkOmitGroupSeparator->setChecked(true);
	ui.chkOmitLeadingZeroInExponent->setChecked(true);
	ui.chkIncludeTrailingZeroesAfterDot->setChecked(false);
	ui.chkUseHyphen->setChecked(false);
	ui.chkAutoSave->setChecked(false);
	ui.sbAutoSaveInterval->setValue(5);
	ui.chkSaveDockStates->setChecked(false);
	ui.chkSaveCalculations->setChecked(true);
	ui.chkCompatible->setChecked(false);
	ui.chkInfoTrace->setChecked(false);
	ui.chkDebugTrace->setChecked(false);
	ui.chkPerfTrace->setChecked(false);
	ui.cbDockWindowPositionReopen->setCurrentIndex(ui.cbDockWindowPositionReopen->findData(static_cast<int>(Settings::DockPosBehavior::AboveLastActive)));
}

void SettingsGeneralPage::loadSettings() {
	const auto group = Settings::settingsGeneral();

	const auto loadOnStart = group.readEntry(QLatin1String("LoadOnStart"), static_cast<int>(MainWin::LoadOnStart::NewProject));
	ui.cbLoadOnStart->setCurrentIndex(ui.cbLoadOnStart->findData(loadOnStart));
	loadOnStartChanged();

	const auto newProject = group.readEntry(QLatin1String("NewProject"), static_cast<int>(MainWin::NewProject::WithSpreadsheet));
	ui.cbNewProject->setCurrentIndex(ui.cbNewProject->findData(newProject));
	newProjectChanged(); // call it to update notebook related widgets also if the current index above was not changed (true for index=0)

#ifdef HAVE_CANTOR_LIBS
	const auto& backendName = group.readEntry(QLatin1String("LoadOnStartNotebook"), QString());
	int index = ui.cbNewProjectNotebook->findText(backendName);
	if (index == -1 && ui.cbNewProjectNotebook->count() > 0)
		ui.cbNewProjectNotebook->setCurrentIndex(0); // select the first available backend if not backend was select yet
	else
		ui.cbNewProjectNotebook->setCurrentIndex(index);
#endif

	ui.cbTitleBar->setCurrentIndex(group.readEntry(QLatin1String("TitleBar"), 0));
	ui.cbDockWindowPositionReopen->setCurrentIndex(ui.cbDockWindowPositionReopen->findData(static_cast<int>(Settings::readDockPosBehavior())));

	ui.cbUnits->setCurrentIndex(group.readEntry(QLatin1String("Units"), 0));
	ui.chkGUMTerms->setChecked(group.readEntry<bool>(QLatin1String("GUMTerms"), false));

	// number format
	const auto language = group.readEntry(QLatin1String("NumberFormat"), static_cast<int>(QLocale::AnyLanguage));
	ui.cbNumberFormat->setCurrentIndex(ui.cbNumberFormat->findData(language));

	// number options
	QLocale::NumberOptions numberOptions{
										 static_cast<QLocale::NumberOptions>(group.readEntry(QLatin1String("NumberOptions"), static_cast<int>(QLocale::DefaultNumberOptions)))};
	if (numberOptions & QLocale::OmitGroupSeparator)
		ui.chkOmitGroupSeparator->setChecked(true);
	if (numberOptions & QLocale::OmitLeadingZeroInExponent)
		ui.chkOmitLeadingZeroInExponent->setChecked(true);
	if (numberOptions & QLocale::IncludeTrailingZeroesAfterDot)
		ui.chkIncludeTrailingZeroesAfterDot->setChecked(true);

	ui.chkUseHyphen->setChecked(group.readEntry<bool>(QLatin1String("UseHyphen"), false));

	ui.chkAutoSave->setChecked(group.readEntry<bool>(QLatin1String("AutoSave"), false));
	ui.sbAutoSaveInterval->setValue(group.readEntry(QLatin1String("AutoSaveInterval"), 0));
	ui.chkSaveDockStates->setChecked(group.readEntry<bool>(QLatin1String("SaveDockStates"), false));
	ui.chkSaveCalculations->setChecked(group.readEntry<bool>(QLatin1String("SaveCalculations"), true));
	ui.chkCompatible->setChecked(group.readEntry<bool>(QLatin1String("CompatibleSave"), false));
	ui.chkInfoTrace->setChecked(group.readEntry<bool>(QLatin1String("InfoTrace"), false));
	ui.chkDebugTrace->setChecked(group.readEntry<bool>(QLatin1String("DebugTrace"), false));
	ui.chkPerfTrace->setChecked(group.readEntry<bool>(QLatin1String("PerfTrace"), false));

	m_changed = false;
}

void SettingsGeneralPage::retranslateUi() {
	ui.cbLoadOnStart->clear();
	ui.cbLoadOnStart->addItem(i18n("Create New Project"), static_cast<int>(MainWin::LoadOnStart::NewProject));
	ui.cbLoadOnStart->addItem(i18n("Load Last Used Project"), static_cast<int>(MainWin::LoadOnStart::LastProject));
	// ui.cbLoadOnStart->addItem(i18n("Show Welcome Screen"), static_cast<int>(MainWin::LoadOnStart::WelcomeScreen));

	ui.cbNewProject->clear();
	ui.cbNewProject->addItem(i18n("With Spreadsheet"), static_cast<int>(MainWin::NewProject::WithSpreadsheet));
	ui.cbNewProject->addItem(i18n("With Worksheet"), static_cast<int>(MainWin::NewProject::WithWorksheet));
	ui.cbNewProject->addItem(i18n("With Spreadsheet and Worksheet"), static_cast<int>(MainWin::NewProject::WithSpreadsheetWorksheet));
#ifdef HAVE_CANTOR_LIBS
	ui.cbNewProject->addItem(i18n("With Notebook"), static_cast<int>(MainWin::NewProject::WithNotebook));
#endif

	QString msg = i18n("Notebook type to create automatically on startup");
	ui.lNewProjectNotebook->setToolTip(msg);
	ui.cbNewProjectNotebook->setToolTip(msg);

	msg = i18n("Controls the behavior of where the dock widgets are placed after being re-opened");
	ui.lDockWindowPositionReopen->setToolTip(msg);
	ui.cbDockWindowPositionReopen->setToolTip(msg);
	ui.cbDockWindowPositionReopen->clear();
	ui.cbDockWindowPositionReopen->addItem(i18n("Original Position"), static_cast<int>(Settings::DockPosBehavior::OriginalPos));
	ui.cbDockWindowPositionReopen->addItem(i18n("On top of the last active Dock Widget"), static_cast<int>(Settings::DockPosBehavior::AboveLastActive));

	ui.cbTitleBar->clear();
	ui.cbTitleBar->addItem(i18n("Show File Path"));
	ui.cbTitleBar->addItem(i18n("Show File Name"));
	ui.cbTitleBar->addItem(i18n("Show Project Name"));

	ui.cbUnits->clear();
	ui.cbUnits->addItem(i18n("Metric"));
	ui.cbUnits->addItem(i18n("Imperial"));

	msg = i18n("Characters for the decimal and group separators defining the number format");
	ui.lNumberFormat->setToolTip(msg);
	ui.cbNumberFormat->setToolTip(msg);
	ui.cbNumberFormat->clear();
	ui.cbNumberFormat->addItem(i18n("%1 (System, %2)", m_defaultSystemLocale.toString(1000.01), QLocale::languageToString(m_defaultSystemLocale.language())), static_cast<int>(QLocale::Language::AnyLanguage));
	ui.cbNumberFormat->insertSeparator(1);
	ui.cbNumberFormat->addItem(QLatin1String("1,000.01"), static_cast<int>(QLocale::Language::English));
	ui.cbNumberFormat->addItem(QLatin1String("1.000,01"), static_cast<int>(QLocale::Language::German));
	ui.cbNumberFormat->addItem(QLatin1String("1 000.01"), static_cast<int>(QLocale::Language::Zarma));
	ui.cbNumberFormat->addItem(QLatin1String("1 000,01"), static_cast<int>(QLocale::Language::Ukrainian));
	ui.cbNumberFormat->addItem(QString::fromUtf8("1’000,01"), static_cast<int>(QLocale::Language::SwissGerman));
	ui.cbNumberFormat->addItem(QString::fromUtf8("١٬٠٠٠٫٠١"), static_cast<int>(QLocale::Language::Arabic));

	msg = i18n("Use the hyphen sign instead of the minus sign for negative numbers shown on the worksheet");
	ui.chkUseHyphen->setToolTip(msg);

	msg = i18n(
		"Use terms compliant with the <a href=\"http://www.bipm.org/utils/common/documents/jcgm/JCGM_100_2008_E.pdf\">Guide to the Expression of Uncertainty "
		"in Measurement (GUM)</a>");
	ui.chkGUMTerms->setWhatsThis(msg);

	msg = i18n(
		"Save the state (position and geometry) of the docks in the project file. \n"
		"Determines the default behavior for new projects. \n"
		"The setting can be changed for every project separately in the project properties.");
	ui.lSaveDockStates->setToolTip(msg);
	ui.chkSaveDockStates->setToolTip(msg);

	msg = i18n(
		"Save the results of the calculations in the analysis curves in the project file. \n"
		"Uncheck this option to reduce the size of the project file at costs of the longer project load times. \n"
		"Determines the default behavior for new projects. \n"
		"The setting can be changed for every project separately in the project properties.");
	ui.lSaveCalculations->setToolTip(msg);
	ui.chkSaveCalculations->setToolTip(msg);

	ui.lTracing->setToolTip(i18n("Activates additional tracing output in the terminal."));
	ui.chkInfoTrace->setToolTip(i18n("Info trace - helpful to get information and warnings when running the application."));
	ui.chkDebugTrace->setToolTip(i18n("Debug trace - helpful to diagnose the application, can have a negative impact on the performance."));
	ui.chkPerfTrace->setToolTip(i18n("Performance trace - helpful to analyze performance relevant aspects and bottlenecks."));
}

void SettingsGeneralPage::loadOnStartChanged() {
	const auto loadOnStart = static_cast<MainWin::LoadOnStart>(ui.cbLoadOnStart->currentData().toInt());
	const bool visible = (loadOnStart == MainWin::LoadOnStart::NewProject);
	ui.lNewProject->setVisible(visible);
	ui.cbNewProject->setVisible(visible);
	newProjectChanged();
}

void SettingsGeneralPage::newProjectChanged() {
	const auto newProject = static_cast<MainWin::NewProject>(ui.cbNewProject->currentData().toInt());
	const bool visible = (newProject == MainWin::NewProject::WithNotebook);
	ui.lNewProjectNotebook->setVisible(visible);
	ui.cbNewProjectNotebook->setVisible(visible);
	changed();
}

void SettingsGeneralPage::autoSaveChanged(bool state) {
	ui.lAutoSaveInterval->setVisible(state);
	ui.sbAutoSaveInterval->setVisible(state);
	changed();
}

void SettingsGeneralPage::changed() {
	m_changed = true;
	Q_EMIT settingsChanged();
}
