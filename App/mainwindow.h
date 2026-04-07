#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QTextEdit>
#include <QGroupBox>
#include <QPushButton>
#include <QSlider>
#include <QComboBox>
#include <QLabel>
#include <QImage>

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void uploadImageA();
    void uploadImageB();
    void applyHarris();
    void applySIFT();
    void applyMatching();
    void logMessage(const QString& msg);

private:
    // UI Setup methods
    void setupUI();
    void setupSidebar(QWidget* sidebar);
    QGroupBox* createHarrisGroup();
    QGroupBox* createSIFTGroup();
    QGroupBox* createMatchingGroup();
    void displayImage(QGraphicsScene* scene, const QImage& img);
    void showLoader(const QString& taskName);

    // Scenes
    QGraphicsScene *sceneA, *sceneB, *sceneOutput;
    QGraphicsView *viewA, *viewB, *viewOutput;

    // Terminal
    QTextEdit *terminal;

    // Images
    QImage imgA, imgB;

    // UI Elements
    // Harris
    QSlider *harrisSigmaSlider;
    QSlider *harrisThresholdSlider;
    QLabel *harrisSigmaLabel;
    QLabel *harrisThresholdLabel;

    // SIFT
    QSlider *siftSigmaSlider;
    QSlider *siftIntervalsSlider;
    QSlider *siftContrastSlider;
    QLabel *siftSigmaLabel;
    QLabel *siftIntervalsLabel;
    QLabel *siftContrastLabel;

    // Matcher
    QComboBox *matcherCombo;
};
#endif // MAINWINDOW_H
