all: nsl_stats_test nsl_smooth_ma_test nsl_smooth_mal_test nsl_smooth_percentile_test nsl_smooth_savgol_test nsl_dft_test nsl_dft_test_fftw nsl_sf_window_test

nsl_stats_test: nsl_stats_test.c nsl_stats.c
	gcc -o $@ $^ -lm -lgsl -lgslcblas

nsl_smooth_ma_test: nsl_smooth_ma_test.c nsl_smooth.c nsl_sf_kernel.c nsl_stats.c
	gcc -o $@ $^ -lm -lgsl -lgslcblas
nsl_smooth_mal_test: nsl_smooth_mal_test.c nsl_smooth.c nsl_sf_kernel.c nsl_stats.c
	gcc -o $@ $^ -lm -lgsl -lgslcblas
nsl_smooth_percentile_test: nsl_smooth_percentile_test.c nsl_smooth.c nsl_sf_kernel.c nsl_stats.c
	gcc -o $@ $^ -lm -lgsl -lgslcblas
nsl_smooth_savgol_test: nsl_smooth_savgol_test.c nsl_smooth.c nsl_sf_kernel.c nsl_stats.c
	gcc -o $@ $^ -lm -lgsl -lgslcblas
nsl_dft_test: nsl_dft_test.c nsl_dft.c
	gcc -o $@ $^ -lm -lgsl -lgslcblas
nsl_dft_test_fftw: nsl_dft_test.c nsl_dft.c
	gcc -o $@ $^ -lm -DHAVE_FFTW3 -lfftw3 -lgsl -lgslcblas
nsl_sf_window_test: nsl_sf_window_test.c nsl_sf_window.c
	gcc -o $@ $^ -lm -lgsl -lgslcblas

clean:
	rm -f nsl_stats_test nsl_smooth_ma_test nsl_smooth_mal_test nsl_smooth_percentile_test nsl_smooth_savgol_test nsl_dft_test nsl_dft_test_fftw nsl_sf_window_test
