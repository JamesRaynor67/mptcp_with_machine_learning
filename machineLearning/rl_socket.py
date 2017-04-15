import socket

class Interacter_socket():

    def __init__(self, host='', port=''):
        self.host = host
        self.port = port
        self.s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        self.s.bind((self.host, self.port))
        self.conn = None
        self.addr = None

    def listen(self):
        print self.host , ':', self.port, ' is listening'
        # The backlog argument specifies the maximum number of queued connections and should be at least 0;
        # the maximum value is system-dependent (usually 5), the minimum value is forced to 0.
        self.s.listen(1)
        self.conn, self.addr = self.s.accept()
        print('Connected by', self.addr)

    def recv(self):
        # TCP connection is a stream of bytes. But here I assume both side send/read socket fast enough.
        # (since both ns3 and RL takes time and makes enough intervals between send/read)
        # If I find further problem in the future, I will fix it.
        try:
            rcv_str = self.conn.recv(65536)
            if not rcv_str:
                return 'THIS BATCH IS END', True
            else:
                return rcv_str, False
        except socket.error:
            print "Error while receive from socket"
            self.conn.close()
            return 'Socekt Error', True

    def send(self, string):
        self.conn.sendall(string)

    def close(self):
        try:
            self.conn.close()
        except:
            print "Error while close socket"
