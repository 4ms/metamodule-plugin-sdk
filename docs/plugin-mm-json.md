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
      "name": "Module Number One",
      "displayName": "Alias for Module1"
    },
    {
      "slug": "Module2",
      "name": "Module Number Two"
    }
  ]
}
```

All fields all are optional, but some are required for being visible on the
plugins website. Order does not matter:

- `MetaModuleBrandName`: This is the display name for your plugin. When adding
  a new module to a patch, modules are sorted by brand names. This field sets
  how the brand name will be shown. If not present, then the `name` field from 
  the `plugin.json` file will be used (if that file is present), otherwise the 
  plugin file name stem will be used (e.g. `CoolModular-v1.2.mmplugin` =>
  `CoolModular`). Required for website listing.

- `MetaModuleBrandSlug`: This is the brand slug that uniquely identifies your
  plugin brand. The brand slug is used when opening a patch file to determine
  which plugins to find the modules. It's also used when saving a patch file.
  If present, this must match the brand slug used in patch files. For VCV
  Rack ports this must be the same as the VCV Rack slug. Putting a value here
  will override the `slug` value found in the `plugin.json` file, and if that's
  not found then the brand slug will default to the file name stem (as
  described above). For VCV Rack ports, this is usually not populated. Required for 
  native plugins.

- `MetaModuleBrandAliases`: This is a list of slugs that should be considered
  as the same plugin when opening a patch file. In the example json file above,
  there are three aliases: "4ms-company", "4MS" and "4msVCV". If we open a
  patch file that asks for a module named "4msVCV:SuperVCO" then the MetaModule
  will first look for a plugin called "4msVCV", and if it doesn' find that then it
  will scan the aliases of all plugins until it finds one that has "4msVCV" for an alias.
  If two plugins happen to have the same alias, then either one might get used
  depending on the order that the plugins were loaded. So make sure to choose
  aliases that will definitely be used only by your brand.

- `MetaModulePluginMaintainer`
- `MetaModulePluginMaintainerEmail`
- `MetaModulePluginMaintainerUrl`
- `MetaModuleDescription`: These four fields are used for the plugin website at https://metamodule.info
   At least one method of contact is required for website listing.

- `MetaModuleIncludedModules`: This is a list of the modules in the MetaModule
  plugin (as opposed to the modules in the VCV plugin). 
  Each entry should contain "slug" and "name". Optionally, each entry can have
  the "displayName" field.
  If this is a ported plugin, the list of modules might be different than what's
  included in the original plugin, so remove any modules that aren't present in
  the MetaModule version. Required for website listing.
      - `slug`: This must match the slug used by patch files (i.e. must match the slug in the VCV plugin.json file)
      - `name`: This is a short name of the module to be displayed on the website.
      - `displayName`: This is a short name to be displayed on the MetaModule hardware.


