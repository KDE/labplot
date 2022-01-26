/*
    File                 : CantorWorksheet.cpp
    Project              : LabPlot
    Description          : Aspect providing a Cantor Worksheets for Multiple backends
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2015 Garvit Khatri <garvitdelhi@gmail.com>
    SPDX-FileCopyrightText: 2016 Alexander Semke <alexander.semke@web.de>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "CantorWorksheet.h"
#include "VariableParser.h"
#include "backend/core/column/Column.h"
#include "backend/core/column/ColumnPrivate.h"
#include "backend/core/Project.h"
#include "backend/lib/XmlStreamReader.h"
#include "commonfrontend/cantorWorksheet/CantorWorksheetView.h"

#include <cantor/cantorlibs_version.h>
#include "3rdparty/cantor/cantor_part.h"
#include <cantor/worksheetaccess.h>

#ifdef HAVE_NEW_CANTOR_LIBS
#include <cantor/panelpluginhandler.h>
#include <cantor/panelplugin.h>
#else
#include "3rdparty/cantor/panelpluginhandler.h"
#include "3rdparty/cantor/panelplugin.h"
#endif

#include <QAction>
#include <QFileInfo>
#include <QModelIndex>

#include <KLocalizedString>
#include <KParts/ReadWritePart>

CantorWorksheet::CantorWorksheet(const QString &name, bool loading)
	: AbstractPart(name, AspectType::CantorWorksheet), m_backendName(name) {

	if (!loading)
		init();
}

/*!
	initializes Cantor's part and plugins
*/
bool CantorWorksheet::init(QByteArray* content) {
	DEBUG(Q_FUNC_INFO)
	KPluginLoader loader(QLatin1String("kf5/parts/cantorpart"));
	KPluginLoader oldLoader(QLatin1String("cantorpart"));	// old path
	KPluginFactory* factory = loader.factory();

	if (!factory) {	// try old path
		WARN("Failed to load Cantor plugins; file name: " << STDSTRING(loader.fileName()))
		WARN("Error message: " << STDSTRING(loader.errorString()))
		factory = oldLoader.factory();
	}

	if (!factory) {
		//we can only get to this here if we open a project having Cantor content and Cantor plugins were not found.
		//return false here, a proper error message will be created in load() and propagated further.
		WARN("Failed to load Cantor plugins; file name: " << STDSTRING(oldLoader.fileName()))
		WARN("Error message: " << STDSTRING(oldLoader.errorString()))
		m_error = i18n("Couldn't find the dynamic library 'cantorpart'. Please check your installation.");
		return false;
	} else {
		m_part = factory->create<KParts::ReadWritePart>(this, QVariantList() << m_backendName << QLatin1String("--noprogress"));
		if (!m_part) {
			WARN("Could not create the Cantor Part for backend " << STDSTRING(m_backendName))
			m_error = i18n("Couldn't find the plugin for %1. Please check your installation.", m_backendName);
			return false;
		}
		m_worksheetAccess = m_part->findChild<Cantor::WorksheetAccessInterface*>(Cantor::WorksheetAccessInterface::Name);
		if (!m_worksheetAccess)
			return false;

		//load worksheet content if available
		if (content)
			m_worksheetAccess->loadWorksheetFromByteArray(content);

		connect(m_worksheetAccess, SIGNAL(modified()), this, SLOT(modified()));

		//Cantor's session
		m_session = m_worksheetAccess->session();
		if (m_session) {
			connect(m_session, &Cantor::Session::statusChanged, this, &CantorWorksheet::statusChanged);

			//variable model
			m_variableModel = m_session->variableDataModel();
			connect(m_variableModel, &QAbstractItemModel::dataChanged, this, &CantorWorksheet::dataChanged);
			connect(m_variableModel, &QAbstractItemModel::rowsInserted, this, &CantorWorksheet::rowsInserted);
			connect(m_variableModel, &QAbstractItemModel::rowsAboutToBeRemoved, this, &CantorWorksheet::rowsAboutToBeRemoved);
			connect(m_variableModel, &QAbstractItemModel::modelReset, this, &CantorWorksheet::modelReset);
		}

		//default settings
		const KConfigGroup group = KSharedConfig::openConfig()->group(QLatin1String("Settings_Notebook"));

		//TODO: right now we don't have the direct accces to Cantor's worksheet and to all its public methods
		//and we need to go through the actions provided in cantor_part.
		//-> redesign this! expose Cantor's Worksheet directly and add more settings here.
		auto* action = m_part->action("enable_highlighting");
		if (action) {
			bool value = group.readEntry(QLatin1String("SyntaxHighlighting"), false);
			action->setChecked(value);
		}

		action = m_part->action("enable_completion");
		if (action) {
			bool value = group.readEntry(QLatin1String("SyntaxCompletion"), false);
			action->setChecked(value);
		}

		action = m_part->action("enable_expression_numbers");
		if (action) {
			bool value = group.readEntry(QLatin1String("LineNumbers"), false);
			action->setChecked(value);
		}

		action = m_part->action("enable_typesetting");
		if (action) {
			bool value = group.readEntry(QLatin1String("LatexTypesetting"), false);
			action->setChecked(value);
		}

		action = m_part->action("enable_animations");
		if (action) {
			bool value = group.readEntry(QLatin1String("Animations"), false);
			action->setChecked(value);
		}

		//bool value = group.readEntry(QLatin1String("ReevaluateEntries"), false);
		//value = group.readEntry(QLatin1String("AskConfirmation"), true);
	}

	return true;
}

const QString& CantorWorksheet::error() const {
	return m_error;
}

//SLots
void CantorWorksheet::dataChanged(const QModelIndex& index) {
	const QString& name = m_variableModel->data(m_variableModel->index(index.row(), 0)).toString();
	Column* col = child<Column>(name);
	if (!col)
		return;

	QVariant dataValue = m_variableModel->data(m_variableModel->index(index.row(), 1), 257); //Cantor::DefaultVariableModel::DataRole == 257
	if (dataValue.isNull())
		dataValue = m_variableModel->data(m_variableModel->index(index.row(), 1));

	const QString& value = dataValue.toString();
	VariableParser parser(m_backendName, value);
	if (parser.isParsed()) {
		switch(parser.dataType()) {
		case AbstractColumn::ColumnMode::Integer:
			col->setColumnMode(AbstractColumn::ColumnMode::Integer);
			col->replaceInteger(0, parser.integers());
			break;
		case AbstractColumn::ColumnMode::BigInt:
			col->setColumnMode(AbstractColumn::ColumnMode::BigInt);
			col->replaceBigInt(0, parser.bigInt());
			break;
		case AbstractColumn::ColumnMode::Double:
			col->setColumnMode(AbstractColumn::ColumnMode::Double);
			col->replaceValues(0, parser.doublePrecision());
			break;
		case AbstractColumn::ColumnMode::Month:
		case AbstractColumn::ColumnMode::Day:
		case AbstractColumn::ColumnMode::DateTime:
			col->setColumnMode(AbstractColumn::ColumnMode::DateTime);
			col->replaceDateTimes(0, parser.dateTime());
			break;
		case AbstractColumn::ColumnMode::Text:
			col->setColumnMode(AbstractColumn::ColumnMode::Text);
			col->replaceTexts(0, parser.text());
			break;
		}
	}
}

void CantorWorksheet::rowsInserted(const QModelIndex& /*parent*/, int first, int last) {
	for (int i = first; i <= last; ++i) {
		const QString& name = m_variableModel->data(m_variableModel->index(i, 0)).toString();
		QVariant dataValue = m_variableModel->data(m_variableModel->index(i, 1), 257);
		if (dataValue.isNull())
			dataValue = m_variableModel->data(m_variableModel->index(i, 1));

		const QString& value = dataValue.toString();
		VariableParser parser(m_backendName, value);

		if (parser.isParsed()) {
			Column* col = child<Column>(name);
			if (col) {
				switch(parser.dataType()) {
					case AbstractColumn::ColumnMode::Integer:
					col->setColumnMode(AbstractColumn::ColumnMode::Integer);
					col->replaceInteger(0, parser.integers());
					break;
				case AbstractColumn::ColumnMode::BigInt:
					col->setColumnMode(AbstractColumn::ColumnMode::BigInt);
					col->replaceBigInt(0, parser.bigInt());
					break;
				case AbstractColumn::ColumnMode::Double:
					col->setColumnMode(AbstractColumn::ColumnMode::Double);
					col->replaceValues(0, parser.doublePrecision());
					break;
				case AbstractColumn::ColumnMode::Month:
				case AbstractColumn::ColumnMode::Day:
				case AbstractColumn::ColumnMode::DateTime:
					col->setColumnMode(AbstractColumn::ColumnMode::DateTime);
					col->replaceDateTimes(0, parser.dateTime());
					break;
				case AbstractColumn::ColumnMode::Text:
					col->setColumnMode(AbstractColumn::ColumnMode::Text);
					col->replaceTexts(0, parser.text());
					break;
				}
			} else {
				switch(parser.dataType()) {
					case AbstractColumn::ColumnMode::Integer:
					col = new Column(name, parser.integers());
					break;
				case AbstractColumn::ColumnMode::BigInt:
					col = new Column(name, parser.bigInt());
					break;
				case AbstractColumn::ColumnMode::Double:
					col = new Column(name, parser.doublePrecision());
					break;
				case AbstractColumn::ColumnMode::Month:
				case AbstractColumn::ColumnMode::Day:
				case AbstractColumn::ColumnMode::DateTime:
					col = new Column(name, parser.dateTime(), parser.dataType());
					break;
				case AbstractColumn::ColumnMode::Text:
					col = new Column(name, parser.text());
					break;
				}
				col->setUndoAware(false);
				col->setFixed(true);
				addChild(col);

				//TODO: Cantor currently ignores the order of variables in the worksheets
				//and adds new variables at the last position in the model.
				//Fix this in Cantor and switch to insertChildBefore here later.
				//insertChildBefore(col, child<Column>(i));
			}
		} else {
			//the already existing variable doesn't contain any numerical values -> remove it
			Column* col = child<Column>(name);
			if (col)
				removeChild(col);
		}
	}

	project()->setChanged(true);
}

void CantorWorksheet::modified() {
	project()->setChanged(true);
}

void CantorWorksheet::modelReset() {
	for (auto* column : children<Column>())
		column->remove();
}

void CantorWorksheet::rowsAboutToBeRemoved(const QModelIndex& /*parent*/, int first, int last) {
	for (int i = first; i <= last; ++i) {
		const QString& name = m_variableModel->data(m_variableModel->index(first, 0)).toString();
		Column* column = child<Column>(name);
		if (column)
			column->remove();
	}
}

QList<Cantor::PanelPlugin*> CantorWorksheet::getPlugins() {
	if (!m_pluginsLoaded) {
#ifdef HAVE_NEW_CANTOR_LIBS
		auto* handler = new Cantor::PanelPluginHandler(this);
		handler->loadPlugins();
		m_plugins = handler->activePluginsForSession(m_session, Cantor::PanelPluginHandler::PanelStates());
		for (auto* plugin : m_plugins)
			plugin->connectToShell(m_part);
#else
		auto* handler = m_part->findChild<Cantor::PanelPluginHandler*>(QLatin1String("PanelPluginHandler"));
		if (!handler) {
			m_error = i18n("Couldn't find panel plugins. Please check your installation.");
			return false;
		}
		m_plugins = handler->plugins();
#endif

		m_pluginsLoaded = true;
	}

	return m_plugins;
}

KParts::ReadWritePart* CantorWorksheet::part() {
	return m_part;
}

QIcon CantorWorksheet::icon() const {
	if (m_session)
		return QIcon::fromTheme(m_session->backend()->icon());
	return QIcon();
}

QWidget* CantorWorksheet::view() const {
	if (!m_partView) {
		m_view = new CantorWorksheetView(const_cast<CantorWorksheet*>(this));
		m_view->setBaseSize(1500, 1500);
		m_partView = m_view;
		// 	connect(m_view, SIGNAL(statusInfo(QString)), this, SIGNAL(statusInfo(QString)));

		//set the current path in the session to the path of the project file
		if (m_session) {
			const Project* project = const_cast<CantorWorksheet*>(this)->project();
			const QString& fileName = project->fileName();
			if (!fileName.isEmpty()) {
				QFileInfo fi(fileName);
				m_session->setWorksheetPath(fi.filePath());
			}
		}
	}
	return m_partView;
}

//! Return a new context menu.
/**
 * The caller takes ownership of the menu.
 */
QMenu* CantorWorksheet::createContextMenu() {
	QMenu* menu = AbstractPart::createContextMenu();
	Q_ASSERT(menu);
	Q_EMIT requestProjectContextMenu(menu);
	return menu;
}

QString CantorWorksheet::backendName() {
	return this->m_backendName;
}

//TODO
bool CantorWorksheet::exportView() const {
	return false;
}

bool CantorWorksheet::printView() {
	m_part->action("file_print")->trigger();
	return true;
}

bool CantorWorksheet::printPreview() const {
	m_part->action("file_print_preview")->trigger();
	return true;
}

void CantorWorksheet::evaluate() {
	m_part->action("evaluate_worksheet")->trigger();
}

void CantorWorksheet::restart() {
	m_part->action("restart_backend")->trigger();
}

//##############################################################################
//##################  Serialization/Deserialization  ###########################
//##############################################################################

//! Save as XML
void CantorWorksheet::save(QXmlStreamWriter* writer) const{
	writer->writeStartElement("cantorWorksheet");
	writeBasicAttributes(writer);
	writeCommentElement(writer);

	//general
	writer->writeStartElement( "general" );
	writer->writeAttribute( "backend_name", m_backendName);
	//TODO: save worksheet settings
	writer->writeEndElement();

	//save the content of Cantor's worksheet
	QByteArray content = m_worksheetAccess->saveWorksheetToByteArray();
	writer->writeStartElement("worksheet");
	writer->writeAttribute("content", content.toBase64());
	writer->writeEndElement();

	//save columns(variables)
	for (auto* col : children<Column>(ChildIndexFlag::IncludeHidden))
		col->save(writer);

	writer->writeEndElement(); // close "cantorWorksheet" section
}

//! Load from XML
bool CantorWorksheet::load(XmlStreamReader* reader, bool preview) {
	//reset the status of the reader differentiating between
	//"failed because of the missing CAS" and "failed because of the broken XML"
	reader->setFailedCASMissing(false);

	if (!readBasicAttributes(reader))
		return false;

	KLocalizedString attributeWarning = ki18n("Attribute '%1' missing or empty, default value is used");
	QXmlStreamAttributes attribs;
	bool rc = false;

	while (!reader->atEnd()) {
		reader->readNext();
		if (reader->isEndElement() && reader->name() == "cantorWorksheet")
			break;

		if (!reader->isStartElement())
			continue;

		if (reader->name() == "comment") {
			if (!readCommentElement(reader))
				return false;
		} else if (!preview && reader->name() == "general") {
			attribs = reader->attributes();

			m_backendName = attribs.value("backend_name").toString().trimmed();
			if (m_backendName.isEmpty())
				reader->raiseWarning(attributeWarning.subs("backend_name").toString());
		} else if (!preview && reader->name() == "worksheet") {
			attribs = reader->attributes();

			QString str = attribs.value("content").toString().trimmed();
			if (str.isEmpty())
				reader->raiseWarning(attributeWarning.subs("content").toString());

			QByteArray content = QByteArray::fromBase64(str.toLatin1());
			rc = init(&content);
			if (!rc) {
				reader->raiseMissingCASWarning(m_backendName);

				//failed to load this object because of the missing CAS plugin
				//and not because of the broken project XML. Set this flag to
				//handle this case correctly.
				//TODO: we also can fail in the limit in cases where Cantor's content is broken
				//and not because of the missing CAS plugin. This also needs to be treated accrodingly...
				reader->setFailedCASMissing(true);
				return false;
			}
		} else if (!preview && reader->name() == "column") {
			Column* column = new Column(QString());
			column->setUndoAware(false);
			if (!column->load(reader, preview)) {
				delete column;
				return false;
			}
			column->setFixed(true);
			addChild(column);
		} else { // unknown element
			reader->raiseWarning(i18n("unknown element '%1'", reader->name().toString()));
			if (!reader->skipToEndElement()) return false;
		}
	}

	return true;
}
