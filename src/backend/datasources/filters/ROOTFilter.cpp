/*
    File                 : ROOTFilter.cpp
    Project              : LabPlot
    Description          : ROOT(CERN) I/O-filter
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2018 Christoph Roick <chrisito@gmx.de>
    SPDX-FileCopyrightText: 2018 Stefan Gerlach <stefan.gerlach@uni.kn>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "backend/datasources/filters/ROOTFilter.h"
#include "backend/datasources/filters/ROOTFilterPrivate.h"
#include "backend/spreadsheet/Spreadsheet.h"
#include "backend/core/column/Column.h"
#include "backend/lib/macros.h"
#include "backend/lib/XmlStreamReader.h"

#include <KLocalizedString>

#include <QDebug>
#include <QFileInfo>
#include <QSet>
#include <QStack>

#ifdef HAVE_ZIP
#include <lz4.h>
#include <zlib.h>
#endif

#include <cmath>
#include <fstream>
#include <limits>
#include <map>
#include <string>
#include <vector>

ROOTFilter::ROOTFilter():AbstractFileFilter(FileType::ROOT), d(new ROOTFilterPrivate) {}

ROOTFilter::~ROOTFilter() = default;

void ROOTFilter::readDataFromFile(const QString& fileName, AbstractDataSource* dataSource,
			AbstractFileFilter::ImportMode importMode) {
	d->readDataFromFile(fileName, dataSource, importMode);
}

void ROOTFilter::write(const QString& fileName, AbstractDataSource* dataSource) {
	d->write(fileName, dataSource);
}

void ROOTFilter::loadFilterSettings(const QString& /*filterName*/) {
}

void ROOTFilter::saveFilterSettings(const QString& /*filterName*/) const {
}

void ROOTFilter::setCurrentObject(const QString& object) {
	d->currentObject = object;
}

const QString ROOTFilter::currentObject() const {
	return d->currentObject;
}

ROOTFilter::Directory ROOTFilter::listHistograms(const QString& fileName) const {
	return d->listHistograms(fileName);
}

ROOTFilter::Directory ROOTFilter::listTrees(const QString& fileName) const {
	return d->listTrees(fileName);
}

QVector<QStringList> ROOTFilter::listLeaves(const QString& fileName, qint64 pos) const {
	return d->listLeaves(fileName, pos);
}

QVector<QStringList> ROOTFilter::previewCurrentObject(const QString& fileName, int first, int last) const {
	return d->previewCurrentObject(fileName, first, last);
}

int ROOTFilter::rowsInCurrentObject(const QString& fileName) const {
	return d->rowsInCurrentObject(fileName);
}

void ROOTFilter::setStartRow(const int s) {
	d->startRow = s;
}

int ROOTFilter::startRow() const {
	return d->startRow;
}

void ROOTFilter::setEndRow(const int e) {
	d->endRow = e;
}

int ROOTFilter::endRow() const {
	return d->endRow;
}

void ROOTFilter::setColumns(const QVector<QStringList>& columns) {
	d->columns = columns;
}

QVector<QStringList> ROOTFilter::columns() const {
	return d->columns;
}

void ROOTFilter::save(QXmlStreamWriter* writer) const {
	writer->writeStartElement("rootFilter");
	writer->writeAttribute("object", d->currentObject);
	writer->writeAttribute("startRow", QString::number(d->startRow));
	writer->writeAttribute("endRow", QString::number(d->endRow));
	for (const auto & c : d->columns) {
		writer->writeStartElement("column");
		for (const auto & s : c)
			writer->writeTextElement("id", s);
		writer->writeEndElement();
	}
	writer->writeEndElement();
}

bool ROOTFilter::load(XmlStreamReader* reader) {
	QString attributeWarning = i18n("Attribute '%1' missing or empty, default value is used");
	QXmlStreamAttributes attribs = reader->attributes();

	// read attributes
	d->currentObject = attribs.value("object").toString();
	if (d->currentObject.isEmpty())
		reader->raiseWarning(attributeWarning.arg("object"));

	QString str = attribs.value("startRow").toString();
	if (str.isEmpty())
		reader->raiseWarning(attributeWarning.arg("startRow"));
	else
		d->startRow = str.toInt();

	str = attribs.value("endRow").toString();
	if (str.isEmpty())
		reader->raiseWarning(attributeWarning.arg("endRow"));
	else
		d->endRow = str.toInt();

	d->columns.clear();
	while (reader->readNextStartElement()) {
		if (reader->name() == "column") {
			QStringList c;
			while (reader->readNextStartElement()) {
				if (reader->name() == "id")
					c << reader->readElementText();
				else
					reader->skipCurrentElement();
			}
			if (!c.empty())
				d->columns << c;
		} else
			reader->skipCurrentElement();
	}
	if (d->columns.empty())
		reader->raiseWarning(i18n("No column available"));

	return true;
}

/**************** ROOTFilterPrivate implementation *******************/

ROOTFilterPrivate::ROOTFilterPrivate() = default;

ROOTFilterPrivate::FileType ROOTFilterPrivate::currentObjectPosition(const QString& fileName, long int& pos)
{
	QStringList typeobject = currentObject.split(':');
	if (typeobject.size() < 2)
		return FileType::Invalid;

	FileType type;
	if (typeobject.first() == QStringLiteral("Hist"))
		type = FileType::Hist;
	else if (typeobject.first() == QStringLiteral("Tree"))
		type = FileType::Tree;
	else
		return FileType::Invalid;

	typeobject.removeFirst();
	QStringList path = typeobject.join(':').split('/');
	ROOTFilter::Directory dir = type == FileType::Hist ? listHistograms(fileName) : listTrees(fileName);
	const ROOTFilter::Directory* node = &dir;
	while (path.size() > 1) {
		bool next = false;
		for (const auto& child : node->children) {
			if (child.name == path.first()) {
				node = &child;
				path.pop_front();
				next = true;
				break;
			}
		}
		if (!next)
			return FileType::Invalid;
	}
	for (const auto& child : node->content) {
		if (child.first == path.first()) {
			pos = child.second;
			break;
		}
	}
	return type;
}

void ROOTFilterPrivate::readDataFromFile(const QString& fileName, AbstractDataSource* dataSource,
		AbstractFileFilter::ImportMode importMode) {
	DEBUG(Q_FUNC_INFO);

	long int pos = 0;
	auto type = currentObjectPosition(fileName, pos);
	if (pos == 0)
		return;

	if (type == FileType::Hist) {
		auto bins = readHistogram(pos);
		const int nbins = static_cast<int>(bins.size());

		// skip underflow and overflow bins by default
		int first = qMax(qAbs(startRow), 0);
		int last = endRow < 0 ? nbins - 1 : qMax(first - 1, qMin(endRow, nbins - 1));

		QStringList headers;
		for (const auto& l : columns) {
			headers << l.last();
		}

		std::vector<void*> dataContainer;
		const int columnOffset = dataSource->prepareImport(dataContainer, importMode, last - first + 1, columns.size(),
			headers, QVector<AbstractColumn::ColumnMode>(columns.size(), AbstractColumn::ColumnMode::Double));

		// read data
		DEBUG("	reading " << first - last + 1 << " lines");

		int c = 0;
		auto* spreadsheet = dynamic_cast<Spreadsheet*>(dataSource);

		for (const auto& l : columns) {
			QVector<double>& container = *static_cast<QVector<double>*>(dataContainer[c]);
			if (l.first() == QStringLiteral("center")) {
				if (spreadsheet)
					spreadsheet->column(columnOffset + c)->setPlotDesignation(AbstractColumn::PlotDesignation::X);
				for (int i = first; i <= last; ++i)
					container[i - first] = (i > 0 && i < nbins - 1) ? 0.5 * (bins[i].lowedge + bins[i + 1].lowedge)
					                                                : i == 0 ? bins.front().lowedge   // -infinity
					                                                         : -bins.front().lowedge; // +infinity
			} else if (l.first() == QStringLiteral("low")) {
				if (spreadsheet)
					spreadsheet->column(columnOffset + c)->setPlotDesignation(AbstractColumn::PlotDesignation::X);
				for (int i = first; i <= last; ++i)
					container[i - first] = bins[i].lowedge;
			} else if (l.first() == QStringLiteral("content")) {
				if (spreadsheet)
					spreadsheet->column(columnOffset + c)->setPlotDesignation(AbstractColumn::PlotDesignation::Y);
				for (int i = first; i <= last; ++i)
					container[i - first] = bins[i].content;
			} else if (l.first() == QStringLiteral("error")) {
				if (spreadsheet)
					spreadsheet->column(columnOffset + c)->setPlotDesignation(AbstractColumn::PlotDesignation::YError);
				for (int i = first; i <= last; ++i)
					container[i - first] = std::sqrt(bins[i].sumw2);
			}
			++c;
		}

		dataSource->finalizeImport(columnOffset, 0, columns.size() - 1, QString(), importMode);
	} else if (type == FileType::Tree) {
		const int nentries = static_cast<int>(currentROOTData->treeEntries(pos));

		int first = qMax(qAbs(startRow), 0);
		int last = qMax(first - 1, qMin(endRow, nentries - 1));

		QStringList headers;
		for (const auto& l : columns) {
			QString lastelement = l.back();
			bool isArray = false;
			if (lastelement.at(0) == '[' && lastelement.at(lastelement.size() - 1) == ']') {
				lastelement.midRef(1, lastelement.length() - 2).toUInt(&isArray);
			}
			if (!isArray || l.count() == 2)
				headers << l.join(isArray ? QString() : QString(':'));
			else
				headers << l.first() + QChar(':') + l.at(1) + l.back();
		}

		std::vector<void*> dataContainer;
		const int columnOffset = dataSource->prepareImport(dataContainer, importMode, last - first + 1, columns.size(),
			headers, QVector<AbstractColumn::ColumnMode>(columns.size(), AbstractColumn::ColumnMode::Double));

		int c = 0;
		for (const auto& l : columns) {
			unsigned int element = 0;
			QString lastelement = l.back(), leaf = l.front();
			bool isArray = false;
			if (lastelement.at(0) == '[' && lastelement.at(lastelement.size() - 1) == ']') {
				element = lastelement.midRef(1, lastelement.length() - 2).toUInt(&isArray);
				if (!isArray)
					element = 0;
				if (l.count() > 2)
					leaf = l.at(1);
			} else if (l.count() > 1)
				leaf = l.at(1);

			QVector<double>& container = *static_cast<QVector<double>*>(dataContainer[c++]);
			auto data = readTree(pos, l.first(), leaf, (int)element, last);
			for (int i = first; i <= last; ++i)
				container[i - first] = data[i];
		}

		dataSource->finalizeImport(columnOffset, 0, columns.size() - 1, QString(), importMode);
	}
}

void ROOTFilterPrivate::write(const QString& /*fileName*/, AbstractDataSource* /*dataSource*/) {
}

ROOTFilter::Directory ROOTFilterPrivate::listContent(const std::map<long int, ROOTData::Directory>& dataContent, std::string (ROOTData::*nameFunc)(long int))
{
	ROOTFilter::Directory dirs;
	QHash<const std::remove_reference<decltype(dataContent)>::type::value_type*, ROOTFilter::Directory*> filledDirs;
	for (const auto& path : dataContent) {
		if (!path.second.content.empty()) {
			QStack<decltype(filledDirs)::key_type> addpath;
			auto pos = &path;
			ROOTFilter::Directory* currentdir = &dirs;
			while (true) {
				auto it = filledDirs.find(pos);
				if (it != filledDirs.end()) {
					currentdir = it.value();
					break;
				}

				auto jt = dataContent.find(pos->second.parent);
				if (jt != dataContent.end()) {
					addpath.push(pos);
					pos = &(*jt);
				} else
					break;
			}
			while (!addpath.empty()) {
				auto pos = addpath.pop();
				ROOTFilter::Directory dir;
				dir.name = QString::fromStdString(pos->second.name);
				currentdir->children << dir;
				currentdir = &currentdir->children.last();
				filledDirs[pos] = currentdir;
			}
			for (auto hist : path.second.content) {
				auto name = ((*currentROOTData).*nameFunc)(hist);
				if (!name.empty())
					currentdir->content << qMakePair(QString::fromStdString(name), hist);
			}
		}
	}

	return dirs;
}

ROOTFilter::Directory ROOTFilterPrivate::listHistograms(const QString& fileName) {
	if (setFile(fileName))
		return listContent(currentROOTData->listHistograms(), &ROOTData::histogramName);
	else
		return ROOTFilter::Directory{};
}

ROOTFilter::Directory ROOTFilterPrivate::listTrees(const QString& fileName) {
	if (setFile(fileName))
		return listContent(currentROOTData->listTrees(), &ROOTData::treeName);
	else
		return ROOTFilter::Directory{};
}

QVector<QStringList> ROOTFilterPrivate::listLeaves(const QString& fileName, quint64 pos) {
	QVector<QStringList> leafList;

	if (setFile(fileName)) {
		for (const auto& leaf : currentROOTData->listLeaves(pos)) {
			leafList << QStringList(QString::fromStdString(leaf.branch));
			if (leaf.branch != leaf.leaf)
				leafList.last() << QString::fromStdString(leaf.leaf);
			if (leaf.elements > 1)
				leafList.last() << QString("[%1]").arg(leaf.elements);
		}
	}

	return leafList;
}

QVector<QStringList> ROOTFilterPrivate::previewCurrentObject(const QString& fileName, int first, int last) {
	DEBUG("ROOTFilterPrivate::previewCurrentObject()");

	long int pos = 0;
	auto type = currentObjectPosition(fileName, pos);
	if (pos == 0)
		return {1, QStringList()};

	if (type == FileType::Hist) {
		auto bins = readHistogram(pos);
		const int nbins = static_cast<int>(bins.size());

		last = qMin(nbins - 1, last);

		QVector<QStringList> preview(qMax(last - first + 2, 1));
		DEBUG("	reading " << preview.size() - 1 << " lines");

		// set headers
		for (const auto& l : columns) {
			preview.last() << l.last();
		}

		// read data
		for (const auto& l : columns) {
			if (l.first() == QStringLiteral("center")) {
				for (int i = first; i <= last; ++i)
					preview[i - first] << QString::number(
						(i > 0 && i < nbins - 1) ? 0.5 * (bins[i].lowedge + bins[i + 1].lowedge)
						                         : i == 0 ? bins.front().lowedge    // -infinity
						                                  : -bins.front().lowedge); // +infinity
			} else if (l.first() == QStringLiteral("low")) {
				for (int i = first; i <= last; ++i)
					preview[i - first] << QString::number(bins[i].lowedge);
			} else if (l.first() == QStringLiteral("content")) {
				for (int i = first; i <= last; ++i)
					preview[i - first] << QString::number(bins[i].content);
			} else if (l.first() == QStringLiteral("error")) {
				for (int i = first; i <= last; ++i)
					preview[i - first] << QString::number(std::sqrt(bins[i].sumw2));
			}
		}

		return preview;
	} else if (type == FileType::Tree) {
		last = qMin(last, currentROOTData->treeEntries(pos) - 1);

		QVector<QStringList> preview(qMax(last - first + 2, 1));
		DEBUG("	reading " << preview.size() - 1 << " lines");

		// read data leaf by leaf and set headers
		for (const auto& l : columns) {
			unsigned int element = 0;
			QString lastelement = l.back(), leaf = l.front();
			bool isArray = false;
			if (lastelement.at(0) == '[' && lastelement.at(lastelement.size() - 1) == ']') {
				element = lastelement.midRef(1, lastelement.length() - 2).toUInt(&isArray);
				if (!isArray)
					element = 0;
				if (l.count() > 2)
					leaf = l.at(1);
			} else if (l.count() > 1)
				leaf = l.at(1);

			auto data = readTree(pos, l.first(), leaf, (int)element, last);
			for (int i = first; i <= last; ++i)
				preview[i - first] << QString::number(data[i]);
			if (!isArray || l.count() == 2)
				preview.last() << l.join(isArray ? QString() : QString(':'));
			else
				preview.last() << l.first() + QChar(':') + l.at(1) + l.back();
		}

		return preview;
	} else
		return {1, QStringList()};
}

int ROOTFilterPrivate::rowsInCurrentObject(const QString& fileName) {
	long int pos = 0;
	auto type = currentObjectPosition(fileName, pos);
	if (pos == 0)
		return 0;

	switch (type) {
		case FileType::Hist:
			return currentROOTData->histogramBins(pos);
		case FileType::Tree:
			return currentROOTData->treeEntries(pos);
		case FileType::Invalid:
		default:
			return 0;
	}
}

bool ROOTFilterPrivate::setFile(const QString& fileName) {
	QFileInfo file(fileName);
	if (!file.exists()) {
		currentObject.clear();
		columns.clear();
		currentROOTData.reset();
		return false;
	}

	QDateTime modified = file.lastModified();
	qint64 size = file.size();
	if (!currentROOTData || fileName != currentFile.name
	                     || modified != currentFile.modified
	                     || size != currentFile.size) {
		currentFile.name = fileName;
		currentFile.modified = modified;
		currentFile.size = size;
		currentROOTData.reset(new ROOTData(fileName.toStdString()));
	}
	return true;
}

std::vector<ROOTData::BinPars> ROOTFilterPrivate::readHistogram(quint64 pos) {
	return currentROOTData->readHistogram(pos);
}

std::vector<double> ROOTFilterPrivate::readTree(quint64 pos, const QString& branchName, const QString& leafName, int element, int last) {
	return currentROOTData->listEntries<double>(pos, branchName.toStdString(), leafName.toStdString(), element, last + 1);
}


/******************** ROOTData implementation ************************/

namespace ROOTDataHelpers {

/// Read value from stream
template<class T>
T read(std::ifstream& is) {
	union {
		T val;
		char buf[sizeof(T)];
	} r;
	for (size_t i = 0; i < sizeof(T); ++i)
		is.get(r.buf[sizeof(T) - i - 1]);

	return r.val;
}

/// Read value from buffer
template<class T>
T read(char*& s) {
	union {
		T val;
		char buf[sizeof(T)];
	} r;
	for (size_t i = 0; i < sizeof(T); ++i)
		r.buf[sizeof(T) - i - 1] = *(s++);

	return r.val;
}

/// Read value from buffer and cast to U
template<class T, class U>
U readcast(char*& s) {
	return static_cast<U>(read<T>(s));
}

/// Get version of ROOT object, obtain number of bytes in object
short Version(char*& buffer, size_t& count) {
	// root/io/io/src/TBufferFile.cxx -> ReadVersion
	count = read<unsigned int>(buffer);
	short version = (count & 0x40000000) ? read<short>(buffer) : read<short>(buffer -= 4);
	count = (count & 0x40000000) ? (count & ~0x40000000) - 2 : 2;
	return version;
}

/// Get version of ROOT object
short Version(char*& buffer) {
	size_t c;
	return Version(buffer, c);
}

/// Skip ROOT object
void Skip(char*& buffer, size_t n) {
	for (size_t i = 0; i < n; ++i) {
		size_t count;
		Version(buffer, count);
		buffer += count;
	}
}

/// Skip TObject header
void SkipObject(char*& buffer) {
	Version(buffer);
	buffer += 8;
}

/// Get TString
std::string String(char*& buffer) {
	// root/io/io/src/TBufferFile.cxx -> ReadTString
	size_t s = *(buffer++);
	if (s == 0)
		return {};
	else {
		if (s == 0xFF)
			s = read<int>(buffer);
		buffer += s;
		return {buffer - s, buffer};
	}
}

/// Get the header of an object in TObjArray
std::string readObject(char*& buf, char* const buf0, std::map<size_t, std::string>& tags) {
	// root/io/io/src/TBufferFile.cxx -> ReadObjectAny
	std::string clname;
	unsigned int tag = read<unsigned int>(buf);
	if (tag & 0x40000000) {
		tag = read<unsigned int>(buf);
		if (tag == 0xFFFFFFFF) {
			tags[buf - buf0 - 2] = clname = buf;
			buf += clname.size() + 1;
		} else {
			clname = tags[tag & ~0x80000000];
		}
	}

	return clname;
}

}

using namespace ROOTDataHelpers;

ROOTData::ROOTData(const std::string& filename) : filename(filename) {
	// The file structure is described in root/io/io/src/TFile.cxx
	std::ifstream is(filename, std::ifstream::binary);
	std::string root(4, 0);
	is.read(const_cast<char*>(root.data()), 4);
	if (root != "root")
		return;

	int fileVersion = read<int>(is);
	long int pos = read<int>(is);
	histdirs.emplace(pos, Directory{});
	treedirs.emplace(pos, Directory{});
	long int endpos = fileVersion < 1000000 ? read<int>(is) : read<long int>(is);

	is.seekg(33);
	int compression = read<int>(is);
	compression = compression > 0 ? compression : 0;

	while (is.good() && pos < endpos) {
		is.seekg(pos);
		int lcdata = read<int>(is);
		if (lcdata == 0) {
			break;
		}
		if (lcdata < 0) {
			pos -= lcdata;
			continue;
		}
		short version = read<short>(is);
		size_t ldata = read<unsigned int>(is);
		is.seekg(4, is.cur); // skip the date
		size_t lkey = read<unsigned short int>(is);
		short cycle = read<short>(is);
		long int pseek;
		if (version > 1000) {
			is.seekg(8, is.cur);
			pseek = read<long int>(is);
		} else {
			is.seekg(4, is.cur);
			pseek = read<int>(is);
		}
		std::string cname(read<unsigned char>(is), 0);
		is.read(&cname[0], cname.size());
		std::string name(read<unsigned char>(is), 0);
		is.read(&name[0], name.size());
		std::string title(read<unsigned char>(is), 0);
		is.read(&title[0], title.size());

		ContentType type = ContentType::Invalid;
		if (cname.size() == 4 && cname.substr(0, 3) == "TH1") {
			type = histType(cname[3]);
		} else if (cname == "TTree")
			type = ContentType::Tree;
		else if (cname.substr(0, 7) == "TNtuple")
			type = ContentType::NTuple;
		else if (cname == "TBasket")
			type = ContentType::Basket;
		else if (cname == "TList" && name == "StreamerInfo")
			type = ContentType::Streamer;
		else if (cname == "TDirectory") {
			auto it = histdirs.find(pseek);
			if (it == histdirs.end())
				it = histdirs.begin();
			histdirs.emplace(pos, Directory{name, it->first});
			it = treedirs.find(pseek);
			if (it == treedirs.end())
				it = treedirs.begin();
			treedirs.emplace(pos, Directory{name, it->first});
		}

		if (type != ContentType::Invalid) {
			if (type == ContentType::Basket)
				is.seekg(19, std::ifstream::cur); // TODO read info instead?
			KeyBuffer buffer;
			buffer.type = ContentType::Invalid;
			// see root/io/io/src/TKey.cxx for reference
			int complib = 0;
			if (compression) {
				// Default: compression level
				// ZLIB: 100 + compression level
				// LZ4:  400 + compression level
				// do not rely on this, but read the header
				std::string lib(2, 0);
				is.read(&lib[0], 2);
				complib = lib == "ZL" ? 1 :
				          lib == "XZ" ? 2 :
				          lib == "CS" ? 3 :
				          lib == "L4" ? 4 : 0;
			}
			if (complib > 0) {
#			ifdef HAVE_ZIP
				// see root/core/zip/src/RZip.cxx -> R__unzip
				const int method = is.get();
				size_t chcdata = is.get();
				chcdata |= (is.get() << 8);
				chcdata |= (is.get() << 16);
				size_t chdata = is.get();
				chdata |= (is.get() << 8);
				chdata |= (is.get() << 16);

				if (chcdata == lcdata - lkey - 9 && chdata == ldata) {
					if (complib == 1 && method == Z_DEFLATED) {
						buffer = KeyBuffer{type, name, title, cycle, lkey, KeyBuffer::CompressionType::zlib,
						                   pos + lkey + 9, chcdata, chdata, 0};
					} else if (complib == 4 && method == LZ4_versionNumber() / 10000) {
						buffer = KeyBuffer{type, name, title, cycle, lkey, KeyBuffer::CompressionType::lz4,
						                   pos + lkey + 9 + 8, chcdata - 8, chdata, 0};
					}
				}
#			endif
			} else {
				buffer = KeyBuffer{type, name, title, cycle, lkey, KeyBuffer::CompressionType::none,
				                   pos + lkey, ldata, ldata, 0};
			}
			switch (buffer.type) {
				case ContentType::Basket:
					basketkeys.emplace(pos, buffer);
					break;
				case ContentType::Tree:
				case ContentType::NTuple: {
					auto it = treedirs.find(pseek);
					if (it == treedirs.end())
						it = treedirs.begin();
					bool keyreplaced = false;
					for (auto & tpos : it->second.content) {
						auto jt = treekeys.find(tpos);
						if (jt != treekeys.end() && jt->second.name == buffer.name && jt->second.cycle < buffer.cycle) {
							// override key with lower cylce number
							tpos = pos;
							treekeys.erase(jt);
							keyreplaced = true;
							break;
						}
					}
					if (!keyreplaced)
						it->second.content.push_back(pos);
					treekeys.emplace(pos, buffer);
					break;
				} case ContentType::Streamer:
					readStreamerInfo(buffer);
					break;
				case ContentType::Double: case ContentType::Float: case ContentType::Int: case ContentType::Short: case ContentType::Byte: {
					auto it = histdirs.find(pseek);
					if (it == histdirs.end())
						it = histdirs.begin();
					it->second.content.push_back(pos);
					histkeys.emplace(pos, buffer);
					break;
				} case ContentType::Invalid: case ContentType::Long: case ContentType::Bool: case ContentType::CString:
					break;
			}
		}
		pos += lcdata;
	}

	// Create default object structures if no StreamerInfo was found.
	// Obtained by running the following in ROOT with a file passed as an argument:
	//
	// _file0->GetStreamerInfoList()->Print()
	//
	// auto l = (TStreamerInfo*)_file0->GetStreamerInfoList()->At(ENTRYNUMBER);
	// l->Print();
	// for (int i = 0; i < l->GetNelement(); ++i) {
	//     auto e = l->GetElement(i);
	//     e->Print();
	//     cout << e->GetFullName() << " " << e->GetTypeName() << " " << e->GetSize() << endl;
	// }

	static const StreamerInfo dummyobject{"Object", 0, std::string(), false, false};
	if (!treekeys.empty()) {
		if (!streamerInfo.count("TTree")) {
			streamerInfo["TTree"] = {dummyobject, dummyobject, dummyobject, dummyobject,
			                         StreamerInfo{"fEntries", 8, std::string(), false, false},
			                         StreamerInfo{std::string(), 5 * 8 + 4 * 4, std::string(), false, false},
			                         StreamerInfo{"fNClusterRange", 4, std::string(), true, false},
			                         StreamerInfo{std::string(), 6 * 8, std::string(), false, false},
			                         StreamerInfo{"fNClusterRangeEnd", 8, "fNClusterRange", false, true},
			                         StreamerInfo{"fNClusterSize", 8, "fNClusterRange", false, true},
			                         StreamerInfo{"fBranches", 0, std::string(), false, false}
			};
		}
		if (!streamerInfo.count("TBranch")) {
			streamerInfo["TBranch"] = {StreamerInfo{"TNamed", 0, std::string(), false, false}, dummyobject,
			                           StreamerInfo{std::string(), 3 * 4, std::string(), false, false},
			                           StreamerInfo{"fWriteBasket", 4, std::string(), false, false},
			                           StreamerInfo{std::string(), 8 + 4, std::string(), false, false},
			                           StreamerInfo{"fMaxBaskets", 4, std::string(), true, false},
			                           StreamerInfo{std::string(), 4 + 4 * 8, std::string(), false, false},
			                           StreamerInfo{"fBranches", 0, std::string(), false, false},
			                           StreamerInfo{"fLeaves", 0, std::string(), false, false},
			                           StreamerInfo{"fBaskets", 0, std::string(), false, false},
			                           StreamerInfo{"fBasketBytes", 4, "fMaxBaskets", false, true},
			                           StreamerInfo{"fBasketEntry", 8, "fMaxBaskets", false, true},
			                           StreamerInfo{"fBasketSeek", 8, "fMaxBaskets", false, true}
			};
		}
	}
	if (!histkeys.empty()) {
		if (!streamerInfo.count("TH1")) {
			streamerInfo["TH1"] = {dummyobject, dummyobject, dummyobject, dummyobject,
			                       StreamerInfo{"fNcells", 4, std::string(), false, false},
			                       StreamerInfo{"fXaxis", 0, std::string(), false, false},
			                       StreamerInfo{"fYaxis", 0, std::string(), false, false},
			                       StreamerInfo{"fZaxis", 0, std::string(), false, false},
			                       StreamerInfo{std::string(), 2 * 2 + 8 * 8, std::string(), false, false},
			                       dummyobject,
			                       StreamerInfo{"fSumw2", 0, std::string(), false, false}};
		}
		if (!streamerInfo.count("TAxis")) {
			streamerInfo["TAxis"] = {dummyobject, dummyobject,
			                         StreamerInfo{"fNbins", 4, std::string(), false, false},
			                         StreamerInfo{"fXmin", 8, std::string(), false, false},
			                         StreamerInfo{"fXmax", 8, std::string(), false, false},
			                         StreamerInfo{"fXbins", 0, std::string(), false, false}};
		}
	}

	for (auto& tree : treekeys)
		readNEntries(tree.second);
	for (auto& hist : histkeys)
		readNBins(hist.second);
}

void ROOTData::readNBins(ROOTData::KeyBuffer& kbuffer) {
	std::string buffer = data(kbuffer);
	if (!buffer.empty()) {
		char* buf = &buffer[0];
		std::map<std::string, size_t> counts;
		Version(buf); // TH1(D/F/I/S/C)
		Version(buf); // TH1
		advanceTo(buf, streamerInfo.find("TH1")->second, std::string(), "fNcells", counts);
		kbuffer.nrows = read<int>(buf); // fNcells
	}
}

std::string ROOTData::histogramName(long int pos) {
	auto it = histkeys.find(pos);
	if (it != histkeys.end())
		return it->second.name + ';' + std::to_string(it->second.cycle);
	return {};
}

int ROOTData::histogramBins(long int pos) {
	auto it = histkeys.find(pos);
	if (it != histkeys.end())
		return it->second.nrows;
	return 0;
}

std::vector<ROOTData::BinPars> ROOTData::readHistogram(long int pos) {
	auto it = histkeys.find(pos);
	if (it == histkeys.end())
		return {};

	std::string buffer = data(it->second);
	if (!buffer.empty()) {
		char* buf = &buffer[0];
		std::map<std::string, size_t> counts;
		auto& streamerTH1 = streamerInfo.find("TH1")->second;
		auto& streamerTAxis = streamerInfo.find("TAxis")->second;

		size_t count;
		Version(buf); // TH1(D/F/I/S/C)
		Version(buf, count); // TH1
		char* const dbuf = buf + count;
		advanceTo(buf, streamerTH1, std::string(), "fNcells", counts);

		std::vector<BinPars> r(read<int>(buf)); // fNcells
		if (r.size() < 3)
			return {};

		r.front().lowedge = -std::numeric_limits<double>::infinity();

		advanceTo(buf, streamerTH1, "fNcells", "fXaxis", counts);
		// x-Axis
		Version(buf, count); // TAxis
		char* const nbuf = buf + count;
		advanceTo(buf, streamerTAxis, std::string(), "fNbins", counts);
		const int nbins = read<int>(buf);
		advanceTo(buf, streamerTAxis, "fNbins", "fXmin", counts);
		const double xmin = read<double>(buf);
		advanceTo(buf, streamerTAxis, "fXmin", "fXmax", counts);
		const double xmax = read<double>(buf);
		advanceTo(buf, streamerTAxis, "fXmax", "fXbins", counts);
		const size_t nborders = read<int>(buf); // TArrayD
		// root/core/cont/src/TArrayD.cxx -> Streamer
		if (nborders == r.size() - 1) {
			for (size_t i = 0; i < nborders; ++i) {
				r[i + 1].lowedge = read<double>(buf);
			}
		} else {
			//UNUSED: buf += sizeof(double) * nbins;
			const double scale = (xmax - xmin) / static_cast<double>(nbins);
			for (size_t i = 0; i < r.size() - 1; ++i) {
				r[i + 1].lowedge = static_cast<double>(i) * scale + xmin;
			}
		}
		buf = nbuf; // go beyond x-Axis

		advanceTo(buf, streamerTH1, "fXaxis", "fSumw2", counts);
		if (static_cast<size_t>(read<int>(buf)) == r.size()) { // TArrayD
			for (auto& b : r)
				b.sumw2 = read<double>(buf); // always double
		}
		buf = dbuf; // skip to contents of TH1(D/F/I/S/C)

		if (static_cast<size_t>(read<int>(buf)) == r.size()) {
			auto readf = readType<double>(it->second.type);
			for (auto& b : r)
				b.content = readf(buf);
		}

		return r;
	} else
		return {};
}

void ROOTData::readNEntries(ROOTData::KeyBuffer& kbuffer) {
	std::string buffer = data(kbuffer);
	if (!buffer.empty()) {
		char* buf = &buffer[0];
		std::map<std::string, size_t> counts;
		if (kbuffer.type == ContentType::NTuple)
			Version(buf); // TNtuple(D)
		Version(buf); // TTree
		advanceTo(buf, streamerInfo.find("TTree")->second, std::string(), "fEntries", counts);
		kbuffer.nrows = read<long int>(buf); // fEntries
	}
}

std::string ROOTData::treeName(long int pos) {
	auto it = treekeys.find(pos);
	if (it != treekeys.end())
		return it->second.name;
	return {};
}

int ROOTData::treeEntries(long int pos) {
	auto it = treekeys.find(pos);
	if (it != treekeys.end())
		return it->second.nrows;
	else
		return 0;
}

std::vector<ROOTData::LeafInfo> ROOTData::listLeaves(long int pos) const {
	std::vector<LeafInfo> leaves;

	auto it = treekeys.find(pos);
	if (it == treekeys.end())
		return leaves;

	std::ifstream is(filename, std::ifstream::binary);
	std::string datastring = data(it->second, is);
	if (datastring.empty())
		return leaves;

	char* buf = &datastring[0];
	char* const buf0 = buf - it->second.keylength;
	std::map<std::string, size_t> counts;
	auto& streamerTBranch = streamerInfo.find("TBranch")->second;

	if (it->second.type == ContentType::NTuple)
		Version(buf); // TNtuple(D)
	Version(buf); // TTree
	advanceTo(buf, streamerInfo.find("TTree")->second, std::string(), "fBranches", counts);

	// read the list of branches
	Version(buf); // TObjArray
	SkipObject(buf);
	String(buf);
	const size_t nbranches = read<int>(buf);
	const size_t lowb = read<int>(buf);
	std::map<size_t, std::string> tags;
	for (size_t i = 0; i < nbranches; ++i) {
		std::string clname = readObject(buf, buf0, tags);
		size_t count;
		Version(buf, count); // TBranch or TBranchElement
		char* const nbuf = buf + count;
		if (i >= lowb) {
			if (clname == "TBranchElement") {
				Version(buf); // TBranch
			}
			advanceTo(buf, streamerTBranch, std::string(), "TNamed", counts);
			Version(buf); // TNamed
			SkipObject(buf);
			const std::string branch = String(buf);
			String(buf);
			// TODO add reading of nested branches (fBranches)
			advanceTo(buf, streamerTBranch, "TNamed", "fLeaves", counts);

			// fLeaves
			Version(buf); // TObjArray
			SkipObject(buf);
			String(buf);
			const size_t nleaves = read<int>(buf);
			const size_t lowb = read<int>(buf);
			for (size_t i = 0; i < nleaves; ++i) {
				std::string clname = readObject(buf, buf0, tags);
				Version(buf, count); // TLeaf(D/F/B/S/I/L/C/O)
				char* nbuf = buf + count;
				if (i >= lowb && clname.size() == 6 && clname.compare(0, 5, "TLeaf") == 0) {
					Version(buf); // TLeaf
					Version(buf); // TNamed
					SkipObject(buf);
					const std::string leafname = String(buf);
					String(buf); // title
					size_t elements = read<int>(buf);
					int bytes = read<int>(buf);
					if ((static_cast<int>(leafType(clname.back())) & 0xF) != bytes)
						qDebug() << "ROOTData: type " << clname.back() << " does not match its size!";
					buf += 5;
					leaves.emplace_back(LeafInfo{branch, leafname, leafType(clname.back()), !read<char>(buf), elements});
				}

				buf = nbuf;
			}
		}

		buf = nbuf;
	}
	return leaves;
}

template<class T>
std::vector<T> ROOTData::listEntries(long int pos, const std::string& branchname, const std::string& leafname, const size_t element, const size_t nentries) const {
	std::vector<T> entries;

	auto it = treekeys.find(pos);
	if (it == treekeys.end())
		return entries;

	std::ifstream is(filename, std::ifstream::binary);
	std::string datastring = data(it->second, is);
	if (datastring.empty())
		return entries;

	char* buf = &datastring[0];
	char* const buf0 = buf - it->second.keylength;
	std::map<std::string, size_t> counts;
	auto& streamerTTree = streamerInfo.find("TTree")->second;
	auto& streamerTBranch = streamerInfo.find("TBranch")->second;

	if (it->second.type == ContentType::NTuple)
		Version(buf); // TNtuple(D)
	Version(buf); // TTree
	advanceTo(buf, streamerTTree, std::string(), "fEntries", counts);
	entries.reserve(std::min(static_cast<size_t>(read<long int>(buf)), nentries)); // reserve space (maximum for number of entries)
	advanceTo(buf, streamerTTree, "fEntries", "fBranches", counts);

	// read the list of branches
	Version(buf); // TObjArray
	SkipObject(buf);
	String(buf);
	const size_t nbranches = read<int>(buf);
	const size_t lowb = read<int>(buf);
	std::map<size_t, std::string> tags;
	for (size_t i = 0; i < nbranches; ++i) {
		std::string clname = readObject(buf, buf0, tags);
		size_t count;
		Version(buf, count); // TBranch or TBranchElement
		char* const nbuf = buf + count;
		if (i >= lowb) {
			if (clname == "TBranchElement") {
				Version(buf);
			}
			Version(buf); // TNamed
			SkipObject(buf);
			const std::string currentbranch = String(buf);
			String(buf);

			advanceTo(buf, streamerTBranch, "TNamed", "fWriteBasket", counts);
			int fWriteBasket = read<int>(buf);
			// TODO add reading of nested branches (fBranches)
			advanceTo(buf, streamerTBranch, "fWriteBasket", "fLeaves", counts);

			// fLeaves
			Version(buf); // TObjArray
			SkipObject(buf);
			String(buf);
			const size_t nleaves = read<int>(buf);
			const size_t lowb = read<int>(buf);
			int leafoffset = 0, leafcount = 0, leafcontent = 0, leafsize = 0;
			bool leafsign = false;
			ContentType leaftype = ContentType::Invalid;
			for (size_t i = 0; i < nleaves; ++i) {
				std::string clname = readObject(buf, buf0, tags);
				Version(buf, count); // TLeaf(D/F/L/I/S/B/O/C/Element)
				char* nbuf = buf + count;
				if (currentbranch == branchname) {
					if (i >= lowb && clname.size() >= 5 && clname.compare(0, 5, "TLeaf") == 0) {
						Version(buf); // TLeaf
						Version(buf); // TNamed
						SkipObject(buf);
						const bool istheleaf = (clname.size() == 6 && leafname == String(buf));
						String(buf);
						const int len = read<int>(buf);
						const int size = read<int>(buf);
						if (istheleaf) {
							leafoffset = leafcount;
							leafsize = size;
							leaftype = leafType(clname.back());
						}
						leafcount += len * size;
						if (istheleaf) {
							leafcontent = leafcount - leafoffset;
							buf += 1;
							leafsign = !read<bool>(buf);
						}
					}
				}

				buf = nbuf;
			}
			if (leafcontent == 0) {
				buf = nbuf;
				continue;
			}

			if (static_cast<int>(element) * leafsize >= leafcontent) {
				qDebug() << "ROOTData: " << leafname.c_str() << " only contains " << leafcontent / leafsize << " elements.";
				break;
			}

			advanceTo(buf, streamerTBranch, "fLeaves", "fBaskets", counts);
			// fBaskets (probably empty)
			Version(buf, count); // TObjArray
			char* const basketsbuf = buf += count + 1; // TODO there is one byte to be skipped in fBaskets, why is that?

			advanceTo(buf, streamerTBranch, "fBaskets", "fBasketEntry", counts);
			for (int i = 0; i <= fWriteBasket; ++i) {
				if (static_cast<size_t>(read<long int>(buf)) > nentries) {
					fWriteBasket = i;
					break;
				}
			}
			// rewind to the end of fBaskets and look for the fBasketSeek array
			advanceTo(buf = basketsbuf, streamerTBranch, "fBaskets", "fBasketSeek", counts);
			auto readf = readType<T>(leaftype, leafsign);
			for (int i = 0; i < fWriteBasket; ++i) {
				long int pos = read<long int>(buf);
				auto it = basketkeys.find(pos);
				if (it != basketkeys.end()) {
					std::string basketbuffer = data(it->second);
					if (!basketbuffer.empty()) {
						char* bbuf = &basketbuffer[0];
						char* const bufend = bbuf + basketbuffer.size();
						while (bbuf + leafcount <= bufend && entries.size() < nentries) {
							bbuf += leafoffset + leafsize * element;
							entries.emplace_back(readf(bbuf));
							bbuf += leafcount - leafsize * (element + 1) - leafoffset;
						}
					}
				} else {
					qDebug() << "ROOTData: fBasketSeek(" << i << "): " << pos << " (not available)";
				}
			}
		}

		buf = nbuf;
	}

	return entries;
}

ROOTData::ContentType ROOTData::histType(const char type) {
	switch (type) {
	case 'D':
		return ContentType::Double;
	case 'F':
		return ContentType::Float;
	case 'I':
		return ContentType::Int;
	case 'S':
		return ContentType::Short;
	case 'C':
		return ContentType::Byte;
	default:
		return ContentType::Invalid;
	}
}

ROOTData::ContentType ROOTData::leafType(const char type) {
	switch (type) {
	case 'D':
		return ContentType::Double;
	case 'F':
		return ContentType::Float;
	case 'L':
		return ContentType::Long;
	case 'I':
		return ContentType::Int;
	case 'S':
		return ContentType::Short;
	case 'B':
		return ContentType::Byte;
	case 'O':
		return ContentType::Bool;
	case 'C':
		return ContentType::CString;
	default:
		return ContentType::Invalid;
	}
}

template<class T>
T (*ROOTData::readType(ROOTData::ContentType type, bool sign) const)(char*&) {
	switch (type) {
	case ContentType::Double:
		return readcast<double, T>;
	case ContentType::Float:
		return readcast<float, T>;
	case ContentType::Long:
		return sign ? readcast<long, T> : readcast<unsigned long, T>;
	case ContentType::Int:
		return sign ? readcast<int, T> : readcast<unsigned int, T>;
	case ContentType::Short:
		return sign ? readcast<short, T> : readcast<unsigned short, T>;
	case ContentType::Byte:
		return sign ? readcast<char, T> : readcast<unsigned char, T>;
	case ContentType::Bool:
		return readcast<bool, T>;
	case ContentType::CString:
	case ContentType::Tree:
	case ContentType::NTuple:
	case ContentType::Basket:
	case ContentType::Streamer:
	case ContentType::Invalid:
		break;
	}
	return readcast<char, T>;
}

std::string ROOTData::data(const ROOTData::KeyBuffer& buffer) const {
	std::ifstream is(filename, std::ifstream::binary);
	return data(buffer, is);
}

std::string ROOTData::data(const ROOTData::KeyBuffer& buffer, std::ifstream& is) const {
	std::string data(buffer.count, 0);
	is.seekg(buffer.start);
	if (buffer.compression == KeyBuffer::CompressionType::none) {
		is.read(&data[0], buffer.count);
		return data;
#ifdef HAVE_ZIP
	} else if (buffer.compression == KeyBuffer::CompressionType::zlib) {
		std::string cdata(buffer.compressed_count, 0);
		is.read(&cdata[0], buffer.compressed_count);
		uLongf luncomp = (uLongf)buffer.count;
		if (uncompress((Bytef *)data.data(), &luncomp, (Bytef *)cdata.data(), (uLong)cdata.size()) == Z_OK && data.size() == luncomp)
			return data;
	} else {
		std::string cdata(buffer.compressed_count, 0);
		is.read(&cdata[0], buffer.compressed_count);
		if (LZ4_decompress_safe(cdata.data(), const_cast<char*>(data.data()), (int)buffer.compressed_count, (int)buffer.count) == static_cast<int>(buffer.count))
			return data;
#endif
	}

	return {};
}

void ROOTData::readStreamerInfo(const ROOTData::KeyBuffer& buffer) {
	std::ifstream is(filename, std::ifstream::binary);
	std::string datastring = data(buffer, is);
	if (!datastring.empty()) {
		char* buf = &datastring[0];
		char* const buf0 = buf - buffer.keylength;
		Version(buf);
		SkipObject(buf); // TCollection
		String(buf);
		const int nobj = read<int>(buf);
		std::map<size_t, std::string> tags;
		for (int i = 0; i < nobj; ++i) {
			std::string clname = readObject(buf, buf0, tags);
			size_t count;
			Version(buf, count);
			char* const nbuf = buf + count;
			if (clname == "TStreamerInfo") {
				Version(buf);
				SkipObject(buf);
				std::vector<StreamerInfo>& sinfo = streamerInfo[String(buf)];
				String(buf);
				buf += 8; // skip check sum and version

				clname = readObject(buf, buf0, tags);
				Version(buf, count);
				if (clname != "TObjArray") {
					buf += count;
					continue;
				}

				SkipObject(buf); // TObjArray
				String(buf);
				const int nobj = read<int>(buf);
				const int lowb = read<int>(buf);
				for (int i = 0; i < nobj; ++i) {
					std::string clname = readObject(buf, buf0, tags);
					Version(buf, count);
					char* const nbuf = buf + count;

					const bool isbasicpointer = clname == "TStreamerBasicPointer";
					const bool ispointer = isbasicpointer || clname == "TStreamerObjectPointer";
					if (i >= lowb) {
						if (ispointer ||
						    clname == "TStreamerBase" ||
						    clname == "TStreamerBasicType" ||
						    clname == "TStreamerObject" ||
						    clname == "TStreamerObjectAny" ||
						    clname == "TStreamerString" ||
						    clname == "TStreamerSTL")
						{
							Version(buf); // TStreamerXXX
							Version(buf); // TStreamerElement
							SkipObject(buf);
							const std::string name = String(buf);
							const std::string title = String(buf);
							int type = read<int>(buf);
							size_t size = read<int>(buf);

							if (clname.compare(0, 15, "TStreamerObject") == 0)
								size = 0;
							std::string counter;
							bool iscounter = false;
							if (ispointer) {
								if (!title.empty() && title.front() == '[') {
									const size_t endref = title.find(']', 1);
									if (endref != title.npos) {
										counter = title.substr(1, endref - 1);
									}
								}
								if (isbasicpointer) {
									// see root/io/io/inc/TStreamerInfo.h -> TStreamerInfo::EReadWrite
									switch (type - 40) {
										case 1:  // char
										case 11: // unsigned char
											size = 1;
											break;
										case 2:  // short
										case 12: // unsigned short
										case 19: // float16
											size = 2;
											break;
										case 3:  // int
										case 5:  // float
										case 9:  // double32
										case 13: // unsigned int
											size = 4;
											break;
										case 4:  // long
										case 8:  // double
										case 14: // unsigned long
										case 16: // long
										case 17: // unsigned long
											size = 8;
											break;
									}
								}
							} else if (clname == "TStreamerBasicType") {
								iscounter = type == 6; // see root/io/io/inc/TStreamerInfo.h -> TStreamerInfo::EReadWrite
							}
							sinfo.emplace_back(StreamerInfo{name, size, counter, iscounter, ispointer});
						}
					}
					buf = nbuf;
				}
			} else
				buf = nbuf;
			buf += 1; // trailing zero of TObjArray*
		}
	} else
		DEBUG("ROOTData: Inflation failed!")
}

bool ROOTData::advanceTo(char*& buf, const std::vector<ROOTData::StreamerInfo>& objects, const std::string& current, const std::string& target, std::map<std::string, size_t>& counts) {
	// The object structure can be retrieved from TFile::GetStreamerInfoList().
	// Every ROOT object contains a version number which may include the byte count
	// for the object. The latter is currently assumed to be present to skip unused
	// objects. No checks are performed. The corresponding ROOT code is quite nested
	// but the actual readout is straight forward.
	auto it = objects.begin();
	if (!current.empty()) {
		for (; it != objects.end(); ++it) {
			if (it->name == target) {
				return false; // target lies before current buffer position
			} else if (it->name == current) {
				++it;
				break;
			}
		}
	}

	for (; it != objects.end(); ++it) {
		if (it->name == target)
			return true;

		if (it->size == 0)
			Skip(buf, 1);
		else if (it->iscounter)
			counts[it->name] = read<int>(buf);
		else if (it->ispointer) {
			if (it->counter.empty())
				buf += it->size + 1;
			else
				buf += it->size * counts[it->counter] + 1;
		} else
			buf += it->size;
	}

	return false;
}

// needs to be after ROOTDataHelpers namespace declaration

QString ROOTFilter::fileInfoString(const QString& fileName) {
	DEBUG("ROOTFilter::fileInfoString()");
	QString info;

	// The file structure is described in root/io/io/src/TFile.cxx
	std::ifstream is(fileName.toStdString(), std::ifstream::binary);
	std::string root(4, 0);
	is.read(const_cast<char*>(root.data()), 4);
	if (root != "root") {
		DEBUG("	Not a ROOT file. root = " << root);
		return i18n("Not a ROOT file");
	}

	int version = read<int>(is);

	info += i18n("File format version: %1", QString::number(version));
	info += QLatin1String("<br>");

	is.seekg(20);
	int freeBytes = read<int>(is);
	int freeRecords = read<int>(is);
	int namedBytes = read<int>(is);
	char pointerBytes = read<char>(is);
	info += i18n("FREE data record size: %1 bytes", QString::number(freeBytes));
	info += QLatin1String("<br>");
	info += i18n("Number of free data records: %1", QString::number(freeRecords));
	info += QLatin1String("<br>");
	info += i18n("TNamed size: %1 bytes", QString::number(namedBytes));
	info += QLatin1String("<br>");
	info += i18n("Size of file pointers: %1 bytes", QString::number(pointerBytes));
	info += QLatin1String("<br>");

	int compression = read<int>(is);
	compression = compression > 0 ? compression : 0;
	info += i18n("Compression level and algorithm: %1", QString::number(compression));
	info += QLatin1String("<br>");

	is.seekg(41);
	int infoBytes = read<int>(is);
	info += i18n("Size of TStreamerInfo record: %1 bytes", QString::number(infoBytes));
	info += QLatin1String("<br>");

	return info;
}
