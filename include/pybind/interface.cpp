// Enable debug mode?
#ifdef STARRY_DEBUG
#undef NDEBUG
#endif

// Enable the Python interface
#ifndef STARRY_ENABLE_PYTHON_INTERFACE
#define STARRY_ENABLE_PYTHON_INTERFACE
#endif

// Select which module to build
#if defined(_STARRY_DEFAULT_DOUBLE_)
#define _STARRY_NAME_ _starry_default_double
#define _STARRY_DEFAULT_
#define _STARRY_DOUBLE_
#define _STARRY_STATIC_
#define _STARRY_SINGLECOL_
#define _STARRY_TYPE_ Default<double>

#elif defined(_STARRY_DEFAULT_MULTI_)
#define _STARRY_NAME_ _starry_default_multi
#define _STARRY_DEFAULT_
#define _STARRY_MULTI_
#define _STARRY_STATIC_
#define _STARRY_SINGLECOL_
#define STARRY_ENABLE_BOOST
#define _STARRY_TYPE_ Default<Multi>

#elif defined(_STARRY_SPECTRAL_DOUBLE_)
#define _STARRY_NAME_ _starry_spectral_double
#define _STARRY_SPECTRAL_
#define _STARRY_DOUBLE_
#define _STARRY_STATIC_
#define _STARRY_MULTI_COL
#define _STARRY_TYPE_ Spectral<double>

#elif defined(_STARRY_SPECTRAL_MULTI_)
#define _STARRY_NAME_ _starry_spectral_multi
#define _STARRY_SPECTRAL_
#define _STARRY_MULTI_
#define _STARRY_STATIC_
#define _STARRY_MULTI_COL
#define STARRY_ENABLE_BOOST
#define _STARRY_TYPE_ Spectral<Multi>

#elif defined(_STARRY_TEMPORAL_DOUBLE_)
#define _STARRY_NAME_ _starry_temporal_double
#define _STARRY_TEMPORAL_
#define _STARRY_DOUBLE_
#define _STARRY_MULTI_COL
#define _STARRY_TYPE_ Temporal<double>

#elif defined(_STARRY_TEMPORAL_MULTI_)
#define _STARRY_NAME_ _starry_temporal_multi
#define _STARRY_TEMPORAL_
#define _STARRY_MULTI_
#define _STARRY_MULTI_COL
#define STARRY_ENABLE_BOOST
#define _STARRY_TYPE_ Temporal<Multi>

#else
static_assert(false, "Invalid or missing STARRY module type.");
#endif

//! Includes
#include <pybind11/embed.h>
#include "interface.h"
using namespace interface;

//! Register the Python module
PYBIND11_MODULE(
    _STARRY_NAME_, 
    m
) {

    // Current Map Type
    using T = _STARRY_TYPE_;

    // Declare the Map class
    py::class_<Map<T>> PyMap(m, "Map");

#if defined(_STARRY_SINGLECOL_) 
    // Constructor for vector maps
    PyMap.def(py::init<int>(), "lmax"_a=2);
#else
    // Constructor for matrix maps
    PyMap.def(py::init<int, int>(), "lmax"_a=2, "ncol"_a=1);
#endif

    // String representation of the map
    PyMap.def("__repr__", &Map<T>::info);

    // Number of Ylm map columns
    PyMap.def_property_readonly(
        "ncoly", [] (
            Map<T> &map
        ) {
            return map.ncoly;
    });

    // Number of Ul map columns
    PyMap.def_property_readonly(
        "ncolu", [] (
            Map<T> &map
        ) {
            return map.ncolu;
    });

    // Highest degree of the map
    PyMap.def_property_readonly(
        "lmax", [] (
            Map<T> &map
        ) {
            return map.lmax;
    });

    // Number of spherical harmonic coefficients
    PyMap.def_property_readonly(
        "N", [] (
            Map<T> &map
        ) {
            return map.N;
    });

    // Set one or more spherical harmonic coefficients to the same scalar value
    PyMap.def(
        "__setitem__", [](
            Map<T>& map, 
            py::tuple lm,
            const double& coeff
        ) {
            auto inds = get_Ylm_inds(map.lmax, lm);
            auto y = map.getY();
            for (auto n : inds)
                y.row(n).setConstant(static_cast<typename T::Scalar>(coeff));
            map.setY(y);
    });

    // Set one or more spherical harmonic coefficients to the same vector value
    PyMap.def(
        "__setitem__", [](
            Map<T>& map, 
            py::tuple lm,
            const typename T::Double::YCoeffType& coeff
        ) {
            auto inds = get_Ylm_inds(map.lmax, lm);
            auto y = map.getY();
            for (auto n : inds)
                y.row(n) = coeff.template cast<typename T::Scalar>();
            map.setY(y);
    });

    // Set multiple spherical harmonic coefficients at once
    PyMap.def(
        "__setitem__", [](
            Map<T>& map, 
            py::tuple lm,
            const typename T::Double::YType& coeff_
        ) {
            auto inds = get_Ylm_inds(map.lmax, lm);
            typename T::YType coeff = coeff_.template cast<typename T::Scalar>();
            if (coeff.rows() != static_cast<long>(inds.size()))
                throw errors::ValueError("Mismatch in slice length and "
                                         "coefficient array size.");
            auto y = map.getY();
            int i = 0;
            for (auto n : inds)
                y.row(n) = coeff.row(i++);
            map.setY(y);
    });

    // Retrieve one or more spherical harmonic coefficients
    PyMap.def(
        "__getitem__", [](
            Map<T>& map,
            py::tuple lm
        ) -> py::object {
            auto inds = get_Ylm_inds(map.lmax, lm);
            auto y = map.getY();
            typename T::Double::YType res;
            res.resize(inds.size(), map.ncoly);
            int i = 0;
            for (auto n : inds)
                res.row(i++) = y.row(n).template cast<double>();
            if (inds.size() == 1) {
                if (T::YType::ColsAtCompileTime == 1)
                    return py::cast<double>(res(0));
                else
                    return py::cast<typename T::Double::YCoeffType>(res.row(0));
            } else {
                return py::cast<typename T::Double::YType>(res);
            }
    });

    // Set one or more limb darkening coefficients to the same scalar value
    PyMap.def(
        "__setitem__", [](
            Map<T>& map, 
            py::object l,
            const double& coeff
        ) {
            auto inds = get_Ul_inds(map.lmax, l);
            auto u = map.getU();
            for (auto n : inds)
                u.row(n - 1).setConstant(static_cast<typename T::Scalar>(coeff));
            map.setU(u);
    });

    // Set one or more limb darkening coefficients to the same vector value
    PyMap.def(
        "__setitem__", [](
            Map<T>& map, 
            py::object l,
            const typename T::Double::UCoeffType& coeff
        ) {
            auto inds = get_Ul_inds(map.lmax, l);
            auto u = map.getU();
            for (auto n : inds)
                u.row(n - 1) = coeff.template cast<typename T::Scalar>();
            map.setU(u);
    });

    // Set multiple limb darkening coefficients at once
    PyMap.def(
        "__setitem__", [](
            Map<T>& map, 
            py::object l,
            const typename T::Double::UType& coeff_
        ) {
            auto inds = get_Ul_inds(map.lmax, l);
            typename T::UType coeff = coeff_.template cast<typename T::Scalar>();
            if (coeff.rows() != static_cast<long>(inds.size()))
                throw errors::ValueError("Mismatch in slice length and "
                                         "coefficient array size.");
            auto u = map.getU();
            int i = 0;
            for (auto n : inds)
                u.row(n - 1) = coeff.row(i++);
            map.setU(u);
    });

    // Retrieve one or more limb darkening coefficients
    PyMap.def(
        "__getitem__", [](
            Map<T>& map,
            py::object l
        ) -> py::object {
            auto inds = get_Ul_inds(map.lmax, l);
            auto u = map.getU();
            typename T::Double::UType res;
            res.resize(inds.size(), map.ncolu);
            int i = 0;
            for (auto n : inds)
                res.row(i++) = u.row(n - 1).template cast<double>();
            if (inds.size() == 1) {
                if (T::UType::ColsAtCompileTime == 1)
                    return py::cast<double>(res(0));
                else
                    return py::cast<typename T::Double::UCoeffType>(res.row(0));
            } else {
                return py::cast<typename T::Double::UType>(res);
            }
    });

    // Reset the map
    PyMap.def("reset", &Map<T>::reset);
    
    // Vector of spherical harmonic coefficients
    PyMap.def_property_readonly(
        "y", [] (
            Map<T> &map
        ) -> typename T::Double::YType {
            return map.getY().template cast<double>();
    });

    // Vector of limb darkening coefficients
    PyMap.def_property_readonly(
        "u", [] (
            Map<T> &map
        ) -> typename T::Double::UType {
            return map.getU().template cast<double>();
    });

    // Get/set the rotation axis
    PyMap.def_property(
        "axis", [] (
            Map<T> &map
        ) -> UnitVector<double> {
            return map.getAxis().template cast<double>();
        }, [] (
            Map<T> &map, 
            UnitVector<double>& axis
        ) {
            map.setAxis(axis.template cast<typename T::Scalar>());
    });

    // Rotate the base map
    PyMap.def("rotate", &Map<T>::rotate, "theta"_a=0.0);

#if defined(_STARRY_SINGLECOL_)
    // Add a gaussian spot with a scalar amplitude
    PyMap.def(
        "add_spot", [](
            Map<T>& map,
            const double& amp,
            const double& sigma,
            const double& lat,
            const double& lon,
            const int l
        ) {
            typename T::YCoeffType amp_;
            amp_(0) = amp;
            map.addSpot(amp_, 
                        sigma, lat, lon, l);
        }, 
        "amp"_a, "sigma"_a=0.1, "lat"_a=0.0, "lon"_a=0.0, "l"_a=-1);
#else
    // Add a gaussian spot with a vector amplitude
    PyMap.def(
        "add_spot", [](
            Map<T>& map,
            const typename T::Double::YCoeffType& amp,
            const double& sigma,
            const double& lat,
            const double& lon,
            const int l
        ) {
            map.addSpot(amp.template cast<typename T::Scalar>(), 
                        sigma, lat, lon, l);
        }, 
        "amp"_a, "sigma"_a=0.1, "lat"_a=0.0, "lon"_a=0.0, "l"_a=-1);
#endif

#if defined(_STARRY_SINGLECOL_)
    // Generate a random map
    PyMap.def(
        "random", [](
            Map<T>& map,
            const Vector<double>& power,
            py::object seed_
        ) {
            if (seed_.is(py::none())) {
                // TODO: We need a better, more thread-safe randomizer seed
                auto seed = std::chrono::system_clock::now()
                            .time_since_epoch().count();
                map.random(power.template cast<typename T::Scalar>(), seed);
            } else {
                double seed = py::cast<double>(seed_);
                map.random(power.template cast<typename T::Scalar>(), seed);
            }
        }, 
        "power"_a, "seed"_a=py::none());
#else
    // Generate a random map
    PyMap.def(
        "random", [](
            Map<T>& map,
            const Vector<double>& power,
            py::object seed_,
            int col
        ) {
            if (seed_.is(py::none())) {
                // TODO: We need a better, more thread-safe randomizer seed
                auto seed = std::chrono::system_clock::now().time_since_epoch().count();
                map.random(power.template cast<typename T::Scalar>(), seed, col);
            } else {
                double seed = py::cast<double>(seed_);
                map.random(power.template cast<typename T::Scalar>(), seed, col);
            }
        }, 
        "power"_a, "seed"_a=py::none(), "col"_a=-1);
#endif

#if defined(_STARRY_STATIC_)
    // Show an image/animation of the map
    PyMap.def(
        "show", [](
            Map<T>& map,
            py::array_t<double> theta_,
            std::string cmap,
            int res,
            int interval,
            std::string gif
        ) -> py::object {
            auto theta = py::cast<Vector<double>>(theta_);
            if (theta.size() == 0) {
                return map.show(0.0, cmap, res);
            } else if (theta.size() == 1) {
                return map.show(theta(0), cmap, res);
            } else {
                return map.show(theta.template cast<typename T::Scalar>(), 
                                cmap, res, interval, gif);
            }
        }, 
        "theta"_a=py::array_t<double>(), "cmap"_a="plasma", "res"_a=300,
        "interval"_a=75, "gif"_a="");
#else
    // Show an image/animation of the map
    PyMap.def(
        "show", [](
            Map<T>& map,
            py::array_t<double> t_,
            py::array_t<double> theta_,
            std::string cmap,
            int res,
            int interval,
            std::string gif
        ) -> py::object {
            auto atleast_1d = py::module::import("numpy").attr("atleast_1d");
            Vector<double> t = py::cast<Vector<double>>(atleast_1d(t_));
            Vector<double> theta = py::cast<Vector<double>>(atleast_1d(theta_));
            int sz = max(t.size(), theta.size());
            if ((t.size() == 0) || (theta.size() == 0)) {
                throw errors::ValueError(
                    "Invalid dimensions for `t` and/or `theta`.");
            } else if (t.size() == 1) {
                t.setConstant(sz, t(0));
            } else if (theta.size() == 1) {
                theta.setConstant(sz, theta(0));
            } else if (t.size() != theta.size()){
                throw errors::ValueError(
                    "Invalid dimensions for `t` and/or `theta`.");
            }
            return map.show(t.template cast<typename T::Scalar>(), 
                            theta.template cast<typename T::Scalar>(), 
                            cmap, res, interval, gif);
        }, 
        "t"_a=0.0, "theta"_a=0.0, 
        "cmap"_a="plasma", "res"_a=300, "interval"_a=75, "gif"_a="");
#endif

#if defined(_STARRY_DEFAULT_)
    // Render the visible map on a square grid
    PyMap.def(
        "render", [](
            Map<T>& map,
            double theta,
            int res
        ) -> py::object {
            auto reshape = py::module::import("numpy").attr("reshape");
            Matrix<typename T::Scalar> intensity;
            map.renderMap(theta, res, intensity);
            return reshape(intensity.template cast<double>(), 
                           py::make_tuple(res, res));

        }, 
        "theta"_a=0.0, "res"_a=300);
#elif defined(_STARRY_SPECTRAL_)
    // Render the visible map on a square grid
    PyMap.def(
        "render", [](
            Map<T>& map,
            double theta,
            int res
        ) -> py::object {
            auto reshape = py::module::import("numpy").attr("reshape");
            Matrix<typename T::Scalar> intensity;
            map.renderMap(theta, res, intensity);
            return reshape(intensity.template cast<double>(), 
                           py::make_tuple(res, res, map.nflx));
        }, 
        "theta"_a=0.0, "res"_a=300);
#elif defined(_STARRY_TEMPORAL_)
    // Render the visible map on a square grid
    PyMap.def(
        "render", [](
            Map<T>& map,
            double t,
            double theta,
            int res
        ) -> py::object {
            auto reshape = py::module::import("numpy").attr("reshape");
            Matrix<typename T::Scalar> intensity;
            map.renderMap(t, theta, res, intensity);
            return reshape(intensity.template cast<double>(), 
                           py::make_tuple(res, res));
        }, 
        "t"_a=0.0, "theta"_a=0.0, "res"_a=300);
#endif

#if defined(_STARRY_SINGLECOL_)
    // Load an image from a file
    PyMap.def(
        "load_image", [](
            Map<T>& map,
            std::string image,
            int l,
            bool normalize,
            int sampling_factor
        ) {
            map.loadImage(image, l, normalize, sampling_factor);
        },
        "image"_a, "l"_a=-1, "normalize"_a=true, "sampling_factor"_a=8);
#else
    // Load an image from a file
    PyMap.def(
        "load_image", [](
            Map<T>& map,
            std::string image,
            int l,
            int col,
            bool normalize,
            int sampling_factor
        ) {
            map.loadImage(image, l, col, normalize, sampling_factor);
        },
        "image"_a, "l"_a=-1, "col"_a=-1, "normalize"_a=true, "sampling_factor"_a=8);
#endif

#if defined(_STARRY_STATIC_)
    // Compute the intensity
    PyMap.def("__call__", intensity<T>(), "theta"_a=0.0, 
              "x"_a=0.0, "y"_a=0.0);
#else
    // Compute the intensity
    PyMap.def("__call__", intensity<T>(), "t"_a=0.0, "theta"_a=0.0, 
              "x"_a=0.0, "y"_a=0.0);
#endif

#if defined(_STARRY_STATIC_)
    // Compute the flux
    PyMap.def("flux", flux<T>(), "theta"_a=0.0, "xo"_a=0.0, 
              "yo"_a=0.0, "ro"_a=0.0, "gradient"_a=false);
#else
    // Compute the flux
    PyMap.def("flux", flux<T>(), "t"_a=0.0,"theta"_a=0.0, "xo"_a=0.0, 
              "yo"_a=0.0, "ro"_a=0.0, "gradient"_a=false);
#endif

// Code version
#ifdef VERSION_INFO
    m.attr("__version__") = VERSION_INFO;
#else
    m.attr("__version__") = "dev";
#endif

    // A dictionary of all compiler flags
    PyMap.def_property_readonly(
        "__compile_flags__", [] (
            Map<T> &map
        ) -> py::dict {

            auto flags = py::dict();
            flags["STARRY_NMULTI"] = STARRY_NMULTI;
            flags["STARRY_ELLIP_MAX_ITER"] = STARRY_ELLIP_MAX_ITER;
            flags["STARRY_MAX_LMAX"] = STARRY_MAX_LMAX;
            flags["STARRY_BCUT"] = STARRY_BCUT;
            flags["STARRY_MN_MAX_ITER"] = STARRY_MN_MAX_ITER;

#ifdef STARRY_KEEP_DFDU_AS_DFDG
            flags["STARRY_KEEP_DFDU_AS_DFDG"] = STARRY_KEEP_DFDU_AS_DFDG;
#else
            flags["STARRY_KEEP_DFDU_AS_DFDG"] = 0;
#endif

#ifdef STARRY_O
            flags["STARRY_O"] = STARRY_O;
#else
            flags["STARRY_O"] = py::none();
#endif

#ifdef STARRY_DEBUG
            flags["STARRY_DEBUG"] = STARRY_DEBUG;
#else
            flags["STARRY_DEBUG"] = 0;
#endif

            return flags;

    });

}