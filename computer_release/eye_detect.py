import cv2
import dlib
import numpy as np
import pickle
import time  # 新增导入 time 模块
from imutils import face_utils

# dlib 人脸检测器 + 预测器 (shape predictor)
detector = dlib.get_frontal_face_detector()
predictor = dlib.shape_predictor(r"C:\Users\hafeizhou\Desktop\jswjj\computer_release\shape_predictor_68_face_landmarks.dat")
# 加载 dlib 的人脸识别模型
face_rec_model = dlib.face_recognition_model_v1(r"C:\Users\hafeizhou\Desktop\jswjj\computer_release\dlib_face_recognition_resnet_model_v1.dat")

# 修改：从文件加载已知特征
with open(r"C:\Users\hafeizhou\Desktop\jswjj\computer_release\known_faces_features.pkl", "rb") as f:
    known_faces = pickle.load(f)

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
    if candidates:
        identity = max(candidates, key=candidates.get)
        max_prob = candidates[identity]
        print(f"选中身份: {identity}，匹配概率: {max_prob:.4f}")
        return identity, max_prob
    else:
        print("没有匹配到已知身份")
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
        
        # 你可以在此计算眼睛轮廓的凸包，然后测量轮廓面积
        # 例：cv2.convexHull(left_eye_pts) -> 求得凸包，再用 cv2.contourArea() 算面积
        # left_eye_hull = cv2.convexHull(left_eye_pts)
        # left_eye_area = cv2.contourArea(left_eye_hull)
        # cv2.putText(frame, f"L_Eye_Area: {int(left_eye_area)}", (x1, y1 - 10),
        #             cv2.FONT_HERSHEY_SIMPLEX, 0.5, (0, 255, 0), 1)
                    
        # --- 人脸身份匹配 ---
        rgb_frame = cv2.cvtColor(frame, cv2.COLOR_BGR2RGB)
        face_descriptor = face_rec_model.compute_face_descriptor(rgb_frame, predictor(cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY), rect))
        face_descriptor = np.array(face_descriptor)
        identity, min_distance = compare_faces(face_descriptor, known_faces)
        cv2.putText(frame, identity, (x1, y2 + 20), cv2.FONT_HERSHEY_SIMPLEX, 0.5, (0, 0, 255), 1)
        # --- 新增结束 ---
        
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
        time.sleep(1)  # 新增：每帧延时1秒

    cap.release()
    cv2.destroyAllWindows()

if __name__ == "__main__":
    main()
