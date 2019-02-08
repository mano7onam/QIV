#ifndef ALGORITHMS_H
#define ALGORITHMS_H

#include <cmath>
#include "kernels.h"

Matrix<float, 5, 5> gaussian_kernel(float);
void magnitude(QImage&, const QImage&, const QImage&);
QImage canny(const QImage&, float, float, float);
QImage sobel(const QImage&);
QImage prewitt(const QImage&);
QImage roberts(const QImage&);
QImage scharr(const QImage&);
QImage hysteresis(const QImage&, float, float);

QImage convolution(const auto& kernel, const QImage& image) {
    int kw = kernel[0].size(), kh = kernel.size(),
        offsetx = kw / 2, offsety = kw / 2;
    QImage out(image.size(), image.format());
    float sum;

    quint8 *line;
    const quint8 *lookup_line;

    for (int y = 0; y < image.height(); y++) {
        line = out.scanLine(y);
        for (int x = 0; x < image.width(); x++) {
            sum = 0;

            for (int j = 0; j < kh; j++) {
                if (y + j < offsety || y + j >= image.height())
                    continue;
                lookup_line = image.constScanLine(y + j - offsety);
                for (int i = 0; i < kw; i++) {
                    if (x + i < offsetx || x + i >= image.width())
                        continue;
                    sum += kernel[j][i] * lookup_line[x + i - offsetx];
                }
            }

            line[x] = qBound(0x00, static_cast<int>(sum), 0xFF);
        }
    }

    return out;
}

QImage convolution1(const Matrix<double, 5, 5>& kernel, const QImage& image) {
    int kw = kernel[0].size(), kh = kernel.size(),
        offsetx = kw / 2, offsety = kw / 2;
    QImage out(image.size(), image.format());
    float sum;

    quint8 *line;
    const quint8 *lookup_line;

    for (int y = 0; y < image.height(); y++) {
        line = out.scanLine(y);
        for (int x = 0; x < image.width(); x++) {
            sum = 0;

            for (int j = 0; j < kh; j++) {
                if (y + j < offsety || y + j >= image.height())
                    continue;
                lookup_line = image.constScanLine(y + j - offsety);
                for (int i = 0; i < kw; i++) {
                    if (x + i < offsetx || x + i >= image.width())
                        continue;
                    sum += kernel[j][i] * lookup_line[x + i - offsetx];
                }
            }

            line[x] = qBound(0x00, static_cast<int>(sum), 0xFF);
        }
    }

    return out;
}

Matrix<double, 5, 5> getGaborKernel1(double sigma, double theta,
                               double lambd, double gamma, double psi)
{
    int W = 5;
    int H = 5;
    double sigma_x = sigma;
    double sigma_y = sigma/gamma;
    int nstds = 3;
    int xmin, xmax, ymin, ymax;
    double c = cos(theta), s = sin(theta);

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

//    CV_Assert( ktype == CV_32F || ktype == CV_64F ); TODO: T == float || T == double

    Matrix<double, 5, 5> kernel;
    double scale = 1;
    double ex = -0.5/(sigma_x*sigma_x);
    double ey = -0.5/(sigma_y*sigma_y);
    double cscale = M_PI*2/lambd;

    for ( int y = ymin; y <= ymax; y++ ) {
        for ( int x = xmin; x <= xmax; x++ ) {
            double xr = x*c + y*s;
            double yr = -x*s + y*c;

            double v = scale * std::exp(ex*xr*xr + ey*yr*yr)*cos(cscale*xr + psi);
            kernel[ymax - y][xmax - x] = static_cast<double>(v);
        }
    }

    return kernel;
}

#endif // ALGORITHMS_H
