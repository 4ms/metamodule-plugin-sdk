#!/usr/bin/env python3
"""Resolve enumerator names in a plugin-mm.json's element groups and order to typed indices.

A module's "groups" list members either by the element's name as shown on screen,
by a typed index ("param:3", "in:1", "out:0", "light:2", "elem:5"), or by the name
of an enumerator of the module's class ("PWM_PARAM", "CutoffKnob").

Enumerator names are a build-time convenience only: this script rewrites them to
typed indices using the DWARF info in the plugin's ELF, so the firmware only ever
sees names and typed indices. A module opts in by naming the class that declares
its enums:

    { "slug": "MyModule", "class": "MyModule", "groups": { ... } }

Enum name -> index kind:
    ParamIds/ParamId   -> param:     InputIds/InputId   -> in:
    OutputIds/OutputId -> out:       LightIds/LightId   -> light:
    Elem               -> elem:      (a native module's info struct)
A prefixed name is accepted too (FadeParamId, ExpLightIds), as are plural-first
spellings (ParamsIds, InputsIds).

A member that isn't a typed index and doesn't match an enumerator is left alone,
and is matched by name on the device.

Usage: resolve_element_groups.py --in plugin-mm.json --out plugin-mm.json --elf file.so
"""
import argparse
import json
import re
import shutil
import struct
import sys

sys.path.insert(0, __file__.rsplit('/', 1)[0])
from elftools.elf.elffile import ELFFile

# Enum type name suffix (lowercased, plural 's' stripped) -> typed index prefix
ID_ENUM_KINDS = {
    'paramid': 'param',
    'inputid': 'in',
    'outputid': 'out',
    'lightid': 'light',
}

TYPED_INDEX = re.compile(r'^(elem|param|in|out|light):\d+$')


def die_name(die):
    attr = die.attributes.get('DW_AT_name')
    return attr.value.decode() if attr else None


def enum_kind(enum_name):
    key = enum_name.lower().rstrip('s')
    # "ParamsIds" -> "paramid"
    if key.endswith('sid'):
        key = key[:-3] + 'id'
    # Elem is matched exactly: it's a native info struct's name, never prefixed
    if key == 'elem':
        return 'elem'
    # Otherwise match the suffix, so a prefixed name like "FadeParamId" works
    for suffix, kind in ID_ENUM_KINDS.items():
        if key.endswith(suffix):
            return kind
    return None


# Namespaces from standard/toolchain headers, which are included by every file and are
# large, but never hold a module's class
SKIP_NAMESPACES = {'std', '__gnu_cxx', '__cxxabiv1', '__gnu_debug'}


def collect_enums(elf_path, class_names):
    """{class name: {enumerator name: "kind:value"}} for every enum we understand that's
    declared in one of the named classes.

    Only namespaces and the named classes are descended into. Everything else (function
    bodies and the members of every other class, which are nearly all of the DWARF) is
    skipped via DW_AT_sibling without being parsed, which is what makes this fast. A
    class nested in another class or in a function is found by a slower fallback.
    """
    by_class = {}

    def add_enum(die, scope):
        kind = enum_kind(die_name(die) or '')
        if kind is None:
            return
        members = by_class.setdefault(scope, {})
        for child in die.iter_children():
            if child.tag != 'DW_TAG_enumerator':
                continue
            value = child.attributes['DW_AT_const_value'].value
            members[die_name(child)] = f'{kind}:{value}'

    def walk(die, scope):
        # scope is the name of the enclosing class, which is what "class" in the json names
        for child in die.iter_children():
            tag = child.tag
            if not child.has_children:
                continue
            if tag == 'DW_TAG_enumeration_type':
                if scope is not None:
                    add_enum(child, scope)
            elif tag == 'DW_TAG_namespace':
                if die_name(child) not in SKIP_NAMESPACES:
                    walk(child, None)
            elif tag in ('DW_TAG_structure_type', 'DW_TAG_class_type'):
                name = die_name(child)
                if name in class_names:
                    walk(child, name)

    with open(elf_path, 'rb') as f:
        elf = ELFFile(f)
        if not elf.has_dwarf_info():
            return by_class

        dwarf = elf.get_dwarf_info()
        mentions = cu_name_filter(dwarf, class_names, elf.little_endian)
        for cu in dwarf.iter_CUs():
            if mentions(cu):
                walk(cu.get_top_DIE(), None)

        # A class that wasn't found is nested somewhere the walk above doesn't go: in
        # another class, or in a function. Search every DIE of just the CUs that mention it.
        missing = set(class_names) - by_class.keys()
        if missing:
            names = ', '.join(sorted(missing))
            print(f'Warning: {names} not found at namespace level (or has no Param/Input/'
                  f'Output/LightId enums). Searching nested classes and functions. '
                  f'This might take a while...', flush=True)
            mentions = cu_name_filter(dwarf, missing, elf.little_endian)
            for cu in dwarf.iter_CUs():
                if not mentions(cu):
                    continue
                for die in cu.iter_DIEs():
                    if die.tag != 'DW_TAG_enumeration_type' or not die.has_children:
                        continue
                    scope = die_name(die.get_parent())
                    if scope in missing:
                        add_enum(die, scope)

    return by_class


def cu_name_filter(dwarf, class_names, little_endian):
    """Returns a quick test of whether a CU might declare one of the named classes.

    Most CUs just include the headers of a few other modules, and even the top level of
    each CU is a lot of DIEs to parse. A CU that declares a class has its name in its
    raw bytes, either inline (DW_FORM_string) or as an offset into .debug_str
    (DW_FORM_strp), so a byte search rules out most CUs cheaply. It can give false
    positives (which just get walked), but never false negatives.
    """
    if dwarf.debug_str_offsets_sec is not None:
        # DW_FORM_strx names are indices we can't cheaply search for
        return lambda cu: True

    info = dwarf.debug_info_sec.stream.getvalue()
    strs = dwarf.debug_str_sec.stream.getvalue() if dwarf.debug_str_sec else b''
    endian = '<' if little_endian else '>'

    needles = []
    for name in class_names:
        cstr = name.encode() + b'\0'
        needles.append(cstr)
        # The linker merges strings, so the name may also be the tail of a longer one
        pos = strs.find(cstr)
        while pos != -1:
            # 32-bit or 64-bit DWARF offset
            if pos <= 0xFFFFFFFF:
                needles.append(struct.pack(endian + 'I', pos))
            needles.append(struct.pack(endian + 'Q', pos))
            pos = strs.find(cstr, pos + 1)

    def mentions(cu):
        raw = info[cu.cu_offset:cu.cu_offset + cu.size]
        return any(needle in raw for needle in needles)

    return mentions


def resolve(mm_json, enums, verbose=False):
    """Rewrite enumerator names in place, in each module's "groups" and "order".

    Returns (number rewritten, unresolved enumerator-like names, names left to be
    matched by name on the device).
    """
    rewritten = 0
    unresolved = []
    by_name = []

    for module in mm_json.get('MetaModuleIncludedModules', []):
        if not isinstance(module, dict):
            continue

        slug = module.get('slug')
        class_name = module.get('class')
        members_by_name = enums.get(class_name, {}) if class_name else {}

        groups = module.get('groups')
        if not isinstance(groups, dict):
            groups = {}

        # The firmware matches a name in "order" against group names first, ignoring case
        group_names = {name.casefold() for name in groups}

        def rewrite(items, where, skip=lambda item: False):
            nonlocal rewritten
            for i, item in enumerate(items):
                if not isinstance(item, str) or TYPED_INDEX.match(item) or skip(item):
                    continue

                if item in members_by_name:
                    items[i] = members_by_name[item]
                    rewritten += 1
                    if verbose:
                        print(f'  {slug}/{where}: {item} -> {items[i]}')

                elif class_name and members_by_name:
                    # The module opted into enumerator names, so an item that isn't one
                    # is either an element named on screen or a typo. An ALL_CAPS name is
                    # a VCV-style id and is probably a typo. A single word might be a
                    # mistyped enumerator, so it's reported as a note; a name with a space
                    # in it can't be an enumerator, so it's plainly a display name.
                    if item.isupper() and '_' in item:
                        unresolved.append(f'{slug}/{where}: {item}')
                    elif not any(c.isspace() for c in item):
                        by_name.append(f'{slug}/{where}: {item}')

        for group_name, members in groups.items():
            if isinstance(members, list):
                rewrite(members, group_name)

        order = module.get('order')
        if isinstance(order, list):
            rewrite(order, 'order', skip=lambda item: item.casefold() in group_names)

    return rewritten, unresolved, by_name


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--in', dest='infile', required=True)
    parser.add_argument('--out', dest='outfile', required=True)
    parser.add_argument('--elf', dest='elf', required=False)
    parser.add_argument('--best-effort', action='store_true',
                        help='Leave enumerator names alone instead of failing when the ELF '
                             'is missing or has no usable DWARF (the simulator builds plugins '
                             'natively, so there may be no ELF to read)')
    parser.add_argument('-v', '--verbose', action='store_true')
    args = parser.parse_args()

    try:
        with open(args.infile) as f:
            mm_json = json.load(f)
    except json.JSONDecodeError as e:
        # The firmware's yaml parser is more forgiving than python's json module
        # (trailing commas, for one). Leave a file we can't parse exactly as it is
        # rather than failing a build over a manifest we may have nothing to do to.
        print(f'Note: {args.infile} is not strict JSON ({e}); element groups left as written')
        return 0

    class_names = {
        m['class'] for m in mm_json.get('MetaModuleIncludedModules', [])
        if isinstance(m, dict) and m.get('class') and (m.get('groups') or m.get('order'))
    }
    needs_enums = bool(class_names)

    enums = {}
    if needs_enums:
        problem = None
        if not args.elf:
            problem = 'a module names a `class` for its element groups, but no ELF was given'
        else:
            try:
                enums = collect_enums(args.elf, class_names)
            except Exception as e:
                problem = f'cannot read DWARF from {args.elf}: {e}'
            if not problem and not enums:
                problem = (f'no usable enums found in {args.elf} -- built without -g, '
                           f'or the enum is never used in the code, so the compiler dropped it')

        if problem:
            if not args.best_effort:
                print(f'**** Error: {problem}')
                return 1
            print(f'Note: element group enumerator names left unresolved: {problem}')
            enums = {}

    rewritten, unresolved, by_name = resolve(mm_json, enums, args.verbose)

    for item in by_name:
        print(f'Note: not an enumerator, matched by name instead: {item}')

    for item in unresolved:
        print(f'**** Error: not an enumerator of its module class: {item}')
    if unresolved:
        return 1

    # Nothing to rewrite: leave the destination alone rather than reformatting a file
    # we didn't change. When --in and --out are the same path this matters even more.
    if not rewritten:
        if args.infile != args.outfile:
            shutil.copyfile(args.infile, args.outfile)
        return 0

    with open(args.outfile, 'w') as f:
        json.dump(mm_json, f, indent=2)
        f.write('\n')

    print(f'Resolved {rewritten} element group member(s) to typed indices')

    return 0


if __name__ == '__main__':
    sys.exit(main())
