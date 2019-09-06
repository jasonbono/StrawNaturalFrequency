##JB this is my modified versioin of arletts script, made in March, 2017



import serial
import time
import pylab as plb
import matplotlib.pyplot as plt
import matplotlib.mlab as mlab
from scipy.optimize import curve_fit
from scipy import asarray as ar,exp
from scipy import interpolate
from scipy.interpolate import interp1d
import scipy.fftpack
import math
import xlsxwriter
import glob
import os.path
import numpy as np

global ser


"""
    To Do:
    • Remove global variables
    • Comment everything
    x• Make plots not frozen..
    • Clean up Run()
    x• Change 's' vs 'b' to F and B
    • interfacing for test params
    • autoformat excel files
    • Improve error checking
    x• Titles of sheets more systematic
    • Handle multile arduinos connected
    """



def InitializeSerial():
    """ Connect to Arduino Serial"""
    global ser
    print("Initializing Arduino...",end='')
    count = -1
    while True: 
        portname = glob.glob("/dev/tty.usbmodem*") #Specific to USB connection, works for MAC now...
        count += 1
        if len(portname) == 0:
            if count==0:
                print("**Could not find an Arduino** \nRetrying")
            else:
                print('.',end='')
            time.sleep(1)
            continue
        try:           
            ser = serial.Serial(portname[0],baudrate=115200,timeout=5,write_timeout=5)
            time.sleep(2)
            # Bootloader has some timeout, we need to wait for that
            ser.flushInput()
            print('!')
            return ser
        except:
            print("Could not connect with Arduino. Trying Again...")

def Re_InitializeSerial():
    """ Restart the Arduino Serial communciation """
    print('.',end='')
    global ser
    ser.flushInput()
    ser.write('ee'.encode('utf-8'))
    ser.close()
    print('.',end='')
    ser = serial.Serial(ser.port,9600)
    time.sleep(1)
    ser.close()
    print('.',end='')
    ser.open()
    ser.baudrate=115200
    ser.timeout=10
    ser.write_timeout=10
    time.sleep(2)
# ***Need to trigger for useful peek data***
def ReadArduino(maxlines):
    """
    Read buffer from the Serial until the done command('*') is recieved. The
    buffer is then decoded into 'ascii' and split accordingly.

    Return:
        - List of Times
        - List of IR values
    """
    global ser
    count = 0
    t = []
    ir= []

    print('Reading Serial...')
    while (len(t)< maxlines and len(ir)<maxlines): 
        if count>=maxlines:
            print("Maxed Out Serial...")
            break
        buffer = ser.readline().decode('ascii')
##        print(buffer)
        if (len(buffer))==0:
            print("Buffer Empty")
            continue
        elif "*" in buffer:
            print("** Sweep Complete! **")
            break
        try:
            t_val,ir_val = buffer.split(',',2)
        except ValueError:
            continue

        t.append(float(t_val)/1000.0)
        ir.append(int(ir_val))       
        
    print("# Rows Read: ",len(ir),",",len(t))
    if not (len(t)==(ir)): #Not sure why this isnt working....
        print("Warning: columns not the same length!")
        pass
    
    return t,ir
        
def startCommunication(command):
    """ Returns true if command successfully sent"""
    global ser
    #Just in case send stop command
    ser.write('ee'.encode('utf-8'))
    i=1
    print("Attempting to Write",end='')
    while True:
        #Just in case send stop command
        ser.write('e'.encode('utf-8'))
        ser.flushInput()
        ser.flushOutput()
        time.sleep(0.1)
        print('.',end='')
        #Write the Start command and wait until successfully received
        wrote = ser.write(command.encode('utf-8'))
        received = ser.read(2).decode('ascii')
        if command[0] in received:
            return True
        elif i>8:
            return False
        elif i%3==0:
            #re-initialize Serial port
            print("Failed to Send \nRe-Initializing Port!..",end='')
            Re_InitializeSerial()
            print('Done')
            print("Attempting to Write",end='')
        i+=1
        time.sleep(0.25)

def getFFT(t,ir):
    """
    Filter out unnecessary data, Interpolate cubicly to get uniform time
    spacing, take the FFT and normalize.

    Returns:
      -positive & nonzero frequencies (including corresponding amplitude)
      -peak frequency
      -transformed data
    """
    # Filter to only look at the impact period
    t_filtered = []
    ir_filtered = []
    stop=False   
    for i in range(len(ir)-20):
        local_std = np.std(ir[i:i+20])
        #only start taking data when there is significant oscillations
        if local_std>12:
            # Ignore duplicates
            if not t[i+20] in t_filtered:
                t_filtered.append(t[i+20])
                ir_filtered.append(ir[i+20])
                stop=True
        # stop taking data as soon as oscillations die down
        elif stop:
            break
    # If could not find the pulse, just use original data
    if len(t_filtered)==0:
        t_filtered = t
        ir_filtered = ir
        
    #Interpolate to get uniform dT
    t_new = np.linspace(t_filtered[0],t_filtered[-1],1000)
    f = interp1d(t_filtered, ir_filtered)
    ir_new = f(t_new)
    #Take the FFT and normalize
    ir_f = np.fft.fft(ir_new,norm='ortho')
    freq = np.fft.fftfreq(len(t_new),t_new[1]-t_new[0])
    #Remove negative frequencies and those close to zero
    n = len(freq)
    freq = freq[1:-1+n/2]
    ir_f = abs(ir_f[1:-1+n/2])
    maxFreq = freq[np.argmax(ir_f)]
    return freq, ir_f, maxFreq,t_new,ir_new
        
    
# Main   
def Run():
    """
    1. Connect to the Arduino Serial
    2. Read Data
    3. Take FFT
    4. Plot and Save to Excel
    """
    print ("TEST\n")
    while True:
        filename = input("Enter Excel file name or leave blank for a default name \nFilename = ")+".xlsx"
        if len(filename) == 5:
            startTime = time.strftime("%y%m%d_%H%M%S")
            filename = 'Time'+startTime+filename
            break
        if not os.path.isfile(filename):
            break
        else:
            print("File with this name already exists")
        
    print("Set filename to: ",filename)
    print("---------------------------------------")
    global ser
    ser = InitializeSerial()
    ser.flushInput()
    ser.flushOutput()
    count = 0
    # Create Excel Sheet
    workbook = xlsxwriter.Workbook(filename, {'strings_to_numbers': True})
    inpcommand = ''
    while inpcommand != "q":
        #Sweep and Read Data
        inpcommand = input("'q' to quit, or Name the next Sheet: ")
        if inpcommand=="q":
            break
        else:
            command = 's'
            if len(inpcommand) == 0:
                inpcommand = str(count)
            print("Starting Trial ",inpcommand)

            # Prepare Worksheets
            worksheet = workbook.add_worksheet("Trial_"+inpcommand)
            worksheet.write(0,0,"Time (s)")
            worksheet.write(0,1,"IR")
            worksheet.write(0,3,"Interpolated Time")
            worksheet.write(0,4,"Interpolated IR")
            worksheet.write(0,5,"Frequencies (Hz)")
            worksheet.write(0,6,"Amplitude")

            
            if ser.writable():
                print("Serial is Writable")
                if startCommunication(command):
                    print("Arduino received command")
##                    time.sleep(2)
                    if ser.readable():
                        (Time,IR) = ReadArduino(10000)
                        time.sleep(2)
                    else:
                        print("**Could not read Serial!")                       
                else:
                    print("**Could NOT Successfully start motors!")
                    break
            else:
                print ("Exiting...")
                print("Arduino Not Writable!")
                break
            
            
            #Write Raw Data to Excel
            for row in range(min(len(Time),len(IR))):
                worksheet.write(row+1,0,Time[row])
                worksheet.write(row+1,1,IR[row])

            #Take the FFT
            freq,IR_f,maxF,t_new,ir_new = getFFT(Time,IR)
            
            #Write FFT Data to Excel
            for row in range(min(len(ir_new),len(t_new))):
                worksheet.write(row+1,3,t_new[row])
                worksheet.write(row+1,4,ir_new[row])
                
            for row in range(min(len(freq),len(IR_f))):
                worksheet.write(row+1,5,freq[row])
                worksheet.write(row+1,6,IR_f[row])
                
            worksheet.write(0,7,"Peak Frequency")
            worksheet.write(1,7,maxF)
                
            
            print("---------------------------------------")
            count +=1           
        ## Plot the data
            #Prepare axes
            fig = plt.figure(filename[:-5]+"_Trial_"+inpcommand)
            axo = plt.subplot2grid((7,5),(0,0),rowspan=3,colspan=2)
            axf = plt.subplot2grid((7,5),(0,3),rowspan=7,colspan=2)
            axt = plt.subplot2grid((7,5),(4,0),rowspan=3,colspan=2)

            #Plot Raw Data
            axo.set_title("Raw Data")
            axo.plot(Time,IR)
            axo.set_xlabel('Time(s)')
            axo.set_ylabel('IR')
            
            #Plot filtered Data
            axt.plot(t_new,ir_new)
            axt.set_xlabel('Time (s)')
            axt.set_ylabel('IR')
            axt.set_title('Selected Data')

            #Plot FFT
            axf.fill_between(freq,0,IR_f,interpolate=True,alpha=0.5)
            axf.axvline(x=maxF,color='green', lw=1, linestyle='--')
            axf.axis([0,200, 0,max(IR_f)+50])
            axf.set_xlabel('Frequency (Hz)')
            axf.set_ylabel('Amplitude')
            axf.set_title("FFT\nPeak Freq:" +str(maxF))

            plt.savefig(filename[:-5]+"_Trial_"+inpcommand) ##savefig is in pyplot library (plt) 
            
            
            
    workbook.close()
    plt.show(block=True)
    # Disconnect Arduino Serial
    ser.close()
    print("\n*Closed the Arduino Serial Port*")
##    notes = input("Notes: ")



# Test the code
##i = [42,42,42,42,42,42,42,42,42,42,42,42,42,42,42,42,42,42,42,42,42,42,42,42,42,42,42,42,42,42,42,42,42,42,42,42,42,42,42,42,42,42,42,42,42,42,42,42,42,42,42,42,42,42,42,42,42,42,42,42,42,42,42,42,42,42,42,42,42,42,42,42,42,42,42,42,42,42,42,42,42,42,42,42,42,42,42,42,42,42,42,42,42,42,42,42,42,43,45,49,54,61,72,89,108,120,113,99,82,69,58,51,48,47,48,50,54,60,73,92,107,111,106,97,83,68,57,52,49,48,49,51,55,62,75,89,100,104,103,95,80,66,57,53,50,50,50,52,56,63,74,85,94,100,99,91,78,66,59,54,52,51,51,53,56,62,71,81,90,96,95,88,77,68,60,55,53,51,51,53,56,61,68,78,87,92,91,85,77,69,62,56,54,52,53,54,56,60,67,75,83,87,86,83,76,69,62,57,55,54,54,54,57,60,66,73,79,82,83,81,76,69,63,59,57,55,55,56,57,61,66,71,75,78,80,79,74,69,64,60,58,56,56,56,58,61,64,68,73,76,77,76,73,69,65,62,59,58,57,57,58,60,63,66,70,73,75,74,72,70,67,63,60,59,58,58,59,60,62,65,68,71,72,72,72,70,67,64,62,60,59,59,59,60,62,65,67,69,71,71,71,70,67,65,63,61,60,59,60,60,62,63,65,67,69,70,70,69,68,66,64,62,61,60,60,60,61,62,64,66,68,69,69,69,68,67,65,63,62,61,61,60,61,62,63,65,67,68,69,69,69,68,66,64,63,62,61,60,61,61,63,64,66,67,68,69,69,68,67,65,64,62,61,60,60,61,62,63,65,67,68,69,69,69,68,66,64,62,61,61,60,61,61,63,64,66,68,69,69,69,68,66,64,63,62,61,60,60,61,62,64,66,67,69,69,69,68,67,65,63,62,61,60,60,61,62,64,65,67,68,69,69,68,67,65,64,62,61,60,60,61,61,63,65,67,68,69,69,69,67,66,64,62,61,60,60,61,62,63,65,67,68,69,69,69,67,66,64,63,61,61,60,61,62,63,65,66,68,69,69,69,67,66,64,62,61,61,61,61,62,63,65,66,68,68,69,68,67,66,64,63,62,61,61,61,62,63,64,66,67,68,69,68,67,66,64,62,62,61,61,61,62,63,64,66,67,68,68,68,67,65,64,63,62,61,61,61,62,63,64,66,67,68,68,67,67,66,64,63,62,61,62,62,62,63,64,65,66,67,67,67,66,65,65,63,62,62,62,62,63,63,64,65,66,67,67,67,66,65,64,63,62,62,62,62,63,63,64,65,66,66,66,67,66,65,64,63,63,62,62,62,63,63,64,65,66,66,66,66,66,65,64,63,63,63,63,63,63,64,64,65,65,66,66,66,65,65,64,64,63,63,63,63,63,64,64,65,66,66,66,66,65,65,64,64,64,63,63,63,63,64,64,64,65,65,65,65,65,65,64,64,64,63,63,63,63,64,64,64,65,65,65,65,65,65,64,64,64,64,63,63,64,64,64,64,64,65,65,65,65,65,64,64,64,63,64,64,64,64,64,64,64,65,65,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,65,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,65,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,65,65,64,65,64,64,64,64]
##t = [1.2921,1.2931,1.2931,1.2941,1.2951,1.2961,1.2961,1.2971,1.2981,1.2991,1.3001,1.3001,1.3011,1.3021,1.3031,1.3031,1.3041,1.3051,1.3061,1.3061,1.3071,1.3081,1.3091,1.3111,1.3111,1.3121,1.3131,1.3141,1.3141,1.3151,1.3161,1.3171,1.3171,1.3181,1.3191,1.3201,1.3201,1.3211,1.3221,1.3231,1.3241,1.3241,1.3251,1.3261,1.3271,1.3271,1.3281,1.3291,1.3301,1.3301,1.3311,1.3321,1.3331,1.3341,1.3341,1.3351,1.3361,1.3371,1.3371,1.3381,1.3391,1.3401,1.3401,1.3411,1.3421,1.3431,1.3441,1.3441,1.3451,1.3461,1.3471,1.3471,1.3481,1.3491,1.3501,1.3501,1.3511,1.3531,1.3541,1.3551,1.3551,1.3561,1.3571,1.3581,1.3581,1.3591,1.3601,1.3611,1.3611,1.3621,1.3631,1.3641,1.3651,1.3651,1.3661,1.3671,1.3681,1.3681,1.3691,1.3701,1.3711,1.3711,1.3721,1.3731,1.3741,1.3751,1.3751,1.3761,1.3771,1.3781,1.3781,1.3791,1.3801,1.3811,1.3821,1.3821,1.3831,1.3841,1.3851,1.3851,1.3861,1.3871,1.3881,1.3891,1.3891,1.3901,1.3911,1.3921,1.3921,1.3931,1.3941,1.3961,1.3961,1.3971,1.3981,1.3991,1.4001,1.4001,1.4011,1.4021,1.4031,1.4041,1.4041,1.4051,1.4061,1.4071,1.4071,1.4081,1.4091,1.4101,1.4101,1.4111,1.4121,1.4131,1.4141,1.4141,1.4151,1.4161,1.4171,1.4171,1.4181,1.4191,1.4201,1.4201,1.4211,1.4221,1.4231,1.4241,1.4241,1.4251,1.4261,1.4271,1.4271,1.4281,1.4291,1.4301,1.4301,1.4311,1.4321,1.4331,1.4341,1.4341,1.4351,1.4361,1.4371,1.4371,1.4391,1.4401,1.4411,1.4411,1.4421,1.4431,1.4441,1.4441,1.4451,1.4461,1.4471,1.4481,1.4481,1.4491,1.4501,1.4511,1.4511,1.4521,1.4531,1.4541,1.4541,1.4551,1.4561,1.4571,1.4581,1.4581,1.4591,1.4601,1.4611,1.4611,1.4621,1.4631,1.4641,1.4641,1.4651,1.4661,1.4671,1.4681,1.4681,1.4691,1.4701,1.4711,1.4711,1.4721,1.4731,1.4741,1.4741,1.4751,1.4761,1.4771,1.4781,1.4781,1.4791,1.4811,1.4821,1.4821,1.4831,1.4841,1.4851,1.4851,1.4861,1.4871,1.4881,1.4891,1.4891,1.4901,1.4911,1.4921,1.4921,1.4931,1.4941,1.4951,1.4951,1.4961,1.4971,1.4981,1.4991,1.4991,1.5002,1.5012,1.5022,1.5022,1.5032,1.5042,1.5052,1.5052,1.5062,1.5072,1.5082,1.5082,1.5092,1.5102,1.5112,1.5122,1.5122,1.5132,1.5142,1.5152,1.5152,1.5162,1.5172,1.5182,1.5182,1.5192,1.5202,1.5212,1.5222,1.5222,1.5242,1.5252,1.5262,1.5262,1.5272,1.5282,1.5292,1.5292,1.5302,1.5312,1.5322,1.5332,1.5332,1.5342,1.5352,1.5362,1.5362,1.5372,1.5382,1.5392,1.5392,1.5402,1.5412,1.5422,1.5432,1.5432,1.5442,1.5452,1.5462,1.5462,1.5472,1.5482,1.5492,1.5492,1.5502,1.5512,1.5522,1.5532,1.5532,1.5542,1.5552,1.5562,1.5562,1.5572,1.5582,1.5592,1.5592,1.5602,1.5612,1.5622,1.5632,1.5632,1.5642,1.5652,1.5672,1.5672,1.5682,1.5692,1.5702,1.5702,1.5712,1.5722,1.5732,1.5742,1.5742,1.5752,1.5762,1.5772,1.5772,1.5782,1.5792,1.5802,1.5802,1.5812,1.5822,1.5832,1.5842,1.5842,1.5852,1.5862,1.5872,1.5872,1.5882,1.5892,1.5902,1.5902,1.5912,1.5922,1.5932,1.5932,1.5942,1.5952,1.5962,1.5972,1.5972,1.5982,1.5992,1.6002,1.6002,1.6012,1.6022,1.6032,1.6032,1.6042,1.6052,1.6062,1.6072,1.6072,1.6092,1.6102,1.6112,1.6112,1.6122,1.6132,1.6142,1.6142,1.6152,1.6162,1.6172,1.6182,1.6182,1.6192,1.6202,1.6212,1.6212,1.6222,1.6232,1.6242,1.6242,1.6252,1.6262,1.6272,1.6282,1.6282,1.6292,1.6302,1.6312,1.6312,1.6322,1.6332,1.6342,1.6342,1.6352,1.6362,1.6372,1.6382,1.6382,1.6392,1.6402,1.6412,1.6412,1.6422,1.6432,1.6442,1.6442,1.6452,1.6462,1.6472,1.6482,1.6482,1.6492,1.6502,1.6522,1.6522,1.6532,1.6542,1.6552,1.6552,1.6562,1.6572,1.6582,1.6592,1.6592,1.6602,1.6612,1.6622,1.6622,1.6632,1.6642,1.6652,1.6652,1.6662,1.6672,1.6682,1.6682,1.6692,1.6702,1.6712,1.6722,1.6722,1.6732,1.6742,1.6752,1.6752,1.6762,1.6772,1.6782,1.6782,1.6792,1.6802,1.6812,1.6822,1.6822,1.6832,1.6842,1.6852,1.6852,1.6862,1.6872,1.6882,1.6882,1.6892,1.6902,1.6912,1.6922,1.6922,1.6932,1.6952,1.6962,1.6962,1.6972,1.6982,1.6992,1.6992,1.7002,1.7012,1.7022,1.7032,1.7032,1.7042,1.7052,1.7062,1.7062,1.7072,1.7082,1.7092,1.7092,1.7102,1.7112,1.7122,1.7132,1.7132,1.7142,1.7152,1.7162,1.7162,1.7172,1.7182,1.7192,1.7192,1.7202,1.7212,1.7222,1.7232,1.7232,1.7242,1.7252,1.7262,1.7262,1.7272,1.7282,1.7292,1.7292,1.7302,1.7312,1.7322,1.7322,1.7332,1.7342,1.7352,1.7372,1.7372,1.7382,1.7392,1.7402,1.7402,1.7412,1.7422,1.7432,1.7432,1.7442,1.7452,1.7462,1.7472,1.7472,1.7482,1.7492,1.7502,1.7502,1.7512,1.7522,1.7532,1.7532,1.7542,1.7552,1.7562,1.7572,1.7572,1.7582,1.7592,1.7602,1.7602,1.7612,1.7622,1.7632,1.7632,1.7642,1.7652,1.7662,1.7672,1.7672,1.7682,1.7692,1.7702,1.7702,1.7712,1.7722,1.7732,1.7732,1.7742,1.7752,1.7762,1.7772,1.7772,1.7782,1.7802,1.7812,1.7812,1.7822,1.7832,1.7842,1.7842,1.7852,1.7862,1.7872,1.7882,1.7882,1.7892,1.7902,1.7912,1.7912,1.7922,1.7932,1.7942,1.7942,1.7952,1.7962,1.7972,1.7972,1.7982,1.7992,1.8002,1.8012,1.8012,1.8022,1.8032,1.8042,1.8042,1.8052,1.8062,1.8072,1.8072,1.8082,1.8092,1.8102,1.8112,1.8112,1.8122,1.8132,1.8142,1.8142,1.8152,1.8162,1.8172,1.8172,1.8182,1.8192,1.8202,1.8212,1.8212,1.8232,1.8242,1.8252,1.8252,1.8262,1.8272,1.8282,1.8282,1.8292,1.8302,1.8312,1.8322,1.8322,1.8332,1.8342,1.8352,1.8352,1.8362,1.8372,1.8382,1.8382,1.8392,1.8402,1.8412,1.8422,1.8422,1.8432,1.8442,1.8452,1.8452,1.8462,1.8472,1.8482,1.8482,1.8492,1.8502,1.8512,1.8512,1.8522,1.8532,1.8542,1.8552,1.8552,1.8562,1.8572,1.8582,1.8582,1.8592,1.8602,1.8612,1.8612,1.8622,1.8632,1.8652,1.8662,1.8662,1.8672,1.8682,1.8692,1.8692,1.8702,1.8712,1.8722,1.8722,1.8732,1.8742,1.8752,1.8762,1.8762,1.8772,1.8782,1.8792,1.8792,1.8802,1.8812,1.8822,1.8822,1.8832,1.8842,1.8852,1.8862,1.8862,1.8872,1.8882,1.8892,1.8892,1.8902,1.8912,1.8922,1.8922,1.8932,1.8942,1.8952,1.8962,1.8962,1.8972,1.8982,1.8992,1.8992,1.9002,1.9012,1.9022,1.9022,1.9032,1.9042,1.9052]
##freq,IR_f,maxF,t_filtered,ir_filtered= getFFT(t,i)
##print(maxF)
##f,(a1,a2) = plt.subplots(1,2)
##a1.plot(t_filtered,ir_filtered)
####a1.plot(t[0:len(std)],std)
####a2.plot(freq,IR_f)
##a2.fill_between(freq,0,IR_f,interpolate=True)
##a2.axvline(x=maxF,color='green', lw=3, linestyle='--')
##a2.axis([0,200, 0,max(IR_f)+100])
##plt.show()
