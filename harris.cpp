#include "harris.h"
#include <cmath>
#include <QPainter>

namespace feature {

    std::vector<CornerPoint> detectHarrisCorners(const utils::Matrix2D& image, double sigma, double threshold) {
        // 1. Compute gradients Ix and Iy using Sobel
        utils::Matrix2D Ix, Iy;
        utils::computeGradients(image, Ix, Iy);

        int rows = image.rows;
        int cols = image.cols;

        // 2. Compute a, b, c for each pixel
        utils::Matrix2D a(rows, cols); // Ix^2
        utils::Matrix2D b(rows, cols); // Ix * Iy
        utils::Matrix2D c(rows, cols); // Iy^2

        for (int y = 0; y < rows; ++y) {
            for (int x = 0; x < cols; ++x) {
                double ix = Ix.at(y, x);
                double iy = Iy.at(y, x);
                a.at(y, x) = ix * ix;
                b.at(y, x) = ix * iy;
                c.at(y, x) = iy * iy;
            }
        }

        // 3. Apply Gaussian Blur to smooth the window
        utils::Matrix2D a_smooth = utils::applyGaussianBlur(a, sigma);
        utils::Matrix2D b_smooth = utils::applyGaussianBlur(b, sigma);
        utils::Matrix2D c_smooth = utils::applyGaussianBlur(c, sigma);

        // 4. Calculate Minimum Eigenvalue (lambda_minus) for every pixel
        utils::Matrix2D lambda_minus(rows, cols, 0.0);
        
        for (int y = 0; y < rows; ++y) {
            for (int x = 0; x < cols; ++x) {
                double a_val = a_smooth.at(y, x);
                double b_val = b_smooth.at(y, x);
                double c_val = c_smooth.at(y, x);

                double trace = a_val + c_val;
                // Using the requested formula: lambda2 = 1/2 [a+c - sqrt(b^2 + (a-c)^2)]
                double lambda2 = 0.5 * (trace - std::sqrt(b_val * b_val + (a_val - c_val) * (a_val - c_val)));
                lambda_minus.at(y, x) = lambda2;
            }
        }

        // 5. Non-Maximal Suppression & Thresholding
        std::vector<CornerPoint> corners;
        int radius = 1; // 3x3 local maximum
        for (int y = radius; y < rows - radius; ++y) {
            for (int x = radius; x < cols - radius; ++x) {
                double val = lambda_minus.at(y, x);
                if (val > threshold) {
                    bool isLocalMax = true;
                    // Check neighborhood
                    for (int ky = -radius; ky <= radius; ++ky) {
                        for (int kx = -radius; kx <= radius; ++kx) {
                            if (ky == 0 && kx == 0) continue;
                            if (lambda_minus.at(y + ky, x + kx) >= val) {
                                isLocalMax = false;
                                break;
                            }
                        }
                        if (!isLocalMax) break;
                    }
                    if (isLocalMax) {
                        corners.push_back(CornerPoint(x, y, val));
                    }
                }
            }
        }

        return corners;
    }

    QImage drawCorners(const QImage& originalImg, const std::vector<CornerPoint>& corners) {
        QImage result = originalImg.copy();
        if (result.format() != QImage::Format_RGB32 && result.format() != QImage::Format_ARGB32) {
            result = result.convertToFormat(QImage::Format_RGB32);
        }

        QPainter painter(&result);
        painter.setPen(QPen(Qt::red, 2)); // Red circle
        for (const auto& pt : corners) {
            painter.drawEllipse(QPoint(pt.x, pt.y), 2, 2);
        }
        painter.end();

        return result;
    }

} // namespace feature
