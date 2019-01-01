/***************************************************************************
File                 : ROOTFilter.cpp
Project              : LabPlot
Description          : ROOT(CERN) I/O-filter
--------------------------------------------------------------------
Copyright            : (C) 2018 by Christoph Roick (chrisito@gmx.de)
Copyright            : (C) 2018 by Stefan Gerlach (stefan.gerlach@uni.kn)
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

#include "backend/datasources/filters/ROOTFilter.h"
#include "backend/datasources/filters/ROOTFilterPrivate.h"
#include "backend/spreadsheet/Spreadsheet.h"
#include "backend/core/column/Column.h"

#include <KLocalizedString>

#include <QDebug>

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

ROOTFilter::ROOTFilter():AbstractFileFilter(ROOT), d(new ROOTFilterPrivate) {}

ROOTFilter::~ROOTFilter() = default;

void ROOTFilter::readDataFromFile(const QString& fileName, AbstractDataSource* dataSource,
			AbstractFileFilter::ImportMode importMode) {
	d->readDataFromFile(fileName, dataSource, importMode);
}

void ROOTFilter::write(const QString& fileName, AbstractDataSource* dataSource) {
	d->write(fileName, dataSource);
}

void ROOTFilter::loadFilterSettings(const QString& filterName) {
	Q_UNUSED(filterName);
}

void ROOTFilter::saveFilterSettings(const QString& filterName) const {
	Q_UNUSED(filterName);
}

void ROOTFilter::setCurrentObject(const QString& object) {
	d->currentObject = object;
}

const QString ROOTFilter::currentObject() const {
	return d->currentObject;
}

QStringList ROOTFilter::listHistograms(const QString& fileName) const {
	return d->listHistograms(fileName);
}

QStringList ROOTFilter::listTrees(const QString& fileName) const {
	return d->listTrees(fileName);
}

QVector<QStringList> ROOTFilter::listLeaves(const QString& fileName, const QString& treeName) const {
	return d->listLeaves(fileName, treeName);
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
	writer->writeAttribute("startRow", QString::number(d->startRow) );
	writer->writeAttribute("endRow", QString::number(d->endRow) );
	writer->writeEndElement();
}

bool ROOTFilter::load(XmlStreamReader* reader) {
	QString attributeWarning = i18n("Attribute '%1' missing or empty, default value is used");
	QXmlStreamAttributes attribs = reader->attributes();

	// read attributes
	QString str = attribs.value("startRow").toString();
	if (str.isEmpty())
		reader->raiseWarning(attributeWarning.arg("'startRow'"));
	else
		d->startRow = str.toInt();

	str = attribs.value("endRow").toString();
	if (str.isEmpty())
		reader->raiseWarning(attributeWarning.arg("'endRow'"));
	else
		d->endRow = str.toInt();

	return true;
}

/**************** ROOTFilterPrivate implementation *******************/

ROOTFilterPrivate::ROOTFilterPrivate() = default;

void ROOTFilterPrivate::readDataFromFile(const QString& fileName, AbstractDataSource* dataSource,
                                         AbstractFileFilter::ImportMode importMode) {
	DEBUG("ROOTFilterPrivate::readDataFromFile()");

	setFile(fileName);

	QStringList typeobject = currentObject.split(':');
	if (typeobject.size() < 2)
		return;
	if (typeobject.first() == QStringLiteral("Hist")) {
		typeobject.removeFirst();
		auto bins = readHistogram(typeobject.join(':'));
		const int nbins = static_cast<int>(bins.size());

		// skip underflow and overflow bins by default
		int first = qMax(qAbs(startRow), 0);
		int last = endRow < 0 ? nbins - 1 : qMax(first - 1, qMin(endRow, nbins - 1));

		QStringList headers;
		for (const auto& l : columns) {
			headers << l.last();
		}

		QVector<void*> dataContainer;
		const int columnOffset = dataSource->prepareImport(dataContainer, importMode, last - first + 1, columns.size(),
		                                                   headers, QVector<AbstractColumn::ColumnMode>(columns.size(),
		                                                   AbstractColumn::Numeric));

		// read data
		DEBUG("	reading " << first - last + 1 << " lines");

		int c = 0;
		Spreadsheet* spreadsheet = dynamic_cast<Spreadsheet*>(dataSource);

		for (const auto& l : columns) {
			QVector<double>& container = *static_cast<QVector<double>*>(dataContainer[c]);
			if (l.first() == QStringLiteral("center")) {
				if (spreadsheet)
					spreadsheet->column(columnOffset + c)->setPlotDesignation(Column::X);
				for (int i = first; i <= last; ++i)
					container[i - first] = (i > 0 && i < nbins - 1) ? 0.5 * (bins[i].lowedge + bins[i + 1].lowedge)
					                                                : i == 0 ? bins.front().lowedge   // -infinity
					                                                         : -bins.front().lowedge; // +infinity
			} else if (l.first() == QStringLiteral("low")) {
				if (spreadsheet)
					spreadsheet->column(columnOffset + c)->setPlotDesignation(Column::X);
				for (int i = first; i <= last; ++i)
					container[i - first] = bins[i].lowedge;
			} else if (l.first() == QStringLiteral("content")) {
				if (spreadsheet)
					spreadsheet->column(columnOffset + c)->setPlotDesignation(Column::Y);
				for (int i = first; i <= last; ++i)
					container[i - first] = bins[i].content;
			} else if (l.first() == QStringLiteral("error")) {
				if (spreadsheet)
					spreadsheet->column(columnOffset + c)->setPlotDesignation(Column::YError);
				for (int i = first; i <= last; ++i)
					container[i - first] = std::sqrt(bins[i].sumw2);
			}
			++c;
		}

		dataSource->finalizeImport(columnOffset, 0, columns.size() - 1, -1, QString(), importMode);
	} else if (typeobject.first() == QStringLiteral("Tree")) {
		typeobject.removeFirst();
		const QString treeName = typeobject.join(':');
		const int nentries = static_cast<int>(currentROOTData->treeEntries(treeName.toStdString()));

		int first = qMax(qAbs(startRow), 0);
		int last = qMax(first - 1, qMin(endRow, nentries - 1));

		QStringList headers;
		for (const auto& l : columns) {
			QString lastelement = l.back(), leaf = l.front();
			bool isArray = false;
			if (lastelement.at(0) == '[' && lastelement.at(lastelement.size() - 1) == ']') {
				lastelement.mid(1, lastelement.length() - 2).toUInt(&isArray);
			}
			if (!isArray || l.count() == 2)
				headers << l.join(isArray ? QString() : QString(':'));
			else
				headers << l.first() + QChar(':') + l.at(1) + l.back();
		}

		QVector<void*> dataContainer;
		const int columnOffset = dataSource->prepareImport(dataContainer, importMode, last - first + 1, columns.size(),
		                                                   headers, QVector<AbstractColumn::ColumnMode>(columns.size(),
		                                                   AbstractColumn::Numeric));

		int c = 0;
		for (const auto& l : columns) {
			unsigned int element = 0;
			QString lastelement = l.back(), leaf = l.front();
			bool isArray = false;
			if (lastelement.at(0) == '[' && lastelement.at(lastelement.size() - 1) == ']') {
				element = lastelement.mid(1, lastelement.length() - 2).toUInt(&isArray);
				if (!isArray)
					element = 0;
				if (l.count() > 2)
					leaf = l.at(1);
			} else if (l.count() > 1)
				leaf = l.at(1);

			QVector<double>& container = *static_cast<QVector<double>*>(dataContainer[c++]);
			auto data = readTree(treeName, l.first(), leaf, (int)element, last);
			for (int i = first; i <= last; ++i)
				container[i - first] = data[i];
		}

		dataSource->finalizeImport(columnOffset, 0, columns.size() - 1, -1, QString(), importMode);
    }
}

void ROOTFilterPrivate::write(const QString& fileName, AbstractDataSource* dataSource) {
	Q_UNUSED(fileName);
	Q_UNUSED(dataSource);
}

QStringList ROOTFilterPrivate::listHistograms(const QString& fileName) {
	setFile(fileName);

	QStringList histList;
	for (const auto& hist : currentROOTData->listHistograms()) {
		histList << QString::fromStdString(hist);
	}

	return histList;
}

QStringList ROOTFilterPrivate::listTrees(const QString& fileName) {
	setFile(fileName);

	QStringList treeList;
	for (const auto& tree : currentROOTData->listTrees()) {
		treeList << QString::fromStdString(tree);
	}

	return treeList;
}

QVector<QStringList> ROOTFilterPrivate::listLeaves(const QString& fileName, const QString& treeName) {
	setFile(fileName);

	QVector<QStringList> leafList;
	for (const auto& leaf : currentROOTData->listLeaves(treeName.toStdString())) {
		leafList << QStringList(QString::fromStdString(leaf.branch));
		if (leaf.branch != leaf.leaf)
			leafList.last() << QString::fromStdString(leaf.leaf);
		if (leaf.elements > 1)
			leafList.last() << QString("[%1]").arg(leaf.elements);
	}

	return leafList;
}

QVector<QStringList> ROOTFilterPrivate::previewCurrentObject(const QString& fileName, int first, int last) {
	DEBUG("ROOTFilterPrivate::previewCurrentObject()");

	setFile(fileName);

	QStringList typeobject = currentObject.split(':');
	if (typeobject.size() < 2)
		return QVector<QStringList>(1, QStringList());

	if (typeobject.first() == QStringLiteral("Hist")) {
		typeobject.removeFirst();
		auto bins = readHistogram(typeobject.join(':'));
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
	} else if (typeobject.first() == QStringLiteral("Tree")) {
		typeobject.removeFirst();
		const QString treeName = typeobject.join(':');
		last = qMin(last, currentROOTData->treeEntries(treeName.toStdString()) - 1);

		QVector<QStringList> preview(qMax(last - first + 2, 1));
		DEBUG("	reading " << preview.size() - 1 << " lines");

		// read data leaf by leaf and set headers
		for (const auto& l : columns) {
			unsigned int element = 0;
			QString lastelement = l.back(), leaf = l.front();
			bool isArray = false;
			if (lastelement.at(0) == '[' && lastelement.at(lastelement.size() - 1) == ']') {
				element = lastelement.mid(1, lastelement.length() - 2).toUInt(&isArray);
				if (!isArray)
					element = 0;
				if (l.count() > 2)
					leaf = l.at(1);
			} else if (l.count() > 1)
				leaf = l.at(1);

			auto data = readTree(treeName, l.first(), leaf, (int)element, last);
			for (int i = first; i <= last; ++i)
				preview[i - first] << QString::number(data[i]);
			if (!isArray || l.count() == 2)
				preview.last() << l.join(isArray ? QString() : QString(':'));
			else
				preview.last() << l.first() + QChar(':') + l.at(1) + l.back();
		}

		return preview;
	} else
		return QVector<QStringList>(1, QStringList());
}

int ROOTFilterPrivate::rowsInCurrentObject(const QString& fileName) {
	setFile(fileName);

	QStringList typeobject = currentObject.split(':');
	if (typeobject.size() < 2)
		return 0;
	if (typeobject.first() == QStringLiteral("Hist")) {
		typeobject.removeFirst();
		QStringList nameindex = typeobject.join(':').split(';');;
		bool ok = nameindex.size() > 1;
		int cycle = ok ? nameindex.last().toInt(&ok) : 1;
		if (ok) {
			nameindex.removeLast();
		} else {
			cycle = 1;
		}

		return currentROOTData->histogramBins(nameindex.join(';').toStdString(), cycle);
	} else if (typeobject.first() == QStringLiteral("Tree")) {
		typeobject.removeFirst();
		return currentROOTData->treeEntries(typeobject.join(':').toStdString());
	} else
		return 0;
}

void ROOTFilterPrivate::setFile(const QString& fileName) {
	if (!currentROOTData || fileName != currentFile) {
		currentFile = fileName;
		currentROOTData.reset(new ROOTData(fileName.toStdString()));
	}
}

std::vector<ROOTData::BinPars> ROOTFilterPrivate::readHistogram(const QString& histName) {
	QStringList nameindex = histName.split(';');
	bool ok = nameindex.size() > 1;
	int cycle = ok ? nameindex.last().toInt(&ok) : 1;
	if (ok) {
		nameindex.removeLast();
	} else {
		cycle = 1;
	}

	return currentROOTData->readHistogram(nameindex.join(';').toStdString(), cycle);
}

std::vector<double> ROOTFilterPrivate::readTree(const QString& treeName, const QString& branchName, const QString& leafName, int element, int last)
{
	return currentROOTData->listEntries<double>(treeName.toStdString(), branchName.toStdString(), leafName.toStdString(), element, last + 1);
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
	for (size_t i = 0; i < sizeof(T); ++i) {
		r.buf[sizeof(T) - i - 1] = is.get();
	}

	return r.val;
}

/// Read value from buffer
template<class T>
T read(char*& s) {
	union {
		T val;
		char buf[sizeof(T)];
	} r;
	for (size_t i = 0; i < sizeof(T); ++i) {
		r.buf[sizeof(T) - i - 1] = *(s++);
	}

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
void Skip(char*& buffer, const size_t& n) {
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
		return std::string();
	else {
		if (s == 0xFF)
			s = read<int>(buffer);
		buffer += s;
		return std::string(buffer - s, buffer);
	}
}

/// Get the header of an object in TObjArray
std::string readObject(char*& buf, char* const buf0, std::map<size_t, std::string>& tags)
{
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
		is.seekg(version > 1000 ? 16 : 8, is.cur); // skip seek positions
		std::string cname(read<unsigned char>(is), 0);
		is.read(&cname[0], cname.size());
		std::string name(read<unsigned char>(is), 0);
		is.read(&name[0], name.size());
		std::string title(read<unsigned char>(is), 0);
		is.read(&title[0], title.size());

		ContentType type = Invalid;
		if (cname.size() == 4 && cname.substr(0, 3) == "TH1") {
			type = histType(cname[3]);
		} else if (cname == "TTree")
			type = Tree;
		else if (cname.substr(0, 7) == "TNtuple")
			type = NTuple;
		else if (cname == "TBasket")
			type = Basket;
		else if (cname == "TList" && name == "StreamerInfo")
			type = Streamer;

		if (type) {
			if (type == Basket)
				is.seekg(19, std::ifstream::cur); // TODO read info instead?
			KeyBuffer buffer;
			buffer.type = Invalid;
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
				char method = is.get();
				size_t chcdata = is.get();
				chcdata |= (is.get() << 8);
				chcdata |= (is.get() << 16);
				size_t chdata = is.get();
				chdata |= (is.get() << 8);
				chdata |= (is.get() << 16);

				if (chcdata == lcdata - lkey - 9 && chdata == ldata) {
					if (complib == 1 && method == Z_DEFLATED) {
						buffer = KeyBuffer{type, name, title, cycle, lkey, KeyBuffer::zlib,
						                   pos + lkey + 9, chcdata, chdata, 0};
					} else if (complib == 4 && method == LZ4_versionNumber() / 10000) {
						buffer = KeyBuffer{type, name, title, cycle, lkey, KeyBuffer::lz4,
						                   pos + lkey + 9 + 8, chcdata - 8, chdata, 0};
					}
				}
#			endif
			} else {
				buffer = KeyBuffer{type, name, title, cycle, lkey, KeyBuffer::none,
				                   pos + lkey, ldata, ldata, 0};
			}
			switch (buffer.type) {
				case Basket:
					basketkeys.emplace(pos, buffer);
					break;
				case Tree:
				case NTuple: {
					auto it = treekeys.find(name);
					if (it != treekeys.end()) {
						// TTrees may be written several times, only consider last cycle
						if (buffer.cycle > it->second.cycle) {
							it->second = buffer;
						}
					} else
						treekeys.emplace(name, buffer);
					break;
				} case Streamer:
					readStreamerInfo(buffer);
					break;
				case Double: case Float: case Int: case Short: case Byte:
					histkeys.emplace(name + ';' + std::to_string(cycle), buffer);
					break;
				case Invalid: case Long: case Bool: case CString:
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

std::vector<std::string> ROOTData::listHistograms() const {
	std::vector<std::string> l;
	for (auto& n : histkeys) {
		l.emplace_back(n.first);
	}
	return l;
}

std::vector<ROOTData::BinPars> ROOTData::readHistogram(const std::string& name, int cycle) {
	auto it = histkeys.find(name + ';' + std::to_string(cycle));
	if (it == histkeys.end())
		return std::vector<ROOTData::BinPars>();

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
			return std::vector<BinPars>();

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
			buf += sizeof(double) * nbins;
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
		return std::vector<BinPars>();
}

void ROOTData::readNEntries(ROOTData::KeyBuffer& kbuffer) {
	std::string buffer = data(kbuffer);
	if (!buffer.empty()) {
		char* buf = &buffer[0];
		std::map<std::string, size_t> counts;
		if (kbuffer.type == NTuple)
			Version(buf); // TNtuple(D)
		Version(buf); // TTree
		advanceTo(buf, streamerInfo.find("TTree")->second, std::string(), "fEntries", counts);
		kbuffer.nrows = read<long int>(buf); // fEntries
	}
}

std::vector<std::string> ROOTData::listTrees() const {
	std::vector<std::string> l;
	for (auto& n : treekeys) {
		l.emplace_back(n.first);
	}
	return l;
}

std::vector<ROOTData::LeafInfo> ROOTData::listLeaves(const std::string& treename) const {
	std::vector<LeafInfo> leaves;

	auto it = treekeys.find(treename);
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

	if (it->second.type == NTuple)
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
					const std::string leaftitle = String(buf);
					size_t elements = read<int>(buf);
					int bytes = read<int>(buf);
					if ((leafType(clname.back()) & 0xF) != bytes)
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
std::vector<T> ROOTData::listEntries(const std::string& treename, const std::string& branchname, const std::string& leafname, const size_t element, const size_t nentries) const {
	std::vector<T> entries;

	auto it = treekeys.find(treename);
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

	if (it->second.type == NTuple)
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
            ContentType leaftype = Invalid;
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
				size_t pos = read<long int>(buf);
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

ROOTData::ContentType ROOTData::histType(const char type)
{
	switch (type) {
		case 'D':
			return Double;
		case 'F':
			return Float;
		case 'I':
			return Int;
		case 'S':
			return Short;
		case 'C':
			return Byte;
		default:
			return Invalid;
	}
}

ROOTData::ContentType ROOTData::leafType(const char type)
{
	switch (type) {
		case 'D':
			return Double;
		case 'F':
			return Float;
		case 'L':
			return Long;
		case 'I':
			return Int;
		case 'S':
			return Short;
		case 'B':
			return Byte;
		case 'O':
			return Bool;
		case 'C':
			return CString;
		default:
			return Invalid;
	}
}

template<class T>
T (*ROOTData::readType(ROOTData::ContentType type, bool sign) const)(char*&)
{
	switch (type) {
		case Double:
			return readcast<double, T>;
		case Float:
			return readcast<float, T>;
		case Long:
			return sign ? readcast<long, T> : readcast<unsigned long, T>;
		case Int:
			return sign ? readcast<int, T> : readcast<unsigned int, T>;
		case Short:
			return sign ? readcast<short, T> : readcast<unsigned short, T>;
		case Byte:
			return sign ? readcast<char, T> : readcast<unsigned char, T>;
		case Bool:
			return readcast<bool, T>;
		case CString:
		case Tree:
		case NTuple:
		case Basket:
		case Streamer:
		case Invalid:
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
	if (buffer.compression == KeyBuffer::none) {
		is.read(&data[0], buffer.count);
		return data;
#ifdef HAVE_ZIP
	} else if (buffer.compression == KeyBuffer::zlib) {
		std::string cdata(buffer.compressed_count, 0);
		is.read(&cdata[0], buffer.compressed_count);
		uLongf luncomp = buffer.count;
		if (uncompress((Bytef *)data.data(), &luncomp, (Bytef *)cdata.data(), cdata.size()) == Z_OK && data.size() == luncomp)
			return data;
	} else {
		std::string cdata(buffer.compressed_count, 0);
		is.read(&cdata[0], buffer.compressed_count);
		if (LZ4_decompress_safe(cdata.data(), const_cast<char*>(data.data()), buffer.compressed_count, buffer.count) == static_cast<int>(buffer.count))
			return data;
#endif
	}

	return std::string();
}

void ROOTData::readStreamerInfo(const ROOTData::KeyBuffer& buffer)
{
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
		qDebug() << "ROOTData: Inflation failed!";
}

bool ROOTData::advanceTo(char*& buf, const std::vector<ROOTData::StreamerInfo>& objects, const std::string& current, const std::string& target, std::map<std::string, size_t>& counts)
{
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

	Q_UNUSED(fileName);

	return info;
}
