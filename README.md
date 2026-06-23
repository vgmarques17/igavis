# igavis
Visualization tool for meshes stored in .igb format or reading from .igb data

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
cd /home/vgmarques/Documents/Code/igavis
PYTHONPATH=src /home/vgmarques/Documents/Code/igavis/igavis_env/bin/python -m igavis --help
```

Then run the visualization with your anatomy and data files:

```bash
PYTHONPATH=src /home/vgmarques/Documents/Code/igavis/igavis_env/bin/python -m igavis testing/two-layers-model24-30-heart-cell.igb /home/vgmarques/data/model-data/debug/exp906c71_short.iga.gz --camera-config src/igavis/camera_config.json --t-start 0 --t-end 100 --np 5 --output-path ./anims --plot-ps --ps-file testing/exp906c71_fake_ps.txt
```

## Create video from output images

```bash
ffmpeg -framerate 30 -pattern_type glob -i 'vm*.png' -c:v libx264 -pix_fmt yuv420p output.mp4
```
