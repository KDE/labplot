all: nsl_geom_linesim_test nsl_geom_linesim_morse_test

nsl_geom_linesim_test: nsl_geom_linesim_test.c nsl_geom_linesim.c nsl_geom.c nsl_sort.c nsl_stats.c
	gcc -o $@ $^ -lm -lgsl -lgslcblas
nsl_geom_linesim_morse_test: nsl_geom_linesim_morse_test.c nsl_geom_linesim.c nsl_geom.c nsl_sort.c nsl_stats.c
	gcc -O2 -o $@ $^ -lm -lgsl -lgslcblas

clean:
	rm -f nsl_dft_test nsl_dft_test_fftw nsl_geom_linesim_test nsl_geom_linesim_morse_test nsl_diff_test
