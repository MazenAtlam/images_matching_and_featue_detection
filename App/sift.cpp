#include "sift.h"
#include <cmath>
#include <algorithm>
#include <QPainter>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace feature {

    std::vector<SiftKeypoint> extractSiftFeatures(const utils::Matrix2D& image, double initial_sigma, int num_intervals, double contrast_thresh) {
        // Strict adherence to David Lowe's scale space calculation parameters
        int num_scales = num_intervals + 3;
        double scale_mult = std::pow(2.0, 1.0 / static_cast<double>(num_intervals));
        int num_octaves = 4; // 4 Octaves: 2x (Octave 0), 1x (Octave 1), 0.5x (Octave 2), 0.25x (Octave 3)

        std::vector<SiftKeypoint> keypoints;
        double r_thresh = 10.0;
        double edge_ratio = (r_thresh + 1.0) * (r_thresh + 1.0) / r_thresh;

        // Base image initially holds an assumed inherent camera blur of ~0.5
        double current_intrinsic_sigma = 0.5;

        // Initial Image Doubling (Octave 0 uses a 2x interpolated native space map to capture high frequency SIFT keys)
        utils::Matrix2D current_base = utils::upsampleByDouble(image);
        // By doubling the physical pixels, the blur functionally spans twice as many pixels radially
        current_intrinsic_sigma *= 2.0;
        
        // OpenCV scales contrast threshold by octave layers for absolute normalized response
        double actual_contrast_thresh = (contrast_thresh * 255.0) / static_cast<double>(num_intervals);

        for (int octave = 0; octave < num_octaves; ++octave) {
            int rows = current_base.rows;
            int cols = current_base.cols;

            // 1. Generate Scale Space (Local Layer pyramid per octave)
            std::vector<utils::Matrix2D> L;
            std::vector<double> sigmas(num_scales);
            
            for (int k = 0; k < num_scales; ++k) {
                // Determine absolute targets relative to this specific octave
                sigmas[k] = initial_sigma * std::pow(scale_mult, k);
            }
            
            // Baseline execution blur for the foundation layer tracking intrinsic state
            if (sigmas[0] > current_intrinsic_sigma) {
                double delta_sigma = std::sqrt(sigmas[0] * sigmas[0] - current_intrinsic_sigma * current_intrinsic_sigma);
                L.push_back(utils::applyGaussianBlur(current_base, delta_sigma));
                current_intrinsic_sigma = sigmas[0];
            } else {
                L.push_back(current_base);
                current_intrinsic_sigma = sigmas[0]; // Treat it practically functionally equivalent
            }
            
            // Build subsequent layers incrementally safely without exploding delta convolutions
            for (int k = 1; k < num_scales; ++k) {
                double delta_sigma = std::sqrt(sigmas[k] * sigmas[k] - current_intrinsic_sigma * current_intrinsic_sigma);
                L.push_back(utils::applyGaussianBlur(L[k - 1], delta_sigma));
                current_intrinsic_sigma = sigmas[k];
            }

            // 2. Generate Difference of Gaussians (DoG) physically bounding matching blocks
            std::vector<utils::Matrix2D> DoG;
            for (int k = 0; k < num_scales - 1; ++k) {
                utils::Matrix2D dog_layer(rows, cols, 0.0);
                for (int y = 0; y < rows; ++y) {
                    for (int x = 0; x < cols; ++x) {
                        dog_layer.at(y, x) = L[k + 1].data[y * cols + x] - L[k].data[y * cols + x];
                    }
                }
                DoG.push_back(dog_layer);
            }

            // 3. Extrema Detection recursively evaluating the active grid
            for (int k = 1; k < DoG.size() - 1; ++k) {
                for (int y = 1; y < rows - 1; ++y) {
                    for (int x = 1; x < cols - 1; ++x) {
                        double val = DoG[k].data[y * cols + x];

                        // Rigid Low contrast rejection matching initial bounds (fast pass)
                        if (std::abs(val) < 0.5 * actual_contrast_thresh) continue; 

                        bool isMax = true;
                        bool isMin = true;

                        for (int dk = -1; dk <= 1; ++dk) {
                            for (int dy = -1; dy <= 1; ++dy) {
                                for (int dx = -1; dx <= 1; ++dx) {
                                    if (dk == 0 && dy == 0 && dx == 0) continue;
                                    double n_val = DoG[k + dk].data[(y + dy) * cols + (x + dx)];
                                    if (val <= n_val) isMax = false;
                                    if (val >= n_val) isMin = false;
                                }
                            }
                        }

                        if (isMax || isMin) {
                            // 3D Taylor Series Subpixel Peak interpolation!
                            int iter = 0;
                            bool converged = false;
                            
                            int cx = x;
                            int cy = y;
                            int ck = k;
                            
                            double dx_off=0, dy_off=0, ds_off=0;
                            
                            while (iter < 5) {
                                if (ck < 1 || ck >= DoG.size() - 1 || cy < 1 || cy >= rows - 1 || cx < 1 || cx >= cols - 1) {
                                    break; // Drifted out of valid octave boundary bounds
                                }
                                
                                // 1st derivative Gradient (x, y, sigma) Note: normalized to step size = 1
                                double grad_x = (DoG[ck].data[cy * cols + (cx+1)] - DoG[ck].data[cy * cols + (cx-1)]) / 2.0;
                                double grad_y = (DoG[ck].data[(cy+1) * cols + cx] - DoG[ck].data[(cy-1) * cols + cx]) / 2.0;
                                double grad_s = (DoG[ck+1].data[cy * cols + cx] - DoG[ck-1].data[cy * cols + cx]) / 2.0;

                                // 2nd derivative Hessian (3x3)
                                double v = DoG[ck].data[cy * cols + cx];
                                double dxx = DoG[ck].data[cy * cols + (cx+1)] + DoG[ck].data[cy * cols + (cx-1)] - 2.0 * v;
                                double dyy = DoG[ck].data[(cy+1) * cols + cx] + DoG[ck].data[(cy-1) * cols + cx] - 2.0 * v;
                                double dss = DoG[ck+1].data[cy * cols + cx] + DoG[ck-1].data[cy * cols + cx] - 2.0 * v;
                                
                                double dxy = (DoG[ck].data[(cy+1) * cols + (cx+1)] - DoG[ck].data[(cy+1) * cols + (cx-1)] -
                                              DoG[ck].data[(cy-1) * cols + (cx+1)] + DoG[ck].data[(cy-1) * cols + (cx-1)]) / 4.0;
                                              
                                double dxs = (DoG[ck+1].data[cy * cols + (cx+1)] - DoG[ck+1].data[cy * cols + (cx-1)] -
                                              DoG[ck-1].data[cy * cols + (cx+1)] + DoG[ck-1].data[cy * cols + (cx-1)]) / 4.0;
                                              
                                double dys = (DoG[ck+1].data[(cy+1) * cols + cx] - DoG[ck+1].data[(cy-1) * cols + cx] -
                                              DoG[ck-1].data[(cy+1) * cols + cx] + DoG[ck-1].data[(cy-1) * cols + cx]) / 4.0;

                                double H[3][3] = {
                                    {dxx, dxy, dxs},
                                    {dxy, dyy, dys},
                                    {dxs, dys, dss}
                                };
                                double H_inv[3][3];
                                
                                if (!utils::invert3x3(H, H_inv)) {
                                    break; // Singular matrix
                                }
                                
                                // Solve H_inv * Gradient
                                dx_off = -(H_inv[0][0]*grad_x + H_inv[0][1]*grad_y + H_inv[0][2]*grad_s);
                                dy_off = -(H_inv[1][0]*grad_x + H_inv[1][1]*grad_y + H_inv[1][2]*grad_s);
                                ds_off = -(H_inv[2][0]*grad_x + H_inv[2][1]*grad_y + H_inv[2][2]*grad_s);

                                if (std::abs(dx_off) < 0.5 && std::abs(dy_off) < 0.5 && std::abs(ds_off) < 0.5) {
                                    converged = true;
                                    break; // Found accurate peak
                                }
                                
                                // Shift integer coordinate to closest neighbor peak
                                cx += std::round(dx_off);
                                cy += std::round(dy_off);
                                ck += std::round(ds_off);
                                
                                iter++;
                            }

                            if (!converged) continue;
                            if (ck < 1 || ck >= DoG.size() - 1 || cy < 1 || cy >= rows - 1 || cx < 1 || cx >= cols - 1) continue;

                            // Check Interpolated Contrast Threshold  D(x^) = D + 0.5 * dot(grad, offset)
                            double v = DoG[ck].data[cy * cols + cx];
                            double grad_x = (DoG[ck].data[cy * cols + (cx+1)] - DoG[ck].data[cy * cols + (cx-1)]) / 2.0;
                            double grad_y = (DoG[ck].data[(cy+1) * cols + cx] - DoG[ck].data[(cy-1) * cols + cx]) / 2.0;
                            double grad_s = (DoG[ck+1].data[cy * cols + cx] - DoG[ck-1].data[cy * cols + cx]) / 2.0;
                            
                            double d_adjusted = v + 0.5 * (grad_x * dx_off + grad_y * dy_off + grad_s * ds_off);
                            
                            if (std::abs(d_adjusted) < actual_contrast_thresh) continue;

                            double dxx = DoG[ck].data[cy * cols + (cx+1)] + DoG[ck].data[cy * cols + (cx-1)] - 2.0 * v;
                            double dyy = DoG[ck].data[(cy+1) * cols + cx] + DoG[ck].data[(cy-1) * cols + cx] - 2.0 * v;
                            double dxy = (DoG[ck].data[(cy+1) * cols + (cx+1)] - DoG[ck].data[(cy+1) * cols + (cx-1)] -
                                          DoG[ck].data[(cy-1) * cols + (cx+1)] + DoG[ck].data[(cy-1) * cols + (cx-1)]) / 4.0;
                            
                            double tr = dxx + dyy;
                            double det = dxx * dyy - dxy * dxy;

                            if (det <= 0 || (tr * tr) / det >= edge_ratio) continue;

                            double local_sigma = initial_sigma * std::pow(scale_mult, ck + ds_off);
                            double coord_mult = std::pow(2.0, octave - 1.0);
                            
                            double map_x = (cx + dx_off) * coord_mult;
                            double map_y = (cy + dy_off) * coord_mult;
                            
                            SiftKeypoint kp(std::round(map_x), std::round(map_y), local_sigma * coord_mult, 0.0);
                            
                            int o_radius = std::max(1, (int)std::round(4.5 * local_sigma)); 
                            double hist[36] = {0.0};
                            
                            for (int oy = -o_radius; oy <= o_radius; ++oy) {
                                for (int ox = -o_radius; ox <= o_radius; ++ox) {
                                    int ny = cy + oy;
                                    int nx = cx + ox;
                                    if (ny > 0 && ny < rows - 1 && nx > 0 && nx < cols - 1) {
                                        double m_dx = L[ck].data[ny * cols + (nx+1)] - L[ck].data[ny * cols + (nx-1)];
                                        double m_dy = L[ck].data[(ny+1) * cols + nx] - L[ck].data[(ny-1) * cols + nx];
                                        
                                        double mag = std::sqrt(m_dx*m_dx + m_dy*m_dy);
                                        double theta = std::atan2(m_dy, m_dx) * 180.0 / M_PI;
                                        while (theta < 0) theta += 360.0;
                                        while (theta >= 360.0) theta -= 360.0;
                                        
                                        double w = std::exp(-(ox*ox + oy*oy) / (2.0 * 1.5 * local_sigma * 1.5 * local_sigma));
                                        
                                        int bin = std::round(theta / 10.0);
                                        if (bin >= 36) bin = 0;
                                        hist[bin] += w * mag;
                                    }
                                }
                            }
                            
                            double max_peak = 0.0;
                            int max_bin = 0;
                            for (int b = 0; b < 36; ++b) {
                                if (hist[b] > max_peak) {
                                    max_peak = hist[b];
                                    max_bin = b;
                                }
                            }
                            kp.orientation = max_bin * 10.0;
                            
                            kp.descriptor.resize(128, 0.0);
                            double cos_t = std::cos(kp.orientation * M_PI / 180.0);
                            double sin_t = std::sin(kp.orientation * M_PI / 180.0);
                            
                            bool valid_descriptor = true;
                            
                            for (int i = -8; i < 8; ++i) {
                                for (int j = -8; j < 8; ++j) {
                                    int rx = std::round(j * cos_t - i * sin_t);
                                    int ry = std::round(j * sin_t + i * cos_t);
                                    
                                    int snx = cx + rx;
                                    int sny = cy + ry;
                                    
                                    if (snx > 0 && snx < cols - 1 && sny > 0 && sny < rows - 1) {
                                        double g_dx = L[ck].data[sny * cols + (snx+1)] - L[ck].data[sny * cols + (snx-1)];
                                        double g_dy = L[ck].data[(sny+1) * cols + snx] - L[ck].data[(sny-1) * cols + snx];
                                        double mag = std::sqrt(g_dx*g_dx + g_dy*g_dy);
                                        double theta = std::atan2(g_dy, g_dx) * 180.0 / M_PI - kp.orientation;
                                        while (theta < 0) theta += 360.0;
                                        while (theta >= 360.0) theta -= 360.0;
                                        
                                        int sub_r = (i + 8) / 4; 
                                        int sub_c = (j + 8) / 4; 
                                        int sub_idx = sub_r * 4 + sub_c;
                                        
                                        int bin = std::round(theta / 45.0);
                                        if (bin < 0) bin = 0;
                                        if (bin >= 8) bin = 0;
                                        
                                        double w = std::exp(-(i*i + j*j) / (2.0 * 8.0 * 8.0)); 
                                        kp.descriptor[sub_idx * 8 + bin] += mag * w;
                                    } else {
                                        valid_descriptor = false;
                                    }
                                }
                            }
                            
                            if (valid_descriptor) {
                                double sq_sum = 0.0;
                                for (double d : kp.descriptor) sq_sum += d * d;
                                sq_sum = std::sqrt(sq_sum);
                                if (sq_sum > 1e-6) {
                                    for (double& d : kp.descriptor) {
                                        d /= sq_sum;
                                        if (d > 0.2) d = 0.2; 
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
            
            // Subsample rigidly locked to Lowe's true octave interval tracking definitions
            if (octave < num_octaves - 1) {
                // By SIFT definition, the layer specifically indexed at 'num_intervals' 
                // contains exactly 2.0x the variance of the base layer.
                utils::Matrix2D safe_base = L[num_intervals];
                
                // Track intrinsic sigma accurately mapped to that layer
                current_intrinsic_sigma = sigmas[num_intervals];

                // Once we physically halve the resolution, the pixel spacing doubles, meaning 
                // the effective blur radius relative to the physical pixels gets cut cleanly in half.
                current_base = utils::subsampleByHalf(safe_base);
                current_intrinsic_sigma /= 2.0;
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
            // Rendering utilizing dynamically assigned relative global sigma coordinate mapping
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
