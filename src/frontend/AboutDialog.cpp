/*
    File                 : AboutDialog.cpp
    Project              : LabPlot
    Description          : Custom about dialog
    --------------------------------------------------------------------
	SPDX-FileCopyrightText: 2020-2024 Stefan Gerlach <stefan.gerlach@uni.kn>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "AboutDialog.h"

#include <KLocalizedString>
#include <KTitleWidget>
#include <kxmlgui_version.h>

#include <QDialogButtonBox>
#include <QGuiApplication>
#include <QLabel>
#include <QLayout>
#include <QTabWidget>

/*!
	\class AboutDialog
	\brief Custom about dialog (not used at the moment)

	\ingroup kdefrontend
 */
//AboutDialog::AboutDialog(const KAboutData& aboutData, QWidget* parent) : QDialog(parent), aboutData(aboutData) {
AboutDialog::AboutDialog(const KAboutData& aboutData, QWidget* parent) : KAboutApplicationDialog(aboutData, parent) {

	// const auto homepage = aboutData.homepage();
	const auto homepage = QStringLiteral("https://labplot.kde.org");
	const auto social = QStringLiteral("https://floss.social/@LabPlot");
	const auto tube = QStringLiteral("https://tube.kockatoo.org/c/labplot");
	const auto twitter = QStringLiteral("https://twitter.com/LabPlot");
	
	QString text = QStringLiteral("<a href=\"%1\">%1</a>").arg(homepage)
			+ QLatin1Char('\n') + QStringLiteral("<a href=\"%1\">%1</a>").arg(social)
			+ QLatin1Char('\n')  + QStringLiteral("<a href=\"%1\">%1</a>").arg(tube)
			+ QLatin1Char('\n')  + QStringLiteral("<a href=\"%1\">%1</a>").arg(twitter);
	auto* linkLabel = new QLabel();
	linkLabel->setWordWrap(true);
	linkLabel->setOpenExternalLinks(true);
	linkLabel->setText(text.replace(QLatin1Char('\n'), QStringLiteral("<br />")));
	((QVBoxLayout *)layout())->insertWidget(1, linkLabel);

	// when deriving from QDialog
	//init();
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
