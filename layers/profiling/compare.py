#!/usr/bin/env python3
import csv
import sys
import argparse
import re

# Regular expression to remove ANSI escape sequences.
ansi_escape = re.compile(r'\x1B\[[0-?]*[ -/]*[@-~]')

def strip_ansi(s):
    """Remove ANSI escape sequences from a string."""
    return ansi_escape.sub('', s)

def pad_ansi(s, width):
    """
    Pads the string s (which may contain ANSI codes) with spaces on the right so that its
    visible length (i.e. after stripping ANSI codes) is at least 'width'.
    """
    visible = strip_ansi(s)
    pad_count = width - len(visible)
    return s + " " * pad_count

def read_overall_data(filename):
    """
    Reads the CSV file and returns a dictionary mapping zone names to metrics.
    Only rows whose "Zone Name" does not include a parenthesized suffix (e.g., " (top 5%)")
    are considered (i.e. the overall row).

    The metrics are parsed as:
      - Count: int
      - Avg (ms), Median (ms), Min (ms), Max (ms): float
    """
    data = {}
    try:
        with open(filename, 'r', newline='') as csvfile:
            reader = csv.DictReader(csvfile)
            for row in reader:
                zone = row.get("Zone Name", "").strip()
                try:
                    count = int(row.get("Count", "0"))
                    avg = float(row.get("Avg (ms)", "0"))
                    median_val = float(row.get("Median (ms)", "0"))
                    min_val = float(row.get("Min (ms)", "0"))
                    max_val = float(row.get("Max (ms)", "0"))
                except Exception as e:
                    print(f"Error parsing metrics for zone '{zone}' in file '{filename}': {e}", file=sys.stderr)
                    continue
                data[zone] = {
                    "Count": count,
                    "Avg (ms)": avg,
                    "Median (ms)": median_val,
                    "Min (ms)": min_val,
                    "Max (ms)": max_val
                }
    except FileNotFoundError:
        print(f"Error: File '{filename}' not found.", file=sys.stderr)
        sys.exit(1)
    return data

def format_diff(ref_val, comp_val):
    """
    Computes the difference between a reference value and a comparison value.
    Returns a string formatted as a signed percentage along with the absolute difference
    in milliseconds, e.g.: "+12.34% (+5.67 ms)".
    
    A color gradient is applied so that:
      - For positive differences, the text is tinted red, with brighter red for +100% and above.
      - For negative differences, the text is tinted green, with brighter green for -100% and below.
      - At 0% the text appears white.
      
    If the reference value is zero, returns "N/A".
    """
    if ref_val == 0:
        return "N/A"
    diff = comp_val - ref_val
    perc_diff = diff / ref_val * 100.0

    # Determine the color gradient.
    if perc_diff >= 0:
        # Clamp percentage to 100 if above 100.
        p = min(perc_diff, 100)
        R = 255
        # Green and Blue go from 255 at 0% to 0 at 100%
        GB = int(255 - (255 * p / 100))
        G = GB
        B = GB
    else:
        p = min(abs(perc_diff), 100)
        G = 255
        # Red and Blue go from 255 at 0% to 0 at -100%
        RB = int(255 - (255 * p / 100))
        R = RB
        B = RB

    # ANSI 24-bit color escape sequence.
    color_code = f"\033[38;2;{R};{G};{B}m"
    reset_code = "\033[0m"
    diff_str = f"{perc_diff:+.2f}% ({diff:+.2f} ms)"
    return f"{color_code}{diff_str}{reset_code}"

def main(reference_csv, comparison_csv):
    # Read overall timing data from both CSV files.
    ref_data = read_overall_data(reference_csv)
    comp_data = read_overall_data(comparison_csv)

    # Determine common zones and zones missing in one file.
    common_zones    = set(ref_data.keys()) & set(comp_data.keys())
    missing_in_comp = set(ref_data.keys()) - common_zones
    extra_in_comp   = set(comp_data.keys()) - common_zones

    # Build rows for common zones.
    # Each row is: [Zone Name, Count Diff, Avg Diff, Median Diff, Min Diff, Max Diff]
    rows = []
    for zone in sorted(common_zones):
        ref_metrics = ref_data[zone]
        comp_metrics = comp_data[zone]
        count_diff = comp_metrics["Count"] - ref_metrics["Count"]
        avg_diff    = format_diff(ref_metrics["Avg (ms)"], comp_metrics["Avg (ms)"])
        median_diff = format_diff(ref_metrics["Median (ms)"], comp_metrics["Median (ms)"])
        min_diff    = format_diff(ref_metrics["Min (ms)"], comp_metrics["Min (ms)"])
        max_diff    = format_diff(ref_metrics["Max (ms)"], comp_metrics["Max (ms)"])
        rows.append([zone, str(count_diff), avg_diff, median_diff, min_diff, max_diff])

    # Define table headers.
    headers = ["Zone Name", "Count Diff", "Avg Diff", "Median Diff", "Min Diff", "Max Diff"]

    # Compute column widths (based on visible text only).
    col_widths = [len(header) for header in headers]
    for row in rows:
        for i, cell in enumerate(row):
            plain_cell = strip_ansi(cell)
            col_widths[i] = max(col_widths[i], len(plain_cell))

    # Build a header line using the custom pad function.
    header_line = " | ".join(pad_ansi(header, col_widths[i]) for i, header in enumerate(headers))
    sep_line = "-" * (sum(col_widths) + 3 * (len(col_widths) - 1))

    # Print the table header.
    print(f"\nZone Timing Comparison ({reference_csv} vs {comparison_csv}):")
    print(header_line)
    print(sep_line)
    
    # Print each row, padding each cell based on its visible width.
    for row in rows:
        padded_cells = [pad_ansi(cell, col_widths[i]) for i, cell in enumerate(row)]
        print(" | ".join(padded_cells))

    # Report zones present in one CSV but not in the other.
    if missing_in_comp:
        print(f"\nZones present in {reference_csv} but missing in {comparison_csv}:")
        for zone in sorted(missing_in_comp):
            print(" -", zone)
    if extra_in_comp:
        print(f"\nZones present in {comparison_csv} but missing in {reference_csv}:")
        for zone in sorted(extra_in_comp):
            print(" -", zone)

if __name__ == "__main__":
    parser = argparse.ArgumentParser(
        description="Compare two CSV files of zone timings (overall rows only). The first CSV is used as the reference."
    )
    parser.add_argument("reference_csv", help="Reference CSV file")
    parser.add_argument("comparison_csv", help="CSV file to compare")
    args = parser.parse_args()
    
    # Pass the parsed arguments to main as parameters.
    main(args.reference_csv, args.comparison_csv)
