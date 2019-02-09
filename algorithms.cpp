#include <QtWidgets>
#include <utility>
#include <queue>
#include "algorithms.h"

using std::queue;
using std::vector;
using std::pair;
using std::make_pair;

namespace algorithms
{


    // Take sqrt(a^2 + b^2) where a and b is color value in gray scale of two images
    void magnitude(QImage& input, const QImage& gx, const QImage& gy) {
        quint8 *line;
        const quint8 *gx_line, *gy_line;

        for (int y = 0; y < input.height(); y++) {
            line = input.scanLine(y);
            gx_line = gx.constScanLine(y);
            gy_line = gy.constScanLine(y);

            for (int x = 0; x < input.width(); x++) {
                line[x] = qBound(0x00, static_cast<int>(hypot(gx_line[x], gy_line[x])), 0xFF);
            }
        }
    }

    QImage hysteresis(const QImage& image, double tmin, double tmax) {
        auto res = QImage(image.size(), image.format());
        res.fill(0x00);

        const quint8 *original_line;
        quint8 *result_line;
        int ni, nj;
        queue<pair<int, int>> edges;

        for (int y = 1; y < res.height() - 1; y++) {
            original_line = image.constScanLine(y);
            result_line = res.scanLine(y);

            for (int x = 1; x < res.width() - 1; x++) {
                if (original_line[x] >= tmax && result_line[x] == 0x00) {
                    result_line[x] = 0xFF;
                    edges.push(make_pair(x, y));

                    while (!edges.empty()) {
                        auto point = edges.front();
                        edges.pop();

                        for (int j = -1; j <= 1; j++) {
                            nj = point.second + j;
                            if (nj < 0 || nj >= image.height())
                                continue;

                            original_line = image.constScanLine(nj);
                            result_line = res.scanLine(nj);

                            for (int i = -1; i <= 1; i++) {
                                if (!i && !j)
                                    continue;

                                ni = point.first + i;
                                if (ni < 0 || ni >= image.width())
                                    continue;

                                if (original_line[ni] >= tmin && result_line[ni] == 0x00) {
                                    result_line[ni] = 0xFF;
                                    edges.push(make_pair(ni, nj));
                                }
                            }
                        }
                    }
                }
            }
        }

        return res;
    }


    QImage canny(const QImage& input, double sigma, double tmin, double tmax) {
        // Gaussian blur
        QImage res = convolution(getGaussianKernel(sigma), input);

        // Gradients
        QImage gx = convolution(sobelx, res);
        QImage gy = convolution(sobely, res);

        // Take sqrt(a^2 + b^2) for each pixel in res
        // Where a - pixel in gx and b - pixel in gy
        magnitude(res, gx, gy);

        // Non-maximum suppression
        quint8 *line;
        const quint8 *prev_line, *next_line, *gx_line, *gy_line;

        for (int y = 1; y < res.height() - 1; y++) {
            line = res.scanLine(y);
            prev_line = res.constScanLine(y - 1);
            next_line = res.constScanLine(y + 1);
            gx_line = gx.constScanLine(y);
            gy_line = gy.constScanLine(y);

            for (int x = 1; x < res.width() - 1; x++) {
                double at = atan2(gy_line[x], gx_line[x]);
                const double dir = fmod(at + M_PI, M_PI) / M_PI * 8;

                if ((1 >= dir || dir > 7) && line[x - 1] < line[x] && line[x + 1] < line[x] ||
                    (1 < dir || dir <= 3) && prev_line[x - 1] < line[x] && next_line[x + 1] < line[x] ||
                    (3 < dir || dir <= 5) && prev_line[x] < line[x] && next_line[x] < line[x] ||
                    (5 < dir || dir <= 7) && prev_line[x + 1] < line[x] && next_line[x - 1] < line[x])
                    continue;

                line[x] = 0x00;
            }
        }

        // Hysteresis
        return hysteresis(res, tmin, tmax);
    }


    QImage sobel(const QImage& input) {
        QImage res(input.size(), input.format());
        magnitude(res, convolution(sobelx, input), convolution(sobely, input));
        return res;
    }


    QImage prewitt(const QImage& input) {
        QImage res(input.size(), input.format());
        magnitude(res, convolution(prewittx, input), convolution(prewitty, input));
        return res;
    }

    QImage roberts(const QImage& input) {
        QImage res(input.size(), input.format());
        magnitude(res, convolution(robertsx, input), convolution(robertsy, input));
        return res;
    }


    QImage scharr(const QImage& input) {
        QImage res(input.size(), input.format());
        magnitude(res, convolution(scharrx, input), convolution(scharry, input));
        return res;
    }
}


