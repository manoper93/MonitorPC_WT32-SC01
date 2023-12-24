import asyncio
import serial
import wmi
import logging
import re
from ctypes import cast, POINTER
from comtypes import CLSCTX_ALL
from pycaw.pycaw import AudioUtilities, IAudioEndpointVolume
import time
time.sleep(10)

logging.basicConfig(level=logging.ERROR)

def rx_tx_com_ports_from_file():
    try:
        with open('COM.txt', 'r') as file:
            com_port = file.readline().strip()  # Read the TX COM port
            return com_port
    except FileNotFoundError:
        logging.error("File 'COM.txt' not found.")
        return None

def change_volume(increase=True, step=0.02, zero=True):
    devices = AudioUtilities.GetSpeakers()
    interface = devices.Activate(
        IAudioEndpointVolume._iid_, CLSCTX_ALL, None)
    volume = cast(interface, POINTER(IAudioEndpointVolume))

    current_volume = volume.GetMasterVolumeLevelScalar()
    if increase:
        new_volume = min(1.0, current_volume + step)
    else:
        new_volume = max(0.0, current_volume - step)
        if zero:
            new_volume = 0

    volume.SetMasterVolumeLevelScalar(new_volume, None)


def sendData(cpuu, mempercused, cput, gpuperc, gpumem, gputemp):
    try:
        com_port = rx_tx_com_ports_from_file()
        connection_tx = serial.Serial(com_port)
        data = f"{cpuu},{mempercused},{cput},{gpuperc},{gpumem},{gputemp}"
        connection_tx.write(data.encode())
        print("Tx:", data)
        connection_tx.close()
    except Exception as e:
        logging.error(f"Error sending data: {e}")

def recetor():
    try:
        com_port = rx_tx_com_ports_from_file()
        connection_rx = serial.Serial(com_port, timeout=0.08)  # Adjust the timeout as needed
        received_data = connection_rx.readline().decode('utf-8').strip()
        connection_rx.close()
        return received_data
    except Exception as e:
        logging.error(f"Error in recetor: {e}")
        return "0"

async def get_hardware_info():
    while True:
        w = wmi.WMI(namespace="root\LibreHardwareMonitor")
        infoHardware = w.Sensor()

        cpuutil = 'nule'
        mem_perc_used = 'nule'
        cpu_temp = 'nule'
        gpu_perc = 'nule'
        gpu_mem = 'nule'
        gpu_memUsed = 'nule'
        gpu_memTotal = 'nule'
        gpu_temp = 'nule'

        for sensor in infoHardware:
            if sensor.SensorType == u'Load' and sensor.Name == u'CPU Total':
                cpuutil = int(sensor.Value)
                break
        for sensor in infoHardware:
            if sensor.SensorType == u'Load' and sensor.Name == u'Memory':
                mem_perc_used = int(sensor.Value)
                break
        for sensor in infoHardware:
            if sensor.SensorType == u'Temperature' and sensor.Name == u'CPU Package':
                cpu_temp = int(sensor.Value)
                break
        for sensor in infoHardware:
            if sensor.SensorType == u'Load' and sensor.Name == u'GPU Core':
                gpu_perc = int(sensor.Value)
                break
        for sensor in infoHardware:
            if sensor.Name == u'GPU Memory Used':
                gpu_memUsed = int(sensor.Value)
                break
        for sensor in infoHardware:
            if sensor.Name == u'GPU Memory Total':
                gpu_memTotal = int(sensor.Value)
                break
        for sensor in infoHardware:
            if sensor.SensorType == u'Temperature' and sensor.Name == u'GPU Core':
                gpu_temp = int(sensor.Value)
                break

        gpu_mem = int((gpu_memUsed * 100) / gpu_memTotal)

        sendData(cpuutil, mem_perc_used, cpu_temp, gpu_perc, gpu_mem, gpu_temp)
        await asyncio.sleep(0.08)  # Sleep for 0.1 seconds (100 milliseconds)

async def recetor_task():
    while True:
        received_data = recetor()

        if received_data is not None:
            numeric_part = re.sub(r'\D', '', received_data)
            received_data = int(numeric_part) if numeric_part.isdigit() else None
            print("Rx:", received_data)
        else:
            print("Error receiving data.")
            

        if received_data == 1:
            print(f"-------------- Aumentar volume")
            change_volume(increase=True, step=0.1, zero=False)
        if received_data == 2:
            print(f"-------------- Diminuir volume")
            change_volume(increase=False, step=0.1, zero=False)
        if received_data == 3:
            print(f"-------------- Volume Mute")
            change_volume(increase=False, step=0.1, zero=True)
        if received_data == 4:
            print(f"-------------- Micro Ativo")
        if received_data == 5:
            print(f"-------------- Micro Mute")

        # Process received_data as needed
        await asyncio.sleep(0.01)  # Sleep for 1 second

# Iniciar o loop assíncrono
async def main():
    asyncio.create_task(get_hardware_info())
    asyncio.create_task(recetor_task())
    while True:
        # Adicione outras tarefas assíncronas, se necessário
        await asyncio.sleep(0.0001)  # Sleep for 1 segundo

if __name__ == "__main__":
    asyncio.run(main())
