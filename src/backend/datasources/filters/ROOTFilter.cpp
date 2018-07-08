/***************************************************************************
File                 : ROOTFilter.cpp
Project              : LabPlot
Description          : ROOT(CERN) I/O-filter
--------------------------------------------------------------------
Copyright            : (C) 2018 by Christoph Roick (chrisito@gmx.de)
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
#include "backend/datasources/AbstractDataSource.h"
#include "backend/core/column/Column.h"

#include <KLocalizedString>

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

ROOTFilter::ROOTFilter():AbstractFileFilter(), d(new ROOTFilterPrivate) {}

ROOTFilter::~ROOTFilter() {}

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

void ROOTFilter::setCurrentHistogram(const QString& histogram) {
	d->currentHistogram = histogram;
}

const QString ROOTFilter::currentHistogram() const {
	return d->currentHistogram;
}

QStringList ROOTFilter::listHistograms(const QString& fileName) {
	return d->listHistograms(fileName);
}

QVector<QStringList> ROOTFilter::previewCurrentHistogram(const QString& fileName,
                                                         int first, int last) {
	return d->previewCurrentHistogram(fileName, first, last);
}

int ROOTFilter::binsInCurrentHistogram(const QString& fileName) {
	return d->binsInCurrentHistogram(fileName);
}

void ROOTFilter::setStartBin(const int s) {
	d->startBin = s;
}

int ROOTFilter::startBin() const {
	return d->startBin;
}

void ROOTFilter::setEndBin(const int e) {
	d->endBin = e;
}

int ROOTFilter::endBin() const {
	return d->endBin;
}

void ROOTFilter::setColumns(const int columns) {
	d->columns = columns;
}

int ROOTFilter::columns() const {
	return d->columns;
}

void ROOTFilter::save(QXmlStreamWriter* writer) const {
	writer->writeStartElement("rootFilter");
	writer->writeAttribute("startBin", QString::number(d->startBin) );
	writer->writeAttribute("endBin", QString::number(d->endBin) );
	writer->writeAttribute("columns", QString::number(d->columns) );
	writer->writeEndElement();
}

bool ROOTFilter::load(XmlStreamReader* reader) {
	QString attributeWarning = i18n("Attribute '%1' missing or empty, default value is used");
	QXmlStreamAttributes attribs = reader->attributes();

	// read attributes
	QString str = attribs.value("startBin").toString();
	if (str.isEmpty())
		reader->raiseWarning(attributeWarning.arg("'startBin'"));
	else
		d->startBin = str.toInt();

	str = attribs.value("endBin").toString();
	if (str.isEmpty())
		reader->raiseWarning(attributeWarning.arg("'endBin'"));
	else
		d->endBin = str.toInt();

	str = attribs.value("columns").toString();
	if (str.isEmpty())
		reader->raiseWarning(attributeWarning.arg("'columns'"));
	else
		d->columns = str.toInt();

	return true;
}

/**************** ROOTFilterPrivate implementation *******************/

ROOTFilterPrivate::ROOTFilterPrivate() {}

void ROOTFilterPrivate::readDataFromFile(const QString& fileName, AbstractDataSource* dataSource,
		AbstractFileFilter::ImportMode importMode) {
	DEBUG("ROOTFilterPrivate::readDataFromFile()");

	setFile(fileName);

	auto bins = readHistogram();
	const int nbins = (int)bins.size();

	// skip underflow and overflow bins by default
	int first = qMax(startBin, 0);
	int last = qMin(endBin, nbins - 1);

	QStringList colNames = createHeaders();

	QVector<void*> dataContainer;
	const int columnOffset = dataSource->prepareImport(dataContainer, importMode, last - first + 1, colNames.size(),
			colNames, QVector<AbstractColumn::ColumnMode>(colNames.size(), AbstractColumn::Numeric));

	// read data
	DEBUG("reading " << first - last + 1 << " lines");

	int c = 0;
	if (columns & ROOTFilter::Center) {
		QVector<double>& container = *static_cast<QVector<double>*>(dataContainer[c++]);
		for (int i = first; i <= last; ++i)
			container[i - first] = (i > 0 && i < nbins - 1) ? 0.5 * (bins[i].lowedge + bins[i + 1].lowedge)
			                                       : i == 0 ? bins.front().lowedge   // -infinity
			                                                : -bins.front().lowedge; // +infinity
	}
	if (columns & ROOTFilter::Low) {
		QVector<double>& container = *static_cast<QVector<double>*>(dataContainer[c++]);
		for (int i = first; i <= last; ++i)
			container[i - first] = bins[i].lowedge;
	}
	if (columns & ROOTFilter::Content) {
		QVector<double>& container = *static_cast<QVector<double>*>(dataContainer[c++]);
		for (int i = first; i <= last; ++i)
			container[i - first] = bins[i].content;
	}
	if (columns & ROOTFilter::Error) {
		QVector<double>& container = *static_cast<QVector<double>*>(dataContainer[c++]);
		for (int i = first; i <= last; ++i)
			container[i - first] = std::sqrt(bins[i].sumw2);
	}

	dataSource->finalizeImport(columnOffset, 0, colNames.size() - 1, QString(), importMode);
}

void ROOTFilterPrivate::write(const QString& fileName, AbstractDataSource* dataSource) {
	Q_UNUSED(fileName);
	Q_UNUSED(dataSource);
	//TODO
}

QStringList ROOTFilterPrivate::listHistograms(const QString& fileName) {
	setFile(fileName);

	QStringList histList;
	for (const auto& hist : currentROOTHist->listHistograms()) {
		histList << QString::fromStdString(hist);
	}

	return histList;
}

QVector<QStringList> ROOTFilterPrivate::previewCurrentHistogram(const QString& fileName, int first, int last) {
	DEBUG("previewCurrentHistogram()");

	setFile(fileName);

	auto bins = readHistogram();
	const int nbins = (int)bins.size();

	last = qMin(nbins - 1, last);

	QVector<QStringList> preview(qMax(last - first + 2, 1));
	preview.last() = createHeaders();

	// read data
	DEBUG("reading " << preview.size() << " lines");

	if (columns & ROOTFilter::Center) {
		for (int i = first; i <= last; ++i)
			preview[i - first] << QString::number(
			    (i > 0 && i < nbins - 1) ? 0.5 * (bins[i].lowedge + bins[i + 1].lowedge)
			                    : i == 0 ? bins.front().lowedge    // -infinity
			                             : -bins.front().lowedge); // +infinity
	}
	if (columns & ROOTFilter::Low) {
		for (int i = first; i <= last; ++i)
			preview[i - first] << QString::number(bins[i].lowedge);
	}
	if (columns & ROOTFilter::Content) {
		for (int i = first; i <= last; ++i)
			preview[i - first] << QString::number(bins[i].content);
	}
	if (columns & ROOTFilter::Error) {
		for (int i = first; i <= last; ++i)
			preview[i - first] << QString::number(std::sqrt(bins[i].sumw2));
	}

	return preview;
}

int ROOTFilterPrivate::binsInCurrentHistogram(const QString& fileName) {
	setFile(fileName);

	QStringList nameindex = currentHistogram.split(';');
	bool ok = nameindex.size() > 1;
	int cycle = ok ? nameindex.last().toInt(&ok) : 1;
	if (ok) {
		nameindex.removeLast();
	} else {
		cycle = 1;
	}

	return currentROOTHist->histogramBins(nameindex.join(';').toStdString(), cycle);
}

QStringList ROOTFilterPrivate::createHeaders() {
	QStringList colNames;
	if (columns & ROOTFilter::Center)
		colNames << i18n("Bin Center");
	if (columns & ROOTFilter::Low)
		colNames << i18n("Low Edge");
	if (columns & ROOTFilter::Content)
		colNames << i18n("Content");
	if (columns & ROOTFilter::Error)
		colNames << i18n("Error");
	return colNames;
}

void ROOTFilterPrivate::setFile(const QString& fileName) {
	if (!currentROOTHist || fileName != currentFile) {
		currentFile = fileName;
		currentROOTHist.reset(new ROOTHist(fileName.toStdString()));
	}
}

std::vector<ROOTHist::BinPars> ROOTFilterPrivate::readHistogram() {
	QStringList nameindex = currentHistogram.split(';');
	bool ok = nameindex.size() > 1;
	int cycle = ok ? nameindex.last().toInt(&ok) : 1;
	if (ok) {
		nameindex.removeLast();
	} else {
		cycle = 1;
	}

	return currentROOTHist->readHistogram(nameindex.join(';').toStdString(), cycle);
}

/******************** ROOTHist implementation ************************/

namespace ROOTHistHelpers {

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

/// Get version of ROOT object, obtain number of bytes in object
short Version(char*& buffer, size_t& count) {
	count = read<unsigned int>(buffer);
	short version = (count & 0x40000000) ? read<short>(buffer) : read<short>(buffer -= 4);
	count &= (count & 0x40000000) ? (~0x40000000) : 0;
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
		char* nbuf = buffer + 4;
		size_t count;
		Version(buffer, count);
		buffer = nbuf + count;
	}
}

/// Skip TObject header
void SkipObject(char*& buffer) {
	Version(buffer);
	buffer += 8;
}

/// Get TString
std::string String(char*& buffer) {
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

}

using namespace ROOTHistHelpers;

ROOTHist::ROOTHist(const std::string& filename) : filename(filename) {
	// The file structure is described in root/io/io/src/TFile.cxx
	std::ifstream is(filename, std::ifstream::binary);
	std::string root(4, 0);
	is.read(const_cast<char*>(root.data()), 4);
	if (root != "root")
		return;

	is.seekg(8); // skip version
	int pos = read<int>(is);
	is.seekg(16);
	int lastpos = read<int>(is);

	is.seekg(33);
	compression = read<int>(is);
	compression = compression > 0 ? compression : 0;

	while (is.good() && pos < lastpos) {
		is.seekg(pos);
		size_t lcdata = read<unsigned int>(is);
		is.seekg(2, is.cur); // short version = read<short>(is);
		size_t ldata = read<unsigned int>(is);
		is.seekg(4, is.cur);
		size_t lkey = read<unsigned short int>(is);
		short cycle = read<short>(is);
		is.seekg(8, is.cur);
		std::string cname(read<unsigned char>(is), 0);
		is.read(&cname[0], cname.size());
		if (cname.size() == 4 && cname.substr(0, 3) == "TH1") {
			KeyBuffer::ContentType type;
			switch (cname[3]) {
				case 'D':
					type = KeyBuffer::ContentType::Double;
					break;
				case 'F':
					type = KeyBuffer::ContentType::Float;
					break;
				case 'I':
					type = KeyBuffer::ContentType::Int;
					break;
				case 'S':
					type = KeyBuffer::ContentType::Short;
					break;
				case 'C':
					type = KeyBuffer::ContentType::Byte;
					break;
				default:
					type = KeyBuffer::ContentType::Invalid;
					break;
			}
			if (type) {
				std::string name(read<unsigned char>(is), 0);
				is.read(&name[0], name.size());
				std::string title(read<unsigned char>(is), 0);
				is.read(&title[0], title.size());

				auto it = histkeys.end();
				// see root/io/io/src/TKey.cxx for reference
				if (compression && ldata > 256) {
#					ifdef HAVE_ZIP
					// Default: compression level
					// ZLIB: 100 + compression level
					// LZ4:  400 + compression level
					if (compression / 100 <= 1 || compression / 100 == 4) {
						// see root/core/zip/src/RZip.cxx -> R__unzip
						std::string lib(2, 0);
						is.read(&lib[0], 2);
						char method = is.get();
						size_t chcdata = is.get();
						chcdata |= (is.get() << 8);
						chcdata |= (is.get() << 16);
						size_t chdata = is.get();
						chdata |= (is.get() << 8);
						chdata |= (is.get() << 16);

						if (chcdata == lcdata - lkey - 9 && chdata == ldata) {
							if (lib == "ZL" && method == Z_DEFLATED) {
								it = histkeys.emplace(name + ";" + std::to_string(cycle),
								                      KeyBuffer{name, title, cycle, type, KeyBuffer::zlib,
								                                pos + lkey + 9, chcdata, chdata, 0}
								                     ).first;
							} else if (lib == "L4" && method == LZ4_versionNumber() / (100 * 100)) {
								it = histkeys.emplace(name + ";" + std::to_string(cycle),
								                      KeyBuffer{name, title, cycle, type, KeyBuffer::lz4,
								                                pos + lkey + 9 + 8, chcdata - 8, chdata, 0}
								                     ).first;
							}
						}
					}
#					endif
				} else {
					it = histkeys.emplace(name + ";" + std::to_string(cycle),
					                      KeyBuffer{name, title, cycle, type, KeyBuffer::none,
					                                pos + lkey, ldata, ldata, 0}
					                     ).first;
				}
				if (it != histkeys.end())
					readNBins(it->second);
			}
		}
		pos += (int)lcdata;
	}
}

std::vector<std::string> ROOTHist::listHistograms() const {
	std::vector<std::string> l;
	for (auto& n : histkeys) {
		l.emplace_back(n.first);
	}
	return l;
}

std::vector<ROOTHist::BinPars> ROOTHist::readHistogram(const std::string& name, int cycle) {
	auto it = histkeys.find(name + ";" + std::to_string(cycle));
	if (it == histkeys.end())
		return std::vector<ROOTHist::BinPars>();

	std::string buffer = data(it->second);
	if (!buffer.empty()) {
		// The object structure can be retrieved from TFile::GetStreamerInfoList().
		// Every ROOT object contains a version number which may include the byte count
		// for the object. The latter is currently assumed to be present to skip unused
		// objects. No checks are performed. The corresponding ROOT code is quite nested
		// but the actual readout is straight forward.
		// Files for reference:
		// root/io/io/src/TBufferFile.cxx -> ReadVersion, ReadTString, ReadObjectAny
		// root/core/cont/src/TArrayD.cxx -> Streamer
		// root/hist/hist/src/TH1.cxx -> Streamer
		char *buf = &buffer[0];
		size_t count;
		Version(buf, count); // TH1(D/F/I/S/C)
		char* dbuf = buf + 4;
		Version(buf, count); // TH1
		dbuf += count;
		Skip(buf, 4); // skip TNamed, TAttLine, TAttFill, TAttMarker

		std::vector<BinPars> r(read<int>(buf)); // fNcells
		if (r.size() < 3)
			return std::vector<BinPars>();

		r.front().lowedge = -std::numeric_limits<double>::infinity();

		// x-Axis
		char* nbuf = buf + 4;
		Version(buf, count); // TAxis
		nbuf += count;
		Skip(buf, 2); // skip TNamed, TAttAxis

		int nbins = read<int>(buf);
		double xmin = read<double>(buf);
		double xmax = read<double>(buf);
		const size_t nborders = read<int>(buf);

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
		buf = nbuf;
		Skip(buf, 2); // skip y-Axis and z-Axis;
		buf += 68; // skip 2 shorts and 8 doubles
		buf += read<int>(buf) * 8; // skip fContour array

		if (static_cast<size_t>(read<int>(buf)) == r.size()) {
			for (auto& b : r)
				b.sumw2 = read<double>(buf); // always double
		}

		buf = dbuf; // skip to contents of TH1(D/F/I/S/C)

		if (static_cast<size_t>(read<int>(buf)) == r.size()) {
			switch (it->second.type) {
				case KeyBuffer::ContentType::Double:
					for (auto& b : r)
						b.content = read<double>(buf);
					break;
				case KeyBuffer::ContentType::Float:
					for (auto& b : r)
						b.content = read<float>(buf);
					break;
				case KeyBuffer::ContentType::Int:
					for (auto& b : r)
						b.content = read<int>(buf);
					break;
				case KeyBuffer::ContentType::Short:
					for (auto& b : r)
						b.content = read<short>(buf);
					break;
				case KeyBuffer::ContentType::Byte:
					for (auto& b : r)
						b.content = read<char>(buf);
					break;
				case KeyBuffer::ContentType::Invalid: // never reached
				default:
					break;
			}
		}

		return r;
	} else
		return std::vector<BinPars>();
}

void ROOTHist::readNBins(ROOTHist::KeyBuffer& kbuffer) {
	std::string buffer = data(kbuffer);
	if (!buffer.empty()) {
		// The object structure can be retrieved from TFile::GetStreamerInfoList().
		// Every ROOT object contains a version number which may include the byte count
		// for the object. The latter is currently assumed to be present to skip unused
		// objects. No checks are performed. The corresponding ROOT code is quite nested
		// but the actual readout is straight forward.
		// Files for reference:
		// root/io/io/src/TBufferFile.cxx -> ReadVersion, ReadTString, ReadObjectAny
		// root/core/cont/src/TArrayD.cxx -> Streamer
		// root/hist/hist/src/TH1.cxx -> Streamer
		char *buf = &buffer[0];
		size_t count;
		Version(buf, count); // TH1(D/F/I/S/C)
		char* dbuf = buf + 4;
		Version(buf, count); // TH1
		dbuf += count;
		Skip(buf, 4); // skip TNamed, TAttLine, TAttFill, TAttMarker

		kbuffer.nbins = read<int>(buf); // fNcells
	}
}

std::string ROOTHist::data(const ROOTHist::KeyBuffer& buffer) const {
	std::ifstream is(filename, std::ifstream::binary);
	return data(buffer, is);
}

std::string ROOTHist::data(const ROOTHist::KeyBuffer& buffer, std::ifstream& is) const {
	std::string data(buffer.count, 0);
	is.seekg(buffer.start);
	if (buffer.compression == KeyBuffer::none) {
		is.read(&data[0], buffer.count);
		return data;
#ifdef HAVE_ZIP
	} else if (buffer.compression == KeyBuffer::zlib) {
		std::string cdata(buffer.compressed_count, 0);
		is.read(&cdata[0], buffer.compressed_count);
		size_t luncomp = buffer.count;
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
