# generate_lightmap_26sections.py
FILENAME = "lightmap.txt"

SECTION_SIZE = 2048
NUM_SECTIONS = 26
BYTE_VALUE = 0xFF

def encode_varint(value):
    """Zwraca listę bajtów VarInt"""
    bytes_ = []
    val = value
    while True:
        temp = val & 0x7F
        val >>= 7
        if val != 0:
            temp |= 0x80
        bytes_.append(temp)
        if val == 0:
            break
    return bytes_

light_map_data = []
for _ in range(NUM_SECTIONS):
    light_map_data.extend(encode_varint(SECTION_SIZE))
    light_map_data.extend([BYTE_VALUE] * SECTION_SIZE)

# zapis w formacie C++
with open(FILENAME, "w") as f:
    f.write("const uint8_t lightMap[] = {\n")
    for i, b in enumerate(light_map_data):
        f.write(f"0x{b:02X}")
        if i != len(light_map_data) - 1:
            f.write(", ")
        if (i + 1) % 16 == 0:
            f.write("\n")
    f.write("\n};\n")

print(f"Gotowe! lightMap z 26 sekcjami zapisane do {FILENAME}, bajtów: {len(light_map_data)}")
