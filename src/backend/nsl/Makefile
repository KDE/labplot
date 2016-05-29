all: nsl_stats_test nsl_smooth_ma_test nsl_smooth_mal_test nsl_smooth_savgol_test

nsl_stats_test: nsl_stats_test.c nsl_stats.c
	gcc -o $@ $^ -lm -lgsl -lgslcblas

nsl_smooth_ma_test: nsl_smooth_ma_test.c nsl_smooth.c nsl_sf_kernel.c nsl_stats.c
	gcc -o $@ $^ -lm -lgsl -lgslcblas
nsl_smooth_mal_test: nsl_smooth_mal_test.c nsl_smooth.c nsl_sf_kernel.c nsl_stats.c
	gcc -o $@ $^ -lm -lgsl -lgslcblas
nsl_smooth_savgol_test: nsl_smooth_savgol_test.c nsl_smooth.c nsl_sf_kernel.c nsl_stats.c
	gcc -o $@ $^ -lm -lgsl -lgslcblas

clean:
	rm -f nsl_stats_test nsl_smooth_ma_test nsl_smooth_mal_test nsl_smooth_savgol_test
