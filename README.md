# IGAVis

## Introduction

IGAVis is a Python visualization tool for rendering simulation data off-screen. It supports anatomy meshes in `.igb` or `.vtk` format together with time series data in `.iga` or `.igb` format, and can generate image sequences suitable for video creation.

### Some nice features:
- You don't need to make meshalyzer work offscreen
- You can set the camera in Paraview and save its configuration in an easy to read .json file
    - You can also use the helper function provided here
- You can plot different layers with different opacities (endocardial solid and epicardial transparent, for example)
- You can plot and control the appearance of phase singularity points (in the formats for igb meshes and `.pts_t`)

### Some background
This software is a new implementation of the code I used to generate videos from the massive `.iga` files that came out of `propag` (hence the name). Because of that, it is implemented to read the data as fast as possible while the other processes plot the .png files. 

I have now adapted it to work similarly with the typical openCARP output, namely data as `.igb` files and PS tracking, as well as other auxiliary mesh data, as `pts_t`

## What is in this repository

- `src/igavis/main.py` — application launcher and pipeline selector
- `src/igavis/setup_camera.py` — Helper script to create custom cameras. Does not work headless
- `src/igavis/camera_config.json` — default camera configuration
- `setup_venv.sh` — helper script to create the virtual environment and install dependencies
- `setup_venv.sh` — helper script to create the virtual environment and install dependencies
- `launch.json` — Gives examples of how to run the different scripts and can be used for VSCode debug 

## Requirements and setup

This package relies on VTK and PyVista for plotting, and needs gcc for compiling a helper library for IGB support. 

From the repository root, use the setup script to create a virtual environment and install the package:

```bash
./setup_venv.sh
```

If `gcc` is present, the script also builds the native IGB helper library. 

### Manual IGB helper library build

If you need to build the IGB helper library manually:

```bash
cd src/igavis/io/igb
gcc -fPIC -shared -o libigb.so header.c igbwrapper.c
```

This produces `src/igavis/io/igb/libigb.so` and enables IGB file loading.

## How to run

Run the package with Python from the repo root.

### Using the installed entrypoint

```bash
python -m igavis <anatomy-file> <data-file> [options]
```

### Example command

Check `launch.json` for examples of both pipelines


### Common options

- `--output-path`, `-o` — output folder for generated images
- `--camera-config` — JSON file with camera settings. I provide a default one but this can be adjusted
- `--t-start`, `--t-end`, `--t-int` — frame range and sampling interval
- `--plot-ps` — enable phase singularity plotting
- `--ps-file` — phase singularity data file path
- `--np` — number of parallel processes for image generation

## Troubleshoot

- If display errors occur on headless systems, ensure an X server is available and `DISPLAY` is set:
```bash
Xvfb :99 -screen 0 1024x768x24 >/dev/null 2>&1 & # or whatever display number instead of 99
export DISPLAY=:99

python ...

killall Xvfb
```

- Use `.igb` anatomy with `.iga` data, or `.vtk` anatomy with `.igb` data.
-
- If dependency issues appear, confirm the virtual environment is activated and dependencies from `requirements.txt` are installed.

## To-do

- Adjust the readers to take .pts/.elem meshes (aka carp_txt)
- Breakthrough plots
- Transparent mesh is always plotted



## Video creation

After image frames are generated, use `ffmpeg` to assemble them into a video:

```bash
ffmpeg -framerate 30 -pattern_type glob -i 'vm*.png' -c:v libx264 -pix_fmt yuv420p output.mp4
```
