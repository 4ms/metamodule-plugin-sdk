# Releasing a MetaModule Plugin

Now that you have a working plugin, it's time to share it with the world!

## Licensing and Permissions

Before you consider releasing your plugin, make sure you are 100% in
compliance with all licensing requirements and have obtained permission
from any original author(s) of the code (e.g. if you are porting a plugin
that you did not originally create).

You must read and follow all guidance here: [Licensing and Permissions](licensing_permissions.md).


## Listing on the official MetaModule plugins page

The official MetaModule plugin page is at [https://metamodule.info/plugins](https://metamodule.info/plugins)

We want this page to list all plugins that are available for the MetaModule
platform, so we encourage you to list your plugin here if you want others to
use it.

### Requirements

All plugins listed on our site must meet these requirements:

- The .mmplugin file name is properly written (see below)

- The plugin-mm.json file is properly populated with the maintainer's name and
  the names of the modules included (see below)

- For ports from VCV Rack or other platforms: You are the original author of
  the code, or the original author of the work has given you permission to port
  their work to the MetaModule.

Whether your code is open source or closed source does not effect how the
plugin is listed on the MetaModule plugin page. But it does matter whether the
plugin is free to download, or if you require payment.

### Free-to-download plugins:

Plugins that can be freely downloaded must meet these requirements:

- The .mmplugin file is hosted on a GitHub repo Releases page (we use this to
  scan for updates and manage versions)

- The GitHub release is NOT marked as "pre-release" (that is, we will ignore
  pre-releases)

If you do not wish to host on GitHub, email us and we can discuss alternatives
such as GitLab, hosting on our site, or static hosting.

Maintainers of free-to-download plugins just need to send us the URL to their
GitHub repo that contains the releases. Email us at 4ms@4mscompany.com or send
a message on the MetaModule forum.

### Pay-to-download plugins

Plugins that are pay-to-download need to host their own downloads (or have them 
on a third-party site) and manage payment and downloads. In this case, the
maintainers must send us at least the following:

- A link to the plugin product info or checkout page

- The compiled .mmplugin file (or at least the plugin-mm.json and SDK version)


## .mmplugin file name

The .mmplugin file name is important! It's used in three places:
- The script that scans releases for putting them on the plugin page on our website
- The MetaModule firmware plugin loader (Settings > Plugin tab, and the preloader)
- Users who want to keep all their plugins organized

The file name must be in this format:

```
[BrandSlug]-v[plugin-version].mmplugin
```


Optionally, you can include the SDK version, which is only required if you're building
for a development (e.g., beta or pre-release) firmware.
```
[BrandSlug]-v[plugin-version]-[sdk-version].mmplugin
```

- `BrandSlug`: the file must start with the brand slug. This is the unique
  identifier for your plugin brand. For VCV-ported plugins, this is the `slug:`
  field in the `plugin.json` file.

- `plugin-version`. This is the version and revision of your plugin code. It
  must be in the form `X.Y` or `X.Y.Z` where X, Y and Z are integers. You also
  can have suffixes separated by dashes like `X.Y.Z-beta-2.1`. Just don't use 
  the reserved words `-fw` or `-dev` (see next item). For legacy reasons, this
  field is technically optional but we will ask you to add it if missing.
  Notice that there is a `v` before the first number.
  Since the official plugins website only lists the latest versions, this field 
  is used to determine what the latest version is. We follow the SemVer 2.0 standard.

- `sdk-version`. This is the SDK version used to build your plugin, which also
  tells users about the firmware version needed to load the plugin.
    - This field is mandatory if the plugin only runs on development firmware.
      In this case the field must be in the form `dev-Z`, where `Z` is the
      minimum development firmware version required.
    - This field is optional if the plugin runs on the currently 
      released firmware. If it's omitted then the website and the MM firmware
      will scan the contents to determine if it's compatible with the current
      firmware. If this field is present then it must be in the form `fw-X.Y`,
      e.g. `fw-2.0` indicates it requires firmware v2.0 or later.
        - Note: if you recompile using a new SDK, make sure to bump the version up.
          Otherwise it'll be unclear to users which version is for which firmware.

- optional text after the `sdk-version`, but before the `.mmplugin` is OK.

Example: If it's compiled with SDK v2.0, then these are all OK:
  - `Myplugin-v0.9.mmplugin` 
  - `Myplugin-v3.1-hotfix99.mmplugin` 
  - `Myplugin-v0.9-fw-2.0.mmplugin` 
  - `Myplugin-v1.2.3-fw-2.0.1.mmplugin`
  - `Myplugin-v1.2.3-fw-2.0-test.mmplugin`

Example: If it's compiled with SDK v2.1, then these are all OK:
  - `Myplugin-v0.9.mmplugin` 
  - `Myplugin-v1.2.3-testing.mmplugin`
  - `Myplugin-v0.9-fw-2.1.mmplugin` 
  - `Myplugin-v3.2.1-fw-2.1.mmplugin`
  - `Myplugin-v1.2.3-testing.mmplugin`

Example: If it's compiled with SDK v2.0-dev-13, then these are all OK:
  - `Myplugin-v0.9.mmplugin` 
  - `Myplugin-v0.9-newideas.mmplugin` 
  - `Myplugin-v0.9-dev-13.mmplugin` 
  - `Myplugin-v14.2-dev-13.1.mmplugin`
  - `Myplugin-v1.2.3-dev-13.6.mmplugin`

Example: This is never OK for a release (only OK for testing locally):
  - `Myplugin.mmplugin`

Notice `Myplugin-v0.9.mmplugin` could be valid for any version firmware.
This ambiguity will not happen in actual circumstances because when re-compiling
with a newer SDK you should bump up the plugin version (or use the -fw-X.Y tag).
The website and MM firmware will scan the contents of the plugin file
to determine the SDK version, so no special considerations need to be made with
regards to those.


## plugin-mm.json file

This json file will get packaged into the plugin by the SDK cmake script. 
At minimum, to be listed on the website, it must contain these fields:

```json
{
	"MetaModuleBrandName": "Display Name for Plugin",
	"MetaModulePluginMaintainer": "My Name",
	"MetaModulePluginMaintainerEmail": "myname@example.com",
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
    ]
}
```

See the [plugin-mm.json](plugin-mm-json.md) docs for descriptions of the fields, and more fields you can add.

The file must be valid json and not contain syntax errors (json is very picky).
Consider running it in an online json checker to verify it.



## Publishing by manually creating GitHub releases

This is a simple way to publish a release, but it takes some steps to do each time
you make a release. See the next section if you want to set up an automatic release system.

In a GitHub repo, go to the Releases tab and click the button to draft a new release.

You need to create a new tag: click Choose a tag and then type a tag name in the box. Click "Create new tag (..your tag..) on publish"
Any tag would work, but a good practice is to use the version for your tag. For instance, "v0.8" if your plugin is currently v0.8.
You also can put the sdk version in the tag if you intend to make a separate tag for each SDK version (e.g. "v0.8-fw-1.6" and "v0.8-dev-12.2")

Next, type in a title and description for your release.

Then, upload your .mmplugin file(s) to the release with the "Attach binaries here" box.

The .mmplugin files must be named properly, see the section above.

Make sure "Set this as pre-release" is not checked, or else we won't be able to add it to our site.
If for some reason you want to hide a release from our website, checking this box will do that.

Click "Publish Release" and send us a message to scan for your new release!


## Publishing with a GitHub Workflow

This is optional, and takes some time to set up, but it pays off by making 
it easy to generate releases. It also makes builds reproducible which
eliminates a whole class of strange issues.

Use this file as a template and save it as a .yml file in your
.github/workflows/ dir (create that dir if it doesn't exist), for example
`.github/workflows/build-metamodule-plugin.yml`. Then commit and push this file
to github. 



```yml
name: Build and release MetaModule plugin

on:
  workflow_dispatch:
    inputs:
      SDK_branch:
        description: 'MetaModule SDK branch'
        required: true
        type: choice
        default: main
        options:
          - main
          # You can add more options here if you need to test on different SDK versions
      do_release:
        description: 'Create Release (must also select Tag above!")'
        required: true
        default: false
        type: boolean

jobs:
  build-lin:
    strategy:
        matrix:
          gcc: ['12.3.Rel1']
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
          # Create directory for the plugin to be created in:
          mkdir -p ${{ github.workspace }}/metamodule-plugins
          # Get the SDK:
          git clone -b ${{ inputs.SDK_branch }} https://github.com/4ms/metamodule-plugin-sdk --recursive ${{ github.workspace }}/metamodule-plugin-sdk

          # Build the plugin:
          cmake -B build -G Ninja -DMETAMODULE_SDK_DIR=${{ github.workspace }}/metamodule-plugin-sdk
          cmake --build build
          # Add version tag to the plugin file name:
          cd ${{ github.workspace }}/metamodule-plugins && for f in *.mmplugin; do mv $f ${f%.mmplugin}-${{ env.CI_REF_NAME }}.mmplugin; done;

      - name: Release
        if: startsWith(github.ref, 'refs/tags/') && inputs.do_release
        uses: softprops/action-gh-release@v2
        with:
          name: "Release: ${{ env.CI_REF_NAME }}"
          files: |
            metamodule-plugins/*.mmplugin
```


### Enabling the workflow permissions
Go to your repo's GitHub page and make sure you see the "Build and release
MetaModule plugin" action in the Actions tab. 

If not, check your settings to see if Actions or workflows is enabled: In your
repo Settings, click Actions > General in the left side bar. Then scroll down
to "Workflow permissions" and enable "Read and write permissions"

![Workflow permissions](settings-actions-general-permissions.png)


### Running the workflow to create a release

To make a release, you first need to push a git tag that contains the plugin
version that you want to release. For example, if you are releasing v1.0 of your
plugin, then type these commands:

```
git tag -a v1.0 -m "First release, yay!"
git push origin v1.0
```

The version in the tag you create will get appended to the name of the plugin file.
In the above, the plugin will be named `Plugin-v1.0.mmplugin`. Make sure your tag
follows the rules for naming plugins.

Next, go to the Actions tab on the GitHub site for your repo and click
on the "Build and release plugin" action. On the right, you can then select
"Run workflow":

![Run workflow](run-workflow.png)

From this menu, you will need to pick the tag you just pushed. This is
important: you cannot pick a branch; you have to pick a tag. GitHub does not
allow releases from branches, only from tags. 

To pick the tag, first click on "Use workflow from:" and then in the pop-up
click "Tags". Then click the tag you just created.

If you don't see the tag you just pushed, make sure you really pushed it and
refresh your browser. You can go ahead and run the workflow without a tag, but
you cannot generate a release without a tag.

Check the "Create Release" checkbox (unless you just want to test building).

After it's done building, you will see the release on the Releases page of your
github repo (assuming you chose a tag -- not a branch -- and you clicked
"Create Release").

Now, download the release file and make sure it works as intended. 
If so, send us a message to add your release.

### Customizing the workflow

You may need to customize this script a bit so that it builds your plugin.
For example, if you structured your plugin files so that the MetaModule plugin
is built from the `metamodule/` directory, then add `cd metamodule/` before the
other build commands:

```yaml
      - name: Build
        run: |
          # Create directory for the plugin to be created in:
          mkdir -p ${{ github.workspace }}/metamodule-plugins
          # Get the SDK:
          git clone -b ${{ inputs.SDK_branch }} https://github.com/4ms/metamodule-plugin-sdk --recursive ${{ github.workspace }}/metamodule-plugin-sdk
          # Build the plugin:
          cd metamodule  # <<< This line added to build from the metamodule/ directory.
                         # The rest of the script is the same...
          cmake -B build -G Ninja -DMETAMODULE_SDK_DIR=${{ github.workspace }}/metamodule-plugin-sdk
```


Or, perhaps your repository contains the SDK already as a submodule. In that case,
you don't want the workflow script to download the SDK. Instead, do this:

```yaml
      - name: Build
        run: |
          # Create directory for the plugin to be created in:
          mkdir -p ${{ github.workspace }}/metamodule-plugins
          # Omit the line that gets the SDK.
          # Build the plugin:
          cmake -B build -G Ninja -DMETAMODULE_SDK_DIR=${{ github.workspace }}/metamodule-plugin-sdk
          # You may need to adjust the path to the SDK ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
          # The rest of the script is the same...
```


If you need different options for the SDK version, you can modify this script
to pick a different branch or tag. 

Another improvement is to have the workflow automatically run when you push a
tag. You can also can run the workflow from the command line using the `gh`
program (that's [how we build our
releases](https://github.com/4ms/metamodule-plugin-examples/blob/main/release.sh)).

GitHub has extensive docs on workflows, and there are tons of examples
online.
