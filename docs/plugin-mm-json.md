## plugin-mm.json 

This file contains MetaModule-specific information used when cataloging the plugin.

If this plugin was ported from another system (e.g. VCV Rack) then populate these fields
with what makes sense for the MetaModule port of the plugin. For example, the maintainer
of the VCV Rack version and this version might differ -- or the list of modules
might be different.

This metadata is used to display and catalog plugins on the website, and also to
know how to display and load the modules on the MetaModule hardware.

Here is an example: 

`MyPlugin/plugin-mm.json`:
```json
{
  "MetaModuleBrandName": "4ms",
  "MetaModulePluginMaintainer": "My Name",
  "MetaModulePluginMaintainerEmail": "me@example.com",
  "MetaModulePluginMaintainerUrl": "https://example.com/modules",
  "MetaModuleDescription": "A brief description of the plugin. If ported from another work, mention that here.",
  "MetaModuleIncludedModules":
  [
    {
      "slug": "Module1",
      "name": "Module Number One",
      "displayName": "Alias for Module1"
    },
    {
      "slug": "Module2",
      "name": "Module Number Two"
    },
    {
      "slug": "DualLFO",
      "name": "Dual LFO",
      "groups": {
        "LFO 1": ["Rate 1", "Wave 1", "Out 1"],
        "LFO 2": ["Rate 2", "Wave 2", "Out 2"],
      },
      "order": ["LFO 1", "LFO 2", "Mix Out"]
    }
  ]
}
```

All fields all are optional, but some are recommnded, and strictly required for
being visible on the plugins website.

#### Recommended fields (required for website listing):

- `MetaModuleBrandName`: This is the display name for your plugin. When adding
  a new module to a patch, modules are sorted by brand names. This field sets
  how the brand name will be shown. If not present, then the `name` field from 
  the `plugin.json` file will be used (if that file is present), otherwise the 
  plugin file name stem will be used (e.g. `CoolModular-v1.2.mmplugin` =>
  `CoolModular`). Required for website listing.

- `MetaModulePluginMaintainer`
- `MetaModulePluginMaintainerEmail`
- `MetaModulePluginMaintainerUrl`: These four fields are used for the plugin website at https://metamodule.info
   At least one method of contact is required for website listing.

- `MetaModuleDescription`: This is a short bit of text (1-2 lines) describing the plugin.  On
  the plugin website, it's the blurb of text you see right after the plugin name.
  Required for website listing.

- `MetaModuleIncludedModules`: This is a list of the modules in the MetaModule
  plugin (as opposed to the modules in the VCV plugin). 
  Each entry should contain `slug` and `name`. Optionally, each entry can have
  the `displayName` and `groups` and `order` fields
  If this is a ported plugin, the list of modules might be different than what's
  included in the original plugin, so remove any modules that aren't present in
  the MetaModule version. Required for website listing.
      - `slug`: This must match the slug used by patch files (**must** match the slug in the VCV plugin.json file)
      - `name`: This is a short name of the module to be displayed on the website.
      - `displayName`: This is a short name to be displayed on the MetaModule hardware.
      - `groups`: Group together jacks/params of the module so that they'll be displayed
        together. See [Element Groups](element-groups.md)
      - `order`: Specify the order of jacks and params. See [Element Groups](element-groups.md)

#### Optional fields (not recommended unless necessary):
- `MetaModuleBrandSlug`: **If your plugin also has a VCV Rack version, then 
  DO NOT USE THIS FIELD.** For native plugins, this is where you set your
  brand slug, and it's required. For plugins that have a VCV equivalent, just omit
  this field.
  *Note: If you populate this field for a non-native plugins, then we will ask
  for an explanation before releasing it because it can cause confusion and
  incompatibility with VCV Rack if used improperly.* 
  Usage: `"MetaModuleBrandSlug": "4msCompany"`

- `MetaModuleBrandAliases`: **This is a rarely used field, and should not be used
  unless you have a specific reason** (usually involving some legacy plugin
  with a naming or licensing issue). If you're not sure, just leave this field
  out of your `plugin-mm.json`. If you do include it, we will ask for an
  explanation because alias names can conflict with each other if multiple
  plugins use the same alias.

  Usage: `"MetaModuleBrandAliases": ["4ms-company", "4MS", "4msVCV"]`

  This field is a list of slugs that should be considered as
  the same plugin when opening a patch file. In the json above, there are three
  aliases: "4ms-company", "4MS" and "4msVCV". If we open a patch file that asks
  for a module named "4msVCV:SuperVCO" then the MetaModule will first look for
  a plugin called "4msVCV", and if it doesn' find that then it will scan the
  aliases of all plugins until it finds one that has "4msVCV" for an alias. If
  two plugins happen to have the same alias, then either one might get used
  depending on the order that the plugins were loaded. This can be a problem,
  so don't use aliases if you don't have to.


