import network
import os
from umqtt import simple
from tls_sock import TLSSock

class MQTTClient(simple.MQTTClient):
    def connect(self, clean_session=True):
        self.sock = tls_sock.TLSSock()
        addr = socket.getaddrinfo(self.server, self.port)[0][-1]
        self.sock.connect(addr)
        premsg = bytearray(b"\x10\0\0\0\0\0")
        msg = bytearray(b"\x04MQTT\x04\x02\0\0")

        sz = 10 + 2 + len(self.client_id)
        msg[6] = clean_session << 1
        if self.user is not None:
            sz += 2 + len(self.user) + 2 + len(self.pswd)
            msg[6] |= 0xC0
        if self.keepalive:
            assert self.keepalive < 65536
            msg[7] |= self.keepalive >> 8
            msg[8] |= self.keepalive & 0x00FF
        if self.lw_topic:
            sz += 2 + len(self.lw_topic) + 2 + len(self.lw_msg)
            msg[6] |= 0x4 | (self.lw_qos & 0x1) << 3 | (self.lw_qos & 0x2) << 3
            msg[6] |= self.lw_retain << 5

        i = 1
        while sz > 0x7f:
            premsg[i] = (sz & 0x7f) | 0x80
            sz >>= 7
            i += 1
        premsg[i] = sz

        self.sock.write(premsg, i + 2)
        self.sock.write(msg)
        #print(hex(len(msg)), hexlify(msg, ":"))
        self._send_str(self.client_id)
        if self.lw_topic:
            self._send_str(self.lw_topic)
            self._send_str(self.lw_msg)
        if self.user is not None:
            self._send_str(self.user)
            self._send_str(self.pswd)
        resp = self.sock.read(4)
        assert resp[0] == 0x20 and resp[1] == 0x02
        if resp[3] != 0:
            raise MQTTException(resp[3])
        return resp[2] & 1

n = network.NRF91()
n.connect()

while not n.ifconfig():
    pass

print(n.ifconfig())

# General
AWS                = "$aws/things/"
SHADOW_BASE_TOPIC  = AWS + "%s/shadow"
# RX
ACCEPTED_TOPIC     = AWS + "%s/shadow/get/accepted"
REJECTED_TOPIC     = AWS + "%s/shadow/get/rejected"
UPDATE_DELTA_TOPIC = AWS + "%s/shadow/update/delta"
# TX
UPDATE_TOPIC       = AWS + "%s/shadow/update"
SHADOW_GET         = AWS + "%s/shadow/get"

hostname = "a2n7tk1kp18wix-ats.iot.us-east-1.amazonaws.com\0"
client_id = "nrf-352656100039703"
port = 8883
#ip = socket.getaddrinfo(hostname, port)[0][-1][0]
#print("IP", ip)

sec_tag_found = False
sec_tag = 16842753
os.chdir("/secure/ca")
if str(sec_tag) in os.listdir():
    sec_tag_found = True

if sec_tag_found:
    print("Sec TAG found!")
    m = simple.MQTTClient(client_id, hostname, port)
    print(m)

else:
    print("Sec TAG NOT found!")
