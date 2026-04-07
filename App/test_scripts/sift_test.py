import sys
import cv2
import numpy as np
import time
from PyQt6.QtWidgets import (QApplication, QMainWindow, QWidget, QVBoxLayout, 
                             QHBoxLayout, QPushButton, QLabel, QSlider, QFileDialog, 
                             QTextEdit, QSplitter)
from PyQt6.QtGui import QImage, QPixmap
from PyQt6.QtCore import Qt

class SIFTTestApp(QMainWindow):
    def __init__(self):
        super().__init__()
        self.setWindowTitle("SIFT Extractor Test (Python/OpenCV)")
        self.resize(1000, 700)
        
        self.original_image = None
        self.gray_image = None
        self.current_display = None
        
        self.initUI()
        
    def initUI(self):
        central_widget = QWidget()
        self.setCentralWidget(central_widget)
        main_layout = QHBoxLayout(central_widget)
        
        # Left Sidebar (Controls)
        sidebar = QVBoxLayout()
        sidebar_widget = QWidget()
        sidebar_widget.setLayout(sidebar)
        sidebar_widget.setFixedWidth(280)
        
        self.btn_upload = QPushButton("Upload Image")
        self.btn_upload.clicked.connect(self.upload_image)
        sidebar.addWidget(self.btn_upload)
        
        sidebar.addSpacing(20)
        sidebar.addWidget(QLabel("Task 2: SIFT Extractor"))
        
        # Initial Sigma slider
        self.lbl_sigma = QLabel("Initial Sigma: 1.6")
        sidebar.addWidget(self.lbl_sigma)
        self.slider_sigma = QSlider(Qt.Orientation.Horizontal)
        self.slider_sigma.setRange(1, 50)
        self.slider_sigma.setValue(16)
        self.slider_sigma.valueChanged.connect(self.update_labels)
        sidebar.addWidget(self.slider_sigma)
        
        # Intervals per Octave slider
        self.lbl_intervals = QLabel("Intervals per Octave (s): 3")
        sidebar.addWidget(self.lbl_intervals)
        self.slider_intervals = QSlider(Qt.Orientation.Horizontal)
        self.slider_intervals.setRange(1, 10)
        self.slider_intervals.setValue(3)
        self.slider_intervals.valueChanged.connect(self.update_labels)
        sidebar.addWidget(self.slider_intervals)
        
        # Contrast Threshold slider
        self.lbl_contrast = QLabel("Contrast Threshold: 0.03")
        sidebar.addWidget(self.lbl_contrast)
        self.slider_contrast = QSlider(Qt.Orientation.Horizontal)
        self.slider_contrast.setRange(1, 100)
        self.slider_contrast.setValue(3)
        self.slider_contrast.valueChanged.connect(self.update_labels)
        sidebar.addWidget(self.slider_contrast)
        
        self.btn_apply = QPushButton("Apply SIFT")
        self.btn_apply.clicked.connect(self.apply_sift)
        sidebar.addWidget(self.btn_apply)
        
        sidebar.addStretch()
        
        # Right Splitter (Image + Terminal)
        self.right_splitter = QSplitter(Qt.Orientation.Vertical)
        
        # Image Display Label
        self.image_label = QLabel("Upload an image to see SIFT Keypoints")
        self.image_label.setAlignment(Qt.AlignmentFlag.AlignCenter)
        self.image_label.setStyleSheet("border: 1px solid black; background-color: #222; color: white;")
        
        # Terminal Log
        self.terminal = QTextEdit()
        self.terminal.setReadOnly(True)
        self.terminal.setStyleSheet("background-color: black; color: #00FF00; font-family: Consolas;")
        
        self.right_splitter.addWidget(self.image_label)
        self.right_splitter.addWidget(self.terminal)
        self.right_splitter.setStretchFactor(0, 4)
        self.right_splitter.setStretchFactor(1, 1)
        
        main_layout.addWidget(sidebar_widget)
        main_layout.addWidget(self.right_splitter)
        
        self.log_message("SIFT Application Started...")
        
    def log_message(self, msg):
        self.terminal.append(msg)
        
    def upload_image(self):
        file_name, _ = QFileDialog.getOpenFileName(self, "Open Image", "", "Images (*.png *.jpg *.jpeg *.bmp *.webp)")
        if file_name:
            self.original_image = cv2.imread(file_name)
            self.gray_image = cv2.cvtColor(self.original_image, cv2.COLOR_BGR2GRAY)
            self.current_display = self.original_image.copy()
            self.display_image()
            self.log_message(f"Loaded Image: {file_name}")
            
    def update_labels(self):
        self.lbl_sigma.setText(f"Initial Sigma: {self.slider_sigma.value() / 10.0}")
        self.lbl_intervals.setText(f"Intervals per Octave (s): {self.slider_intervals.value()}")
        self.lbl_contrast.setText(f"Contrast Threshold: {self.slider_contrast.value() / 100.0}")
            
    def apply_sift(self):
        if self.original_image is None or self.gray_image is None:
             self.log_message("[Error] Please upload an image first.")
             return
            
        self.log_message("> Running SIFT Feature Extraction...")
        start_time = time.time()
            
        sigma = self.slider_sigma.value() / 10.0
        num_intervals = self.slider_intervals.value()
        contrast = self.slider_contrast.value() / 100.0
        
        # Our C++ app now aligns perfectly with OpenCV's parameter methodology mathematically.
        # We can map our input natively perfectly into nOctaveLayers.
        n_octave_layers = num_intervals
            
        # Initialize OpenCV's built-in SIFT matching parameters
        sift = cv2.SIFT_create(
            nfeatures=0, 
            nOctaveLayers=n_octave_layers, 
            contrastThreshold=contrast, 
            edgeThreshold=10, 
            sigma=sigma
        )
        
        # Extract SIFT features & local 128-d descriptors mapping
        keypoints, _ = sift.detectAndCompute(self.gray_image, None)
        
        # Draw Results visually
        # using cv2.DRAW_MATCHES_FLAGS_DRAW_RICH_KEYPOINTS automatically scales the circle
        # to the size equivalent of the octave sigma and renders the orientation angle line flawlessly
        result_img = cv2.drawKeypoints(self.original_image, keypoints, None, 
                                       flags=cv2.DRAW_MATCHES_FLAGS_DRAW_RICH_KEYPOINTS)
        
        self.current_display = result_img
        self.display_image()
        
        elapsed_ms = int((time.time() - start_time) * 1000)
        num_kps = len(keypoints) if keypoints is not None else 0
        self.log_message(f"   Extracted {num_kps} SIFT keypoints")
        self.log_message(f"   Time taken: {elapsed_ms} ms")
        
    def display_image(self):
        if self.current_display is None:
            return
        
        rgb_image = cv2.cvtColor(self.current_display, cv2.COLOR_BGR2RGB)
        h, w, ch = rgb_image.shape
        bytes_per_line = ch * w
        
        qt_img = QImage(rgb_image.data, w, h, bytes_per_line, QImage.Format.Format_RGB888)
        pixmap = QPixmap.fromImage(qt_img)
        
        pixmap = pixmap.scaled(self.image_label.width(), self.image_label.height(), 
                               Qt.AspectRatioMode.KeepAspectRatio, 
                               Qt.TransformationMode.SmoothTransformation)
                               
        self.image_label.setPixmap(pixmap)
        
    def resizeEvent(self, event):
        if self.current_display is not None:
            self.display_image()
        super().resizeEvent(event)

if __name__ == '__main__':
    app = QApplication(sys.argv)
    window = SIFTTestApp()
    window.show()
    sys.exit(app.exec())
