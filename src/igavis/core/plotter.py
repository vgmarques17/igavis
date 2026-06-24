
import numpy as np
from ..io.readers import load_camera_preset
from .vtkobjects import SmoothFilters
import pyvista as pv
pv.global_theme.allow_empty_mesh = True
pv.OFF_SCREEN=True

def Plotter(args,anatomy_solid,anatomy_transp,ps_array,BTpos,DataQueue):
    print('Worker Initialized')

    # Initialize array for solid and transparent layers
    ## Endo
    solid_idx = anatomy_solid.points
    solid_idx = np.rint(solid_idx/args['scale']).astype('int') # Redundant

    anatomy_solid.point_data['vmf'] = np.ones(solid_idx.shape[0],dtype='float32')*-80
    # EndoObject = anatomy_solid.VTKObject

    ## Epi
    transp_idx = anatomy_transp.points
    transp_idx = np.rint(transp_idx/args['scale']).astype('int') # Redundant

    anatomy_transp.point_data['vmf'] = np.ones(transp_idx.shape[0],dtype='float32')*-80
    # EpiObject = anatomy_transp.VTKObject

    # Create filters
    if args['anatomy_ext']=='.igb':
        anatomy_solid = SmoothFilters(anatomy_solid)
        anatomy_transp = SmoothFilters(anatomy_transp)

    # Make a pyvista Plotter (subfunction, put in vtkobjects)
    if args['window_type'] == "side-by-side":
            layout = (1, 2)
    elif args['window_type'] == "vertical":
        layout = (2, 1)
    else:
        layout = (1, 1)
    
    plotter = pv.Plotter(shape=layout, window_size=[args['fig_width'], args['fig_height']],off_screen=True)

    # Load cameras
    cams = load_camera_preset(args['camera_config'],
                              args['camera_preset'])
    cam_names = list(cams.keys())
    # colorbar
    sargs = dict(height=0.25, vertical=True, position_x=0.9, position_y=0.05)
    # --- View 1 ---
    plotter.subplot(0, 0)
    mesh_solid_1 = plotter.add_mesh(anatomy_solid , opacity=1.0, scalars="vmf", show_scalar_bar = layout==(1,1),colormap=args['colormap'],clim=(-80,20)) #only add scalar bar if only one view
    mesh_transp_1 = plotter.add_mesh(anatomy_transp, opacity=0.5, scalars="vmf", show_scalar_bar = layout==(1,1),colormap=args['colormap'],clim=(-80,20))
    text_actor = plotter.add_text(f"{args['initial_stamp']} ms",font_size=14) #FIXME: tStep

    #Set camera
    plotter.camera_position = [cams[cam_names[0]]["position"],
                               cams[cam_names[0]]["focal_point"],
                               cams[cam_names[0]]["view_up"]]
    plotter.camera.zoom(cams[cam_names[0]]["zoom"])


    # --- View 2 (if applicable) ---
    if layout != (1, 1):
        plotter.subplot(layout[0]-1, layout[1]-1)

        mesh_solid_2 = plotter.add_mesh(anatomy_solid , opacity=1.0, scalars="vmf", show_scalar_bar=True,scalar_bar_args=sargs,colormap=args['colormap'],clim=(-80,20))
        mesh_transp_2 = plotter.add_mesh(anatomy_transp, opacity=0.5, scalars="vmf", show_scalar_bar=True,scalar_bar_args=sargs,colormap=args['colormap'],clim=(-80,20))
        # Set camera
        plotter.camera_position = [cams[cam_names[1]]["position"],
                                   cams[cam_names[1]]["focal_point"],
                                   cams[cam_names[1]]["view_up"]]
        plotter.camera.zoom(cams[cam_names[1]]["zoom"])

    plotter.subplot(0, 0)

    if args['plot_ps']:
        plotter,actors_to_remove = update_ps_plot(args,args['t_start'],ps_array,layout,plotter)

    # if args.plot_bt:
    #     if args.t_start in BTpos.keys():
    #         BTActor,BTGlyph = MakeBTActors(np.asarray(BTpos[args.t_start]),args)
    #     else:
    #         BTActor,BTGlyph = MakeBTActors(np.empty((0,3)),args) # This creates the actors but without points, I think
    #     # Add PS actor
    #     PosRen.AddActor(BTActor)
    #     AntRen.AddActor(BTActor)
    #     if args.plot_rv: RenRV.AddActor(BTActor)
    #     #Update
    #     PosRen.Modified()
    #     AntRen.Modified()     
    #     if args.plot_rv: RenRV.Modified()
    while True:

        QOut = DataQueue.get(True)
        message, data = QOut

        if message == "kill":
            print("Worker killed")
            break

        elif message == "data":
            t, solid_vals, transp_vals = data
            t_step = args['t_start'] + t * args['t_int']

            # ---- UPDATE DATA ARRAYS ----
            if args['anatomy_ext']=='.igb':
                anatomy_solid["vmf"] = solid_vals[anatomy_solid.point_data["vtkOriginalPointIds"]]
                anatomy_transp["vmf"] = transp_vals[anatomy_transp.point_data["vtkOriginalPointIds"]]
            else:
                anatomy_solid["vmf"] = solid_vals
                anatomy_transp["vmf"] = transp_vals

            # ---- UPDATE TEXT ----
            plotter.remove_actor(text_actor)
            text_actor = plotter.add_text(f"{t_step:04d} ms", font_size=14)
            # ---- UPDATE PS Array ----
            if args['plot_ps']:
                plotter,actors_to_remove = update_ps_plot(args,t_step,ps_array,layout,plotter,actors_to_remove = actors_to_remove)

            # ---- TRIGGER RENDER ----
            plotter.render()

            # ---- SCREENSHOT ----
            plotter.screenshot(
                f"{args['output_path']}/{args['basename']}{t_step:05d}.png"
            )


def update_ps_plot(args,t,ps_array,layout,plotter,actors_to_remove=None):
    # If there were actors before, remove them to update the points
    # Note: "actors" here are just pv instances added to the plotter
    if actors_to_remove is not None:
        for actor in actors_to_remove: plotter.remove_actor(actor)


    if t in ps_array[:,0]:
        ps_points = pv.PolyData(ps_array[np.where(ps_array[:,0]==t)[0],1:])
    else:
        ps_points = pv.PolyData(np.empty((0,3)))
    
    # Add PS to plotters
    mesh_ps_1 = plotter.add_mesh(ps_points , opacity=1.0,colormap=args['sphere_color'],render_points_as_spheres=True,point_size=args['sphere_radius']) 
    actors = [mesh_ps_1]
    if layout != (1, 1):
        plotter.subplot(layout[0]-1, layout[1]-1)
        mesh_ps_2 = plotter.add_mesh(ps_points , opacity=1.0,colormap=args['sphere_color'],render_points_as_spheres=True,point_size=args['sphere_radius']) 
        plotter.subplot(0, 0)
        actors = actors + [mesh_ps_2]

    return plotter,actors

    # Missing: Right view, BT

