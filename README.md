# IGAVis
Visualization tool for rendering videos of simulations off screen.

This software is based on the a "legacy" version of my plotting tool for making videos for propag simulations. There, I had to be very efficient to read and plot the massive iga files. I extended the same logic now to work with the openCARP formats

This repository contains:

- igavis: the main software, able to render transparent and solid meshes in a layout of one or two subplots

 meshes stored in .igb format or reading from .igb data

## Setup

From the repository root, run the setup script to create the virtual environment, install dependencies, install the package in editable mode, and build the IGB helper library:

```bash
./setup_venv.sh
```

If `gcc` is not available, the script will still create the environment and install the package, but building `libigb.so` will be skipped. Install `gcc` and rerun the script to build the helper library.

## Build the IGB helper library manually

The native IGB helper library must be built before using the package.

From the repository root:

```bash
cd src/igavis/io/igb
gcc -fPIC -shared -o libigb.so header.c igbwrapper.c
```

This creates `src/igavis/io/igb/libigb.so` and allows `igavis` to load IGB files.

## Running the app

Install the package in editable mode:

```bash
python -m igavis --help

# You may need to unset the pythonpath to avoid conflicting library versions
# You may also need an X server on a HPC platform
export DISPLAY=:0
# killall Xvfb
Xvfb :0 -screen 0 1600x1200x24 &

igavis ...

killall Xvfb
```

Then run the visualization with your anatomy and data files:

```bash
PYTHONPATH=src /home/vgmarques/Documents/Code/igavis/igavis_env/bin/python -m igavis testing/two-layers-model24-30-heart-cell.igb /home/vgmarques/data/model-data/debug/exp906c71_short.iga.gz --camera-config src/igavis/camera_config.json --t-start 0 --t-end 100 --np 5 --output-path ./anims --plot-ps --ps-file testing/exp906c71_fake_ps.txt
```

## Create video from output images

```bash
ffmpeg -framerate 30 -pattern_type glob -i 'vm*.png' -c:v libx264 -pix_fmt yuv420p output.mp4
```
