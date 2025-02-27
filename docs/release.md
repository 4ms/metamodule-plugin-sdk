# Releasing a MetaModule Plugin

Now that you have a working plugin, it's time to share it with the world!


### Licensing

According to the license, you are free to release your plugin under any license
you wish, including commercial licensing. Your choice of license needs to
comply with the license of any code that you used.

## Listing on the official MetaModule plugins page

The official MetaModule plugin page is at [https://metamodule.info/plugins](https://metamodule.info/plugins)

We want this page to list all plugins that are available for the MetaModule
platform, so we encourage you to list your plugin here if you want others to
use it.

Whether your code is open source or closed source does not effect how the
plugin is listed on the MetaModule plugin page. But whether the plugin is free
to download, or if you require payment matters.

Plugins that can be freely downloaded must meet these requirements:

- The .mmplugin file is hosted on a github repo Releases page (we use this to
  scan for updates and manage versions)

- The Releases are tagged and the tag contains a proper version string,
  e.g. "MyPlugin-v1.2.3" (see below)

If you do not wish to host on github, email us and we can discuss alternatives.

Free plugin maintainers just need to send us the URL to their github repo that
contains the releases. Email us at 4ms@4mscompany.com or send a message on the
MetaModule forum.

Plugins that are pay-to-download need to host their own downloads and manage
payment and downloads. In this case, the maintainers must send us at least the
following:

- A link to the plugin product info or checkout page

- The compiled .mmplugin file (or at least the plugin-mm.json and SDK version)

Regardless of free or pay status, all plugins must meet these requirements:

- The plugin-mm.json file is properly populated with the maintainer's name and
  the names of the modules included (see below)

- The .mmplugin file name contains either the SDK version or it contains no
  version. Example: If it's compiled with SDK v1.6, then
  `Myplugin-v1.6.2.mmplugin` or `Myplugin.mmplugin` or OK, but not
  `Myplugin-v3.4.mmplugin` or `Myplugin-v1.5.mmplugin`.

- For plugins that run on the current development firmware (currently
  v2.0-dev-12), include correct dev version string in the release version (as
  of writing, this is `dev-12`, but see below)


## Release Tag Version 

Release tags must contain a valid version as defined in [Semantic Versioning v2.0.0](https://semver.org).

The version is used so that our scripts only show the latest plugin versions
for each firmware version on our site.

The release tag version does not need to match the SDK version or MetaModule
firmware versions -- you can use whatever versioning you wish. Keep in mind
that you still must follow the rule above regarding putting a version in the
.mmplugin file name.

The scripts we use to display plugins on our website (and to generate the
plugin zip files for download) will scan the releases for the latest version
that will run on the official firmware, and also for the latest plugin version
that will run on the current dev firmware release.

If the version major number is 0 (e.g. `v0.7`), or if the string `dev` appears
after the version (e.g. `v1.6.2-beta-dev) then the plugin will only be
considered for the dev plugin releases. That is, releases with v0.x or that
contain `dev` will not appear on the main plugin page. See Dev Firmware section
below for more details.

If you have multiple plugins (that is, multiple brands hosted in the same
repo), then the script will make a separate pass for each plugin brand (so each
brand has its own "latest official firmware version" and "latest dev firmware version").

It's OK if a plugin is only available for the official firmware or if it's only 
available for the dev firmware, or if it's available for both.


## Dev Firmware Plugins

Before a major firmware version is officially released, we will make frequent
development releases of the `dev` firmware version. This firmware is unstable
and can be thought of as an alpha or beta firmware version. At the time of this
being written, v1.6.9 is the official firmware and v2.0-dev-12 is the dev
version.

On the MetaModule downloads page, there is a zip file with all the known
plugins that run on the latest dev firmware (e.g. `v2.0-dev-12`). In order to
release your plugin in that zip file, you need to follow all the same rules
above, except also add the string `-dev-X` to the plugin version, where X is
the major version of the dev firmware. Notice there is no `v` before the `X`.
Currently, this means plugins meant to run on the dev firmware must contain
`-dev-12` after the version.

For example, all of these will get put into the dev plugin zip file if dev-12
is the latest version
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
firmware version).

None of these will be considered for the dev plugin zip file for firmware dev-12:
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
found in the plugin.json file (assuming the plugin comes from VCV Rack). The
"name" field can be any text you want to display on our site for the plugin
name. So in the above example, our site will list your plugin containing two
modules: "Module Number One" and "Module Number Two"


The file must be valid json and not contain syntax errors (json is very picky).
Consider running it in an online json checker to verify it.


## Publishing with a Github Workflow

This is the easiest way to make a release if you do it regularly, though it
needs to be set up in order to work.

Use this file as a template and save it as a .yml file in your
.github/workflows/ dir (create that dir if it doesn't exist), for example
`.github/workflows/build-metamodule-plugin.yml`. Then commit and push this file
to github. Go to your repo's github page and make sure you see
the "Build and release plugin" action in the Actions tab. If not, check your
settings to see if Actions or workflows is enabled.


```yml
name: Build and release plugin

on:
  workflow_dispatch:
    inputs:
      SDK_branch:
        description: 'MetaModule SDK branch'
        required: true
        type: choice
        options:
          - main
          - v2.0-dev
      do_release:
        description: 'Create Release (must also tag!)'
        required: true
        default: false
        type: boolean

jobs:
  build-lin:
    if: ${{ github.event_name == 'push' }}
    strategy:
        matrix:
          gcc: ['12.3.Rel1']  # can add other versions if needed
    name: "Build firmware on linux"
    runs-on: ubuntu-24.04
    steps:
      - name: Install Arm GNU Toolchain (arm-none-eabi-gcc)
        uses: carlosperate/arm-none-eabi-gcc-action@v1
        with:
          release: ${{ matrix.gcc }}

      - name: Install cmake
        uses: jwlawson/actions-setup-cmake@v1.13
        with:
          cmake-version: '3.26.x'

      - name: Git setup
        run: git config --global --add safe.directory '*'

      - name: Checkout
        uses: actions/checkout@v4
        with:
          submodules: 'recursive'

      - name: Install linux dependencies
        run: |
          sudo apt-get update
          sudo apt-get install ninja-build

      - name: Set Release Version
        if: startsWith(github.ref, 'refs/tags/')
        uses: FranzDiebold/github-env-vars-action@v2

      - name: Build
        run: |
          mkdir -p metamodule-plugins
          git clone -b ${{ inputs.SDK_branch }} https://github.com/4ms/metamodule-plugin-sdk --recursive 
          cmake -B build -G Ninja -DMETAMODULE_SDK_DIR=metamodule-plugin-sdk -DINSTALL_DIR=metamodule-plugins
          cmake --build build
          cd metamodule-plugins
          for f in *.mmplugin; do mv $f ${{ env.CI_REF_NAME }}.mmplugin; done;

      - name: Release
        if: startsWith(github.ref, 'refs/tags/') && ${{ inputs.do_release }}
        uses: softprops/action-gh-release@v1
        with:
          name: "Release: ${{ env.CI_REF_NAME }}"
          files: |
            metamodule-plugins/*.mmplugin
```

To make a release, you need to push a tag that meets all the requirements listed in the above sections. For example:

```
git tag -a MyPlugin-v0.1-dev-12 -m "First release for v2.0 firmware, yay!"
git push origin MyPlugin-v0.1-dev-12
```

Then you can go to the Actions tab on the github site for your repo and click
on the "Build and release plugin" action. On the right, you can then select
"Run workflow". From this menu, you will need to pick the tag you just pushed.

Then pick the SDK version you want to build with. Check the "Create release"
checkbox (unless you just want to test building).

After it's done building, you will see the release on the Releases tab.

If you need different options for the SDK version, you can modify this script
to pick a different branch or tag. Or, a more flexible approach is to include
the SDK as a submodule (maybe your repo already does this?), in which case
there's no need for the SDK branch selection, so you'll want to remove that
from the workflow script.

Another improvement is to have the workflow automatically run when you push a
tag. Github has extensive docs on workflows, and there are tons of examples
online.

