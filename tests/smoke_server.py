#!/usr/bin/env python3
"""
tests/smoke_server.py: kiểm thử tích hợp device_sim qua TCP (thay cho gõ tay bằng nc)

Cách dùng (device_sim phải đang chạy với coop_A):
    ./bin/device_sim -c config/coop_A.conf          # terminal 1
    python3 tests/smoke_server.py                    # terminal 2

Mỗi kiểm tra tương ứng một mục trong docs/tuan1_code.md (tiêu chí 1 và 2).
Tương đương bằng nc:
    (printf 'HEL'; sleep 1; printf 'LO\\nINFO FAN-xxxx\\n') | nc localhost 7000
"""
import socket
import sys
import time

HOST = "127.0.0.1"
PORT = int(sys.argv[1]) if len(sys.argv) > 1 else 7000
STATE = "data/devices_coop_A.conf"
fails = 0


def check(cond, desc):
    global fails
    print(("[PASS] " if cond else "[FAIL] ") + desc)
    if not cond:
        fails += 1


def connect():
    s = socket.create_connection((HOST, PORT), timeout=3)
    s.settimeout(3)
    return s


def read_lines(s, n):
    """Đọc đúng n dòng (TCP có thể trả nhiều dòng hoặc nửa dòng mỗi lần recv)."""
    buf = b""
    while buf.count(b"\n") < n:
        chunk = s.recv(4096)
        if not chunk:
            break
        buf += chunk
    return [l.decode() for l in buf.split(b"\n")[:n]]


def fan_id():
    with open(STATE) as f:
        for line in f:
            if "type=FAN " in line:
                return line.split("id=")[1].split()[0]
    raise SystemExit("khong tim thay FAN trong " + STATE)


fan = fan_id()

s = connect()
s.sendall(b"HELLO\n")
check(read_lines(s, 1)[0].startswith("200 OK devices=7"), "HELLO -> 200 OK devices=7")

s.sendall(f"INFO {fan}\n".encode())
r = read_lines(s, 1)[0]
check(r.startswith(f"200 OK id={fan} type=FAN") and "Tmax=" in r, f"INFO {fan} -> 200 OK ... Tmax=")

s.sendall(b"INFO XYZ\n")
check(read_lines(s, 1)[0] == "404 NO_SUCH_DEVICE", "INFO XYZ -> 404 NO_SUCH_DEVICE")
s.sendall(b"INFO\n")
check(read_lines(s, 1)[0].startswith("400 BAD_REQUEST"), "INFO thieu tham so -> 400")
s.sendall(b"ABC\n")
check(read_lines(s, 1)[0] == "400 BAD_REQUEST unknown_command", "ABC -> 400 unknown_command")

# Tiêu chí 1: cắt vụn + dồn dòng
s.sendall(b"HEL")
time.sleep(0.5)
s.sendall(f"LO\nINFO {fan}\n".encode())
r = read_lines(s, 2)
check(r[0].startswith("200 OK devices") and r[1].startswith("200 OK id="),
      "'HEL' + (1s) + 'LO\\nINFO ..' -> dung 2 phan hoi")
s.sendall(b"HELLO\r\n")
check(read_lines(s, 1)[0].startswith("200 OK"), "HELLO\\r\\n (kieu telnet) -> 200")
s.sendall(b"x" * 5000 + b"\nHELLO\n")
r = read_lines(s, 2)
check(r[0] == "400 BAD_REQUEST LINE_TOO_LONG" and r[1].startswith("200 OK"),
      "dong 5000 byte -> 400 LINE_TOO_LONG, ket noi van dung duoc")

# Tiêu chí 2: nhiều client cùng lúc, một client ngắt đột ngột
others = [connect() for _ in range(5)]
others[2].close()
ok = True
for i, o in enumerate(others):
    if i == 2:
        continue
    o.sendall(b"HELLO\n")
    ok = ok and read_lines(o, 1)[0].startswith("200 OK")
check(ok, "5 client dong thoi, 1 client ngat giua chung, 4 client con lai van duoc phuc vu")
for o in others:
    o.close()

s.sendall(b"BYE\n")
r = read_lines(s, 1)[0]
closed = s.recv(10) == b""
check(r == "200 OK" and closed, "BYE -> 200 OK roi server dong ket noi")
s.close()

print("==> %s" % ("TAT CA PASS" if fails == 0 else f"{fails} FAIL"))
sys.exit(1 if fails else 0)
