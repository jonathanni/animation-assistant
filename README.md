# Animation Assistant

Low footprint GUI application for playing image sequences at variable framerates.

## Build

Prerequisite: download the wxWidgets library, and follow its build instructions.

Either grab the Windows binary in the releases or do:

```console
foo@bar:~/AnimationHelper$ cd build
```

Edit `subdir.mk` and set the `WXDIR` variable to point to the wxWidgets base directory. Set the `WXBUILD` variable to the absolute path `lib/<your_build_directory>/<your_platform>`.

Then

```console
foo@bar:~/AnimationHelper/build$ make -f makefile
```

You may add `-j4` to, for example, speed up the build with four cores.
