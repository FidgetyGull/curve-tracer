import serial
import matplotlib.pyplot as plt

# CONFIGURATION
SERIAL_PORT = "COM5" # Must match operating system

BAUD_RATE = 115200
TIMEOUT_S = 5

def run_sweep(port: str, baud: int):

    with serial.Serial(port, baud, timeout=TIMEOUT_S) as ser:

        ser.reset_input_buffer()

        while True:
            line = ser.readline().decode(errors="ignore").strip()
            if line:
                print(line)
            if line == "READY":
                break

        # Begin esp32 curve tracing program by sending "RUN" command
        print("Sending RUN")
        ser.write(b"RUN\n")

        v_diode_list = []
        current_list = []
        v_forced_list = []
        dac_code_list = []

        reading = False
        while True:
            line = ser.readline().decode(errors="ignore").strip()
            if not line:
                continue
 
            if line == "BEGIN":
                reading = True
                continue
            if line == "END":
                break
            if line.startswith("dac_code"):
                continue  # header row
 
            if reading:
                parts = line.split(",")
                if len(parts) != 4:
                    continue
                dac_code, v_forced, v_diode, current_mA = parts
                dac_code_list.append(int(dac_code))
                v_forced_list.append(float(v_forced))
                v_diode_list.append(float(v_diode))
                current_list.append(float(current_mA))
 
    return v_diode_list, current_list, v_forced_list, dac_code_list
 
 
def plot_curve(v_diode, current_mA):
    fig, (ax1, ax2) = plt.subplots(1, 2, figsize=(11, 4.5))
 
    # Linear I-V curve
    ax1.plot(v_diode, current_mA, marker=".", markersize=2, linewidth=1)
    ax1.set_xlabel("Diode voltage (V)")
    ax1.set_ylabel("Current (mA)")
    ax1.set_title("1N4148 I-V curve")
    ax1.grid(True, alpha=0.3)
 
    # Semi-log plot: exponential region should look like a straight line
    positive = [(v, i) for v, i in zip(v_diode, current_mA) if i > 0]
    if positive:
        v_pos, i_pos = zip(*positive)
        ax2.semilogy(v_pos, i_pos, marker=".", markersize=2, linewidth=1)
    ax2.set_xlabel("Diode voltage (V)")
    ax2.set_ylabel("Current (mA, log scale)")
    ax2.set_title("Semi-log view")
    ax2.grid(True, which="both", alpha=0.3)
 
    fig.tight_layout()
    plt.show()
 
 
if __name__ == "__main__":
    v_diode, current_mA, v_forced, dac_code = run_sweep(SERIAL_PORT, BAUD_RATE)
    print(f"Captured {len(v_diode)} points")
    plot_curve(v_diode, current_mA)
