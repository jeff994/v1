import socket
import time


TCP_IP = '127.0.0.1'
TCP_PORT = 9090
BUFFER_SIZE = 1024
MESSAGE_ex = bytearray("!EX\r", 'ascii')
MESSAGE_mg = bytearray("!MG\r", 'ascii')
MESSAGE_g = bytearray("!G 1 500\r", 'ascii')
MESSAGE_r = bytearray("!r\r", 'ascii')


s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.connect((TCP_IP, TCP_PORT))
while True:
	s.send(MESSAGE_r)
	data = s.recv(1024)
	time.sleep(1);
	print(data.decode("ascii"))
s.close()
