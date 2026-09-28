#!/usr/bin/env python3
"""Check a plugin's element groups, order, and names (plugin-mm.json) before running it.

Enum names in groups/order/names are resolved when the plugin is built, so a typo there
is a build error. But display names ("Cutoff") are only matched against the module's
elements at runtime, so a typo there is only reported on hardware (or in the simulator).

This script builds the MetaModule headless simulator with the plugin built in as an
ext-plugin, creates each module, and resolves its groups/order/names with the same code
the module view uses. Every mistake is reported, and the exit code is non-zero if there
were any.

If the plugin has been built with the SDK, enum names are resolved against that build
(its <name>-debug.so), so they're checked too: e.g. an enum and a display name that
refer to the same element, or an enum in `order` that's also in a group.

Usage:
    check_element_layout.py --firmware-repo ~/meta-module --plugin ~/MyPlugin --all-modules
    check_element_layout.py --firmware-repo ~/meta-module --plugin ~/MyPlugin --module Osc --module Filter

--firmware-repo is a checkout of the MetaModule firmware repo (github.com/4ms/metamodule),
which contains the simulator. --plugin is the dir with the plugin's CMakeLists.txt and
plugin-mm.json.
"""
import argparse
import json
import os
import re
import subprocess
import sys
import tempfile
from pathlib import Path

SCRIPTS_DIR = Path(__file__).resolve().parent
RESOLVE_ELEMENT_GROUPS = SCRIPTS_DIR / 'resolve_element_groups.py'


def fail(message):
    print(f'**** Error: {message}')
    sys.exit(1)


def load_json(path):
    try:
        with open(path) as f:
            return json.load(f)
    except FileNotFoundError:
        fail(f'cannot find {path}')
    except json.JSONDecodeError as e:
        fail(f'JSON syntax error in {path}: {e}')


def create_plugin_arg(cmakelists, name):
    """The value of an argument to create_plugin() in the plugin's CMakeLists.txt,
    if it's written as a literal (not a variable)"""
    text = cmakelists.read_text()
    call = re.search(r'create_plugin\s*\((.*?)\)', text, re.DOTALL)
    if not call:
        return None
    arg = re.search(rf'\b{name}\s+("[^"]*"|\S+)', call.group(1))
    if not arg:
        return None
    value = arg.group(1).strip('"')
    return None if '${' in value else value


def find_plugin_json(plugin_dir, cmakelists):
    """plugin.json: as given to create_plugin(), or next to CMakeLists.txt, or in its parent dir"""
    text = cmakelists.read_text()
    call = re.search(r'create_plugin\s*\((.*?)\)', text, re.DOTALL)
    if call:
        arg = re.search(r'\bPLUGIN_JSON\s+("[^"]*"|\S+)', call.group(1))
        if arg:
            value = arg.group(1).strip('"')
            value = value.replace('${CMAKE_CURRENT_LIST_DIR}', str(plugin_dir))
            value = value.replace('${CMAKE_CURRENT_SOURCE_DIR}', str(plugin_dir))
            if '${' not in value:
                path = Path(value)
                path = path if path.is_absolute() else plugin_dir / path
                if path.exists():
                    return path

    for path in (plugin_dir / 'plugin.json', plugin_dir.parent / 'plugin.json'):
        if path.exists():
            return path
    return None


def uses_enum_names(mm_json):
    return any(isinstance(m, dict) and m.get('class') for m in mm_json.get('MetaModuleIncludedModules', []))


def headless_min_omitted_brands(simulator_dir):
    """The built-in brands that the simulator's `headless-min` preset leaves out, so the
    headless build only compiles what it needs"""
    presets = load_json(simulator_dir / 'CMakePresets.json')
    for preset in presets.get('configurePresets', []):
        if preset.get('name') == 'headless-min':
            return [k for k in preset.get('cacheVariables', {}) if k.startswith('OMIT_BRAND_')]
    return []


def run(cmd, **kwargs):
    print('+ ' + ' '.join(str(c) for c in cmd), flush=True)
    return subprocess.run([str(c) for c in cmd], **kwargs).returncode


def main():
    parser = argparse.ArgumentParser(
        description='Check the element groups, order, and names in a plugin\'s plugin-mm.json',
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog=__doc__.split('Usage:')[1] if 'Usage:' in __doc__ else None)
    parser.add_argument('--firmware-repo', required=True, type=Path,
                        help='Path to the MetaModule firmware repo (which contains simulator/)')
    parser.add_argument('--plugin', required=True, type=Path,
                        help='Path to the plugin: the dir with its CMakeLists.txt and plugin-mm.json')
    which = parser.add_mutually_exclusive_group(required=True)
    which.add_argument('--module', action='append', metavar='SLUG',
                       help='Module slug to check. Can be given more than once')
    which.add_argument('--all-modules', action='store_true', help='Check every module in the plugin')
    parser.add_argument('--plugin-build-dir', type=Path,
                        help='The plugin\'s SDK build dir, containing <name>-debug.so, used to resolve enum '
                             'names. Default: <plugin>/build, if it has one')
    parser.add_argument('--lib-name', help='The plugin\'s cmake library target (SOURCE_LIB in create_plugin()). '
                                           'Default: read from CMakeLists.txt')
    parser.add_argument('--plugin-name', help='PLUGIN_NAME in create_plugin(). Default: read from CMakeLists.txt')
    parser.add_argument('--build-dir', type=Path,
                        help='Where to build the headless simulator. Default: <firmware-repo>/simulator/build-layout-check')
    args = parser.parse_args()

    firmware_repo = args.firmware_repo.expanduser().resolve()
    simulator_dir = firmware_repo / 'simulator'
    if not (simulator_dir / 'CMakeLists.txt').exists():
        fail(f'{firmware_repo} does not look like the MetaModule firmware repo (no simulator/CMakeLists.txt)')

    plugin_dir = args.plugin.expanduser().resolve()
    cmakelists = plugin_dir / 'CMakeLists.txt'
    if not cmakelists.exists():
        fail(f'no CMakeLists.txt in {plugin_dir}')

    mm_json_path = plugin_dir / 'plugin-mm.json'
    mm_json = load_json(mm_json_path)

    plugin_json_path = find_plugin_json(plugin_dir, cmakelists)
    plugin_json = load_json(plugin_json_path) if plugin_json_path else {}

    lib_name = args.lib_name or create_plugin_arg(cmakelists, 'SOURCE_LIB')
    if not lib_name:
        fail('cannot find SOURCE_LIB in create_plugin() in CMakeLists.txt: use --lib-name')

    plugin_name = args.plugin_name or create_plugin_arg(cmakelists, 'PLUGIN_NAME') or lib_name

    brand_slug = mm_json.get('MetaModuleBrandSlug') or plugin_json.get('slug')
    if not brand_slug:
        fail('cannot find the brand slug: plugin.json has no "slug", and plugin-mm.json no "MetaModuleBrandSlug"')

    build_dir = (args.build_dir or simulator_dir / 'build-layout-check').expanduser().resolve()

    with tempfile.TemporaryDirectory() as tmp:
        # Resolve enum names to typed indices, the same way the SDK does when packaging
        resolved_mm_json = Path(tmp) / 'plugin-mm.json'
        resolve_cmd = [sys.executable, RESOLVE_ELEMENT_GROUPS, '--in', mm_json_path, '--out', resolved_mm_json]

        if uses_enum_names(mm_json):
            plugin_build_dir = args.plugin_build_dir or plugin_dir / 'build'
            elf = plugin_build_dir.expanduser().resolve() / f'{plugin_name}-debug.so'
            if not elf.exists():
                fail(f'plugin-mm.json uses enum names (a module has a "class"), which are resolved against the '
                     f'plugin built with the SDK, but {elf} does not exist. Build the plugin first, or give '
                     f'its build dir with --plugin-build-dir')
            print(f'Resolving enum names against {elf}')
            if Path(elf).stat().st_mtime < mm_json_path.stat().st_mtime:
                print('Note: plugin-mm.json is newer than the plugin build. That\'s fine unless you also changed '
                      'the enums: then rebuild the plugin first.')
            resolve_cmd += ['--elf', elf]

        if run(resolve_cmd) != 0:
            fail('could not resolve enum names in plugin-mm.json')

        # Build the headless simulator with this plugin built in. Only the plugin and the
        # 4ms built-in modules are compiled.
        configure = ['cmake', '-S', simulator_dir, '-B', build_dir, '-G', 'Ninja',
                     '-DHEADLESS=ON',
                     '-DCMAKE_BUILD_TYPE=RelWithDebInfo',
                     '-DLOG_LEVEL=NONE',
                     f'-DEXT_BUILTIN_BRAND_PATHS={plugin_dir}',
                     f'-DEXT_BUILTIN_BRAND_LIBNAMES={lib_name}',
                     f'-DEXT_BUILTIN_BRAND_SLUGS={brand_slug}']
        configure += [f'-D{omit}=ON' for omit in headless_min_omitted_brands(simulator_dir)]

        if run(configure) != 0:
            fail('configuring the headless simulator failed')

        # A full build, not just the simulator target, so that plugin-mm.json is validated too
        if run(['cmake', '--build', build_dir]) != 0:
            fail('building the headless simulator failed')

        check = [build_dir / 'simulator', '--check-element-layout', '--plugin-mm-json', resolved_mm_json]
        if plugin_json_path:
            check += ['--plugin-json', plugin_json_path]
        for slug in args.module or []:
            check += ['--module', slug]

        print(flush=True)
        return run(check)


if __name__ == '__main__':
    sys.exit(main())
