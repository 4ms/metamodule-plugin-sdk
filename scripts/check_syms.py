#!/usr/bin/env python3

import argparse
import logging
from helpers import read_symbol_list
from elftools.elf.elffile import ELFFile
from elftools.elf.sections import SymbolTableSection

def GetPluginRequiredSymbolNames(file):
    needed_syms = []
    elf = ELFFile(file)
    mmsyms = elf.get_section_by_name(".symtab")
    if isinstance(mmsyms, SymbolTableSection):
        for i in range(mmsyms.num_symbols()):
            n = mmsyms.get_symbol(i).name
            l = mmsyms.get_symbol(i)['st_info']['bind']
            t = mmsyms.get_symbol(i)['st_info']['type']
            ndx = mmsyms.get_symbol(i)['st_shndx']
            if n != "" and l == "STB_GLOBAL" and ndx == 'SHN_UNDEF':
                logging.debug(f"{i}: bind:{l} type:{t} {n}")
                needed_syms.append(n)
    return needed_syms


if __name__ == "__main__":
    parser = argparse.ArgumentParser("Check if all symbols in the plugin would be resolved by the symbols in a given symbol table")
    parser.add_argument("--plugin", required=True, help="plugin file to check (.so file)")
    parser.add_argument("--api", required=True, help="API-provided symbols (json or text file)")
    parser.add_argument("-v", dest="verbose", help="Verbose logging", action="store_true")
    args = parser.parse_args()

    if args.verbose:
        logging.basicConfig(level=logging.DEBUG, format='%(message)s')
    else:
        logging.basicConfig(level=logging.INFO, format='%(message)s')


    provided_syms = read_symbol_list(args.api)

    required_syms = []
    logging.info(f"Checking if symbols in {args.plugin} would be resolved")
    with open(args.plugin, "rb") as f:
        required_syms += GetPluginRequiredSymbolNames(f)

    # remove duplicates
    required_syms = list(set(required_syms))

    missing_syms = []
    for sym in required_syms:
        if sym not in provided_syms:
            missing_syms.append(sym)
            logging.error(f"Symbol in plugin not found in api: {sym}")

    if len(missing_syms) == 0:
        logging.info(f"All symbols found!")


