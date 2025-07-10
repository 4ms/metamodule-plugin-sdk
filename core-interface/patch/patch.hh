#pragma once

namespace MetaModule
{

// Indicates the currently playing patch has been modified. A red
// dot will appear above the File icon and the user can save the patch file.
// This is only necessary if the internal state changes in a way users would
// expect to be able to recall later (e.g. the user loaded a sample file)
//
// You do not need to call this if a normal parameter or AltParam changed
// (e.g. a button was pressed).
//
// Note that if a user is updating the patch via Wifi from a computer, calling
// this function will prevent further updates until the user Reverts the file.
// So use this only if you're sure the user will want the patch to be marked
// as modified.
void mark_patch_modified();

} // namespace MetaModule
