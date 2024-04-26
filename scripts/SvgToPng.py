#!/usr/bin/env python3

import argparse
from pathlib import Path
import actions.png as png

# Version check
f"Python 3.6+ is required"

if __name__ == "__main__":
    parser = argparse.ArgumentParser(
                 prog="SvgToPng", 
                 description="MetaModule SVG to PNG Conversion Helper. Converts SVG file(s) to 47.44 dpi PNG file(s) (240px/5.059in = 47.44dpi).",
                 epilog="Requires inkscape (v1.2.2) to be present on PATH, or found at the environment variable INKSCAPE_BIN_PATH")

    parser.add_argument("--input", required=True, help="Path to .svg file or directory containing svg files")
    parser.add_argument("--output", required=True, help="Directory where converted .png files will be saved")
    parser.add_argument("--white", help="Make the background white (no transparency)", action="store_true")
    parser.add_argument("--height", help="Force the height in pixels (otherwise is deduced)")
    parser.add_argument("--layer", help="Only export the given SVG layer (otherwise export all layers)")

    args = parser.parse_args()

    height = args.height if args.height else 0
    layer = args.layer if args.layer else "all"
    bg = "white" if args.white else "none"

    try:
        if Path(args.input).is_file():
            png.convertSvgToPng(args.input, args.output, bg, height, layer)

        elif Path(args.input).is_dir():
            svg_files = Path(args.input).glob("*.svg")
            for svg_file in svg_files:
                png.convertSvgToPng(str(svg_file), args.output, bg, height, layer)

    except KeyboardInterrupt:
        pass
