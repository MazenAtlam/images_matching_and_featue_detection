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

    std::vector<double> get1DGaussianKernel(double sigma) {
        if (sigma <= 0.0) sigma = 0.0001;
        int hc = std::ceil(3.0 * sigma);
        int size = 2 * hc + 1;
        std::vector<double> kernel(size);
        double sum = 0.0;

        for (int x = -hc; x <= hc; ++x) {
            double val = std::exp(-(x * x) / (2.0 * sigma * sigma));
            kernel[x + hc] = val;
            sum += val;
        }
        // Normalize
        for (int i = 0; i < size; ++i) kernel[i] /= sum;
        
        return kernel;
    }

    Matrix2D applyGaussianBlur(const Matrix2D& input, double sigma) {
        if (sigma <= 0.0) return input;
        
        // Using separable 1D Gaussian kernels for exponential speedup
        // O(W * H * 2K) instead of O(W * H * K^2)
        std::vector<double> kernel = get1DGaussianKernel(sigma);
        int kSize = kernel.size();
        int pad = kSize / 2;
        
        int rows = input.rows;
        int cols = input.cols;

        // Pass 1: Horizontal Blur
        Matrix2D temp(rows, cols, 0.0);
        for (int y = 0; y < rows; ++y) {
            for (int x = 0; x < cols; ++x) {
                double sum = 0.0;
                for (int kx = -pad; kx <= pad; ++kx) {
                    int px = x + kx;
                    // Edge clamping
                    if (px < 0) px = 0;
                    if (px >= cols) px = cols - 1;
                    
                    sum += input.at(y, px) * kernel[kx + pad];
                }
                temp.at(y, x) = sum;
            }
        }

        // Pass 2: Vertical Blur
        Matrix2D output(rows, cols, 0.0);
        for (int y = 0; y < rows; ++y) {
            for (int x = 0; x < cols; ++x) {
                double sum = 0.0;
                for (int ky = -pad; ky <= pad; ++ky) {
                    int py = y + ky;
                    // Edge clamping
                    if (py < 0) py = 0;
                    if (py >= rows) py = rows - 1;
                    
                    sum += temp.at(py, x) * kernel[ky + pad];
                }
                output.at(y, x) = sum;
            }
        }
        
        return output;
    }

    Matrix2D subsampleByHalf(const Matrix2D& input) {
        int r = input.rows / 2;
        int c = input.cols / 2;
        Matrix2D out(r, c, 0.0);
        for(int y = 0; y < r; ++y) {
            for(int x = 0; x < c; ++x) {
                out.at(y, x) = input.data[(y * 2) * input.cols + (x * 2)];
            }
        }
        return out;
    }

    Matrix2D upsampleByDouble(const Matrix2D& input) {
        int r = input.rows * 2;
        int c = input.cols * 2;
        Matrix2D out(r, c, 0.0);
        
        for(int y = 0; y < r; ++y) {
            double src_y = y / 2.0;
            int y1 = (int)std::floor(src_y);
            int y2 = std::min(y1 + 1, input.rows - 1);
            double dy = src_y - y1;
            
            for(int x = 0; x < c; ++x) {
                double src_x = x / 2.0;
                int x1 = (int)std::floor(src_x);
                int x2 = std::min(x1 + 1, input.cols - 1);
                double dx = src_x - x1;
                
                double v1 = input.data[y1 * input.cols + x1];
                double v2 = input.data[y1 * input.cols + x2];
                double v3 = input.data[y2 * input.cols + x1];
                double v4 = input.data[y2 * input.cols + x2];
                
                double top_interp = v1 * (1.0 - dx) + v2 * dx;
                double bot_interp = v3 * (1.0 - dx) + v4 * dx;
                
                out.at(y, x) = top_interp * (1.0 - dy) + bot_interp * dy;
            }
        }
        return out;
    }

    bool invert3x3(const double m[3][3], double inv[3][3]) {
        double det = m[0][0] * (m[1][1] * m[2][2] - m[2][1] * m[1][2]) -
                     m[0][1] * (m[1][0] * m[2][2] - m[1][2] * m[2][0]) +
                     m[0][2] * (m[1][0] * m[2][1] - m[1][1] * m[2][0]);
                     
        if (std::abs(det) < 1e-8) return false; // Matrix is singular mathematically
        
        double invdet = 1.0 / det;
        
        inv[0][0] = (m[1][1] * m[2][2] - m[2][1] * m[1][2]) * invdet;
        inv[0][1] = (m[0][2] * m[2][1] - m[0][1] * m[2][2]) * invdet;
        inv[0][2] = (m[0][1] * m[1][2] - m[0][2] * m[1][1]) * invdet;
        
        inv[1][0] = (m[1][2] * m[2][0] - m[1][0] * m[2][2]) * invdet;
        inv[1][1] = (m[0][0] * m[2][2] - m[0][2] * m[2][0]) * invdet;
        inv[1][2] = (m[1][0] * m[0][2] - m[0][0] * m[1][2]) * invdet;
        
        inv[2][0] = (m[1][0] * m[2][1] - m[2][0] * m[1][1]) * invdet;
        inv[2][1] = (m[2][0] * m[0][1] - m[0][0] * m[2][1]) * invdet;
        inv[2][2] = (m[0][0] * m[1][1] - m[1][0] * m[0][1]) * invdet;
        
        return true;
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
