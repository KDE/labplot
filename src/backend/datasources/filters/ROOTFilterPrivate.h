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

#include <QDateTime>
#include <QVector>

#include <map>
#include <memory>
#include <string>
#include <vector>

class ROOTFilter;

class QString;
class QStringList;
class AbstractDataSource;
class AbstractColumn;

/**
 * @brief Read TH1 histograms and TTrees from ROOT files without depending on ROOT libraries
 */
class ROOTData
 {
public:
	/**
	 * @brief Open ROOT file and save file positions of histograms and trees
	 *
	 * Also checks for the compression level. Currently the default ZLIB and LZ4 compression
	 * types are supported. The TStreamerInfo is read if it is available, otherwise the
	 * data structure as of ROOT v6.15 is used. No tests were performed with data written
	 * prior to ROOT v5.34.
	 *
	 * @param[in] filename ROOT file to be read
	 */
	explicit ROOTData (const std::string& filename);

	/// Parameters to describe a bin
	struct BinPars {
		double content;
		double sumw2;
		double lowedge;
	};

	/**
	 * @brief Identifiers for different data types
	 *
	 * Histograms are identified by their bin type. The lowest byte indicates the size
	 * of the numeric types for cross checks during the import.
	 */
	enum ContentType {Invalid = 0, Tree = 0x10, NTuple = 0x11, Basket = 0x20,
	                  Streamer = 0x30,
	                  Double = 0x48, Float = 0x54,
	                  Long = 0x68, Int = 0x74, Short = 0x82, Byte = 0x91,
	                  Bool = 0xA1, CString = 0xB0};

	/// Information about leaf contents
	struct LeafInfo {
		std::string branch;
		std::string leaf;
		ContentType type;
		bool issigned;
		size_t elements;
	};

	/**
	 * @brief List available histograms in the ROOT file
	 */
	std::vector<std::string> listHistograms() const;

	/**
	 * @brief List available trees in the ROOT file
	 */
	std::vector<std::string> listTrees() const;

	/**
	 * @brief List information about data contained in leaves
	 *
	 * @param[in] treename Name of the tree
	 */
	std::vector<LeafInfo> listLeaves(const std::string& treename) const;

	/**
	 * @brief Get entries of a leaf
	 *
	 * @param[in] treename Name of the tree
	 * @param[in] branchname Name of the branch
	 * @param[in] leafname Name of the leaf
	 * @param[in] element Index, if leaf is an array
	 * @param[in] nentries Maximum number of entries to be read
	 */
	template<typename T>
	std::vector<T> listEntries(const std::string& treename, const std::string& branchname, const std::string& leafname,
	                           const size_t element = 0, const size_t nentries = std::numeric_limits<size_t>::max()) const;
	/**
	 * @brief Get entries of a leaf with the same name as its branch
	 *
	 * @param[in] treename Name of the tree
	 * @param[in] branchname Name of the branch
	 * @param[in] nentries Maximum number of entries to be read
	 */
	template<typename T>
	std::vector<T> listEntries(const std::string& treename, const std::string& branchname,
	                           const size_t element = 0, const size_t nentries = std::numeric_limits<size_t>::max()) const
	{
		return listEntries<T>(treename, branchname, branchname, element, nentries);
	}

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
		auto it = histkeys.find(name + ';' + std::to_string(cycle));
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
		auto it = histkeys.find(name + ';' + std::to_string(cycle));
		if (it != histkeys.end())
			return it->second.nrows;
		else
			return 0;
	}

	/**
	 * @brief Get number of entries in tree
	 *
	 * The number of entries is stored in the buffer. No file access required.
	 *
	 * @param[in] name Tree name
	 */
	int treeEntries(const std::string& name)
	{
		auto it = treekeys.find(name);
		if (it != treekeys.end())
			return it->second.nrows;
		else
			return 0;
	}
private:
	struct KeyBuffer {
		ContentType type;
		std::string name;
		std::string title;
		int cycle;
		size_t keylength;
		enum CompressionType { none, zlib, lz4 } compression;
		size_t start;
		size_t compressed_count;
		size_t count;
		int nrows;
	};

	struct StreamerInfo
	{
		std::string name;
		size_t size;
		std::string counter;
		bool iscounter;
		bool ispointer;
	};

	/// Get data type from histogram identifier
	static ContentType histType(const char type);
	/// Get data type from leaf identifier
	static ContentType leafType(const char type);
	/// Get function to read a buffer of the specified type
	template<class T>
	T (*readType(ContentType type, bool sign = true) const)(char*&);

	/// Get the number of bins contained in a histogram
	void readNBins(KeyBuffer& buffer);
	/// Get the number of entries contained in a tree
	void readNEntries(KeyBuffer& buffer);
	/// Get buffer from file content at histogram position
	std::string data(const KeyBuffer& buffer) const;
	/// Get buffer from file content at histogram position, uses already opened stream
	std::string data(const KeyBuffer& buffer, std::ifstream& is) const;
	/// Load streamer information
	void readStreamerInfo(const KeyBuffer& buffer);
	/**
	 * @brief Advance to an object inside a class according to streamer information
	 *
	 * The number of entries is stored in the buffer. No file access required.
	 *
	 * @param[in] buf Pointer to the current position in the class object
	 * @param[in] objects A list of objects in the class defined by the streamer information
	 * @param[in] current The name of the current object
	 * @param[in] target The name of the object to be advanced to
	 * @param[in] counts A list of the number of entries in objects of dynamic length; updated while reading
	 */
	static bool advanceTo(char*& buf, const std::vector<StreamerInfo>& objects, const std::string& current, const std::string& target, std::map<std::string, size_t>& counts);

	std::string filename;
	std::map<std::string, KeyBuffer> histkeys, treekeys;
	std::map<size_t, KeyBuffer> basketkeys;

	std::map<std::string, std::vector<StreamerInfo> > streamerInfo;
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
	/// List names of trees contained in ROOT file
	QStringList listTrees(const QString& fileName);
	/// List names of leaves contained in ROOT tree
	QVector<QStringList> listLeaves(const QString& fileName, const QString& treeName);

	/// Get preview data of the currently set histogram
	QVector<QStringList> previewCurrentObject(const QString& fileName,
	                                             int first, int last);

	/// Get the number of bins in the current histogram
	int rowsInCurrentObject(const QString& fileName);

	/// Identifier of the current histogram
	QString currentObject;
	/// First row to read (can be -1, skips the underflow bin 0)
	int startRow = -1;
	/// Last row to read (can be -1, skips the overflow bin)
	int endRow = -1;
	/// Columns to read
	QVector<QStringList> columns;
private:
	/// Checks and updates the current ROOT file path
	void setFile(const QString& fileName);
	/// Calls ReadHistogram from ROOTHist
	std::vector<ROOTData::BinPars> readHistogram(const QString& histName);
	/// Calls listEntries from ROOTHist
	std::vector<double> readTree(const QString& treeName, const QString& branchName, const QString& leafName, int element, int last);

	/// Information about currently set ROOT file
    struct {
        QString name;
        QDateTime modified;
        qint64 size;
    } currentFile;
	/// ROOTHist instance kept alive while currentFile does not change
	std::unique_ptr<ROOTData> currentROOTData;
};

#endif
