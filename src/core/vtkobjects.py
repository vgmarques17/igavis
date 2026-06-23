"""
Created on Tue Feb  2 09:25:17 2021

DESCRIPTION: This is a library to organize the standard configurations
of VTK objects to be used in 3D visualizations. It is essentially only
VTK functions organized to make the main code cleaner

@author: Victor G. Marques, v.goncalvesmarques@maastrichtuniversity.nl
"""
import vtk
import numpy as np
import pyvista as pv

__all__ = ['SmoothFilters',
           'StandardAtrialMapper','EGMAtrialMapper','EGMPhaseAtrialMapper',
           'StandardLayerActor','ColorbarActor','TimeStampActor',
           'StandardViewRenderer',
           'TwoViewsRenderWindow','OneViewRenderWindow'] #Outer packages

__author__ = "Victor G. Marques"
__email__  = "v.goncalvesmarques@maastrichtuniversity.nl"

#%% Filters
def SmoothFilters(input_object,iterations = 10, relax_factor = 0.5):

    # If the input_object has a custom "vtkOriginalPointIds" (meaning it was extracted/sub-sampled),
    # rename it so it doesn't get overwritten by extract_surface.
    has_original_ids = "vtkOriginalPointIds" in input_object.point_data
    if has_original_ids:
        input_object.point_data["vtkOriginalPointIds_original"] = input_object.point_data["vtkOriginalPointIds"]

    # Convert everything to PyVista PolyData surface
    surf = pv.wrap(input_object).extract_surface(algorithm='dataset_surface',pass_cellid=True, pass_pointid=True)

    # Smooth (equivalent to vtkSmoothPolyDataFilter)
    surf = surf.smooth(
        n_iter=iterations,
        relaxation_factor=relax_factor,
        feature_smoothing=False,
        boundary_smoothing=True,
    )

    # Compute normals (equivalent to vtkPolyDataNormals)
    surf = surf.compute_normals(
        point_normals=True,
        cell_normals=True,
        auto_orient_normals=False,
        consistent_normals=True,
    )

    # If we saved the original IDs, map the surface's vtkOriginalPointIds back to the original mesh points
    if has_original_ids:
        surf.point_data["vtkOriginalPointIds"] = surf.point_data["vtkOriginalPointIds_original"][surf.point_data["vtkOriginalPointIds"]]
        # Clean up the temporary array
        del surf.point_data["vtkOriginalPointIds_original"]
        # Also clean up from input_object to avoid polluting it
        del input_object.point_data["vtkOriginalPointIds_original"]

    return surf


#%% Mappers
def StandardAtrialMapper(inputObject,tableSize, amplitudeRange = [-80,40],**kwargs):

    AtrialMapper = vtk.vtkPolyDataMapper()
    AtrialMapper.SetInputData(inputObject)
    AtrialMapper.SetScalarRange(amplitudeRange)

    LuT = FrontiersColormap(tableSize)
    AtrialMapper.SetLookupTable(LuT)

    return AtrialMapper

def EGMAtrialMapper(inputObject,tableSize, amplitudeRange = [-10,10],**kwargs):

    AtrialMapper = vtk.vtkPolyDataMapper()
    AtrialMapper.SetInputData(inputObject)
    AtrialMapper.SetScalarRange(amplitudeRange)

    if tableSize is None:
        return AtrialMapper
 
    LuT = EGMColormap(tableSize)
    AtrialMapper.SetLookupTable(LuT)

    return AtrialMapper

def EGMPhaseAtrialMapper(inputObject,tableSize,**kwargs):

    AtrialMapper = vtk.vtkPolyDataMapper()
    AtrialMapper.SetInputData(inputObject)
    AtrialMapper.SetScalarRange([-np.pi,np.pi])

    if tableSize is None:
        return AtrialMapper

    LuT = PhaseColormap(tableSize)
    AtrialMapper.SetLookupTable(LuT)

    return AtrialMapper

#%% Data Actors
def StandardLayerActor(atrialMapper,opacity):
    LayerActor = vtk.vtkActor()
    LayerActor.GetProperty().SetOpacity(opacity)
    LayerActor.SetMapper(atrialMapper) 

    return LayerActor

#%% Renderers
def StandardViewRenderer(actorsList,cameraType=None,colorbarActor = None,textActor = None,viewportSplit = None):

    ViewRenderer = vtk.vtkRenderer()
    ViewRenderer.SetUseDepthPeeling(True)

    viewportSplitDict = {'horizontal-left':[0,0,0.4,1],
                        'horizontal-right':[0.4,0,1,1],
                        'half-left':[0,0,0.5,1],
                        'half-right':[0.5,0,1,1],
                        'vertical-down':[0,0,1,0.5],
                        'vertical-up':[0,0.5,1,1],
                        None:[0,0,1,1]}

    ViewRenderer.SetViewport(viewportSplitDict[viewportSplit])
    for actor in actorsList:
        ViewRenderer.AddActor(actor)

    if colorbarActor is not None:
        ViewRenderer.AddActor2D(colorbarActor)
    if textActor is not None:
        ViewRenderer.AddActor2D(textActor)

    if cameraType is not None:
        SetCamera(ViewRenderer,cameraType)

    colors = vtk.vtkNamedColors()
    bkg = map(lambda x: x, [255, 255, 255, 255])
    colors.SetColor("BkgColor", *bkg)

    ViewRenderer.SetBackground(colors.GetColor3d("BkgColor"))	

    return ViewRenderer

#%% Cameras
# Camera dictionaries
PosteriorDict = {'Position':[108.09430696880418, -76.1951818138545,-149.05382459124448],
                'FocalPoint':[54.693900645253095, 37.85250878678161, 40.10619050431344],
                'ViewUp':[-0.1105058675706691, 0.837042209778677, -0.5358626617719163],
                'Zoom':1.2}

AnteriorDict = {'Position':[107.51474741673346, 305.1080215893674, 45.847554586203245],
                'FocalPoint':[63.225564671242715, 39.61607918958433, 40.33986524809277],
                'ViewUp':[-0.22367346377168093, 0.043813444269235405, -0.9736788812055263],
                'Zoom':1.2}
closerPosteriorDict = PosteriorDict.copy()
closerPosteriorDict['Zoom'] = 1.4

closerAnteriorDict = AnteriorDict.copy()
closerAnteriorDict['Zoom'] = 1.6
        
EGMPosteriorDict = {'Position':[129.71740725676034, -110.78234218779386, -136.57692291435296],
                'FocalPoint':[-11.318411565279515, 188.6087097174139, 210.25384007551384],
                'ViewUp':[-0.0612513387002729, 0.74310281432688, -0.6663681271241108],
                'Zoom':1.4}
EGMAnteriorDict = {'Position':[133.19591932597, 296.08851219797714, 14.21473694798931],
            'FocalPoint':[132.8867172753933, 295.0643000090744, 14.323431586007993],
            'ViewUp':[-0.2582443023575035, -0.024530222872463432, -0.9657681649680288],
            'Zoom':1.5}    

VMRightDict = {'Position':[-160.21175566775457, 89.54452626561813, 105.79946321976377],
                'FocalPoint':[52.067371495331464, 38.632232415124484, 42.66863918292802],
                'ViewUp':[0.12537549694129713, 0.9348034735673706, -0.3323002416085507],
                'Zoom':1.4}

def SetCamera(renderer,cameraType):
    # For backwards compatibility, 
    # I don't know if I need both resets here
    if type(cameraType) is dict: 
        cameraDict=cameraType
    else:
        #Define new cameras in this dictionary
        CameraTypeDict= {'Posterior':PosteriorDict,
                        'Anterior':AnteriorDict,
                        'closerPosterior':closerPosteriorDict,
                        'closerAnterior':closerAnteriorDict,
                        'egmPosterior':EGMPosteriorDict,
                        'egmAnterior':EGMAnteriorDict,
                        'VMRightDict':VMRightDict}
        cameraDict = CameraTypeDict[cameraType]

    renderer.GetActiveCamera().SetPosition(cameraDict['Position'])
    renderer.GetActiveCamera().SetFocalPoint(cameraDict['FocalPoint'])
    renderer.GetActiveCamera().SetViewUp(cameraDict['ViewUp'])
    renderer.ResetCamera()
    renderer.GetActiveCamera().Zoom(cameraDict['Zoom'])
    renderer.ResetCameraClippingRange()
    #print(renderer.GetActiveCamera().Zoom())



#%% Render Windows
def TwoViewsRenderWindow(rendererView1,rendererView2,windowSize = [1800,800],
                         OffScreenRendering = True):

    renWin = vtk.vtkRenderWindow()
    renWin.SetOffScreenRendering(OffScreenRendering)
    renWin.SetAlphaBitPlanes(True)
    renWin.SetMultiSamples(0)
    renWin.AddRenderer(rendererView1)
    renWin.AddRenderer(rendererView2)
    renWin.SetSize(windowSize[0],windowSize[1])

    return renWin

def OneViewRenderWindow(rendererView1,windowSize = [1800,800],
                        OffScreenRendering = True):

    renWin = vtk.vtkRenderWindow()
    renWin.SetOffScreenRendering(OffScreenRendering)
    renWin.SetAlphaBitPlanes(True)
    renWin.SetMultiSamples(0)
    renWin.AddRenderer(rendererView1)
    renWin.SetSize(windowSize[0],windowSize[1])

    return renWin
#%% Lights

#%% Text

def TimeStampActor(tStep):
    #Very hard coded at the moment
    
    TextActor = vtk.vtkTextActor()
    TextActor.SetInput('%04d ms'%(tStep))
    TextActor.SetPosition(.10,.9)
    TextActor.GetTextProperty().SetFontSize(24)
    TextActor.GetTextProperty().SetColor(0.,0.,0.)

    return TextActor

#%% Colorbars and colormaps
def FrontiersColormap(tableSize):
    '''
    Use a color transfer Function to generate the colors in the lookup table.
    See: http://www.vtk.org/doc/nightly/html/classvtkColorTransferFunction.html
    :param: tableSize - The table size
    :return: The lookup table.
    '''
    ctf = vtk.vtkColorTransferFunction()
    ctf.SetColorSpaceToDiverging()

    # Green to tan.
    ctf.AddRGBPoint(0.0,  4.00000000e-01,  1.09803922e-01,  8.62745098e-02)
    ctf.AddRGBPoint(0.1,  6.03921569e-01,  1.13725490e-01,  6.27450980e-02)
    ctf.AddRGBPoint(0.2,  7.76470588e-01,  9.80392157e-02,  3.52941176e-02)
    ctf.AddRGBPoint(0.3,  9.33333333e-01,  9.01960784e-02,  7.84313725e-03)
    ctf.AddRGBPoint(0.4,  1.00000000e+00,  2.07843137e-01,  1.56862745e-02)
    ctf.AddRGBPoint(0.5,  1.00000000e+00,  3.60784314e-01,  4.31372549e-02)
    ctf.AddRGBPoint(0.6,  1.00000000e+00,  5.45098039e-01,  7.05882353e-02)
    ctf.AddRGBPoint(0.7,  1.00000000e+00,  5.45098039e-01,  7.05882353e-02)
    ctf.AddRGBPoint(0.8,  1.00000000e+00,  7.05882353e-01,  1.01960784e-01)
    ctf.AddRGBPoint(0.9,  1.00000000e+00,  9.09803922e-01,  1.33333333e-01)
    ctf.AddRGBPoint(1.0,  1.00000000e+00,  9.37254902e-01,  1.41176471e-01)	



    lut = vtk.vtkLookupTable()
    lut.SetNumberOfTableValues(tableSize)
    lut.Build()

    for i in range(0,tableSize):
        rgb = list(ctf.GetColor(float(i)/tableSize))+[1]
        lut.SetTableValue(i,rgb)

    return lut

def HeatmapColormap(tableSize):
    '''
    Use a color transfer Function to generate the colors in the lookup table.
    See: http://www.vtk.org/doc/nightly/html/classvtkColorTransferFunction.html
    :param: tableSize - The table size
    :return: The lookup table.
    '''
    ctf = vtk.vtkColorTransferFunction()
    ctf.SetColorSpaceToDiverging()

    # Green to tan.
    N = 7
    # ctf.AddRGBPoint(0.0,  0.101961, 0.101961, 0.101961) 
    # ctf.AddRGBPoint(1/N,  0.227451, 0.227451, 0.227451)
    # ctf.AddRGBPoint(2/N,  0.359939, 0.359939, 0.359939)
    ctf.AddRGBPoint(0,  0.8, 0.8, 0.8)
    # ctf.AddRGBPoint(1/N,  0.631373, 0.631373, 0.631373)
    # ctf.AddRGBPoint(2/N,  0.749865, 0.749865, 0.749865)
    # ctf.AddRGBPoint(3/N,  0.843368, 0.843368, 0.843368)
    ctf.AddRGBPoint(0.1+1/N,  0.960323, 0.66782, 0.536332)
    ctf.AddRGBPoint(0.1+2/N,  0.894579, 0.503806, 0.399769)
    ctf.AddRGBPoint(0.1+3/N,  0.81707, 0.33218, 0.281046)
    ctf.AddRGBPoint(0.1+4/N,  0.728489, 0.155017, 0.197386)	
    ctf.AddRGBPoint(0.1+5/N,  0.576932, 0.055363, 0.14925)	
    ctf.AddRGBPoint(0.1+6/N,  0.403922, 0, 0.121569)	
    ctf.AddRGBPoint(0.1+7/N,  1.,0.,0.)	



    lut = vtk.vtkLookupTable()
    lut.SetNumberOfTableValues(tableSize)
    lut.Build()

    for i in range(0,tableSize):
        rgb = list(ctf.GetColor(float(i)/tableSize))+[1]
        lut.SetTableValue(i,rgb)
    lut.SetNanColor([.7,.7,.7,1])
    return lut

def EGMColormap(tableSize):
    '''
    Use a color transfer Function to generate the colors in the lookup table.
    See: http://www.vtk.org/doc/nightly/html/classvtkColorTransferFunction.html
    :param: tableSize - The table size
    :return: The lookup table.
    '''
    # colorSeries = vtk.vtkColorSeries()
    # colorSeries.SetColorScheme(vtk.vtkColorSeries.BREWER_DIVERGING_PURPLE_ORANGE_3 )
    # lut = vtk.vtkLookupTable()
    # colorSeries.BuildLookupTable(lut, vtk.vtkColorSeries.ORDINAL)
    ctf = vtk.vtkColorTransferFunction()
    ctf.SetColorSpaceToDiverging()
 
    ctf.AddRGBPoint(0.0,  0.231373,  0.298039,  0.752941)
    ctf.AddRGBPoint(0.5,  0.865003,  0.865003,  0.865003)
    ctf.AddRGBPoint(1,  0.705882,  0.0156863,  0.14902)

    lut = vtk.vtkLookupTable()
    lut.SetNumberOfTableValues(tableSize)
    lut.Build()

    for i in range(0,tableSize):
        rgb = list(ctf.GetColor(float(i)/tableSize))+[1]
        lut.SetTableValue(i,rgb)    

    return lut

def PhaseColormap(tableSize):
    ctf = vtk.vtkColorTransferFunction()
    ctf.SetColorSpaceToRGB()
   
    ctf.AddRGBPoint(0.0  ,0  ,0,0)
    ctf.AddRGBPoint(0.1  ,0.7  ,0.7,0.7)
    '''
    ctf.AddRGBPoint(0.01  ,1.  ,0.3,0)
    ctf.AddRGBPoint(0.2 ,1.,	1.   ,0)
    ctf.AddRGBPoint(0.4 ,0.,	1.   ,0)
    ctf.AddRGBPoint(0.6 ,0.,	1.   ,1)
    ctf.AddRGBPoint(0.8 ,0.,	0.  ,0.6)
    ctf.AddRGBPoint(0.99  ,1.,	0.   ,1.) 
    '''
    ctf.AddRGBPoint(0.9  ,0.7  ,0.7,0.7)
    ctf.AddRGBPoint(1.  ,1.,	0.   ,1.) 


    lut = vtk.vtkLookupTable()
    lut.SetNumberOfTableValues(tableSize)
    lut.Build()

    for i in range(0,tableSize):
        rgb = list(ctf.GetColor(float(i)/tableSize))+[1]
        lut.SetTableValue(i,rgb)    

    return lut

def JetColormap(tableSize):
    ctf = vtk.vtkColorTransferFunction()
    ctf.SetColorSpaceToRGB()
    

    for i in range(256):
        i4=4*i/256
        r = np.min([np.max([np.min([i4-1.5,-i4+4.5]),0]),1])
        g = np.min([np.max([np.min([i4-0.5,-i4+3.5]),0]),1])
        b = np.min([np.max([np.min([i4+0.5,-i4+2.5]),0]),1])
        ctf.AddRGBPoint(i/255 ,r  ,g,b)


    lut = vtk.vtkLookupTable()
    lut.SetNumberOfTableValues(tableSize)
    lut.Build()

    for i in range(0,tableSize):
        rgb = list(ctf.GetColor(float(i)/tableSize))+[1]
        lut.SetTableValue(i,rgb)    

    return lut

def ViridisColormap(tableSize):
    ctf = vtk.vtkColorTransferFunction()
    ctf.SetColorSpaceToRGB()
   
    colormap = np.array([#[255,255,255],
                        [68, 1, 84],
                        [70, 48, 124],
                        [55, 89, 140],
                        [40,124,142],
                        [36,156,135],
                        [78,193,107],
                        [158,217,58],
                        [253, 231, 37]],dtype=float)/256
    
    proportion = np.linspace(0,1.,len(colormap))#np.ones(len(colormap),dtype=float)
    # proportion[0] = 0
    for i in range(len(colormap)):
        ctf.AddRGBPoint(proportion[i],
                        colormap[i,0],
                        colormap[i,1],
                        colormap[i,2])


    lut = vtk.vtkLookupTable()
    lut.SetNumberOfTableValues(tableSize)
    

    for i in range(0,tableSize):
        rgb = list(ctf.GetColor(float(i)/tableSize))+[1]
        lut.SetTableValue(i,rgb)    
    lut.SetNanColor([.7,.7,.7,1])
    lut.Build()
    return lut

def ColorbarActor(atrialMapper,title = 'Vm (mV)'):    
    colorBar = vtk.vtkScalarBarActor()
    colorBar.SetLookupTable(atrialMapper.GetLookupTable())
 
    colorBar.SetTitle(title)
    colorBar.SetVerticalTitleSeparation(5)
    colorBar.GetTitleTextProperty().SetColor([0,0,0])
    #colorBar.GetTitleTextProperty().SetOrientation(0)

    colorBar.GetLabelTextProperty().SetColor([0,0,0])
    colorBar.GetLabelTextProperty().BoldOff()
    colorBar.GetLabelTextProperty().ItalicOff()
    colorBar.GetLabelTextProperty().ShadowOff()

    colorBar.GetPositionCoordinate().SetCoordinateSystemToNormalizedViewport()
    colorBar.GetPositionCoordinate().SetValue(0.85,0.01)
    colorBar.SetWidth(0.1)
    colorBar.SetHeight(0.5)

    return colorBar

