#%%
import time
from datetime import datetime
import os
import sys
from .config.argument_parser import parse_arguments
from .io.readers import read_anatomy
from .core.iga_pipeline import run_iga_pipeline
from .core.igb_pipeline import run_igb_pipeline

#%%

def main():
    args = parse_arguments()

    os.makedirs(args.output_path, exist_ok=True)

    ### Read the mesh ###
    fname_anatomy = args.anatomy
    if not os.path.isfile(fname_anatomy):
        print('Anatomy file %s does not exist. Exiting.' % (fname_anatomy))
        sys.exit(1)
    else:
        args.anatomy_ext = os.path.splitext(fname_anatomy)[1]

    print('Reading anatomy file %s...' % (fname_anatomy))
    anatomy_solid = read_anatomy(args, fname_anatomy, layer='solid')
    anatomy_transp = read_anatomy(args, fname_anatomy, layer='transparent')

    ### Choose the pipeline: iga or igb ###
    data_filename = os.path.split(args.data)[1]
    args.basename, args.data_ext = os.path.splitext(data_filename)

    # If compressed, check again
    if args.data_ext == '.gz':
        args.data_ext = os.path.splitext(args.basename)[1] + args.data_ext
        args.basename = os.path.splitext(args.basename)[0]

    print(f'Basename: {args.basename} and extension {args.data_ext}')
    if args.anatomy_ext == '.igb':
        # only iga files allowed
        if args.data_ext != '.iga' and args.data_ext != '.iga.gz':
            print('Anatomy file %s needs an iga file to plot. Exiting.' % (fname_anatomy))
            sys.exit(1)
        else:
            run_iga_pipeline(args, anatomy_solid, anatomy_transp)

    elif args.anatomy_ext == '.vtk':
        # only igb files allowed
        if args.data_ext != '.igb' and args.data_ext != '.igb.gz':
            print('Anatomy file %s needs an igb file to plot. Exiting.' % (fname_anatomy))
            sys.exit(1)
        else:
            run_igb_pipeline(args, anatomy_solid, anatomy_transp)



if __name__ == "__main__":
    start_time = time.time()
    human_start_time = datetime.fromtimestamp(start_time).strftime('%Y-%m-%d %H:%M:%S') 
    print('Execution started at: %s'%(human_start_time))
    main()
    end_time = time.time()
    human_end_time = datetime.fromtimestamp(end_time).strftime('%Y-%m-%d %H:%M:%S') 
    print('Execution time: %0.3f seconds'%(end_time - start_time))