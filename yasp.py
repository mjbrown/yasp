__author__ = 'mjbrown'

import serial
import array
import time

class yasp(object):
    def __init__(self, port):
        self.ser = serial.Serial(port, timeout=1)

    def send_command(self, cmd, payload):
        cmd_length = len(payload) + 3
        cmd_list = [cmd_length / 256, cmd_length % 256] + [cmd] + payload
        checksum = sum(cmd_list) % 256
        msg_list = [0xFF, 0xFF] + cmd_list + [checksum]
        msg = array.array('B', msg_list)
        self.ser.write(msg)

def Main():
    ysp = yasp("COM5")
    ysp.send_command(0x01, [0x00, 0x02, 0x5C, 0x00, 0x00])
    print ysp.ser.read(100)

if __name__ == "__main__":
    Main()