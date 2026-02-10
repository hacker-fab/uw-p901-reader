import argparse
from serial import Serial
import csv
import sys
import time
import lib901p



def main():
    path = sys.argv[1]

    with lib901p.PressureSensor(path, 9600) as sens, open("output.csv", "w") as file:
        try:
            t0 = time.monotonic_ns()
            while True:
                pressure = sens.query_raw("PR3")

                now = time.monotonic_ns()
                dt = (now - t0) / 1_000_000_000

                file.write(f"{dt:.9f},{pressure}\n")
                print(f"{dt:5.3f}: {pressure}")

                time.sleep(0.7)
                
        except KeyboardInterrupt:
            pass

if __name__ == "__main__":
    main()