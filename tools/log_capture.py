#!/usr/bin/env python3
"""Capture Pump Guard serial output into a dated run folder.

Writes two files into data/<YYYY-MM-DD>_<label>/ :
  <label>.log  every serial line exactly as received (the raw record)
  <label>.csv  only the machine-readable "DATA," lines, parsed, with a PC timestamp

Usage:
  python tools/log_capture.py --port COM3 --label ranging-20cm
  python tools/log_capture.py --port COM3 --label ranging-20cm --minutes 10
Stop with Ctrl+C. Needs: pip install pyserial
Do not run this at the same time as `idf.py monitor` (only one program can hold the port).
"""
import argparse
import csv
import re
import sys
from datetime import datetime, timezone
from pathlib import Path

# Column order of the DATA line the firmware prints (see tools/README.md).
FIRMWARE_FIELDS = [
    "uptime_ms", "echo_us", "raw_cm", "corrected_cm", "temp_c",
    "median_cm", "ok_count", "timeout_count", "rejected_count",
]
CSV_HEADER = ["pc_time_utc"] + FIRMWARE_FIELDS

# ESP-IDF may wrap the line in colour codes and a prefix like "I (1234) tag: ".
DATA_LINE = re.compile(r"DATA,([^\r\n\x1b]*)")


def parse_data_line(line):
    """Return the list of firmware fields, or None if this is not a valid DATA line."""
    match = DATA_LINE.search(line)
    if not match:
        return None
    fields = match.group(1).split(",")
    if len(fields) != len(FIRMWARE_FIELDS):
        return None  # truncated or garbled line: skip, it stays in the .log file
    return [field.strip() for field in fields]


def main():
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawTextHelpFormatter)
    parser.add_argument("--port", required=True, help="serial port, e.g. COM3 or /dev/ttyUSB0")
    parser.add_argument("--baud", type=int, default=115200)
    parser.add_argument("--label", required=True, help="short name of this run, e.g. ranging-20cm")
    parser.add_argument("--minutes", type=float, default=0, help="stop after this long (0 = until Ctrl+C)")
    parser.add_argument("--data-dir", default=str(Path(__file__).resolve().parent.parent / "data"))
    args = parser.parse_args()

    try:
        import serial  # pyserial
    except ImportError:
        sys.exit("pyserial is missing. Run: pip install pyserial")

    run_dir = Path(args.data_dir) / f"{datetime.now().strftime('%Y-%m-%d')}_{args.label}"
    run_dir.mkdir(parents=True, exist_ok=True)
    log_path = run_dir / f"{args.label}.log"
    csv_path = run_dir / f"{args.label}.csv"

    started = datetime.now(timezone.utc)
    rows_written = 0
    with serial.Serial(args.port, args.baud, timeout=1) as port, \
            open(log_path, "a", encoding="utf-8", newline="") as log_file, \
            open(csv_path, "a", encoding="utf-8", newline="") as csv_file:
        writer = csv.writer(csv_file)
        if csv_file.tell() == 0:
            writer.writerow(CSV_HEADER)
        print(f"Logging to {run_dir}  (Ctrl+C to stop)")
        try:
            while True:
                if args.minutes and (datetime.now(timezone.utc) - started).total_seconds() > args.minutes * 60:
                    break
                raw = port.readline()
                if not raw:
                    continue
                text = raw.decode("utf-8", errors="replace").rstrip("\r\n")
                log_file.write(text + "\n")
                fields = parse_data_line(text)
                if fields is not None:
                    writer.writerow([datetime.now(timezone.utc).isoformat(timespec="milliseconds")] + fields)
                    rows_written += 1
                    if rows_written % 20 == 0:
                        csv_file.flush()
                        log_file.flush()
        except KeyboardInterrupt:
            pass
    print(f"Done. {rows_written} rows in {csv_path}")


if __name__ == "__main__":
    main()
