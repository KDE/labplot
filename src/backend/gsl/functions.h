#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include <gsl/gsl_math.h>
#include <gsl/gsl_sf.h>
#include <gsl/gsl_randist.h>

/********* redefine functions to use double parameter *********/

/* stdlib.h */
double my_rand() { return rand(); }
double my_random() { return random(); }
double my_drand() { return random()/(double)RAND_MAX; }
/* math.h */
double my_jn(double n, double x) { return jn((int)n,x); }
double my_ldexp(double x, double expo) { return ldexp(x,(int)expo); }
double my_yn(double n,double x) { return yn((int)n,x); }

/* wrapper for GSL functions with integer parameters */
#ifdef HAVE_GSL
#define MODE GSL_PREC_DOUBLE
/* Airy functions */
double airy_Ai(double x) { return gsl_sf_airy_Ai(x,MODE); }
double airy_Bi(double x) { return gsl_sf_airy_Bi(x,MODE); }
double airy_Ais(double x) { return gsl_sf_airy_Ai_scaled(x,MODE); }
double airy_Bis(double x) { return gsl_sf_airy_Bi_scaled(x,MODE); }
double airy_Aid(double x) { return gsl_sf_airy_Ai_deriv(x,MODE); }
double airy_Bid(double x) { return gsl_sf_airy_Bi_deriv(x,MODE); }
double airy_Aids(double x) { return gsl_sf_airy_Ai_deriv_scaled(x,MODE); }
double airy_Bids(double x) { return gsl_sf_airy_Bi_deriv_scaled(x,MODE); }
double airy_0_Ai(double s) { return gsl_sf_airy_zero_Ai((unsigned int)s); }
double airy_0_Bi(double s) { return gsl_sf_airy_zero_Bi((unsigned int)s); }
double airy_0_Aid(double s) { return gsl_sf_airy_zero_Ai_deriv((unsigned int)s); }
double airy_0_Bid(double s) { return gsl_sf_airy_zero_Bi_deriv((unsigned int)s); }
/* Bessel functions */
double bessel_Jn(double n,double x) { return gsl_sf_bessel_Jn((int)n,x); }
double bessel_Yn(double n,double x) { return gsl_sf_bessel_Yn((int)n,x); }
double bessel_In(double n,double x) { return gsl_sf_bessel_In((int)n,x); }
double bessel_Ins(double n,double x) { return gsl_sf_bessel_In_scaled((int)n,x); }
double bessel_Kn(double n,double x) { return gsl_sf_bessel_Kn((int)n,x); }
double bessel_Kns(double n,double x) { return gsl_sf_bessel_Kn_scaled((int)n,x); }
double bessel_jl(double l,double x) { return gsl_sf_bessel_jl((int)l,x); }
double bessel_yl(double l,double x) { return gsl_sf_bessel_yl((int)l,x); }
double bessel_ils(double l,double x) { return gsl_sf_bessel_il_scaled((int)l,x); }
double bessel_kls(double l,double x) { return gsl_sf_bessel_kl_scaled((int)l,x); }
double bessel_0_J0(double s) { return gsl_sf_bessel_zero_J0((unsigned int)s); }
double bessel_0_J1(double s) { return gsl_sf_bessel_zero_J1((unsigned int)s); }
double bessel_0_Jnu(double nu,double s) { return gsl_sf_bessel_zero_Jnu(nu,(unsigned int)s); }

double hydrogenicR(double n, double l, double z, double r) { return gsl_sf_hydrogenicR((int)n,(int)l,z,r); }
/* elliptic integrals */
double ellint_Kc(double x) { return gsl_sf_ellint_Kcomp(x,MODE); }
double ellint_Ec(double x) { return gsl_sf_ellint_Ecomp(x,MODE); }
double ellint_F(double phi,double k) { return gsl_sf_ellint_F(phi,k,MODE); }
double ellint_E(double phi,double k) { return gsl_sf_ellint_E(phi,k,MODE); }
double ellint_P(double phi,double k,double n) { return gsl_sf_ellint_P(phi,k,n,MODE); }
double ellint_D(double phi,double k,double n) { return gsl_sf_ellint_D(phi,k,n,MODE); }
double ellint_RC(double x,double y) { return gsl_sf_ellint_RC(x,y,MODE); }
double ellint_RD(double x,double y,double z) { return gsl_sf_ellint_RD(x,y,z,MODE); }
double ellint_RF(double x,double y,double z) { return gsl_sf_ellint_RF(x,y,z,MODE); }
double ellint_RJ(double x,double y,double z, double p) { return gsl_sf_ellint_RJ(x,y,z,p,MODE); }

double exprel_n(double n,double x) { return gsl_sf_exprel_n((int)n,x); }
double fermi_dirac_int(double j,double x) { return gsl_sf_fermi_dirac_int((int)j,x); }
/* gamma */
double fact(double n) { return gsl_sf_fact((unsigned int)n); }
double doublefact(double n) { return gsl_sf_doublefact((unsigned int)n); }
double lnfact(double n) { return gsl_sf_lnfact((unsigned int)n); }
double lndoublefact(double n) { return gsl_sf_lndoublefact((unsigned int)n); }
double choose(double n,double m) { return gsl_sf_choose((unsigned int)n,(unsigned int)m); }
double lnchoose(double n,double m) { return gsl_sf_lnchoose((unsigned int)n,(unsigned int)m); }
double taylorcoeff(double n,double x) { return gsl_sf_taylorcoeff((int)n,x); }

double gegenpoly_n(double n,double l,double x) { return gsl_sf_gegenpoly_n((int)n,l,x); }
double hyperg_1F1i(double m,double n,double x) { return gsl_sf_hyperg_1F1_int((int)m,(int)n,x); }
double hyperg_Ui(double m,double n,double x) { return gsl_sf_hyperg_U_int((int)m,(int)n,x); }
double laguerre_n(double n,double a,double x) { return gsl_sf_laguerre_n((int)n,a,x); }

double legendre_Pl(double l,double x) { return gsl_sf_legendre_Pl((int)l,x); }
double legendre_Ql(double l,double x) { return gsl_sf_legendre_Ql((int)l,x); }
double legendre_Plm(double l,double m,double x) { return gsl_sf_legendre_Plm((int)l,(int)m,x); }
double legendre_sphPlm(double l,double m,double x) { return gsl_sf_legendre_sphPlm((int)l,(int)m,x); }
double conicalP_sphreg(double l,double L,double x) { return gsl_sf_conicalP_sph_reg((int)l,L,x); }
double conicalP_cylreg(double m,double l,double x) { return gsl_sf_conicalP_sph_reg((int)m,l,x); }
double legendre_H3d(double l, double L,double e) { return gsl_sf_legendre_H3d((int)l,L,e); }

double gsl_pow(double x, double n) { return gsl_sf_pow_int(x,(int)n); }
double psii(double n) { return gsl_sf_psi_int((int)n); }
double psi1i(double n) { return gsl_sf_psi_1_int((int)n); }
double psi_n(double m, double x) { return gsl_sf_psi_n((int)m,x); }

double zetai(double n) { return gsl_sf_zeta_int((int)n); }
double etai(double n) { return gsl_sf_eta_int((int)n); }

/* random number distributions */
double poisson(double k, double m) { return gsl_ran_poisson_pdf((unsigned int)k,m); }
double bernoulli(double k, double p) { return gsl_ran_bernoulli_pdf((unsigned int)k,p); }
double binomial(double k, double p,double n) { return gsl_ran_binomial_pdf((unsigned int)k,p,(unsigned int)n); }
double negative_binomial(double k, double p,double n) { return gsl_ran_negative_binomial_pdf((unsigned int)k,p,n); }
double pascal(double k, double p,double n) { return gsl_ran_pascal_pdf((unsigned int)k,p,(unsigned int)n); }
double geometric(double k, double p) { return gsl_ran_geometric_pdf((unsigned int)k,p); }
double hypergeometric(double k, double n1,double n2,double t) {
	return gsl_ran_hypergeometric_pdf((unsigned int)k,(unsigned int)n1,(unsigned int)n2,(unsigned int)t);
}
double logarithmic(double k, double p) { return gsl_ran_logarithmic_pdf((unsigned int)k,p); }
#endif

struct func _functions[] = {
	/* Standard functions */
	/* stdlib.h */
	{"rand",my_rand},
	{"random",my_random},
	{"drand",my_drand},
	/* math.h */
	{"acos",acos},
	{"acosh",acosh},
	{"asin",asin},
	{"asinh",asinh},
	{"atan",atan},
	{"atan2",atan2},
	{"atanh",atanh},
	{"cbrt",cbrt},
	{"ceil",ceil},
	{"cos", cos},
	{"cosh",cosh},
	{"erf",erf},
	{"erfc",erfc},	
	{"exp", exp},	
	{"expm1",expm1},
	{"fabs",fabs},
	{"gamma",gamma},
	{"hypot",hypot},
	{"j0",j0},
	{"j1",j1},
	{"jn",my_jn},
	{"ldexp",my_ldexp},
	{"lgamma",lgamma},
	{"ln", log},	/* german version natural log */
	{"log",log},
	{"log10",log10},
	{"log1p",log1p},
	{"logb",logb},
	{"pow",pow},
	{"rint",rint},
	{"round",round},
	{"sin", sin},
	{"sinh",sinh},
	{"sqrt", sqrt},
	{"tan",tan},
	{"tanh",tanh},
	{"tgamma",tgamma},
	{"trunc",trunc},
	{"y0",y0},
	{"y1",y1},
	{"yn",my_yn},

	/* GSL functions: see http://www.gnu.org/software/gsl/manual/html_node/Special-Functions.html */
  {"gsl_log1p",gsl_log1p},
  {"gsl_expm1",gsl_expm1},
  {"gsl_hypot",gsl_hypot},
  {"gsl_acosh",gsl_acosh},
  {"gsl_asinh",gsl_asinh},
  {"gsl_atanh",gsl_atanh},
  {"Ai",airy_Ai},
  {"Bi",airy_Bi},
  {"Ais",airy_Ais},
  {"Bis",airy_Bis},
  {"Aid",airy_Aid},
  {"Bid",airy_Bid},
  {"Aids",airy_Aids},
  {"Bids",airy_Bids},
  {"Ai0",airy_0_Ai},
  {"Bi0",airy_0_Bi},
  {"Aid0",airy_0_Aid},
  {"Bid0",airy_0_Bid},
  {"J0",gsl_sf_bessel_J0},
  {"J1",gsl_sf_bessel_J1},
  {"Jn",bessel_Jn},
  {"Y0",gsl_sf_bessel_Y0},
  {"Y1",gsl_sf_bessel_Y1},
  {"Yn",bessel_Yn},
  {"I0",gsl_sf_bessel_I0},
  {"I1",gsl_sf_bessel_I1},
  {"In",bessel_In},
  {"I0s",gsl_sf_bessel_I0_scaled},
  {"I1s",gsl_sf_bessel_I1_scaled},
  {"Ins",bessel_Ins},
  {"K0",gsl_sf_bessel_K0},
  {"K1",gsl_sf_bessel_K1},
  {"Kn",bessel_Kn},
  {"K0s",gsl_sf_bessel_K0_scaled},
  {"K1s",gsl_sf_bessel_K1_scaled},
  {"Kns",bessel_Kns},
  {"gsl_j0",gsl_sf_bessel_j0},
  {"gsl_j1",gsl_sf_bessel_j1},
  {"j2",gsl_sf_bessel_j2},
  {"jl",bessel_jl},
  {"gsl_y0",gsl_sf_bessel_y0},
  {"gsl_y1",gsl_sf_bessel_y1},
  {"y2",gsl_sf_bessel_y2},
  {"yl",bessel_yl},
  {"i0s",gsl_sf_bessel_i0_scaled},
  {"i1s",gsl_sf_bessel_i1_scaled},
  {"i2s",gsl_sf_bessel_i2_scaled},
  {"ils",bessel_ils},
  {"k0s",gsl_sf_bessel_k0_scaled},
  {"k1s",gsl_sf_bessel_k1_scaled},
  {"k2s",gsl_sf_bessel_k2_scaled},
  {"kls",bessel_kls},
  {"Jnu",gsl_sf_bessel_Jnu},
  {"Ynu",gsl_sf_bessel_Ynu},
  {"Inu",gsl_sf_bessel_Inu},
  {"Inus",gsl_sf_bessel_Inu_scaled},
  {"Knu",gsl_sf_bessel_Knu},
  {"lnKnu",gsl_sf_bessel_lnKnu},
  {"Knus",gsl_sf_bessel_Knu_scaled},
  {"J0_0",bessel_0_J0},
  {"J1_0",bessel_0_J1},
  {"Jnu_0",bessel_0_Jnu},
  {"clausen",gsl_sf_clausen},
  {"hydrogenicR_1",gsl_sf_hydrogenicR_1},
  {"hydrogenicR",hydrogenicR},
  {"dawson",gsl_sf_dawson},
  {"debye_1",gsl_sf_debye_1},
  {"debye_2",gsl_sf_debye_2},
  {"debye_3",gsl_sf_debye_3},
  {"debye_4",gsl_sf_debye_4},
  {"debye_5",gsl_sf_debye_5},
  {"debye_6",gsl_sf_debye_6},
  {"dilog",gsl_sf_dilog},
  {"ellint_Kc",ellint_Kc},
  {"ellint_Ec",ellint_Ec},
  {"ellint_F",ellint_F},
  {"ellint_E",ellint_E},
  {"ellint_P",ellint_P},
  {"ellint_D",ellint_D},
  {"ellint_RC",ellint_RC},
  {"ellint_RD",ellint_RD},
  {"ellint_RF",ellint_RF},
  {"ellint_RJ",ellint_RJ},
  {"gsl_erf",gsl_sf_erf},
  {"gsl_erfc",gsl_sf_erfc},
  {"log_erfc",gsl_sf_log_erfc},
  {"erf_z",gsl_sf_erf_Z},
  {"erf_q",gsl_sf_erf_Q},
  {"gsl_exp",gsl_sf_exp},
  {"exprel",gsl_sf_exprel},
  {"exprel_2",gsl_sf_exprel_2},
  {"exprel_n",exprel_n},
  {"expint_e1",gsl_sf_expint_E1},
  {"expint_e2",gsl_sf_expint_E2},
  {"expint_ei",gsl_sf_expint_Ei},
  {"shi",gsl_sf_Shi},
  {"chi",gsl_sf_Chi},
  {"expint_3",gsl_sf_expint_3},
  {"si",gsl_sf_Si},
  {"ci",gsl_sf_Ci},
  {"atanint",gsl_sf_atanint},
  {"fermi_dirac_m1",gsl_sf_fermi_dirac_m1},
  {"fermi_dirac_0",gsl_sf_fermi_dirac_0},
  {"fermi_dirac_1",gsl_sf_fermi_dirac_1},
  {"fermi_dirac_2",gsl_sf_fermi_dirac_2},
  {"fermi_dirac_int",fermi_dirac_int},
  {"fermi_dirac_mhalf",gsl_sf_fermi_dirac_mhalf},
  {"fermi_dirac_half",gsl_sf_fermi_dirac_half},
  {"fermi_dirac_3half",gsl_sf_fermi_dirac_3half},
  {"fermi_dirac_inc_0",gsl_sf_fermi_dirac_inc_0},
  {"gsl_gamma",gsl_sf_gamma},
  {"lngamma",gsl_sf_lngamma},
  {"gammastar",gsl_sf_gammastar},
  {"gammainv",gsl_sf_gammainv},
  {"fact",fact},
  {"doublefact",doublefact},
  {"lnfact",lnfact},
  {"lndoublefact",lndoublefact},
  {"choose",choose},
  {"lnchoose",lnchoose},
  {"taylorcoeff",taylorcoeff},
  {"poch",gsl_sf_poch},
  {"lnpoch",gsl_sf_lnpoch},
  {"pochrel",gsl_sf_pochrel},
  {"gamma_inc",gsl_sf_gamma_inc},
  {"gamma_inc_Q",gsl_sf_gamma_inc_Q},
  {"gamma_inc_P",gsl_sf_gamma_inc_P},
  {"beta",gsl_sf_beta},
  {"lnbeta",gsl_sf_lnbeta},
  {"beta_inc",gsl_sf_beta_inc},
  {"gegenpoly_1",gsl_sf_gegenpoly_1},
  {"gegenpoly_2",gsl_sf_gegenpoly_2},
  {"gegenpoly_3",gsl_sf_gegenpoly_3},
  {"gegenpoly_n",gegenpoly_n},
  {"hyperg_0F1",gsl_sf_hyperg_0F1},
  {"hyperg_1F1i",hyperg_1F1i},
  {"hyperg_1F1",gsl_sf_hyperg_1F1},
  {"hyperg_Ui",hyperg_Ui},
  {"hyperg_U",gsl_sf_hyperg_U},
  {"hyperg_2F1",gsl_sf_hyperg_2F1},
  {"hyperg_2F1c",gsl_sf_hyperg_2F1_conj},
  {"hyperg_2F1r",gsl_sf_hyperg_2F1_renorm},
  {"hyperg_2F1cr",gsl_sf_hyperg_2F1_conj_renorm},
  {"hyperg_2F0",gsl_sf_hyperg_2F0},
  {"laguerre_1",gsl_sf_laguerre_1},
  {"laguerre_2",gsl_sf_laguerre_2},
  {"laguerre_3",gsl_sf_laguerre_3},
  {"lambert_W0",gsl_sf_lambert_W0},
  {"lambert_Wm1",gsl_sf_lambert_Wm1},
  {"legendre_p1",gsl_sf_legendre_P1},
  {"legendre_p2",gsl_sf_legendre_P2},
  {"legendre_p3",gsl_sf_legendre_P3},
  {"legendre_pl",legendre_Pl},
  {"legendre_Q0",gsl_sf_legendre_Q0},
  {"legendre_Q1",gsl_sf_legendre_Q1},
  {"legendre_Ql",legendre_Ql},
  {"legendre_Plm",legendre_Plm},
  {"legendre_sphPlm",legendre_sphPlm},
  {"conicalP_half",gsl_sf_conicalP_half},
  {"conicalP_mhalf",gsl_sf_conicalP_mhalf},
  {"conicalP_0",gsl_sf_conicalP_0},
  {"conicalP_1",gsl_sf_conicalP_1},
  {"conicalP_sphreg",conicalP_sphreg},
  {"conicalP_cylreg",conicalP_cylreg},
  {"legendre_H3d_0",gsl_sf_legendre_H3d_0},
  {"legendre_H3d_1",gsl_sf_legendre_H3d_1},
  {"legendre_H3d",legendre_H3d},
  {"gsl_log",gsl_sf_log},
  {"loga",gsl_sf_log_abs},
  {"logp",gsl_sf_log_1plusx},
  {"logm",gsl_sf_log_1plusx_mx},
  {"gsl_pow",gsl_pow},
  {"psii",psii},
  {"psi",gsl_sf_psi},
  {"psiy",gsl_sf_psi_1piy},
  {"psi1i",psi1i},
  {"psi_n",psi_n},
  {"synchrotron_1",gsl_sf_synchrotron_1},
  {"synchrotron_2",gsl_sf_synchrotron_2},
  {"transport_2",gsl_sf_transport_2},
  {"transport_3",gsl_sf_transport_3},
  {"transport_4",gsl_sf_transport_4},
  {"transport_5",gsl_sf_transport_5},
  {"gsl_sin",gsl_sf_sin},
  {"gsl_cos",gsl_sf_cos},
  {"gsl_hypot",gsl_sf_hypot},
  {"sinc",gsl_sf_sinc},
  {"lnsinh",gsl_sf_lnsinh},
  {"lncosh",gsl_sf_lncosh},
  {"zetai",zetai},
  {"zeta",gsl_sf_zeta},
  {"zetam1",gsl_sf_zetam1},
  {"hzeta",gsl_sf_hzeta},
  {"etai",etai},
  {"eta",gsl_sf_eta},
/* from gsl_randist.h */
  {"gaussian",gsl_ran_gaussian_pdf},
  {"ugaussian",gsl_ran_ugaussian_pdf},
  {"gaussian_tail",gsl_ran_gaussian_tail_pdf},
  {"ugaussian_tail",gsl_ran_ugaussian_tail_pdf},
  {"bivariate_gaussian",gsl_ran_bivariate_gaussian_pdf},
  {"exponential",gsl_ran_exponential_pdf},
  {"laplace",gsl_ran_laplace_pdf},
  {"exppow",gsl_ran_exppow_pdf},
  {"cauchy",gsl_ran_cauchy_pdf},
  {"rayleigh",gsl_ran_rayleigh_pdf},
  {"rayleigh_tail",gsl_ran_rayleigh_tail_pdf},
  {"landau",gsl_ran_landau_pdf},
  {"gamma_pdf",gsl_ran_gamma_pdf},
  {"flat",gsl_ran_flat_pdf},
  {"lognormal",gsl_ran_lognormal_pdf},
  {"chisq",gsl_ran_chisq_pdf},
  {"fdist",gsl_ran_fdist_pdf},
  {"tdist",gsl_ran_tdist_pdf},
  {"beta_pdf",gsl_ran_beta_pdf},
  {"logistic",gsl_ran_logistic_pdf},
  {"pareto",gsl_ran_pareto_pdf},
  {"weibull",gsl_ran_weibull_pdf},
  {"gumbel1",gsl_ran_gumbel1_pdf},
  {"gumbel2",gsl_ran_gumbel2_pdf},
  {"poisson",poisson},
  {"bernoulli",bernoulli},
  {"binomial",binomial},
  {"negative_binomial",negative_binomial},
  {"pascal",pascal},
  {"geometric",geometric},
  {"hypergeometric",hypergeometric},
  {"logarithmic",logarithmic},
  {0, 0}
};

#endif /*FUNCTIONS_H*/
