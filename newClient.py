import time
import serial
import re
import socket
import os
import glob


HOST = '192.168.0.73' 
PORT = 65431       

dataToSend = ""
sampleNo = 0
timestamp = ""
voltage = 0.0
isNotObstacle = 0
pressure = 0.0
temperature = 0.0
brightness = 0
isActionCommand = None;
##['10', '2019-1-20 23:44:49', '47.21', '1', '1007.50', '25.18', '945\n']
ser = serial.Serial(port = '/dev/ttyS0',baudrate = 9600)
count = 0
missionStartV = 1
while 1:
    
    try:
    
        if missionStartV:
            ser.write(b'1');
            missionStartV = 0
        
        with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
                s.connect((HOST, PORT))
                print('connected')
                data = s.recv(1024)
                indata = repr(data)
                print(indata[2:6])
                if indata == 'b\'-\'':
                    print('skipping')
                if indata == 'b\'RETURN\'':
                    ser.write(b'RETURN')
                    response = ser.readline().decode()
                    print(response)
                    s.close()
                    break
                if indata == 'b\'FETCH\'':
                        print('Fetching')
                        dirPath = '/home/pi/Desktop/SamplesFolder/*.txt'
                        files = glob.glob(dirPath)
                        
                        for filename in files:
                            f = open(filename, 'r')
                            print(f.read())
                            s.sendall(f.read().encode())
                            f.close()

                        s.close()
                        continue
                if indata[2:6] == 'Freq':
                    ser.write(b'FREQ')
                    ser.write(indata[6:-1].encode())
                    print('Sample rate changing')
                if indata[2:6] == 'Time':
                    ser.write(b'TIME')
                    ser.write(indata[6:-1].encode())
                    print('Mission time changing')
                    
                s.close()

        line = ser.readline().decode()

        dataArray = re.split("#", str(line))
        if re.sub(r'\D', "", dataArray[0]) == "1":
            print("Action 1")
            with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
                s.connect((HOST, PORT))
                s.sendall((b'Adjusting course to avoid an obstacle...\n'))
                s.close()
                
        elif re.sub(r'\D', "", dataArray[0]) == "2":
            print("Action 2")
            with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
                s.connect((HOST, PORT))
                s.sendall((b'Voltage is dangerously low! Retreating back to base! Mission aborted. \n'))
                s.close()
                break
        elif re.sub(r'\D', "", dataArray[0]) == "3":
            print("Action 3")
            with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
                s.connect((HOST, PORT))
                s.sendall((b'Time\'s up! Mission over.\n'))
                s.close()
        
        print(dataArray)
        timestamp = dataArray[1]
        voltage = dataArray[2]
        isNotObstacle = dataArray[3]
        pressure = dataArray[4]
        temperature = dataArray[5]
        brightness = re.sub(r'\D', "", dataArray[6])
        file_name = str(timestamp)
        file = open("SamplesFolder/" + file_name + ".txt", "w+")
        file.write("Pressure: " + str(pressure) + " hPa\n")
        file.write("Temperature: " + str(temperature) + " C\n")
        file.write("Brightness level: " + str(brightness) + "\n")
        file = open("SamplesFolder/" + file_name + ".txt", "r")
        dataToSend += "Sample datetime:\n" + file_name + "\n" + file.read() + "======\n"
        
        count = count + 1
        if count == 10:
            with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
                s.connect((HOST, PORT))
                s.sendall(dataToSend.encode("utf-8"))
                dataToSend = ""
                count = 0
                s.close()
    except ConnectionResetError:
        print('Data batch sent.')
        
print("Mission finished, robot back at base.") 
