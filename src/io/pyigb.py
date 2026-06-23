import ctypes
import numpy as np
import os

# retrieve the dynamic library
import sysconfig
import importlib.resources
libname = "_igbwrapper" + sysconfig.get_config_var("EXT_SUFFIX")
# with importlib.resources.path('.',libname) as p:
#     igblib = ctypes.CDLL(p)
from pathlib import Path

igblib = ctypes.CDLL(str(Path(__file__).parent / "igb/libigb.so"))

__all__ = ["igb_read","igb_write","igb_header"]

_igb2numpy_type = {
        1: 'uint8',   # BYTE
        2: 'int8',    # CHAR
        3: 'int16',   # SHORT
        4: 'long',    # LONG
        5: 'float32', # FLOAT
        6: 'float64'} # DOUBLE
_igb_type = {
        1: 'byte',
        2: 'char',
        3: 'short',
        4: 'long',
        5: 'float',
        6: 'double'}
# header fields definition
class cIGBHeader(ctypes.Structure):
    _fields_  = [(s,ctypes.c_int) for s in ["x","y","z","t"]]
    _fields_ += [(s,ctypes.c_int) for s in ["type","taille","architecture"]]
    _fields_ += [("systeme",ctypes.c_uint)]
    _fields_ += [(s,ctypes.c_int) for s in ["num","bin","trame"]]
    _fields_ += [(s,ctypes.c_uint) for s in ["lut","comp"]]
    _fields_ += [("epais",ctypes.c_float)]
    _fields_ += [("{}_{}".format(s1,s2),ctypes.c_float)
                 for s1 in ["org","inc","dim","fac"]
                 for s2 in ["x","y","z","t"]]
    _fields_ += [("vect_z",ctypes.POINTER(ctypes.c_float))]
    _fields_ += [("unites{}".format(s),ctypes.c_char*41)
                 for s in ["_x","_y","_z","_t",""]]
    _fields_ += [(s,ctypes.c_float) for s in ["facteur","zero"]]
    _fields_ += [(s,ctypes.c_char*41)
                 for s in ["struct_desc","aut_name","id"]]
    _fields_ += [("date",ctypes.c_char*11)]
    _fields_ += [("comment",ctypes.POINTER(ctypes.c_char_p))]

# header booleans structure
class cIGBBoolHeader(ctypes.Structure):
    _fields_ = [(s__[0],ctypes.c_int) for s__ in cIGBHeader._fields_]

class IGBFile(object):

    def __init__(self,fname,mode="r"):
        self.fname = fname
        self.mode = mode
        self.c_header = cIGBHeader()
        self.c_bool_header = cIGBBoolHeader()
        if "r" in mode:
            # read the header from file if on read-mode
            self.read_header()
        elif "w" in mode:
            # initialize the header structure
            igblib.Bool_Header_Ini(ctypes.byref(self.c_bool_header))
            igblib.Header_Ini(ctypes.byref(self.c_header))
        else:
            raise RuntimeError("'r' and 'w' are only valid modes'")

    def get_header(self):
        header = {k:getattr(self.c_header,k)
                  for k,_ in self.c_header._fields_
                  if getattr(self.c_bool_header,k)}
        # fix comment
        if "comment" in header:
            it = iter(header["comment"])
            cmm = []
            while True:
                c = next(it)
                if c == None: break
                cmm.append(c)
            header["comment"] = cmm
        return header

    def read_header(self):
        c_fname = ctypes.create_string_buffer(self.fname.encode())
        e = igblib.igb_header_read(c_fname,
                ctypes.byref(self.c_header),
                ctypes.byref(self.c_bool_header))
        if e: raise RuntimeError

    def read_data(self,reshape=True):
        if "r" not in self.mode:
            raise RuntimeError
        x = self.get_property('x')
        y = self.get_property('y')
        z = self.get_property('z')
        try:
            t = self.get_property('t')
        except:
            t = 1
        igb_type = self.get_property('type')
        dtype = _igb2numpy_type[igb_type]
        with open(self.fname,"r") as f:
            f.seek(1024)
            data = np.fromfile(f,dtype=dtype,count=x*y*z*t)
        if reshape:
            if t==1:
                return data.reshape((z,y,x))
            else:
                return data.reshape((t,z,y,x))
        else:
            return data

    def write_header(self):
        c_fname = ctypes.create_string_buffer(self.fname.encode())
        igblib.igb_header_write(c_fname,
                ctypes.byref(self.c_header),
                ctypes.byref(self.c_bool_header))

    def write_data(self,data,**kwargs):
        # update headers
        for k,v in list(kwargs.items()):
            setattr(self.c_header,k,v)
            setattr(self.c_bool_header,k,1)
        # dimensions
        if data.ndim==3:
            z,y,x = data.shape
        else:
            t,z,y,x = data.shape
            self.c_header.t = t
            self.c_bool_header.t = 1
        self.c_header.x = x
        self.c_bool_header.x = 1
        self.c_header.y = y
        self.c_bool_header.y = 1
        self.c_header.z = z
        self.c_bool_header.z = 1
        # datatype
        np2igb = {v:k for k,v in list(_igb2numpy_type.items())}
        self.c_header.type = np2igb[data.dtype.name]
        self.c_bool_header.type = 1
        # write the header
        self.write_header()
        # write data
        with open(self.fname,"ab") as f:
            #data.tofile(f)
            f.write(data.tobytes())

    def get_property(self,attr):
        if hasattr(self.c_header,attr):
            if getattr(self.c_bool_header,attr):
                return getattr(self.c_header,attr)
            else:
                raise AttributeError("'{}' not available".format(attr))
        else:
            raise AttributeError("'{}' not found".format(attr))

    def set_property(self,attr,value):
        if hasattr(self.c_header,attr):
            setattr(self.c_header,attr,value)
            setattr(self.c_bool_header,attr,1)
        else:
            raise AttributeError("'{}' not found".format(attr))

    def unset_property(self,attr):
        if hasattr(self.c_header,attr):
            setattr(self.c_bool_header,attr,0)
        else:
            raise AttributeError("'{}' not found".format(attr))

    def __del__(self):
        igblib.Header_Clear(ctypes.byref(self.c_header))

# easy-access function to read IGBFile
def igb_read(fname,only_header=False,reshape=True):
    igbfile = IGBFile(fname,"r")
    if only_header:
        # convert to dictionary
        hdr = igbfile.get_header()
        hdr['type'] = _igb_type[hdr['type']]
        return hdr
    else:
        return igbfile.read_data(reshape=reshape)

def iga_header(stdout):
    buf = stdout.read(1024*np.uint8().nbytes)
    if buf.decode()[:8]=='leanIGB1':
        #
        hdr = {}
        dims = np.frombuffer(stdout.read(16*np.uint32().nbytes),dtype = np.uint32)
        hdr['Nx'] = dims[0]
        hdr['Ny'] = dims[1]
        hdr['Nz'] = dims[2]
        hdr['Nt'] = dims[3]
        hdr['Ne'] = dims[4] # number of foreground elements stored
        hdr['dtype'] = dims[5]
        hdr['arch'] = dims[6]
        #
        scaling = np.frombuffer(stdout.read(2*np.float32().nbytes),dtype = np.float32)
        hdr['facteur'] = scaling[0]
        hdr['zero'] = scaling[1]
        vs = 'coordinates'
        ts = stdout.read(len(vs)).decode()
        if ts!=vs: 
            RuntimeError('Lost track')
        coords = np.frombuffer(stdout.read((hdr['Ne']*3)*np.uint32().nbytes),dtype = np.uint32).reshape(hdr['Ne'],3)
        vs = 'data'
        ts = stdout.read(len(vs)).decode()
        if ts!=vs: 
            RuntimeError('Lost track')
    else:
        hdr = buf.decode().split('\r\n')
        cmm = [l.strip()[2:] for l in hdr if l.startswith("#")]
        hdr = sum((l.split() for l in (l.strip() for l in hdr if not l.startswith("#") or l.startswith("comments")) if len(l)>0),[])
        hdr = dict(tuple(ob.split(":")) for ob in hdr)
        hdr['comments'] = cmm
        for a in 'xyzt':
            if a in hdr: hdr[a] = int(hdr[a]) #TODO: should I use the ctypes?
        for a in ['zero','facteur']:
            if a in hdr: hdr[a] = float(hdr[a]) #TODO: should I use the ctypes?
        coords=None
    return hdr,coords


def igb_header(fname,lean=False):
    '''
    Reads only the igb/iga header. Adapted to deal with opened files (like streaming from iga.gz) and lean headers
    The "lean" part has been substituted by iga_header. I've kept the function to avoid errors
    '''
    hdr = igb_read(fname,only_header=True)
    return hdr

# easy-access function to write IGBFile
def igb_write(data,fname,**kwargs):
    igbfile = IGBFile(fname,"w")
    igbfile.write_data(data,**kwargs)

if __name__ == "__main__":
    # test
    cdata = igb_read("/data/anatomy/potse/crt012/crt012-01-heart1mm-c.igb")
    igb_write(cdata,"test.igb",inc_x=1.0,facteur=0.6)
    print(igb_read("test.igb",only_header=True))
    cdata2 = igb_read("test.igb")
    print((cdata2 == cdata).all())

