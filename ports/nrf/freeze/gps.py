import sys, network, socket
n = network.NRF91()

AT_XSYSTEMMODE = "AT%XSYSTEMMODE=1,0,1,0"
AT_MAGPIO = "AT%XMAGPIO=1,0,0,1,1,1574,1577"

at = socket.socket(socket.AF_LTE, socket.SOCK_DGRAM, socket.PROTO_AT)
at.send(AT_XSYSTEMMODE)
at.recv(20)
at.send(AT_MAGPIO)
at.recv(20)

#if sys.platform == "nrf9160-DK":
AT_COEX0  = "AT%XCOEX0=1,1,1570,1580"
at.send(AT_COEX0)
at.recv(20)

n.connect()

gps = socket.socket(socket.AF_LOCAL, socket.SOCK_DGRAM, socket.PROTO_GNSS)
gps.setsockopt(socket.SOL_GNSS, socket.SO_GNSS_FIX_RETRY, bytearray([0, 0]))
gps.setsockopt(socket.SOL_GNSS, socket.SO_GNSS_FIX_INTERVAL, bytearray([1, 0]))

nmea_mask = (
    socket.GNSS_NMEA_CONFIG_GGA_FLAG |
    socket.GNSS_NMEA_CONFIG_GLL_FLAG |
    socket.GNSS_NMEA_CONFIG_GSA_FLAG |
    socket.GNSS_NMEA_CONFIG_GSV_FLAG |
    socket.GNSS_NMEA_CONFIG_RMC_FLAG)
gps.setsockopt(socket.SOL_GNSS, socket.SO_GNSS_NMEA_MASK, bytearray([nmea_mask, 0]))
gps.setsockopt(socket.SOL_GNSS, socket.SO_GNSS_START, bytearray([0, 0, 0, 0]))
count = 0
while 1:
    count += 1
    bytes = gps.recv(580)
    if (bytes[0] == 2):
        print("NMEA:%s" % str(bytes[8:], 'utf-8'))
