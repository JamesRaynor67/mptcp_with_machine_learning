import socket

host = socket.gethostname()
port = 12345                   # The same port as used by the server
s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.connect((host, port))
count = 0
while count < 10:
    # s.sendall(b'Hello, world')
    s.sendall(str(count))
    count += 1
    data = s.recv(1024)
    print('Received', repr(data))
s.close()
