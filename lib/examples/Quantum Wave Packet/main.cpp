#include <QApplication>
#include <QTimer>
#include <QElapsedTimer>
#include <cmath>
#include <complex>
#include <vector>

#include <labplot.h>

// Physical constants (natural units: ℏ = m = 1)
constexpr double HBAR = 1.0;
constexpr double MASS = 1.0;

// Wave packet parameters
constexpr double X0 = 0.0;      // Initial position
constexpr double K0 = 5.0;      // Initial wave number
constexpr double SIGMA0 = 1.0;  // Initial width

// Spatial grid
constexpr int N_POINTS = 512;
constexpr double X_MIN = -10.0;
constexpr double X_MAX = 10.0;

// Time parameters
constexpr int N_STEPS = 100;
constexpr double T_MAX = 3.0;
constexpr double DT = T_MAX / N_STEPS;

// Animation delay (ms)
constexpr int FRAME_DELAY = 50;

/**
 * Compute the wave function ψ(x,t) for a free Gaussian wave packet
 * Returns complex-valued wave function
 */
std::vector<std::complex<double>> computeWaveFunction(const std::vector<double>& x, double t) {
	std::vector<std::complex<double>> psi(x.size());

	// Time-dependent width
	double sigma_t = SIGMA0 * std::sqrt(1.0 + std::pow(HBAR * t / (MASS * SIGMA0 * SIGMA0), 2));

	// Normalization factor
	double norm = 1.0 / std::sqrt(sigma_t * std::sqrt(M_PI));

	for (size_t i = 0; i < x.size(); ++i) {
		double dx = x[i] - X0;
		double dx_shifted = dx - HBAR * K0 * t / MASS;

		// Phase factors
		std::complex<double> phase1(0.0, K0 * dx);
		std::complex<double> phase2(-dx_shifted * dx_shifted / (2.0 * sigma_t * sigma_t), 0.0);
		std::complex<double> phase3(0.0, HBAR * K0 * K0 * t / (2.0 * MASS));

		psi[i] = norm * std::exp(phase1 + phase2 + phase3);
	}

	return psi;
}

int main(int argc, char** argv) {
	QApplication app(argc, argv);

	// Setup spatial grid
	std::vector<double> x(N_POINTS);
	double dx = (X_MAX - X_MIN) / (N_POINTS - 1);
	for (int i = 0; i < N_POINTS; ++i) {
		x[i] = X_MIN + i * dx;
	}

	// Create project structure
	auto* project = new Project();

	// Create spreadsheet with data columns
	auto* spreadsheet = new Spreadsheet(QStringLiteral("Wave Data"));
	project->addChild(spreadsheet);

	// Get columns (automatically created)
	auto* colX = spreadsheet->column(0);
	auto* colReal = spreadsheet->column(1);
	auto* colImag = spreadsheet->column(2);
	auto* colProb = spreadsheet->column(3);

	colX->setName(QStringLiteral("x"));
	colReal->setName(QStringLiteral("Re(ψ)"));
	colImag->setName(QStringLiteral("Im(ψ)"));
	colProb->setName(QStringLiteral("|ψ|²"));

	// Initialize with t=0 data
	QVector<double> x_data, real_data, imag_data, prob_data;
	x_data.reserve(N_POINTS);
	real_data.reserve(N_POINTS);
	imag_data.reserve(N_POINTS);
	prob_data.reserve(N_POINTS);

	auto psi0 = computeWaveFunction(x, 0.0);
	for (int i = 0; i < N_POINTS; ++i) {
		x_data.append(x[i]);
		real_data.append(psi0[i].real());
		imag_data.append(psi0[i].imag());
		prob_data.append(std::norm(psi0[i]));
	}

	colX->replaceValues(0, x_data);
	colReal->replaceValues(0, real_data);
	colImag->replaceValues(0, imag_data);
	colProb->replaceValues(0, prob_data);

	// Create worksheet
	auto* worksheet = new Worksheet(QStringLiteral("Quantum Wave Packet"));
	project->addChild(worksheet);

	worksheet->setUseViewSize(false);
	double w = Worksheet::convertToSceneUnits(20, Worksheet::Unit::Centimeter);
	double h = Worksheet::convertToSceneUnits(15, Worksheet::Unit::Centimeter);
	worksheet->setPageRect(QRectF(0, 0, w, h));
	worksheet->setTheme(QStringLiteral("Tufte"));

	// Create plot area
	auto* plotArea = new CartesianPlot(QStringLiteral("Wave Function Plot"));
	plotArea->setType(CartesianPlot::Type::FourAxes);
	plotArea->title()->setText(QStringLiteral("Quantum Wave Packet Evolution"));
	worksheet->addChild(plotArea);

	// Configure axes
	for (auto* axis : plotArea->children<Axis>()) {
		if (axis->orientation() == WorksheetElement::Orientation::Horizontal
			&& axis->position() == Axis::Position::Bottom) {
			axis->title()->setText(QStringLiteral("Position x"));
		} else if (axis->orientation() == WorksheetElement::Orientation::Vertical
				   && axis->position() == Axis::Position::Left) {
			axis->title()->setText(QStringLiteral("ψ(x,t)"));
		}
	}

	// Create curves
	auto* curveReal = new XYCurve(QStringLiteral("Re(ψ)"));
	curveReal->setXColumn(colX);
	curveReal->setYColumn(colReal);
	curveReal->setLineType(XYCurve::LineType::Line);
	curveReal->symbol()->setStyle(Symbol::Style::NoSymbols);
	plotArea->addChild(curveReal);

	auto* curveImag = new XYCurve(QStringLiteral("Im(ψ)"));
	curveImag->setXColumn(colX);
	curveImag->setYColumn(colImag);
	curveImag->setLineType(XYCurve::LineType::Line);
	curveImag->symbol()->setStyle(Symbol::Style::NoSymbols);
	plotArea->addChild(curveImag);

	auto* curveProb = new XYCurve(QStringLiteral("|ψ|²"));
	curveProb->setXColumn(colX);
	curveProb->setYColumn(colProb);
	curveProb->setLineType(XYCurve::LineType::Line);
	curveProb->symbol()->setStyle(Symbol::Style::NoSymbols);
	curveProb->line()->setWidth(Worksheet::convertToSceneUnits(3, Worksheet::Unit::Point));
	plotArea->addChild(curveProb);

	plotArea->addLegend();

	// Show the worksheet
	worksheet->view()->show();

	// Animation using QTimer
	int currentStep = 0;
	QTimer* timer = new QTimer();

	QObject::connect(timer, &QTimer::timeout, [&]() {
		if (currentStep >= N_STEPS) {
			timer->stop();
			qDebug() << "Animation complete!";
			return;
		}

		double t = currentStep * DT;
		auto psi_t = computeWaveFunction(x, t);

		// Update column data
		real_data.clear();
		imag_data.clear();
		prob_data.clear();

		for (const auto& val : psi_t) {
			real_data.append(val.real());
			imag_data.append(val.imag());
			prob_data.append(std::norm(val));
		}

		colReal->replaceValues(0, real_data);
		colImag->replaceValues(0, imag_data);
		colProb->replaceValues(0, prob_data);

		if ((currentStep + 1) % 10 == 0) {
			qDebug() << "Frame" << (currentStep + 1) << "/" << N_STEPS << "(t =" << t << ")";
		}

		currentStep++;
	});

	// Start animation
	timer->start(FRAME_DELAY);

	return app.exec();
}
