def crc8(data):
    """
    Calculates and returns an 8-bit CRC checksum using the CRC-8 algorithm.
    """
    crc = 0
    for b in data:
        crc ^= b
    return crc

while True:
    s = input("Enter a string (or 'quit' to exit): ")
    if s == 'quit':
        break
    checksum = crc8(s.encode())
    print(f"The CRC-8 checksum of '{s}' is {checksum}")

