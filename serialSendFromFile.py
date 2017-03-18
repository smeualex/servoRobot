import time
import serial

ser = serial.Serial('COM5', 115200)

def serialSend(stringData):
    print("sending cmd - " + stringData)
    ser.write(bytes(stringData.encode('ascii')))
    time.sleep(0.05)

with open('./logs/1_serialCommands.out') as f:
    commands = f.readlines()
    commands = [x.strip() for x in commands] 
    for cmd in commands:
        serialSend(cmd)
