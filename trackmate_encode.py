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


def ID_to_list(uid):
    upper = uid // 256
    lower = uid - (upper * 256)
    return [upper, lower]


def PrintID(var_ID):
    print(f"ID number: {var_ID}")
    data = ID_to_list(var_ID)
    print(f"ID number in hex: 0x{var_ID:04X}")

    # Test with standard initial values
    print(f"CRC value in hex: 0x{compute_crc16(data, 0x1021, 0x0000, 0x7C40):04X}")


def main():
    """Entry point: accept an ID from the command line and print its CRC.

    If no ID is provided, defaults to 8192.
    """
    parser = argparse.ArgumentParser(description="Print ID and computed CRC for a numeric ID")
    parser.add_argument(
        'id',
        nargs='?',
        default='8192',
        metavar='ID',
        help='Numeric ID (0-9999). Defaults to 8192 if omitted.'
    )
    args = parser.parse_args()

    try:
        var_id = int(args.id)
    except ValueError:
        parser.error('ID must be a decimal number between 0 and 9999')

    if var_id < 0 or var_id > 9999:
        parser.error('ID must be between 0 and 9999')

    PrintID(var_id)


if __name__ == '__main__':
    main()
