/*
    File                 : AboutDialog.cpp
    Project              : LabPlot
    Description          : Custom about dialog
    --------------------------------------------------------------------
	SPDX-FileCopyrightText: 2020-2024 Stefan Gerlach <stefan.gerlach@uni.kn>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "AboutDialog.h"
#include "backend/lib/macros.h"

#include <KLocalizedString>
#include <KTitleWidget>
#include <kxmlgui_version.h>

#include <QClipboard>
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
#ifdef HAVE_MQTT
#include <QtMqtt/qtmqttversion.h>
#endif
#include <gsl/gsl_version.h>
#ifdef HAVE_LIBORIGIN
#include <OriginFile.h>
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
//TODO: #include "mcap/types.hpp"
#endif

/*!
	\class AboutDialog
	\brief Custom about dialog (not used at the moment)

	\ingroup kdefrontend
 */
//AboutDialog::AboutDialog(const KAboutData& aboutData, QWidget* parent) : QDialog(parent), aboutData(aboutData) {
AboutDialog::AboutDialog(const KAboutData& aboutData, QWidget* parent) : KAboutApplicationDialog(aboutData, parent) {

	//const auto homepage = aboutData.homepage();
	const auto homepage = QStringLiteral("https://labplot.kde.org");
	
	auto text = QStringLiteral("<a href=\"%1\">%1</a>").arg(homepage);
	auto* linkLabel = new QLabel();
	linkLabel->setOpenExternalLinks(true);
	linkLabel->setText(text.replace(QLatin1Char('\n'), QStringLiteral("<br />")));

	// button to copy config
	auto* copyButton = new QPushButton;
	copyButton->setIcon(QIcon::fromTheme(QLatin1String("edit-copy")));
        connect(copyButton, &QPushButton::clicked, this, &AboutDialog::copyConfig);

	auto* linkCopyLayout = new QHBoxLayout;
	linkCopyLayout->addWidget(linkLabel);
	linkCopyLayout->addStretch();
	linkCopyLayout->addWidget(copyButton);

	((QVBoxLayout *)layout())->insertLayout(1, linkCopyLayout);

	// when deriving from QDialog
	//init();
}

/*
 * collect all system info to show
 */
QString AboutDialog::systemInfo() {
	// build type
#ifdef NDEBUG
	const QString buildType(i18n("Release build ") + QLatin1String(GIT_COMMIT));
#else
	const QString buildType(i18n("Debug build ") + QLatin1String(GIT_COMMIT));
#endif
	QLocale locale;
	const QString numberSystemInfo{QStringLiteral("(") + i18n("Decimal point ") + QLatin1Char('\'') + QString(locale.decimalPoint()) + QLatin1String("\', ")
								   + i18n("Group separator ") + QLatin1Char('\'') + QString(locale.groupSeparator()) + QLatin1Char('\'')};

	const QString numberLocaleInfo{QStringLiteral(" ") + i18n("Decimal point ") + QLatin1Char('\'') + QString(QLocale().decimalPoint()) + QLatin1String("\', ")
								   + i18n("Group separator ") + QLatin1Char('\'') + QString(QLocale().groupSeparator()) + QLatin1String("\', ")
								   + i18n("Exponential ") + QLatin1Char('\'') + QString(QLocale().exponential()) + QLatin1String("\', ") + i18n("Zero digit ")
								   + QLatin1Char('\'') + QString(QLocale().zeroDigit()) + QLatin1String("\', ") + i18n("Percent ") + QLatin1Char('\'')
								   + QString(QLocale().percent()) + QLatin1String("\', ") + i18n("Positive/Negative sign ") + QLatin1Char('\'')
								   + QString(QLocale().positiveSign()) + QLatin1Char('\'') + QLatin1Char('/') + QLatin1Char('\'')
								   + QString(QLocale().negativeSign()) + QLatin1Char('\'')};

	// get language set in 'switch language'
	const QString configPath = QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation);
	QSettings languageoverride(configPath + QStringLiteral("/klanguageoverridesrc"), QSettings::IniFormat);
	languageoverride.beginGroup(QStringLiteral("Language"));
	QString usedLocale = languageoverride.value(qAppName(), QString()).toString(); // something like "en_US"
	if (!usedLocale.isEmpty())
		locale = QLocale(usedLocale);
	QString usedLanguage = QLocale::languageToString(locale.language()) + QStringLiteral(",") + QLocale::countryToString(locale.country());
	QString path = QProcessEnvironment::systemEnvironment().value(QLatin1String("PATH"));

	return buildType + QLatin1Char('\n')
#ifndef REPRODUCIBLE_BUILD
		+ QStringLiteral("%1, %2").arg(QLatin1String(__DATE__), QLatin1String(__TIME__)) + QLatin1Char('\n')
#endif
		+ QLatin1String("<table>")
		+ QLatin1String("<tr><td>") + i18n("System:") + QLatin1String("</td><td>") + QSysInfo::prettyProductName() + QLatin1String("</td></tr>")
		+ QLatin1String("<tr><td>") + i18n("Locale:") + QLatin1String("</td><td>") + usedLanguage + QLatin1Char(' ') + numberSystemInfo + QLatin1String("</td></tr>")
		+ QLatin1String("<tr><td>") + i18n("Number Settings:") + QLatin1String("</td><td>") + numberLocaleInfo + QLatin1String(" (") + i18n("Updated on restart") + QLatin1Char(')') + QLatin1String("</td></tr>")
		+ QLatin1String("<tr><td>") + i18n("Architecture:") + QLatin1String("</td><td>") + QSysInfo::buildAbi() + QLatin1String("</td></tr>")
		+ QLatin1String("<tr><td>") + i18n("Kernel: ") + QLatin1String("</td><td>") + QSysInfo::kernelType() + QLatin1Char(' ') + QSysInfo::kernelVersion() + QLatin1String("</td></tr>")
		+ QLatin1String("<tr><td>") +i18n("Executable Path:") + QLatin1String("</td><td>") + path + QLatin1String("</td></tr>")
		+ QLatin1String("</table>");
}

QVector<QStringList> AboutDialog::components() {
	QVector<QStringList> components;

	// compiler info
	components << (QStringList() << i18n("C++ Compiler: ") + QLatin1String(CXX_COMPILER) << i18n("C++ Compiler Flags: ") + QLatin1String(CXX_COMPILER_FLAGS) << QString() << QString());

	components << (QStringList() << i18n("QADS") << i18n("Qt Advanced Docking System") << QString() << QStringLiteral("https://github.com/githubuser0xFFFF/Qt-Advanced-Docking-System"));
#ifdef HAVE_CANTOR_LIBS
	components << (QStringList() << i18n("Cantor") << i18n("Frontend to Mathematical Applications") << QLatin1String(CANTOR_VERSION_STRING) << QStringLiteral("https://cantor.kde.org/"));
#else
	components << (QStringList() << QLatin1String("<em>") + i18n("Cantor") + QLatin1String("</em>") << i18n("missing") < QString() << QStringLiteral("https://cantor.kde.org/"));
#endif
#ifdef HAVE_POPPLER
	components << (QStringList() << i18n("Poppler") << i18n("PDF rendering library") << QLatin1String(POPPLER_VERSION) << QStringLiteral("https://poppler.freedesktop.org/"));
#else
	components << (QStringList() << QLatin1String("<em>") + i18n("Poppler") + QLatin1String("</em>") << i18n("missing") << QString() << QStringLiteral("https://poppler.freedesktop.org/"));
#endif
#ifdef HAVE_DISCOUNT
	components << (QStringList() << i18n("Discount") << i18n("Markdown markup language support") << QLatin1String(DISCOUNT_VERSION_STRING) << QStringLiteral("http://www.pell.portland.or.us/~orc/Code/discount/"));
#else
	components << (QStringList() << QLatin1String("<em>") + i18n("Discount") + QLatin1String("</em>") << i18n("missing") << QString() << QStringLiteral("http://www.pell.portland.or.us/~orc/Code/discount/"));
#endif
#ifdef HAVE_KUSERFEEDBACK
	components << (QStringList() << i18n("KUserfeedback") << i18n("Support collecting feedback from users") << QLatin1String(KUSERFEEDBACK_VERSION_STRING) << QStringLiteral("https://github.com/KDE/kuserfeedback"));
#else
	components << (QStringList() << QLatin1String("<em>") + i18n("KUserfeedback") + QLatin1String("</em>") << i18n("missing") << QString() << QStringLiteral("https://github.com/KDE/kuserfeedback"));
#endif
#ifdef HAVE_KF_SYNTAX_HIGHLIGHTING
	components << (QStringList() << i18n("KSyntaxHighlighting") << i18n("Syntax highlighting engine") << QLatin1String(KSYNTAXHIGHLIGHTING_VERSION_STRING) << QStringLiteral("https://api.kde.org/frameworks/syntax-highlighting/html/index.html"));
#else
	components << (QStringList() << QLatin1String("<em>") + i18n("KSyntaxHighlighting") + QLatin1String("</em>") << i18n("missing") << QString() << QStringLiteral("https://api.kde.org/frameworks/syntax-highlighting/html/index.html"));
#endif
#ifdef HAVE_PURPOSE
	components << (QStringList() << i18n("Purpose") << i18n("Offers available actions for a specific purpose") << QLatin1String(PURPOSE_VERSION_STRING) << QStringLiteral("https://api.kde.org/frameworks/purpose/html/index.html"));
#else
	components << (QStringList() << QLatin1String("<em>") + i18n("Purpose") + QLatin1String("</em>") << i18n("missing") << QString() << QStringLiteral("https://api.kde.org/frameworks/purpose/html/index.html"));
#endif
#ifdef HAVE_QTSERIALPORT
	components << (QStringList() << i18n("Qt SerialPort") << i18n("Serial port functionality support") << QLatin1String(QTSERIALPORT_VERSION_STR) << QStringLiteral("https://doc.qt.io/qt-6/qtserialport-index.html"));
#else
	components << (QStringList() << QLatin1String("<em>") + i18n("Qt SerialPort") + QLatin1String("</em>") << i18n("missing") << QString() << QStringLiteral("https://doc.qt.io/qt-6/qtserialport-index.html"));
#endif
#ifdef HAVE_MQTT
	components << (QStringList() << i18n("Qt MQTT") << i18n("Support data from MQTT brokers") << QLatin1String(QTMQTT_VERSION_STR) << QStringLiteral("https://doc.qt.io/qt-6/qtmqtt-index.html"));
#else
	components << (QStringList() << QLatin1String("<em>") + i18n("Qt MQTT") + QLatin1String("</em>") << i18n("missing") << QString() << QStringLiteral("https://doc.qt.io/qt-6/qtmqtt-index.html"));
#endif
// numerical libraries
	components << (QStringList() << i18n("GSL") << i18n("GNU Scientific Library") << QStringLiteral(GSL_VERSION) << QStringLiteral("https://www.gnu.org/software/gsl"));
#ifdef HAVE_FFTW3
	components << (QStringList() << i18n("FFTW3") << i18n("Fastest Fourier Transform in the West") << QLatin1String(FFTW3_VERSION_STRING) << QStringLiteral("http://fftw.org/"));
#else
	components << (QStringList() << QLatin1String("<em>") + i18n("FFTW3") + QLatin1String("</em>") << i18n("missing") << QString() << QStringLiteral("http://fftw.org/"));
#endif
#ifdef HAVE_LIBCERF
	components << (QStringList() << i18n("libcerf") << i18n("Complex error and related functions") << QLatin1String(LIBCERF_VERSION_STRING) << QStringLiteral("https://jugit.fz-juelich.de/mlz/libcerf"));
#else
	components << (QStringList() << QLatin1String("<em>") + i18n("libcerf") + QLatin1String("</em>") << i18n("missing") << QString() << QStringLiteral("https://jugit.fz-juelich.de/mlz/libcerf"));
#endif
#ifdef HAVE_EIGEN3
	components << (QStringList() << i18n("Eigen3") << i18n("C++ library for linear algebra") << QLatin1String(EIGEN3_VERSION_STRING) << QStringLiteral("https://eigen.tuxfamily.org/index.php?title=Main_Page"));
#else
	components << (QStringList() << QLatin1String("<em>") + i18n("Eigen3") + QLatin1String("</em>") << i18n("missing") << QString() << QStringLiteral("https://eigen.tuxfamily.org/index.php?title=Main_Page"));
#endif
// data formats
#ifdef HAVE_LIBORIGIN
	components << (QStringList() << i18n("liborigin") << i18n("importing Origin OPJ project files") << QLatin1String(liboriginVersionString()) << QStringLiteral("https://sourceforge.net/projects/liborigin"));
#else
	components << (QStringList() << QLatin1String("<em>") + i18n("liborigin") + QLatin1String("</em>") << i18n("missing") << QString() << QStringLiteral("https://sourceforge.net/projects/liborigin"));
#endif
#ifdef HAVE_HDF5
	components << (QStringList() << i18n("HDF5") << i18n("High-performance data management and storage suite") << QLatin1String(H5_VERS_INFO) << QStringLiteral("https://www.hdfgroup.org/solutions/hdf5"));
#else
	components << (QStringList() << QLatin1String("<em>") + i18n("HDF5") + QLatin1String("</em>") << i18n("missing") << QString() << QStringLiteral("https://www.hdfgroup.org/solutions/hdf5"));
#endif
#ifdef HAVE_NETCDF
	components << (QStringList() << i18n("NetCDF") << i18n("Network Common Data Form") << QLatin1String(NC_VERSION) << QStringLiteral("https://www.unidata.ucar.edu/software/netcdf"));
#else
	components << (QStringList() << QLatin1String("<em>") + i18n("NetCDF") + QLatin1String("</em>") << i18n("missing") << QString() << QStringLiteral("https://www.unidata.ucar.edu/software/netcdf"));
#endif
#ifdef HAVE_FITS
	components << (QStringList() << i18n("CFITSIO") << i18n("Support data files in FITS (Flexible Image Transport System) data format") << QLatin1String(CFITSIO_VERSION_STRING) << QStringLiteral("https://heasarc.gsfc.nasa.gov/fitsio"));
#else
	components << (QStringList() << QLatin1String("<em>") + i18n("CFITSIO") + QLatin1String("</em>") << i18n("missing") << QString() << QStringLiteral("https://heasarc.gsfc.nasa.gov/fitsio"));
#endif
#ifdef HAVE_READSTAT
        // TODO: version
	components << (QStringList() << i18n("ReadStat") << i18n("Read (and write) data sets from SAS, Stata, and SPSS") << QString() << QStringLiteral("https://github.com/WizardMac/ReadStat"));
#else
	components << (QStringList() << QLatin1String("<em>") + i18n("ReadStat") + QLatin1String("</em>") << i18n("missing") << QString() << QStringLiteral("https://github.com/WizardMac/ReadStat"));
#endif
#ifdef HAVE_MATIO
	components << (QStringList() << i18n("Matio") << i18n("Import binary MATLAB MAT files") << QLatin1String(MATIO_VERSION_STR) << QStringLiteral("https://github.com/tbeu/matio"));
#else
	components << (QStringList() << QLatin1String("<em>") + i18n("Matio") + QLatin1String("</em>") << i18n("missing") << QString() << QStringLiteral("https://github.com/tbeu/matio"));
#endif
#ifdef HAVE_QXLSX
	components << (QStringList() << i18n("QXlsx") << i18n("Import Excel xlsx files") << QString() << QStringLiteral("https://github.com/QtExcel/QXlsx"));
#else
	components << (QStringList() << QLatin1String("<em>") + i18n("QXlsx") + QLatin1String("</em>") << i18n("missing") << QString() << QStringLiteral("https://github.com/QtExcel/QXlsx"));
#endif
#ifdef HAVE_ORCUS
	components << (QStringList() << i18n("ORCUS") << i18n("Import ODS (Open Document Spreadsheet) files") << QLatin1String(ORCUS_VERSION_STRING) << QStringLiteral("https://orcus.readthedocs.io/en/stable/index.html"));
#else
	components << (QStringList() << QLatin1String("<em>") + i18n("ORCUS") + QLatin1String("</em>") << i18n("missing") << QString() << QStringLiteral("https://orcus.readthedocs.io/en/stable/index.html"));
#endif
#ifdef HAVE_MCAP
	components << (QStringList() << i18n("MCAP") << i18n("MCAP file support") << QString() << QStringLiteral("https://mcap.dev"));
//TODO	components << (QStringList() << i18n("MCAP") << i18n("MCAP file support") << QLatin1String(MCAP_LIBRARY_VERSION) << QStringLiteral("https://mcap.dev"));
#else
	components << (QStringList() << QLatin1String("<em>") + i18n("MCAP") + QLatin1String("</em>") << i18n("missing") << QString() << QStringLiteral("https://mcap.dev"));
#endif
#ifdef HAVE_VECTOR_BLF
	components << (QStringList() << i18n("Vector BLF") << i18n("Binary Log File (BLF) file support") << QString() << QStringLiteral("https://github.com/Technica-Engineering/vector_blf"));
#else
	components << (QStringList() << QLatin1String("<em>") + i18n("Vector BLF") + QLatin1String("</em>") << i18n("missing") << QString() << QStringLiteral("https://github.com/Technica-Engineering/vector_blf"));
#endif

	return components;

}

void AboutDialog::copyConfig() {
	QString text;
	text += QLatin1String("LabPlot ") + QLatin1String(LVERSION) + QLatin1Char('\n');

	// system info
	text += QTextDocumentFragment::fromHtml(systemInfo()).toPlainText();

	// components
	text += QLatin1String("\nComponents:\n");
	for (auto c: AboutDialog::components())
		if (c.at(1) != i18n("missing"))	// only available components
			text += c.at(0) + QLatin1Char(' ') + c.at(2) + QLatin1Char('\n');

	QApplication::clipboard()->setText(text);
}

/*
// see KAboutApplicationDialogPrivate::init()
void AboutDialog::init() {
	setWindowTitle(i18nc("@title:window", "About %1", aboutData.displayName()));

	// Set up the title widget...
	QIcon titleIcon;
	if (aboutData.programLogo().canConvert<QPixmap>()) {
		titleIcon = QIcon(aboutData.programLogo().value<QPixmap>());
	} else if (aboutData.programLogo().canConvert<QImage>()) {
		titleIcon = QIcon(QPixmap::fromImage(aboutData.programLogo().value<QImage>()));
	} else if (aboutData.programLogo().canConvert<QIcon>()) {
		titleIcon = aboutData.programLogo().value<QIcon>();
	} else {
		titleIcon = windowIcon();
	}

	auto* titleWidget = createTitleWidget(titleIcon, aboutData.displayName(), aboutData.version(), this);

	// Then the tab bar...
	QTabWidget *tabWidget = new QTabWidget;
	tabWidget->setUsesScrollButtons(false);

	// Set up the first page...
	QWidget *aboutWidget = createAboutWidget(aboutData.shortDescription(), //
			aboutData.otherText(),
			aboutData.copyrightStatement(),
			aboutData.homepage(),
			aboutData.licenses(),
			this);
	tabWidget->addTab(aboutWidget, i18nc("@title:tab", "About"));

	// Components page
	auto* componentWidget = createComponentWidget(aboutData.components(), this);
        const QString componentPageTitle = i18nc("@title:tab", "Components");
        tabWidget->addTab( componentWidget, componentPageTitle);

	// And here we go, authors page...
	const int authorCount = aboutData.authors().count();
	if (authorCount) {
		//TODO
//		auto* authorWidget = createAuthorsWidget(aboutData.authors(), aboutData.customAuthorTextEnabled(), aboutData.customAuthorRichText(), aboutData.bugAddress(),
//					this);
//		const QString authorPageTitle = i18ncp("@title:tab", "Author", "Authors", authorCount);
//		tabWidget->addTab(authorWidget, authorPageTitle);
	}

	//
	// And credits page...
	if (!aboutData.credits().isEmpty()) {
		//TODO
		//auto* creditWidget = createCreditWidget(aboutData.credits(), this);
		//tabWidget->addTab(creditWidget, i18nc("@title:tab", "Thanks To"));
	}
	// Finally, the optional translators page...
	if (!aboutData.translators().isEmpty()) {
		//TODO
		//auto* translatorWidget = createTranslatorsWidget(aboutData.translators(), this);
		//tabWidget->addTab(translatorWidget, i18nc("@title:tab", "Translation"));
	}

	auto* buttonBox = new QDialogButtonBox(this);
	buttonBox->setStandardButtons(QDialogButtonBox::Close);
	QObject::connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
	QObject::connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

	auto* layout = new QVBoxLayout(this);
        layout->addWidget(titleWidget);
	layout->addWidget(tabWidget);
	layout->addWidget(buttonBox);
}

// see KAbstractAboutDialogPrivate::createTitleWidget()
QWidget* AboutDialog::createTitleWidget(const QIcon &icon, const QString &displayName, const QString &version, QWidget *parent) {
	auto* titleWidget = new KTitleWidget(parent);
	titleWidget->setIconSize(QSize(48, 48));
	titleWidget->setIcon(icon,
			KTitleWidget::ImageLeft);
	titleWidget->setText(
			QLatin1String("<html><font size=\"5\">%1</font><br />%2</html>").arg(
				displayName, i18nc("Version version-number", "Version %1", version)));
	return titleWidget;
}

// see KAbstractAboutDialogPrivate::createAboutWidget()
QWidget* AboutDialog::createAboutWidget(const QString &shortDescription,
                                                        const QString &otherText,
                                                        const QString &copyrightStatement,
                                                        const QString &homepage,
                                                        const QList<KAboutLicense> &licenses,
                                                        QWidget *parent) {
	auto* aboutWidget = new QWidget(parent);
	auto* aboutLayout = new QVBoxLayout(aboutWidget);
	QString aboutPageText = shortDescription + QLatin1Char('\n');
	if (!otherText.isEmpty()) {
		aboutPageText += QLatin1Char('\n') + otherText + QLatin1Char('\n');
	}
	if (!copyrightStatement.isEmpty()) {
		aboutPageText += QLatin1Char('\n') + copyrightStatement + QLatin1Char('\n');
	}
	if (!homepage.isEmpty()) {
		aboutPageText += QLatin1Char('\n') + QStringLiteral("<a href=\"%1\">%1</a>").arg(
				homepage) + QLatin1Char('\n');
	}
	aboutPageText = aboutPageText.trimmed();
	auto* aboutLabel = new QLabel;
	aboutLabel->setWordWrap(true);
	aboutLabel->setOpenExternalLinks(true);
	aboutLabel->setText(aboutPageText.replace(
				QLatin1Char('\n'), QStringLiteral("<br />")));
	aboutLabel->setTextInteractionFlags(Qt::TextBrowserInteraction);
	aboutLayout->addStretch();
	aboutLayout->addWidget(aboutLabel);

	//TODO: KLicenseDialog
	Q_UNUSED(licenses)
//	const int licenseCount = licenses.count();
//	for (int i = 0; i < licenseCount; ++i) {
//		const KAboutLicense &license = licenses.at(i);
//		QLabel *showLicenseLabel = new QLabel;
//		showLicenseLabel->setText(QStringLiteral("<a href=\"%1\">%2</a>").arg(
//					QString::number(i), i18n("License: %1", license.name(KAboutLicense::FullName))));
//		showLicenseLabel->setTextInteractionFlags(Qt::TextBrowserInteraction);
//		QObject::connect(showLicenseLabel, &QLabel::linkActivated, parent,
//				[license, parent]() {
//				auto *dialog = new KLicenseDialog(license, parent);
//				dialog->show();
//				});
//		aboutLayout->addWidget(showLicenseLabel);
//	}

	aboutLayout->addStretch();
	return aboutWidget;
}

// see KAbstractAboutDialogPrivate::createComponentWidget()
QWidget* AboutDialog::createComponentWidget(const QList<KAboutComponent> &components, QWidget *parent) {
	auto* componentWidget = new QWidget(parent);
	auto* componentLayout = new QVBoxLayout(componentWidget);
	componentLayout->setContentsMargins(0, 0, 0, 0);
	auto allComponents = components;
	allComponents.prepend(KAboutComponent(i18n("The <em>%1</em> windowing system", QGuiApplication::platformName())));
	allComponents.prepend(KAboutComponent(i18n("Qt"), QString(),
			i18n("%1 (built against %2)", QString::fromLocal8Bit(qVersion()), QStringLiteral(QT_VERSION_STR)),
			QStringLiteral("https://www.qt.io/")));
	allComponents.prepend(KAboutComponent(i18n("KDE Frameworks"),
                                          QString(),
                                          QStringLiteral(KXMLGUI_VERSION_STRING),
                                          QStringLiteral("https://develop.kde.org/products/frameworks/")));

	//TODO
	//    auto* componentModel = new KDEPrivate::KAboutApplicationComponentModel(allComponents, componentWidget);
	//    auto* componentView = new KDEPrivate::KAboutApplicationListView(componentWidget);
	//    auto* componentDelegate = new KDEPrivate::KAboutApplicationComponentListDelegate(componentView, componentView);
	//    componentView->setModel(componentModel);
	//    componentView->setItemDelegate(componentDelegate);
	//    componentView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	//    componentLayout->addWidget(componentView);
    return componentWidget;
}*/
