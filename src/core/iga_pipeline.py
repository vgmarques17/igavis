import subprocess
from src.io.pyigb import iga_header
import numpy as np
import multiprocessing as mp
from src.core.plotter import Plotter


def Reader(args,solid_idx,transp_idx,DataQueue):
    print('Reader initialized')
    fname_signals = args['data-file']
    t_start = args['t_start']
    t_end   = args['t_end']
    t_int   = args['t_int']

    tString = str(t_start)+':'+str(t_int)+':'+str(t_end)   

    # Reads the data and puts in queue

    with subprocess.Popen(["gzip","-dc",fname_signals],stdout=subprocess.PIPE) as gz:
        with subprocess.Popen([args['iga2igb'],"-x","0:1:-1","-y","0:1:-1","-z","0:1:-1",
			   "-t",tString,"-","-"],stdin=gz.stdout,stdout=subprocess.PIPE) as iga:
            # read the header
            hdr,_ = iga_header(iga.stdout) #.read(1024)

            # size of a single slice in time
            nx,ny,nz = hdr['x'],hdr['y'],hdr['z']
            dtype = np.short
            csize = nx*ny*nz*dtype().nbytes

            if t_end==-1:
                t_end=t_start+hdr['t']*t_int-1 #FIXME: I think this is correct now, but have to check
                print('t_end: %d'%t_end)

            # Loop over data
            for t in range(0,int((t_end-t_start)/t_int+1)):
                vals = np.frombuffer(iga.stdout.read(csize),dtype=dtype).reshape((nz,ny,nx))
                # Endocardium
                solid_vals = vals[solid_idx[:,2],solid_idx[:,1],solid_idx[:,0]].astype('float32')
                solid_vals = hdr['facteur']*solid_vals + hdr['zero']
                # Epicardium
                transp_vals = vals[transp_idx[:,2],transp_idx[:,1],transp_idx[:,0]].astype('float32')
                transp_vals = hdr['facteur']*transp_vals + hdr['zero']

                DataQueue.put(('data',(t,solid_vals,transp_vals)))
    [DataQueue.put(('kill',None)) for i in range(args['n_processes'])];

def run_iga_pipeline(args,anatomy_solid,anatomy_transp):
    print('Running IGA pipeline...')

    solid_idx = anatomy_solid.points
    solid_idx = np.rint(solid_idx/args['scale']).astype('int') # Redundant
    transp_idx = anatomy_transp.points
    transp_idx = np.rint(transp_idx/args['scale']).astype('int') # Redundant

    if args['plot_ps']:
        # Read file
        fi = open(args['ps_file'],'r')
        rows = fi.readlines()

        # parse file accordingly
        header = rows[0]
        ps_array = np.asarray([row.split('\n')[0].split(' ') for row in rows[1:]]).astype(float)
        ps_array = ps_array[:,:-1]
        ps_array[:,1:] = ps_array[:,1:]*args['scale']
    else:
        ps_array = None
        

    # Plotter(args,anatomy_solid,anatomy_transp,ps_array,None,None)
    with mp.Manager() as DataManager:
        DataQueue = DataManager.Queue(maxsize=5) #maxsize=15
        
        #Data In
        DataReader = mp.Process(target = Reader,args = (args,solid_idx,transp_idx,DataQueue,))
        # Plotting
        PlottingPool = mp.Pool(args['n_processes'],Plotter,(args,anatomy_solid,anatomy_transp,ps_array,None,DataQueue,)) #PSpos, BTPos

        # Open and wait for end 
        DataReader.start()
        DataReader.join()

        PlottingPool.close()
        PlottingPool.join()