template <class S>
class Cache
{
protected:

    // Types
    using Scalar = typename S::Scalar;
    using YType = typename S::YType;
    using UType = typename S::UType;
    using FType = typename S::FType;

public:
    
    int lmax;
    int ncoly;
    int ncolu;
    int nflx;
    int N;

    // Flags
    bool compute_agol_g;
    bool compute_Zeta;
    bool compute_YZeta;
    bool compute_degree_y;
    bool compute_degree_u;
    bool compute_P;
    bool compute_agol_p;

    // Cached variables
    int res;
    Scalar taylort;
    Scalar theta;
    Scalar theta_with_grad;
    Matrix<Scalar> P;                                                          /**< The change of basis matrix from Ylms to pixels */
    UType agol_g;
    Matrix<Scalar> dAgolGdu;                                                   /**< Deriv of Agol `c` coeffs w/ respect to the limb darkening coeffs */
    UType agol_p;
    YType Ry;
    YType A1Ry;
    YType dRdthetay;
    YType p_uy;
    RowVector<Scalar> pT;
    std::vector<Matrix<Scalar>> EulerD;
    std::vector<Matrix<Scalar>> EulerR;

    // Pybind cache
    FType pb_flux;
    FType pb_time;
    FType pb_theta;
    FType pb_xo;
    FType pb_yo;
    FType pb_ro;
    RowMatrix<Scalar> pb_y;
    RowMatrix<Scalar> pb_u;

    //
    inline void yChanged () {
        compute_YZeta = true;
        compute_degree_y = true;
        compute_P = true;
        theta = NAN;
        theta_with_grad = NAN;
        // Recall that the normalization of the LD
        // polynomial depends on Y_{0,0}
        compute_agol_p = true;
        compute_agol_g = true;
    }

    inline void uChanged () {
        compute_degree_u = true;
        compute_agol_p = true;
        compute_agol_g = true;
    }

    inline void axisChanged () {
        compute_Zeta = true;
        compute_YZeta = true;
        theta = NAN;
        theta_with_grad = NAN;
    }

    inline void mapRotated () {
        compute_P = true;
        theta = NAN;
        theta_with_grad = NAN;
    }

    //! Reset all flags
    inline void reset () 
    {
        compute_agol_g = true;
        compute_Zeta = true;
        compute_YZeta = true;
        compute_degree_y = true;
        compute_degree_u = true;
        compute_P = true;
        compute_agol_p = true;
        res = -1;
        taylort = NAN;
        theta = NAN;
        theta_with_grad = NAN;
    };

    //! Constructor
    Cache (
        int lmax,
        int ncoly,
        int ncolu,
        int nflx
    ) :
        lmax(lmax),
        ncoly(ncoly),
        ncolu(ncolu),
        nflx(nflx),
        N((lmax + 1) * (lmax + 1)),
        agol_g(lmax + 1, ncolu),
        dAgolGdu(lmax * ncolu, lmax + 1),
        agol_p(N, ncolu),
        Ry(N, ncoly),
        A1Ry(N, ncoly),
        dRdthetay(N, ncoly),
        p_uy(N, ncoly),
        pT(N),
        EulerD(lmax + 1),
        EulerR(lmax + 1)
    {
        for (int l = 0; l < lmax + 1; ++l) {
            int sz = 2 * l + 1;
            EulerD[l].resize(sz, sz);
            EulerR[l].resize(sz, sz);
        }
        reset();
    };

}; // class Cache