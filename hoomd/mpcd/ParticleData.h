// Copyright (c) 2009-2017 The Regents of the University of Michigan
// This file is part of the HOOMD-blue project, released under the BSD 3-Clause License.

// Maintainer: mphoward

#ifndef MPCD_PARTICLE_DATA_H_
#define MPCD_PARTICLE_DATA_H_

/*!
 * \file mpcd/ParticleData.h
 * \brief Declaration of mpcd::ParticleData
 */

#ifdef NVCC
#error This header cannot be compiled by nvcc
#endif

#include "ParticleDataSnapshot.h"
#include "ParticleDataUtilities.h"

#ifdef ENABLE_CUDA
#include "ParticleData.cuh"
#endif // ENABLE_CUDA

#include "hoomd/BoxDim.h"
#include "hoomd/DomainDecomposition.h"
#include "hoomd/ExecutionConfiguration.h"
#include "hoomd/GPUArray.h"
#include "hoomd/GPUVector.h"
#include "hoomd/Profiler.h"

// pybind11
#include "hoomd/extern/pybind/include/pybind11/pybind11.h"

namespace mpcd
{
//! Stores MPCD particle data
/*!
 * MPCD particles are characterized by position, velocity, and mass. We assume all
 * particles have the same mass. The data is laid out as follows:
 * - position + type in array of Scalar4
 * - velocity + cell index in array of Scalar4
 * - tag in array of unsigned int
 *
 * Unlike the standard ParticleData, a reverse tag mapping is not currently maintained
 * in order to save local memory. (That is, it is possible to read the tag of a local particle,
 * but it is not possible to efficiently find the local particle that has a given
 * tag.)
 *
 * The MPCD cell index is stored with the velocity because most MPCD operations
 * are based on around the velocity and cell. For details of what the cell means,
 * refer to the mpcd::CellList.
 *
 * \todo Because the local cell index changes with position, a signal will be put
 * in place to indicate when the cached cell index is still valid.
 *
 * \todo Likewise, MPCD benefits from sorting data into cell order, so a signal
 * needs to be put in place when the ordering changes.
 *
 * \todo Likewise, a signal should be incorporated to indicate when particles are
 * added or removed locally, as is the case during particle migration.
 *
 * \ingroup data_structs
 */
class ParticleData
    {
    public:
        //! Number constructor
        ParticleData(unsigned int N,
                     unsigned int n_types,
                     const BoxDim& global_box,
                     std::shared_ptr<ExecutionConfiguration> exec_conf,
                     std::shared_ptr<DomainDecomposition> decomposition = std::shared_ptr<DomainDecomposition>());

        //! Snapshot constructor
        ParticleData(mpcd::ParticleDataSnapshot& snapshot,
                     const BoxDim& global_box,
                     std::shared_ptr<const ExecutionConfiguration> exec_conf,
                     std::shared_ptr<DomainDecomposition> decomposition = std::shared_ptr<DomainDecomposition>());

        //! Destructor
        ~ParticleData();

        //! Initialize the MPCD particle data from a snapshot
        void initializeFromSnapshot(const mpcd::ParticleDataSnapshot& snapshot,
                                    const BoxDim& global_box);

        //! Take a snapshot of the MPCD particle data
        void takeSnapshot(mpcd::ParticleDataSnapshot& snapshot, const BoxDim& global_box) const;

        //! \name accessor methods
        //@{
        //! Get number of MPCD particles on the rank
        unsigned int getN() const
            {
            return m_N;
            }

        //! Get global number of MPCD particles
        unsigned int getNGlobal() const
            {
            return m_N_global;
            }

        //! Get number of MPCD particle types
        unsigned int getNTypes() const
            {
            return m_type_mapping.size();
            }

        //! Get the type-name mapping
        const std::vector<std::string>& getTypeNames() const
            {
            return m_type_mapping;
            }

        //! Get the type index by its name
        unsigned int getTypeByName(const std::string& name) const;

        //! Get the name of a type by its index
        std::string getNameByType(unsigned int type) const;

        //! Get array of MPCD particle positions
        const GPUArray<Scalar4>& getPositions() const
            {
            return m_pos;
            }

        //! Get array of MPCD particle velocities
        const GPUArray<Scalar4>& getVelocities() const
            {
            return m_vel;
            }

        //! Get array of MPCD particle tags
        const GPUArray<unsigned int>& getTags() const
            {
            return m_tag;
            }

        //! Get particle mass
        Scalar getMass() const
            {
            return m_mass;
            }

        //! Set particle mass
        void setMass(Scalar mass);

        //! Get the position of the particle on the local rank
        Scalar3 getPosition(unsigned int idx) const;

        //! Get the type of the particle on the local rank
        unsigned int getType(unsigned int idx) const;

        //! Get the velocity of the particle on the local rank
        Scalar3 getVelocity(unsigned int idx) const;

        //! Get the tag of the particle on the local rank
        unsigned int getTag(unsigned int idx) const;

        //! Set the profiler for the particle data to use
        void setProfiler(std::shared_ptr<Profiler> prof)
            {
            m_prof = prof;
            }
        //@}

        //! \name swap methods
        //@{
        //! Get alternate array of MPCD particle positions
        const GPUArray<Scalar4>& getAltPositions() const
            {
            return m_pos_alt;
            }

        //! Swap out alternate MPCD particle position array
        void swapPositions()
            {
            m_pos.swap(m_pos_alt);
            }

        //! Get alternate array of MPCD particle velocities
        const GPUArray<Scalar4>& getAltVelocities() const
            {
            return m_vel_alt;
            }

        //! Swap out alternate MPCD particle velocity array
        void swapVelocities()
            {
            m_vel.swap(m_vel_alt);
            }

        //! Get alternate array of MPCD particle tags
        const GPUArray<unsigned int>& getAltTags() const
            {
            return m_tag_alt;
            }

        //! Swap out alternate MPCD particle tags
        void swapTags()
            {
            m_tag.swap(m_tag_alt);
            }
        //@}

        #ifdef ENABLE_MPI
        //! \name communication methods
        //@{

        //! Pack particle data into a buffer
        void removeParticles(std::vector<mpcd::detail::pdata_element>& out);

        //! Add new local particles
        void addParticles(const std::vector<mpcd::detail::pdata_element>& in);

        #ifdef ENABLE_CUDA
        //! Pack particle data into a buffer (GPU version)
        void removeParticlesGPU(GPUVector<mpcd::detail::pdata_element>& out, GPUVector<unsigned int>& comm_flags);

        //! Add new local particles (GPU version)
        void addParticlesGPU(const GPUVector<mpcd::detail::pdata_element>& in);
        #endif // ENABLE_CUDA

        //! Get the MPCD particle communication flags
        const GPUArray<unsigned int>& getCommFlags() const
            {
            return m_comm_flags;
            }

        //@}
        #endif // ENABLE_MPI

    private:
        unsigned int m_N;           //!< Number of MPCD particles
        unsigned int m_N_global;    //!< Total number of MPCD particles
        unsigned int m_N_max;       //!< Maximum number of MPCD particles arrays can hold

        std::shared_ptr<const ExecutionConfiguration> m_exec_conf;  //!< GPU execution configuration
        std::shared_ptr<DomainDecomposition> m_decomposition;       //!< Domain decomposition
        std::shared_ptr<Profiler> m_prof;                           //!< Profiler
        #ifdef ENABLE_CUDA
        mgpu::ContextPtr m_mgpu_context;             //!< moderngpu context
        #endif

        GPUArray<Scalar4> m_pos;    //!< MPCD particle positions plus type
        GPUArray<Scalar4> m_vel;    //!< MPCD particle velocities plus cell list id
        Scalar m_mass;              //!< MPCD particle mass
        GPUArray<unsigned int> m_tag;   //!< MPCD particle tags
        std::vector<std::string> m_type_mapping;  //!< Type name mapping
        #ifdef ENABLE_MPI
        GPUArray<unsigned int> m_comm_flags;    //!< MPCD particle communication flags
        #endif // ENABLE_MPI

        GPUArray<Scalar4> m_pos_alt;        //!< Alternate position array
        GPUArray<Scalar4> m_vel_alt;        //!< Alternate velocity array
        GPUArray<unsigned int> m_tag_alt;   //!< Alternate tag array

        //! Check for a valid snapshot
        bool checkSnapshot(const mpcd::ParticleDataSnapshot& snapshot);

        //! Check if all particles lie within the box
        bool checkInBox(const mpcd::ParticleDataSnapshot& snapshot, const BoxDim& box);

        //! Set the global number of particles (for parallel simulations)
        void setNGlobal(unsigned int nglobal);

        //! Allocate data arrays
        void allocate(unsigned int N_max);

        //! Reallocate data arrays
        void reallocate(unsigned int N_max);

        const static float resize_factor; //!< Amortized growth factor the data arrays
        //! Resize the data
        void resize(unsigned int N);
    };

namespace detail
{
//! Export MPCD ParticleData to python
void export_ParticleData(pybind11::module& m);
} // end namespace detail

} // end namespace mpcd

#endif // MPCD_PARTICLE_DATA_H_
