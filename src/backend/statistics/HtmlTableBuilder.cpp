#include "HtmlTableBuilder.h"
#include <QString>

HtmlCell::HtmlCell(const QString& cellContent, bool header, const QString& tip, int rSpan, int cSpan, const QString& extra)
	: m_content(cellContent)
	, m_isHeader(header)
	, m_tooltip(tip)
	, m_rowSpan(rSpan)
	, m_colSpan(cSpan)
	, m_extraAttributes(extra) {
}

HtmlRow& HtmlRow::addCell(const HtmlCell& cell) {
	m_cells.append(cell);
	return *this;
}

HtmlRow& HtmlRow::addCells(const QStringList& cellContents, bool isHeader) {
	for (const QString& text : cellContents)
		m_cells.append(HtmlCell(text, isHeader));
	return *this;
}

const QList<HtmlCell>& HtmlRow::cells() const {
	return m_cells;
}

HtmlTableBuilder& HtmlTableBuilder::setStyle(const QString& css) {
	m_style = css;
	return *this;
}

HtmlTableBuilder& HtmlTableBuilder::setTableAttributes(const QString& attributes) {
	m_tableAttributes = attributes;
	return *this;
}

HtmlTableBuilder& HtmlTableBuilder::addRow(const HtmlRow& row) {
	m_rows.append(row);
	return *this;
}

HtmlTableBuilder& HtmlTableBuilder::addRow(const QStringList& cellContents, bool isHeader) {
	HtmlRow row;
	row.addCells(cellContents, isHeader);
	m_rows.append(row);
	return *this;
}
QString HtmlTableBuilder::build() const {
	QString html;
	html.append(QStringLiteral("<style type='text/css'>\n"));
	html.append(m_style);
	html.append(QStringLiteral("\n</style>\n"));
	html.append(QStringLiteral("<div style='text-align: center;'>\n"));
	html.append(QStringLiteral("<table ") + m_tableAttributes + QStringLiteral(">\n"));

	for (const HtmlRow& row : m_rows) {
		html.append(QStringLiteral("<tr>"));
		for (const HtmlCell& cell : row.cells()) {
			QString tag = cell.m_isHeader ? QStringLiteral("th") : QStringLiteral("td");
			QString attributes;

			if (!cell.m_extraAttributes.isEmpty())
				attributes.append(QStringLiteral(" ") + cell.m_extraAttributes);

			if (cell.m_rowSpan > 1)
				attributes.append(QStringLiteral(" rowspan=\"") + QString::number(cell.m_rowSpan) + QStringLiteral("\""));
			if (cell.m_colSpan > 1)
				attributes.append(QStringLiteral(" colspan=\"") + QString::number(cell.m_colSpan) + QStringLiteral("\""));

			QString cellContent = cell.m_content;
			if (!cell.m_tooltip.isEmpty())
				cellContent = QStringLiteral("<span title='") + cell.m_tooltip + QStringLiteral("'>") + cell.m_content + QStringLiteral("</span>");

			html.append(QStringLiteral("<") + tag + attributes + QStringLiteral(">") + cellContent + QStringLiteral("</") + tag + QStringLiteral(">"));
		}
		html.append(QStringLiteral("</tr>\n"));
	}
	html.append(QStringLiteral("</table>\n"));
	html.append(QStringLiteral("</div>\n"));

	return html;
}
