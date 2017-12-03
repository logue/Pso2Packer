# repacker_ice

ver. 1.3.1

This file was translated by Aida Enna. Thank you very much.

## Extracting an ICE file (.ice)

When you drag and drop an ICE file onto ice.exe,
a folder containing the extracted files will be created in the same directory as the exe.
Dragging and dropping also supports multiple files.

## Creating an ICE files (.ice)

If you drag and drop a file onto `Pack_compressed.bat` or `Pack.bat`,
an ICE file will be created.
Dragging and dropping also supports multiple files.

## How to modify/create the list of files in an ICE file (.ice)

When you drag and drop an ICE file onto the "filelist.bat",
it will create a text file, listing the contents.
Dragging and dropping also supports multiple files.

## How to replace files stored in the ICE file (.ice)

Run the following commands via the command prompt.
If there is a file with the same name in the archive, only it will be replaced.
When replacing, it will be saved in it's original group.

```bat
ice.exe -r "File path of file you want to replace with" "File path to the ICE File"
```

To compress the archive, add the -c option.

```bat
ice.exe -c -r "File path of file you want to replace with" "File path to the ICE File"
```

Also, when you replace with this method, you can use the -m option to keep the CRC the same.

```bat
ice.exe -m -r "File path of file you want to replace with" "File path to the ICE File"

ice.exe -c -m -r "File path of file you want to replace with" "File path to the ICE File"
```

## Specifications

Currently supported ICE versions are v3, v4, v5, v6, v7, v8, and v9

In an ICE file, stored files are divided into two groups.

Since the criteria for the grouping isn't fully known, when creating an ICE file,
Put the extensions in the `GroupList.txt` file into the group 1 folder.
Put all other files into the group 2 folder.

I've added the option to pass keys to the program. You can use -k to specify the
8 digit hexadecimal key.

```bat
ice.exe -k CD50379E "File path to the ICE File"
```

## Disclaimer

The author assumes no responsibility for any damage caused by using this application.
You use it at your own risk.
