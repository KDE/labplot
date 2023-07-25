#include <fstream>
#include <iostream>
#include <math.h>
#include <string>

#include <gsl/gsl_randist.h>
#include <gsl/gsl_rng.h>

int main() {
	std::string fileName = "out_1M_5pathes.txt";
	int count = 1000000;
	int pathes = 5;

	double delta = 0.25;
	int dt = 1;
	double sigma = std::pow(delta, 2) * dt;
	double path[pathes] = {0.0};

	std::ofstream ostrm(fileName, std::ios::out);

	// header
	ostrm << 't';
	for (int p = 0; p < pathes; ++p)
		ostrm << "\tx" << std::to_string(p + 1);

	ostrm << '\n';

	// create a generator chosen by the environment variable GSL_RNG_TYPE
	gsl_rng_env_setup();
	const gsl_rng_type* T = gsl_rng_default;
	gsl_rng* r = gsl_rng_alloc(T);
	gsl_rng_set(r, 12345);

	// data
	for (int i = 0; i < count; ++i) {
		ostrm << std::to_string(i * dt);

		for (int p = 0; p < pathes; ++p) {
			path[p] += gsl_ran_gaussian(r, sigma);
			ostrm << '\t' << std::to_string(path[p]);
		}
		ostrm << '\n';
	}
}
