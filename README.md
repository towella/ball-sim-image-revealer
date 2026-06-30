# ball-sim-image-revealer

Project designed for physically simulating balls which, when coloured, settle to form an image.

## Dependencies
### python 3.14 (inc tkinter)
- `tkinter` (part of python install)
- `pyinstaller` (for bundling)
- `generate-iconset` (for icon generation)
### C++
- `SDL2` (included)
- `ffmpeg` (included)

## Notes
- Mac executable and application files in dist folder
- required C++ packages and executables for python execution (including compiled c++ sim) are contained under include folder
- requirements.txt does not include tkinter as it is not provided by pip (see python installation or use brew)
- app_icon.icns generated with `generate-iconset` python package: `generate-iconset path/to/file.png --use-sips` where the image file is 512x512 or 1024x1024 at 300dpi