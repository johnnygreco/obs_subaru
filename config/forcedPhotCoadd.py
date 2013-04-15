# Add another aperture flux - it shouldn't be this painful to do (see LSST #2465)
from lsst.meas.algorithms.algorithmRegistry import AlgorithmRegistry, SincFluxConfig
AlgorithmRegistry.register("flux.sinc2", target=SincFluxConfig.Control, ConfigClass=SincFluxConfig)
root.measurement.algorithms["flux.sinc2"].radius = 5.0
root.measurement.algorithms.names |= ["flux.sinc2"]

# Enable model mags, disable Gaussian mags (which aren't really forced)
root.measurement.algorithms.names -= ["flux.gaussian"]
root.measurement.slots.instFlux = None
try:
    import lsst.meas.extensions.multiShapelet
    root.measurement.algorithms.names |= lsst.meas.extensions.multiShapelet.algorithms
    root.measurement.slots.modelFlux = "multishapelet.combo.flux"
except ImportError:
    print "meas_extensions_multiShapelet is not setup; disabling model mags"
