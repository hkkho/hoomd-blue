// Copyright (c) 2009-2016 The Regents of the University of Michigan
// This file is part of the HOOMD-blue project, released under the BSD 3-Clause License.


// Maintainer: joaander

/*! \file HOOMDDumpWriter.h
    \brief Declares the HOOMDDumpWriter class
*/

#ifdef NVCC
#error This header cannot be compiled by nvcc
#endif

#include "Analyzer.h"

#include <string>

#include <boost/shared_ptr.hpp>

#ifndef __HOOMD_DUMP_WRITER_H__
#define __HOOMD_DUMP_WRITER_H__

//! Analyzer for writing out HOOMD  dump files
/*! HOOMDDumpWriter can be used to write out xml files containing various levels of information
    of the current time step of the simulation. At a minimum, the current time step and box
    dimensions are output. Optionally, particle positions, velocities and types can be included
    in the file.

    Usage:<br>
    Construct a HOOMDDumpWriter, attaching it to a ParticleData and specifying a base file name.
    Call analyze(timestep) to output a dump file with the state of the current time step
    of the simulation. It will create base_file.timestep.xml where timestep is a 0-padded
    10 digit number. The 0 padding is so files sorted "alphabetically" will be read in
    numberical order.

    To include positions, velocities and types, see: setOutputPosition() setOutputVelocity()
    and setOutputType(). Similarly, bonds can be included with setOutputBond().

    Future versions will include the ability to dump forces on each particle to the file also.

    For information on the structure of the xml file format: see \ref page_dev_info
    Although, HOOMD's  user guide probably has a more up to date documentation on the format.
    \ingroup analyzers
*/
class HOOMDDumpWriter : public Analyzer
    {
    public:
        //! Construct the writer
        HOOMDDumpWriter(boost::shared_ptr<SystemDefinition> sysdef, std::string base_fname, bool mode_restart=false);

        //! Destructor
        ~HOOMDDumpWriter();

        //! Write out the data for the current timestep
        void analyze(unsigned int timestep);
        //! Enables/disables the writing of the particle positions
        void setOutputPosition(bool enable);
        //! Enables/disables the writing of particle images
        void setOutputImage(bool enable);
        //! Enables/disables the writing of particle velocities
        void setOutputVelocity(bool enable);
        //! Enables/disables the writing of particle masses
        void setOutputMass(bool enable);
        //! Enables/disables the writing of particle diameters
        void setOutputDiameter(bool enable);
        //! Enables/disables the writing of particle types
        void setOutputType(bool enable);
        //! Enables/disables the writing of bonds
        void setOutputBond(bool enable);
        //! Enables/disables the writing of angles
        void setOutputAngle(bool enable);
        //! Enables/disables the writing of dihedrals
        void setOutputDihedral(bool enable);
        //! Enables/disables the writing of impropers
        void setOutputImproper(bool enable);
        //! Enables/disables the writing of constraints
        void setOutputConstraint(bool enable);
        //! Enables/disables the writing of acceleration
        void setOutputAccel(bool enable);
        //! Enables/disables the writing of body
        void setOutputBody(bool enable);
        //! Enables/disables the writing of charge
        void setOutputCharge(bool enable);
        //! Enables/disables the writing of orientation
        void setOutputOrientation(bool enable);
        //! Enables/disables the writing of angular momentum
        void setOutputAngularMomentum(bool enable);
        //! Enables/disables the writing of moment_inertia
        void setOutputMomentInertia(bool enable);
        //! Sets the vizsigma value to write
        void setVizSigma(Scalar vizsigma)
            {
            m_vizsigma = vizsigma;
            m_vizsigma_set = true;
            }

        //! Writes a file at the current time step
        void writeFile(std::string fname, unsigned int timestep);
    private:
        std::string m_base_fname;   //!< String used to store the file name of the XML file
        bool m_output_position;     //!< true if the particle positions should be written
        bool m_output_image;        //!< true if the particle positions should be written
        bool m_output_velocity;     //!< true if the particle velocities should be written
        bool m_output_mass;         //!< true if the particle masses should be written
        bool m_output_diameter;     //!< true if the particle diameters should be written
        bool m_output_type;         //!< true if the particle types should be written
        bool m_output_bond;         //!< true if the bonds should be written
        bool m_output_angle;        //!< true if the angles should be written
        bool m_output_dihedral;     //!< true if dihedrals should be written
        bool m_output_improper;     //!< true if impropers should be written
        bool m_output_constraint;   //!< true if constraints shoulds be written
        bool m_output_accel;        //!< true if acceleration should be written
        bool m_output_body;         //!< true if body should be written
        bool m_output_charge;       //!< true if body should be written
        bool m_output_orientation;  //!< true if orientation should be written
        bool m_output_angmom;       //!< true if angular momenta should be written
        bool m_output_moment_inertia;  //!< true if moment_inertia should be written
        Scalar m_vizsigma;          //!< vizsigma value to write out to xml files
        bool m_vizsigma_set;        //!< true if vizsigma has been set
        bool m_mode_restart;        //!< true if we are writing restart files
        };

//! Exports the HOOMDDumpWriter class to python
void export_HOOMDDumpWriter();

#endif