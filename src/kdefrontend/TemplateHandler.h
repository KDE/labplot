/*
	File                 : TemplateHandler.h
	Project              : LabPlot
	Description          : Widget for handling saving and loading of templates
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2012 Stefan Gerlach <stefan.gerlach@uni-konstanz.de>
	SPDX-FileCopyrightText: 2012-2022 Alexander Semke <alexander.semke@web.de>
	SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef TEMPLATEHANDLER_H
#define TEMPLATEHANDLER_H

#include <QWidget>

class QMenu;
class QToolButton;
class KConfig;

class TemplateHandler : public QWidget {
	Q_OBJECT

public:
	TemplateHandler(QWidget* parent, const QString& className, bool alignRight = true);
	void setClassName(const QString&);

	void setToolButtonStyle(Qt::ToolButtonStyle);
	void setSaveDefaultAvailable(bool);
	void setLoadAvailable(bool);

	KConfig config(const QString& name);
	static QString templateName(const KConfig&);
	QStringList templateNames() const;

private:
	void retranslateUi();
	bool eventFilter(QObject*, QEvent*) override;

	QString m_dirName;
	QString m_className;

	QMenu* m_textPositionMenu{nullptr};
	QToolButton* m_tbLoad;
	QToolButton* m_tbSave;
	QToolButton* m_tbSaveDefault;
	// 	QToolButton* m_tbCopy;
	// 	QToolButton* m_tbPaste;

private Q_SLOTS:
	void loadMenu();
	void saveMenu();
	void loadMenuSelected(QAction*);
	void saveMenuSelected(QAction*);
	void saveNewSelected(const QString&);
	void saveDefaults();
	void updateTextPosition(QAction*);

Q_SIGNALS:
	void loadConfigRequested(KConfig&);
	void saveConfigRequested(KConfig&);
	void info(const QString&);
};

#endif
