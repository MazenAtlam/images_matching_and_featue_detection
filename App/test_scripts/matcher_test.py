import sys
import cv2
import numpy as np
import time
from PyQt6.QtWidgets import (QApplication, QMainWindow, QWidget, QVBoxLayout, 
                             QHBoxLayout, QPushButton, QLabel, QSlider, QFileDialog, 
                             QTextEdit, QSplitter, QComboBox)
from PyQt6.QtGui import QImage, QPixmap
from PyQt6.QtCore import Qt

class MatcherTestApp(QMainWindow):
    def __init__(self):
        super().__init__()
        self.setWindowTitle("Feature Matcher Test (Python/OpenCV)")
        self.resize(1100, 750)
        
        self.imgA_original = None
        self.imgA_gray = None
        
        self.imgB_original = None
        self.imgB_gray = None
        
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
        
        self.btn_upload_a = QPushButton("Upload Image A")
        self.btn_upload_a.clicked.connect(self.upload_image_a)
        sidebar.addWidget(self.btn_upload_a)
        
        self.btn_upload_b = QPushButton("Upload Image B")
        self.btn_upload_b.clicked.connect(self.upload_image_b)
        sidebar.addWidget(self.btn_upload_b)
        
        sidebar.addSpacing(20)
        sidebar.addWidget(QLabel("Task 3: Feature Matcher Parameters"))
        
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
        
        sidebar.addSpacing(10)
        self.lbl_metric = QLabel("Matching Metric:")
        sidebar.addWidget(self.lbl_metric)
        self.combo_metric = QComboBox()
        self.combo_metric.addItems(["SSD (Ratio Test)", "NCC (Dot Product)"])
        sidebar.addWidget(self.combo_metric)
        
        sidebar.addSpacing(10)
        self.btn_apply = QPushButton("Apply Matching")
        self.btn_apply.clicked.connect(self.apply_matching)
        sidebar.addWidget(self.btn_apply)
        
        sidebar.addStretch()
        
        # Right Splitter (Image + Terminal)
        self.right_splitter = QSplitter(Qt.Orientation.Vertical)
        
        # Image Display Label
        self.image_label = QLabel("Upload Image A and Image B to test matching")
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
        
        self.log_message("Matcher Application Started...")
        
    def log_message(self, msg):
        self.terminal.append(msg)
        
    def upload_image_a(self):
        file_name, _ = QFileDialog.getOpenFileName(self, "Open Image A", "", "Images (*.png *.jpg *.jpeg *.bmp *.webp)")
        if file_name:
            self.imgA_original = cv2.imread(file_name)
            self.imgA_gray = cv2.cvtColor(self.imgA_original, cv2.COLOR_BGR2GRAY)
            self.log_message(f"Loaded Image A: {file_name}")
            
    def upload_image_b(self):
        file_name, _ = QFileDialog.getOpenFileName(self, "Open Image B", "", "Images (*.png *.jpg *.jpeg *.bmp *.webp)")
        if file_name:
            self.imgB_original = cv2.imread(file_name)
            self.imgB_gray = cv2.cvtColor(self.imgB_original, cv2.COLOR_BGR2GRAY)
            self.log_message(f"Loaded Image B: {file_name}")
            
    def update_labels(self):
        self.lbl_sigma.setText(f"Initial Sigma: {self.slider_sigma.value() / 10.0}")
        self.lbl_intervals.setText(f"Intervals per Octave (s): {self.slider_intervals.value()}")
        self.lbl_contrast.setText(f"Contrast Threshold: {self.slider_contrast.value() / 100.0}")
            
    def extract_sift(self, gray_image):
        sigma = self.slider_sigma.value() / 10.0
        num_intervals = self.slider_intervals.value()
        contrast = self.slider_contrast.value() / 100.0
        
        sift = cv2.SIFT_create(
            nfeatures=0, 
            nOctaveLayers=num_intervals, 
            contrastThreshold=contrast, 
            edgeThreshold=10, 
            sigma=sigma
        )
        
        start_time = time.time()
        keypoints, descriptors = sift.detectAndCompute(gray_image, None)
        elapsed_ms = int((time.time() - start_time) * 1000)
        
        if descriptors is None:
            descriptors = np.array([])
            keypoints = []
        else:
            # Enforcing strict L2 Matrix normalizations geometrically utilizing cv2.normalize identically to bounds precisely algebraically testing NCC explicitly natively!
            descriptors = descriptors.astype(np.float32)
            for i in range(len(descriptors)):
                cv2.normalize(descriptors[i], descriptors[i], alpha=1.0, norm_type=cv2.NORM_L2)
                
        return keypoints, descriptors, elapsed_ms

    def match_ssd(self, descA, descB, ratio_thresh=0.8):
        if descA is None or descB is None or len(descA) == 0 or len(descB) == 0:
            return []
            
        matcher = cv2.BFMatcher(cv2.NORM_L2, crossCheck=False)
        raw_matches = matcher.knnMatch(descA, descB, k=2)
        
        good_matches = []
        for match_set in raw_matches:
            if len(match_set) == 2:
                m, n = match_set
                if m.distance < ratio_thresh * n.distance:
                    good_matches.append(m)
            elif len(match_set) == 1:
                good_matches.append(match_set[0])
                
        return good_matches

    def match_ncc(self, descA, descB, ncc_thresh=0.85):
        if descA is None or descB is None or len(descA) == 0 or len(descB) == 0:
            return []
            
        good_matches = []
        for i, a_vec in enumerate(descA):
            ncc_array = np.dot(descB, a_vec)
            best_j = np.argmax(ncc_array)
            best_ncc = ncc_array[best_j]
            
            if best_ncc >= ncc_thresh:
                match = cv2.DMatch(_queryIdx=i, _trainIdx=best_j, _imgIdx=0, _distance=float(1.0 - best_ncc))
                good_matches.append(match)
                
        return good_matches

    def apply_matching(self):
        if self.imgA_gray is None or self.imgB_gray is None:
             self.log_message("[Error] Please upload both Image A and Image B first.")
             return
            
        self.log_message("> Running Feature Matcher...")
        
        kpsA, descA, timeA = self.extract_sift(self.imgA_gray)
        self.log_message(f"   Image A: Extracted {len(kpsA)} keypoints in {timeA} ms")
        
        kpsB, descB, timeB = self.extract_sift(self.imgB_gray)
        self.log_message(f"   Image B: Extracted {len(kpsB)} keypoints in {timeB} ms")
        
        metric_str = self.combo_metric.currentText()
        
        match_start = time.time()
        if "SSD" in metric_str:
            self.log_message(f"   Matching via {metric_str}...")
            matches = self.match_ssd(descA, descB, ratio_thresh=0.8)
        else:
            self.log_message(f"   Matching via {metric_str}...")
            matches = self.match_ncc(descA, descB, ncc_thresh=0.85)
            
        match_time = int((time.time() - match_start) * 1000)
        self.log_message(f"   Matched {len(matches)} features successfully!")
        self.log_message(f"   Time taken: {match_time} ms")
        
        if len(matches) > 0:
            result_img = cv2.drawMatches(
                self.imgA_original, kpsA, 
                self.imgB_original, kpsB, 
                matches, None, 
                matchColor=(0, 255, 0), 
                singlePointColor=(255, 0, 0),
                flags=cv2.DRAW_MATCHES_FLAGS_NOT_DRAW_SINGLE_POINTS
            )
            self.current_display = result_img
            self.display_image()
        else:
            self.log_message("   [Warning] No matches found strictly.")
        
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
    window = MatcherTestApp()
    window.show()
    sys.exit(app.exec())
