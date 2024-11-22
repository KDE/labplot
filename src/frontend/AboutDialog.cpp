/*
    File                 : AboutDialog.cpp
    Project              : LabPlot
    Description          : Custom about dialog
    --------------------------------------------------------------------
	SPDX-FileCopyrightText: 2020-2024 Stefan Gerlach <stefan.gerlach@uni.kn>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "AboutDialog.h"

#include <klocalizedstring.h>

#include <KTitleWidget>

#include <QDialogButtonBox>
#include <QLabel>
#include <QLayout>
#include <QTabWidget>

/*!
	\class AboutDialog
	\brief Custom about dialog

	\ingroup kdefrontend
 */
AboutDialog::AboutDialog(const KAboutData& aboutData, QWidget* parent) : QDialog(parent), aboutData(aboutData) {

	// test
	//QLabel* customLabel = new QLabel(i18n("Additional custom information here."), this);
        //layout()->addWidget(customLabel);

	init();

	// TODO: add library information (GSL version, etc.) in about dialog
	// TODO: custom stuff
}

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

	auto* buttonBox = new QDialogButtonBox(this);
	buttonBox->setStandardButtons(QDialogButtonBox::Close);
	QObject::connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
	QObject::connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

	auto* layout = new QVBoxLayout(this);
        layout->addWidget(titleWidget);
	layout->addWidget(tabWidget);
	layout->addWidget(buttonBox);
}

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

QWidget* AboutDialog::createAboutWidget(const QString &shortDescription,
                                                        const QString &otherText,
                                                        const QString &copyrightStatement,
                                                        const QString &homepage,
                                                        const QList<KAboutLicense> &licenses,
                                                        QWidget *parent) {
	QWidget *aboutWidget = new QWidget(parent);
	QVBoxLayout *aboutLayout = new QVBoxLayout(aboutWidget);
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
	QLabel *aboutLabel = new QLabel;
	aboutLabel->setWordWrap(true);
	aboutLabel->setOpenExternalLinks(true);
	aboutLabel->setText(aboutPageText.replace(
				QLatin1Char('\n'), QStringLiteral("<br />")));
	aboutLabel->setTextInteractionFlags(Qt::TextBrowserInteraction);
	aboutLayout->addStretch();
	aboutLayout->addWidget(aboutLabel);
	/*const int licenseCount = licenses.count();
	for (int i = 0; i < licenseCount; ++i) {
		const KAboutLicense &license = licenses.at(i);
		QLabel *showLicenseLabel = new QLabel;
		showLicenseLabel->setText(QStringLiteral("<a href=\"%1\">%2</a>").arg(
					QString::number(i), i18n("License: %1", license.name(KAboutLicense::FullName))));
		showLicenseLabel->setTextInteractionFlags(Qt::TextBrowserInteraction);
		QObject::connect(showLicenseLabel, &QLabel::linkActivated, parent,
				[license, parent]() {
				auto *dialog = new KLicenseDialog(license, parent);
				dialog->show();
				});
		aboutLayout->addWidget(showLicenseLabel);
	} */

	aboutLayout->addStretch();
	return aboutWidget;
}
