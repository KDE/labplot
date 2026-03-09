/*
 * STL C++ v0.2.0
 * https://github.com/ankane/stl-cpp
 * Unlicense OR MIT License
 *
 * Ported from https://www.netlib.org/a/stl
 *
 * Cleveland, R. B., Cleveland, W. S., McRae, J. E., & Terpenning, I. (1990).
 * STL: A Seasonal-Trend Decomposition Procedure Based on Loess.
 * Journal of Official Statistics, 6(1), 3-33.
 *
 * Bandara, K., Hyndman, R. J., & Bergmeir, C. (2021).
 * MSTL: A Seasonal-Trend Decomposition Algorithm for Time Series with Multiple Seasonal Patterns.
 * arXiv:2107.13462 [stat.AP]. https://doi.org/10.48550/arXiv.2107.13462
 */

#pragma once

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <numeric>
#include <optional>
#include <stdexcept>
#include <tuple>
#include <vector>

#if __cplusplus >= 202002L
#include <span>
#endif

namespace stl {

namespace detail {

template<typename T>
bool est(const std::vector<T>& y, size_t n, size_t len, int ideg, T xs, T* ys, size_t nleft, size_t nright, std::vector<T>& w, bool userw, const std::vector<T>& rw) {
    auto range = ((T) n) - 1.0;
    auto h = std::max(xs - ((T) nleft), ((T) nright) - xs);

    if (len > n) {
        h += (T) ((len - n) / 2);
    }

    auto h9 = 0.999 * h;
    auto h1 = 0.001 * h;

    // compute weights
    auto a = 0.0;
    for (auto j = nleft; j <= nright; j++) {
        w[j - 1] = 0.0;
        auto r = std::abs(((T) j) - xs);
        if (r <= h9) {
            if (r <= h1) {
                w[j - 1] = 1.0;
            } else {
                w[j - 1] = (T) std::pow(1.0 - std::pow(r / h, 3), 3);
            }
            if (userw) {
                w[j - 1] *= rw[j - 1];
            }
            a += w[j - 1];
        }
    }

    if (a <= 0.0) {
        return false;
    } else { // weighted least squares
        for (auto j = nleft; j <= nright; j++) { // make sum of w(j) == 1
            w[j - 1] /= (T) a;
        }

        if (h > 0.0 && ideg > 0) { // use linear fit
            a = 0.0;
            for (auto j = nleft; j <= nright; j++) { // weighted center of x values
                a += w[j - 1] * ((T) j);
            }
            auto b = xs - a;
            auto c = 0.0;
            for (auto j = nleft; j <= nright; j++) {
                c += w[j - 1] * std::pow(((T) j) - a, 2);
            }
            if (std::sqrt(c) > 0.001 * range) {
                b /= c;

                // points are spread out enough to compute slope
                for (auto j = nleft; j <= nright; j++) {
                    w[j - 1] *= (T) (b * (((T) j) - a) + 1.0);
                }
            }
        }

        *ys = 0.0;
        for (auto j = nleft; j <= nright; j++) {
            *ys += w[j - 1] * y[j - 1];
        }

        return true;
    }
}

template<typename T>
void ess(const std::vector<T>& y, size_t n, size_t len, int ideg, size_t njump, bool userw, const std::vector<T>& rw, T* ys, std::vector<T>& res) {
    if (n < 2) {
        ys[0] = y[0];
        return;
    }

    size_t nleft = 0;
    size_t nright = 0;

    auto newnj = std::min(njump, n - 1);
    if (len >= n) {
        nleft = 1;
        nright = n;
        for (size_t i = 1; i <= n; i += newnj) {
            auto ok = est(y, n, len, ideg, (T) i, &ys[i - 1], nleft, nright, res, userw, rw);
            if (!ok) {
                ys[i - 1] = y[i - 1];
            }
        }
    } else if (newnj == 1) { // newnj equal to one, len less than n
        auto nsh = (len + 1) / 2;
        nleft = 1;
        nright = len;
        for (size_t i = 1; i <= n; i++) { // fitted value at i
            if (i > nsh && nright != n) {
                nleft += 1;
                nright += 1;
            }
            auto ok = est(y, n, len, ideg, (T) i, &ys[i - 1], nleft, nright, res, userw, rw);
            if (!ok) {
                ys[i - 1] = y[i - 1];
            }
        }
    } else { // newnj greater than one, len less than n
        auto nsh = (len + 1) / 2;
        for (size_t i = 1; i <= n; i += newnj) { // fitted value at i
            if (i < nsh) {
                nleft = 1;
                nright = len;
            } else if (i >= n - nsh + 1) {
                nleft = n - len + 1;
                nright = n;
            } else {
                nleft = i - nsh + 1;
                nright = len + i - nsh;
            }
            auto ok = est(y, n, len, ideg, (T) i, &ys[i - 1], nleft, nright, res, userw, rw);
            if (!ok) {
                ys[i - 1] = y[i - 1];
            }
        }
    }

    if (newnj != 1) {
        for (size_t i = 1; i <= n - newnj; i += newnj) {
            auto delta = (ys[i + newnj - 1] - ys[i - 1]) / ((T) newnj);
            for (auto j = i + 1; j <= i + newnj - 1; j++) {
                ys[j - 1] = ys[i - 1] + delta * ((T) (j - i));
            }
        }
        auto k = ((n - 1) / newnj) * newnj + 1;
        if (k != n) {
            auto ok = est(y, n, len, ideg, (T) n, &ys[n - 1], nleft, nright, res, userw, rw);
            if (!ok) {
                ys[n - 1] = y[n - 1];
            }
            if (k != n - 1) {
                auto delta = (ys[n - 1] - ys[k - 1]) / ((T) (n - k));
                for (auto j = k + 1; j <= n - 1; j++) {
                    ys[j - 1] = ys[k - 1] + delta * ((T) (j - k));
                }
            }
        }
    }
}

template<typename T>
void ma(const std::vector<T>& x, size_t n, size_t len, std::vector<T>& ave) {
    auto newn = n - len + 1;
    double flen = (T) len;
    double v = 0.0;

    // get the first average
    for (size_t i = 0; i < len; i++) {
        v += x[i];
    }

    ave[0] = (T) (v / flen);
    if (newn > 1) {
        size_t k = len;
        size_t m = 0;
        for (size_t j = 1; j < newn; j++) {
            // window down the array
            v = v - x[m] + x[k];
            ave[j] = (T) (v / flen);
            k += 1;
            m += 1;
        }
    }
}

template<typename T>
void fts(const std::vector<T>& x, size_t n, size_t np, std::vector<T>& trend, std::vector<T>& work) {
    ma(x, n, np, trend);
    ma(trend, n - np + 1, np, work);
    ma(work, n - 2 * np + 2, 3, trend);
}

template<typename T>
void rwts(const T* y, size_t n, const std::vector<T>& fit, std::vector<T>& rw) {
    for (size_t i = 0; i < n; i++) {
        rw[i] = std::abs(y[i] - fit[i]);
    }

    auto mid1 = (n - 1) / 2;
    auto mid2 = n / 2;

    // sort
    std::sort(rw.begin(), rw.begin() + n);

    auto cmad = 3.0 * (rw[mid1] + rw[mid2]); // 6 * median abs resid
    auto c9 = 0.999 * cmad;
    auto c1 = 0.001 * cmad;

    for (size_t i = 0; i < n; i++) {
        auto r = std::abs(y[i] - fit[i]);
        if (r <= c1) {
            rw[i] = 1.0;
        } else if (r <= c9) {
            rw[i] = (T) std::pow(1.0 - std::pow(r / cmad, 2), 2);
        } else {
            rw[i] = 0.0;
        }
    }
}

template<typename T>
void ss(const std::vector<T>& y, size_t n, size_t np, size_t ns, int isdeg, size_t nsjump, bool userw, std::vector<T>& rw, std::vector<T>& season, std::vector<T>& work1, std::vector<T>& work2, std::vector<T>& work3, std::vector<T>& work4) {
    for (size_t j = 1; j <= np; j++) {
        size_t k = (n - j) / np + 1;

        for (size_t i = 1; i <= k; i++) {
            work1[i - 1] = y[(i - 1) * np + j - 1];
        }
        if (userw) {
            for (size_t i = 1; i <= k; i++) {
                work3[i - 1] = rw[(i - 1) * np + j - 1];
            }
        }
        ess(work1, k, ns, isdeg, nsjump, userw, work3, work2.data() + 1, work4);
        T xs = 0.0;
        auto nright = std::min(ns, k);
        auto ok = est(work1, k, ns, isdeg, xs, &work2[0], 1, nright, work4, userw, work3);
        if (!ok) {
            work2[0] = work2[1];
        }
        xs = k + 1;
        size_t nleft = static_cast<size_t>(std::max(1, static_cast<int>(k) - static_cast<int>(ns) + 1));
        ok = est(work1, k, ns, isdeg, xs, &work2[k + 1], nleft, k, work4, userw, work3);
        if (!ok) {
            work2[k + 1] = work2[k];
        }
        for (size_t m = 1; m <= k + 2; m++) {
            season[(m - 1) * np + j - 1] = work2[m - 1];
        }
    }
}

template<typename T>
void onestp(const T* y, size_t n, size_t np, size_t ns, size_t nt, size_t nl, int isdeg, int itdeg, int ildeg, size_t nsjump, size_t ntjump, size_t nljump, size_t ni, bool userw, std::vector<T>& rw, std::vector<T>& season, std::vector<T>& trend, std::vector<T>& work1, std::vector<T>& work2, std::vector<T>& work3, std::vector<T>& work4, std::vector<T>& work5) {
    for (size_t j = 0; j < ni; j++) {
        for (size_t i = 0; i < n; i++) {
            work1[i] = y[i] - trend[i];
        }

        ss(work1, n, np, ns, isdeg, nsjump, userw, rw, work2, work3, work4, work5, season);
        fts(work2, n + 2 * np, np, work3, work1);
        ess(work3, n, nl, ildeg, nljump, false, work4, work1.data(), work5);
        for (size_t i = 0; i < n; i++) {
            season[i] = work2[np + i] - work1[i];
        }
        for (size_t i = 0; i < n; i++) {
            work1[i] = y[i] - season[i];
        }
        ess(work1, n, nt, itdeg, ntjump, userw, rw, trend.data(), work3);
    }
}

template<typename T>
void stl(const T* y, size_t n, size_t np, size_t ns, size_t nt, size_t nl, int isdeg, int itdeg, int ildeg, size_t nsjump, size_t ntjump, size_t nljump, size_t ni, size_t no, std::vector<T>& rw, std::vector<T>& season, std::vector<T>& trend) {
    if (ns < 3) {
        throw std::invalid_argument("seasonal_length must be at least 3");
    }
    if (nt < 3) {
        throw std::invalid_argument("trend_length must be at least 3");
    }
    if (nl < 3) {
        throw std::invalid_argument("low_pass_length must be at least 3");
    }
    if (np < 2) {
        throw std::invalid_argument("period must be at least 2");
    }

    if (isdeg != 0 && isdeg != 1) {
        throw std::invalid_argument("seasonal_degree must be 0 or 1");
    }
    if (itdeg != 0 && itdeg != 1) {
        throw std::invalid_argument("trend_degree must be 0 or 1");
    }
    if (ildeg != 0 && ildeg != 1) {
        throw std::invalid_argument("low_pass_degree must be 0 or 1");
    }

    if (ns % 2 != 1) {
        throw std::invalid_argument("seasonal_length must be odd");
    }
    if (nt % 2 != 1) {
        throw std::invalid_argument("trend_length must be odd");
    }
    if (nl % 2 != 1) {
        throw std::invalid_argument("low_pass_length must be odd");
    }

    auto work1 = std::vector<T>(n + 2 * np);
    auto work2 = std::vector<T>(n + 2 * np);
    auto work3 = std::vector<T>(n + 2 * np);
    auto work4 = std::vector<T>(n + 2 * np);
    auto work5 = std::vector<T>(n + 2 * np);

    auto userw = false;
    size_t k = 0;

    while (true) {
        onestp(y, n, np, ns, nt, nl, isdeg, itdeg, ildeg, nsjump, ntjump, nljump, ni, userw, rw, season, trend, work1, work2, work3, work4, work5);
        k += 1;
        if (k > no) {
            break;
        }
        for (size_t i = 0; i < n; i++) {
            work1[i] = trend[i] + season[i];
        }
        rwts(y, n, work1, rw);
        userw = true;
    }

    if (no <= 0) {
        for (size_t i = 0; i < n; i++) {
            rw[i] = 1.0;
        }
    }
}

template<typename T>
double var(const std::vector<T>& series) {
    auto mean = std::accumulate(series.begin(), series.end(), 0.0) / series.size();
    double sum = 0.0;
    for (auto v : series) {
        double diff = v - mean;
        sum += diff * diff;
    }
    return sum / (series.size() - 1);
}

template<typename T>
double strength(const std::vector<T>& component, const std::vector<T>& remainder) {
    std::vector<T> sr;
    sr.reserve(remainder.size());
    for (size_t i = 0; i < remainder.size(); i++) {
        sr.push_back(component[i] + remainder[i]);
    }
    return std::max(0.0, 1.0 - var(remainder) / var(sr));
}

} // namespace detail

/// A STL result.
template<typename T = float>
class StlResult {
  public:
    /// Returns the seasonal component.
    std::vector<T> seasonal;

    /// Returns the trend component.
    std::vector<T> trend;

    /// Returns the remainder.
    std::vector<T> remainder;

    /// Returns the weights.
    std::vector<T> weights;

    /// Returns the seasonal strength.
    inline double seasonal_strength() const {
        return detail::strength(seasonal, remainder);
    }

    /// Returns the trend strength.
    inline double trend_strength() const {
        return detail::strength(trend, remainder);
    }
};

/// A set of STL parameters.
class StlParams {
  public:
    /// @private
    std::optional<size_t> ns_ = std::nullopt;

  private:
    std::optional<size_t> nt_ = std::nullopt;
    std::optional<size_t> nl_ = std::nullopt;
    int isdeg_ = 0;
    int itdeg_ = 1;
    std::optional<int> ildeg_ = std::nullopt;
    std::optional<size_t> nsjump_ = std::nullopt;
    std::optional<size_t> ntjump_ = std::nullopt;
    std::optional<size_t> nljump_ = std::nullopt;
    std::optional<size_t> ni_ = std::nullopt;
    std::optional<size_t> no_ = std::nullopt;
    bool robust_ = false;

  public:
    /// Sets the length of the seasonal smoother.
    inline StlParams seasonal_length(size_t length) {
        this->ns_ = length;
        return *this;
    }

    /// Sets the length of the trend smoother.
    inline StlParams trend_length(size_t length) {
        this->nt_ = length;
        return *this;
    }

    /// Sets the length of the low-pass filter.
    inline StlParams low_pass_length(size_t length) {
        this->nl_ = length;
        return *this;
    }

    /// Sets the degree of locally-fitted polynomial in seasonal smoothing.
    inline StlParams seasonal_degree(int degree) {
        this->isdeg_ = degree;
        return *this;
    }

    /// Sets the degree of locally-fitted polynomial in trend smoothing.
    inline StlParams trend_degree(int degree) {
        this->itdeg_ = degree;
        return *this;
    }

    /// Sets the degree of locally-fitted polynomial in low-pass smoothing.
    inline StlParams low_pass_degree(int degree) {
        this->ildeg_ = degree;
        return *this;
    }

    /// Sets the skipping value for seasonal smoothing.
    inline StlParams seasonal_jump(size_t jump) {
        this->nsjump_ = jump;
        return *this;
    }

    /// Sets the skipping value for trend smoothing.
    inline StlParams trend_jump(size_t jump) {
        this->ntjump_ = jump;
        return *this;
    }

    /// Sets the skipping value for low-pass smoothing.
    inline StlParams low_pass_jump(size_t jump) {
        this->nljump_ = jump;
        return *this;
    }

    /// Sets the number of loops for updating the seasonal and trend components.
    inline StlParams inner_loops(size_t loops) {
        this->ni_ = loops;
        return *this;
    }

    /// Sets the number of iterations of robust fitting.
    inline StlParams outer_loops(size_t loops) {
        this->no_ = loops;
        return *this;
    }

    /// Sets whether robustness iterations are to be used.
    inline StlParams robust(bool robust) {
        this->robust_ = robust;
        return *this;
    }

    /// Decomposes a time series from an array.
    template<typename T>
    StlResult<T> fit(const T* series, size_t series_size, size_t period) const;

    /// Decomposes a time series from a vector.
    template<typename T>
    StlResult<T> fit(const std::vector<T>& series, size_t period) const;

#if __cplusplus >= 202002L
    /// Decomposes a time series from a span.
    template<typename T>
    StlResult<T> fit(std::span<const T> series, size_t period) const;
#endif
};

/// Creates a new set of STL parameters.
inline StlParams params() {
    return StlParams();
}

template<typename T>
StlResult<T> StlParams::fit(const T* series, size_t series_size, size_t period) const {
    auto y = series;
    auto np = period;
    auto n = series_size;

    if (n < 2 * np) {
        throw std::invalid_argument("series has less than two periods");
    }

    auto ns = this->ns_.value_or(np);

    auto isdeg = this->isdeg_;
    auto itdeg = this->itdeg_;

    auto res = StlResult<T> {
        std::vector<T>(n),
        std::vector<T>(n),
        std::vector<T>(),
        std::vector<T>(n)
    };

    auto ildeg = this->ildeg_.value_or(itdeg);
    auto newns = std::max(ns, static_cast<size_t>(3));
    if (newns % 2 == 0) {
        newns += 1;
    }

    auto newnp = std::max(np, static_cast<size_t>(2));
    auto nt = static_cast<size_t>(std::ceil((1.5 * newnp) / (1.0 - 1.5 / static_cast<float>(newns))));
    nt = this->nt_.value_or(nt);
    nt = std::max(nt, static_cast<size_t>(3));
    if (nt % 2 == 0) {
        nt += 1;
    }

    auto nl = this->nl_.value_or(newnp);
    if (nl % 2 == 0 && !this->nl_.has_value()) {
        nl += 1;
    }

    auto ni = this->ni_.value_or(this->robust_ ? 1 : 2);
    auto no = this->no_.value_or(this->robust_ ? 15 : 0);

    auto nsjump = this->nsjump_.value_or(static_cast<size_t>(std::ceil(static_cast<float>(newns) / 10.0)));
    auto ntjump = this->ntjump_.value_or(static_cast<size_t>(std::ceil(static_cast<float>(nt) / 10.0)));
    auto nljump = this->nljump_.value_or(static_cast<size_t>(std::ceil(static_cast<float>(nl) / 10.0)));

    detail::stl(y, n, newnp, newns, nt, nl, isdeg, itdeg, ildeg, nsjump, ntjump, nljump, ni, no, res.weights, res.seasonal, res.trend);

    res.remainder.reserve(n);
    for (size_t i = 0; i < n; i++) {
        res.remainder.push_back(y[i] - res.seasonal[i] - res.trend[i]);
    }

    return res;
}

template<typename T>
StlResult<T> StlParams::fit(const std::vector<T>& series, size_t period) const {
    return StlParams::fit(series.data(), series.size(), period);
}

#if __cplusplus >= 202002L
template<typename T>
StlResult<T> StlParams::fit(std::span<const T> series, size_t period) const {
    return StlParams::fit(series.data(), series.size(), period);
}
#endif

/// A MSTL result.
template<typename T = float>
class MstlResult {
  public:
    /// Returns the seasonal component.
    std::vector<std::vector<T>> seasonal;

    /// Returns the trend component.
    std::vector<T> trend;

    /// Returns the remainder.
    std::vector<T> remainder;

    /// Returns the seasonal strength.
    inline std::vector<double> seasonal_strength() const {
        std::vector<double> res;
        for (auto& s : seasonal) {
            res.push_back(detail::strength(s, remainder));
        }
        return res;
    }

    /// Returns the trend strength.
    inline double trend_strength() const {
        return detail::strength(trend, remainder);
    }
};

/// A set of MSTL parameters.
class MstlParams {
    size_t iterate_ = 2;
    std::optional<float> lambda_ = std::nullopt;
    std::optional<std::vector<size_t>> swin_ = std::nullopt;
    StlParams stl_params_;

  public:
    /// Sets the number of iterations.
    inline MstlParams iterations(size_t iterations) {
        this->iterate_ = iterations;
        return *this;
    }

    /// Sets lambda for Box-Cox transformation.
    inline MstlParams lambda(float lambda) {
        this->lambda_ = lambda;
        return *this;
    }

    /// Sets the lengths of the seasonal smoothers.
    inline MstlParams seasonal_lengths(const std::vector<size_t>& lengths) {
        this->swin_ = lengths;
        return *this;
    }

    /// Sets the STL parameters.
    inline MstlParams stl_params(const StlParams& stl_params) {
        this->stl_params_ = stl_params;
        return *this;
    }

    /// Decomposes a time series from an array.
    template<typename T>
    MstlResult<T> fit(const T* series, size_t series_size, const size_t* periods, size_t periods_size) const;

    /// Decomposes a time series from a vector.
    template<typename T>
    MstlResult<T> fit(const std::vector<T>& series, const std::vector<size_t>& periods) const;

#if __cplusplus >= 202002L
    /// Decomposes a time series from a span.
    template<typename T>
    MstlResult<T> fit(std::span<const T> series, std::span<const size_t> periods) const;
#endif
};

/// Creates a new set of MSTL parameters.
inline MstlParams mstl_params() {
    return MstlParams();
}

namespace detail {

template<typename T>
std::vector<T> box_cox(const T* y, size_t y_size, float lambda) {
    std::vector<T> res;
    res.reserve(y_size);
    if (lambda != 0.0) {
        for (size_t i = 0; i < y_size; i++) {
            res.push_back((T) (std::pow(y[i], lambda) - 1.0) / lambda);
        }
    } else {
        for (size_t i = 0; i < y_size; i++) {
            res.push_back(std::log(y[i]));
        }
    }
    return res;
}

template<typename T>
std::tuple<std::vector<T>, std::vector<T>, std::vector<std::vector<T>>> mstl(
    const T* x,
    size_t k,
    const size_t* seas_ids,
    size_t seas_size,
    size_t iterate,
    std::optional<float> lambda,
    const std::optional<std::vector<size_t>>& swin,
    const StlParams& stl_params
) {
    // keep track of indices instead of sorting seas_ids
    // so order is preserved with seasonality
    std::vector<size_t> indices;
    for (size_t i = 0; i < seas_size; i++) {
        indices.push_back(i);
    }
    std::sort(indices.begin(), indices.end(), [&seas_ids](size_t a, size_t b) {
        return seas_ids[a] < seas_ids[b];
    });

    if (seas_size == 1) {
        iterate = 1;
    }

    std::vector<std::vector<T>> seasonality;
    seasonality.reserve(seas_size);
    std::vector<T> trend;

    auto deseas = lambda.has_value() ? box_cox(x, k, lambda.value()) : std::vector<T>(x, x + k);

    if (seas_size != 0) {
        for (size_t i = 0; i < seas_size; i++) {
            seasonality.push_back(std::vector<T>());
        }

        for (size_t j = 0; j < iterate; j++) {
            for (size_t i = 0; i < indices.size(); i++) {
                auto idx = indices[i];

                if (j > 0) {
                    for (size_t ii = 0; ii < deseas.size(); ii++) {
                        deseas[ii] += seasonality[idx][ii];
                    }
                }

                StlResult<T> fit;
                if (swin) {
                    StlParams clone = stl_params;
                    fit = clone.seasonal_length((*swin)[idx]).fit(deseas, seas_ids[idx]);
                } else if (stl_params.ns_.has_value()) {
                    fit = stl_params.fit(deseas, seas_ids[idx]);
                } else {
                    StlParams clone = stl_params;
                    fit = clone.seasonal_length(7 + 4 * (i + 1)).fit(deseas, seas_ids[idx]);
                }

                seasonality[idx] = fit.seasonal;
                trend = fit.trend;

                for (size_t ii = 0; ii < deseas.size(); ii++) {
                    deseas[ii] -= seasonality[idx][ii];
                }
            }
        }
    } else {
        // TODO use Friedman's Super Smoother for trend
        throw std::invalid_argument("periods must not be empty");
    }

    std::vector<T> remainder;
    remainder.reserve(k);
    for (size_t i = 0; i < k; i++) {
        remainder.push_back(deseas[i] - trend[i]);
    }

    return std::make_tuple(trend, remainder, seasonality);
}

} // namespace detail

template<typename T>
MstlResult<T> MstlParams::fit(const T* series, size_t series_size, const size_t* periods, size_t periods_size) const {
    // return error to be consistent with stl
    // and ensure seasonal is always same length as periods
    for (size_t i = 0; i < periods_size; i++) {
        if (periods[i] < 2) {
            throw std::invalid_argument("periods must be at least 2");
        }
    }

    // return error to be consistent with stl
    // and ensure seasonal is always same length as periods
    for (size_t i = 0; i < periods_size; i++) {
        if (series_size < periods[i] * 2) {
            throw std::invalid_argument("series has less than two periods");
        }
    }

    if (lambda_.has_value()) {
        auto lambda = lambda_.value();
        if (lambda < 0 || lambda > 1) {
            throw std::invalid_argument("lambda must be between 0 and 1");
        }
    }

    if (swin_.has_value()) {
        auto swin = swin_.value();
        if (swin.size() != periods_size) {
            throw std::invalid_argument("seasonal_lengths must have the same length as periods");
        }
    }

    auto [trend, remainder, seasonal] = detail::mstl(
        series,
        series_size,
        periods,
        periods_size,
        iterate_,
        lambda_,
        swin_,
        stl_params_
    );

    return MstlResult<T> {
        seasonal,
        trend,
        remainder
    };
}

template<typename T>
MstlResult<T> MstlParams::fit(const std::vector<T>& series, const std::vector<size_t>& periods) const {
    return MstlParams::fit(series.data(), series.size(), periods.data(), periods.size());
}

#if __cplusplus >= 202002L
template<typename T>
MstlResult<T> MstlParams::fit(std::span<const T> series, std::span<const size_t> periods) const {
    return MstlParams::fit(series.data(), series.size(), periods.data(), periods.size());
}
#endif

} // namespace stl
