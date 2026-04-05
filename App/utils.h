#ifndef UTILS_H
#define UTILS_H

#include <vector>
#include <cmath>
#include <QImage>

namespace utils {

    // Custom 2D Matrix structure for double precision values
    struct Matrix2D {
        int rows;
        int cols;
        std::vector<double> data;

        Matrix2D(int r = 0, int c = 0, double init_val = 0.0) 
            : rows(r), cols(c), data(r * c, init_val) {}

        double& at(int r, int c) {
            return data[r * cols + c];
        }

        const double& at(int r, int c) const {
            return data[r * cols + c];
        }
    };

    // Conversions
    Matrix2D QImageToGrayMatrix(const QImage& img);
    QImage MatrixToQImage(const Matrix2D& mat);

    // Padding
    Matrix2D padMatrix(const Matrix2D& mat, int pad, double pad_val = 0.0);

    // Convolutions
    Matrix2D convolve2D(const Matrix2D& input, const Matrix2D& kernel);

    // Gaussian Blurring
    Matrix2D getGaussianKernel(double sigma);
    Matrix2D applyGaussianBlur(const Matrix2D& input, double sigma);

    // Gradients
    void computeGradients(const Matrix2D& input, Matrix2D& Ix, Matrix2D& Iy);

} // namespace utils

#endif // UTILS_H
