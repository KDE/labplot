/*
    File                 : AboutDialog.cpp
    Project              : LabPlot
    Description          : Custom about dialog
    --------------------------------------------------------------------
	SPDX-FileCopyrightText: 2020-2025 Stefan Gerlach <stefan.gerlach@uni.kn>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "AboutDialog.h"
#include "backend/core/Settings.h"
#include "backend/lib/macros.h"

#include <KLocalizedString>
#include <KTitleWidget>
#include <KWindowConfig>

#include <QClipboard>
#include <QDesktopServices>
#include <QDialogButtonBox>
#include <QGuiApplication>
#include <QLabel>
#include <QLayout>
#include <QProcessEnvironment>
#include <QPushButton>
#include <QSettings>
#include <QStandardPaths>
#include <QTabWidget>
#include <QTextDocumentFragment>
#include <QWindow>

#ifdef HAVE_POPPLER
#include <poppler-version.h>
#endif
#ifdef HAVE_KUSERFEEDBACK
#include <kuserfeedback_version.h>
#endif
#ifdef HAVE_KF_SYNTAX_HIGHLIGHTING
#include <ksyntaxhighlighting_version.h>
#endif
#ifdef HAVE_PURPOSE
#include <purpose_version.h>
#endif
#ifdef HAVE_QTSERIALPORT
#include <QtSerialPort/qtserialportversion.h>
#endif
#ifdef HAVE_QTSVG
#include <QtSvg/qtsvgversion.h>
#endif
#ifdef HAVE_MQTT
#include <QtMqtt/qtmqttversion.h>
#endif
#include <gsl/gsl_version.h>
#ifdef HAVE_LIBORIGIN
#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wshadow"
#endif
#include <OriginFile.h>
#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif
#endif
#ifdef HAVE_HDF5
#include <H5public.h>
#endif
#ifdef HAVE_NETCDF
#include <netcdf_meta.h>
#endif
#ifdef HAVE_MATIO
#include <matio_pubconf.h>
#endif
#ifdef HAVE_MCAP
#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wshadow"
#endif
#include "mcap/types.hpp"
#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif
#endif

/*!
	\class AboutDialog
	\brief Custom about dialog (not used at the moment)

	\ingroup frontend
 */
AboutDialog::AboutDialog(const KAboutData& aboutData, QWidget* parent) : KAboutApplicationDialog(aboutData, parent) {

	QString text = QStringLiteral("<a href=\"https://labplot.org/changelog/#%1\">").arg(QLatin1String(LVERSION)) + i18n("What's new") + QStringLiteral("</a>");
	auto* linkLabel = new QLabel();
	linkLabel->setOpenExternalLinks(true);
	linkLabel->setText(text.replace(QLatin1Char('\n'), QStringLiteral("<br />")));

	// button to copy config
	auto* copyEnvButton = new QPushButton(i18n("Copy Environment"));
	copyEnvButton->setIcon(QIcon::fromTheme(QLatin1String("edit-copy")));
	connect(copyEnvButton, &QPushButton::clicked, this, &AboutDialog::copyEnvironment);

	// button to copy citation
	auto* copyCiteButton = new QPushButton(i18n("Copy Citation"));
	copyCiteButton->setIcon(QIcon::fromTheme(QLatin1String("edit-copy")));
	connect(copyCiteButton, &QPushButton::clicked, this, &AboutDialog::copyCitation);

	auto* donateButton = new QPushButton(i18n("Donate"));
	donateButton->setIcon(QIcon::fromTheme(QLatin1String("love-symbolic")));
	connect(donateButton, &QPushButton::clicked, this, &AboutDialog::openDonateLink);

	auto* linkCopyLayout = new QHBoxLayout;
	linkCopyLayout->addWidget(linkLabel);
	linkCopyLayout->addStretch();
	linkCopyLayout->addWidget(copyEnvButton);
	linkCopyLayout->addWidget(copyCiteButton);
	linkCopyLayout->addWidget(donateButton);

	static_cast<QVBoxLayout *>(layout())->insertLayout(1, linkCopyLayout);

	// Find and hide the "Copy to Clipboard" button
	auto buttons = findChildren<QPushButton *>();
	for (auto* button : buttons) {
		if (button->text() == i18n("Copy to Clipboard"))
			button->hide();
	}

	// restore saved settings if available
	create(); // ensure there's a window created
	const auto conf = Settings::group(QStringLiteral("AboutDialog"));
	if (conf.exists()) {
		KWindowConfig::restoreWindowSize(windowHandle(), conf);
		resize(windowHandle()->size()); // workaround for QTBUG-40584
	} else
		resize(QSize(400, 600).expandedTo(minimumSize()));
}

AboutDialog::~AboutDialog() {
	// save current window size
	auto conf = Settings::group(QStringLiteral("AboutDialog"));
	KWindowConfig::saveWindowSize(windowHandle(), conf);
}

/*
 * collect all system info to show
 */
QString AboutDialog::systemInfo() {
	// build type
#ifdef NDEBUG
	const QString buildType(i18n("Release build") + QLatin1Char(' ') + QLatin1String(GIT_COMMIT));
#else
	const QString buildType(i18n("Debug build") + QLatin1Char(' ') + QLatin1String(GIT_COMMIT));
#endif

	//QLocale locale = QLocale();
	// const QString usedLanguage = QLocale::languageToString(locale.language()) + QStringLiteral(", ") + QLocale::countryToString(locale.country());

	QLocale systemLocale = QLocale::system();
	const QString systemLanguage = QLocale::languageToString(systemLocale.language()) + QStringLiteral(", ") + QLocale::countryToString(systemLocale.country());

	// different ways to get language settings
	//const QString configPath = QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation);
	//QSettings languageoverride(configPath + QStringLiteral("/klanguageoverridesrc"), QSettings::IniFormat);
	//languageoverride.beginGroup(QStringLiteral("Language"));
	//QString setLanguageString = languageoverride.value(qAppName(), QString()).toString(); // something like "en_US"
	//auto usedLocale = QLocale(setLanguageString);
	//const QString setLanguage = QLocale::languageToString(usedLocale.language()) + QStringLiteral(", ") + QLocale::countryToString(usedLocale.country());
	//if (!usedLocale.isEmpty())
	//	locale = QLocale(usedLocale);

	// read number format locale from settings (MainWin is not initialized yet)
	const auto group = Settings::group(QStringLiteral("Settings_General"));
        auto language = static_cast<QLocale::Language>(group.readEntry(QLatin1String("NumberFormat"), static_cast<int>(QLocale::Language::AnyLanguage)));
        QLocale numberLocale(language);
        // number options
        auto numberOptions = static_cast<QLocale::NumberOptions>(group.readEntry(QLatin1String("NumberOptions"), static_cast<int>(QLocale::DefaultNumberOptions)));
        numberLocale.setNumberOptions(numberOptions);
	const QString numberFormat{numberLocale.toString(1000.01)};

// not included for privacy
//	QString path = QProcessEnvironment::systemEnvironment().value(QLatin1String("PATH"));

	return buildType + QLatin1Char('\n')
#ifndef REPRODUCIBLE_BUILD
		+ QStringLiteral("%1, %2").arg(QLatin1String(__DATE__), QLatin1String(__TIME__)) + QLatin1Char('\n')
#endif
		+ QLatin1String("<table>")
		+ QLatin1String("<tr><td>") + i18n("System:") + QLatin1String(" </td><td>") + QSysInfo::prettyProductName() + QLatin1String("</td></tr>")
		+ QLatin1String("<tr><td>") + i18n("Locale:") + QLatin1String(" </td><td>") + systemLanguage + QLatin1String("</td></tr>")
		// + QLatin1String("<tr><td>") + i18n("Used Locale:") + QLatin1String(" </td><td>") + usedLanguage + QLatin1String("</td></tr>")
		+ QLatin1String("<tr><td>") + i18n("Number Format:") + QLatin1String(" </td><td>") + numberFormat + QStringLiteral(" (") + i18n("Updated on restart") + QStringLiteral(")")  + QLatin1String("</td></tr>")
		+ QLatin1String("<tr><td>") + i18n("Architecture:") + QLatin1String(" </td><td>") + QSysInfo::buildAbi() + QLatin1String("</td></tr>")
		+ QLatin1String("<tr><td>") + i18n("Kernel: ") + QLatin1String(" </td><td>") + QSysInfo::kernelType() + QLatin1Char(' ') + QSysInfo::kernelVersion() + QLatin1String("</td></tr>")
//		+ QLatin1String("<tr><td>") +i18n("Executable Path:") + QLatin1String(" </td><td>") + path + QLatin1String("</td></tr>")
		+ QLatin1String("</table>") + QLatin1Char('\n');
}

// build a formatted list of helpful links
QString AboutDialog::links() {
	QString links = QLatin1String("<table>")
		+ QLatin1String("<tr><td>") + i18n("Visit website:") + QLatin1String(" </td><td>") + QStringLiteral("<a href=\"%1\">%1</a>").arg(QStringLiteral("https://labplot.org")) + QLatin1String("</td></tr>")
		// Release notes: LINK ?
		+ QLatin1String("<tr><td>") + i18n("Watch video tutorials:") + QLatin1String(" </td><td>") + QStringLiteral("<a href=\"%1\">%1</a>").arg(QStringLiteral("https://www.youtube.com/@LabPlot")) + QLatin1String("</td></tr>")
		+ QLatin1String("<tr><td>") + i18n("Discuss on Mastodon:") + QLatin1String(" </td><td>") + QStringLiteral("<a href=\"%1\">%1</a>").arg(QStringLiteral("https://floss.social/@LabPlot")) + QLatin1String("</td></tr>")
		+ QLatin1String("<tr><td>") + i18n("Development:") + QLatin1String(" </td><td>") + QStringLiteral("<a href=\"%1\">%1</a>").arg(QStringLiteral("https://invent.kde.org/education/labplot")) + QLatin1String("</td></tr>")
		+ QLatin1String("<tr><td>") + i18n("Please report bugs to:") + QLatin1String(" </td><td>") + QStringLiteral("<a href=\"%1\">%1</a>").arg(QStringLiteral("https://bugs.kde.org"))
		+ QLatin1String("<tr><td>") + i18n("Partly funded by:") + QLatin1String(" </td><td>") + QStringLiteral("<a href=\"%1\">").arg(QStringLiteral("https://nlnet.nl/project/LabPlot")) + QStringLiteral("NGI Zero Core (NLnet)</a>")
		+ QLatin1String("</table>");

	return links;
}

QVector<QStringList> AboutDialog::components() {
	QVector<QStringList> components;

	// alphabetically
	QString version;
	QString missing = QLatin1String("<font color=\"red\">") + i18n("missing") + QLatin1String("</font>");
#ifdef HAVE_PYTHON_SCRIPTING
	version = QLatin1String(PYTHON3_VERSION_STRING);
#else
	version = missing;
#endif
	components << (QStringList() << QLatin1String("Python") << i18n("Python scripting support") << version << QStringLiteral("https://www.python.org"));
#ifdef HAVE_CANTOR_LIBS
	version = QLatin1String(CANTOR_VERSION_STRING);
#else
	version = missing;
#endif
	components << (QStringList() << QLatin1String("Cantor") << i18n("Frontend to Mathematical Applications") << version << QStringLiteral("https://cantor.kde.org/"));
#ifdef HAVE_FITS
	version = QLatin1String(CFITSIO_VERSION_STRING);
#else
	version = missing;
#endif
	components << (QStringList() << QLatin1String("CFITSIO") << i18n("Support data files in FITS (Flexible Image Transport System) data format") << version << QStringLiteral("https://heasarc.gsfc.nasa.gov/fitsio"));
#ifdef HAVE_DISCOUNT
	version = QLatin1String(DISCOUNT_VERSION_STRING);
#else
	version = missing;
#endif
	components << (QStringList() << QLatin1String("Discount") << i18n("Markdown markup language support") << version << QStringLiteral("http://www.pell.portland.or.us/~orc/Code/discount/"));
#ifdef HAVE_EIGEN3
	version = QLatin1String(EIGEN3_VERSION_STRING);
#else
	version = missing;
#endif
	components << (QStringList() << QLatin1String("Eigen3") << i18n("C++ library for linear algebra") << version << QStringLiteral("https://eigen.tuxfamily.org/index.php?title=Main_Page"));
#ifdef HAVE_FFTW3
	version = QLatin1String(FFTW3_VERSION_STRING);
#else
	version = missing;
#endif
	components << (QStringList() << QLatin1String("FFTW3") << i18n("Fastest Fourier Transform in the West") << version << QStringLiteral("http://fftw.org/"));
	components << (QStringList() << QLatin1String("GSL") << i18n("GNU Scientific Library") << QStringLiteral(GSL_VERSION) << QStringLiteral("https://www.gnu.org/software/gsl"));
#ifdef HAVE_HDF5
	version = QLatin1String(H5_VERS_INFO);
#else
	version = missing;
#endif
	components << (QStringList() << QLatin1String("HDF5") << i18n("High-performance data management and storage suite") << version << QStringLiteral("https://www.hdfgroup.org/solutions/hdf5"));
#ifdef HAVE_KF_SYNTAX_HIGHLIGHTING
	version = QLatin1String(KSYNTAXHIGHLIGHTING_VERSION_STRING);
#else
	version = missing;
#endif
	components << (QStringList() << QLatin1String("KSyntaxHighlighting") << i18n("Syntax highlighting engine") << version << QStringLiteral("https://api.kde.org/frameworks/syntax-highlighting/html/index.html"));
#ifdef HAVE_KUSERFEEDBACK
	version = QLatin1String(KUSERFEEDBACK_VERSION_STRING);
#else
	version = missing;
#endif
	components << (QStringList() << QLatin1String("KUserfeedback") << i18n("Support collecting feedback from users") << version << QStringLiteral("https://github.com/KDE/kuserfeedback"));
#ifdef HAVE_LIBCERF
	version = QLatin1String(LIBCERF_VERSION_STRING);
#else
	version = missing;
#endif
	components << (QStringList() << QLatin1String("libcerf") << i18n("Complex error and related functions") << version << QStringLiteral("https://jugit.fz-juelich.de/mlz/libcerf"));
#ifdef HAVE_LIBORIGIN
	version = QLatin1String(liboriginVersionString());
#else
	version = missing;
#endif
	components << (QStringList() << QLatin1String("liborigin") << i18n("importing Origin OPJ project files") << version << QStringLiteral("https://sourceforge.net/projects/liborigin"));
#ifdef HAVE_MATIO
	version = QLatin1String(MATIO_VERSION_STR);
#else
	version = missing;
#endif
	components << (QStringList() << QLatin1String("Matio") << i18n("Import binary MATLAB MAT files") << version << QStringLiteral("https://github.com/tbeu/matio"));
#ifdef HAVE_MCAP
	version = QLatin1String(MCAP_LIBRARY_VERSION);
#else
	version = missing;
#endif
	components << (QStringList() << QLatin1String("MCAP") << i18n("MCAP file support") << version << QStringLiteral("https://mcap.dev"));
#ifdef HAVE_NETCDF
	version = QLatin1String(NC_VERSION);
#else
	version = missing;
#endif
	components << (QStringList() << QLatin1String("NetCDF") << i18n("Network Common Data Form") << version << QStringLiteral("https://www.unidata.ucar.edu/software/netcdf"));
#ifdef HAVE_ORCUS
	version = QLatin1String(ORCUS_VERSION_STRING);
#else
	version = missing;
#endif
	components << (QStringList() << QLatin1String("ORCUS") << i18n("Import ODS (Open Document Spreadsheet) files") << version << QStringLiteral("https://orcus.readthedocs.io/en/stable/index.html"));
#ifdef HAVE_POPPLER
	version = QLatin1String(POPPLER_VERSION);
#else
	version = missing;
#endif
	components << (QStringList() << QLatin1String("Poppler") << i18n("PDF rendering library") << version << QStringLiteral("https://poppler.freedesktop.org/"));
#ifdef HAVE_PURPOSE
	version = QLatin1String(PURPOSE_VERSION_STRING);
#else
	version = missing;
#endif
	components << (QStringList() << QLatin1String("Purpose") << i18n("Offers available actions for a specific purpose") << version << QStringLiteral("https://api.kde.org/frameworks/purpose/html/index.html"));
	components << (QStringList() << QLatin1String("QADS") << i18n("Qt Advanced Docking System") << QLatin1String(QADS_VERSION_STRING) << QStringLiteral("https://github.com/githubuser0xFFFF/Qt-Advanced-Docking-System"));
#ifdef HAVE_MQTT
	version = QLatin1String(QTMQTT_VERSION_STR);
#else
	version = missing;
#endif
	components << (QStringList() << QLatin1String("Qt MQTT") << i18n("Support data from MQTT brokers") << version << QStringLiteral("https://doc.qt.io/qt-6/qtmqtt-index.html"));
#ifdef HAVE_QTSERIALPORT
	version = QLatin1String(QTSERIALPORT_VERSION_STR);
#else
	version = missing;
#endif
	components << (QStringList() << QLatin1String("Qt SerialPort") << i18n("Serial port functionality support") << version << QStringLiteral("https://doc.qt.io/qt-6/qtserialport-index.html"));
#ifdef HAVE_QTSVG
	version = QLatin1String(QTSVG_VERSION_STR);
#else
	version = missing;
#endif
	components << (QStringList() << QLatin1String("Qt Svg") << i18n("SVG export support") << version << QStringLiteral("https://doc.qt.io/qt-6/qtsvg-index.html"));
#ifdef HAVE_QXLSX
	version = QLatin1String(QXLSX_VERSION_STRING);
#else
	version = missing;
#endif
	components << (QStringList() << QLatin1String("QXlsx") << i18n("Import Excel xlsx files") << version << QStringLiteral("https://github.com/QtExcel/QXlsx"));
#ifdef HAVE_READSTAT
	version = QString();    // TODO
#else
	version = missing;
#endif
	components << (QStringList() << QLatin1String("ReadStat") << i18n("Read (and write) data sets from SAS, Stata, and SPSS") << version << QStringLiteral("https://github.com/WizardMac/ReadStat"));
#ifdef HAVE_VECTOR_BLF
	version = QLatin1String(VECTOR_BLF_VERSION_STRING);
#else
	version = missing;
#endif
	components << (QStringList() << QLatin1String("Vector BLF") << i18n("Binary Log File (BLF) file support") << version << QStringLiteral("https://github.com/Technica-Engineering/vector_blf"));

	// compiler info
	components << (QStringList() << i18n("C++ Compiler: ") + QLatin1String(CXX_COMPILER_ID) << QLatin1String(CXX_COMPILER_VERSION) << QString() << QString());

	// compiler flags
	auto flags = QString::fromLatin1(CXX_COMPILER_FLAGS);
	flags.replace(QLatin1String(" -"), QStringLiteral("\n-")); // add line breaks to avoid big window width
	components << (QStringList() << i18n("C++ Compiler Flags:") << flags << QString() << QString());

	return components;

}

void AboutDialog::copyEnvironment() {
	QString text;
	text += QLatin1String("LabPlot ") + QLatin1String(LVERSION) + QLatin1Char('\n');

	// system info
	text += QTextDocumentFragment::fromHtml(systemInfo()).toPlainText();

	// components
	text += QLatin1Char('\n') + i18n("Components:") + QLatin1Char('\n');
	text += i18n("Qt") + QLatin1Char(' ') + QLatin1String(QT_VERSION_STR) + QLatin1Char('\n');
	text += i18n("KDE Frameworks") + QLatin1Char(' ') + QLatin1String(KCOREADDONS_VERSION_STRING) + QLatin1Char('\n');
	for (const auto& c : AboutDialog::components()) {
		if (QTextDocumentFragment::fromHtml(c.at(2)).toPlainText() == i18n("missing"))
			text += QTextDocumentFragment::fromHtml(c.at(0)).toPlainText() + QLatin1Char(' ') + i18n("missing") + QLatin1Char('\n');
		else if (c.at(0) == i18n("C++ Compiler: ") + QLatin1String(CXX_COMPILER_ID) || c.at(0) == i18n("C++ Compiler Flags:"))
			text += c.at(0) + QLatin1Char(' ') + c.at(1) + QLatin1Char('\n');
		else
			text += c.at(0) + QLatin1Char(' ') + c.at(2) + QLatin1Char('\n');
	}

	QApplication::clipboard()->setText(text);
}

void AboutDialog::copyCitation() {
	QString text;
	text += i18n("LabPlot Team (%1), LabPlot: A FREE, open source, cross-platform Data Visualization and Analysis software accessible to everyone, (Version %2) [Computer software]. %3.", QLatin1String(YEAR), QLatin1String(LVERSION), QLatin1String("https://labplot.org"));

	QApplication::clipboard()->setText(text);
}

void AboutDialog::openDonateLink() {
	const auto donatepage = QStringLiteral("https://liberapay.com/LabPlot/donate");

	QDesktopServices::openUrl(QUrl(donatepage));
}
