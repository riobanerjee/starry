/**
\file wigner.h
\brief Implements the spherical harmonic rotation matrices.

These are adapted from the Fortran code of

    Alvarez Collado  et al. (1989) "Rotation of real spherical harmonics".
    Physics Communications 52, 3.
    https://doi.org/10.1016/0010-4655(89)90107-0

who computed the Euleriean rotation matrices for real spherical harmonics
from the Wigner-D matrices for complex spherical harmonics.

*/

#ifndef _STARRY_WIGNER_H_
#define _STARRY_WIGNER_H_

#include "utils.h"

namespace starry {
namespace wigner {

using namespace starry::utils;

/**
Axis-angle rotation matrix, used to rotate Cartesian
vectors around in 3D space.

*/
template <typename T>
Matrix<T> AxisAngle (
    const UnitVector<T>& u, 
    const T& theta
) {
    Matrix<T> R(3, 3);
    T cost = cos(theta);
    T sint = sin(theta);
    R(0, 0) = cost + u(0) * u(0) * (1 - cost);
    R(0, 1) = u(0) * u(1) * (1 - cost) - u(2) * sint;
    R(0, 2) = u(0) * u(2) * (1 - cost) + u(1) * sint;
    R(1, 0) = u(1) * u(0) * (1 - cost) + u(2) * sint;
    R(1, 1) = cost + u(1) * u(1) * (1 - cost);
    R(1, 2) = u(1) * u(2) * (1 - cost) - u(0) * sint;
    R(2, 0) = u(2) * u(0) * (1 - cost) - u(1) * sint;
    R(2, 1) = u(2) * u(1) * (1 - cost) + u(0) * sint;
    R(2, 2) = cost + u(2) * u(2) * (1 - cost);
    return R;
}

/**
Compute the Wigner d matrices.

*/
template <class Scalar>
inline void dlmn (
    int l, 
    const Scalar& s1,
    const Scalar& c1,
    const Scalar& c2,
    const Scalar& tgbet2,
    const Scalar& s3,
    const Scalar& c3,
    std::vector<Matrix<Scalar>>& D,
    std::vector<Matrix<Scalar>>& R
) {
    int iinf = 1 - l;
    int isup = -iinf;
    int m, mp;
    int al, al1, tal1, amp, laux, lbux, am, lauz, lbuz;
    int sign;
    Scalar ali, auz, aux, cux, fact, term, cuz;
    Scalar cosaux, cosmal, sinmal, cosag, sinag, cosagm, sinagm, cosmga, sinmga;
    Scalar d1, d2;

    // Compute the D[l;m',m) matrix.
    // First row by recurrence (Eq. 19 and 20 in Alvarez Collado et al.)
    D[l](2 * l, 2 * l) = 0.5 * D[l - 1](isup + l - 1, isup + l - 1) 
                             * (Scalar(1.0) + c2);
    D[l](2 * l, 0) = 0.5 * D[l - 1](isup + l - 1, -isup + l - 1) 
                         * (Scalar(1.0) - c2);
    for (m = isup; m > iinf - 1; --m)
        D[l](2 * l, m + l) = -tgbet2 * sqrt(Scalar(l + m + 1) / (l - m)) 
                                         * D[l](2 * l, m + 1 + l);

    // The rows of the upper quarter triangle of the D[l;m',m) matrix
    // (Eq. 21 in Alvarez Collado et al.)
    al = l;
    al1 = al - 1;
    tal1 = al + al1;
    ali = Scalar(1.0) / al1;
    cosaux = c2 * al * al1;
    for (mp = l - 1; mp > -1; --mp) {
        amp = mp;
        laux = l + mp;
        lbux = l - mp;
        aux = ali / sqrt(Scalar(laux * lbux));
        cux = sqrt(Scalar((laux - 1) * (lbux - 1))) * al;
        for (m = isup; m > iinf - 1; --m) {
            am = m;
            lauz = l + m;
            lbuz = l - m;
            auz = Scalar(1.0) / sqrt(Scalar(lauz * lbuz));
            fact = aux * auz;
            term = tal1 * (cosaux - Scalar(am * amp)) 
                        * D[l - 1](mp + l - 1, m + l - 1);
            if ((lbuz != 1) && (lbux != 1)) {
                cuz = sqrt(Scalar((lauz - 1) * (lbuz - 1)));
                term = term - D[l - 2](mp + l - 2, m + l - 2) * cux * cuz;
            }
            D[l](mp + l, m + l) = fact * term;
        }
        ++iinf;
        --isup;
    }

    // The remaining elements of the D[l;m',m) matrix are calculated
    // using the corresponding symmetry relations:
    // reflection ---> ((-1)**(m-m')) D[l;m,m') = D[l;m',m), m'<=m
    // inversion ---> ((-1)**(m-m')) D[l;-m',-m) = D[l;m',m)

    // Reflection
    sign = 1;
    iinf = -l;
    isup = l - 1;
    for (m = l; m > 0; --m) {
        for (mp = iinf; mp < isup + 1; ++mp) {
            D[l](mp + l, m + l) = sign * D[l](m + l, mp + l);
            sign *= -1;
        }
        ++iinf;
        --isup;
    }

    // Inversion
    iinf = -l;
    isup = iinf;
    for (m = l - 1; m > -(l + 1); --m) {
        sign = -1;
        for (mp = isup; mp > iinf - 1; --mp) {
            D[l](mp + l, m + l) = sign * D[l](-mp + l, -m + l);
            sign *= -1;
        }
        ++isup;
    }

    // Compute the real rotation matrices R from the complex ones D
    R[l](0 + l, 0 + l) = D[l](0 + l, 0 + l);
    cosmal = c1;
    sinmal = s1;
    sign = -1;
    Scalar root_two = sqrt(Scalar(2.0));
    for (mp = 1; mp < l + 1; ++mp) {
        cosmga = c3;
        sinmga = s3;
        aux = root_two * D[l](0 + l, mp + l);
        R[l](mp + l, 0 + l) = aux * cosmal;
        R[l](-mp + l, 0 + l) = aux * sinmal;
        for (m = 1; m < l + 1; ++m) {
            aux = root_two * D[l](m + l, 0 + l);
            R[l](l, m + l) = aux * cosmga;
            R[l](l, -m + l) = -aux * sinmga;
            d1 = D[l](-mp + l, -m + l);
            d2 = sign * D[l](mp + l, -m + l);
            cosag = cosmal * cosmga - sinmal * sinmga;
            cosagm = cosmal * cosmga + sinmal * sinmga;
            sinag = sinmal * cosmga + cosmal * sinmga;
            sinagm = sinmal * cosmga - cosmal * sinmga;
            R[l](mp + l, m + l) = d1 * cosag + d2 * cosagm;
            R[l](mp + l, -m + l) = -d1 * sinag + d2 * sinagm;
            R[l](-mp + l, m + l) = d1 * sinag + d2 * sinagm;
            R[l](-mp + l, -m + l) = d1 * cosag - d2 * cosagm;
            aux = cosmga * c3 - sinmga * s3;
            sinmga = sinmga * c3 + cosmga * s3;
            cosmga = aux;
        }
        sign *= -1;
        aux = cosmal * c1 - sinmal * s1;
        sinmal = sinmal * c1 + cosmal * s1;
        cosmal = aux;
    }
}

/**
Compute the Wigner D matrices.

*/
template <class Scalar>
inline void rotar (
    const int ydeg,
    const Scalar& c1,
    const Scalar& s1,
    const Scalar& c2,
    const Scalar& s2,
    const Scalar& c3,
    const Scalar& s3,
    const Scalar& tol,
    std::vector<Matrix<Scalar>>& D,
    std::vector<Matrix<Scalar>>& R
) {
    Scalar cosag, cosamg, sinag, sinamg, tgbet2;
    Scalar root_two = sqrt(Scalar(2.0));

    // Compute the initial matrices D0, R0, D1 and R1
    D[0](0, 0) = 1.0;
    R[0](0, 0) = 1.0;
    D[1](2, 2) = 0.5 * (Scalar(1.0) + c2);
    D[1](2, 1) = -s2 / root_two;
    D[1](2, 0) = 0.5 * (Scalar(1.0) - c2);
    D[1](1, 2) = -D[1](2, 1);
    D[1](1, 1) = D[1](2, 2) - D[1](2, 0);
    D[1](1, 0) = D[1](2, 1);
    D[1](0, 2) = D[1](2, 0);
    D[1](0, 1) = D[1](1, 2);
    D[1](0, 0) = D[1](2, 2);
    cosag = c1 * c3 - s1 * s3;
    cosamg = c1 * c3 + s1 * s3;
    sinag = s1 * c3 + c1 * s3;
    sinamg = s1 * c3 - c1 * s3;
    R[1](1, 1) = D[1](1, 1);
    R[1](2, 1) = root_two * D[1](1, 2) * c1;
    R[1](0, 1) = root_two * D[1](1, 2) * s1;
    R[1](1, 2) = root_two * D[1](2, 1) * c3;
    R[1](1, 0) = -root_two * D[1](2, 1) * s3;
    R[1](2, 2) = D[1](2, 2) * cosag - D[1](2, 0) * cosamg;
    R[1](2, 0) = -D[1](2, 2) * sinag - D[1](2, 0) * sinamg;
    R[1](0, 2) = D[1](2, 2) * sinag - D[1](2, 0) * sinamg;
    R[1](0, 0) = D[1](2, 2) * cosag + D[1](2, 0) * cosamg;

    // The remaining matrices are calculated using 
    // symmetry and and recurrence relations
    if (abs(s2) < tol)
        tgbet2 = s2; // = 0
    else
        tgbet2 = (Scalar(1.0) - c2) / s2;

    for (int l = 2; l < ydeg + 1; ++l)
        dlmn(l, s1, c1, c2, tgbet2, s3, c3, D, R);

    return;
}

/**

*/
template <typename Scalar>
inline void axisAngleToEuler (
    const Scalar& axis_x,
    const Scalar& axis_y,
    const Scalar& costheta,
    const Scalar& sintheta,
    const Scalar& tol,
    Scalar& cosalpha,
    Scalar& sinalpha,
    Scalar& cosbeta,
    Scalar& sinbeta,
    Scalar& cosgamma,
    Scalar& singamma
) {
    // Construct the axis-angle rotation matrix R_A
    Scalar RA01 = axis_x * axis_y * (Scalar(1.0) - costheta);
    Scalar RA02 = axis_y * sintheta;
    Scalar RA11 = costheta + axis_y * axis_y * (Scalar(1.0) - costheta);
    Scalar RA12 = -axis_x * sintheta;
    Scalar RA20 = -axis_y * sintheta;
    Scalar RA21 = axis_x * sintheta;
    Scalar RA22 = costheta;

    // Determine the Euler angles
    Scalar norm1, norm2;
    if ((RA22 < Scalar(-1.0) + tol) && (RA22 > Scalar(-1.0) - tol)) {
        cosbeta = RA22; // = -1
        sinbeta = Scalar(1.0) + RA22; // = 0
        cosgamma = RA11;
        singamma = RA01;
        cosalpha = -RA22; // = 1
        sinalpha = Scalar(1.0) + RA22; // = 0
    } else if ((RA22 < Scalar(1.0) + tol) && (RA22 > Scalar(1.0) - tol)) {
        cosbeta = RA22; // = 1
        sinbeta = Scalar(1.0) - RA22; // = 0
        cosgamma = RA11;
        singamma = -RA01;
        cosalpha = RA22; // = 1
        sinalpha = Scalar(1.0) - RA22; // = 0
    } else {
        cosbeta = RA22;
        sinbeta = sqrt(Scalar(1.0) - cosbeta * cosbeta);
        norm1 = sqrt(RA20 * RA20 + RA21 * RA21);
        norm2 = sqrt(RA02 * RA02 + RA12 * RA12);
        cosgamma = -RA20 / norm1;
        singamma = RA21 / norm1;
        cosalpha = RA02 / norm2;
        sinalpha = RA12 / norm2;
    }
}

/**
Rotation matrix class for the spherical harmonics.

*/
template <class Scalar>
class Wigner {

protected:

    const int ydeg;                                                            /**< Highest degree of the map */
    const int ufdeg;                                                           /**< Degree of limb darkening + filter map */
    const int Ny;                                                              /**< Number of map coefficients */
    const int N;                                                               /**< Total number of coefficients */
    const Scalar tol;                                                          /**< Numerical tolerance used to prevent division-by-zero errors */

    // The rotation matrices
    std::vector<Matrix<Scalar>> DZeta;                                         /**< The complex Wigner matrix in the `zeta` frame */
    std::vector<Matrix<Scalar>> RZeta;                                         /**< The real Wigner matrix in the `zeta` frame */
    std::vector<Matrix<Scalar>> RZetaInv;                                      /**< The inverse of the real Wigner matrix in the `zeta` frame */
    
    // Their derivatives
    using ADType = ADScalar<Scalar, 2>;                                        /**< AutoDiffScalar type for derivs w.r.t. the rotation axis */
    std::vector<Matrix<ADType>> DZeta_ad;                                      /**< [AutoDiffScalar] The complex Wigner matrix in the `zeta` frame */
    std::vector<Matrix<ADType>> RZeta_ad;                                      /**< [AutoDiffScalar] The real Wigner matrix in the `zeta` frame */
    std::vector<Matrix<Scalar>> DRZetaDInc;
    std::vector<Matrix<Scalar>> DRZetaInvDInc;
    std::vector<Matrix<Scalar>> DRZetaDObl;
    std::vector<Matrix<Scalar>> DRZetaInvDObl;

    Vector<Scalar> cosmt;                                                      /**< Vector of cos(m theta) values */
    Vector<Scalar> sinmt;                                                      /**< Vector of sin(m theta) values */
    Vector<Scalar> cosnt;                                                      /**< Vector of cos(n theta) values */
    Vector<Scalar> sinnt;                                                      /**< Vector of sin(n theta) values */

    Scalar cache_costheta;
    Scalar cache_sintheta;

public:

    template <typename T1, typename T2>
    inline void leftMultiplyRz (
        const MatrixBase<T1>& vT, 
        MatrixBase<T2> const & uT
    );

    template <typename T1, typename T2>
    inline void leftMultiplyRzAugmented (
        const MatrixBase<T1>& vT, 
        MatrixBase<T2> const & uT
    );

    template <typename T1, typename T2>
    inline void leftMultiplyDRz (
        const MatrixBase<T1>& vT, 
        MatrixBase<T2> const & uT
    );

    template <typename T1, typename T2>
    inline void leftMultiplyDRzAugmented (
        const MatrixBase<T1>& vT, 
        MatrixBase<T2> const & uT
    );

    template <typename T1, typename T2>
    inline void leftMultiplyRZeta (
        const MatrixBase<T1>& vT, 
        MatrixBase<T2> const & uT
    );

    template <typename T1, typename T2>
    inline void leftMultiplyRZetaInv (
        const MatrixBase<T1>& vT, 
        MatrixBase<T2> const & uT
    );

    template <typename T1, typename T2>
    inline void leftMultiplyDRZetaDInc (
        const MatrixBase<T1>& vT, 
        MatrixBase<T2> const & uT
    );

    template <typename T1, typename T2>
    inline void leftMultiplyDRZetaInvDInc (
        const MatrixBase<T1>& vT, 
        MatrixBase<T2> const & uT
    );

    template <typename T1, typename T2>
    inline void leftMultiplyDRZetaDObl (
        const MatrixBase<T1>& vT, 
        MatrixBase<T2> const & uT
    );

    template <typename T1, typename T2>
    inline void leftMultiplyDRZetaInvDObl (
        const MatrixBase<T1>& vT, 
        MatrixBase<T2> const & uT
    );

    template <typename T1, typename T2>
    inline void leftMultiplyR (
        const MatrixBase<T1>& vT, 
        MatrixBase<T2> const & uT
    );

    inline void compute (
        const Scalar& costheta, 
        const Scalar& sintheta
    );

    template <typename T1, typename T2>
    inline void rotate (
        const MatrixBase<T1>& y,
        const Scalar& costheta,
        const Scalar& sintheta,
        MatrixBase<T2>& Ry
    );

    inline void updateAxis (
        const Scalar& inc_,
        const Scalar& obl_
    );

    Wigner(
        int ydeg, 
        int udeg,
        int fdeg,
        const Scalar& inc_,
        const Scalar& obl_
    ) : 
        ydeg(ydeg), 
        ufdeg(udeg + fdeg),
        Ny((ydeg + 1) * (ydeg + 1)), 
        N((ydeg + ufdeg + 1) * (ydeg + ufdeg + 1)),
        tol(10 * mach_eps<Scalar>()), 
        DZeta(ydeg + 1),
        RZeta(ydeg + 1),
        RZetaInv(ydeg + 1),
        DZeta_ad(ydeg + 1),
        RZeta_ad(ydeg + 1),
        DRZetaDInc(ydeg + 1),
        DRZetaInvDInc(ydeg + 1),
        DRZetaDObl(ydeg + 1),
        DRZetaInvDObl(ydeg + 1)
    {
        // Allocate the Wigner matrices
        for (int l = 0; l < ydeg + 1; ++l) {
            int sz = 2 * l + 1;
            DZeta[l].resize(sz, sz);
            RZeta[l].resize(sz, sz);
            RZetaInv[l].resize(sz, sz);
            DZeta_ad[l].resize(sz, sz);
            RZeta_ad[l].resize(sz, sz);
            DRZetaDObl[l].resize(sz, sz);
            DRZetaInvDObl[l].resize(sz, sz);
            DRZetaDInc[l].resize(sz, sz);
            DRZetaInvDInc[l].resize(sz, sz);
        }

        // Initialize our z rotation vectors
        cosnt.resize(max(2, ydeg + ufdeg + 1));
        cosnt(0) = 1.0;
        sinnt.resize(max(2, ydeg + ufdeg + 1));
        sinnt(0) = 0.0;
        cosmt.resize(N);
        sinmt.resize(N);

        // Reset the cache
        cache_costheta = NAN;
        cache_sintheta = NAN;

        // Update the Zeta matrices
        updateAxis(inc_, obl_);
    }

};

/* 
Computes the dot product uT = vT . Rz.

*/
template <class Scalar>
template <typename T1, typename T2>
inline void Wigner<Scalar>::leftMultiplyRz (
    const MatrixBase<T1>& vT, 
    MatrixBase<T2> const & uT
) {
    for (int l = 0; l < ydeg + 1; ++l) {
        for (int j = 0; j < 2 * l + 1; ++j) {
            MBCAST(uT, T2).col(l * l + j) = 
                vT.col(l * l + j) * cosmt(l * l + j) +
                vT.col(l * l + 2 * l - j) * sinmt(l * l + j);
        }
    }
}

/* 
Computes the dot product uT = vT . dRz / dtheta.

*/
template <class Scalar>
template <typename T1, typename T2>
inline void Wigner<Scalar>::leftMultiplyDRz (
    const MatrixBase<T1>& vT, 
    MatrixBase<T2> const & uT
) {
    for (int l = 0; l < ydeg + 1; ++l) {
        for (int j = 0; j < 2 * l + 1; ++j) {
            int m = j - l;
            MBCAST(uT, T2).col(l * l + j) = 
                vT.col(l * l + 2 * l - j) * m * cosmt(l * l + j) -
                vT.col(l * l + j) * m * sinmt(l * l + j);
        }
    }
}

/* 
Computes the dot product uT = vT . Rz.

*/
template <class Scalar>
template <typename T1, typename T2>
inline void Wigner<Scalar>::leftMultiplyRzAugmented (
    const MatrixBase<T1>& vT, 
    MatrixBase<T2> const & uT
) {
    for (int l = 0; l < ydeg + ufdeg + 1; ++l) {
        for (int j = 0; j < 2 * l + 1; ++j) {
            MBCAST(uT, T2).col(l * l + j) = 
                vT.col(l * l + j) * cosmt(l * l + j) +
                vT.col(l * l + 2 * l - j) * sinmt(l * l + j);
        }
    }
}

/* 
Computes the dot product uT = vT . dRz / dtheta.

*/
template <class Scalar>
template <typename T1, typename T2>
inline void Wigner<Scalar>::leftMultiplyDRzAugmented (
    const MatrixBase<T1>& vT, 
    MatrixBase<T2> const & uT
) {
    for (int l = 0; l < ydeg + ufdeg + 1; ++l) {
        for (int j = 0; j < 2 * l + 1; ++j) {
            int m = j - l;
            MBCAST(uT, T2).col(l * l + j) = 
                vT.col(l * l + 2 * l - j) * m * cosmt(l * l + j) -
                vT.col(l * l + j) * m * sinmt(l * l + j);
        }
    }
}

/* 
Computes the dot product uT = vT . RZeta.

*/
template <class Scalar>
template <typename T1, typename T2>
inline void Wigner<Scalar>::leftMultiplyRZeta (
    const MatrixBase<T1>& vT, 
    MatrixBase<T2> const & uT
) {
    for (int l = 0; l < ydeg + 1; ++l) {
        MBCAST(uT, T2).block(0, l * l, uT.rows(), 2 * l + 1) =
            vT.block(0, l * l, uT.rows(), 2 * l + 1) * RZeta[l];
    }
}

/* 
Computes the dot product uT = vT . RZetaInv.

*/
template <class Scalar>
template <typename T1, typename T2>
inline void Wigner<Scalar>::leftMultiplyRZetaInv (
    const MatrixBase<T1>& vT, 
    MatrixBase<T2> const & uT
) {
    for (int l = 0; l < ydeg + 1; ++l) {
        MBCAST(uT, T2).block(0, l * l, uT.rows(), 2 * l + 1) =
            vT.block(0, l * l, uT.rows(), 2 * l + 1) * RZetaInv[l];
    }
}

/* 
Computes the dot product uT = vT . dRZeta / dInc.

*/
template <class Scalar>
template <typename T1, typename T2>
inline void Wigner<Scalar>::leftMultiplyDRZetaDInc (
    const MatrixBase<T1>& vT, 
    MatrixBase<T2> const & uT
) {
    for (int l = 0; l < ydeg + 1; ++l) {
        MBCAST(uT, T2).block(0, l * l, uT.rows(), 2 * l + 1) =
            vT.block(0, l * l, uT.rows(), 2 * l + 1) * DRZetaDInc[l];
    }
}

/* 
Computes the dot product uT = vT . dRZetaInv / dInc.

*/
template <class Scalar>
template <typename T1, typename T2>
inline void Wigner<Scalar>::leftMultiplyDRZetaInvDInc (
    const MatrixBase<T1>& vT, 
    MatrixBase<T2> const & uT
) {
    for (int l = 0; l < ydeg + 1; ++l) {
        MBCAST(uT, T2).block(0, l * l, uT.rows(), 2 * l + 1) =
            vT.block(0, l * l, uT.rows(), 2 * l + 1) * DRZetaInvDInc[l];
    }
}

/* 
Computes the dot product uT = vT . dRZeta / dObl.

*/
template <class Scalar>
template <typename T1, typename T2>
inline void Wigner<Scalar>::leftMultiplyDRZetaDObl (
    const MatrixBase<T1>& vT, 
    MatrixBase<T2> const & uT
) {
    for (int l = 0; l < ydeg + 1; ++l) {
        MBCAST(uT, T2).block(0, l * l, uT.rows(), 2 * l + 1) =
            vT.block(0, l * l, uT.rows(), 2 * l + 1) * DRZetaDObl[l];
    }
}

/* 
Computes the dot product uT = vT . dRZetaInv / dObl.

*/
template <class Scalar>
template <typename T1, typename T2>
inline void Wigner<Scalar>::leftMultiplyDRZetaInvDObl (
    const MatrixBase<T1>& vT, 
    MatrixBase<T2> const & uT
) {
    for (int l = 0; l < ydeg + 1; ++l) {
        MBCAST(uT, T2).block(0, l * l, uT.rows(), 2 * l + 1) =
            vT.block(0, l * l, uT.rows(), 2 * l + 1) * DRZetaInvDObl[l];
    }
}

/* 
Computes the dot product uT = vT . R.

*/
template <class Scalar>
template <typename T1, typename T2>
inline void Wigner<Scalar>::leftMultiplyR (
    const MatrixBase<T1>& vT, 
    MatrixBase<T2> const & uT
) {
    typename T1::PlainObject tmp(uT.rows(), uT.cols());
    leftMultiplyRZetaInv(vT, uT);
    leftMultiplyRz(uT, tmp);
    leftMultiplyRZeta(tmp, uT);
}

template <class Scalar>
inline void Wigner<Scalar>::compute (
    const Scalar& costheta,
    const Scalar& sintheta
) {
    // Did we do this already?
    if ((costheta == cache_costheta) && (sintheta == cache_sintheta))
        return;
    cache_costheta = costheta;
    cache_sintheta = sintheta;

    // Compute the cos and sin vectors for the zhat rotation
    cosnt(1) = costheta;
    sinnt(1) = sintheta;
    for (int n = 2; n < ydeg + ufdeg + 1; ++n) {
        cosnt(n) = 2.0 * cosnt(n - 1) * cosnt(1) - cosnt(n - 2);
        sinnt(n) = 2.0 * sinnt(n - 1) * cosnt(1) - sinnt(n - 2);
    }
    int n = 0;
    for (int l = 0; l < ydeg + ufdeg + 1; ++l) {
        for (int m = -l; m < 0; ++m) {
            cosmt(n) = cosnt(-m);
            sinmt(n) = -sinnt(-m);
            ++n;
        }
        for (int m = 0; m < l + 1; ++m) {
            cosmt(n) = cosnt(m);
            sinmt(n) = sinnt(m);
            ++n;
        }
    }
}

/**
Rotates a spherical harmonic vector `y`. 
Returns the rotated vector `R(theta) . y`.

*/
template <class Scalar>
template <typename T1, typename T2>
inline void Wigner<Scalar>::rotate (
    const MatrixBase<T1>& y,
    const Scalar& costheta,
    const Scalar& sintheta,
    MatrixBase<T2>& Ry
) {
    Ry.resize(Ny);
    compute(costheta, -sintheta);
    leftMultiplyR(y.transpose(), Ry.transpose());
}

/**
Update the zeta rotation matrix and compute its gradient
with respect to the axis of rotation.

*/
template <class Scalar>
inline void Wigner<Scalar>::updateAxis (
    const Scalar& inc_,
    const Scalar& obl_
) 
{
    // Reset the cache
    cache_costheta = NAN;
    cache_sintheta = NAN;

    // Convert to radians & ADType
    ADType inc = inc_ * pi<Scalar>() / 180.;
    ADType obl = obl_ * pi<Scalar>() / 180.;
    inc.derivatives() = Vector<Scalar>::Unit(2, 0);
    obl.derivatives() = Vector<Scalar>::Unit(2, 1);
    ADType sini = sin(inc),
           cosi = cos(inc),
           neg_sino = -sin(obl),
           coso = cos(obl);

    // Compute the rotation transformation into and out of the `zeta` frame
    if (ydeg == 0) {
        // Trivial case
        RZeta[0](0, 0) = 1;
        RZetaInv[0](0, 0) = 1;
        DRZetaDInc[0].setZero();
        DRZetaInvDInc[0].setZero();
        DRZetaDObl[0].setZero();
        DRZetaInvDObl[0].setZero();
    } else if (abs(sini.value()) < tol) {
        // The rotation axis is about +/- zhat
        if (cosi.value() > 0) {
            // Trivial: the zeta frame is the current frame
            for (int l = 0; l < ydeg + 1; l++) {
                RZeta[l] = Matrix<Scalar>::Identity(2 * l + 1, 2 * l + 1);
            } 
        } else {
            // We must flip the body around, since the axis is -zhat
            rotar(ydeg, Scalar(1.0), Scalar(0.0), Scalar(-1.0), 
                  Scalar(0.0), Scalar(1.0), Scalar(0.0), tol, DZeta, RZeta);
        }
        for (int l = 0; l < ydeg + 1; l++) {
            RZetaInv[l] = RZeta[l].transpose();
            DRZetaDInc[l].setZero();
            DRZetaInvDInc[l].setZero();
            DRZetaDObl[l].setZero();
            DRZetaInvDObl[l].setZero();
        }
    } else {
        // We need to compute the actual Wigner matrices
        ADType tol_ad = tol;
        ADType cosalpha, sinalpha, cosbeta, sinbeta, cosgamma, singamma;
        axisAngleToEuler(coso, neg_sino, cosi, sini, tol_ad,
                         cosalpha, sinalpha, cosbeta, sinbeta, 
                         cosgamma, singamma);

        // Call the Eulerian rotation function
        rotar(ydeg, cosalpha, sinalpha, cosbeta, sinbeta, 
              cosgamma, singamma, tol_ad, DZeta_ad, RZeta_ad);

        // Extract the matrices and their derivatives
        for (int l = 0; l < ydeg + 1; ++l) {
            // \todo This data copy is *very* slow
            for (int i = 0; i < 2 * l + 1; ++i) {
                for (int j = 0; j < 2 * l + 1; ++j) {
                    RZeta[l](i, j) = RZeta_ad[l](i, j).value();
                    DRZetaDInc[l](i, j) = RZeta_ad[l](i, j).derivatives()(0);
                    DRZetaDObl[l](i, j) = RZeta_ad[l](i, j).derivatives()(1);
                }
            }
            RZetaInv[l] = RZeta[l].transpose();
            DRZetaInvDInc[l] = DRZetaDInc[l].transpose();
            DRZetaInvDObl[l] = DRZetaDObl[l].transpose();
        }

    }
}

} // namespace wigner
} // namespace starry

#endif