#!/bin/python3

from pygments import highlight
from pygments.lexers import get_lexer_by_name
from pygments.formatters import HtmlFormatter

import select
import re
import sys
import os 

def log(level, msg):
    colors = {
        "INFO": "32"
    }
    print("\033[34m[disasmd] \033[" + colors[level] + "m" + str(level) + ":  \033[0m" + msg)

os.system("rm *.disasmd.dump")
pygments_lexer = get_lexer_by_name("nasm", stripall=True)
pygments_format = HtmlFormatter()
pid = sys.argv[2]
REGION_SIZE = 2000

def hexdump(data: bytes, want_addr, start_addr):
    def to_printable_ascii(byte):
        return chr(byte) if 32 <= byte <= 126 else "."

    out = ""
    offset = 0
    while offset < len(data):
        chunk = data[offset : offset + 16]
        hex_values = "".join(f"{byte:02x}" for byte in chunk)
        ascii_values = "".join(to_printable_ascii(byte) for byte in chunk)
        color = "#e41c5c"
        if eval(want_addr) > (eval(start_addr) + offset) and eval(want_addr) < (eval(start_addr) + offset + 16):
            color = "#FFFFFF"
        if abs(eval(want_addr) - (eval(start_addr) + offset)) < REGION_SIZE:
            out += f"<font color={color}>{start_addr}</font>:<font color=#87AC81>{offset:06x}</font>    <font color=#586777>{hex_values:<48}</font>    <font color=#ABF6D0>|{ascii_values}|</font>"
            out += "<br>"
        offset += 16
    return out

def header_split(text, header_pattern):
    find_pattern = r'%s.*?(?=%s|$)' % (header_pattern, header_pattern)
    parts = re.findall(find_pattern, text, re.DOTALL)
    return parts

def highlight_line(line):
    new_line = highlight(line, pygments_lexer, pygments_format)
    new_line = new_line.replace('<div class="highlight">', "")
    new_line = new_line.replace("</div>", "")
    new_line = new_line.replace("<span>", "<font>")
    new_line = new_line.replace("</span>", "</font>")
    new_line = new_line.replace("<pre>", "")
    new_line = new_line.replace("</pre>", "")
    new_line = new_line.replace('<span class="nf">', '<font color=#de205d>') #de205d
    new_line = new_line.replace('<span class="nv">', '<font color=red>')
    new_line = new_line.replace('<span class="nb">', '<font color=#85ebff>')
    new_line = new_line.replace('<span class="c1">', '<font color=gray>')
    new_line = new_line.replace('<span class="w">', '<font>&nbsp;')
    new_line = new_line.replace('<span class="o">', '<font color=yellow>')
    new_line = new_line.replace('<span class="p">', '<font color=orange>')
    new_line = new_line.replace('<span class="mh">', '<font color=#c0c1c2>')
    return new_line

def memory_view(addr):
    if addr[:2] == "0x":
        addr = addr[2:]

    output = os.popen("grep rw-p /proc/" + str(pid) + "/maps | awk ' { print $1 }'").read()
    memory_bytes = ""
    start = ""
    addr = eval("0x" + addr)
    actual_start = ""

    for region in output.split():
        spli = region.split("-")
        start = eval("0x" + spli[0])
        end = eval("0x" + spli[1])

        if addr >= start and addr <= end:
            print(start,end)
            log("INFO", "memory view range " + str(hex(start)) + " to " + str(hex(end)))
            if not os.path.exists(region + '.disasmd.dump'):
                os.system('sudo gdb --batch --pid '+str(pid)+' -ex "dump memory '+region+'.disasmd.dump 0x'+spli[0]+' 0x'+spli[1]+'";')

            with open(region + '.disasmd.dump', 'rb') as f:
                memory_bytes = f.read()
            actual_start = start
    if not actual_start: return ""
    return hexdump(memory_bytes, str(hex(addr)), str(hex(actual_start)))

def instruction_view(addr):
    addrint = eval("0x" + addr)
    start_addr = addrint - 100
    out = os.popen('sudo gdb --batch --pid '+str(pid)+' -ex "x/200i ' + str(hex(start_addr)) + '"').read()

    data = ""
    lines = out.split("\n")
    for line in lines:
        if line[:3] == "   ":
            linespli = line.split(":")
            astring = linespli[0]
            a = astring.lstrip().split(" ")[0]
            s = linespli[1]
            s = highlight_line(s)
            a_prefix = "<font color=grey>"
            if addrint == eval(a):
                a_prefix = "<font color=white>"
            data += a_prefix + astring + " .... </font>" + s + "<br>"
    return data

def breakpoint(watch, addr):
    output = os.popen('sudo gdb --batch --pid '+pid+' -ex "'+watch+' *0x'+addr+'" -ex "continue"').read()
    lines = output.split("\n")
    address = lines[-3].split(" ")[0]
    log("INFO", "breakpoint at " + str(address))
    return instruction_view(address[2:])

def goto(address):
    if address[:2] == "0x":
        address = address[2:]
    return instruction_view(address)

def address_value(data_type, address):
    if address[:2] != "0x":
        address = "0x" + address
    data_type_fmt = {
        "int": "d",
        "float": "f",
        "string": "s",
    }[data_type]
    output = os.popen('sudo gdb --batch --pid '+pid+' -ex "x/'+data_type_fmt+' '+address+'"').read()
    print(output)
    return output.split(" ")[-1]

log("INFO", "spawned disasmd id=" + sys.argv[1] + " pid=" + sys.argv[2])
fifo_out = open("/tmp/o_disasmd_pipe_" + sys.argv[1], 'w')
fifo_in = open("/tmp/i_disasmd_pipe_" + sys.argv[1], 'r', os.O_NONBLOCK)

while True:
    select.select([fifo_in],[],[])
    recvdata = fifo_in.readline().replace("\n", "")
    cmd = recvdata.split("|")
    data = ""

    for c in cmd:
        cpart = c.split(" ")
        if len(cpart) == 3 and cpart[0] == "address_value":
            log("INFO", "cmd address_value " + str(cpart[1]) + " " + str(cpart[2]))
            data += address_value(cpart[1], cpart[2])
        if len(cpart) == 2 and cpart[0] == "view":
            log("INFO", "cmd view " + str(cpart[1]))
            data += memory_view(cpart[1])
        if len(cpart) == 2 and cpart[0] == "watch":
            log("INFO", "cmd watch " + str(cpart[1]))
            data += breakpoint("awatch", cpart[1])
        if len(cpart) == 2 and cpart[0] == "rwatch":
            log("INFO", "cmd rwatch " + str(cpart[1]))
            data += breakpoint("rwatch", cpart[1])
        if len(cpart) == 2 and cpart[0] == "wwatch":
            log("INFO", "cmd wwatch " + str(cpart[1]))
            data += breakpoint("watch", cpart[1])
        if len(cpart) == 2 and cpart[0] == "jump":
            log("INFO", "cmd jump " + str(cpart[1]))
            data += goto(cpart[1])

    data += "\0"
    #fifo_out.write("FULLSTOP\n")
    fifo_out.write(data)
    fifo_out.flush()
