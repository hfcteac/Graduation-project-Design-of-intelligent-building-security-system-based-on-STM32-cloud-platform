import cv2
import dlib
import os
import pickle
import numpy as np
from imutils import face_utils
import os.path

# 初始化模型
detector = dlib.get_frontal_face_detector()
predictor = dlib.shape_predictor("shape_predictor_68_face_landmarks.dat")
face_rec_model = dlib.face_recognition_model_v1("dlib_face_recognition_resnet_model_v1.dat")

# 存放原图的目录和保存特征的文件路径
faces_dir = r"C:\Users\yyx\Desktop\face_detect\known_faces"
save_path = r"C:\Users\yyx\Desktop\face_detect\known_faces_features.pkl"

# 调整为更严格的阈值，避免误识别
IDENTITY_THRESHOLD = 0.45  # 降低阈值会减少误识别率，但可能会增加拒识率

known_faces = {}

# 为每个身份收集多个样本特征
for filename in os.listdir(faces_dir):
    if filename.lower().endswith((".jpg", ".png", ".jpeg")):
        identity = os.path.splitext(filename)[0]
        img_path = os.path.join(faces_dir, filename)
        img_path = os.path.normpath(img_path)
        print("读取图片：", img_path)
        
        if not os.path.exists(img_path):
            print(f"文件不存在: {img_path}")
            continue
            
        image = cv2.imread(img_path)
        if image is None:
            print(f"无法读取图片: {img_path}")
            continue
            
        # 数据增强：创建多个微调的图像版本
        augmented_images = []
        augmented_images.append(image)  # 原始图像
        
        # 添加轻微旋转版本
        for angle in [-5, 5]:
            h, w = image.shape[:2]
            center = (w // 2, h // 2)
            M = cv2.getRotationMatrix2D(center, angle, 1.0)
            rotated = cv2.warpAffine(image, M, (w, h), borderMode=cv2.BORDER_REPLICATE)
            augmented_images.append(rotated)
        
        # 处理所有增强版本
        face_descriptors = []
        for img in augmented_images:
            gray = cv2.cvtColor(img, cv2.COLOR_BGR2GRAY)
            rects, scores, _ = detector.run(gray, 1, -1)
            
            if len(rects) == 0:
                continue
                
            # 选择置信度最高的人脸
            best_idx = np.argmax(scores)
            rect = rects[best_idx]
            confidence = scores[best_idx]
            
            if confidence < 0.5:  # 过滤低置信度检测
                continue
                
            # 显示检测结果
            vis_img = img.copy()
            (x, y, w, h) = face_utils.rect_to_bb(rect)
            cv2.rectangle(vis_img, (x, y), (x+w, y+h), (0,255,0), 2)
            cv2.putText(vis_img, f"{confidence:.2f}", (x, y-10),
                      cv2.FONT_HERSHEY_SIMPLEX, 0.5, (0,255,0), 2)
            cv2.imshow("Detection", vis_img)
            cv2.waitKey(0)
            
            # 面部对齐提高特征提取质量
            shape = predictor(gray, rect)
            face_chip = dlib.get_face_chip(img, shape, size=150)
            
            # 提取特征
            descriptor = face_rec_model.compute_face_descriptor(face_chip)
            face_descriptors.append(list(descriptor))
        
        # 存储该身份的所有特征向量
        if face_descriptors:
            if identity not in known_faces:
                known_faces[identity] = []
            known_faces[identity].extend(face_descriptors)
            print(f"已为 {identity} 提取 {len(face_descriptors)} 个特征向量")

# 保存特征
save_dir = os.path.dirname(save_path)
if not os.path.exists(save_dir):
    os.makedirs(save_dir)

with open(save_path, "wb") as f:
    pickle.dump(known_faces, f)

print(f"已保存 {len(known_faces)} 个身份特征到 {save_path}")