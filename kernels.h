#ifndef KERNELS_H
#define KERNELS_H

#include <array>

//template<class T, size_t Rows, size_t Cols> using Matrix = std::array<std::array<T, Cols>, Rows>;
namespace algorithms
{

    template<class T>
    struct Matrix {
        mutable std::vector<std::vector<T>> _v;

    public:
        Matrix() {}

        Matrix(std::vector<std::vector<T>> &&v) : _v(v) {}

        Matrix(const std::vector<std::vector<T>> &v) : _v(v) {}

        std::vector<T>& operator[] (int i) {
            return _v[i];
        }

        std::vector<T>& operator[] (int i) const {
            return _v[i];
        }

        auto begin() {
            return _v.begin();
        }

        auto end() {
            return _v.end();
        }

        int size() const {
            return _v.size();
        }
    };

    const Matrix<int> sobelx({ {-1, 0, 1}, {-2, 0, 2}, {-1, 0, 1} });
    const Matrix<int> sobely ({ {1, 2, 1}, {0, 0, 0}, {-1, -2, -1} });
    const Matrix<int> prewittx ({ {-1, 0, 1}, {-1, 0, 1}, {-1, 0, 1} });
    const Matrix<int> prewitty ({ {-1, -1, -1}, {0, 0, 0}, {1, 1, 1} });
    const Matrix<int> robertsx ({ {1, 0}, {0, -1} });
    const Matrix<int> robertsy ({ {0, 1} , {-1, 0} });
    const Matrix<int> scharrx ({ {3, 10, 3}, {0, 0, 0}, {-3, -10, -3} });
    const Matrix<int> scharry ({ {3, 0, -3}, {10, 0, -10}, {3, 0, -3} });

    template <class T>
    auto getGaborKernel(T sigma, T theta,
                        T lambd, T gamma, T psi,
                        int W = 5, int H = 5)
    {
        int nstds = 3;
        int xmin, xmax, ymin, ymax;
        T sigma_x = sigma;
        T sigma_y = sigma / gamma;
        T c = cos(theta);
        T s = sin(theta);

        if ( W > 0 ) {
            xmax = W / 2;
        } else {
            xmax = round(std::max(fabs(nstds*sigma_x*c), fabs(nstds*sigma_y*s)));
        }

        if ( H > 0 ) {
            ymax = H / 2;
        } else {
            ymax = round(std::max(fabs(nstds*sigma_x*s), fabs(nstds*sigma_y*c)));
        }

        xmin = -xmax;
        ymin = -ymax;

    //    CV_Assert( ktype == CV_32F || ktype == CV_64F ); TODO: T == double || T == double

        Matrix<T> kernel(std::vector<std::vector<T>>(W, std::vector<T>(H)));
        T scale = 1;
        T ex = -0.5 / (sigma_x * sigma_x);
        T ey = -0.5 / (sigma_y * sigma_y);
        T cscale = M_PI*2/lambd;

        for ( int y = ymin; y <= ymax; y++ ) {
            for ( int x = xmin; x <= xmax; x++ ) {
                T xr = x*c + y*s;
                T yr = -x*s + y*c;

                T v = scale * std::exp(ex*xr*xr + ey*yr*yr)*cos(cscale*xr + psi);
                kernel[ymax - y][xmax - x] = static_cast<T>(v);
            }
        }

        return kernel;
    }
}

#endif // KERNELS_H
