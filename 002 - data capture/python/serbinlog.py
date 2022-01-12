#!/usr/bin/env python

# Read binary records from the serial port that are formatted like this:
# start_record  32-bits 0xFFFFFFFF
# timestamp     32-bits microseconds
# v1            16-bits
# v2            16-bits

START_SEQ = [0xFF, 0xFF, 0xFF, 0xFF]

import os
import sys
import optparse
import serial
import datetime
import traceback
import struct

# wait for the start sequence
def waitForStart(ser):
    # wait until we've seen the exact start sequence
    index = 0
    while True:
        # get a byte from the serial link
        data = ord(ser.read(1))
        if data != START_SEQ[index]:
            # not what we want
            index = 0
            continue # start again

        # this byte matches
        index += 1
        if index == len(START_SEQ):
            # we have found the whole sequence
            return



# Read a binary record from the serial port and return a CSV string
# formatted for the fields in the record: time,v1,v2
def readRecord(ser):
    # wait until we've seen the start record
    waitForStart(ser)

    # Now collect the 8 bytes of the data record
    data = ser.read(8)

    # Parse the byte array into a struct of:
    # uint32, uint16, uint16
    # We assume little endian format (<)
    s = struct.unpack('<LHH', data)

    # Convert the fields of the struct into decimal values
    # in a CSV string
    ts = s[0]
    v1 = s[1]
    v2 = s[2]
    str = '{:d},{:d},{:d}'.format(ts, v1, v2)

    # return the string
    return str

def main():
    usage = '''
    usage: %prog -p port [options]
    Use -h to get full help
    '''

    # debug stuff
    global START_SEQ
    print 'Start seq is', len(START_SEQ), 'bytes'

    # create an option parser
    p = optparse.OptionParser(usage, version='%prog 1.0')
    
    # Add a verbose option
    p.add_option('--verbose', '-v', action='store_true', help='Show data on screen as logging happens')
    
    # Add an option to collect a USB port name
    p.add_option('--port', '-p', default="", action='store', help='Select the USB serial port')

    # Add an option to set the baud rate
    p.add_option('--baud', '-b', default="115200", action='store', help='Set the baud rate. Default is 115,200')

    # Add an option to set the filename
    # And set the default name from a timestamp
    dt = datetime.datetime.now()
    fn_def = dt.strftime("%Y-%m-%d-%H%M%S-log_data.csv")

    p.add_option('--filename', '-f', default=fn_def, action='store', help='Set the filename to write the log data to. Default is <timestamp>-log_data.csv')

    # parse the command line ignoring argv[0] (the application name)
    opt, args = p.parse_args(args=sys.argv[1:])
    
    if opt.port == '':
        p.error('Port is required')

    # try to open the port
    try:
        ser = serial.Serial(opt.port, opt.baud, timeout=10)
    except IOError as e:
        print 'Exception', e
        exit(1)
    
    print "Receiving serial data from:", opt.port, 'at', opt.baud, 'baud'

    # try to open the file
    try:
        opfile = open(opt.filename, 'w')
        print "Writing log data to:", opt.filename
    except IOError as e:
        print 'Exception', e
        exit(1)

    while True:
        try:
            s = readRecord(ser)
            if opt.verbose:
                # show the record on the screen
                print s
            opfile.write(s)
            opfile.write('\n')
        except serial.SerialTimeoutException:
            print "Timed out"
            break
        except Exception as e:
            print "Other exception", e
            print(traceback.format_exc())
            break

    # close the file
    print "Closing file:", opt.filename
    opfile.close()


if __name__ == '__main__':
    main()
