#include "utils.h"

#define M_PI 3.14159265358979323846

namespace utils {

    Matrix2D QImageToGrayMatrix(const QImage& img) {
        Matrix2D mat(img.height(), img.width());
        for (int y = 0; y < img.height(); ++y) {
            for (int x = 0; x < img.width(); ++x) {
                QRgb pixel = img.pixel(x, y);
                // standard grayscale conversion
                double gray = 0.299 * qRed(pixel) + 0.587 * qGreen(pixel) + 0.114 * qBlue(pixel);
                mat.at(y, x) = gray;
            }
        }
        return mat;
    }

    QImage MatrixToQImage(const Matrix2D& mat) {
        QImage img(mat.cols, mat.rows, QImage::Format_Grayscale8);
        for (int y = 0; y < mat.rows; ++y) {
            for (int x = 0; x < mat.cols; ++x) {
                double val = mat.at(y, x);
                if (val < 0.0) val = 0.0;
                if (val > 255.0) val = 255.0;
                img.setPixel(x, y, qRgb(val, val, val));
            }
        }
        return img;
    }

    Matrix2D padMatrix(const Matrix2D& mat, int pad, double pad_val) {
        Matrix2D padded(mat.rows + 2 * pad, mat.cols + 2 * pad, pad_val);
        for (int y = 0; y < mat.rows; ++y) {
            for (int x = 0; x < mat.cols; ++x) {
                padded.at(y + pad, x + pad) = mat.at(y, x);
            }
        }
        return padded;
    }

    Matrix2D convolve2D(const Matrix2D& input, const Matrix2D& kernel) {
        int kSize = kernel.rows;
        int pad = kSize / 2;
        Matrix2D padded = padMatrix(input, pad);
        Matrix2D output(input.rows, input.cols, 0.0);

        for (int y = 0; y < input.rows; ++y) {
            for (int x = 0; x < input.cols; ++x) {
                double sum = 0.0;
                for (int ky = 0; ky < kSize; ++ky) {
                    for (int kx = 0; kx < kSize; ++kx) {
                        sum += padded.at(y + ky, x + kx) * kernel.at(ky, kx);
                    }
                }
                output.at(y, x) = sum;
            }
        }
        return output;
    }

    Matrix2D getGaussianKernel(double sigma) {
        if (sigma <= 0.0) sigma = 0.0001;
        // typically size is 2 * ceil(3 * sigma) + 1
        int hc = std::ceil(3.0 * sigma);
        int size = 2 * hc + 1;
        Matrix2D kernel(size, size);
        double sum = 0.0;

        for (int y = -hc; y <= hc; ++y) {
            for (int x = -hc; x <= hc; ++x) {
                double val = std::exp(-(x * x + y * y) / (2.0 * sigma * sigma)) / (2.0 * M_PI * sigma * sigma);
                kernel.at(y + hc, x + hc) = val;
                sum += val;
            }
        }
        // Normalize
        for (int y = 0; y < size; ++y) {
            for (int x = 0; x < size; ++x) {
                kernel.at(y, x) /= sum;
            }
        }
        return kernel;
    }

    Matrix2D applyGaussianBlur(const Matrix2D& input, double sigma) {
        if (sigma <= 0.0) return input;
        Matrix2D kernel = getGaussianKernel(sigma);
        return convolve2D(input, kernel);
    }

    void computeGradients(const Matrix2D& input, Matrix2D& Ix, Matrix2D& Iy) {
        Matrix2D kx(3, 3);
        kx.at(0,0)=-1; kx.at(0,1)=0; kx.at(0,2)=1;
        kx.at(1,0)=-2; kx.at(1,1)=0; kx.at(1,2)=2;
        kx.at(2,0)=-1; kx.at(2,1)=0; kx.at(2,2)=1;

        Matrix2D ky(3, 3);
        ky.at(0,0)=-1; ky.at(0,1)=-2; ky.at(0,2)=-1;
        ky.at(1,0)= 0; ky.at(1,1)= 0; ky.at(1,2)= 0;
        ky.at(2,0)= 1; ky.at(2,1)= 2; ky.at(2,2)= 1;

        Ix = convolve2D(input, kx);
        Iy = convolve2D(input, ky);
    }

} // namespace utils
