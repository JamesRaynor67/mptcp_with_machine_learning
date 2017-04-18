import socket
from time import sleep

def IsInt(s):
    # A naive method, but enough here
    if "." in s:
        return False
    else:
        return True

class Train_data:

    def __init__(self):
        self.next_seq_num = 0
        self.train_data = {}

    def add_one_string(self, str_data):
        # name$value#name$value...
        pair_list = str_data.split("$")
        one_row_of_train_data = {}
        for pair in pair_list:
            if len(pair) > 3:
                name_val_list = pair.split("#")
                # print "Hong Jiaming: 1 ", pair, len(name_val_list), name_val_list[0], name_val_list[1]
                if IsInt(name_val_list[1]):
                    one_row_of_train_data[name_val_list[0]] = int(name_val_list[1])
                else:
                    one_row_of_train_data[name_val_list[0]] = float(name_val_list[1])

        assert one_row_of_train_data["ssn"] == self.next_seq_num
        self.train_data[self.next_seq_num] = one_row_of_train_data
        self.next_seq_num += 1

    def get_train_data_dic(self):
        return self.train_data

    def get_latest_train_data(self):
        return self.train_data[self.next_seq_num-1]

    def print_all_train_data(self):
        print "dic size: ", len(self.train_data)
        for ssn, data in self.train_data.iteritems():
            for k, v in data.iteritems():
                print "key: ", k, "value: ", v

    def print_latest_train_data(self):
        latest_data = self.train_data[self.next_seq_num-1]
        for k, v in latest_data.iteritems():
            print "key: ", k, "value: ", v

# Return a string
def get_new_cWnd(train_data):
    if train_data["m_cWnd"] < train_data["m_ssThresh"]:
        # in slow-start state
        return str(train_data["m_cWnd"] + train_data["m_segmentSize"])
    else:
        alpha_scale = 1;
        adder = min(train_data["m_alpha"] * train_data["m_segmentSize"] * train_data["m_segmentSize"] / (train_data["totalCwnd"]* alpha_scale),
                    train_data["m_segmentSize"] * train_data["m_segmentSize"] / train_data["m_cWnd"]);
        print "--- Set m_cWnd to ", train_data["m_cWnd"] + adder
        return str(train_data["m_cWnd"] + adder);

if __name__ == "__main__":
    train_data = Train_data()
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
            rcv_str = conn.recv(1024)
            if not rcv_str:
                break
            else:
                print rcv_str
                train_data.add_one_string(rcv_str)
                train_data.print_latest_train_data()
                # conn.sendall("10000")
                conn.sendall(get_new_cWnd(train_data.get_latest_train_data()))
        except socket.error:
            conn.close()
            print "Error Occured."
            break

    conn.close()


#   // Increase of cwnd based on current phase (slow start or congestion avoidance)
#   if (tcb->m_cWnd < tcb->m_ssThresh)
#   {
#     tcb->m_cWnd += tcb->m_segmentSize;
#     NS_LOG_INFO ("In SlowStart, updated tcb " << tcb << " cwnd to " << tcb->m_cWnd << " ssthresh " << tcb->m_ssThresh);
#   }
#   else
#   {
#     Ptr<MpTcpMetaSocket> metaSock = DynamicCast<MpTcpMetaSocket>(tcb->m_socket);
#     uint32_t totalCwnd = metaSock->GetTotalCwnd ();
#
#     m_alpha = ComputeAlpha (metaSock, tcb);
#     double alpha_scale = 1;
# //         The alpha_scale parameter denotes the precision we want for computing alpha
# //                alpha  bytes_acked * MSS_i   bytes_acked * MSS_i
# //          min ( --------------------------- , ------------------- )  (3)
# //                 alpha_scale * cwnd_total              cwnd_i
#
#   double adder = std::min (m_alpha* tcb->m_segmentSize * tcb->m_segmentSize / (totalCwnd* alpha_scale),
#       static_cast<double>((tcb->m_segmentSize * tcb->m_segmentSize) / tcb->m_cWnd.Get ()));
#
#   // Congestion avoidance mode, increase by (segSize*segSize)/cwnd. (RFC2581, sec.3.1)
#     // To increase cwnd for one segSize per RTT, it should be (ackBytes*segSize)/cwnd
#
# //    adder = std::max (1.0, adder);
#     tcb->m_cWnd += static_cast<uint32_t> (adder);
#     NS_LOG_INFO ("In CongAvoid, updated tcb " << tcb << " cwnd to " << tcb->m_cWnd << " ssthresh " << tcb->m_ssThresh);
#   }
