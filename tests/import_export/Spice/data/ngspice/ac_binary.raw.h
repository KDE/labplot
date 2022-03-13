namespace ac_binary {
    const QString filename = "ac_binary.raw";
    const int refDataRowCount = 201;
	const int numberPreviewData = 100;

	QString refFileInfoString = R"(Title: * simulation de rc2
<br>Date: Sat Jun 16 23:11:45  2018
<br>Plotname: AC Analysis
<br>Flags: complex
<br>No. Variables: 8
<br>No. Points: 201
<br>Variables:
<br>	0	frequency	frequency grid=3
<br>	1	n0	voltage
<br>	2	n1	voltage
<br>	3	n2	voltage
<br>	4	n3	voltage
<br>	5	n4	voltage
<br>	6	n5	voltage
<br>	7	i(v1)	current
)"; // last \n is important

	QStringList columnNames = {"frequency, frequency grid=3 REAL", "frequency, frequency grid=3 IMAGINARY",
							   "n0, voltage REAL", "n0, voltage IMAGINARY",
							   "n1, voltage REAL", "n1, voltage IMAGINARY",
							   "n2, voltage REAL", "n2, voltage IMAGINARY",
							   "n3, voltage REAL", "n3, voltage IMAGINARY",
							   "n4, voltage REAL", "n4, voltage IMAGINARY",
							   "n5, voltage REAL", "n5, voltage IMAGINARY",
							   "i(v1), current REAL", "i(v1), current IMAGINARY"};
} // namespace ac_binary
