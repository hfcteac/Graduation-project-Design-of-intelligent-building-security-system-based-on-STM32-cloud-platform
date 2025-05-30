import cv2
import dlib
import numpy as np
import pickle
import time
from imutils import face_utils
import Hobot.GPIO as GPIO
# 音频播放相关库
import subprocess
import threading
import os

# dlib 人脸检测器 + 预测器 (shape predictor)
detector = dlib.get_frontal_face_detector()
predictor = dlib.shape_predictor("/root/sn_bishe/shape_predictor_68_face_landmarks.dat")
# 加载 dlib 的人脸识别模型
face_rec_model = dlib.face_recognition_model_v1("/root/sn_bishe/dlib_face_recognition_resnet_model_v1.dat")
GPIO.setmode(GPIO.BOARD)
GPIO.setup(37, GPIO.OUT, initial=GPIO.LOW)
# 从文件加载已知特征
with open("/root/sn_bishe/known_faces_features.pkl", "rb") as f:
    known_faces = pickle.load(f)

# 添加不同场景的音频文件路径
KNOWN_AUDIO = "/root/sn_bishe/已开门.mp3"  # 识别到已知身份时播放
UNKNOWN_AUDIO = "/root/sn_bishe/识别失败.mp3"  # 识别到未知身份时播放

# I2S设备配置
I2S_DEVICE = "hw:1,0"  # 根据您的系统设置 card 1: duplexaudio

# 设置音频音量（在程序启动时调用）
def setup_audio():
    try:
        # 设置主音量为80%
        subprocess.call(["amixer", "-c", "1", "sset", "Master", "80%"])
    except Exception as e:
        print(f"设置音频音量时出错: {e}")

# 防止重复播放的标志
audio_playing = False
last_played_time = 0
PLAY_INTERVAL = 3  # 音频播放间隔(秒)

def play_audio(audio_file):
    """使用I2S接口播放音频文件"""
    def play_in_background(file):
        global audio_playing
        try:
            # 检查文件格式并使用适当的命令通过I2S播放
            if file.lower().endswith('.wav'):
                # 使用aplay通过I2S播放WAV文件
                subprocess.call(["aplay", "-D", I2S_DEVICE, file])
            elif file.lower().endswith('.mp3'):
                # 使用mpg123通过I2S播放MP3文件
                subprocess.call(["mpg123", "--quiet", "-a", I2S_DEVICE, file])
            else:
                # 使用ffplay播放其他格式
                subprocess.call(["ffplay", "-nodisp", "-autoexit", "-af", f"alsa={I2S_DEVICE}", file])
        except Exception as e:
            print(f"I2S播放音频时出错: {e}")
        finally:
            audio_playing = False
    
    global audio_playing
    if not audio_playing:
        audio_playing = True
        threading.Thread(target=play_in_background, args=(audio_file,), daemon=True).start()

# filepath: [shibie.py](http://_vscodecontentref_/2)
def compare_faces(face_descriptor, known_faces, threshold=0.45):
    """
    对多个已知样本进行筛选，计算匹配概率，
    最终选取概率最高的身份作为识别结果。
    """
    candidates = {}  # 存储每个身份的最大匹配概率
    for name, descriptors_list in known_faces.items():
        for known_descriptor in descriptors_list:
            distance = np.linalg.norm(face_descriptor - np.array(known_descriptor))
            if distance < threshold:
                # 将距离转换为匹配概率：距离越近，匹配概率越高
                probability = 1 - distance / threshold
                if name not in candidates or probability > candidates[name]:
                    candidates[name] = probability
    
    # 根据时间间隔控制决定是否播放音频
    global last_played_time
    current_time = time.time()
    
    if candidates:
        identity = max(candidates, key=candidates.get)
        max_prob = candidates[identity]
        if max_prob > 0.2:
            print(f"选中身份: {identity}，匹配概率: {max_prob:.4f}")
            GPIO.output(37, GPIO.HIGH)
        
            # 识别到已知身份，播放欢迎音频
            if current_time - last_played_time > PLAY_INTERVAL:
                play_audio(KNOWN_AUDIO)
                last_played_time = current_time
                
            return identity, max_prob
        else:
            # 添加这个else分支，处理置信度低的情况
            print(f"置信度过低: {max_prob:.4f}，视为未知身份")
            GPIO.output(37, GPIO.LOW)
            
            if current_time - last_played_time > PLAY_INTERVAL:
                play_audio(UNKNOWN_AUDIO)
                last_played_time = current_time
                
            return "Low Confidence", max_prob
    else:
        print("没有匹配到已知身份")
        GPIO.output(37, GPIO.LOW)
    
        # 识别到未知身份，播放警告音频
        if current_time - last_played_time > PLAY_INTERVAL:
            play_audio(UNKNOWN_AUDIO)
            last_played_time = current_time
            
        return "Unknown", 0.0

def detect_eyes_with_dlib(frame):
    gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)
    rects, scores, _ = detector.run(gray, 1, -1)
    
    for rect, score in zip(rects, scores):
        norm_score = 1 / (1 + np.exp(-score))  # 使用 sigmoid 将 score 映射到 0-1 区间
        if norm_score < 0.5:  # 归一化后低于阈值直接过滤
            continue
        (x1, y1, w, h) = face_utils.rect_to_bb(rect)
        x2, y2 = x1 + w, y1 + h
        confidence = norm_score
        cv2.rectangle(frame, (x1, y1), (x2, y2), (0, 255, 255), 2)
        cv2.putText(frame, f"{confidence:.2f}", (x1, y1 - 10),
                    cv2.FONT_HERSHEY_SIMPLEX, 0.5, (0, 255, 255), 1)
        
        shape = predictor(gray, rect)
        shape = face_utils.shape_to_np(shape)  # 转为 NumPy 数组
        
        # 左眼 landmark 索引通常为 [36, 37, 38, 39, 40, 41]
        left_eye_pts = shape[36:42]
        # 右眼 landmark 索引通常为 [42, 43, 44, 45, 46, 47]
        right_eye_pts = shape[42:48]
        
        # 画出左眼的轮廓
        for (ex, ey) in left_eye_pts:
            cv2.circle(frame, (ex, ey), 2, (0, 255, 0), -1)
        # 画出右眼的轮廓
        for (ex, ey) in right_eye_pts:
            cv2.circle(frame, (ex, ey), 2, (0, 255, 0), -1)
        
        # --- 人脸身份匹配 ---
        rgb_frame = cv2.cvtColor(frame, cv2.COLOR_BGR2RGB)
        face_descriptor = face_rec_model.compute_face_descriptor(rgb_frame, predictor(cv2.cvtColor(frame, cv2.COLOR_BGR2RGB), rect))
        face_descriptor = np.array(face_descriptor)
        identity, min_distance = compare_faces(face_descriptor, known_faces)
        cv2.putText(frame, identity, (x1, y2 + 20), cv2.FONT_HERSHEY_SIMPLEX, 0.5, (0, 0, 255), 1)
    
    return frame

def main():
    cap = cv2.VideoCapture(0)
    while True:
        ret, frame = cap.read()
        if not ret:
            break
        
        frame = detect_eyes_with_dlib(frame)
        
        cv2.imshow("Eye Detection (dlib landmarks)", frame)
        if cv2.waitKey(1) & 0xFF == ord('q'):
            break
        #time.sleep(1)  # 每帧延时1秒

    cap.release()
    cv2.destroyAllWindows()
    GPIO.cleanup()  # 清理GPIO

if __name__ == "__main__":
    main()