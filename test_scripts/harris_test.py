import sys
import cv2
import numpy as np
import time
from PyQt6.QtWidgets import (QApplication, QMainWindow, QWidget, QVBoxLayout, 
                             QHBoxLayout, QPushButton, QLabel, QSlider, QFileDialog, 
                             QTextEdit, QSplitter)
from PyQt6.QtGui import QImage, QPixmap
from PyQt6.QtCore import Qt

class HarrisTestApp(QMainWindow):
    def __init__(self):
        super().__init__()
        self.setWindowTitle("Harris Operator Test (Python/OpenCV)")
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
        sidebar_widget.setFixedWidth(250)
        
        self.btn_upload = QPushButton("Upload Image")
        self.btn_upload.clicked.connect(self.upload_image)
        sidebar.addWidget(self.btn_upload)
        
        sidebar.addSpacing(20)
        sidebar.addWidget(QLabel("Task 1: Harris Operator"))
        
        # Gaussian Sigma slider
        self.lbl_sigma = QLabel("Gaussian Sigma: 1.0")
        sidebar.addWidget(self.lbl_sigma)
        self.slider_sigma = QSlider(Qt.Orientation.Horizontal)
        self.slider_sigma.setRange(1, 100)
        self.slider_sigma.setValue(10)
        self.slider_sigma.valueChanged.connect(self.update_labels)
        sidebar.addWidget(self.slider_sigma)
        
        # Threshold slider
        self.lbl_thresh = QLabel("Corner Threshold: 1000")
        sidebar.addWidget(self.lbl_thresh)
        self.slider_thresh = QSlider(Qt.Orientation.Horizontal)
        self.slider_thresh.setRange(10, 50000)
        self.slider_thresh.setValue(1000)
        self.slider_thresh.valueChanged.connect(self.update_labels)
        sidebar.addWidget(self.slider_thresh)
        
        self.btn_apply = QPushButton("Apply Harris")
        self.btn_apply.clicked.connect(self.apply_harris)
        sidebar.addWidget(self.btn_apply)
        
        sidebar.addStretch()
        
        # Right Splitter (Image + Terminal)
        self.right_splitter = QSplitter(Qt.Orientation.Vertical)
        
        # Image Display Label
        self.image_label = QLabel("Upload an image to see corners")
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
        
        self.log_message("Application Started...")
        
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
        self.lbl_sigma.setText(f"Gaussian Sigma: {self.slider_sigma.value() / 10.0}")
        self.lbl_thresh.setText(f"Corner Threshold: {self.slider_thresh.value()}")
            
    def apply_harris(self):
        if self.original_image is None or self.gray_image is None:
             self.log_message("[Error] Please upload an image first.")
             return
            
        self.log_message("> Running Harris Corner Detection...")
        start_time = time.time()
            
        sigma = self.slider_sigma.value() / 10.0
        threshold = self.slider_thresh.value()
        
        # 1. Gradients
        Ix = cv2.Sobel(self.gray_image, cv2.CV_64F, 1, 0, ksize=3)
        Iy = cv2.Sobel(self.gray_image, cv2.CV_64F, 0, 1, ksize=3)
        
        # 2. Computing a, b, c
        a = Ix * Ix
        b = Ix * Iy
        c = Iy * Iy
        
        # 3. Gaussian Blur built-in (ksize=0 auto-calculates from sigma)
        a_smooth = cv2.GaussianBlur(a, (0, 0), sigmaX=sigma, sigmaY=sigma)
        b_smooth = cv2.GaussianBlur(b, (0, 0), sigmaX=sigma, sigmaY=sigma)
        c_smooth = cv2.GaussianBlur(c, (0, 0), sigmaX=sigma, sigmaY=sigma)
        
        # 4. Calculate exact Minimum Eigenvalue (Lambda -)
        trace = a_smooth + c_smooth
        lambda_minus = 0.5 * (trace - np.sqrt(b_smooth*b_smooth + (a_smooth - c_smooth)**2))
        
        # 5. Non-Maximal Suppression & Thresholding
        threshold_mask = lambda_minus > threshold
        local_max = cv2.dilate(lambda_minus, np.ones((3,3)))
        nms_mask = (lambda_minus == local_max) & threshold_mask
        
        # 6. Draw Red circles on the result
        result_img = self.original_image.copy()
        keypoints = np.argwhere(nms_mask)
        
        for pt in keypoints:
            y, x = pt
            cv2.circle(result_img, (x, y), 2, (0, 0, 255), 1)
            
        self.current_display = result_img
        self.display_image()
        
        elapsed_ms = int((time.time() - start_time) * 1000)
        self.log_message(f"   Detected {len(keypoints)} corners")
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
    window = HarrisTestApp()
    window.show()
    sys.exit(app.exec())
