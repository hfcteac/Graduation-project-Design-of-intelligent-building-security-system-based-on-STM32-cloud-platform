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
# 为GPIO直接内存访问导入所需模块
import mmap
import struct

# 设置环境变量以解决Qt平台插件问题
os.environ["QT_QPA_PLATFORM"] = "xcb"  # 强制使用X11而不是Wayland

# GPIO控制相关变量
DOOR_CONTROL_PIN = 18  # 使用GPIO18控制门禁
gpio_available = False  # 标记GPIO是否可用

# 针对不同树莓派型号的GPIO寄存器地址
RPI5_GPIO_BASE = 0xFE200000  # 树莓派5
RPI4_GPIO_BASE = 0xFE200000  # 树莓派4
RPI3_GPIO_BASE = 0x3F200000  # 树莓派3及更早版本

# 树莓派5特定的寄存器偏移量
GPIO_FSEL_OFFSET = 0x00   # 功能选择寄存器
GPIO_SET_OFFSET = 0x1C    # 设置寄存器
GPIO_CLR_OFFSET = 0x28    # 清除寄存器
GPIO_LEV_OFFSET = 0x34    # 电平寄存器

# 添加最底层的GPIO控制方法 - 使用/dev/mem直接访问
def setup_gpio_direct():
    """最底层的GPIO控制方法，直接访问物理内存"""
    global gpio_available, gpio_mem, gpio_base
    try:
        # 默认使用树莓派4B的GPIO寄存器基地址
        gpio_base = RPI4_GPIO_BASE
        
        # 根据不同的树莓派型号设置不同的基地址
        with open('/proc/device-tree/model', 'r') as f:
            model = f.read()
            if '5' in model:  # 树莓派5型号
                gpio_base = RPI5_GPIO_BASE
                print("检测到树莓派5，使用专用寄存器地址")
            elif '4' not in model:  # 非树莓派4型号
                gpio_base = RPI3_GPIO_BASE
        
        print(f"检测到树莓派型号: {model.strip()}")
        print(f"使用GPIO基地址: 0x{gpio_base:X}")
        
        # 内存映射长度
        BLOCK_SIZE = 4096
        
        # 打开/dev/mem并创建内存映射
        with open("/dev/mem", "r+b") as f:
            mem = mmap.mmap(f.fileno(), BLOCK_SIZE, offset=gpio_base)
            
            # GPIO功能选择寄存器 (GPFSELn)
            # 每个引脚有3位控制
            FSEL_OFFSET = (DOOR_CONTROL_PIN // 10) * 4
            FSEL_SHIFT = (DOOR_CONTROL_PIN % 10) * 3
            
            # 读取当前的寄存器值
            mem.seek(FSEL_OFFSET)
            reg = struct.unpack("<I", mem.read(4))[0]
            
            # 清除该引脚的功能位
            reg &= ~(7 << FSEL_SHIFT)
            # 设置为输出模式 (001)
            reg |= (1 << FSEL_SHIFT)
            
            # 写回寄存器
            mem.seek(FSEL_OFFSET)
            mem.write(struct.pack("<I", reg))
            
            # 初始状态设为低电平
            set_gpio_direct_low(mem)
            
            print("GPIO初始化完成（使用直接内存访问）")
            gpio_available = True
            
            # 保存内存映射对象以便后续使用
            gpio_mem = mem
            
            # 读取当前GPIO状态并显示
            read_gpio_status()
            
            return True
    except Exception as e:
        print(f"直接GPIO访问失败: {e}")
        print("请使用sudo运行此程序以获取足够权限")
        return False

def read_gpio_status():
    """读取GPIO当前状态（树莓派5特有功能）"""
    global gpio_mem
    try:
        # 读取GPIO电平寄存器
        gpio_mem.seek(GPIO_LEV_OFFSET)
        reg = struct.unpack("<I", gpio_mem.read(4))[0]
        
        # 检查指定GPIO的状态
        pin_status = (reg >> DOOR_CONTROL_PIN) & 1
        print(f"GPIO{DOOR_CONTROL_PIN}当前状态: {'高' if pin_status else '低'}")
        return pin_status
    except Exception as e:
        print(f"读取GPIO状态失败: {e}")
        return None

def set_gpio_direct_high(mem=None):
    """设置GPIO为高电平（直接内存访问）"""
    global gpio_mem
    if mem is None:
        mem = gpio_mem
    
    try:
        # GPIO设置寄存器 (GPSET0)
        # 计算位偏移
        GPIO_BIT = 1 << (DOOR_CONTROL_PIN % 32)
        
        # 写入设置寄存器
        mem.seek(GPIO_SET_OFFSET)
        mem.write(struct.pack("<I", GPIO_BIT))
        
        # 强制同步内存
        mem.flush()
        
        # 添加短暂延时确保寄存器写入生效
        time.sleep(0.01)
        
        # 重新写入一次，确保设置成功
        mem.seek(GPIO_SET_OFFSET)
        mem.write(struct.pack("<I", GPIO_BIT))
        mem.flush()
        
        print("门禁已开启（直接内存访问）")
        
        # 添加延时后再读取状态
        time.sleep(0.05)
        
        # 验证设置是否成功
        status = read_gpio_status()
        if status == 1:
            print("验证成功：GPIO已设为高电平")
        else:
            print("警告：GPIO未能设为高电平，尝试强制设置...")
            # 使用不同的偏移量再试一次（针对树莓派5可能的寄存器差异）
            for offset_adjust in [0, 4, -4]:
                try:
                    mem.seek(GPIO_SET_OFFSET + offset_adjust)
                    mem.write(struct.pack("<I", GPIO_BIT))
                    mem.flush()
                    time.sleep(0.05)
                    if read_gpio_status() == 1:
                        print(f"使用偏移调整 {offset_adjust} 成功设置高电平")
                        break
                except Exception:
                    pass
        
        # 无论验证结果如何，都返回成功以继续执行
        return True
    except Exception as e:
        print(f"设置GPIO高电平失败: {e}")
        return False

def set_gpio_direct_low(mem=None):
    """设置GPIO为低电平（直接内存访问）"""
    global gpio_mem
    if mem is None:
        mem = gpio_mem
    
    try:
        # GPIO清除寄存器 (GPCLR0)
        # 计算位偏移
        GPIO_BIT = 1 << (DOOR_CONTROL_PIN % 32)
        
        # 写入清除寄存器
        mem.seek(GPIO_CLR_OFFSET)
        mem.write(struct.pack("<I", GPIO_BIT))
        print("门禁已关闭（直接内存访问）")
        
        # 验证设置是否成功（树莓派5特有功能）
        status = read_gpio_status()
        if status == 0:
            print("验证成功：GPIO已设为低电平")
            
        return True
    except Exception as e:
        print(f"设置GPIO低电平失败: {e}")
        return False

# 添加延时开门功能
def open_door_with_timeout(seconds=3):
    """设置GPIO为高电平，持续指定的秒数后自动设为低电平"""
    def door_control():
        set_gpio_high()  # 开门
        print(f"门已开启，将在{seconds}秒后自动关闭")
        time.sleep(seconds)  # 等待
        set_gpio_low()  # 关门
        print("门已自动关闭")
    
    # 启动后台线程控制门
    threading.Thread(target=door_control, daemon=True).start()

def cleanup_gpio_direct():
    """清理直接GPIO访问的资源"""
    global gpio_mem, gpio_available
    try:
        if 'gpio_mem' in globals() and gpio_mem:
            # 设置为低电平
            set_gpio_direct_low()
            # 关闭内存映射
            gpio_mem.close()
            gpio_available = False
            print("GPIO资源已释放（直接内存访问）")
            return True
    except Exception as e:
        print(f"清理GPIO资源失败: {e}")
        return False

# 添加备用GPIO控制方法（使用sysfs接口，更兼容，适用于树莓派5）
def setup_gpio_sysfs():
    """使用sysfs接口设置GPIO（更兼容树莓派5）"""
    global gpio_available
    try:
        # 检查GPIO导出目录是否存在
        if not os.path.exists(f"/sys/class/gpio/gpio{DOOR_CONTROL_PIN}"):
            # 导出GPIO
            with open("/sys/class/gpio/export", "w") as f:
                f.write(str(DOOR_CONTROL_PIN))
            
            # 等待GPIO导出完成
            time.sleep(0.1)
        
        # 设置GPIO方向为输出
        with open(f"/sys/class/gpio/gpio{DOOR_CONTROL_PIN}/direction", "w") as f:
            f.write("out")
        
        # 初始设置为低电平
        with open(f"/sys/class/gpio/gpio{DOOR_CONTROL_PIN}/value", "w") as f:
            f.write("0")
        
        print(f"GPIO{DOOR_CONTROL_PIN}初始化完成（使用sysfs接口）")
        gpio_available = True
        return True
    except Exception as e:
        print(f"使用sysfs设置GPIO失败: {e}")
        print("请使用sudo运行此程序以获取足够权限")
        return False

def set_gpio_high_sysfs():
    """使用sysfs接口设置GPIO为高电平"""
    try:
        with open(f"/sys/class/gpio/gpio{DOOR_CONTROL_PIN}/value", "w") as f:
            f.write("1")
        print(f"GPIO{DOOR_CONTROL_PIN}已设为高电平（使用sysfs接口）")
        return True
    except Exception as e:
        print(f"设置GPIO高电平失败: {e}")
        return False

def set_gpio_low_sysfs():
    """使用sysfs接口设置GPIO为低电平"""
    try:
        with open(f"/sys/class/gpio/gpio{DOOR_CONTROL_PIN}/value", "w") as f:
            f.write("0")
        print(f"GPIO{DOOR_CONTROL_PIN}已设为低电平（使用sysfs接口）")
        return True
    except Exception as e:
        print(f"设置GPIO低电平失败: {e}")
        return False

def cleanup_gpio_sysfs():
    """清理sysfs GPIO资源"""
    try:
        # 将GPIO取消导出
        if os.path.exists(f"/sys/class/gpio/gpio{DOOR_CONTROL_PIN}"):
            with open("/sys/class/gpio/unexport", "w") as f:
                f.write(str(DOOR_CONTROL_PIN))
            print(f"GPIO{DOOR_CONTROL_PIN}资源已释放（使用sysfs接口）")
        return True
    except Exception as e:
        print(f"清理GPIO资源失败: {e}")
        return False

# 添加命令行方式控制GPIO（使用raspi-gpio工具，树莓派官方支持）
def setup_gpio_cmd():
    """使用raspi-gpio命令行工具设置GPIO"""
    global gpio_available
    try:
        # 设置为输出模式
        subprocess.call(["raspi-gpio", "set", str(DOOR_CONTROL_PIN), "op"])
        # 设置为低电平
        subprocess.call(["raspi-gpio", "set", str(DOOR_CONTROL_PIN), "dl"])
        print(f"GPIO{DOOR_CONTROL_PIN}初始化完成（使用raspi-gpio命令行）")
        gpio_available = True
        return True
    except Exception as e:
        print(f"使用raspi-gpio设置GPIO失败: {e}")
        
        # 检查raspi-gpio工具是否安装
        try:
            subprocess.call(["which", "raspi-gpio"])
        except:
            print("找不到raspi-gpio命令，请安装：sudo apt-get install raspi-gpio")
        
        return False

def set_gpio_high_cmd():
    """使用raspi-gpio命令行工具设置GPIO为高电平"""
    try:
        subprocess.call(["raspi-gpio", "set", str(DOOR_CONTROL_PIN), "dh"])
        print(f"GPIO{DOOR_CONTROL_PIN}已设为高电平（使用raspi-gpio命令行）")
        return True
    except Exception as e:
        print(f"设置GPIO高电平失败: {e}")
        return False

def set_gpio_low_cmd():
    """使用raspi-gpio命令行工具设置GPIO为低电平"""
    try:
        subprocess.call(["raspi-gpio", "set", str(DOOR_CONTROL_PIN), "dl"])
        print(f"GPIO{DOOR_CONTROL_PIN}已设为低电平（使用raspi-gpio命令行）")
        return True
    except Exception as e:
        print(f"设置GPIO低电平失败: {e}")
        return False

def cleanup_gpio_cmd():
    """清理命令行方式的GPIO资源"""
    try:
        # 设置为低电平即可
        subprocess.call(["raspi-gpio", "set", str(DOOR_CONTROL_PIN), "dl"])
        print(f"GPIO{DOOR_CONTROL_PIN}资源已释放（使用raspi-gpio命令行）")
        return True
    except Exception as e:
        print(f"清理GPIO资源失败: {e}")
        return False

# 作为最后尝试，直接操作 /sys/class/gpio 的替代方法
def setup_gpio_alt():
    """使用替代方法直接操作/sys文件"""
    global gpio_available
    try:
        # 不使用Python的文件打开方式，而是使用命令行方式
        os.system(f"echo {DOOR_CONTROL_PIN} > /sys/class/gpio/export 2>/dev/null || true")
        time.sleep(0.1)
        os.system(f"echo out > /sys/class/gpio/gpio{DOOR_CONTROL_PIN}/direction 2>/dev/null || true")
        os.system(f"echo 0 > /sys/class/gpio/gpio{DOOR_CONTROL_PIN}/value 2>/dev/null || true")
        
        print(f"GPIO{DOOR_CONTROL_PIN}初始化完成（使用替代方法）")
        gpio_available = True
        return True
    except Exception as e:
        print(f"使用替代方法设置GPIO失败: {e}")
        return False

def set_gpio_high_alt():
    """使用替代方法设置GPIO为高电平"""
    try:
        os.system(f"echo 1 > /sys/class/gpio/gpio{DOOR_CONTROL_PIN}/value 2>/dev/null || true")
        print(f"GPIO{DOOR_CONTROL_PIN}已设为高电平（使用替代方法）")
        return True
    except Exception as e:
        print(f"设置GPIO高电平失败: {e}")
        return False

def set_gpio_low_alt():
    """使用替代方法设置GPIO为低电平"""
    try:
        os.system(f"echo 0 > /sys/class/gpio/gpio{DOOR_CONTROL_PIN}/value 2>/dev/null || true")
        print(f"GPIO{DOOR_CONTROL_PIN}已设为低电平（使用替代方法）")
        return True
    except Exception as e:
        print(f"设置GPIO低电平失败: {e}")
        return False

def cleanup_gpio_alt():
    """清理替代方法的GPIO资源"""
    try:
        os.system(f"echo 0 > /sys/class/gpio/gpio{DOOR_CONTROL_PIN}/value 2>/dev/null || true")
        os.system(f"echo {DOOR_CONTROL_PIN} > /sys/class/gpio/unexport 2>/dev/null || true")
        print(f"GPIO{DOOR_CONTROL_PIN}资源已释放（使用替代方法）")
        return True
    except Exception as e:
        print(f"清理GPIO资源失败: {e}")
        return False

# 修改GPIO控制函数，增加pinctrl作为树莓派5的首选方法
def setup_gpio():
    """初始化GPIO，尝试多种方法"""
    global set_gpio_high, set_gpio_low, cleanup_gpio
    
    print(f"尝试初始化GPIO {DOOR_CONTROL_PIN}...")
    
    # 检查系统版本和平台信息
    print("系统信息:")
    os.system("uname -a")
    
    # 检测是否为树莓派5
    is_pi5 = False
    try:
        with open('/proc/device-tree/model', 'r') as f:
            model = f.read()
            if '5' in model:
                is_pi5 = True
                print("检测到树莓派5，将优先使用pinctrl工具")
            print(f"设备型号: {model.strip()}")
    except:
        print("无法读取设备型号")
    
    # 创建方法列表，按优先级排序
    methods = []
    
    # 如果是树莓派5，优先使用pinctrl
    if is_pi5:
        methods.append({
            "name": "pinctrl工具(Pi5专用)",
            "setup": setup_gpio_pinctrl,
            "high": set_gpio_high_pinctrl,
            "low": set_gpio_low_pinctrl,
            "cleanup": cleanup_gpio_pinctrl
        })
    
    # 添加其他通用方法
    methods.extend([
        {
            "name": "命令行方式(raspi-gpio)",
            "setup": setup_gpio_cmd,
            "high": set_gpio_high_cmd,
            "low": set_gpio_low_cmd,
            "cleanup": cleanup_gpio_cmd
        },
        {
            "name": "直接内存访问",
            "setup": setup_gpio_direct,
            "high": set_gpio_direct_high,
            "low": set_gpio_direct_low,
            "cleanup": cleanup_gpio_direct
        },
        {
            "name": "sysfs接口",
            "setup": setup_gpio_sysfs,
            "high": set_gpio_high_sysfs,
            "low": set_gpio_low_sysfs,
            "cleanup": cleanup_gpio_sysfs
        },
        {
            "name": "替代方法",
            "setup": setup_gpio_alt,
            "high": set_gpio_high_alt,
            "low": set_gpio_low_alt,
            "cleanup": cleanup_gpio_alt
        }
    ])
    
    # 尝试每种方法
    for method in methods:
        print(f"\n尝试使用 {method['name']} 初始化GPIO...")
        try:
            if method["setup"]():
                print(f"使用 {method['name']} 初始化GPIO成功!")
                set_gpio_high = method["high"]
                set_gpio_low = method["low"]
                cleanup_gpio = method["cleanup"]
                return True
        except Exception as e:
            print(f"使用 {method['name']} 初始化GPIO失败: {e}")
    
    print("\n警告: 所有GPIO初始化方法均失败！门禁控制将不可用。")
    
    # 如果所有方法都失败，使用空操作函数
    def dummy_function(*args, **kwargs):
        print("GPIO控制不可用")
        return False
    
    set_gpio_high = dummy_function
    set_gpio_low = dummy_function
    cleanup_gpio = dummy_function
    return False

# 更新测试函数以检查是否有sudo权限
def test_gpio_toggle():
    """测试GPIO高低电平切换，帮助诊断问题"""
    print("\n======== 开始GPIO测试序列 ========")
    print(f"使用GPIO: {DOOR_CONTROL_PIN}")
    
    # 检查是否有root权限
    is_root = os.geteuid() == 0 if hasattr(os, 'geteuid') else False
    print(f"运行权限: {'Root/sudo' if is_root else '普通用户'}")
    if not is_root:
        print("警告: 没有sudo权限可能导致GPIO控制失败")
        print("建议使用: sudo python3 eye_detect.py")
    
    # 测试低电平
    print("\n测试设置低电平...")
    result = set_gpio_low()
    print(f"设置低电平结果: {'成功' if result else '失败'}")
    time.sleep(1)
    
    # 测试高电平
    print("\n测试设置高电平...")
    result = set_gpio_high()
    print(f"设置高电平结果: {'成功' if result else '失败'}")
    time.sleep(1)
    
    # 再次测试低电平
    print("\n再次测试设置低电平...")
    result = set_gpio_low()
    print(f"设置低电平结果: {'成功' if result else '失败'}")
    
    print("\n======== GPIO测试序列完成 ========\n")

# dlib 人脸检测器 + 预测器 (shape predictor)
detector = dlib.get_frontal_face_detector()
predictor = dlib.shape_predictor("/home/pi/Desktop/face/shape_predictor_68_face_landmarks.dat")
# 加载 dlib 的人脸识别模型
face_rec_model = dlib.face_recognition_model_v1("/home/pi/Desktop/face/dlib_face_recognition_resnet_model_v1.dat")

# 从文件加载已知特征
with open("/home/pi/Desktop/face/known_faces_features.pkl", "rb") as f:
    known_faces = pickle.load(f)

# 添加不同场景的音频文件路径
KNOWN_AUDIO = "/home/pi/Desktop/face/已开门.mp3"  # 识别到已知身份时播放
UNKNOWN_AUDIO = "/home/pi/Desktop/face/识别失败.mp3"  # 识别到未知身份时播放

# I2S设备配置
I2S_DEVICE = "hw:1,0"  # 根据您的系统设置 card 1: duplexaudio

# 设置音频音量（在程序启动时调用）
def setup_audio():
    try:
        # 尝试多种可能的音量控制方式
        # 1. 尝试设置卡1的音量
        try:
            subprocess.call(["amixer", "-c", "1", "sset", "Master", "80%"])
        except Exception:
            pass
        
        # 2. 尝试默认设备的音量
        try:
            subprocess.call(["amixer", "sset", "Master", "80%"])
        except Exception:
            pass
            
        # 3. 尝试PCM控制器
        try:
            subprocess.call(["amixer", "-c", "1", "sset", "PCM", "80%"])
        except Exception:
            pass
            
        # 4. 尝试列出所有控制器，用于调试
        try:
            result = subprocess.check_output(["amixer", "-c", "1", "controls"]).decode()
            print("可用的音频控制器:")
            print(result)
        except Exception as e:
            print(f"获取音频控制器列表失败: {e}")
            
        print("音频设置完成")
    except Exception as e:
        print(f"设置音频音量时出错: {e}")
        print("继续运行，但音频可能不可用")

# 音频播放功能增强
audio_playing = False
last_played_time = 0
PLAY_INTERVAL = 3  # 音频播放间隔(秒)

def play_audio(audio_file):
    """使用I2S接口播放音频文件"""
    def play_in_background(file):
        global audio_playing
        try:
            # 尝试多种播放方式
            if file.lower().endswith('.wav'):
                # 1. 尝试使用指定设备
                try:
                    subprocess.call(["aplay", "-D", I2S_DEVICE, file])
                except Exception:
                    # 2. 尝试使用默认设备
                    try:
                        subprocess.call(["aplay", file])
                    except Exception as e:
                        print(f"播放WAV失败: {e}")
            elif file.lower().endswith('.mp3'):
                # 1. 尝试使用mpg123+指定设备
                try:
                    subprocess.call(["mpg123", "--quiet", "-a", I2S_DEVICE, file])
                except Exception:
                    # 2. 尝试使用mpg123默认设备
                    try:
                        subprocess.call(["mpg123", "--quiet", file])
                    except Exception as e:
                        print(f"播放MP3失败: {e}")
            else:
                # 尝试使用ffplay
                try:
                    subprocess.call(["ffplay", "-nodisp", "-autoexit", file])
                except Exception as e:
                    print(f"使用ffplay播放失败: {e}")
        except Exception as e:
            print(f"播放音频时出错: {e}")
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
        
            # 识别到已知身份，播放欢迎音频
            if current_time - last_played_time > PLAY_INTERVAL:
                play_audio(KNOWN_AUDIO)
                last_played_time = current_time
                
            # 识别到已知身份，设置GPIO高电平并在3秒后自动关闭
            open_door_with_timeout(3)
                
            return identity, max_prob
        else:
            # 添加这个else分支，处理置信度低的情况
            print(f"置信度过低: {max_prob:.4f}，视为未知身份")
            
            if current_time - last_played_time > PLAY_INTERVAL:
                play_audio(UNKNOWN_AUDIO)
                last_played_time = current_time
                
            # 置信度低，设置GPIO低电平
            set_gpio_low()
                
            return "Low Confidence", max_prob
    else:
        print("没有匹配到已知身份")
    
        # 识别到未知身份，播放警告音频
        if current_time - last_played_time > PLAY_INTERVAL:
            play_audio(UNKNOWN_AUDIO)
            last_played_time = current_time
            
        # 未知身份，设置GPIO低电平
        set_gpio_low()
            
        return "Unknown", 0.0

def detect_eyes_with_dlib(frame):
    gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)
    rects, scores, _ = detector.run(gray, 1, -1)
    
    # 如果没有检测到人脸，则设置GPIO为低电平
    if len(rects) == 0:
        # 设置GPIO低电平
        set_gpio_low()
        print("未检测到人脸，门禁已关闭")
        return frame
    
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

# 添加树莓派5专用的pinctrl GPIO控制方法
def setup_gpio_pinctrl():
    """使用pinctrl命令行工具设置GPIO（专为树莓派5设计）"""
    global gpio_available
    try:
        # 检查pinctrl命令是否存在
        result = subprocess.call(["which", "pinctrl"], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
        if result != 0:
            print("找不到pinctrl命令，此方法仅适用于树莓派5")
            return False
            
        # 配置GPIO为输出模式
        subprocess.call(["sudo", "pinctrl", "set", str(DOOR_CONTROL_PIN), "op", "pn"])
        # 设置为低电平
        subprocess.call(["sudo", "pinctrl", "set", str(DOOR_CONTROL_PIN), "dl"])
        
        print(f"GPIO{DOOR_CONTROL_PIN}初始化完成（使用pinctrl工具，专为树莓派5设计）")
        gpio_available = True
        return True
    except Exception as e:
        print(f"使用pinctrl设置GPIO失败: {e}")
        return False

def set_gpio_high_pinctrl():
    """使用pinctrl命令行工具设置GPIO为高电平"""
    try:
        subprocess.call(["sudo", "pinctrl", "set", str(DOOR_CONTROL_PIN), "dh"])
        print(f"GPIO{DOOR_CONTROL_PIN}已设为高电平（使用pinctrl工具）")
        return True
    except Exception as e:
        print(f"设置GPIO高电平失败: {e}")
        return False

def set_gpio_low_pinctrl():
    """使用pinctrl命令行工具设置GPIO为低电平"""
    try:
        subprocess.call(["sudo", "pinctrl", "set", str(DOOR_CONTROL_PIN), "dl"])
        print(f"GPIO{DOOR_CONTROL_PIN}已设为低电平（使用pinctrl工具）")
        return True
    except Exception as e:
        print(f"设置GPIO低电平失败: {e}")
        return False

def cleanup_gpio_pinctrl():
    """清理pinctrl方式的GPIO资源"""
    try:
        # 设置为低电平即可
        subprocess.call(["sudo", "pinctrl", "set", str(DOOR_CONTROL_PIN), "dl"])
        print(f"GPIO{DOOR_CONTROL_PIN}资源已释放（使用pinctrl工具）")
        return True
    except Exception as e:
        print(f"清理GPIO资源失败: {e}")
        return False

# 添加命令行方式控制GPIO（使用raspi-gpio工具，树莓派官方支持）
def setup_gpio_cmd():
    """使用raspi-gpio命令行工具设置GPIO"""
    global gpio_available
    try:
        # 设置为输出模式
        subprocess.call(["raspi-gpio", "set", str(DOOR_CONTROL_PIN), "op"])
        # 设置为低电平
        subprocess.call(["raspi-gpio", "set", str(DOOR_CONTROL_PIN), "dl"])
        print(f"GPIO{DOOR_CONTROL_PIN}初始化完成（使用raspi-gpio命令行）")
        gpio_available = True
        return True
    except Exception as e:
        print(f"使用raspi-gpio设置GPIO失败: {e}")
        
        # 检查raspi-gpio工具是否安装
        try:
            subprocess.call(["which", "raspi-gpio"])
        except:
            print("找不到raspi-gpio命令，请安装：sudo apt-get install raspi-gpio")
        
        return False

def set_gpio_high_cmd():
    """使用raspi-gpio命令行工具设置GPIO为高电平"""
    try:
        subprocess.call(["raspi-gpio", "set", str(DOOR_CONTROL_PIN), "dh"])
        print(f"GPIO{DOOR_CONTROL_PIN}已设为高电平（使用raspi-gpio命令行）")
        return True
    except Exception as e:
        print(f"设置GPIO高电平失败: {e}")
        return False

def set_gpio_low_cmd():
    """使用raspi-gpio命令行工具设置GPIO为低电平"""
    try:
        subprocess.call(["raspi-gpio", "set", str(DOOR_CONTROL_PIN), "dl"])
        print(f"GPIO{DOOR_CONTROL_PIN}已设为低电平（使用raspi-gpio命令行）")
        return True
    except Exception as e:
        print(f"设置GPIO低电平失败: {e}")
        return False

def cleanup_gpio_cmd():
    """清理命令行方式的GPIO资源"""
    try:
        # 设置为低电平即可
        subprocess.call(["raspi-gpio", "set", str(DOOR_CONTROL_PIN), "dl"])
        print(f"GPIO{DOOR_CONTROL_PIN}资源已释放（使用raspi-gpio命令行）")
        return True
    except Exception as e:
        print(f"清理GPIO资源失败: {e}")
        return False

# 作为最后尝试，直接操作 /sys/class/gpio 的替代方法
def setup_gpio_alt():
    """使用替代方法直接操作/sys文件"""
    global gpio_available
    try:
        # 不使用Python的文件打开方式，而是使用命令行方式
        os.system(f"echo {DOOR_CONTROL_PIN} > /sys/class/gpio/export 2>/dev/null || true")
        time.sleep(0.1)
        os.system(f"echo out > /sys/class/gpio/gpio{DOOR_CONTROL_PIN}/direction 2>/dev/null || true")
        os.system(f"echo 0 > /sys/class/gpio/gpio{DOOR_CONTROL_PIN}/value 2>/dev/null || true")
        
        print(f"GPIO{DOOR_CONTROL_PIN}初始化完成（使用替代方法）")
        gpio_available = True
        return True
    except Exception as e:
        print(f"使用替代方法设置GPIO失败: {e}")
        return False

def set_gpio_high_alt():
    """使用替代方法设置GPIO为高电平"""
    try:
        os.system(f"echo 1 > /sys/class/gpio/gpio{DOOR_CONTROL_PIN}/value 2>/dev/null || true")
        print(f"GPIO{DOOR_CONTROL_PIN}已设为高电平（使用替代方法）")
        return True
    except Exception as e:
        print(f"设置GPIO高电平失败: {e}")
        return False

def set_gpio_low_alt():
    """使用替代方法设置GPIO为低电平"""
    try:
        os.system(f"echo 0 > /sys/class/gpio/gpio{DOOR_CONTROL_PIN}/value 2>/dev/null || true")
        print(f"GPIO{DOOR_CONTROL_PIN}已设为低电平（使用替代方法）")
        return True
    except Exception as e:
        print(f"设置GPIO低电平失败: {e}")
        return False

def cleanup_gpio_alt():
    """清理替代方法的GPIO资源"""
    try:
        os.system(f"echo 0 > /sys/class/gpio/gpio{DOOR_CONTROL_PIN}/value 2>/dev/null || true")
        os.system(f"echo {DOOR_CONTROL_PIN} > /sys/class/gpio/unexport 2>/dev/null || true")
        print(f"GPIO{DOOR_CONTROL_PIN}资源已释放（使用替代方法）")
        return True
    except Exception as e:
        print(f"清理GPIO资源失败: {e}")
        return False

# 修改GPIO控制函数，增加pinctrl作为树莓派5的首选方法
def setup_gpio():
    """初始化GPIO，尝试多种方法"""
    global set_gpio_high, set_gpio_low, cleanup_gpio
    
    print(f"尝试初始化GPIO {DOOR_CONTROL_PIN}...")
    
    # 检查系统版本和平台信息
    print("系统信息:")
    os.system("uname -a")
    
    # 检测是否为树莓派5
    is_pi5 = False
    try:
        with open('/proc/device-tree/model', 'r') as f:
            model = f.read()
            if '5' in model:
                is_pi5 = True
                print("检测到树莓派5，将优先使用pinctrl工具")
            print(f"设备型号: {model.strip()}")
    except:
        print("无法读取设备型号")
    
    # 创建方法列表，按优先级排序
    methods = []
    
    # 如果是树莓派5，优先使用pinctrl
    if is_pi5:
        methods.append({
            "name": "pinctrl工具(Pi5专用)",
            "setup": setup_gpio_pinctrl,
            "high": set_gpio_high_pinctrl,
            "low": set_gpio_low_pinctrl,
            "cleanup": cleanup_gpio_pinctrl
        })
    
    # 添加其他通用方法
    methods.extend([
        {
            "name": "命令行方式(raspi-gpio)",
            "setup": setup_gpio_cmd,
            "high": set_gpio_high_cmd,
            "low": set_gpio_low_cmd,
            "cleanup": cleanup_gpio_cmd
        },
        {
            "name": "直接内存访问",
            "setup": setup_gpio_direct,
            "high": set_gpio_direct_high,
            "low": set_gpio_direct_low,
            "cleanup": cleanup_gpio_direct
        },
        {
            "name": "sysfs接口",
            "setup": setup_gpio_sysfs,
            "high": set_gpio_high_sysfs,
            "low": set_gpio_low_sysfs,
            "cleanup": cleanup_gpio_sysfs
        },
        {
            "name": "替代方法",
            "setup": setup_gpio_alt,
            "high": set_gpio_high_alt,
            "low": set_gpio_low_alt,
            "cleanup": cleanup_gpio_alt
        }
    ])
    
    # 尝试每种方法
    for method in methods:
        print(f"\n尝试使用 {method['name']} 初始化GPIO...")
        try:
            if method["setup"]():
                print(f"使用 {method['name']} 初始化GPIO成功!")
                set_gpio_high = method["high"]
                set_gpio_low = method["low"]
                cleanup_gpio = method["cleanup"]
                return True
        except Exception as e:
            print(f"使用 {method['name']} 初始化GPIO失败: {e}")
    
    print("\n警告: 所有GPIO初始化方法均失败！门禁控制将不可用。")
    
    # 如果所有方法都失败，使用空操作函数
    def dummy_function(*args, **kwargs):
        print("GPIO控制不可用")
        return False
    
    set_gpio_high = dummy_function
    set_gpio_low = dummy_function
    cleanup_gpio = dummy_function
    return False

# 更新测试函数以检查是否有sudo权限
def test_gpio_toggle():
    """测试GPIO高低电平切换，帮助诊断问题"""
    print("\n======== 开始GPIO测试序列 ========")
    print(f"使用GPIO: {DOOR_CONTROL_PIN}")
    
    # 检查是否有root权限
    is_root = os.geteuid() == 0 if hasattr(os, 'geteuid') else False
    print(f"运行权限: {'Root/sudo' if is_root else '普通用户'}")
    if not is_root:
        print("警告: 没有sudo权限可能导致GPIO控制失败")
        print("建议使用: sudo python3 eye_detect.py")
    
    # 测试低电平
    print("\n测试设置低电平...")
    result = set_gpio_low()
    print(f"设置低电平结果: {'成功' if result else '失败'}")
    time.sleep(1)
    
    # 测试高电平
    print("\n测试设置高电平...")
    result = set_gpio_high()
    print(f"设置高电平结果: {'成功' if result else '失败'}")
    time.sleep(1)
    
    # 再次测试低电平
    print("\n再次测试设置低电平...")
    result = set_gpio_low()
    print(f"设置低电平结果: {'成功' if result else '失败'}")
    
    print("\n======== GPIO测试序列完成 ========\n")

# dlib 人脸检测器 + 预测器 (shape predictor)
detector = dlib.get_frontal_face_detector()
predictor = dlib.shape_predictor("/home/pi/Desktop/face/shape_predictor_68_face_landmarks.dat")
# 加载 dlib 的人脸识别模型
face_rec_model = dlib.face_recognition_model_v1("/home/pi/Desktop/face/dlib_face_recognition_resnet_model_v1.dat")

# 从文件加载已知特征
with open("/home/pi/Desktop/face/known_faces_features.pkl", "rb") as f:
    known_faces = pickle.load(f)

# 添加不同场景的音频文件路径
KNOWN_AUDIO = "/home/pi/Desktop/face/已开门.mp3"  # 识别到已知身份时播放
UNKNOWN_AUDIO = "/home/pi/Desktop/face/识别失败.mp3"  # 识别到未知身份时播放

# I2S设备配置
I2S_DEVICE = "hw:1,0"  # 根据您的系统设置 card 1: duplexaudio

# 设置音频音量（在程序启动时调用）
def setup_audio():
    try:
        # 尝试多种可能的音量控制方式
        # 1. 尝试设置卡1的音量
        try:
            subprocess.call(["amixer", "-c", "1", "sset", "Master", "80%"])
        except Exception:
            pass
        
        # 2. 尝试默认设备的音量
        try:
            subprocess.call(["amixer", "sset", "Master", "80%"])
        except Exception:
            pass
            
        # 3. 尝试PCM控制器
        try:
            subprocess.call(["amixer", "-c", "1", "sset", "PCM", "80%"])
        except Exception:
            pass
            
        # 4. 尝试列出所有控制器，用于调试
        try:
            result = subprocess.check_output(["amixer", "-c", "1", "controls"]).decode()
            print("可用的音频控制器:")
            print(result)
        except Exception as e:
            print(f"获取音频控制器列表失败: {e}")
            
        print("音频设置完成")
    except Exception as e:
        print(f"设置音频音量时出错: {e}")
        print("继续运行，但音频可能不可用")

# 音频播放功能增强
audio_playing = False
last_played_time = 0
PLAY_INTERVAL = 3  # 音频播放间隔(秒)

def play_audio(audio_file):
    """使用I2S接口播放音频文件"""
    def play_in_background(file):
        global audio_playing
        try:
            # 尝试多种播放方式
            if file.lower().endswith('.wav'):
                # 1. 尝试使用指定设备
                try:
                    subprocess.call(["aplay", "-D", I2S_DEVICE, file])
                except Exception:
                    # 2. 尝试使用默认设备
                    try:
                        subprocess.call(["aplay", file])
                    except Exception as e:
                        print(f"播放WAV失败: {e}")
            elif file.lower().endswith('.mp3'):
                # 1. 尝试使用mpg123+指定设备
                try:
                    subprocess.call(["mpg123", "--quiet", "-a", I2S_DEVICE, file])
                except Exception:
                    # 2. 尝试使用mpg123默认设备
                    try:
                        subprocess.call(["mpg123", "--quiet", file])
                    except Exception as e:
                        print(f"播放MP3失败: {e}")
            else:
                # 尝试使用ffplay
                try:
                    subprocess.call(["ffplay", "-nodisp", "-autoexit", file])
                except Exception as e:
                    print(f"使用ffplay播放失败: {e}")
        except Exception as e:
            print(f"播放音频时出错: {e}")
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
        
            # 识别到已知身份，播放欢迎音频
            if current_time - last_played_time > PLAY_INTERVAL:
                play_audio(KNOWN_AUDIO)
                last_played_time = current_time
                
            # 识别到已知身份，设置GPIO高电平并在3秒后自动关闭
            open_door_with_timeout(3)
                
            return identity, max_prob
        else:
            # 添加这个else分支，处理置信度低的情况
            print(f"置信度过低: {max_prob:.4f}，视为未知身份")
            
            if current_time - last_played_time > PLAY_INTERVAL:
                play_audio(UNKNOWN_AUDIO)
                last_played_time = current_time
                
            # 置信度低，设置GPIO低电平
            set_gpio_low()
                
            return "Low Confidence", max_prob
    else:
        print("没有匹配到已知身份")
    
        # 识别到未知身份，播放警告音频
        if current_time - last_played_time > PLAY_INTERVAL:
            play_audio(UNKNOWN_AUDIO)
            last_played_time = current_time
            
        # 未知身份，设置GPIO低电平
        set_gpio_low()
            
        return "Unknown", 0.0

def detect_eyes_with_dlib(frame):
    gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)
    rects, scores, _ = detector.run(gray, 1, -1)
    
    # 如果没有检测到人脸，则设置GPIO为低电平
    if len(rects) == 0:
        # 设置GPIO低电平
        set_gpio_low()
        print("未检测到人脸，门禁已关闭")
        return frame
    
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

# 调试函数：测试GPIO切换
def test_gpio_toggle():
    """测试GPIO高低电平切换，帮助诊断问题"""
    print("\n======== 开始GPIO测试序列 ========")
    print(f"使用GPIO: {DOOR_CONTROL_PIN}")
    
    # 输出系统信息
    try:
        uname = subprocess.check_output(["uname", "-a"]).decode().strip()
        print(f"系统信息: {uname}")
    except:
        pass
    
    # 测试低电平
    print("\n测试设置低电平...")
    result = set_gpio_low()
    print(f"设置低电平结果: {'成功' if result else '失败'}")
    time.sleep(1)
    
    # 测试高电平
    print("\n测试设置高电平...")
    result = set_gpio_high()
    print(f"设置高电平结果: {'成功' if result else '失败'}")
    time.sleep(1)
    
    # 再次测试低电平
    print("\n再次测试设置低电平...")
    result = set_gpio_low()
    print(f"设置低电平结果: {'成功' if result else '失败'}")
    
    print("\n======== GPIO测试序列完成 ========\n")

def main():
    # 初始化GPIO
    setup_gpio()
    
    # 测试GPIO功能
    test_gpio_toggle()
    
    # 初始化音频
    setup_audio()
    
    cap = cv2.VideoCapture(0)
    
    # 检查摄像头是否正常打开
    if not cap.isOpened():
        print("错误: 无法打开摄像头，请检查设备连接")
        return
    
    print("程序已启动，按'q'键退出")
    
    try:
        while True:
            ret, frame = cap.read()
            if not ret:
                print("警告: 无法从摄像头读取帧，尝试重新连接...")
                # 尝试重新连接摄像头
                cap.release()
                time.sleep(1)
                cap = cv2.VideoCapture(0)
                if not cap.isOpened():
                    print("错误: 摄像头重连失败")
                    break
                continue
            
            try:
                frame = detect_eyes_with_dlib(frame)
                
                # 显示图像
                try:
                    cv2.imshow("Eye Detection (dlib landmarks)", frame)
                except Exception as e:
                    print(f"显示图像失败: {e}")
                
                # 检查键盘输入
                key = cv2.waitKey(1) & 0xFF
                if key == ord('q'):
                    print("用户按下'q'键，退出程序")
                    break
            except Exception as e:
                print(f"处理帧时出错: {e}")
    except KeyboardInterrupt:
        print("用户中断，程序退出")
    except Exception as e:
        print(f"程序出现未处理的异常: {e}")
    finally:
        # 程序退出时释放资源
        print("正在清理资源...")
        cap.release()
        cv2.destroyAllWindows()
        cleanup_gpio()
        print("程序已安全退出")

if __name__ == "__main__":
    main()
