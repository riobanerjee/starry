/**
Return a human-readable map string.

*/
template <class S>
std::string Map<S>::info () {
    std::ostringstream os;
    os << "<starry.Map("
        << "lmax=" << lmax << ", "
        << "ncol=" << ncol
        << ")>";
    return std::string(os.str());
}

/**
Set the full spherical harmonic vector/matrix.

*/
template <class S>
inline void Map<S>::setY (
    const MapType& y_
) {
    cache.yChanged();
    if ((y_.rows() == y.rows()) && (y_.cols() == y.cols()))
        y = y_;
    else
        throw errors::ValueError("Dimension mismatch in `y`.");
}

/**
Set the (l, m) row of the spherical harmonic coefficient *matrix* to an
array of coefficients.

*/
template <class S>
inline void Map<S>::setY (
    int l, 
    int m, 
    const Ref<const CoeffType>& coeff
) {
    cache.yChanged();
    if ((0 <= l) && (l <= lmax) && (-l <= m) && (m <= l))
        y.row(l * l + l + m) = coeff;
    else
        throw errors::IndexError("Invalid value for `l` and/or `m`.");
}

/**
Set the (l, m) index of the spherical harmonic coefficient vector, or
the entire (l, m) row of the spherical harmonic coefficient matrix, to a
single value.

*/
template <class S>
inline void Map<S>::setY (
    int l, 
    int m, 
    const Scalar& coeff
) {
    cache.yChanged();
    if ((0 <= l) && (l <= lmax) && (-l <= m) && (m <= l))
        y.row(l * l + l + m).setConstant(coeff);
    else
        throw errors::IndexError("Invalid value for `l` and/or `m`.");
}

/**
Get the full spherical harmonic vector/matrix

*/
template <class S>
inline typename S::MapType Map<S>::getY () const {
    return y;
}

/**
Set the full limb darkening vector/matrix.

*/
template <class S>
inline void Map<S>::setU (
    const MapType& u_
) 
{
    cache.uChanged();
    if ((u_.rows() == u.rows() - 1) && (u_.cols() == u.cols()))
        u.block(1, 0, lmax, ncol) = u_;
    else
        throw errors::ValueError("Dimension mismatch in `u`.");
}

/**
Set the `l`th index of the limb darkening coefficient *matrix* to an
array of coefficients.

*/
template <class S>
inline void Map<S>::setU (
    int l, 
    const Ref<const CoeffType>& coeff
) {
    cache.uChanged();
    if ((1 <= l) && (l <= lmax))
        u.row(l) = coeff;
    else
        throw errors::IndexError("Invalid value for `l`.");
}

/**
Set the `l`th index of the limb darkening coefficient vector, or
the entire `l`th row of the limb darkening coefficient matrix, to a
single value.

*/
template <class S>
inline void Map<S>::setU (
    int l, 
    const Scalar& coeff
) {
    cache.uChanged();
    if ((1 <= l) && (l <= lmax))
        u.row(l).setConstant(coeff);
    else
        throw errors::IndexError("Invalid value for `l`.");
}

/**
Get the full limb darkening vector/matrix.

*/
template <class S>
inline typename S::MapType Map<S>::getU ()
const {
    return u.block(1, 0, lmax, ncol);
}

/**
Set the axis of rotation for the map.

*/
template <class S>
inline void Map<S>::setAxis (
    const UnitVector<Scalar>& axis_
) {
    cache.axisChanged();
    axis(0) = axis_(0);
    axis(1) = axis_(1);
    axis(2) = axis_(2);
    axis = axis / sqrt(axis(0) * axis(0) +
                       axis(1) * axis(1) +
                       axis(2) * axis(2));
}

/**
Return a copy of the axis.

*/
template <class S>
UnitVector<typename S::Scalar> Map<S>::getAxis () const {
    return axis;
}