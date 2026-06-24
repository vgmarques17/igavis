import vtk
from .pyigb import igb_read
from itertools import product
import numpy as np
import os
from vtkmodules.numpy_interface import dataset_adapter as dsa
import pyvista as pv

import json

__all__ = ['read_anatomy']

def read_anatomy(args,anatomy_file,layer='solid'):
    '''
    Read the anatomy, either as igb or openCARP formats, and return a unstructured grid
    '''
    file_extension = os.path.splitext(anatomy_file)[1]
    if file_extension == '.igb':
        if layer == 'solid':
            valid_cells = args.solid_val
        elif layer == 'transparent':
            valid_cells = args.transp_val
        else:
            RuntimeError('Invalid layer name')
            
        grid = IGBUnstructuredGrid(anatomy_file,valid_cells=valid_cells,units=args.scale)
        vtk_grid = grid.to_vtkUnstructuredGrid() 
        vtk_grid = pv.UnstructuredGrid(vtk_grid)
        # vtk_grid = dsa.WrapDataObject(vtk_grid)
        # bpts  = EndoGrid.Points
        # EndoIdx = np.rint(bpts/scale).astype('int') # Redundant
        return vtk_grid
    
    elif file_extension == '.vtk':
        vtk_grid = pv.UnstructuredGrid(anatomy_file)
        
        if layer == 'solid':
            valid_cells = args.solid_val
        elif layer == 'transparent':
            valid_cells = args.transp_val
        else:
            raise RuntimeError('Invalid layer name')
            
        tag_name = 'elemTag'
        if valid_cells==[-1]:
            #Pick all elem tags
            valid_cells = np.unique(vtk_grid.cell_data[tag_name])
        vtk_grid = vtk_grid.extract_cells(np.isin(vtk_grid.cell_data[tag_name],valid_cells),pass_cell_ids=True, pass_point_ids=True)
        
        return vtk_grid

class IGBUnstructuredGrid:
    "Simple wrapper around IGB file to convert it to unstructured grid"
    
    def __init__(self,fname_cell,valid_cells=None,units=1.0):
        ""
    
        self.fname = fname_cell
        self.valid_cells = valid_cells
        self.units = units

    def get_cell(self):
        
        if not hasattr(self,"cell"):
            # read cell file
            self.cell = igb_read(self.fname)

        return self.cell

    def get_dims(self):
        cell = self.get_cell()
        return cell.shape
    
    def get_cubes(self):
        
        if not hasattr(self,"cubes"):
            cell = self.get_cell()
            valid = np.isin(cell,self.valid_cells)
            self.cubes = np.where(valid)
            self.ncub  = len(self.cubes[0])
        
        return self.cubes

    def get_nodes(self):
        
        if not hasattr(self,"nodes"):
            cell = self.get_cell()
            nz,ny,nx = cell.shape
        
            # cube coordinates
            cz,cy,cx = self.get_cubes()

            # vertex map
            ppt = np.zeros(shape=(nz+1,ny+1,nx+1),dtype=bool)
            for i,j,k in product(*[[0,1]]*3):
                ppt[cz+k,cy+j,cx+i] = True

            self.nodes = np.where(ppt)
            
        return self.nodes
 
    def get_ver2nod(self):
        
        tz,ty,tx = self.get_nodes()
        nz,ny,nx = self.get_cell().shape
        
        ver2nod = np.zeros(shape=(nz+1,ny+1,nx+1),dtype=int)
        ver2nod[tz,ty,tx] = np.arange(tz.shape[0])
        
        return ver2nod

    def get_points(self):
        "Return the list of points"

        # nodes coordinates
        tz,ty,tx = self.get_nodes()
        pts = self.units*(np.c_[tx,ty,tz].astype(float))
        
        return pts
        
    def get_topology(self,ordering=None):
        "Compute topology"
        
        if ordering is None:
            # VTK ordering
            ordering= [[0,0,0],[1,0,0],[1,1,0],[0,1,0],
                       [0,0,1],[1,0,1],[1,1,1],[0,1,1]]
            
        cz,cy,cx = self.get_cubes()
        ver2nod = self.get_ver2nod()
        hexa = np.empty(shape=(self.ncub,8),dtype=int)
        for col,(i,j,k) in enumerate(ordering):
            hexa[:,col] = ver2nod[cz+k,cy+j,cx+i]
        
        return hexa
        
    def to_vtkUnstructuredGrid(self):
        "Convert to VTK"

        import vtk
        from vtk.util.numpy_support import numpy_to_vtkIdTypeArray,numpy_to_vtk #type:ignore

        pts  = self.get_points()
        hexa = self.get_topology()
        
        # VTK points
        points = vtk.vtkPoints()
        pdata = numpy_to_vtk(pts)
        points.SetData(pdata)
        
        # VTK cells
        cells = vtk.vtkCellArray()
        cdata_con = np.empty(shape=(hexa.shape[0],9),dtype=np.int64)
        cdata_con[:,0] = 8
        cdata_con[:,1:] = hexa
        cdata_con = numpy_to_vtkIdTypeArray(cdata_con.ravel())
        cells.SetCells(hexa.shape[0],cdata_con)
        
        # VTK grid
        grid = vtk.vtkUnstructuredGrid()
        grid.SetPoints(points)
        grid.SetCells(vtk.VTK_HEXAHEDRON,cells)

        return grid

from importlib.resources import files


def load_camera_preset(config_file: str | None, preset: str):
    """
    Load a camera preset from the camera configuration file.

    Parameters
    ----------
    config_file : str | None
        Path to camera_config.json. If None, use the packaged config.
    preset : str
        Name of the preset group under "presets".

    Returns
    -------
    dict
        {
            camera_name: {
                "position": tuple,
                "focal_point": tuple,
                "view_up": tuple,
                "zoom": float,
            }
        }
    """

    if config_file is None:
        config_file = files("igavis").joinpath("camera_config.json")

    with open(config_file, "r") as f:
        cfg = json.load(f)

    presets = cfg.get("presets", {})

    if preset not in presets:
        available = ", ".join(sorted(presets))
        raise ValueError(
            f"Camera preset '{preset}' not found. "
            f"Available presets: {available}"
        )

    cameras = {}

    for cam_name, cam in presets[preset].items():
        cameras[cam_name] = {
            "position": tuple(cam["position"]),
            "focal_point": tuple(cam["focal_point"]),
            "view_up": tuple(cam["view_up"]),
            "zoom": float(cam.get("zoom", 1.0)),
        }

    return cameras