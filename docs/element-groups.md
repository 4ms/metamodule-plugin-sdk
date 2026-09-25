# Element groups

You can group related elements together to make the Module View element roller
easier for users to navigate. This is completely a GUI feature, it doesn't change
the way params or signals are processed. 

When groups are present, the user sees just the group name at the top level, like `Filter >`.
Clicking it will show all the group's elements, with "< Back" above them. 

Groups are a good way to organize components so that the user can find something quickly.
They're also useful for reducing the amount of scrolling if the module has lots of elements.
Groups are particularly suited for modules with multiple channels or several discrete 
sub-sections. For modules will only a small number of jacks and params (e.g. a
two channel attenuator with just one knob and two jacks per channel), groups
are probably not necessary because they cause the user to have to click one
extra time to reach an element.

Groups are declared in your plugin's `plugin-mm.json`.
They work the same for native (`CoreProcessor`) modules and for modules ported
from VCV Rack.

Elements can only belong to one group, and you can't nest groups inside each other.
Elements that aren't in a group are listed the same as before.

## Declaring groups

Add a `groups` object to a module's entry in `MetaModuleIncludedModules`. Each key
is a group name, and its value is the list of elements in that group:

```json
{
	"MetaModuleBrandName": "My Brand",
	"MetaModuleIncludedModules": [
		{
			"slug": "SimpleVoice",
			"name": "Simple Voice",
			"groups": {
				"Filter": ["Cutoff", "Resonance", "Cutoff CV"],
				"Envelope": ["Attack", "Decay", "Sustain", "Release"]
			}
		}
	]
}
```

A group lists its elements in the order they're written. A group can mix element
types — knobs, switches, and jacks can all be in the same group.
The element can be specified using any of the ways described below (see 
[Naming elements](#naming-elements))

Unless the module has an `order` (below), each group appears at the position of its
first element.

## Ordering

Add an `order` list to set the order of the top level of the list. Each item is a
group's name or an element, written any of the ways described under
[Naming elements](#naming-elements):


```json
{
	"slug": "QCD",
	"class": "QCDInfo",
	"groups": {
		"Channel 1": ["DivMult1Knob", "DivMultCv1Knob", "GatePw1Knob"],
		"Channel 2": ["DivMult2Knob", "DivMultCv2Knob", "GatePw2Knob"]
	},
	"order": ["Tap Tempo", "Tap Out", "Channel 1", "Channel 2"]
}
```

`order` can be partial: anything you don't list appears in its default position.

- A name in `order` is matched against the module's group names first, and then
  against its elements. If a group and an element share a name, the group wins.
- An element that belongs to a group can't be listed at the top level: it's shown
  inside its group, and the `order` entry is skipped with a warning.
- The element list separates element types with headers ("Params:", "Jacks:"). A
  new header appears whenever the type changes, so keep elements of the same type
  together unless you want the headers to repeat.
- While you're patching a cable, the list shows jacks only, in panel order,
  ignoring groups and `order`.

## Naming elements

An element can be specified in one of three ways:

### By name

You can specify the string display name of the element, as it appears on the
MetaModule screen (e.g. `"Cutoff"`). Matching ignores case.

For a module ported from VCV Rack, the name is the one you passed to
`configParam()` / `configInput()` / `configOutput()`. MetaModule appends " In" or
" Out" to a jack's name when it doesn't already contain that word, so a jack you
configured as "Cutoff CV" shows as "Cutoff CV In". Either way matches, so
write whichever reads better.

An exact match always wins over one that needed the " In"/" Out" appended. So if
a module has both a "Pitch" knob and a "Pitch" input (shown as "Pitch In"),
`"Pitch"` finds the knob and `"Pitch In"` finds the jack.

### By enum name

You can specify the name of an enum of the module's class, like `"CUTOFF_PARAM"`. 
For some modules, the string display name is long or ambigious, so this method
might be a good choice.

This way is compile-time checked, and you'll see typos and errors immediately, so
that's a big help if you have a lot of modules. It's fragile, however, and it won't work in 
some cases. So if it doesn't work, use the display name method.

If you use this method, then you must also include the `class` field like this:

```json
{
	"slug": "Rad-Mod1",
    "name": "My Rad Module",
	"class": "MyModule",
	"groups": {
		"Filter": ["CUTOFF_PARAM", "RESO_PARAM", "CUTOFF_CV_INPUT"]
	}
}
```

The `class` field's value should be the exact C++ name of the class that **defines**
the enums. Usually that's your Module (the class that derives from `rack::Module`), e.g.:
```
class MyModule1 : rack::Module {  // <<<< "class": "MyModule1"

// or you might see:
class MyModule2 : Module {  // <<<< "class": "MyModule2"
```

But if the enums are defined in a different class than the one that uses them, such
as a base class shared by several modules, then `class` must name the class that
defines them:
```
struct MixModule : Module {       // <<<< enums are defined here, so use "class": "MixModule"
	enum FadeParamId { FADE_TIME_PARAM, ... };
};
struct MixFade : MixModule { ... };  // <<<< using "class": "MixFade" will not work
```
Just use the class's actual name, without any namespace: for `bogaudio::Mix4`, write `"class": "Mix4"`.

If the enums come from different classes, then you'll need to use one of the other two methods.

The enums are resolved when the plugin is built and packaged by the SDK.
A script reads the plugin's debug info, and converts the enum names to
the integer value of the enum.
If an enum name can't be resolved, it's a **build error**, which makes 
this the most robust method: unlike using the display name, you'll know right away
if you made a typo.

The enum determines what it refers to:

| enum | refers to |
|---|---|
| `ParamIds` / `ParamId` | a param |
| `InputIds` / `InputId` | an input jack |
| `OutputIds` / `OutputId` | an output jack |
| `LightIds` / `LightId` | a light |
| `Elem` | an element of a native module's info struct |

The enum's name only has to *end* with one of these, so prefixed names such as
`FadeParamId` or `ExpLightIds` work too. So do plural-first spellings such as
`ParamsIds` or `InputsIds`. `Elem` must be matched exactly.


Two things to keep in mind: 
  - The `class` field is required (as explained above)
  - The enum must actually be *used* somewhere in the module's code or else the
    compiler drops it and so the script won't find it.

### By index

The final way is not recommended unless the other two methods won't work.
You can specify by a typed index: `"param:3"`, `"in:1"`, `"out:0"`, `"light:2"`, or `"elem:5"`.

`param:`/`in:`/`out:`/`light:` are the ids the module uses at runtime, that is,
the integer values of the `ParamIds`/`InputIds`/`OutputIds`/`LightIds` enum members.

`elem:` is an index into a native info struct's `Elements` array.

This is what the other two methods resolve to, and it's the fallback method if the other
methods don't work (duplicate display names, no enum classes, etc.). Generally,
you won't ever need to use this, which is a good thing because it's not very legible.

## Using groups in the simulator with external plugins

When simulating a plugin as an external built-in (that is, using `ext-plugin.cmake`
with the simulator), the enum method of groups is not supported. Use either the display 
name, or the typed index (`param:3`), or just test groups on hardware.

## Built-in brands
If you're using the SDK then you're NOT making a built-in brand, so this section is 
for curious minds only and is only tangentially relevant.

Brands built into the firmware read the same `groups` key from their
`firmware/assets/<brand>/plugin-mm.json`. Enumerator names work there: the firmware build 
resolves them against its own debug info, the same way the SDK resolves them
for a plugin. Names and typed indices work as usual.

When building firmware, the arm-none-eabi-gcc toolchain is used, which
generates the debug info used to scan and resolve enum names. On the other
hand, the simulator is built by your computer's native toolchain and so its
binaries cannot reliably be scanned to resolve the enums. So, in order to see
groups specified by enum names in built-in brands, tell the simulator to use
the firmware's assets image, not the simulator's assets: `./build/simulator -s
../firmware/build/assets.uimg`.

## Details

- An element listed in more than one group stays in the first group that used it.
- A member that doesn't resolve (e.g. typo in the display name) is dropped and
  there is a a warning printed on the console.
- A group with no name or no valid members is dropped.
- Names and indices are resolved the first time a module's element list is shown,
  not when the plugin loads, because a VCV-ported module's element names and
  indices aren't final until the module has been created.
