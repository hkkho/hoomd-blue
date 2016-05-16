// Copyright (c) 2009-2016 The Regents of the University of Michigan
// This file is part of the HOOMD-blue project, released under the BSD 3-Clause License.


// Maintainer: joaander

/*! \file BoxResizeUpdater.h
    \brief Declares an updater that resizes the simulation box of the system
*/

#ifdef NVCC
#error This header cannot be compiled by nvcc
#endif

#include "Updater.h"
#include "Variant.h"

#include <boost/shared_ptr.hpp>

#ifndef __BOXRESIZEUPDATER_H__
#define __BOXRESIZEUPDATER_H__

//! Updates the simulation box over time
/*! This simple updater gets the box lengths from specified variants and sets those box sizes
    over time. As an option, particles can be rescaled with the box lengths or left where they are.

    \ingroup updaters
*/
class BoxResizeUpdater : public Updater
    {
    public:
        //! Constructor
        BoxResizeUpdater(boost::shared_ptr<SystemDefinition> sysdef,
                         boost::shared_ptr<Variant> Lx,
                         boost::shared_ptr<Variant> Ly,
                         boost::shared_ptr<Variant> Lz,
                         boost::shared_ptr<Variant> xy,
                         boost::shared_ptr<Variant> xz,
                         boost::shared_ptr<Variant> yz);

        //! Destructor
        virtual ~BoxResizeUpdater();

        //! Sets parameter flags
        void setParams(bool scale_particles);

        //! Take one timestep forward
        virtual void update(unsigned int timestep);

    private:
        boost::shared_ptr<Variant> m_Lx;    //!< Box Lx vs time
        boost::shared_ptr<Variant> m_Ly;    //!< Box Ly vs time
        boost::shared_ptr<Variant> m_Lz;    //!< Box Lz vs time
        boost::shared_ptr<Variant> m_xy;    //!< Box xy tilt factor vs time
        boost::shared_ptr<Variant> m_xz;    //!< Box xz tilt factor vs time
        boost::shared_ptr<Variant> m_yz;    //!< Box yz tilt factor vs time
        bool m_scale_particles;                //!< Set to true if particle positions are to be scaled as well
    };

//! Export the BoxResizeUpdater to python
void export_BoxResizeUpdater();

#endif