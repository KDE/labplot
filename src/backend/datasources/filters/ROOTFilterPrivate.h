/***************************************************************************
File                 : ROOTFilterPrivate.h
Project              : LabPlot
Description          : Private implementation class for ROOTFilter.
--------------------------------------------------------------------
Copyright            : (C) 2018 Christoph Roick (chrisito@gmx.de)
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

#ifndef ROOTFILTERPRIVATE_H
#define ROOTFILTERPRIVATE_H

#include <QMap>
#include <QString>
#include <QStringList>
#include <QVector>

#include <map>
#include <memory>
#include <string>
#include <vector>

class ROOTFilter;

class AbstractDataSource;
class AbstractColumn;

/**
 * @brief Read TH1 histograms from ROOT files without depending on ROOT libraries
 */
class ROOTHist {
public:
	/**
	 * @brief Open ROOT file and save file positions of histograms
	 *
	 * Also checks for the compression level. Currently the default ZLIB and LZ4 compression
	 * types are supported. The TH1 object structure is hard coded, TStreamerInfo is not read.
	 *
	 * @param[in] filename ROOT file to be read
	 */
	explicit ROOTHist(const std::string& filename);

	struct BinPars {
		double content;
		double sumw2;
		double lowedge;
	};

	/**
	 * @brief List available histograms in the ROOT file
	 */
	std::vector<std::string> listHistograms() const;

	/**
	 * @brief Read histogram from file
	 *
	 * Jumps to memoized file position, decompresses the object if required and analyzes
	 * the buffer. Overflow and underflow bins are included.
	 *
	 * @param[in] name Histogram name without cycle indicator
	 * @param[in] cycle Indicator for object cycle
	 */
	std::vector<BinPars> readHistogram(const std::string& name, int cycle = 1);

	/**
	 * @brief Get histogram title
	 *
	 * The title is stored in the buffer. No file access required.
	 *
	 * @param[in] name Histogram name without cycle indicator
	 * @param[in] cycle Indicator for object cycle
	 */
	std::string histogramTitle(const std::string& name, int cycle = 1)
	{
		auto it = histkeys.find(name + ";" + std::to_string(cycle));
		if (it != histkeys.end())
			return it->second.title;
		else
			return std::string();
	}

	/**
	 * @brief Get number of bins in histogram
	 *
	 * The number of bins is stored in the buffer. No file access required.
	 *
	 * @param[in] name Histogram name without cycle indicator
	 * @param[in] cycle Indicator for object cycle
	 */
	int histogramBins(const std::string& name, int cycle = 1)
	{
		auto it = histkeys.find(name + ";" + std::to_string(cycle));
		if (it != histkeys.end())
			return it->second.nbins;
		else
			return 0;
	}
private:
	struct KeyBuffer {
		std::string name;
		std::string title;
		int cycle;
		enum ContentType { Invalid = 0, Double, Float, Int, Short, Byte } type;
		enum CompressionType { none, zlib, lz4 } compression;
		size_t start;
		size_t compressed_count;
		size_t count;
		int nbins;
	};

	/// Get the number of bins contained in the histogram
	void readNBins(ROOTHist::KeyBuffer& buffer);
	/// Get buffer from file content at histogram position
	std::string data(const ROOTHist::KeyBuffer& buffer) const;
	/// Get buffer from file content at histogram position, uses already opened stream
	std::string data(const ROOTHist::KeyBuffer& buffer, std::ifstream& is) const;

	std::string filename;
	std::map<std::string, KeyBuffer> histkeys;
	int compression;
};

class ROOTFilterPrivate {

public:
	ROOTFilterPrivate();

	/**
	 * @brief Read data from the currently selected histogram
	 *
	 * The ROOT file is kept open until the file name is changed
	 */
	void readDataFromFile(const QString& fileName, AbstractDataSource* dataSource,
	                      AbstractFileFilter::ImportMode importMode);
	/// Currently writing to ROOT files is not supported
	void write(const QString& fileName, AbstractDataSource*);

	/// List names of histograms contained in ROOT file
	QStringList listHistograms(const QString& fileName);

	/// Get preview data of the currently set histogram
	QVector<QStringList> previewCurrentHistogram(const QString& fileName,
	                                             int first, int last);

	/// Get the number of bins in the current histogram
	int binsInCurrentHistogram(const QString& fileName);

	/// Identifier of the current histogram
	QString currentHistogram;
	/// Start bin to read (can be -1, skips the underflow bin 0)
	int startBin = -1;
	/// End bin to read (can be -1, skips the overflow bin)
	int endBin = -1;
	/// Start column to read
	int columns = 0;
private:
	/// Create headers from set columns
	QStringList createHeaders();
	/// Checks and updates the current ROOT file path
	void setFile(const QString& fileName);
	/// Calls ReadHistogram from ROOTHist
	std::vector<ROOTHist::BinPars> readHistogram();

	/// Currently set ROOT file path
	QString currentFile;
	/// ROOTHist instance kept alive while currentFile does not change
	std::unique_ptr<ROOTHist> currentROOTHist;
};

#endif
