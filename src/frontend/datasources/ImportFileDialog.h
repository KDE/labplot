/*
	File                 : ImportFileDialog.h
	Project              : LabPlot
	Description          : import data dialog
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2008-2025 Alexander Semke <alexander.semke@web.de>
	SPDX-FileCopyrightText: 2008-2015 Stefan Gerlach <stefan.gerlach@uni.kn>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef IMPORTFILEDIALOG_H
#define IMPORTFILEDIALOG_H

#include "ImportDialog.h"
#include "backend/datasources/LiveDataSource.h"

class MainWin;
class ImportFileWidget;

class QStatusBar;

#ifdef HAVE_MQTT
class MQTTClient;
#endif

class ImportFileDialog : public ImportDialog {
	Q_OBJECT

public:
	explicit ImportFileDialog(MainWin*, bool liveDataSource = false, const QString& path = QString(), bool importDir = false);
	~ImportFileDialog() override;

	QString selectedObject() const override;
	LiveDataSource::SourceType sourceType() const;
	void importToLiveDataSource(LiveDataSource*, QStatusBar*) const;
	bool importTo(QStatusBar*) const override;
#ifdef HAVE_MQTT
	void importToMQTT(MQTTClient*) const;
#endif

private:
	ImportFileWidget* m_importFileWidget;
	bool m_showOptions{false};
	QPushButton* m_optionsButton;

	void importFile(const QString& fileName, AbstractAspect*, AbstractFileFilter*, AbstractFileFilter::ImportMode mode = AbstractFileFilter::ImportMode::Replace) const;

protected Q_SLOTS:
	void checkOkButton() override;

private Q_SLOTS:
	void toggleOptions();
	void enableImportToMatrix(const bool enable);
};

#endif // IMPORTFILEDIALOG_H
