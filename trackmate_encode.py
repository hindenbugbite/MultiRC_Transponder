import argparse


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

# Input ID number (eg 1234 or 409)
ID = 8192


def ID_to_list(uid):
    upper = uid//256
    lower = uid-(upper*256)
    return [upper, lower]


def PrintID(var_ID):
    print(f"ID number: {var_ID}")
    data = ID_to_list(var_ID)
    print(f"ID number in hex: 0x{var_ID:04X}")

    # Test with standard initial values
    print(f"CRC value in hex: 0x{compute_crc16(data, 0x1021, 0x0000, 0x7C40):04X}")


def main():
    """Entry point: accept a 4-digit decimal ID on the command line and print its CRC."""
    parser = argparse.ArgumentParser(description="Print ID and computed CRC for a 4-digit ID number")
    parser.add_argument('id', metavar='ID', help='4-digit ID number (0000-9999)')
    args = parser.parse_args()

    id_str = args.id
    if not id_str.isdigit() or len(id_str) != 4:
        parser.error('ID must be a 4-digit decimal number, for example 0123 or 409 (use leading zeros to make 4 digits)')

    var_id = int(id_str)
    PrintID(var_id)


if __name__ == '__main__':
    main()
