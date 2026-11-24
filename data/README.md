# data/

Raw logs, one folder per test run, named `YYYY-MM-DD_<test-or-purpose>/`. Each folder holds:

- `*.csv`: parsed measurements (see `tools/README.md` for the columns)
- `*.log`: the untouched serial output
- `notes.md`: conditions (temperature, target distance, supply, firmware commit, what you did)

Created by `tools/log_capture.py`. Never edit a CSV by hand; fix the analysis script instead.
