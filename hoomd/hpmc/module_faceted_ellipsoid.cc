// Copyright (c) 2009-2019 The Regents of the University of Michigan
// This file is part of the HOOMD-blue project, released under the BSD 3-Clause License.

// Include the defined classes that are to be exported to python
#include "IntegratorHPMC.h"
#include "IntegratorHPMCMono.h"
#include "ComputeFreeVolume.h"

#include "ShapeFacetedEllipsoid.h"
#include "AnalyzerSDF.h"

#include "ExternalField.h"
#include "ExternalFieldWall.h"
#include "ExternalFieldLattice.h"
#include "ExternalFieldComposite.h"
#include "ExternalCallback.h"

#include "UpdaterExternalFieldWall.h"
#include "UpdaterRemoveDrift.h"
#include "UpdaterMuVT.h"
#include "UpdaterClusters.h"

#ifdef ENABLE_HIP
#include "IntegratorHPMCMonoGPU.h"
#include "ComputeFreeVolumeGPU.h"
#endif




namespace py = pybind11;
using namespace hpmc;

using namespace hpmc::detail;

namespace hpmc
{

//! Export the base HPMCMono integrators
void export_faceted_ellipsoid(py::module& m)
    {
    export_IntegratorHPMCMono< ShapeFacetedEllipsoid >(m, "IntegratorHPMCMonoFacetedEllipsoid");
    export_ComputeFreeVolume< ShapeFacetedEllipsoid >(m, "ComputeFreeVolumeFacetedEllipsoid");
    export_AnalyzerSDF< ShapeFacetedEllipsoid >(m, "AnalyzerSDFFacetedEllipsoid");
    export_UpdaterMuVT< ShapeFacetedEllipsoid >(m, "UpdaterMuVTFacetedEllipsoid");
    export_UpdaterClusters< ShapeFacetedEllipsoid >(m, "UpdaterClustersFacetedEllipsoid");

    export_ExternalFieldInterface<ShapeFacetedEllipsoid>(m, "ExternalFieldFacetedEllipsoid");
    export_LatticeField<ShapeFacetedEllipsoid>(m, "ExternalFieldLatticeFacetedEllipsoid");
    export_ExternalFieldComposite<ShapeFacetedEllipsoid>(m, "ExternalFieldCompositeFacetedEllipsoid");
    export_RemoveDriftUpdater<ShapeFacetedEllipsoid>(m, "RemoveDriftUpdaterFacetedEllipsoid");
    export_ExternalFieldWall<ShapeFacetedEllipsoid>(m, "WallFacetedEllipsoid");
    export_UpdaterExternalFieldWall<ShapeFacetedEllipsoid>(m, "UpdaterExternalFieldWallFacetedEllipsoid");
    export_ExternalCallback<ShapeFacetedEllipsoid>(m, "ExternalCallbackFacetedEllipsoid");

    #ifdef ENABLE_HIP
    export_IntegratorHPMCMonoGPU< ShapeFacetedEllipsoid >(m, "IntegratorHPMCMonoFacetedEllipsoidGPU");
    export_ComputeFreeVolumeGPU< ShapeFacetedEllipsoid >(m, "ComputeFreeVolumeFacetedEllipsoidGPU");
    #endif
    }

}
