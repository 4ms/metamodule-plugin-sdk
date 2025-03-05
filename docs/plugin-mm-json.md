## plugin-mm.json 

This file contains MetaModule-specific information used when cataloging the plugin.
The fields in this file refer to the MetaModule plugin, which may be different
than the maintainer of the main repo (e.g. different maintainers, different
list of modules).

Currently this metadata is only used to display and catalog plugins on the website, 
but future firmware will parse the json files, so make sure the information is accurate.

If you have no intention of listing this on the MetaModule website, or just
want to get started quickly, you can just use a blank file for now.

Here is an example: 

`MyPlugin/plugin-mm.json`:
```json
{
	"MetaModulePluginMaintainer": "My Name",
	"MetaModulePluginMaintainerEmail": "",
	"MetaModulePluginMaintainerUrl": "",
	"MetaModuleDescription": "",
	"MetaModuleIncludedModules": [
	{
		"slug": "Module1",
		"name": "Module Number One"
	},
	{
		"slug": "Module2",
		"name": "Module Number Two"
	}
}
```

