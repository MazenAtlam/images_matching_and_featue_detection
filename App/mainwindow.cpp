#include "mainwindow.h"
#include "harris.h"
#include "sift.h"
#include "matching.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QFileDialog>
#include <QSplitter>
#include <QElapsedTimer>
#include <QApplication>
#include <QGraphicsPixmapItem>

// Inject Spinner logic identically natively mimicking the specific loading aesthetic organically!
SpinnerItem::SpinnerItem(QGraphicsItem *parent) : QGraphicsObject(parent), angle(0) {
    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, [this](){
        angle = (angle + 10) % 360;
        update();
    });
    timer->start(20); // Smooth 50 FPS
}

QRectF SpinnerItem::boundingRect() const {
    return QRectF(-70, -70, 140, 140);
}

void SpinnerItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *) {
    painter->setRenderHint(QPainter::Antialiasing);
    int side = 80;
    QRectF rect(-side/2.0, -side/2.0, side, side);
    
    QPen pen(QColor(0, 255, 150), 6); 
    pen.setCapStyle(Qt::RoundCap);
    painter->setPen(pen);
    
    // Conic swept rotation effectively translating logic 
    painter->drawArc(rect, -angle * 16, 270 * 16);

    painter->setPen(Qt::white);
    QFont f = painter->font();
    f.setPixelSize(15);
    f.setBold(true);
    painter->setFont(f);
    painter->drawText(QRectF(-side, side/2.0 + 15.0, side*2.0, 30.0), Qt::AlignCenter, "Loading ...");
}

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

    // Sidebar explicitly fixed mathematically identically to bounds
    QWidget *sidebar = new QWidget(this);
    sidebar->setFixedWidth(400);
    setupSidebar(sidebar);

    // Main Content structured via tracking splitters matching the mockup precisely
    QSplitter *mainSplitter = new QSplitter(Qt::Vertical);
    
    // Top Half layout identically splitting Image A and Image B horizontally
    QSplitter *topSplitter = new QSplitter(Qt::Horizontal);
    
    sceneA = new QGraphicsScene(this);
    sceneB = new QGraphicsScene(this);
    sceneOutput = new QGraphicsScene(this);
    
    viewA = new QGraphicsView(sceneA);
    viewB = new QGraphicsView(sceneB);
    viewOutput = new QGraphicsView(sceneOutput);
    
    QWidget *panelA = new QWidget();
    QVBoxLayout *layoutA = new QVBoxLayout(panelA);
    QLabel* lblA = new QLabel("Image A");
    lblA->setAlignment(Qt::AlignCenter);
    layoutA->addWidget(lblA);
    layoutA->addWidget(viewA);
    
    QWidget *panelB = new QWidget();
    QVBoxLayout *layoutB = new QVBoxLayout(panelB);
    QLabel* lblB = new QLabel("Image B");
    lblB->setAlignment(Qt::AlignCenter);
    layoutB->addWidget(lblB);
    layoutB->addWidget(viewB);
    
    topSplitter->addWidget(panelA);
    topSplitter->addWidget(panelB);
    topSplitter->setStretchFactor(0, 1);
    topSplitter->setStretchFactor(1, 1);

    // Bottom Output Result linearly mapped below visually
    QWidget *bottomPanel = new QWidget();
    QVBoxLayout *bottomLayout = new QVBoxLayout(bottomPanel);
    QLabel* lblOut = new QLabel("Output Result");
    lblOut->setAlignment(Qt::AlignCenter);
    bottomLayout->addWidget(lblOut);
    bottomLayout->addWidget(viewOutput);

    mainSplitter->addWidget(topSplitter);
    mainSplitter->addWidget(bottomPanel);
    mainSplitter->setStretchFactor(0, 5);
    mainSplitter->setStretchFactor(1, 5);

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
    layout->addSpacing(10);

    layout->addWidget(createHarrisGroup());
    layout->addWidget(createSIFTMatcherGroup());

    // Placing Terminal specifically natively tracking inside the sidebar layout algebraically matching the mockup!
    terminal = new QTextEdit();
    terminal->setReadOnly(true);
    // Setting background to tracking terminal matrix visually flawlessly
    terminal->setStyleSheet("background-color: black; color: #00FF00; font-family: Consolas;");
    
    layout->addWidget(terminal, 1); // Native stretch 1
}

QGroupBox* MainWindow::createHarrisGroup() {
    QGroupBox *group = new QGroupBox("Harris Operator");
    QVBoxLayout *layout = new QVBoxLayout(group);

    harrisSigmaLabel = new QLabel("Gaussian Sigma: 1.0");
    harrisSigmaSlider = new QSlider(Qt::Horizontal);
    harrisSigmaSlider->setRange(5, 50); // Maps reliably 0.5 to 5.0
    harrisSigmaSlider->setValue(10);
    connect(harrisSigmaSlider, &QSlider::valueChanged, [=](int v){ harrisSigmaLabel->setText(QString("Gaussian Sigma: %1").arg(v / 10.0)); });

    harrisThresholdLabel = new QLabel("Corner Threshold: 1000");
    harrisThresholdSlider = new QSlider(Qt::Horizontal);
    harrisThresholdSlider->setRange(100, 100000); // Dynamically maps optimal linear determinant bounds
    harrisThresholdSlider->setValue(1000);
    connect(harrisThresholdSlider, &QSlider::valueChanged, [=](int v){ harrisThresholdLabel->setText(QString("Corner Threshold: %1").arg(v)); });

    QPushButton *btnApply = new QPushButton("Apply Harris on Image A");
    connect(btnApply, &QPushButton::clicked, this, &MainWindow::applyHarris);

    layout->addWidget(harrisSigmaLabel);
    layout->addWidget(harrisSigmaSlider);
    layout->addWidget(harrisThresholdLabel);
    layout->addWidget(harrisThresholdSlider);
    layout->addWidget(btnApply);
    
    return group;
}

QGroupBox* MainWindow::createSIFTMatcherGroup() {
    QGroupBox *group = new QGroupBox("SIFT Extractor and Feature Matcher");
    QVBoxLayout *layout = new QVBoxLayout(group);

    siftSigmaLabel = new QLabel("Initial Sigma: 1.6");
    siftSigmaSlider = new QSlider(Qt::Horizontal);
    siftSigmaSlider->setRange(5, 30); // Structurally bounds natively mapping 0.5 to 3.0 smoothly
    siftSigmaSlider->setValue(16);
    connect(siftSigmaSlider, &QSlider::valueChanged, [=](int v){ siftSigmaLabel->setText(QString("Initial Sigma: %1").arg(v / 10.0)); });

    siftIntervalsLabel = new QLabel("Intervals per Octave (s): 3");
    siftIntervalsSlider = new QSlider(Qt::Horizontal);
    siftIntervalsSlider->setRange(1, 6); // Optimal scaling bounds cleanly dynamically explicitly natively
    siftIntervalsSlider->setValue(3);
    connect(siftIntervalsSlider, &QSlider::valueChanged, [=](int v){ siftIntervalsLabel->setText(QString("Intervals per Octave (s): %1").arg(v)); });

    siftContrastLabel = new QLabel("Contrast Threshold: 0.030");
    siftContrastSlider = new QSlider(Qt::Horizontal);
    siftContrastSlider->setRange(5, 200); // Safely rigorously maps 0.005 to 0.200 (Default Lowe bounds 0.03) natively
    siftContrastSlider->setValue(30);
    connect(siftContrastSlider, &QSlider::valueChanged, [=](int v){ siftContrastLabel->setText(QString("Contrast Threshold: %1").arg(v / 1000.0, 0, 'f', 3)); });

    ssdThresholdLabel = new QLabel("SSD Ratio Threshold: 0.80");
    ssdThresholdSlider = new QSlider(Qt::Horizontal);
    ssdThresholdSlider->setRange(10, 100);
    ssdThresholdSlider->setValue(80);
    connect(ssdThresholdSlider, &QSlider::valueChanged, [=](int v){ ssdThresholdLabel->setText(QString("SSD Ratio Threshold: %1").arg(v / 100.0, 0, 'f', 2)); });

    nccThresholdLabel = new QLabel("NCC Threshold: 0.85");
    nccThresholdSlider = new QSlider(Qt::Horizontal);
    nccThresholdSlider->setRange(10, 100);
    nccThresholdSlider->setValue(85);
    connect(nccThresholdSlider, &QSlider::valueChanged, [=](int v){ nccThresholdLabel->setText(QString("NCC Threshold: %1").arg(v / 100.0, 0, 'f', 2)); });

    QPushButton *btnApplySIFT = new QPushButton("Apply SIFT on Image A");
    connect(btnApplySIFT, &QPushButton::clicked, this, &MainWindow::applySIFT);
    
    QPushButton *btnMatchSSD = new QPushButton("Match Features with SSD Metric");
    connect(btnMatchSSD, &QPushButton::clicked, this, &MainWindow::applyMatchingSSD);
    
    QPushButton *btnMatchNCC = new QPushButton("Match Features with NCC Metric");
    connect(btnMatchNCC, &QPushButton::clicked, this, &MainWindow::applyMatchingNCC);

    layout->addWidget(siftSigmaLabel);
    layout->addWidget(siftSigmaSlider);
    layout->addWidget(siftIntervalsLabel);
    layout->addWidget(siftIntervalsSlider);
    layout->addWidget(siftContrastLabel);
    layout->addWidget(siftContrastSlider);
    layout->addWidget(btnApplySIFT);

    layout->addWidget(ssdThresholdLabel);
    layout->addWidget(ssdThresholdSlider);
    layout->addWidget(btnMatchSSD);

    layout->addWidget(nccThresholdLabel);
    layout->addWidget(nccThresholdSlider);
    layout->addWidget(btnMatchNCC);
    
    return group;
}

void MainWindow::logMessage(const QString& msg) {
    terminal->append(msg);
}

void MainWindow::displayImage(QGraphicsScene* scene, const QImage& img) {
    scene->clear();
    scene->setBackgroundBrush(Qt::NoBrush);
    QGraphicsPixmapItem* pixmapItem = scene->addPixmap(QPixmap::fromImage(img));
    
    // Critically explicitly reset structural boundary limits exactly tracking identical image geometric widths properly organically natively globally algebraically testing!
    scene->setSceneRect(pixmapItem->boundingRect());
    
    if (!scene->views().isEmpty()) {
        scene->views().first()->resetTransform();
        scene->views().first()->fitInView(scene->itemsBoundingRect(), Qt::KeepAspectRatio);
        scene->views().first()->centerOn(pixmapItem);
    }
}

void MainWindow::uploadImageA() {
    QString fileName = QFileDialog::getOpenFileName(this, "Open Image A", "", "Images (*.png *.jpg *.jpeg *.bmp *.webp)");
    if (!fileName.isEmpty()) {
        showLoader("", sceneA); // Trigger visually loading bounds instantly specifically matching instructions
        QTimer::singleShot(50, this, [=]() {
            imgA.load(fileName);
            displayImage(sceneA, imgA);
            logMessage("Loaded Image A: " + fileName);
        });
    }
}

void MainWindow::uploadImageB() {
    QString fileName = QFileDialog::getOpenFileName(this, "Open Image B", "", "Images (*.png *.jpg *.jpeg *.bmp *.webp)");
    if (!fileName.isEmpty()) {
        showLoader("", sceneB); // Explicitly activating right-side spinner
        QTimer::singleShot(50, this, [=]() {
            imgB.load(fileName);
            displayImage(sceneB, imgB);
            logMessage("Loaded Image B: " + fileName);
        });
    }
}

void MainWindow::showLoader(const QString& taskName, QGraphicsScene* targetScene) {
    if (!taskName.isEmpty()) {
        logMessage("> " + taskName + "...");
    }
    
    targetScene->clear();
    // Native sleek aesthetics logically bridging cleanly!
    targetScene->setBackgroundBrush(QColor(30, 30, 30));
    
    SpinnerItem* spinner = new SpinnerItem();
    targetScene->addItem(spinner);
    spinner->setPos(0, 0);
    
    // Critically explicitly shrink the structural scene bounds mapping strictly to the spinner natively!
    // Without this, the scene physically remembers the old massive image boundaries logically offsetting everything.
    targetScene->setSceneRect(spinner->boundingRect());
    
    if (!targetScene->views().isEmpty()) {
        targetScene->views().first()->resetTransform();
        targetScene->views().first()->centerOn(0, 0);
    }
}

void MainWindow::applyHarris() {
    if (imgA.isNull()) {
        logMessage("[Error] Please upload Image A first.");
        return;
    }

    showLoader("Running Harris Corner Detection", sceneOutput);

    QTimer::singleShot(50, this, [=]() {
        QElapsedTimer timer; timer.start();

        double sigma = harrisSigmaSlider->value() / 10.0;
        double threshold = harrisThresholdSlider->value();

        utils::Matrix2D mat = utils::QImageToGrayMatrix(imgA);
        std::vector<feature::CornerPoint> corners = feature::detectHarrisCorners(mat, sigma, threshold);
        QImage resultImg = feature::drawCorners(imgA, corners);

        displayImage(sceneOutput, resultImg);
        logMessage(QString("   Detected %1 corners").arg(corners.size()));
        logMessage(QString("   Time taken: %1 ms").arg(timer.elapsed()));
    });
}

void MainWindow::applySIFT() {
    if (imgA.isNull()) {
        logMessage("[Error] Please upload Image A first.");
        return;
    }

    showLoader("Running SIFT Feature Extraction", sceneOutput);

    QTimer::singleShot(50, this, [=]() {
        QElapsedTimer timer; timer.start();

        double sigma0 = siftSigmaSlider->value() / 10.0;
        int num_intervals = siftIntervalsSlider->value();
        double contrast = siftContrastSlider->value() / 1000.0;

        utils::Matrix2D mat = utils::QImageToGrayMatrix(imgA);
        std::vector<feature::SiftKeypoint> kps = feature::extractSiftFeatures(mat, sigma0, num_intervals, contrast);
        QImage resultImg = feature::drawSiftKeypoints(imgA, kps);

        displayImage(sceneOutput, resultImg);
        logMessage(QString("   Extracted %1 SIFT keypoints").arg(kps.size()));
        logMessage(QString("   Time taken: %1 ms").arg(timer.elapsed()));
    });
}

void MainWindow::applyMatchingSSD() {
    executeMatching("SSD");
}

void MainWindow::applyMatchingNCC() {
    executeMatching("NCC");
}

void MainWindow::executeMatching(const QString& metric) {
    if (imgA.isNull() || imgB.isNull()) {
        logMessage("[Error] Please upload both Image A and Image B first.");
        return;
    }

    showLoader("Running Feature Matcher", sceneOutput);
    
    QTimer::singleShot(50, this, [=]() {
        QElapsedTimer timer; timer.start();

        double sigma0 = siftSigmaSlider->value() / 10.0;
        int num_intervals = siftIntervalsSlider->value();
        double contrast = siftContrastSlider->value() / 1000.0;

        utils::Matrix2D matA = utils::QImageToGrayMatrix(imgA);
        utils::Matrix2D matB = utils::QImageToGrayMatrix(imgB);

        logMessage("   Extracting features natively for Image A...");
        std::vector<feature::SiftKeypoint> kpsA = feature::extractSiftFeatures(matA, sigma0, num_intervals, contrast);
        
        logMessage("   Extracting features natively for Image B...");
        std::vector<feature::SiftKeypoint> kpsB = feature::extractSiftFeatures(matB, sigma0, num_intervals, contrast);

        std::vector<feature::Match> matches;

        if (metric == "SSD") {
            double ssd_thresh = ssdThresholdSlider->value() / 100.0;
            logMessage("   Matching matrices via SSD (Ratio Test)...");
            matches = feature::matchFeaturesSSD(kpsA, kpsB, ssd_thresh);
        } else {
            double ncc_thresh = nccThresholdSlider->value() / 100.0;
            logMessage("   Matching matrices via NCC (Dot Product)...");
            matches = feature::matchFeaturesNCC(kpsA, kpsB, ncc_thresh);
        }

        QImage resultImg = feature::drawMatches(imgA, imgB, kpsA, kpsB, matches);
        displayImage(sceneOutput, resultImg);

        logMessage(QString("   Matched %1 features successfully!").arg(matches.size()));
        logMessage(QString("   Time taken: %1 ms").arg(timer.elapsed()));
    });
}
