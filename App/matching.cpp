#include "matching.h"
#include <cmath>
#include <limits>
#include <algorithm>
#include <QPainter>

namespace feature {

    std::vector<Match> matchFeaturesSSD(const std::vector<SiftKeypoint>& a, const std::vector<SiftKeypoint>& b, double ratio_thresh) {
        std::vector<Match> final_matches;

        // Iterate identically tracking coordinate references natively
        for (int i = 0; i < a.size(); ++i) {
            double best_dist = std::numeric_limits<double>::max();
            double second_best_dist = std::numeric_limits<double>::max();
            int best_j = -1;

            // Exhaustive linear topological evaluation block iterating mapping combinations identically
            for (int j = 0; j < b.size(); ++j) {
                double dist = 0.0;
                
                // Tracking standard Sum of Squared Euclidean variances with Partial Vector Bail-Out
                for (int d = 0; d < 128; ++d) {
                    // Loop unrolling optimizations are natively handled by GCC, but we actively dynamically bound early
                    double diff = a[i].descriptor[d] - b[j].descriptor[d];
                    dist += diff * diff;
                    
                    // Early Termination Bounding:
                    // If the partial distance calculation already mathematically exceeds the second minimum boundary,
                    // it is physically impossible for this vector to challenge the primary tracking constraints!
                    if (dist >= second_best_dist) {
                        break;
                    }
                }

                if (dist < best_dist) {
                    second_best_dist = best_dist;
                    best_dist = dist;
                    best_j = j;
                } else if (dist < second_best_dist) {
                    second_best_dist = dist;
                }
            }

            // Utilizing Lowe's strict tracking ratio filter natively optimizing bounds algebraically squaring geometric matrices tracking CPU limits natively!
            if (second_best_dist > 1e-6) {
                // Lowe's linear ratio bound natively calculates: best < second * threshold
                // By structurally tracking squared vectors inherently statically we strip computationally massive std::sqrt natively avoiding redundant CPU cycle drops!
                if (best_dist < (ratio_thresh * ratio_thresh) * second_best_dist) {
                    Match m;
                    m.queryIdx = i;
                    m.trainIdx = best_j;
                    m.distance = std::sqrt(best_dist); // Calculate exact boundary once dynamically tracking final arrays!
                    final_matches.push_back(m);
                }
            }
        }

        return final_matches;
    }

    std::vector<Match> matchFeaturesNCC(const std::vector<SiftKeypoint>& a, const std::vector<SiftKeypoint>& b, double ncc_thresh) {
        std::vector<Match> final_matches;

        for (int i = 0; i < a.size(); ++i) {
            double best_ncc = -2.0; 
            int best_j = -1;

            for (int j = 0; j < b.size(); ++j) {
                double ncc = 0.0;
                // Directly processing the mathematically normalized vectors algebraic dot product
                for (int d = 0; d < 128; ++d) {
                    ncc += a[i].descriptor[d] * b[j].descriptor[d];
                }

                if (ncc > best_ncc) {
                    best_ncc = ncc;
                    best_j = j;
                }
            }

            // Stripping structural ambiguities statically
            if (best_ncc >= ncc_thresh) {
                Match m;
                m.queryIdx = i;
                m.trainIdx = best_j;
                m.distance = 1.0 - best_ncc; // Flipped mathematically validating logically smaller = better
                final_matches.push_back(m);
            }
        }

        return final_matches;
    }

    QImage drawMatches(const QImage& imgA, const QImage& imgB, const std::vector<SiftKeypoint>& kpsA, const std::vector<SiftKeypoint>& kpsB, const std::vector<Match>& matches) {
        // Expand the primary framework horizontally mirroring topological matrix bounds strictly mapped
        int total_width = imgA.width() + imgB.width();
        int max_height = std::max(imgA.height(), imgB.height());

        QImage result(total_width, max_height, QImage::Format_RGB32);
        result.fill(Qt::darkGray);

        QPainter painter(&result);
        
        painter.drawImage(0, 0, imgA);
        painter.drawImage(imgA.width(), 0, imgB);

        painter.setPen(QPen(QColor(0, 255, 0, 150), 1)); 

        for (const auto& m : matches) {
            const auto& ptA = kpsA[m.queryIdx];
            const auto& ptB = kpsB[m.trainIdx];

            QPointF pA(ptA.x, ptA.y);
            // Structurally shifting natively tracking horizontal bounds rigidly matching Image A's physical size
            QPointF pB(ptB.x + imgA.width(), ptB.y);

            painter.drawLine(pA, pB);
            
            painter.setPen(QPen(Qt::blue, 1));
            painter.drawEllipse(pA, 2, 2);
            painter.drawEllipse(pB, 2, 2);
            painter.setPen(QPen(QColor(0, 255, 0, 150), 1)); 
        }

        painter.end();
        return result;
    }

} // namespace feature
