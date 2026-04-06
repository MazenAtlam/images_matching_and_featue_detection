#include "sift.h"
#include <cmath>
#include <algorithm>
#include <QPainter>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace feature {

    std::vector<SiftKeypoint> extractSiftFeatures(const utils::Matrix2D& image, double initial_sigma, double scale_mult, double contrast_thresh) {
        int rows = image.rows;
        int cols = image.cols;
        int num_scales = 5;

        // 1. Generate Scale Space (1 Octave representation generating enough DoG levels)
        std::vector<utils::Matrix2D> L;
        for (int k = 0; k < num_scales; ++k) {
            double sigma_k = initial_sigma * std::pow(scale_mult, k);
            L.push_back(utils::applyGaussianBlur(image, sigma_k));
        }

        // 2. Generate Difference of Gaussians (DoG)
        std::vector<utils::Matrix2D> DoG;
        for (int k = 0; k < num_scales - 1; ++k) {
            utils::Matrix2D dog_layer(rows, cols, 0.0);
            for (int y = 0; y < rows; ++y) {
                for (int x = 0; x < cols; ++x) {
                    dog_layer.at(y, x) = L[k + 1].at(y, x) - L[k].at(y, x);
                }
            }
            DoG.push_back(dog_layer);
        }

        std::vector<SiftKeypoint> keypoints;
        double r_thresh = 10.0;
        double edge_ratio = (r_thresh + 1.0) * (r_thresh + 1.0) / r_thresh;

        // 3. Extrema Detection (3x3x3 neighborhood)
        for (int k = 1; k < DoG.size() - 1; ++k) {
            for (int y = 1; y < rows - 1; ++y) {
                for (int x = 1; x < cols - 1; ++x) {
                    double val = DoG[k].at(y, x);

                    // Low contrast rejection (using standard mapping for 255.0 intensity)
                    if (std::abs(val) < contrast_thresh * 255.0) continue; 

                    bool isMax = true;
                    bool isMin = true;

                    // 26 surrounding neighbors across 3 scales
                    for (int dk = -1; dk <= 1; ++dk) {
                        for (int dy = -1; dy <= 1; ++dy) {
                            for (int dx = -1; dx <= 1; ++dx) {
                                if (dk == 0 && dy == 0 && dx == 0) continue;
                                double n_val = DoG[k + dk].at(y + dy, x + dx);
                                if (val <= n_val) isMax = false;
                                if (val >= n_val) isMin = false;
                            }
                        }
                    }

                    if (isMax || isMin) {
                        // Edge response rejection parsing through Hessian trace over determinant
                        double dx = (DoG[k].at(y, x+1) - DoG[k].at(y, x-1)) / 2.0;
                        double dy = (DoG[k].at(y+1, x) - DoG[k].at(y-1, x)) / 2.0;
                        double dxx = DoG[k].at(y, x+1) + DoG[k].at(y, x-1) - 2.0 * val;
                        double dyy = DoG[k].at(y+1, x) + DoG[k].at(y-1, x) - 2.0 * val;
                        double dxy = (DoG[k].at(y+1, x+1) - DoG[k].at(y+1, x-1) - DoG[k].at(y-1, x+1) + DoG[k].at(y-1, x-1)) / 4.0;

                        double tr = dxx + dyy;
                        double det = dxx * dyy - dxy * dxy;

                        if (det > 0 && (tr * tr) / det < edge_ratio) {
                            
                            SiftKeypoint kp(x, y, initial_sigma * std::pow(scale_mult, k), 0.0);
                            
                            // 4. Orientation Assignment
                            int o_radius = std::round(4.5 * kp.sigma); // Neighborhood radius typically 3 * 1.5 * sigma
                            double hist[36] = {0.0};
                            
                            for (int oy = -o_radius; oy <= o_radius; ++oy) {
                                for (int ox = -o_radius; ox <= o_radius; ++ox) {
                                    int ny = y + oy;
                                    int nx = x + ox;
                                    if (ny > 0 && ny < rows - 1 && nx > 0 && nx < cols - 1) {
                                        double m_dx = L[k].at(ny, nx+1) - L[k].at(ny, nx-1);
                                        double m_dy = L[k].at(ny+1, nx) - L[k].at(ny-1, nx);
                                        
                                        double mag = std::sqrt(m_dx*m_dx + m_dy*m_dy);
                                        double theta = std::atan2(m_dy, m_dx) * 180.0 / M_PI;
                                        if (theta < 0) theta += 360.0;
                                        
                                        double w = std::exp(-(ox*ox + oy*oy) / (2.0 * 1.5 * kp.sigma * 1.5 * kp.sigma));
                                        
                                        int bin = std::round(theta / 10.0);
                                        if (bin >= 36) bin = 0;
                                        hist[bin] += w * mag;
                                    }
                                }
                            }
                            
                            // Discover the peak orientation bin
                            double max_peak = 0.0;
                            int max_bin = 0;
                            for (int b = 0; b < 36; ++b) {
                                if (hist[b] > max_peak) {
                                    max_peak = hist[b];
                                    max_bin = b;
                                }
                            }
                            kp.orientation = max_bin * 10.0;
                            
                            // 5. 128-d Descriptor Feature Extractor Logic
                            kp.descriptor.resize(128, 0.0);
                            double cos_t = std::cos(kp.orientation * M_PI / 180.0);
                            double sin_t = std::sin(kp.orientation * M_PI / 180.0);
                            
                            bool valid_descriptor = true;
                            
                            for (int i = -8; i < 8; ++i) {
                                for (int j = -8; j < 8; ++j) {
                                    // Cartesian transformation utilizing rotation mappings
                                    int rx = std::round(j * cos_t - i * sin_t);
                                    int ry = std::round(j * sin_t + i * cos_t);
                                    
                                    int snx = x + rx;
                                    int sny = y + ry;
                                    
                                    if (snx > 0 && snx < cols - 1 && sny > 0 && sny < rows - 1) {
                                        double g_dx = L[k].at(sny, snx+1) - L[k].at(sny, snx-1);
                                        double g_dy = L[k].at(sny+1, snx) - L[k].at(sny-1, snx);
                                        double mag = std::sqrt(g_dx*g_dx + g_dy*g_dy);
                                        double theta = std::atan2(g_dy, g_dx) * 180.0 / M_PI - kp.orientation;
                                        while (theta < 0) theta += 360.0;
                                        while (theta >= 360.0) theta -= 360.0;
                                        
                                        // Find corresponding 4x4 subregion block bin index (total 16 elements)
                                        int sub_r = (i + 8) / 4; 
                                        int sub_c = (j + 8) / 4; 
                                        int sub_idx = sub_r * 4 + sub_c;
                                        
                                        // Interpolate 8-direction orientations
                                        int bin = std::round(theta / 45.0);
                                        if (bin < 0) bin = 0;
                                        if (bin >= 8) bin = 0;
                                        
                                        double w = std::exp(-(i*i + j*j) / (2.0 * 8.0 * 8.0)); 
                                        kp.descriptor[sub_idx * 8 + bin] += mag * w;
                                    } else {
                                        // Keypoint extends past bounds of our canvas, rejecting entirely
                                        valid_descriptor = false;
                                    }
                                }
                            }
                            
                            if (valid_descriptor) {
                                // Light adjustment and dual-L2 normalization loop
                                double sq_sum = 0.0;
                                for (double d : kp.descriptor) sq_sum += d * d;
                                sq_sum = std::sqrt(sq_sum);
                                if (sq_sum > 1e-6) {
                                    for (double& d : kp.descriptor) {
                                        d /= sq_sum;
                                        if (d > 0.2) d = 0.2; // Robust cutoff
                                    }
                                }
                                
                                sq_sum = 0.0;
                                for (double d : kp.descriptor) sq_sum += d * d;
                                sq_sum = std::sqrt(sq_sum);
                                if (sq_sum > 1e-6) {
                                    for (double& d : kp.descriptor) d /= sq_sum;
                                }
                                
                                keypoints.push_back(kp);
                            }
                        }
                    }
                }
            }
        }

        return keypoints;
    }

    QImage drawSiftKeypoints(const QImage& originalImg, const std::vector<SiftKeypoint>& keypoints) {
        QImage result = originalImg.copy();
        if (result.format() != QImage::Format_RGB32 && result.format() != QImage::Format_ARGB32) {
            result = result.convertToFormat(QImage::Format_RGB32);
        }

        QPainter painter(&result);
        for (const auto& kp : keypoints) {
            painter.setPen(QPen(Qt::blue, 1));
            // Radius scales inherently with detected target scale
            int radius = std::max(3, (int)std::round(kp.sigma * 1.5));
            painter.drawEllipse(QPoint(kp.x, kp.y), radius, radius);
            
            painter.setPen(QPen(Qt::green, 1));
            int ex = kp.x + radius * std::cos(kp.orientation * M_PI / 180.0);
            int ey = kp.y + radius * std::sin(kp.orientation * M_PI / 180.0);
            painter.drawLine(kp.x, kp.y, ex, ey);
        }
        painter.end();

        return result;
    }

} // namespace feature
