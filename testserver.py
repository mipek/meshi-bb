#! /usr/bin/env python

# Client and server for udp (datagram) echo.
#
# Usage: udpecho -s [port]			(to start a server)
# or:	udpecho -c host [port] <file (client)

import sys
import struct
from socket import *

PORT = 13337
BUFSIZE = 1024

def unpack(fmt, data):
    size = struct.calcsize(fmt)
    return struct.unpack(fmt, data[:size]), data[size:]
	
def main():
	server()

def server():
	if len(sys.argv) > 2:
		port = eval(sys.argv[2])
	else:
		port = PORT
	s = socket(AF_INET, SOCK_DGRAM)
	s.bind(('', port))
	print 'udp echo server ready, port %r' % (port)
	while 1:
		data, addr = s.recvfrom(BUFSIZE)
		print 'server received %r from %r' % (data, addr)
		print 'total byte count: ' + str(len(data))
		packet_id, data = unpack('c', data)
		crc, data = unpack('I', data)
		bbid, data = unpack('H', data)
		packet_num, data = unpack('c', data)
		ptime, data = unpack('I', data)
		lat, data = unpack('f', data)
		lng, data = unpack('f', data)
		print 'packet id + flag: ' + str(packet_id)
		print 'crc32: ' + str(crc)
		print 'bbid: ' + str(bbid)
		print 'packet_num: ' + str(packet_num)
		print 'time: ' + str(ptime)
		print 'lat: ' + str(lat)
		print 'lng: ' + str(lng)
		print '------------------'
		s.sendto(data, addr)

main()