import socket
import sys
import _thread
import time

sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
server_address = ('localhost', 9090)
#print >>sys.stderr, 'starting up on %s port %s' % server_address
sock.bind(server_address)

pan = 0
tilt = 0

res_pan = 0 
res_tilt = 0

sock.listen(1)

def adjust_antena( threadName):
    global pan, tilt, res_pan, res_tilt
    while True:
        if res_pan > pan:
            pan += 1
        if res_pan < pan:
            pan -= 1
        if res_tilt > tilt:
            tilt += 1
        if res_tilt < tilt:
            tilt -= 1 
        time.sleep(0.05)
     
try:
    _thread.start_new_thread( adjust_antena, ("Thread-1",))
except:
  pass


while True:
    # Wait for a connection
    #print >>sys.stderr, 'waiting for a connection'
    connection, client_address = sock.accept()

    try:
        #print >>sys.stderr, 'connection from', client_address

        # Receive the data in small chunks and retransmit it
        while True:
            data = connection.recv(1024)
            strData =  data.decode('ascii')
            if(len(strData) == 0):
                connection.sendall(data)
                continue 

            # command 

            if(strData[0] == '!'):
                strDataCommand = strData[1:]
                lsCommand = strDataCommand.split(); 
                if(lsCommand[1] == '1'): 
                    res_pan = int(lsCommand[2])
                if(lsCommand[1] == '2'):
                    res_tilt = int(lsCommand[2]) 
            elif strData[0] == '?':
                strDataCommand = strData[1:]
                lsCommand = strDataCommand.split(); 
                strOut = 'F='
                if(lsCommand[1] == '1'): 
                    strOut = strOut + str(pan) ;
                if(lsCommand[1] == '2'):
                    strOut = strOut + str(tilt); 
                MESSAGE = bytearray(strOut, 'ascii')
                data += MESSAGE
                data += bytearray('\r', 'ascii')
                #connection.sendall(data)
            print(data)
            connection.sendall(data)
    finally:
        # Clean up the connection
        connection.close()