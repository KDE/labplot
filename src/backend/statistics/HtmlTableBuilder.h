#ifndef TABLEBUILDER_H
#define TABLEBUILDER_H

#include <QList>
#include <QString>
#include <QStringList>

/*!
 * \brief The HtmlTableCell struct represents one cell in a table.
 *
 * This struct encapsulates the content and display properties for a single cell,
 * including whether it is a header cell, an optional tooltip, rowspan/colspan, and
 * any extra HTML attributes you might want to include.
 */
struct HtmlCell {
	QString m_content; // The cell's text or HTML content.
	bool m_isHeader; // True if this cell should be rendered as a header (<th>), false for a normal cell (<td>).
	QString m_tooltip; // Optional tooltip text for the cell.
	int m_rowSpan; // Number of rows this cell should span.
	int m_colSpan; // Number of columns this cell should span.
	QString m_extraAttributes; // Any extra HTML attributes (for example, CSS classes or inline styles).

	HtmlCell(const QString& cellContent = QString(),
			 bool header = false,
			 const QString& tip = QString(),
			 int rSpan = 1,
			 int cSpan = 1,
			 const QString& extra = QString());
};

/*!
 * \brief The TableRow class represents a single row in the table.
 *
 * Each row contains a list of HtmlTableCell objects.
 */
class HtmlRow {
public:
	HtmlRow() = default;
	HtmlRow& addCell(const HtmlCell& cell);
	HtmlRow& addCells(const QStringList& cellContents, bool isHeader = false);
	const QList<HtmlCell>& cells() const;

private:
	QList<HtmlCell> m_cells;
};

/*!
 * \brief The TableBuilder class provides a fluent API to build HTML tables.
 *
 * With this class you can add rows (which in turn consist of cells), set a custom CSS
 * style, and add extra attributes to the table tag. Calling build() generates the final HTML.
 */
class HtmlTableBuilder {
public:
	HtmlTableBuilder() = default;
	HtmlTableBuilder& setStyle(const QString& css);
	HtmlTableBuilder& setTableAttributes(const QString& attributes);
	HtmlTableBuilder& addRow(const HtmlRow& row);
	HtmlTableBuilder& addRow(const QStringList& cellContents, bool isHeader = false);
	QString build() const;

private:
	QString m_style = QString::fromLatin1(
		"table { border-collapse: collapse; margin: auto; width: 80%; font-size: 18px; } "
		"th, td { border: 1px solid black; padding: 12px; text-align: center; font-size: 16px; } "
		"th { background-color: #f2f2f2; font-size: 18px; }");
	QString m_tableAttributes;
	QList<HtmlRow> m_rows;
};

#endif // TABLEBUILDER_H
