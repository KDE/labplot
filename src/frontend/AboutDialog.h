/*
	File                 : AboutDialog.h
	Project              : LabPlot
	Description          : Custom about dialog
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2020-2024 Stefan Gerlach <stefan.gerlach@uni.kn>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef ABOUTDIALOG_H
#define ABOUTDIALOG_H

#include <KAboutApplicationDialog>
#include <KAboutData>

#include <QDialog>

// subclassed from KAboutApplicationDialog to add custom label (links)
// changing title or tabs needs subclassing from QDialog, but this needs reimplementing all widgets

// class AboutDialog: public QDialog {
class AboutDialog : public KAboutApplicationDialog {
	Q_OBJECT
public:
	~AboutDialog() override;
	static QString systemInfo();
	static QVector<QStringList> components(); // list of additional components
	explicit AboutDialog(const KAboutData&, QWidget*);

private Q_SLOTS:
	void copyEnvironment();
	void copyCitation();
	void openDonateLink();

	// used only when deriving from QDialog
	/*	void init();
		QWidget* createTitleWidget(const QIcon&, const QString &displayName, const QString &version, QWidget *parent);
		QWidget* createAboutWidget(const QString &shortDescription,
															const QString &otherText,
															const QString &copyrightStatement,
															const QString &homepage,
															const QList<KAboutLicense> &licenses,
															QWidget *parent);
		QWidget* createComponentWidget(const QList<KAboutComponent>&, QWidget *parent);

		KAboutData aboutData;
	*/
};

#endif
