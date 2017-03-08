import socket
from time import sleep

host = ''        # Symbolic name meaning all available interfaces
port = 12345     # Arbitrary non-privileged port
s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.bind((host, port))

print host , port
s.listen(1)
conn, addr = s.accept()
print('Connected by', addr)
while True:
    try:
        data = conn.recv(1024)

        if not data: break

        print "Client Says: " + data
        sleep(0.5)
        conn.sendall("Server Says: " + str(int(data)*2))

    except socket.error:
        print "Error Occured."
        break

conn.close()
