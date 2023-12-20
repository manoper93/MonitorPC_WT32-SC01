import os
import time
import ctypes
import serial
import wmi

time.sleep(10)

def rx_tx_com_ports_from_file():
    try:
        with open('COM.txt', 'r') as file:
            com_port_rx = file.readline().strip()  # Read the RX COM port
            com_port_tx = file.readline().strip()  # Read the TX COM port
            return com_port_rx, com_port_tx
    except FileNotFoundError:
        logging.error("File 'COM.txt' not found.")
        return None, None

def sendData(cpuu, mempercused, cput, gpuperc, gpumem, gputemp, com_port_tx):
    try:
        connection_tx = serial.Serial(com_port_tx)
        data = str(cpuu) + ',' + str(mempercused) + ',' + str(cput) + ',' + str(gpuperc) + ',' + str(gpumem) + ',' + str(gputemp)
        connection_tx.write(data.encode())
        print("Data written", data.encode())
        connection_tx.close()
    except Exception as e:
        print(e)

def recetor(com_port_rx):
    try:
        connection_rx = serial.Serial(com_port_rx)
        received_data = connection_rx.read(1024)
        print("Data received:", received_data.decode())
        connection_rx.close()
        return received_data.decode()
    except Exception as e:
        print(e)
        return None

        
def adjust_volume(volume_change):
    try:
        current_volume = os.system('powershell (Get-AudioDevice -Playback -Name "Speakers").Volume')
        new_volume = max(0, min(100, current_volume + volume_change))
        os.system(f'powershell (Get-AudioDevice -Playback -Name "Speakers").Volume = {new_volume}')
        logging.info(f"Volume adjusted by {volume_change}. New volume: {new_volume}")
    except Exception as e:
        logging.error(f"Error adjusting volume: {e}")


while True:
    com_port_rx, com_port_tx = rx_tx_com_ports_from_file()
    #received_data = recetor(com_port_rx)
    
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

    #if received_data is not None:
        #print("Received data:", received_data)
    #else:
        #print("Erro ao receber dados.")
    
    sendData(cpuutil, mem_perc_used, cpu_temp, gpu_perc, gpu_mem, gpu_temp, com_port_tx)
    time.sleep(0.5)
