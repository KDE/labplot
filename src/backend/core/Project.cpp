/***************************************************************************
    File                 : Project.cpp
    Project              : LabPlot
    Description          : Represents a LabPlot project.
    --------------------------------------------------------------------
    Copyright            : (C) 2011-2014 Alexander Semke (alexander.semke@web.de)
    Copyright            : (C) 2007-2008 Tilman Benkert (thzs@gmx.net)
    Copyright            : (C) 2007 Knut Franke (knut.franke@gmx.de)
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *  This program is free software; you can redistribute it and/or modify   *
 *  it under the terms of the GNU General Public License as published by   *
 *  the Free Software Foundation; either version 2 of the License, or      *
 *  (at your option) any later version.                                    *
 *                                                                         *
 *  This program is distributed in the hope that it will be useful,        *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *  GNU General Public License for more details.                           *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the Free Software           *
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor,                    *
 *   Boston, MA  02110-1301  USA                                           *
 *                                                                         *
 ***************************************************************************/
#include "backend/core/Project.h"
#include "backend/lib/XmlStreamReader.h"
#include "backend/datasources/LiveDataSource.h"
#include "backend/spreadsheet/Spreadsheet.h"
#include "backend/worksheet/Worksheet.h"
#include "backend/worksheet/plots/cartesian/CartesianPlot.h"
#include "backend/worksheet/plots/cartesian/Histogram.h"
#include "backend/worksheet/plots/cartesian/XYEquationCurve.h"
#include "backend/worksheet/plots/cartesian/XYDataReductionCurve.h"
#include "backend/worksheet/plots/cartesian/XYDifferentiationCurve.h"
#include "backend/worksheet/plots/cartesian/XYIntegrationCurve.h"
#include "backend/worksheet/plots/cartesian/XYInterpolationCurve.h"
#include "backend/worksheet/plots/cartesian/XYSmoothCurve.h"
#include "backend/worksheet/plots/cartesian/XYFitCurve.h"
#include "backend/worksheet/plots/cartesian/XYFourierFilterCurve.h"
#include "backend/worksheet/plots/cartesian/XYFourierTransformCurve.h"
#include "backend/worksheet/plots/cartesian/Axis.h"
#include "backend/datapicker/DatapickerCurve.h"

#include <QDateTime>
#include <QFile>
#include <QMenu>
#include <QThreadPool>
#include <QUndoStack>

#include <KConfig>
#include <KConfigGroup>
#include <KFilterDev>
#include <KLocalizedString>
#include <KMessageBox>

/**
 * \class Project
 * \brief Represents a project.
 * \ingroup core
 * Project represents the root node of all objects created during the runtime of the program.
 * Manages also the undo stack.
 */

/**
 * \enum Project::MdiWindowVisibility
 * \brief MDI subwindow visibility setting
 */
/**
 * \var Project::folderOnly
 * \brief only show MDI windows corresponding to Parts in the current folder
 */
/**
 * \var Project::foldAndSubfolders
 * \brief show MDI windows corresponding to Parts in the current folder and its subfolders
 */
/**
 * \var Project::allMdiWindows
 * \brief show MDI windows for all Parts in the project simultaneously
 */

class Project::Private {
public:
	Private() :
		mdiWindowVisibility(Project::folderOnly),
		scriptingEngine(nullptr),
		version(LVERSION),
		author(QString(qgetenv("USER"))),
		modificationTime(QDateTime::currentDateTime()),
		changed(false) {
	}

	QUndoStack undo_stack;
	MdiWindowVisibility mdiWindowVisibility;
	AbstractScriptingEngine* scriptingEngine;
	QString fileName;
	QString version;
	QString author;
	QDateTime modificationTime;
	bool changed;
};

Project::Project() : Folder(i18n("Project")), d(new Private()) {
	//load default values for name, comment and author from config
	KConfig config;
	KConfigGroup group = config.group("Project");

	d->author = group.readEntry("Author", QString());

	//we don't have direct access to the members name and comment
	//->temporaly disable the undo stack and call the setters
	setUndoAware(false);
	setIsLoading(true);
	setName(group.readEntry("Name", i18n("Project")));
	setComment(group.readEntry("Comment", QString()));
	setUndoAware(true);
	setIsLoading(false);
	d->changed = false;

	// TODO: intelligent engine choosing
// 	Q_ASSERT(ScriptingEngineManager::instance()->engineNames().size() > 0);
// 	QString engine_name = ScriptingEngineManager::instance()->engineNames()[0];
// 	d->scriptingEngine = ScriptingEngineManager::instance()->engine(engine_name);

	connect(this, &Project::aspectDescriptionChanged,this, &Project::descriptionChanged);
}

Project::~Project() {
	//if the project is being closed and the live data sources still continue reading the data,
	//the dependent objects (columns, etc.), which are already deleted maybe here,  are still being notified about the changes.
	//->stop reading the live data sources prior to deleting all objects.
	for (auto* lds : children<LiveDataSource>())
		lds->pauseReading();

	//if the project is being closed, in Worksheet the scene items are being removed and the selection in the view can change.
	//don't react on these changes since this can lead crashes (worksheet object is already in the destructor).
	//->notify all worksheets about the project being closed.
	for (auto* w : children<Worksheet>(AbstractAspect::Recursive))
		w->setIsClosing();

	d->undo_stack.clear();
	delete d;
}

QUndoStack* Project::undoStack() const {
	return &d->undo_stack;
}

QMenu* Project::createContextMenu() {
	QMenu* menu = new QMenu(); // no remove action from AbstractAspect in the project context menu
	emit requestProjectContextMenu(menu);
	return menu;
}

QMenu* Project::createFolderContextMenu(const Folder* folder) {
	QMenu* menu = const_cast<Folder*>(folder)->AbstractAspect::createContextMenu();
	emit requestFolderContextMenu(folder, menu);
	return menu;
}

void Project::setMdiWindowVisibility(MdiWindowVisibility visibility) {
	d->mdiWindowVisibility = visibility;
	emit mdiWindowVisibilityChanged();
}

Project::MdiWindowVisibility Project::mdiWindowVisibility() const {
	return d->mdiWindowVisibility;
}

AbstractScriptingEngine* Project::scriptingEngine() const {
	return d->scriptingEngine;
}

CLASS_D_ACCESSOR_IMPL(Project, QString, fileName, FileName, fileName)
BASIC_D_ACCESSOR_IMPL(Project, QString, version, Version, version)
CLASS_D_ACCESSOR_IMPL(Project, QString, author, Author, author)
CLASS_D_ACCESSOR_IMPL(Project, QDateTime, modificationTime, ModificationTime, modificationTime)

void Project::setChanged(const bool value) {
	if (isLoading())
		return;

	if (value)
		emit changed();

	d->changed = value;
}

bool Project ::hasChanged() const {
	return d->changed ;
}

void Project::descriptionChanged(const AbstractAspect* aspect) {
	if (isLoading())
		return;

	if (this != aspect)
		return;

	d->changed = true;
	emit changed();
}

void Project::navigateTo(const QString& path) {
	emit requestNavigateTo(path);
}

bool Project::isLabPlotProject(const QString& fileName) {
	return fileName.endsWith(QStringLiteral(".lml"), Qt::CaseInsensitive) || fileName.endsWith(QStringLiteral(".lml.gz"), Qt::CaseInsensitive)
		|| fileName.endsWith(QStringLiteral(".lml.bz2"), Qt::CaseInsensitive) || fileName.endsWith(QStringLiteral(".lml.xz"), Qt::CaseInsensitive);
}

QString Project::supportedExtensions() {
	static const QString extensions = "*.lml *.lml.gz *.lml.bz2 *.lml.xz *.LML *.LML.GZ *.LML.BZ2 *.LML.XZ";
	return extensions;
}

//##############################################################################
//##################  Serialization/Deserialization  ###########################
//##############################################################################
/**
 * \brief Save as XML
 */
void Project::save(QXmlStreamWriter* writer) const {
	//set the version and the modification time to the current values
	d->version = LVERSION;
	d->modificationTime = QDateTime::currentDateTime();

	writer->setAutoFormatting(true);
	writer->writeStartDocument();
	writer->writeDTD("<!DOCTYPE LabPlotXML>");

	writer->writeStartElement("project");
	writer->writeAttribute("version", version());
	writer->writeAttribute("fileName", fileName());
	writer->writeAttribute("modificationTime", modificationTime().toString("yyyy-dd-MM hh:mm:ss:zzz"));
	writer->writeAttribute("author", author());
	writeBasicAttributes(writer);

	writeCommentElement(writer);

	//save all children
	for (auto* child : children<AbstractAspect>(IncludeHidden)) {
		writer->writeStartElement("child_aspect");
		child->save(writer);
		writer->writeEndElement();
	}

	//save the state of the views (visible, maximized/minimized/geometry)
	//and the state of the project explorer (expanded items, currently selected item)
	emit requestSaveState(writer);

	writer->writeEndElement();
	writer->writeEndDocument();
}

bool Project::load(const QString& filename, bool preview) {
	QIODevice *file;
	// first try gzip compression, because projects can be gzipped and end with .lml
	if (filename.endsWith(QLatin1String(".lml"), Qt::CaseInsensitive))
		file = new KCompressionDevice(filename,KFilterDev::compressionTypeForMimeType("application/x-gzip"));
	else	// opens filename using file ending
		file = new KFilterDev(filename);

	if (!file)
		file = new QFile(filename);

	if (!file->open(QIODevice::ReadOnly)) {
		KMessageBox::error(nullptr, i18n("Sorry. Could not open file for reading."));
		return false;
	}

	char c;
	bool rc = file->getChar(&c);
	if (!rc) {
		KMessageBox::error(nullptr, i18n("The project file is empty."), i18n("Error opening project"));
		file->close();
		delete file;
		return false;
	}
	file->seek(0);

	//parse XML
	XmlStreamReader reader(file);
	setIsLoading(true);
	rc = this->load(&reader, preview);
	setIsLoading(false);
	if (rc == false) {
		RESET_CURSOR;
		QString msg_text = reader.errorString();
		KMessageBox::error(nullptr, msg_text, i18n("Error when opening the project"));
		return false;
	}

	if (reader.hasWarnings()) {
		qWarning("The following problems occurred when loading the project file:");
		const QStringList& warnings = reader.warningStrings();
		for (const auto& str : warnings)
			qWarning() << qUtf8Printable(str);

//TODO: show warnings in a kind of "log window" but not in message box
// 		KMessageBox::error(this, msg, i18n("Project loading partly failed"));
	}

	file->close();
	delete file;

	return true;
}

/**
 * \brief Load from XML
 */
bool Project::load(XmlStreamReader* reader, bool preview) {
	while (!(reader->isStartDocument() || reader->atEnd()))
		reader->readNext();

	if(!(reader->atEnd())) {
		if (!reader->skipToNextTag())
			return false;

		if (reader->name() == "project") {
			QString version = reader->attributes().value("version").toString();
			if(version.isEmpty())
				reader->raiseWarning(i18n("Attribute 'version' is missing."));
			else
				d->version = version;

			if (!readBasicAttributes(reader)) return false;
			if (!readProjectAttributes(reader)) return false;

			while (!reader->atEnd()) {
				reader->readNext();

				if (reader->isEndElement()) break;

				if (reader->isStartElement()) {
					if (reader->name() == "comment") {
						if (!readCommentElement(reader))
							return false;
					} else if(reader->name() == "child_aspect") {
						if (!readChildAspectElement(reader, preview))
							return false;
					} else if(reader->name() == "state") {
						//load the state of the views (visible, maximized/minimized/geometry)
						//and the state of the project explorer (expanded items, currently selected item)
						emit requestLoadState(reader);
					} else {
						reader->raiseWarning(i18n("unknown element '%1'", reader->name().toString()));
						if (!reader->skipToEndElement()) return false;
					}
				}
			}

			//wait until all columns are decoded from base64-encoded data
			QThreadPool::globalInstance()->waitForDone();

			//everything is read now.
			//restore the pointer to the data sets (columns) in xy-curves etc.
			QVector<Column*> columns = children<Column>(AbstractAspect::Recursive);

			//xy-curves
			QVector<XYCurve*> curves = children<XYCurve>(AbstractAspect::Recursive);
			for (auto* curve : curves) {
				if (!curve) continue;
				curve->suppressRetransform(true);

				XYEquationCurve* equationCurve = dynamic_cast<XYEquationCurve*>(curve);
				XYAnalysisCurve* analysisCurve = dynamic_cast<XYAnalysisCurve*>(curve);
				if (equationCurve) {
					//curves defined by a mathematical equations recalculate their own columns on load again.
					if (!preview)
						equationCurve->recalculate();
				} else if (analysisCurve) {
					RESTORE_COLUMN_POINTER(analysisCurve, xDataColumn, XDataColumn);
					RESTORE_COLUMN_POINTER(analysisCurve, yDataColumn, YDataColumn);
					RESTORE_COLUMN_POINTER(analysisCurve, y2DataColumn, Y2DataColumn);
					XYFitCurve* fitCurve = dynamic_cast<XYFitCurve*>(curve);
					if (fitCurve) {
						RESTORE_COLUMN_POINTER(fitCurve, xErrorColumn, XErrorColumn);
						RESTORE_COLUMN_POINTER(fitCurve, yErrorColumn, YErrorColumn);
					}
				} else {
					RESTORE_COLUMN_POINTER(curve, xColumn, XColumn);
					RESTORE_COLUMN_POINTER(curve, yColumn, YColumn);
					RESTORE_COLUMN_POINTER(curve, valuesColumn, ValuesColumn);
					RESTORE_COLUMN_POINTER(curve, xErrorPlusColumn, XErrorPlusColumn);
					RESTORE_COLUMN_POINTER(curve, xErrorMinusColumn, XErrorMinusColumn);
					RESTORE_COLUMN_POINTER(curve, yErrorPlusColumn, YErrorPlusColumn);
					RESTORE_COLUMN_POINTER(curve, yErrorMinusColumn, YErrorMinusColumn);
				}
				if (dynamic_cast<XYAnalysisCurve*>(curve))
					RESTORE_POINTER(dynamic_cast<XYAnalysisCurve*>(curve), dataSourceCurve, DataSourceCurve, XYCurve, curves);

				curve->suppressRetransform(false);
			}

			//axes
			QVector<Axis*> axes = children<Axis>(AbstractAspect::Recursive);
			for (auto* axis : axes) {
				if (!axis) continue;
				RESTORE_COLUMN_POINTER(axis, majorTicksColumn, MajorTicksColumn);
				RESTORE_COLUMN_POINTER(axis, minorTicksColumn, MinorTicksColumn);
			}

			//histograms
			QVector<Histogram*> hists = children<Histogram>(AbstractAspect::Recursive);
			for (auto* hist : hists) {
				if (!hist) continue;
				RESTORE_COLUMN_POINTER(hist, dataColumn, DataColumn);
			}

			//data picker curves
			QVector<DatapickerCurve*> dataPickerCurves = children<DatapickerCurve>(AbstractAspect::Recursive);
			for (auto* dataPickerCurve : dataPickerCurves) {
				if (!dataPickerCurve) continue;
				RESTORE_COLUMN_POINTER(dataPickerCurve, posXColumn, PosXColumn);
				RESTORE_COLUMN_POINTER(dataPickerCurve, posYColumn, PosYColumn);
				RESTORE_COLUMN_POINTER(dataPickerCurve, plusDeltaXColumn, PlusDeltaXColumn);
				RESTORE_COLUMN_POINTER(dataPickerCurve, minusDeltaXColumn, MinusDeltaXColumn);
				RESTORE_COLUMN_POINTER(dataPickerCurve, plusDeltaYColumn, PlusDeltaYColumn);
				RESTORE_COLUMN_POINTER(dataPickerCurve, minusDeltaYColumn, MinusDeltaYColumn);
			}
		} else  // no project element
			reader->raiseError(i18n("no project element found"));
	} else  // no start document
		reader->raiseError(i18n("no valid XML document found"));

	if (!preview) {
		for (auto* plot : children<WorksheetElementContainer>(AbstractAspect::Recursive)) {
			plot->setIsLoading(false);
			plot->retransform();
		}
	}

	emit loaded();
	return !reader->hasError();
}

bool Project::readProjectAttributes(XmlStreamReader* reader) {
	QXmlStreamAttributes attribs = reader->attributes();
	QString str = attribs.value(reader->namespaceUri().toString(), "fileName").toString();
	if(str.isEmpty()) {
		reader->raiseError(i18n("Project file name missing."));
		return false;
	}
	d->fileName = str;

	str = attribs.value(reader->namespaceUri().toString(), "modificationTime").toString();
	QDateTime modificationTime = QDateTime::fromString(str, "yyyy-dd-MM hh:mm:ss:zzz");
	if(str.isEmpty() || !modificationTime.isValid()) {
		reader->raiseWarning(i18n("Invalid project modification time. Using current time."));
		d->modificationTime = QDateTime::currentDateTime();
	} else
		d->modificationTime = modificationTime;

	str = attribs.value(reader->namespaceUri().toString(), "author").toString();
	d->author = str;

	return true;
}
