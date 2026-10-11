def crc_xmodem_update(crc, data):
    crc = crc ^ (data << 8)
    for _ in range(8):
        if crc & 0x8000:
            crc = ((crc << 1) ^ 0x1021) & 0xFFFF
        else:
            crc = (crc << 1) & 0xFFFF
    return crc


def calc_crc(msg):
    x = 0
    for byte in msg:
        x = crc_xmodem_update(x, byte)
    return x


def make_output_code(tcode):
    framewidth = 24
    outputcode = [0] * 60
    codeLen = 0

    # Split ID into digits
    tdigit7 = tcode % 10
    tdigit6 = (tcode // 10) % 10
    tdigit5 = (tcode // 100) % 10
    tdigit4 = (tcode // 1000) % 10
    tdigit3 = (tcode // 10000) % 10
    tdigit2 = (tcode // 100000) % 10
    tdigit1 = (tcode // 1000000) % 10

    # Fill in code values
    fullcode = [0] * 6
    fullcode[0] = 0xF0 + tdigit1
    fullcode[1] = 0x00
    fullcode[2] = (tdigit2 << 4) + tdigit3
    fullcode[3] = (tdigit4 << 4) + tdigit5
    fullcode[4] = (tdigit6 << 4) + tdigit7
    fullcode[5] = 0x00

    # Debug: Initial code
    print(f"ID number: {tcode}")
    print("Initial code: ")
    print(
        f"{fullcode[0]:X} {fullcode[1]:X} {fullcode[2]:X} {fullcode[3]:X} {fullcode[4]:X} {fullcode[5]:X}"
    )

    # Generate CRC check bits
    CRC1 = [fullcode[4], fullcode[3], fullcode[2], fullcode[0]]
    crc_out = calc_crc(CRC1)

    # Fill in check bits
    fullcode[1] = (crc_out >> 8) & 0xFF
    fullcode[5] = crc_out & 0xFF

    # Debug: CRC details
    print("Feeding bytes into CRC16 XMODEM: ")
    print(f"{fullcode[4]:X} {fullcode[3]:X} {fullcode[2]:X} {fullcode[0]:X}")
    print("Got back CRC high byte: ")
    print(f"{fullcode[1]:X}")
    print("CRC low byte: ")
    print(f"{fullcode[5]:X}")
    print("Full code: ")
    print(
        f"{fullcode[0]:X} {fullcode[1]:X} {fullcode[2]:X} {fullcode[3]:X} {fullcode[4]:X} {fullcode[5]:X}"
    )
    # Invert all bytes (masked to 8 bits to match byte behavior)
    fullcode[0] = (~fullcode[0]) & 0xFF
    fullcode[1] = (~fullcode[1]) & 0xFF
    fullcode[2] = (~fullcode[2]) & 0xFF
    fullcode[3] = (~fullcode[3]) & 0xFF
    fullcode[4] = (~fullcode[4]) & 0xFF
    fullcode[5] = (~fullcode[5]) & 0xFF

    # Debug: After inversion
    print("Full code after inversion: ")
    print(
        f"0{fullcode[0]:X} {fullcode[1]:X} {fullcode[2]:X} {fullcode[3]:X} {fullcode[4]:X} {fullcode[5]:X}"
    )
    #return fullcode
    
# Test a known code
make_output_code(2545114)