def compute_crc16(data_bytes, polynomial=0x1021, init_value=0x0000, xor_out=0x7C40):
    crc = init_value
    for byte in data_bytes:
        # XOR the byte into the most significant byte of the CRC register
        crc ^= (byte << 8)
        for _ in range(8):
            # Check if the MSB (bit 15) is set
            if crc & 0x8000:
                crc = ((crc << 1) ^ polynomial) & 0xFFFF
            else:
                crc = (crc << 1) & 0xFFFF
    # Apply the final XOR value
    return crc ^ xor_out
 
# Input data: 0x05A6 represented as bytes [0x05, 0xA6]
data = [0x05, 0xA6]
 
# Test with standard initial values
print(f"CRC with Init 0x0000: 0x{compute_crc16(data, 0x1021, 0x0000, 0x7C40):04X}")