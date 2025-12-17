import cv2
import dlib
import numpy as np
import pickle
import time
from imutils import face_utils
# 音频播放相关库
import subprocess
import threading
import os

# dlib 人脸检测器 + 预测器 (shape predictor)
detector = dlib.get_frontal_face_detector()
predictor = dlib.shape_predictor("/home/pi/Desktop/face/shape_predictor_68_face_landmarks.dat")
# 加载 dlib 的人脸识别模型
face_rec_model = dlib.face_recognition_model_v1("/home/pi/Desktop/face/dlib_face_recognition_resnet_model_v1.dat")

# 添加GPIO异常处理
try:
    import RPi.GPIO as GPIO  # 树莓派GPIO库
    # 修改为树莓派的GPIO配置
    GPIO.setwarnings(False)  # 关闭警告
    GPIO.setmode(GPIO.BCM)  # 使用BCM编号方式
    RELAY_PIN = 26  # 假设使用GPIO26连接继电器，对应树莓派BCM编号
    GPIO.setup(RELAY_PIN, GPIO.OUT, initial=GPIO.LOW)
    gpio_available = True
    print("GPIO初始化成功")
except Exception as e:
    print(f"GPIO初始化失败: {e}")
    print("程序将继续运行，但继电器控制功能将被禁用")
    gpio_available = False
    
    # 定义空函数来替代GPIO函数，防止程序崩溃
    class DummyGPIO:
        HIGH = 1
        LOW = 0
        OUT = "out"
        
        @staticmethod
        def output(pin, state):
            print(f"[模拟GPIO] 设置引脚 {pin} 为 {'HIGH' if state == 1 else 'LOW'}")
        
        @staticmethod
        def cleanup():
            print("[模拟GPIO] 清理GPIO")
    
    # 如果GPIO库初始化失败，使用模拟的GPIO类
    GPIO = DummyGPIO()

# 从文件加载已知特征
with open("/home/pi/Desktop/face/known_faces_features.pkl", "rb") as f:
    known_faces = pickle.load(f)
