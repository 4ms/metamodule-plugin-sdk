import os
from shutil import which
import subprocess
import re

def Log(x):
    TAG = "    png.py: "
    print(TAG+x)


def convertSvgToPng(svgFilename, outputDir, bg="none", resize=0, exportLayer="all"):
    outputDir = outputDir.rstrip("/") + "/"

    pngFilename = outputDir + os.path.splitext(os.path.basename(svgFilename))[0] + ".png"

    inkscapeBin = which('inkscape') or os.getenv('INKSCAPE_BIN_PATH')
    if inkscapeBin is None:
        Log("inkscape is not found. Please put it in your shell PATH, or set INKSCAPE_BIN_PATH to the path to the binary")
        Log("Aborting")
        return

    bg = "--export-background=white" if bg == "white" else ""

    exportLayer = f"--export-id=\"{exportLayer}\" --export-id-only" if exportLayer != "all" else ""

    if resize == 0:
        dpi = determine_dpi(svgFilename)
        export_size = f"--export-dpi={dpi}"
        resize_str = f"{dpi} dpi"
    else:
        export_size = f"--export-height={resize}"
        resize_str = f"{resize} px high"


    inkscape_cmd = f'{inkscapeBin} --export-type="png" {exportLayer} --export-png-use-dithering=false {export_size} {bg} --export-filename=\"{pngFilename}\" {svgFilename}'

    try:
        subprocess.run(f'{inkscape_cmd}', shell=True, check=True)
        Log(f"Converted {svgFilename} to {os.path.basename(pngFilename)} at {resize_str}.")
    except:
        Log(f"Failed running {inkscape_cmd}. Aborting")
        return


def determine_dpi(filename):
    # Workaround for different SVGs;
    # Some need to be exported at 47.44 DPI (which makes sense since 240px screen = 5.059" module)
    # But it seems some are internally 96DPI, so they're scaled by a factor of 96/75, and need 
    # to be exported at 60.72 DPI. I don't know the reason, but the common factor seems to be
    # the occurance of 'width="*mm"' somewhere in the file contents

    with open(filename, 'r') as fp:
        contents = fp.read()
        m = re.search('width=".+mm"',contents)
        if m is None:
            return 60.72
        if m.group(0) is None:
            return 60.72
        else:
            return 47.44

