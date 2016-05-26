all: nsl_stats_test nsl_smooth_test

nsl_stats_test: nsl_stats_test.c nsl_stats.c
	gcc -o $@ $^ -lm -lgsl -lgslcblas

nsl_smooth_test: nsl_smooth_test.c nsl_smooth.c
	gcc -o $@ $^ -lm -lgsl -lgslcblas
