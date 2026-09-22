#!/usr/bin/env python3

# Validates a plugin's plugin-mm.json file:
#  - syntactically valid JSON
#  - every slug in MetaModuleIncludedModules exists in the modules list
#    of the corresponding plugin.json
#  - element groups and order, if any, are shaped correctly

import json
import argparse
import re
import sys

TYPED_INDEX = re.compile(r"^(elem|param|in|out|light):\d+$")


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


def check_groups(plugin_mm_json_path, slug, module):
    groups = module.get("groups")
    if groups is None:
        return

    def warn(*lines):
        print("************************WARNING******************")
        print(f"Module `{slug}` in {plugin_mm_json_path}:")
        for line in lines:
            print(line)
        print("*************************************************")

    if not isinstance(groups, dict):
        warn("`groups` must be an object of group name => list of elements.")
        return

    for group_name, members in groups.items():
        if not isinstance(members, list) or not members:
            warn(f"element group `{group_name}` must be a non-empty list of elements.")
            continue

        for member in members:
            if not isinstance(member, str) or not member:
                warn(f"element group `{group_name}` has a member that is not a string.")
            elif ":" in member and not TYPED_INDEX.match(member):
                warn(
                    f"element group `{group_name}` member `{member}` looks like a typed",
                    "index but is not one. Use param:N, in:N, out:N, light:N or elem:N.",
                )

    if module.get("class") is None:
        return

    if not isinstance(module.get("class"), str):
        warn("`class` must be the name of the class that declares the module's enums.")


def check_order(plugin_mm_json_path, slug, module):
    order = module.get("order")
    if order is None:
        return

    def warn(*lines):
        print("************************WARNING******************")
        print(f"Module `{slug}` in {plugin_mm_json_path}:")
        for line in lines:
            print(line)
        print("*************************************************")

    if not isinstance(order, list):
        warn("`order` must be a list of group names and elements.")
        return

    seen = set()
    for item in order:
        if not isinstance(item, str) or not item:
            warn("`order` has an item that is not a string.")
            continue

        if ":" in item and not TYPED_INDEX.match(item):
            warn(
                f"`order` item `{item}` looks like a typed index but is not one.",
                "Use param:N, in:N, out:N, light:N or elem:N.",
            )

        if item.casefold() in seen:
            warn(f"`order` lists `{item}` more than once: only the first counts.")
        seen.add(item.casefold())


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

        check_groups(plugin_mm_json_path, slug, module)
        check_order(plugin_mm_json_path, slug, module)

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
