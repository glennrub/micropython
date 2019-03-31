import socket
import os

class TLSSock:
    def __init__(self, sec_tag, peer_verify=False):
        st_bytes = sec_tag.to_bytes(4, 10)
        sec_tag_bytearray = bytearray([st_bytes[3], st_bytes[2], st_bytes[1], st_bytes[0]])
        s = socket.socket(socket.AF_INET, socket.SOCK_STREAM, socket.SPROTO_TLS1v2)
        if socket:
            if peer_verify:
                try:
                    s.setsockopt(socket.SOL_SECURE, socket.SO_SEC_PEER_VERIFY, 2) #0 = none, 1 = optional, 2 = strict
                except Exception:
                    print("Setting PEER VERIFY failed.")
                    s.close()

            try:
    	        s.setsockopt(socket.SOL_SECURE, socket.SO_SEC_TAG_LIST, sec_tag_bytearray)
	    except Exception:
	        print("Setting SEC TAG LIST failed.")
	    s.close()
#           try:        
#    	        s.setsockopt(socket.SOL_SECURE, socket.SO_SEC_HOSTNAME, hostname)
#	    except Exception:
#	        print("Setting HOSTNAME failed.")
#	        s.close()
#
        return s
