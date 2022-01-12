#!/usr/bin/env python

# Dump everything from the serial port (from an Arduino board)
# to a log file

import os
import sys
import optparse
import serial
import datetime



def main():
    usage = '''
    usage: %prog -p port [options]
    Use -h to get full help
    '''

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
            s = ser.readline()
            s = s.strip()
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
