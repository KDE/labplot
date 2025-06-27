/*
	File                 : ScriptDock.cpp
	Project              : LabPlot
	Description          : Dock for configuring scripts
	--------------------------------------------------------------------
	SPDX-FileCopyrightText: 2025 Israel Galadima <izzygaladima@gmail.com>

	SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "ScriptDock.h"
#include "frontend/TemplateHandler.h"
#include "backend/script/Script.h"

#include <KConfig>
#include <KConfigGroup>
#include <KSyntaxHighlighting/Repository>
#include <KSyntaxHighlighting/Theme>
#include <KTextEditor/Editor>
#include <QAction>
#include <QPushButton>
#include <QActionGroup>
#include <QMenu>
#include <QContextMenuEvent>
#include <QClipboard>

ScriptDock::ScriptDock(QWidget* parent)
	: BaseDock(parent) {
	ui.setupUi(this);
	setBaseWidgets(ui.leName, ui.teComment);

	ui.tvVariables->setRootIsDecorated(false);
    ui.tvVariables->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui.tvVariables->setAlternatingRowColors(true);

	ui.tbFilterOptions->setIcon(QIcon::fromTheme(QLatin1String("configure")));

	m_caseSensitiveAction = new QAction(i18n("Case Sensitive"), this);
    m_caseSensitiveAction->setCheckable(true);
    m_caseSensitiveAction->setChecked(false);

    m_matchCompleteWordAction = new QAction(i18n("Match Complete Word"), this);
    m_matchCompleteWordAction->setCheckable(true);
    m_matchCompleteWordAction->setChecked(false);

	connect(ui.leFilter, &QLineEdit::textChanged, this, &ScriptDock::filterTextChanged);
    connect(ui.tbFilterOptions, &QPushButton::toggled, this, &ScriptDock::toggleFilterOptionsMenu);
    connect(m_caseSensitiveAction, &QAction::triggered, this, [=]() {filterTextChanged(ui.leFilter->text());});
    connect(m_matchCompleteWordAction, &QAction::triggered, this, [=]() {filterTextChanged(ui.leFilter->text());});

	// initialize the actions if not done yet
	auto* group = new QActionGroup(this);
	m_copyNameAction = new QAction(QIcon::fromTheme(QLatin1String("edit-copy")), i18n("Copy Name"), group);
	m_copyValueAction = new QAction(QIcon::fromTheme(QLatin1String("edit-copy")), i18n("Copy Value"), group);
	m_copyNameValueAction = new QAction(QIcon::fromTheme(QLatin1String("edit-copy")), i18n("Copy Name and Value"), group);
	connect(group, &QActionGroup::triggered, this, &ScriptDock::copy);
}

void ScriptDock::setScriptsList(QList<Script*> list) {
	m_scriptsList = list;
	m_script = list.first();
	setAspects(std::move(list));

	CONDITIONAL_LOCK_RETURN;

    ui.leRuntime->setText(m_script->language());
	ui.tvVariables->setModel(m_script->variableModel());
}

void ScriptDock::retranslateUi() {
}

void ScriptDock::toggleFilterOptionsMenu(bool checked) {
    if (checked) {
        QMenu menu;
        menu.addAction(m_caseSensitiveAction);
        menu.addAction(m_matchCompleteWordAction);
        connect(&menu, &QMenu::aboutToHide, ui.tbFilterOptions, &QPushButton::toggle);
        menu.exec(ui.tbFilterOptions->mapToGlobal(QPoint(0, ui.tbFilterOptions->height())));
    }
}

void ScriptDock::filterTextChanged(const QString& text) {
    auto sensitivity = m_caseSensitiveAction->isChecked() ? Qt::CaseSensitive : Qt::CaseInsensitive;
    bool matchCompleteWord = m_matchCompleteWordAction->isChecked();
    const auto* model = ui.tvVariables->model();

    for (int i = 0; i < model->rowCount(); i++) {
        const auto& child = model->index(i, 0);
        const auto& name = model->data(child).toString();
        bool visible = true;
        if (text.isEmpty())
            visible = true;
        else if (matchCompleteWord)
            visible = name.startsWith(text, sensitivity);
        else
            visible = name.contains(text, sensitivity);

        ui.tvVariables->setRowHidden(i, QModelIndex(), !visible);
    }
}

void ScriptDock::contextMenuEvent(QContextMenuEvent* event) {
    const auto& index  = ui.tvVariables->currentIndex();
    if (!index.isValid())
        return;

    QMenu menu;
    menu.addAction(m_copyNameAction);
    menu.addAction(m_copyValueAction);
    menu.addAction(m_copyNameValueAction);
    menu.exec(event->globalPos());
}

void ScriptDock::copy(const QAction* action) const {
    const auto& items = ui.tvVariables->selectionModel()->selectedIndexes();
    QString text;
    if (action == m_copyNameAction) {
        text = items.at(0).data().toString();
	} else if (action == m_copyValueAction) {
        text = items.at(1).data().toString();
        text = text.replace(QStringLiteral("; "), QStringLiteral("\n"));
    } else if (action == m_copyNameValueAction) {
        text = items.at(0).data().toString();
        text += QLatin1Char('\n') + items.at(1).data().toString();
        text = text.replace(QStringLiteral("; "), QStringLiteral("\n"));
    }

    QApplication::clipboard()->setText(text);
}
