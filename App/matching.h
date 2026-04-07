#ifndef MATCHING_H
#define MATCHING_H

#include "sift.h"
#include <vector>
#include <QImage>

namespace feature {

    struct Match {
        int queryIdx; // Index cleanly referencing origin array keypoint in Image A
        int trainIdx; // Index referencing matched array geometry inside Image B
        double distance; // Scalar mapping offset
    };

    // Implements SSD distance tracking natively isolated across the 128-D matrix utilizing Lowe's fraction rejection mathematics
    std::vector<Match> matchFeaturesSSD(const std::vector<SiftKeypoint>& a, const std::vector<SiftKeypoint>& b, double ratio_thresh = 0.8);

    // Executes Normalized Cross-Correlation structural vector dot products dynamically bypassing unbounded Euclidean lengths natively
    std::vector<Match> matchFeaturesNCC(const std::vector<SiftKeypoint>& a, const std::vector<SiftKeypoint>& b, double ncc_thresh = 0.85);

    // Binds matrices and mathematically physically interpolates vectors horizontally mapping `(x,y)` geometrically to `(x+offset, y)`
    QImage drawMatches(const QImage& imgA, const QImage& imgB, const std::vector<SiftKeypoint>& kpsA, const std::vector<SiftKeypoint>& kpsB, const std::vector<Match>& matches);

} // namespace feature

#endif // MATCHING_H
