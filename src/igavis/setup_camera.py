#!/usr/bin/env python3
import os
import json
import argparse
import pyvista as pv
import numpy as np

from .io.readers import read_anatomy

def main():
    parser = argparse.ArgumentParser(
        description="Interactive tool to configure camera presets and save them to a JSON file."
    )
    parser.add_argument(
        "anatomy",
        type=str,
        help="Path to the anatomy file (.igb or .vtk)"
    )
    parser.add_argument(
        "--scale",
        type=float,
        default=0.2,
        help="Scale factor for igb mesh (default: 0.2)"
    )
    parser.add_argument(
        "--valid-cells",
        nargs="+",
        type=int,
        default=[-1],
        help="Cell codes to plot (default: all elemTag, but only if the mesh is .vtk)"
    )
    parser.add_argument(
        "--fig-width",
        type=int,
        default=800,
        help="Figure width (default: 1600)"
    )
    parser.add_argument(
        "--fig-height",
        type=int,
        default=900,
        help="Figure height (default: 900)"
    )
    parser.add_argument(
        "--output",
        "-o",
        type=str,
        default="setup_camera_config.json",
        help="Path to save the output JSON configuration (default: setup_camera_config.json)"
    )
    parser.add_argument(
        "--preset-name",
        type=str,
        default="custom_preset",
        help="Name of the preset group in the output JSON (default: custom_preset)"
    )

    args = parser.parse_args()

    # Prepare mock args dictionary for read_anatomy
    args.solid_val  =args.valid_cells

    if not os.path.isfile(args.anatomy):
        print(f"Error: Anatomy file '{args.anatomy}' does not exist.")
        sys.exit(1)

    print(f"Reading anatomy file '{args.anatomy}'...")
    anatomy_solid = read_anatomy(args, args.anatomy, layer='solid')


    presets = {}

    while True:
        print("\nOpening interactive window...")
        print("Rotate, pan, and zoom to position the camera. Close the window when done to save the view.")

        # Create a pyvista Plotter
        plotter = pv.Plotter(window_size=[args.fig_width, args.fig_height], off_screen=False)
        plotter.add_mesh(anatomy_solid, opacity=1.0, color='gray')

        # Set text instruction
        plotter.add_text(
            "Rotate/pan/zoom to configure camera.\nClose the window to save this view.",
            position="upper_left",
            font_size=12,
            color="white"
        )

        # Show the plotter window
        plotter.show()

        # Capture camera position
        cam_pos = plotter.camera_position
        if cam_pos is None:
            print("Warning: No camera position captured.")
            cam_pos = plotter.camera.position, plotter.camera.focal_point, plotter.camera.view_up

        # Ask for the name of the view
        while True:
            view_name = input("\nEnter the name of this view (e.g., 'posterior', 'anterior', 'right'): ").strip()
            if view_name:
                break
            print("View name cannot be empty.")

        presets[view_name] = {
            "position": list(cam_pos[0]),
            "focal_point": list(cam_pos[1]),
            "view_up": list(cam_pos[2]),
            "zoom": 1.0
        }

        print(f"Captured view '{view_name}':")
        print(f"  Position: {presets[view_name]['position']}")
        print(f"  Focal Point: {presets[view_name]['focal_point']}")
        print(f"  View Up: {presets[view_name]['view_up']}")

        # Ask if they want to save another view
        while True:
            choice = input("\nDo you want to save another view? (y/n): ").strip().lower()
            if choice in ['y', 'yes', 'n', 'no']:
                break
            print("Please enter 'y' or 'n'.")

        if choice in ['n', 'no']:
            break

    # Save all views to JSON
    config_data = {"presets": {}}
    if os.path.exists(args.output):
        try:
            with open(args.output, "r") as f:
                config_data = json.load(f)
                if "presets" not in config_data:
                    config_data["presets"] = {}
        except Exception:
            pass

    config_data["presets"][args.preset_name] = presets

    with open(args.output, "w") as f:
        json.dump(config_data, f, indent=2)

    print(f"\nSaved preset '{args.preset_name}' with {len(presets)} view(s) to '{args.output}'.")

if __name__ == "__main__":
    main()
