#ifndef HARRIS_H
#define HARRIS_H

#include "utils.h"
#include <vector>
#include <QImage>

namespace feature {

    // Struct to hold a corner point
    struct CornerPoint {
        int x;
        int y;
        double response;

        CornerPoint(int x=0, int y=0, double r=0.0) : x(x), y(y), response(r) {}
    };

    // Detects Harris corners from a grayscale image matrix
    // Returns a list of detected corners after non-maximal suppression
    std::vector<CornerPoint> detectHarrisCorners(const utils::Matrix2D& image, double sigma, double threshold);

    // Draws detected corner points on top of the original QImage
    QImage drawCorners(const QImage& originalImg, const std::vector<CornerPoint>& corners);

} // namespace feature

#endif // HARRIS_H
