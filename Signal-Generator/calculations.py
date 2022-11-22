# The script creates a memory initialization file in Verilog language.
import math
import numpy as np

# Set input data:
# bit width of DDS generator output data (OutputDataWidth).
# bit width of DDS generator memory address bus (MemoryAddressWidth).
OutputDataWidth = 16;
MemoryAddressWidth = 8;

# Step of phase increment
Step = (2.0 * math.pi) / float(2 ** MemoryAddressWidth)

# Create an array of all possible phase values (Phase).
Phase = np.arange(0, 2.0 * math.pi, Step)
Sin = np.sin(Phase)

# Converting phase values to a range of binary numbers.
Sin = (Sin * (2 ** (OutputDataWidth - 1) - 1)) + 2**(OutputDataWidth-1)
Sin = np.int_(Sin)

# Let's open a text file with phase values in binary code and generate
# a file for initialize vivado's built-in memory primitive.
# Open the file for reading and read all the lines from the file.
arrayString = "const std::array<uint8_t, 256> sin {"

for i in range(16):
    arrayString += "\n"
    arrayString += "\t"
    for j in range(16):
        arrayString += str(Sin[i*16 + j]) + ", "

arrayString = arrayString[:-2]
arrayString += "\n};"

print(arrayString)