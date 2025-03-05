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
  "MetaModuleBrandSlug": "4msCompany",
  "MetaModuleBrandAliases": ["4ms-company", "4MS", "4msVCV"],
  "MetaModulePluginMaintainer": "My Name",
  "MetaModulePluginMaintainerEmail": "me@example.com",
  "MetaModulePluginMaintainerUrl": "https://example.com/modules",
  "MetaModuleDescription": "A brief description of the plugin. If ported from another work, mention that here.",
  "MetaModuleIncludedModules":
  [
    {
      "slug": "Module1",
      "name": "Module Number One"
    },
    {
      "slug": "Module2",
      "name": "Module Number Two"
    }
  ]
}
```

Fields (all are optional -- order does not matter):

- `MetaModuleBrandName`: This is the display name for your plugin. When adding
  a new module to a patch, modules are sorted by brand names. This field sets
  how the brand name will be shown. If not present, then the field `name` from 
  the `plugin.json` file will be used (if that file is present), otherwise the 
  plugin file name stem will be used (e.g. `CoolModular-v1.2.mmplugin` =>
  `CoolModular`)

- `MetaModuleBrandSlug`: This is the brand slug that uniquely identifies your
  plugin brand. The brand slug is used when opening a patch file to determine
  which plugins to find the modules. It's also used when saving a patch file.
  If present, this must match the brand slug used in patch files. For VCV
  Rack ports this must be the same as the VCV Rack slug. Putting a value here
  will override the `slug` value found in the `plugin.json` file, and if that's
  not found then the brand slug will default to the file name stem (as
  described above). For VCV Rack ports, this is usually not populated

- `MetaModuleBrandAliases`: This is a list of slugs that should be considered
  as the same plugin when opening a patch file. In the example json file above,
  there are three aliases: "4ms-company", "4MS" and "4msVCV". If we open a
  patch file that asks for a module named "4msVCV:SuperVCO" then the MetaModule
  will search this plugin for the SuperVCO module. Brand aliases that match the
  brand slug of another plugin will be ignored. However, if two plugins happen
  to have the same alias, then either one might get used depending on the order
  that the user or autoloader loaded the plugins. So make sure to choose
  aliases that will definitely be used only by you.

- `MetaModulePluginMaintainer`
- `MetaModulePluginMaintainerEmail`
- `MetaModulePluginMaintainerUrl`
- `MetaModuleDescription`: These four fields are used for the plugin website at https://metamodule.info

- `MetaModuleIncludedModules`: This is a list of modules, each entry containing
  "slug" and "name". These two fields are currently just used to create a link
  to the VCV Rack Library page for this module. Future firmware will use this
  for more things, include filtering by tags, so make sure it's accurate. If
  this is a ported plugin, the list of modules might be different than what's
  included in the original plugin, so remove any modules that aren't present in
  the MetaModule version.




