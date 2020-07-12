# This file is part of the MicroPython project, http://micropython.org/
#
# The MIT License (MIT)
#
# Copyright (c) 2020 Glenn Ruben Bakke
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
# THE SOFTWARE

import network, socket
n = network.NRF91()

dir_names = ["ca", "pub-cert", "priv-cert", "psk", "identity", "pub-key"]
dir_levels = {key:value for value, key in enumerate(dir_names)}

class SecFSException(Exception):
    pass

class CMNG:
    def __init__(self):
        pass
    
    def _query(self, opcode, sec_tag, cred_type, content=""):
        at = socket.socket(socket.AF_LTE, socket.SOCK_DGRAM, socket.PROTO_AT)
        at.send("AT+CMEE=1") # Subscribe to error notifications.
        r = at.recv(20)
        r_s = str(r, "utf-8")
        if not "OK\r\n" in r_s:
            raise SecFSException(27) # MP_EFBIG - File too large.
        query = ",".join(["AT%CMNG=" + str(opcode), str(sec_tag), str(cred_type), content])
        # Check that it can send the data on AT socket.
        if len(query) > 4096:
            raise SecFSException(27) # MP_EFBIG - File too large.
        at.send(query)
        c = at.recv(4096) # Max payload size on AT.
        at.send("AT+CMEE=0") # Subscribe to error notifications.
        r = at.recv(20)
        at.close()
        r_s = str(r, "utf-8")
        c_s = str(c, "utf-8")
        if not "OK\r\n" in r_s:
            raise SecFSException(27) # MP_EFBIG - File too large.
        if "+CME ERROR:" in c_s:
            cme_error = int(c_s[c_s.find(':')+1:c_s.find("\r")].strip())
            if cme_error == 513:
                # Not found. Applies to read, write and delete.
                raise SecFSException(5) # MP_ENOENT - No such file or directory.
            elif cme_error == 514:
                # No access. Applies to read, write and delete.
                raise SecFSException(13) # MP_EACCES - MP_EACCES.
            elif cme_error == 515:
                # Memory full. Applies to write.
                raise SecFSException(28) # MP_ENOSPC - No space left on device.
            elif cme_error == 518:
                # Not allowed in active state.
                raise SecFSException(16) # MP_EBUSY - Device or resource busy.
            else:
                # unknown CME error.
                raise SecFSException(5) # MP_EIO - I/O error
        if "OK\r\n" in c_s:
            return c_s[0:c_s.find("OK\r\n")]
        return None

    def list(self, cred_type):
        retval = []
        r = self._query(1, "", cred_type)
        if (r):
            split = r.split("\r\n")
            for e in split:
                if e:
                    e = e.lstrip("%CMNG: ")
                    e = e[0:e.find(',')]
                    retval.append(e)
            return retval
        return retval

    def read(self, cred_type, sec_tag):
        r = self._query(2, sec_tag, cred_type)
        if r:
            return r.split(",")[-1].rstrip().replace('"', '')
        return None

    def write(self, cred_type, sec_tag, content):
        r = self._query(0, sec_tag, cred_type, '"' + content + '"')
        if r != None:
            return len(content)
        return -1

    def delete(self, cred_type, sec_tag=""):
        r = self._query(3, sec_tag, cred_type)
        if r != None:
            return ""
        return None

class VSecFSFile:
    def __init__(self, name, folder, content="", mode='r'):
        self.name = name
        self.content = content
        self.mode = mode
        self.seek = 0
        self.folder = folder
        self.r_status = False
        self.w_status = False
        self.open = True

    def __str__(self):
        return self.name

    def __repr__(self):
        return self.name

    def __getitem__(self, item):
        return self.name

    def read(self, rlen=-1):
        if not self.open:
            raise SecFSException(9) # MP_EBADF - Bad file number
        if 'r' in self.mode:
            if not self.r_status:
                self.content = CMNG().read(self.folder, self.name)
                self.r_status = True

            if self.seek > 0 and self.seek > rlen and rlen == -1:
                return "" # EOF

            if rlen == -1:
                self.seek = len(self.content) + 1            
                return self.content
            else:
                buf = self.content[seek:rlen]
                self.seek = rlen
                return buf
        raise SecFSException(1) # MP_EPERM - Operation not permitted

    def write(self, content=""):
        if not self.open:
            raise SecFSException(9) # MP_EBADF - Bad file number
        # Write through
        if 'w' in self.mode:
            r = CMNG().write(self.folder, self.name, content)
            if r > -1:
                self.seek = 0
                self.r_status = False # Reset status of read
                return r
        raise SecFSException(1) # MP_EPERM - Operation not permitted

    def close(self):
        if not self.open:
            raise SecFSException(9) # MP_EBADF - Bad file number
        self.open = False

class IterVSecFSFile:
    def __init__(self, level):
        self.level = level
        self.files = []
        if level == -1:
            self.files = [VSecFSFile(name, level) for level, name in enumerate(dir_names)]
        else:
            self.files = [VSecFSFile(name, level) for name in CMNG().list(level)]
        self.max = len(self.files)

    def __iter__(self):
        self.n = 0
        return self

    def __next__(self):
 
        if self.n < self.max:
            r = self.files[self.n]
            self.n += 1
            return r
        else:
            raise StopIteration

class SecFS:
    def __init__(self):
        self.level = -1

    def _get_folder_index(self, path):
        path = path.lstrip("/")
        if not path:
            return -1
        else:
            paths = path.split("/")
            try:
                return dir_levels[paths[0]]
            except KeyError:
                raise SecFSException(20) # MP_ENOTDIR - Not a directory.

    def mount(self, read_only=True, mkfs=False):
        pass
    
    def open(self, path, mode="r"):
        level = self.level
        if path.find("/") > -1:
            level = self._get_folder_index(path)
        filename = path.split("/")[-1]
        # verify filename
        if int(filename) >= 0 and int(filename) <= 2147483647:
            return VSecFSFile(filename, level, mode=mode)
        raise SecFSException(22) # MP_EINVAL - Invalid argument.

    def remove(self, path):
        level = self.level
        if path.find("/") > -1:
            level = self._get_folder_index(path)        
        filename = path.split("/")[-1]
        if not filename:
            raise SecFSException(21) # MP_EISDIR - Is a directory.
        CMNG().delete(level, filename)

    def rmdir(self, path):
        level = self._get_folder_index(path)
        if level == -1:
            for d in dir_levels.values():
                for f in iter(IterVSecFSFile(d)):
                    CMNG().delete(d, f)
            return
        else:
            for f in iter(IterVSecFSFile(level)):
                CMNG().delete(level, f)
            return
        raise SecFSException("SecFS rmdir not implemented")

    def chdir(self, path):
        self.level = self._get_folder_index(path)

    def getcwd(self):
        if self.level == -1:
            return ""
        else:
            return "/" + dir_names[self.level]

    def stat(self, file_name):
        pass

    def ilistdir(self, path):
        level = self._get_folder_index(path)
        # if within a folder.
        if level == -1 and path == "":
            level = self.level
        it = iter(IterVSecFSFile(level))
        return it

import uos
secfs = SecFS()
uos.mount(secfs, "/sec")
