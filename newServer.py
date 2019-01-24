
import socket
import threading
import time

HOST = '192.168.0.73' 
PORT = 65431
userIn = '-'
isInputReq = 1
def getInput():
    global userIn
    global isInputReq
    while isInputReq:
        
        print('Enter 1 to abort mission')
        while 1:
            userIn = input()
            print('input accepted: ' + userIn)
            isInputReq = 0
        
 

t = threading.Thread(target=getInput)
t.daemon = True
t.start()

while 1:
    
     with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
            s.bind((HOST, PORT))
            s.listen()
            print('Waiting for connection...\n')
            conn, addr = s.accept()
            with conn:
               
                print('Connected by', addr)
                while True:
                    print('input: ' + str(userIn))
                    if userIn == '-':
                        conn.sendall(b'-')
                    
                    elif userIn[:5] == 'Time ':
                        conn.sendall(userIn.encode('utf-8'))
                        userIn = '-'
                        isInputReq = 1
                    
                    elif userIn == 'Return':
                        conn.sendall(b'RETURN')
                        userIn = '-'
                        isInputReq = 1
                    elif userIn == 'Fetch':
                        conn.sendall(b'FETCH')
                        fetchedData = conn.recv(16384)
                        print(fetchedData.decode())
                        userIn = '-'
                        isInputReq = 1
                        break
                    elif userIn[:5] == 'Freq ':
                        conn.sendall(userIn.encode('utf-8'))
                        userIn = '-'
                        isInputReq = 1
                    
                    else:
                        print('Incorrect input. Please try again.')
                        conn.sendall(b'-')
                        userIn ='-'
                        isInputReq = 1
                    
                    data = conn.recv(1024)
                    

                    
                    if not data:
                        print("Reached end of data stream")
                        break
                    print(data.decode())
                    
                    
                conn.close()
