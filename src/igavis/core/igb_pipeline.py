import os
import subprocess
import numpy as np
import multiprocessing as mp
from .plotter import Plotter

# Mapping from IGB type string/ID to numpy data type
_type_mapping = {
    '1': np.uint8,
    '2': np.int8,
    '3': np.int16,
    '4': np.int32,
    '5': np.float32,
    '6': np.float64,
    'byte': np.uint8,
    'char': np.int8,
    'short': np.int16,
    'long': np.int32,
    'float': np.float32,
    'double': np.float64
}

def parse_igb_header_from_bytes(buf):
    # Decode the 1024 bytes as ASCII/UTF-8
    text = buf.decode('utf-8', errors='ignore').strip()
    # Split by whitespace
    parts = text.split()
    hdr = {}
    for part in parts:
        if ':' in part:
            k, v = part.split(':', 1)
            hdr[k] = v
            
    # Convert types
    for key in ['x', 'y', 'z', 't']:
        if key in hdr:
            hdr[key] = int(hdr[key])
    for key in ['facteur', 'zero']:
        if key in hdr:
            hdr[key] = float(hdr[key])
    return hdr

def Reader(args, solid_idx,transp_idx,DataQueue):
    print('Reader initialized')
    fname_signals = args.data
    t_start = args.t_start
    t_end   = args.t_end
    t_int   = args.t_int

    # Open the file stream (supporting gzip and uncompressed)
    proc = None
    if fname_signals.endswith('.gz'):
        proc = subprocess.Popen(["gzip", "-dc", fname_signals], stdout=subprocess.PIPE)
        f = proc.stdout
    else:
        f = open(fname_signals, "rb")

    try:
        # Read the 1024-byte header
        header_bytes = f.read(1024)
        if len(header_bytes) < 1024:
            raise RuntimeError("Failed to read 1024-byte IGB header")

        hdr = parse_igb_header_from_bytes(header_bytes)

        nx = hdr.get('x', 1)
        ny = hdr.get('y', 1)
        nz = hdr.get('z', 1)
        nt = hdr.get('t', 1)
        
        type_str = hdr.get('type', 'float')
        dtype = _type_mapping.get(type_str, np.float32)
        element_size = np.dtype(dtype).itemsize
        frame_size = nx * ny * nz * element_size

        facteur = hdr.get('facteur', 1.0)
        zero = hdr.get('zero', 0.0)

        if t_end == -1 or t_end >= nt:
            t_end = nt - 1
            print(f"t_end adjusted to: {t_end}")

        current_file_frame = 0

        # Loop over data and put in queue
        num_steps = int((t_end - t_start) / t_int + 1)
        for t in range(num_steps):
            target_frame = t_start + t * t_int
            
            skip_frames = target_frame - current_file_frame
            if skip_frames > 0:
                # Skip unused frames
                f.read(skip_frames * frame_size)
            
            # Read current frame
            buf = f.read(frame_size)
            if len(buf) < frame_size:
                print("End of file reached early")
                break
                
            vals = np.frombuffer(buf, dtype=dtype).astype('float32')
            vals = facteur * vals + zero

            # For VTK meshes, solid and transparent values are the same
            solid_vals = vals[solid_idx]
            transp_vals = vals[transp_idx]

            DataQueue.put(('data', (t, solid_vals, transp_vals)))
            current_file_frame = target_frame + 1

    finally:
        if proc is not None:
            proc.terminate()
            proc.wait()
        else:
            f.close()

    # Send kill message to all workers
    for _ in range(args.n_processes):
        DataQueue.put(('kill', None))

def run_igb_pipeline(args, anatomy_solid, anatomy_transp):
    print('Running IGB pipeline...')
    solid_idx = anatomy_solid.point_data['vtkOriginalPointIds']
    transp_idx = anatomy_transp.point_data['vtkOriginalPointIds']

    if args.plot_ps:
        # Read file
        with open(args.ps_file,'r') as fi:
            rows = fi.readlines()
            rows = rows[2:]

        data = []
        new_tstep=False
        for row in rows:
            if new_tstep:
                new_tstep=False
                continue
            
            if row.startswith('#'):
                # That is a timestep
                t = row.split('#')[-1].split()[0]
                t = float(t)
                new_tstep=True
                continue
            else:
                x,y,z = row.split('#')[0].split()
                data.append([t,x,y,z])
        ps_array = np.asarray(data,float)
    else:
        ps_array = None
    # Plotter(args,anatomy_solid,anatomy_transp,ps_array,None,None)
    with mp.Manager() as DataManager:
        DataQueue = DataManager.Queue(maxsize=5)
        
        # Data In
        DataReader = mp.Process(target=Reader, args=(args,solid_idx,transp_idx, DataQueue,))
        # Plotting
        PlottingPool = mp.Pool(args.n_processes, Plotter, (args, anatomy_solid, anatomy_transp, ps_array, None, DataQueue,))

        # Open and wait for end 
        DataReader.start()
        DataReader.join()

        PlottingPool.close()
        PlottingPool.join()
