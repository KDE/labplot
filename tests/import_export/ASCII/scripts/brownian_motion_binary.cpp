#include <fstream>
#include <iostream>
#include <math.h>

#include <gsl/gsl_randist.h>
#include <gsl/gsl_rng.h>

int main() {
	std::string fileName = "out_1M_5pathes.bin";
	int count = 1000000;
	int pathes = 5;

	double delta = 0.25;
	int dt = 1;
	double sigma = std::pow(delta, 2) * dt;
	double path[pathes] = {0.0};

	std::ofstream ostrm(fileName, std::ios::binary);

	// create a generator chosen by the environment variable GSL_RNG_TYPE
	gsl_rng_env_setup();
	const gsl_rng_type* T = gsl_rng_default;
	gsl_rng* r = gsl_rng_alloc(T);
	gsl_rng_set(r, 12345);

	// data
	for (int i = 0; i < count; ++i) {
		double x = i * dt;
		ostrm.write(reinterpret_cast<char*>(&x), sizeof(double));

		for (int p = 0; p < pathes; ++p) {
			path[p] += gsl_ran_gaussian(r, sigma);
			ostrm.write(reinterpret_cast<char*>(&path[p]), sizeof(double));
		}
	}
}
