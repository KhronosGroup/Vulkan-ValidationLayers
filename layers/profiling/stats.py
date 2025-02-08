#!/usr/bin/env python3
import json
import subprocess
import tempfile
import os
import sys
import argparse
import pandas as pd
from statistics import median
from concurrent.futures import ThreadPoolExecutor, as_completed
import math
import csv

def validate_json(json_data):
    """Validate the JSON data structure."""
    if not isinstance(json_data, dict):
        raise ValueError("JSON data must be a dictionary.")
    if "zone" not in json_data:
        raise ValueError("JSON data must contain a 'zone' key.")
    if not isinstance(json_data["zone"], list) or not all(isinstance(item, str) for item in json_data["zone"]):
        raise ValueError("The 'zone' key must be an array of strings.")

def run_tracy_export(tracy_csvexport_path, tracy_file, args, filter_substring=None):
    """Run tracy-csvexport with the given arguments and filter."""
    command = [tracy_csvexport_path] + args  # e.g., "-g" or "-u"
    if filter_substring:
        command.extend(["-f", filter_substring])
    command.append(tracy_file)  # File name is always the last argument

    with tempfile.NamedTemporaryFile(mode='w+', delete=False, suffix=".csv") as tmp_file:
        subprocess.run(command, stdout=tmp_file)
        tmp_file_path = tmp_file.name

    return tmp_file_path

def process_csv(file_path, columns, verbose=False):
    """Process the CSV file and extract relevant data."""
    try:
        df = pd.read_csv(file_path, skiprows=1, header=None)
        if df.empty:  # Skip processing if the CSV is empty
            return None
        df.columns = columns
        return df
    except pd.errors.EmptyDataError:
        return None

def compute_statistics(data):
    """Compute statistics (count, average, median, min, max) for the given data list."""
    stats = {
        "count": len(data),
        "average": sum(data) / len(data),
        "median": median(data),
        "min": min(data),
        "max": max(data)
    }
    return stats

def process_gpu_data_substring(substring, tracy_file, tracy_csvexport_path, verbose=False):
    """
    Process GPU data for a single zone substring.
    Returns a dictionary mapping zone names to lists of GPU times.
    """
    result = {}
    if verbose:
        print(f"Processing GPU zone {substring}...")
    csv_file = run_tracy_export(tracy_csvexport_path, tracy_file, ["-g"], substring)
    df = process_csv(csv_file, ["name", "source_file", "time_start", "gpu_time"], verbose)
    os.remove(csv_file)

    if df is not None:
        for _, row in df.iterrows():
            name = row["name"]
            gpu_time = row["gpu_time"]
            result.setdefault(name, []).append(gpu_time)
    return result

def process_unwrap_data_substring(substring, tracy_file, tracy_csvexport_path, verbose=False):
    """
    Process CPU data for a single zone substring.
    Returns a dictionary mapping zone names to lists of execution times.
    """
    result = {}
    if verbose:
        print(f"Processing CPU zone {substring}...")
    csv_file = run_tracy_export(tracy_csvexport_path, tracy_file, ["-u"], substring)
    df = process_csv(csv_file, ["name", "source_file", "source_line", "time_start", "exec_time", "thread", "value"], verbose)
    os.remove(csv_file)

    if df is not None:
        for _, row in df.iterrows():
            name = row["name"]
            exec_time = row["exec_time"]
            result.setdefault(name, []).append(exec_time)
    return result

def main(tracy_csvexport_path, tracy_file, zones_override, zones_json, verbose, csv_path):
    # Determine the zone substrings to use.
    if zones_override:
        zone_substrings = [z.strip() for z in zones_override.split(',') if z.strip()]
    elif zones_json:
        try:
            with open(zones_json, 'r') as f:
                json_data = json.load(f)
            validate_json(json_data)
            zone_substrings = json_data["zone"]
        except Exception as e:
            print(f"Error loading JSON file '{zones_json}': {e}")
            sys.exit(1)
    else:
        print("No zone substrings provided (--zones or --zones_json). Exiting.")
        sys.exit(0)

    results = {}

    # Process GPU data in parallel.
    with ThreadPoolExecutor() as executor:
        gpu_futures = {
            executor.submit(process_gpu_data_substring, substring, tracy_file, tracy_csvexport_path, verbose): substring
            for substring in zone_substrings
        }
        for future in as_completed(gpu_futures):
            substring = gpu_futures[future]
            try:
                gpu_result = future.result()
                for key, times in gpu_result.items():
                    results.setdefault(key, []).extend(times)
            except Exception as exc:
                print(f"GPU data processing generated an exception for zone {substring}: {exc}")

    # Process CPU data in parallel.
    with ThreadPoolExecutor() as executor:
        cpu_futures = {
            executor.submit(process_unwrap_data_substring, substring, tracy_file, tracy_csvexport_path, verbose): substring
            for substring in zone_substrings
        }
        for future in as_completed(cpu_futures):
            substring = cpu_futures[future]
            try:
                cpu_result = future.result()
                for key, times in cpu_result.items():
                    results.setdefault(key, []).extend(times)
            except Exception as exc:
                print(f"CPU data processing generated an exception for zone {substring}: {exc}")

    # Prepare table rows: one header row and three data rows per zone.
    headers = ["Zone Name", "Count", "Avg (ms)", "Median (ms)", "Min (ms)", "Max (ms)"]
    data_rows = []
    for zone_name, times in results.items():
        if not times:
            continue

        overall_stats = compute_statistics(times)
        sorted_times = sorted(times, reverse=True)

        # Compute stats for the top 25% zone times.
        top25_count = max(1, math.ceil(len(times) * 0.25))
        top25_stats = compute_statistics(sorted_times[:top25_count])

        # Compute stats for the top 10% zone times.
        top10_count = max(1, math.ceil(len(times) * 0.10))
        top10_stats = compute_statistics(sorted_times[:top10_count])

        overall_row = [
            zone_name,
            str(overall_stats["count"]),
            f"{overall_stats['average'] / 1e6:.2f}",
            f"{overall_stats['median'] / 1e6:.2f}",
            f"{overall_stats['min'] / 1e6:.2f}",
            f"{overall_stats['max'] / 1e6:.2f}"
        ]
        top25_row = [
            f"{zone_name} (top 25%)",
            str(top25_stats["count"]),
            f"{top25_stats['average'] / 1e6:.2f}",
            f"{top25_stats['median'] / 1e6:.2f}",
            f"{top25_stats['min'] / 1e6:.2f}",
            f"{top25_stats['max'] / 1e6:.2f}"
        ]
        top10_row = [
            f"{zone_name} (top 10%)",
            str(top10_stats["count"]),
            f"{top10_stats['average'] / 1e6:.2f}",
            f"{top10_stats['median'] / 1e6:.2f}",
            f"{top10_stats['min'] / 1e6:.2f}",
            f"{top10_stats['max'] / 1e6:.2f}"
        ]
        data_rows.append(overall_row)
        data_rows.append(top25_row)
        data_rows.append(top10_row)

    # Branch based on the csv_path option.
    if csv_path:
        # CSV mode now writes a CSV file with the exact same table as printed to stdout.
        try:
            with open(csv_path, 'w', newline='') as csvfile:
                writer = csv.writer(csvfile)
                writer.writerow(headers)
                writer.writerows(data_rows)
            print(f"CSV exported to {csv_path}")
        except Exception as e:
            print(f"Error writing CSV file: {e}")
        return
    else:
        # Normal stdout printing mode: print a formatted table.
        col_widths = [len(h) for h in headers]
        for row in data_rows:
            for i, cell in enumerate(row):
                col_widths[i] = max(col_widths[i], len(cell))
        fmt = " | ".join("{:<" + str(w) + "}" for w in col_widths)
        sep_line = "-" * (sum(col_widths) + 3 * (len(col_widths) - 1))
        
        print("\nZone Execution Time Statistics:")
        print(fmt.format(*headers))
        print(sep_line)
        for row in data_rows:
            print(fmt.format(*row))

if __name__ == "__main__":
    parser = argparse.ArgumentParser(
        description="Process tracy files and compute zone statistics. "
                    "Zones may be provided via --zones (comma separated) or --zones_json (JSON file). "
                    "Optionally export the table to CSV using --csv."
    )
    # New mandatory positional argument for tracy-csvexport executable:
    parser.add_argument("tracy_csvexport_path", help="Path to the tracy-csvexport executable.")
    # The .tracy file to process is now the second positional argument:
    parser.add_argument("tracy_file", help="Path to the .tracy file to process.")
    parser.add_argument("--zones", help="Comma separated list of zone substrings.")
    parser.add_argument("--zones_json", help="Path to a JSON file containing zone substrings (key 'zone').")
    parser.add_argument("--csv", help="Path to export CSV file. In CSV mode the table is identical to the printed table.")
    parser.add_argument("--verbose", "-v", action="store_true", help="Enable verbose output.")
    args = parser.parse_args()

    main(args.tracy_csvexport_path, args.tracy_file, args.zones, args.zones_json, args.verbose, args.csv)
