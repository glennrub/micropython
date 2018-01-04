import ubluepy

KEY_SHIFT = const(0x2)

def event_handler(id, handle, data):
    if id == ubluepy.constants.EVT_GAP_CONNECTED:
        print("GAP Connected")
    elif id == ubluepy.constants.EVT_GAP_DISCONNECTED:
        print("GAP Disconnected")
    elif id == ubluepy.constants.EVT_GATTS_WRITE:
        print("GATTS write")
    elif id == 21:
        print("Enter passkey: ", data)
    elif id == 57: # BLE_GATTC_EVT_HVX
        char_idx = data[1]
        extr_idx = data[0]
        if char_idx >= 4 and char_idx <= 29:
            if (extr_idx == KEY_SHIFT):
                print("char:", chr(char_idx + 61), data)
            else:
                print("char:", chr(char_idx + 93), data)
        else:
            print(data)
    else:
        print("Unhandled event", id)

def bytes_to_str(bytes):
    string = ""
    for b in bytes:
        string += chr(b)
    return string

def get_device_names(scan_entries):
    dev_names = []
    for e in scan_entries:
        scan = e.getScanData()
        if scan:
            for s in scan:
                if s[0] == ubluepy.constants.ad_types.AD_TYPE_COMPLETE_LOCAL_NAME:
                    dev_names.append((e, bytes_to_str(s[2])))
    return dev_names

def find_device_by_name(name):
    s = ubluepy.Scanner()
    scan_res = s.scan(30)

    device_names = get_device_names(scan_res)
    for dev in device_names:
        if name == dev[1]:
            return dev[0]

dev = None
p = None

# connect to the BLE HID keyboard
while not dev:
    dev = find_device_by_name("UniversalFoldableKb")

p = ubluepy.Peripheral()
p.setConnectionHandler(event_handler)
bond_data = None
try:
    f = open("bond.txt", "r")
    bond_data = f.read()
    f.close()
except Exception:
    pass

c = p.connect(dev.addr(), addr_type=dev.addr_type(), pair=True, bond_data=bond_data)

if c:
    f = open("bond.txt", "w")
    f.write(c)
    f.close()

uuid_report = ubluepy.UUID("0x2a4d")

service_hid = p.getServices()[2]

chr_report = service_hid.getCharacteristic(uuid_report)
chr_descs = chr_report.getDescriptors()
desc_cccd = chr_descs[1]
desc_cccd.write(bytearray([0x01, 0x00]))

