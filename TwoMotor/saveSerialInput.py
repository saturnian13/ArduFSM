import sys
import os
import serial
import threading
import io
import datetime

def monitor(sio,fpath):

   line = sio.readline()
   if (line != ""):
       text_file.write(line)
      


def get_serial_port(rigname):
    """Get the serial port for the specified rigname"""
    d = {
    'L4': '/dev/ttyACM0',
    'M1': '/dev/ttyACM2',
    'M2': '/dev/ttyACM3',
    'M3': '/dev/ttyACM4', }

    try:
        return d[rigname]
    except KeyError:
        raise ValueError("can't find serial port for rig %s" % rigname)     

def stop_behavior(ser,text_file):
    """ Stop the serial port and save the log file """
    ser.close()
    text_file.close()
    print "Stopped recording serial input"


"""
MAIN APPLICATION
""" 

#if __name__ == "__main__":
try:
    this_dir_name = os.getcwd()
    rigname = os.path.split(this_dir_name)[1]
    serial_port = get_serial_port(rigname)
    print "Start Serial Monitor"
    baud_rate= 115200;
    
    #NEED TO FIX so it starts or ends by loading a blank file
    
    mouse_name = raw_input("Enter mouse name: ")
    mouse_name = mouse_name.upper().strip()

    ser = serial.Serial(serial_port, baud_rate, timeout=0.1)
    sio = io.TextIOWrapper(io.BufferedRWPair(ser,ser))
    f1 = datetime.datetime.now().strftime('%Y%m%d%H%M%S');
    fname = f1 + '_' + mouse_name + '.log';
    fpath = './logfiles/' + fname;
    text_file = open(fpath, "w")
    while True:
        monitor(sio,fpath)
    
except KeyboardInterrupt:
    stop_behavior(ser,text_file)
except:
    raise
finally:
    stop_behavior(ser,text_file)