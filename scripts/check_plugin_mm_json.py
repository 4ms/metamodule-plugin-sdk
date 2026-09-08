#!/usr/bin/env python3

# Validates a plugin's plugin-mm.json file:
#  - syntactically valid JSON
#  - every slug in MetaModuleIncludedModules exists in the modules list
#    of the corresponding plugin.json

import json
import argparse
import sys


def load_json(path):
    try:
        with open(path, "r") as f:
            return json.load(f)
    except FileNotFoundError:
        print(f"**** Error: cannot find {path}")
        return None
    except json.JSONDecodeError as e:
        print(f"**** Error: JSON syntax error in {path}: {e}")
        return None


def module_slugs(plugin_json):
    slugs = set()
    for module in plugin_json.get("modules", []):
        if isinstance(module, dict) and "slug" in module:
            slugs.add(module["slug"])
    return slugs


def check(plugin_mm_json_path, plugin_json_path):
    plugin_mm = load_json(plugin_mm_json_path)
    plugin = load_json(plugin_json_path)
    if plugin_mm is None or plugin is None:
        return 0

    known_slugs = module_slugs(plugin)

    for module in plugin_mm.get("MetaModuleIncludedModules", []):
        if not isinstance(module, dict):
            continue
        slug = module.get("slug")
        if slug is None:
            print("************************WARNING******************")
            print("A module in MetaModuleIncludedModules of")
            print(f"{plugin_mm_json_path}")
            print("has no `slug` field.")
            print("*************************************************")
            continue

        if slug not in known_slugs:
            print("************************WARNING******************")
            print(f"`{slug}`")
            print("is a module slug in plugin-mm.json, but there is no")
            print("module in plugin.json with that slug.")
            print("This module will not appear on the MetaModule website,")
            print("and firmware will not be able to see its tags.")
            print("*************************************************")

    return 0


def main():
    parser = argparse.ArgumentParser(description="Validate plugin-mm.json")
    parser.add_argument("--plugin-mm-json", required=True, help="path to plugin-mm.json")
    parser.add_argument("--plugin-json", required=True, help="path to plugin.json")
    args = parser.parse_args()

    return check(args.plugin_mm_json, args.plugin_json)


if __name__ == "__main__":
    sys.exit(main())
