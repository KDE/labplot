all: nsl_stats_test nsl_filter_test nsl_filter_test_fftw nsl_geom_linesim_test nsl_geom_linesim_morse_test

nsl_stats_test: nsl_stats_test.c nsl_stats.c
	gcc -o $@ $^ -lm -lgsl -lgslcblas

nsl_filter_test: nsl_filter_test.c nsl_filter.c nsl_sf_poly.c
	gcc -o $@ $^ -lm -lgsl -lgslcblas
nsl_filter_test_fftw: nsl_filter_test.c nsl_filter.c nsl_sf_poly.c
	gcc -o $@ $^ -lm -DHAVE_FFTW3 -lfftw3 -lgsl -lgslcblas
nsl_geom_linesim_test: nsl_geom_linesim_test.c nsl_geom_linesim.c nsl_geom.c nsl_sort.c nsl_stats.c
	gcc -o $@ $^ -lm -lgsl -lgslcblas
nsl_geom_linesim_morse_test: nsl_geom_linesim_morse_test.c nsl_geom_linesim.c nsl_geom.c nsl_sort.c nsl_stats.c
	gcc -O2 -o $@ $^ -lm -lgsl -lgslcblas

clean:
	rm -f nsl_stats_test nsl_dft_test nsl_dft_test_fftw nsl_filter_test nsl_filter_test_fftw nsl_geom_linesim_test nsl_geom_linesim_morse_test nsl_diff_test
