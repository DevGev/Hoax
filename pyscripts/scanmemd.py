#!/bin/python3

import sys
import time
import select
import os
import ctypes
import os
import re
import tempfile
import misc
import io

def log(level, msg):
    colors = {
        "INFO": "32"
    }
    print("\033[34m[scanmemd] \033[" + colors[level] + "m" + str(level) + ":  \033[0m" + msg)

class Scanmem():
    """Wrapper for libscanmem."""
    
    LIBRARY_FUNCS = {
        'sm_init' : (ctypes.c_bool, ),
        'sm_cleanup' : (None, ),
        'sm_backend_exec_cmd' : (None, ctypes.c_char_p),
        'sm_get_num_matches' : (ctypes.c_ulong, ),
        'sm_get_scan_progress' : (ctypes.c_double, ),
        'sm_set_stop_flag' : (None, ctypes.c_bool),
    }

    def __init__(self, libpath='./libscanmem.so'):
        self._lib = ctypes.CDLL(libpath)
        self._init_lib_functions()
        self._lib.sm_init()

    def _init_lib_functions(self):
        for k,v in Scanmem.LIBRARY_FUNCS.items():
            f = getattr(self._lib, k)
            f.restype = v[0]
            f.argtypes = v[1:]

    def send_command(self, cmd, get_output = False):
        """
        Execute command using libscanmem.
        This function is NOT thread safe, send only one command at a time.
        
        cmd: command to run
        get_output: if True, return in a string what libscanmem would print to stdout
        """
        if get_output:
            with tempfile.TemporaryFile() as directed_file:
                backup_stdout_fileno = os.dup(sys.stdout.fileno())
                os.dup2(directed_file.fileno(), sys.stdout.fileno())

                self._lib.sm_backend_exec_cmd(ctypes.c_char_p(misc.encode(cmd)))

                os.dup2(backup_stdout_fileno, sys.stdout.fileno())
                os.close(backup_stdout_fileno)
                directed_file.seek(0)
                return directed_file.read()
        else:
            self._lib.sm_backend_exec_cmd(ctypes.c_char_p(misc.encode(cmd)))

    def get_match_count(self):
        return self._lib.sm_get_num_matches()

    def get_scan_progress(self):
        return self._lib.sm_get_scan_progress()

    def set_stop_flag(self, stop_flag):
        """
        Sets the flag to interrupt the current scan at the next opportunity
        """
        self._lib.sm_set_stop_flag(stop_flag)

    def exit_cleanup(self):
        """
        Frees resources allocated by libscanmem, should be called before disposing of this instance
        """
        self._lib.sm_cleanup()
    
    def matches(self):
        """
        Returns a generator of (match_id_str, addr_str, off_str, region_type, value, types_str) for each match, all strings.
        The function executes commands internally, it is NOT thread safe
        """
        list_bytes = self.send_command('list', get_output=True)
        lines = filter(None, misc.decode(list_bytes).split('\n'))
        
        line_regex = re.compile(r'^\[ *(\d+)\] +([\da-f]+), +\d+ \+ +([\da-f]+), +(\w+), (.*), +\[([\w ]+)\]$')
        for line in lines:
            yield line_regex.match(line).groups()

backend = Scanmem()
backend.send_command('pid %d' % (int(sys.argv[2]),))
fifo_out = open("/tmp/o_scanmemd_pipe_" + sys.argv[1], 'w')
fifo_in = open("/tmp/i_scanmemd_pipe_" + sys.argv[1], 'r', os.O_NONBLOCK)

log("INFO", "spawned scanmemd id=" + sys.argv[1] + " pid=" + sys.argv[2])

while True:
    select.select([fifo_in],[],[])
    recvdata = fifo_in.readline().replace("\n", "")

    cmd = recvdata.split("|")
    for c in cmd:
        if not len(c): continue
        log("INFO", "scanmem command " + c)
        backend.send_command(c)
    matches = backend.matches()

    data = ""
    av_lines = 1000
    for (mid_str, addr_str, off_str, rt, val, t) in matches:
        if len(val) > 12:
            val = val[:10]
            val += "..."
        if av_lines > 0: data += "  " + addr_str + "\t\t" + off_str + "\t\t" + val + "\n"
        av_lines -= 1
        #data += mid_str + delim + addr_str + delim + off_str + delim + rt + delim + val + delim + t + "\n"
    data += "\0"
    #fifo_out.write("FULLSTOP\n")
    fifo_out.write(data)
    fifo_out.flush()

close(fifo_in)
close(fifo_out)
