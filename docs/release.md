# Releaseing a MetaModule Plugin

Now that you have a working plugin, it's time to share it with the world!


### Licensing

According to the license, you are free to release your plugin under any license
you wish, including commercial licensing. Your choice of license needs to
comply with the license of any code that you used.

## Listing on the official MetaModule plugins page

The official MetaModule plugin page is at [https://metamodule.info/plugins](https://metamodule.info/plugins)

We want all plugins that are available on this page, so we encourage you to
list your plugin here if you want others to use it.

Whether your code is open source or closed source does not effect how the
plugin is listed on the MetaModule plugin page. But whether the plugin is free
to download, or if you require payment matters.

Plugins that can be freely downloaded must meet these requirements:

- The .mmplugin file is hosted on a github repo Releases page (we use this to
  scan for updates and manage versions)

- The Releases are tagged and the tag contains a proper sem-ver version string,
  e.g. "MyPlugin-v1.2.3" (see below)

If you do not wish to host on github, email us and we can discuss alternatives.

Plugins that are pay-to-download must send us at least the following:

- A link to the plugin product info or checkout page

- The plugin-mm.json file (which lists the names of the included modules)

Regardless of free or pay status, all plugins must meet these requirements:

- The plugin-mm.json file is properly populated with the names of the modules
  included (see below)

- The .mmplugin file name contains the SDK version or contains no version.
  Example: If it's compiled with SDK v1.6, then `Myplugin-v1.6.2.mmplugin` or
  `Myplugin.mmplugin` or OK, but not `Myplugin-v3.4.mmplugin` or
  `Myplugin-v1.5.mmplugin`.

- For plugins that run on the current development firwmare (currently
  v2.0-dev), include correct dev version string in the release version (see
  below)


## Release tag Version string

Release tags must contain a valid version as defined in [Semantic Versioning v2.0.0](https://semver.org).

The version is used so that our scripts only show the latest plugin versions
for each firmware version on our site.

The release tag version does not need to match the SDK version or MetaModule
firwmare versions -- you can use whatever versioning you wish (Keep in mind
this only applies to the version in the release tag -- you still must follow
the rule above regarding putting a version in the .mmplugin file name).

The scripts we use to display plugins on our website (and to generate the
plugin zip files for download) will scan the releases for the latest version
that will run on the official firmware, and also for the latest plugin version
that will run on the current dev firmware release.

If the version major number is 0 (e.g. `v0.7`), or if the string `dev` appears
after the version (e.g. `v1.6.2-beta-dev) then the plugin will only be
considered for the dev plugin releases. That is, releases with v0.x or that
contain `dev` will not appear on the main plugin page. See below for more details.

If you have multiple plugins (that is, multiple brands hosted in the same
repo), then the script will make a separate pass for each plugin brand (so each
brand has its own "latest official firmware version" and "latest dev firmware version").

It's OK if a plugin is only available for the official firwmare or if it's only 
available for the dev firmware.


## Dev Firmware Plugins

Before a major firmware version is officially released, we will make frequent
development releases of the `dev` firmware version. This firmware is unstable
and can be thought of as an alpha or beta firwmare version. At the time of this
being written, v1.6.9 is the official firmware and v2.0-dev-12 is the dev
version.

On the MetaModule downloads page, there is a zip file with all the known
plugins that run on the latest dev firmware (e.g. `v2.0-dev-12`). In order to
release your plugin in that zip file, you need to follow all the same rules
above, except also add the string `-dev-X` to the plugin version, where X is
the major version of the dev firmware. Notice there is no `v` before the `X`.
Currently, this means plugins meant to run on the dev firwmare must contain
`-dev-12` after the version.

For example, all of these will get put into the dev plugin zip file if dev-12 is the latest version
- `v0.8-dev-12`  
- `v0.8-dev-12.0`  
- `v0.8-dev-12.1.2`  
- `v2.0-dev-12`  
- `v2.0-dev-12.0`  
- `v9.99-dev-12`
- `v0.1-dev-12.9.99-beta`
- `MyPluginMetaModule-v2.0-dev-12.1.1-beta-alpha-1`

The key is that all of these contain `dev-12` so therefore will be packaged in
the dev plugins zip file (assuming that dev-12 is the current latest dev
firwmare version).

None of these will be considered for the dev plugin zip file for firmare dev-12:
- `v2.0-dev-11` -- The latest firmware is dev-12, not dev-11
- `v2.0-dev-v12 -- Cannot contain a `v` before the 12


## Plugin-mm.json file

This json file will get packaged into the plugin by the SDK cmake script. 
At minimum, it must contain these fields:

```json
{
	"MetaModuleBrandName": "BrandName",
	"MetaModulePluginMaintainer": "My Name",
	"MetaModulePluginMaintainerEmail": "myname@example.com",
	"MetaModulePluginMaintainerUrl": "https://maintainers.website/url",
	"MetaModuleDescription": "A brief note describing the plugin (if it's a port of another work, mention that here)",
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


The "MetaModuleBrandName" field is optional, and presently not used. But future
firmware versions will use this to properly display the name of your plugin if
it differs from the VCV Rack slug name, or if you have multiple plugins that
you want "merged" into one brand name. So if either of those situations apply, 
then populate this field.

The "MetaModulePluginMaintainer" field is the name of the person or
organization responsible for maintaining the MetaModule version of the plugin
(which may be different than the VCV Rack plugin maintainer). Similarly,
"MetaModulePluginMaintainerUrl" and "MetaModulePluginMaintainerEmail" should be
ways to contact the maintainer or find out more information about the
maintainer.

The "MetaModuleIncludedModules" list contains the list of modules that are
included in your plugin. The "slug" field should match the VCV Rack slug name
found in the plugin.json file (asumming the plugin comes from VCV Rack). The
"name" field can be any text you want to display on our site for the plugin
name. So in the above example, our site will list your plugin containing two
modules: "Module Number One" and "Module Number Two"


The file must be valid json and not contain syntax errors (json is very picky).
Consider running it in an online json checker to verify it.



