import json
import rclpy
from rclpy.node import Node
from geometry_msgs.msg import Twist
import serial
# there is some basic clamping but its assumed that your arduino code is to handle clamp , deadband, and watch dog
SPD_SCALE = 100  # sets spd -100,100
WATCHDOG_TIMEOUT = 1.0  # stops when cmd_vel goes down

UART_PORT = '/dev/serial0'   # /dev/ttyAMA0
UART_BAUD = 115200           


class DiffDrive(Node):
    def __init__(self):
        super().__init__('diff_drive')
        self.create_subscription(Twist, 'cmd_vel', self.on_cmd, 10)
        self.create_timer(WATCHDOG_TIMEOUT, self.watchdog)
        self.last_cmd = self.get_clock().now()

        try:
            self.ser = serial.Serial(port=UART_PORT, baudrate=UART_BAUD, timeout=1)
            self.get_logger().info(f'UART port {UART_PORT} opened successfully.')
        except serial.SerialException as e:
            self.get_logger().error(f'Serial error: {e}')
            self.ser = None

    def on_cmd(self, msg):
        self.last_cmd = self.get_clock().now()
        v, w = msg.linear.x, msg.angular.z

        if abs(w) > abs(v):
            cmd = 'LEFT' if w > 0 else 'RIGHT'
            spd = max(-100, min(100, int(w * SPD_SCALE)))
        else:
            cmd = 'FWD' if v > 0 else 'BACK'
            spd = max(-100, min(100, int(v * SPD_SCALE)))

        self.send(cmd, spd)

    def watchdog(self):
        age = (self.get_clock().now() - self.last_cmd).nanoseconds * 1e-9
        if age > WATCHDOG_TIMEOUT:
            self.send('STOP', 0)

    def send(self, cmd, spd):
        payload = json.dumps({'cmd': cmd, 'spd': spd, 'pri': 1})
        if self.ser is None or not self.ser.is_open:
            self.get_logger().warn(f'UART not open, dropping: {payload}')
            return
        try:
            self.ser.write((payload + '\n').encode('utf-8'))
        except serial.SerialException as e:
            self.get_logger().error(f'Serial error: {e}')

    def destroy_node(self):
        if getattr(self, 'ser', None) is not None and self.ser.is_open:
            self.ser.close()
            self.get_logger().info('UART port closed.')
        super().destroy_node()


def main(args=None):
    rclpy.init(args=args)
    node = DiffDrive()
    try:
        rclpy.spin(node)
    except KeyboardInterrupt:
        pass
    finally:
        node.destroy_node()
        rclpy.shutdown()


if __name__ == '__main__':
    main()
