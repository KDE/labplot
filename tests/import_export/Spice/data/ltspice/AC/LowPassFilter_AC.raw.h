namespace LowPassFilter_AC {

QString refFileInfoString = QLatin1String(R"(Title: * Z:\home\martin\GITProjekte\labplot\tests\import_export\Spice\data\ltspice\AC\LowPassFilter_AC.asc
<br>Date: Fri Mar 11 17:45:05 2022
<br>Plotname: AC Analysis
<br>Flags: complex forward log
<br>No. Variables: 6
<br>No. Points:          801
<br>Offset:   0.0000000000000000e+000
<br>Command: Linear Technology Corporation LTspice XVII
<br>Variables:
<br>	0	frequency	frequency
<br>	1	V(n001)	voltage
<br>	2	V(n002)	voltage
<br>	3	I(C1)	device_current
<br>	4	I(R1)	device_current
<br>	5	I(V1)	device_current
)"); // last \n is important

const QString filename = QStringLiteral("AC/LowPassFilter_AC.raw");
const int refDataRowCount = 801;
const int numberPreviewData = 100;

QStringList columnNames = {QStringLiteral("frequency, frequency REAL"),
						   QStringLiteral("frequency, frequency IMAGINARY"),
						   QStringLiteral("V(n001), voltage REAL"),
						   QStringLiteral("V(n001), voltage IMAGINARY"),
						   QStringLiteral("V(n002), voltage REAL"),
						   QStringLiteral("V(n002), voltage IMAGINARY"),
						   QStringLiteral("I(C1), device_current REAL"),
						   QStringLiteral("I(C1), device_current IMAGINARY"),
						   QStringLiteral("I(R1), device_current REAL"),
						   QStringLiteral("I(R1), device_current IMAGINARY"),
						   QStringLiteral("I(V1), device_current REAL"),
						   QStringLiteral("I(V1), device_current IMAGINARY")};
}
