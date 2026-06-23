import argparse
import os

def parse_arguments():

    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawDescriptionHelpFormatter,
                                     fromfile_prefix_chars='+')


    # Create argument groups
    file_group = parser.add_argument_group('File I/O')
    mesh_group = parser.add_argument_group('Mesh params')
    # ps_group = parser.add_argument_group('Phase singularity plotting')
    # bt_group = parser.add_argument_group('Breakthrough plotting')
    timing_group = parser.add_argument_group('Timing parameters')
    display_group = parser.add_argument_group('Camera parameters')
    platform_group = parser.add_argument_group('Platform-related parameters')


    # Required arguments
    parser.add_argument('anatomy-file',action='store',type=str,
                        help = 'File containing the anatomy. Can be .igb (for propag meshes) or openCARP formats')

    parser.add_argument('data-file',action='store',type=str,
                        help = 'Name of the experiment file. Can be iga (if the mesh is in igb) or igb (for openCARP meshes). ')

    # # Optional Arguments
    # Mesh arguments
    mesh_group.add_argument('--scale',
                              action='store',default =0.2,type=float,
                              help = 'Scale factor. Default 0.2')
    mesh_group.add_argument('--solid-val',
                             action='store',nargs='+',type=int,default =[50],
                             help = 'Cell codes to be plotted solid (not transparent). Default [50]')
    mesh_group.add_argument('--transp-val',
                             action='store',nargs='+',type=int,default =[52],
                             help = 'Cell codes to be plotted transparent. Default [52]')
    
    # Display arguments
    display_group.add_argument('--window-type',
                              action='store',default = 'side-by-side',type=str,
                              choices=['side-by-side','vertical','single'],
                              help = 'Path to the output folder. Default \'./anims\'')
    display_group.add_argument('--fig_width','-w',action='store',type=int,default =1600,
                              help = 'Figure width in pixels. Default 1800') 
    display_group.add_argument('--fig_height','-g',action='store',type=int,default =900,
                              help = 'Figure width in pixels. Default 800') 	
    display_group.add_argument('--camera-config',action='store',type=str,default = './cli/camera_config.json',
                              help = ".json file with camera position, focal point, view up, and zoom")
    display_group.add_argument('--camera-preset',action='store',type=str,default = 'default',
                              help = "choose the group with >=1 cameras. The cameras will be chosen in order")
    display_group.add_argument('--colormap',action='store',type=str,default = 'YlOrRd_r',
                              help = "Matplotlib colormap for plotting")

    # Timing arguments
    timing_group.add_argument('--initial-stamp',
                              action='store',default =0,type=int,
                              help = 'Initial time stamp for png output. Default 0')						
    timing_group.add_argument('--t-start',dest= 't_start',
                              action='store',default =0,type=int,
                              help = 'Initial time (in frames). Default 0')
    timing_group.add_argument('--t-end',dest= 't_end',
                              action='store',default =-1,type=int,
                              help = 'Final time (in frames). Default -1')
    timing_group.add_argument('--t-int',dest= 't_int',
                              action='store',default =5,type=int,
                              help = 'Time interval between frames. Default 5')
    # # ------------------------------------------------------------------------------
    ## Paths
    file_group.add_argument('--output-path','-o',
                            action='store',default = '%s/anims'%(os.getcwd()),type=str,
                            help = 'Path to the output folder. Default \'./anims\'')
    # # ------------------------------------------------------------------------------
    # ## PS related (currently only igb/iga)
    # ps_group.add_argument('--plot-ps',action='store_true',
    #                       help = 'Whether to plot the PS\'s')
    # ps_group.add_argument('--ps-file',type=str,
    #                       action='store',default = '',
    #                       help = 'Name of the file containing the PS data (with extension, should be a text file).')
    # ps_group.add_argument('--sphere_radius',type=float,
    #                       action='store',default = 10,
    #                       help = 'Radius of the PS spheres')
    # # ------------------------------------------------------------------------------
    # ## BT related
    # bt_group.add_argument('--plot-bt',action='store_true',
    #                       help = 'Whether to plot the BT\'s')
    # bt_group.add_argument('--bt-file',type=str,
    #                       action='store',default = '',
    #                       help = 'Name of the file containing the BT data (with extension, should be a text file).')
    # bt_group.add_argument('--cone-size',type=float,
    #                       action='store',default = 10,
    #                       help = 'Size of the BT cones')
    # # ------------------------------------------------------------------------------
    # ## Platform specific stuff

    platform_group.add_argument('--iga2igb',dest= 'iga2igb',
                             action='store',default = 'iga2igb',type=str,
                             help = 'Path to the iga2igb executable, if needed.')
    platform_group.add_argument('--np',dest= 'n_processes',
                             action='store',default = 2,type=int,
                             help = 'Number of parallel processes to generate the images')
    
                    
    args = parser.parse_args()
    # args.cameraTypePos,args.cameraTypeAnt,args.CameraTypeRight = MakeCameraDicts(args)
    # Run function
    return args
