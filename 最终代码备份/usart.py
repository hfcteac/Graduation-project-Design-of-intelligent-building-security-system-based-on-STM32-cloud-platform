import serial
import time

def serial_communication(port, baudrate):
    try:
        # 打开串口
        ser = serial.Serial(port, baudrate, timeout=1)
        if ser.is_open:
            print(f"串口 {port} 打开成功，波特率 {baudrate}")
        else:
            print(f"串口 {port} 打开失败")
            return
        
        # 发送数据
        def send_data(data):
            if ser.is_open:
                ser.write(data.encode())  # 将字符串编码为字节发送
                print(f"发送数据: {data}")
            else:
                print("串口未打开，无法发送数据")

        # 接收数据
        def receive_data():
            if ser.is_open:
                data = ser.readline().decode().strip()  # 读取一行数据并解码
                if data:
                    print(f"接收到的数据: {data}")
            else:
                print("串口未打开，无法接收数据")

        # 示例：发送和接收循环
        while True:
            send_data("Hello, Serial!")  # 示例发送
            time.sleep(1)  # 延时 1 秒
            receive_data()  # 尝试接收数据
            
    except serial.SerialException as e:
        print(f"串口错误: {e}")
    except KeyboardInterrupt:
        print("用户中断")
    finally:
        if 'ser' in locals() and ser.is_open:
            ser.close()
            print(f"串口 {port} 已关闭")

if __name__ == "__main__":
    # 示例：使用 COM3 串口和 9600 波特率
    serial_communication(port="/dev/ttyS1", baudrate=115200)
