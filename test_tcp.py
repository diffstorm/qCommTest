import socket
import time

# Constants
SERVER_IP = '127.0.0.1'
SERVER_PORT = 6666
SEND_RECEIVE_TIMEOUT = 0.5  # 500 ms timeout for sending/receiving
TEST_INDEX_MAX = 400
FAIL_TRY_MAX = 100

# The initial data sequence to start communication
START_DATA = bytes([0x00, 0x00, 0x01, 0x00, 0xd2, 0x02, 0xef, 0x8d])

def connect_to_server(ip, port, timeout):
    """Establishes a TCP connection to the server."""
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sock.settimeout(timeout)
    try:
        sock.connect((ip, port))
        print(f"Connected to server at {ip}:{port}")
        return sock
    except (socket.timeout, socket.error) as e:
        print(f"Connection failed: {e}")
        return None

def recv_all(sock, length):
    """Receives the exact amount of data specified by 'length' from the socket."""
    data = bytearray()
    while len(data) < length:
        chunk = sock.recv(length - len(data))
        if not chunk:
            raise RuntimeError("Socket connection broken")
        data.extend(chunk)
    return bytes(data)

def send_receive(sock, send_data, expected_recv_length):
    """Sends data to the server and waits for the expected amount of response data."""
    if send_data is None:
        print("No data to send")
        return None

    try:
        sock.sendall(send_data)
        print(f"Sent {len(send_data)} bytes")

        received_data = recv_all(sock, expected_recv_length)
        print(f"Received {len(received_data)} bytes")

        return received_data
    except (socket.timeout, socket.error) as e:
        print(f"Communication error: {e}")
        return None

def run_communication_test():
    """Runs the communication test with the server, retrying on failure up to a limit."""
    test_index = 1
    fail_try = FAIL_TRY_MAX
    response = None

    # Attempt to establish a connection and start the test
    while fail_try > 0:
        sock = connect_to_server(SERVER_IP, SERVER_PORT, SEND_RECEIVE_TIMEOUT)
        if sock is None:
            fail_try -= 1
            time.sleep(0.002)
            continue

        # Send initial data to start the test
        response = send_receive(sock, START_DATA, 7 + test_index)
        if response and len(response) == 7 + test_index:
            print("Test started successfully")
            test_index += 1
            break
        else:
            fail_try -= 1
            sock.close()
            time.sleep(0.002)

    if fail_try == 0:
        print("Failed to start communication test")
        return

    # Main communication loop
    total_time = 0
    while test_index <= TEST_INDEX_MAX and fail_try > 0:
        expected_rx_length = 7 + test_index

        start_time = time.time()
        response = send_receive(sock, response, expected_rx_length)
        elapsed_time = (time.time() - start_time) * 1e6  # Convert to microseconds

        if response and len(response) == expected_rx_length:
            print(f"Success {test_index}, Time: {elapsed_time:.0f} us")
            total_time += elapsed_time
            test_index += 1
        else:
            fail_try -= 1
            print(f"Fail {fail_try}")
        time.sleep(0.002)

    # Test result
    if test_index > TEST_INDEX_MAX and fail_try > 0:
        print("Test passed")
    else:
        print("Test failed")

if __name__ == "__main__":
    run_communication_test()
