import socket
import requests
from urllib.parse import urlparse
import threading
import asyncio
import aiofiles
import websockets
import json
import time
import queue
from datetime import datetime

esp32host = "192.168.1.121"
esp32addr = socket.gethostbyname(esp32host)
baseUrl = f"http://{esp32addr}/"

async def open_file(uri):
    try:
        ts = datetime.now().strftime("%Y-%m-%d %H_%M_%S.%s")
        filename = f"websock_{ts}.csv"
        async with aiofiles.open(filename, mode='a') as f: # Use 'a' for append, 'w' for write (overwrite)
            await subscribe_to_websocket(uri, f)
        print(f"Async File Writer: Finished writing to {filename}.")
    except Exception as e:
        print(f"Async File Writer: Error writing to {filename}: {e}")


async def subscribe_to_websocket(uri, out_file):
    try:
        async with websockets.connect(uri) as websocket:
            print(f"Connected to {uri}")
            while True:
                try:
                    message = await websocket.recv()
                    print(f"Received: {message}")
                    data = json.loads(message)
                    await out_file.write(f"{data['ts']},{data['pwm']},{data['v']},{data['c']}\n")
                    await out_file.flush()  # Ensure data is written to file
                    # Process the data as needed
                except websockets.exceptions.ConnectionClosedOK:
                    print("Connection closed cleanly by the server.")
                    break
                except websockets.exceptions.ConnectionClosedError as e:
                    print(f"Connection closed with error: {e}")
                    break
                except Exception as e:
                    print(f"An unexpected error occurred: {e}")
                    break

    except websockets.exceptions.InvalidURI as e:
        print(f"Invalid WebSocket URI: {e}")
    except ConnectionRefusedError:
        print(f"Connection refused. Is the WebSocket server running at {uri}?")
    except Exception as e:
        print(f"Failed to connect: {e}")

def perform_blocking_task():
    ## configure pwm
    url = f"{baseUrl}/pwmon?channel=0&freq=5000&res=8&pin=25&duty=0"
    response = requests.get(url)
    if response.status_code != 200:
        raise Exception(f"Failed to enable PWM: {response.status_code} {response.text}")

    for pwm_value in range(0, 255):
        print(f"Setting PWM value to {pwm_value}")
        url = f"{baseUrl}/pwmset?channel=0&duty={pwm_value}"
        response = requests.get(url)
        if response.status_code != 200:
            raise Exception(f"Failed to set PWM: {response.status_code} {response.text}")
        time.sleep(30)
        

async def run_blocking_in_thread():
    return await asyncio.to_thread(perform_blocking_task)

async def main():
    ## Try to disable PWM
    url = f"{baseUrl}/pwmoff"
    response = requests.get(url)
    if response.status_code != 200:
        raise Exception(f"Failed to disable PWM: {response.status_code} {response.text}")

    websocket_uri = f"ws://{esp32addr}/ws"
    websocket_task = asyncio.create_task(open_file(websocket_uri))
    blocking_task = asyncio.create_task(run_blocking_in_thread())
    await blocking_task
    websocket_task.cancel()  # Cancel the WebSocket task if needed
    try:
        await websocket_task
    except asyncio.CancelledError:
        print("Main: WebSocket task successfully cancelled.")

if __name__ == "__main__":
    try:
        asyncio.run(main())
    except KeyboardInterrupt:
        print("Program terminated by user.")



