#include "mainwindow.h"
#include "harris.h"
#include "sift.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QFileDialog>
#include <QSplitter>
#include <QElapsedTimer>
#include <QApplication>
#include <QGraphicsTextItem>
#include <QGraphicsEllipseItem>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle("Feature Detection Hub");
    setWindowState(Qt::WindowMaximized);
    setupUI();
}

MainWindow::~MainWindow() {}

void MainWindow::setupUI() {
    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    QHBoxLayout *mainLayout = new QHBoxLayout(centralWidget);

    // Sidebar
    QWidget *sidebar = new QWidget(this);
    sidebar->setFixedWidth(300);
    setupSidebar(sidebar);

    // Main Content
    QSplitter *mainSplitter = new QSplitter(Qt::Vertical);
    
    // Image Grid
    QWidget *gridLayoutWidget = new QWidget();
    QGridLayout *gridLayout = new QGridLayout(gridLayoutWidget);
    
    sceneA = new QGraphicsScene(this);
    sceneB = new QGraphicsScene(this);
    sceneOutput = new QGraphicsScene(this);
    
    viewA = new QGraphicsView(sceneA);
    viewB = new QGraphicsView(sceneB);
    viewOutput = new QGraphicsView(sceneOutput);
    
    gridLayout->addWidget(new QLabel("Input Image A"), 0, 0);
    gridLayout->addWidget(viewA, 1, 0);
    gridLayout->addWidget(new QLabel("Input Image B"), 0, 1);
    gridLayout->addWidget(viewB, 1, 1);
    gridLayout->addWidget(new QLabel("Output Result"), 0, 2);
    gridLayout->addWidget(viewOutput, 1, 2);

    // Terminal
    terminal = new QTextEdit();
    terminal->setReadOnly(true);
    // Setting background to dark terminal like
    terminal->setStyleSheet("background-color: black; color: #00FF00; font-family: Consolas;");

    mainSplitter->addWidget(gridLayoutWidget);
    mainSplitter->addWidget(terminal);
    mainSplitter->setStretchFactor(0, 3);
    mainSplitter->setStretchFactor(1, 1);

    mainLayout->addWidget(sidebar);
    mainLayout->addWidget(mainSplitter);

    logMessage("Application Started...");
}

void MainWindow::setupSidebar(QWidget* sidebar) {
    QVBoxLayout *layout = new QVBoxLayout(sidebar);

    QPushButton *btnUploadA = new QPushButton("Upload Image A");
    QPushButton *btnUploadB = new QPushButton("Upload Image B");
    
    connect(btnUploadA, &QPushButton::clicked, this, &MainWindow::uploadImageA);
    connect(btnUploadB, &QPushButton::clicked, this, &MainWindow::uploadImageB);

    layout->addWidget(btnUploadA);
    layout->addWidget(btnUploadB);
    layout->addSpacing(20);

    layout->addWidget(createHarrisGroup());
    layout->addWidget(createSIFTGroup());
    layout->addWidget(createMatchingGroup());
    layout->addStretch();
}

QGroupBox* MainWindow::createHarrisGroup() {
    QGroupBox *group = new QGroupBox("Harris Operator");
    QVBoxLayout *layout = new QVBoxLayout(group);

    harrisSigmaLabel = new QLabel("Gaussian Sigma: 1.0");
    harrisSigmaSlider = new QSlider(Qt::Horizontal);
    harrisSigmaSlider->setRange(1, 100);
    harrisSigmaSlider->setValue(10);
    connect(harrisSigmaSlider, &QSlider::valueChanged, [=](int v){ harrisSigmaLabel->setText(QString("Gaussian Sigma: %1").arg(v / 10.0)); });

    harrisThresholdLabel = new QLabel("Corner Threshold: 1000");
    harrisThresholdSlider = new QSlider(Qt::Horizontal);
    harrisThresholdSlider->setRange(10, 50000);
    harrisThresholdSlider->setValue(1000);
    connect(harrisThresholdSlider, &QSlider::valueChanged, [=](int v){ harrisThresholdLabel->setText(QString("Corner Threshold: %1").arg(v)); });

    QPushButton *btnApply = new QPushButton("Apply Harris");
    connect(btnApply, &QPushButton::clicked, this, &MainWindow::applyHarris);

    layout->addWidget(harrisSigmaLabel);
    layout->addWidget(harrisSigmaSlider);
    layout->addWidget(harrisThresholdLabel);
    layout->addWidget(harrisThresholdSlider);
    layout->addWidget(btnApply);
    
    return group;
}

QGroupBox* MainWindow::createSIFTGroup() {
    QGroupBox *group = new QGroupBox("SIFT Extractor");
    QVBoxLayout *layout = new QVBoxLayout(group);

    siftSigmaLabel = new QLabel("Initial Sigma: 1.6");
    siftSigmaSlider = new QSlider(Qt::Horizontal);
    siftSigmaSlider->setRange(1, 50);
    siftSigmaSlider->setValue(16);
    connect(siftSigmaSlider, &QSlider::valueChanged, [=](int v){ siftSigmaLabel->setText(QString("Initial Sigma: %1").arg(v / 10.0)); });

    siftScaleLabel = new QLabel("Scale Multiplier (s): 1.41");
    siftScaleSlider = new QSlider(Qt::Horizontal);
    siftScaleSlider->setRange(11, 20);
    siftScaleSlider->setValue(14);
    connect(siftScaleSlider, &QSlider::valueChanged, [=](int v){ siftScaleLabel->setText(QString("Scale Multiplier: %1").arg(v / 10.0)); });

    siftContrastLabel = new QLabel("Contrast Threshold: 0.03");
    siftContrastSlider = new QSlider(Qt::Horizontal);
    siftContrastSlider->setRange(1, 100);
    siftContrastSlider->setValue(3);
    connect(siftContrastSlider, &QSlider::valueChanged, [=](int v){ siftContrastLabel->setText(QString("Contrast Threshold: %1").arg(v / 100.0)); });

    QPushButton *btnApply = new QPushButton("Apply SIFT");
    connect(btnApply, &QPushButton::clicked, this, &MainWindow::applySIFT);

    layout->addWidget(siftSigmaLabel);
    layout->addWidget(siftSigmaSlider);
    layout->addWidget(siftScaleLabel);
    layout->addWidget(siftScaleSlider);
    layout->addWidget(siftContrastLabel);
    layout->addWidget(siftContrastSlider);
    layout->addWidget(btnApply);
    
    return group;
}

QGroupBox* MainWindow::createMatchingGroup() {
    QGroupBox *group = new QGroupBox("Feature Matcher");
    QVBoxLayout *layout = new QVBoxLayout(group);

    matcherCombo = new QComboBox();
    matcherCombo->addItem("SSD");
    matcherCombo->addItem("NCC");

    QPushButton *btnApply = new QPushButton("Match Features");
    connect(btnApply, &QPushButton::clicked, this, &MainWindow::applyMatching);

    layout->addWidget(new QLabel("Metric:"));
    layout->addWidget(matcherCombo);
    layout->addWidget(btnApply);

    return group;
}

void MainWindow::logMessage(const QString& msg) {
    terminal->append(msg);
}

void MainWindow::displayImage(QGraphicsScene* scene, const QImage& img) {
    scene->clear();
    scene->setBackgroundBrush(Qt::NoBrush);
    scene->addPixmap(QPixmap::fromImage(img));
    if (!scene->views().isEmpty()) {
        scene->views().first()->fitInView(scene->itemsBoundingRect(), Qt::KeepAspectRatio);
    }
}

void MainWindow::uploadImageA() {
    QString fileName = QFileDialog::getOpenFileName(this, "Open Image A", "", "Images (*.png *.jpg *.jpeg *.bmp *.webp)");
    if (!fileName.isEmpty()) {
        imgA.load(fileName);
        displayImage(sceneA, imgA);
        logMessage("Loaded Image A: " + fileName);
    }
}

void MainWindow::uploadImageB() {
    QString fileName = QFileDialog::getOpenFileName(this, "Open Image B", "", "Images (*.png *.jpg *.jpeg *.bmp *.webp)");
    if (!fileName.isEmpty()) {
        imgB.load(fileName);
        displayImage(sceneB, imgB);
        logMessage("Loaded Image B: " + fileName);
    }
}

void MainWindow::showLoader(const QString& taskName) {
    logMessage("> " + taskName + "...");
    logMessage("   Loading...");
    
    sceneOutput->clear();
    sceneOutput->setBackgroundBrush(Qt::darkGray);
    
    QGraphicsEllipseItem* circle = sceneOutput->addEllipse(0, 0, 100, 100, QPen(Qt::green, 4));
    QGraphicsTextItem* text = sceneOutput->addText("Working...");
    text->setDefaultTextColor(Qt::white);
    text->setPos(15, 38);
    
    if (!sceneOutput->views().isEmpty()) {
        sceneOutput->views().first()->resetTransform();
        sceneOutput->views().first()->centerOn(50, 50);
    }
    
    QApplication::processEvents();
}

void MainWindow::applyHarris() {
    if (imgA.isNull()) {
        logMessage("[Error] Please upload Image A first.");
        return;
    }

    showLoader("Running Harris Corner Detection");
    QElapsedTimer timer; timer.start();

    // 1. Get parameters
    double sigma = harrisSigmaSlider->value() / 10.0;
    double threshold = harrisThresholdSlider->value();

    // 2. Convert to matrix
    utils::Matrix2D mat = utils::QImageToGrayMatrix(imgA);

    // 3. Apply Harris Algorithm
    std::vector<feature::CornerPoint> corners = feature::detectHarrisCorners(mat, sigma, threshold);

    // 4. Draw result
    QImage resultImg = feature::drawCorners(imgA, corners);
    displayImage(sceneOutput, resultImg);

    int elapsed = timer.elapsed();
    logMessage(QString("   Detected %1 corners").arg(corners.size()));
    logMessage(QString("   Time taken: %1 ms").arg(elapsed));
}

void MainWindow::applySIFT() {
    if (imgA.isNull()) {
        logMessage("[Error] Please upload Image A first.");
        return;
    }

    showLoader("Running SIFT Feature Extraction");
    QElapsedTimer timer; timer.start();

    // 1. Get parameters
    double sigma0 = siftSigmaSlider->value() / 10.0;
    double scale = siftScaleSlider->value() / 10.0;
    double contrast = siftContrastSlider->value() / 100.0;

    // 2. Convert to matrix
    utils::Matrix2D mat = utils::QImageToGrayMatrix(imgA);

    // 3. Apply SIFT Extractor
    std::vector<feature::SiftKeypoint> kps = feature::extractSiftFeatures(mat, sigma0, scale, contrast);

    // 4. Draw result
    QImage resultImg = feature::drawSiftKeypoints(imgA, kps);
    displayImage(sceneOutput, resultImg);

    int elapsed = timer.elapsed();
    logMessage(QString("   Extracted %1 SIFT keypoints").arg(kps.size()));
    logMessage(QString("   Time taken: %1 ms").arg(elapsed));
}

void MainWindow::applyMatching() {
    showLoader("Running Feature Matcher (Dummy)");
    QElapsedTimer timer; timer.start();
    // TODO: Connect to matching.cpp
    logMessage(QString("   Time taken: %1 ms").arg(timer.elapsed()));
    sceneOutput->clear();
    sceneOutput->setBackgroundBrush(Qt::NoBrush);
}
