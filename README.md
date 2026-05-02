# Images Matching and Feature Detection

A comprehensive desktop application developed in C++ and Qt for computer vision feature detection and matching. This project natively implements core computer vision algorithms from scratch, bypassing external libraries for the core mathematical pipelines to provide an educational and highly optimized feature extraction toolkit.

## Features

- **Harris Corner Detection**: Identifies corners in images using the Harris operator with adjustable Gaussian Sigma and threshold parameters. Includes custom Gaussian blur and non-maximal suppression algorithms.

- **SIFT Feature Extraction**: Implements Scale-Invariant Feature Transform (SIFT) to extract reliable 128-dimensional descriptors. Features an interactive octave and contrast threshold tuning pipeline.

- **Feature Matching**: Matches extracted features between two images using two distinct mathematical metrics:

  - **Sum of Squared Differences (SSD)** utilizing Lowe's Ratio Test for accurate filtering.

  - **Normalized Cross-Correlation (NCC)** utilizing vector dot products.

- **Interactive GUI**: A Qt-based desktop interface to upload images, adjust algorithm parameters dynamically, and view visual overlays of detected features and matching lines.

- **Python Reference Implementations**: OpenCV-based Python scripts provided for testing and validation against the native C++ implementations.

## Prerequisites

### C++ Application (Main Project)

- **Compiler**: C++17 compatible compiler (e.g., MinGW/GCC with `-mavx2` support, Clang, or MSVC).

- **Build System**: CMake (version 3.16 or higher).

- **Framework**: Qt 5 or Qt 6 (Requires `Core`, `Gui`, and `Widgets` components).

### Python Test Scripts (Optional)

To run the reference OpenCV test scripts located in `App/test_scripts/`:

- Python 3.x
- `opencv-python`
- `PyQt6`
- `numpy`

## Quick Start

### Windows (Automated Build)

If you are on Windows and using MinGW with Qt, you can use the provided batch script.

1. Navigate to the `App/` directory.
2. Create a `.env` file in the `App/` directory with the following variables configured to your local Qt installation:

    ```env
    QT_PREFIX_PATH=C:/Qt/6.x.x/mingw_64
    WINDEPLOYQT_PATH=C:/Qt/6.x.x/mingw_64/bin/windeployqt.exe
    ```

3. Run the build script:

   ```cmd
   build_and_run.bat
   ```

   *This script will automatically configure CMake, build the executable with Release optimizations, deploy the required Qt DLLs, and launch the application.*

### Manual CMake Build (Cross-Platform)

1. Open a terminal and navigate to the `App/` directory.
2. Create a build directory and navigate into it:

   ```bash
   mkdir build && cd build
   ```

3. Run CMake configuration (ensure `CMAKE_PREFIX_PATH` is set if Qt is not in your system path):

   ```bash
   cmake .. -DCMAKE_BUILD_TYPE=Release
   ```

4. Compile the project:

   ```bash
   cmake --build .
   ```

5. Run the executable generated in the build folder (`FeatureDetectionApp` or `FeatureDetectionApp.exe`).

## Usage

1. Launch the application.
2. Click **Upload Image A** to load your base image.
3. For Harris Corner Detection or SIFT Extraction, adjust the parameters in the left sidebar and click the respective "Apply" button to view the results in the bottom panel.
4. For Feature Matching, click **Upload Image B** to load the target image. Select your preferred matching metric (SSD or NCC) and click the matching button to visualize the connections between the two images.

## License

This project is licensed under the MIT License. See [LICENSE](LICENSE) for details.
