import os
import serial
import time

os.system('start_LibreHardwareMonitor')
time.sleep(10)

def sendData(cpuu, mempercused, cput, gpuperc, gpumem, gputemp):
    try:
        connection = serial.Serial('COM6') # Change this to match your COM port!
        data = str(cpuu) + ',' + str(mempercused) + ',' + str(cput) + ',' + str(gpuperc) + ',' + str(gpumem) + ',' + str(gputemp)
        connection.write(data.encode())
        print("Data written", data.encode())
        connection.close()
    except Exception as e:
        print(e)

while True:
    import wmi
    w = wmi.WMI(namespace="root\LibreHardwareMonitor")
    infoHardware = w.Sensor()

    cpuutil='nule'
    mem_perc_used='nule'
    cpu_temp='nule'
    gpu_perc='nule'
    
    gpu_mem='nule'
    gpu_memUsed='nule'
    gpu_memTotal='nule'
    
    gpu_temp='nule'

    for sensor in infoHardware:
        if sensor.SensorType==u'Load' and sensor.Name==u'CPU Total':
            cpuutil=int(sensor.Value)
            break
    for sensor in infoHardware:
        if sensor.SensorType==u'Load' and sensor.Name==u'Memory':
            mem_perc_used=int(sensor.Value)
            break
    for sensor in infoHardware:
        if sensor.SensorType==u'Temperature' and sensor.Name==u'CPU Package':
            cpu_temp=int(sensor.Value)
            break

    for sensor in infoHardware:
        if sensor.SensorType==u'Load' and sensor.Name==u'GPU Core':
            gpu_perc=int(sensor.Value)
            break        
    for sensor in infoHardware:
        if sensor.Name==u'GPU Memory Used':
            gpu_memUsed=int(sensor.Value)
            break 
    for sensor in infoHardware:
        if sensor.Name==u'GPU Memory Total':
            gpu_memTotal=int(sensor.Value)
            break
    for sensor in infoHardware:
        if sensor.SensorType==u'Temperature' and sensor.Name==u'GPU Core':
            gpu_temp=int(sensor.Value)
            break

    gpu_mem=int((gpu_memUsed*100) / gpu_memTotal)

    sendData(cpuutil, mem_perc_used, cpu_temp, gpu_perc, gpu_mem, gpu_temp)
    time.sleep(0.9)