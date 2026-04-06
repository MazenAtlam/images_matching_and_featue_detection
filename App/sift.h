#ifndef SIFT_H
#define SIFT_H

#include "utils.h"
#include <vector>
#include <QImage>

namespace feature {

    struct SiftKeypoint {
        int x;
        int y;
        double sigma;
        double orientation;
        std::vector<double> descriptor; // strictly 128-dimensional

        SiftKeypoint(int x=0, int y=0, double s=0.0, double o=0.0) 
            : x(x), y(y), sigma(s), orientation(o) {}
    };

    // Detects keypoints and extracts 128-d descriptors from a grayscale image matrix
    std::vector<SiftKeypoint> extractSiftFeatures(const utils::Matrix2D& image, double initial_sigma, double scale_mult, double contrast_thresh);

    // Draws detected keypoints on top of the original QImage
    QImage drawSiftKeypoints(const QImage& originalImg, const std::vector<SiftKeypoint>& keypoints);

} // namespace feature

#endif // SIFT_H
