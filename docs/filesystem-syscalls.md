# Filesystem Calls

The MetaModule uses newlib for libc. This provides standard library filesystem functions like `fopen`, `fread`, `fclose`.

You can use these exactly as they are used on a desktop computer. However, there are some things to keep in mind:

- Paths are different. Paths start with a the volume name (3 characters), then a colon, then a slash. 
  The volumes available on the MetaModule are:
   - `sdc:/` -- The SD Card drive.
   - `usb:/` -- The USB drive.
   - `ram:/` -- The internal RAM disk. This is where plugins live, as well as
     unsaved patches. Everything in here is temporary. You could create temp
     files here, but it's usually a better idea to just store temp data in
     memory as normal C/C++ variables. If you need shared global state, a small
     file could work (like a lock file used in Linux systems).
   - `nor:/` -- NOR Flash (internal storage). There is a ~2MB volume on this
     flash chip that can hold patch files and other data. The filesystem is
     LFS, but you don't need to consider that when using the drive. It's highly
     recommended to avoid using this drive except for very tiny files (and not
     a lot of files). Users will want to save their patches here, so do not
     fill this up with module or patch data.


- Users might eject a disk at any time. Always check for errors when doing any
  file system access and handle them appropriately.

- Filesystem access should never be done from the audio context.

- Disk access is slow. The MetaModule was not designed to read/write to disks
  quickly, like a desktop computer would be designed. Also, SD Cards are by
  nature slow to read and write. Users might be using a slow or heavily
  fragmented disk. USB drives vary widely in performance. Multiple modules may
  be requesting disk access at the same time as your module. All these factors
  mean it's not surprising if a read request occasionally takes up to 4 or 5
  seconds even for just a few kB of data. You must plan for stalls and delays.
  If you have large files, buffering data to/from RAM is a good idea.

