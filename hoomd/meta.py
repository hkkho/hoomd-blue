# Copyright (c) 2009-2019 The Regents of the University of Michigan
# This file is part of the HOOMD-blue project, released under the BSD 3-Clause License.

# Maintainer: joaander / All Developers are free to add commands for new features

R""" Write out simulation and environment context metadata.

Metadata is stored in form of key-value pairs in a JSON file and used
to summarize the per-run simulation parameters so that they can be easily
taken up by other scripts and stored in a database.

Example::

    metadata = meta.dump_metadata()
    meta.dump_metadata(filename = "metadata.json", user = {'debug': True}, indent=2)

"""

import hoomd
import json
import collections
import time
import datetime
import copy
import warnings

def track(func):
    """Decorator for any method whose calls must be tracked in order to
    completely reproduce a simulation. Assumes input is a class method.
    """
    ###TODO: MAKE SURE THAT FORCE CLASSES UPDATE COEFFS BEFORE GETTING METADATA
    if not hasattr(func, call_history):
        func.call_history = []
    def tracked_func(self, *args, **kwargs):
        func.call_history.append({
            'args': args,
            'kwargs': kwargs
            })
        return func(self, *args, **kwargs)
    return tracked_func

## \internal
# \brief A Mixin to facilitate storage of simulation metadata
class _metadata(object):
    R"""Track the metadata of all subclasses.

    The goal of metadata tracking is to make it possible to completely
    reproduce the exact simulation protocol in a script. In order to do so,
    every class that needs to be tracked (system data, neighbor lists,
    integrators, pair potentials, etc) should all inherit from _metadata. The
    class tracks information in three ways:

    #. All constructor arguments are automatically tracked.
    #. Any method decorated with the @track decorator will be recorded whenever called.
    #. Every class may define a class level variable `metadata_fields` that indicates class variables (maybe, haven't decided yet).

    The constructor tracking is accomplished by overriding the __new__ method
    to automatically store all constructor arguments. Similarly, all methods
    decorated with the @track decorator will automatically log their inputs.
    """

    def __new__(cls, *args, **kwargs):
        obj = super(_metadata, cls).__new__(cls)
        obj.args = args
        obj.kwargs = kwargs
        obj.metadata_fields = []
        return obj

    ## \internal
    # \brief Return the metadata
    def get_metadata(self):
        varnames = self.__init__.__code__.co_varnames[1:]  # Skip `self`

        # Fill in positional arguments first, then update all kwargs. No need
        # for extensive error checking since the function signature must have
        # been valid to construct the object in the first place.
        metadata = collections.OrderedDict()
        for varname, arg in zip(varnames, self.args):
            metadata[varname] = arg
        metadata.update(self.kwargs)

        tracked_fields = {}
        for field in self.metadata_fields:
            tracked_fields[field] = getattr(self, field)
        metadata['_tracked_field'] = tracked_fields
        return metadata

# TODO: Maybe include a subclass that calls set_params on a set of special parameters. More generally, that allows any set* functions to be tracked... now I'm going back to what I was doing before.

class _metadata_from_dict(object):
    def __init__(self, d):
        self.d = d

    def get_metadata(self):
        data = collections.OrderedDict()

        for m in self.d.keys():
            data[m] = self.d[m]

        return data

def dump_metadata(filename=None, user=None, indent=4):
    R""" Writes simulation metadata into a file.

    Args:
        filename (str): The name of the file to write JSON metadata to (optional)
        user (dict): Additional metadata.
        indent (int): The json indentation size

    Returns:
        dict: The metadata

    When called, this function will query all registered forces, updaters etc.
    and ask them to provide metadata. E.g. a pair potential will return
    information about parameters, the Logger will output the filename it is
    logging to, etc.

    Custom metadata can be provided as a dictionary to *user*.

    The output is aggregated into a dictionary and written to a
    JSON file, together with a timestamp. The file is overwritten if
    it exists.
    """
    hoomd.util.print_status_line()

    if not hoomd.init.is_initialized():
        hoomd.context.msg.error("Need to initialize system first.\n")
        raise RuntimeError("Error writing out metadata.")

    metadata = dict()

    global_objs = [hoomd.context.current.integrator]
    global_objs += hoomd.context.current.integration_methods
    global_objs += hoomd.context.current.forces
    global_objs += hoomd.context.current.analyzers
    global_objs += hoomd.context.current.updaters
    global_objs += hoomd.context.current.constraint_forces

    to_name = lambda obj: obj.__module__ + '.' + obj.__class__.__name__

    def to_metadata(obj, top=False):
        """Convert object to metadata. At all but the top level, we return a
        mapping of object name->metadata."""
        meta = obj.get_metadata()
        for k, v in meta.items():
            if hasattr(v, 'get_metadata'):
                meta[k] = to_metadata(v)
        return meta if top else {to_name(obj): meta}

    # First put all classes into a set to avoid saving duplicates, then go back
    # through the set and convert to a list of metadata representations of the
    # objects.
    for o in global_objs:
        if o is not None:
            name = to_name(o)
            metadata.setdefault(name, set())
            assert isinstance(metadata[name], set)
            metadata[name].add(o)
    for key, values in metadata.items():
        metadata[key] = [to_metadata(v, True) for v in values]

    # Add additional configuration info, including user provided quantities.
    ts = time.time()
    st = datetime.datetime.fromtimestamp(ts).strftime('%Y-%m-%d %H:%M:%S')
    metadata['timestamp'] = st
    metadata['context'] = hoomd.context.ExecutionContext().get_metadata()
    metadata['hoomd'] = hoomd.context.HOOMDContext().get_metadata()

    if user is not None:
        if not isinstance(user, collections.Mapping):
            hoomd.context.msg.warning("Extra meta data needs to be a mapping type. Ignoring.\n")
        else:
            metadata['user'] = _metadata_from_dict(user)

    # Only write files on rank 0
    if filename is not None and hoomd.comm.get_rank() == 0:
        with open(filename, 'w') as file:
            meta_str = json.dumps(
                metadata, indent=indent, sort_keys=True)
            file.write(meta_str)
    return metadata

def load_metadata(system, metadata=None, filename=None):
    R"""Initialize system information from metadata.

    This function must be called after the system is initialized, but before
    any other HOOMD functions are called. This function will update the system
    data with any bonds, constraints, etc that are encoded in the metadata.
    Additionally, it will instantiate any pair potentials, forces, etc that
    need to be created. All of the created objects will be returned to the
    user, who can modify them or create new objects as necessary.

    Args:
        system (:py:class:`hoomd.data.system_data`): The initial system.
        metadata (dict): The metadata to initialize with. Defaults to None, but
                         must be provided unless a filename is given.
        filename (str): A file containing metadata. Is ignored if a metadata
                        dictionary is provided.

    Returns:
        dict: A mapping from class to the instance created by this function.
    """
    # For now, only use filename if no metadata is given.
    if filename is not None:
        if metadata is None:
            with open(filename) as f:
                metadata = json.load(f)
        else:
            warnings.warn(
                "Both filename and data specified. Ignoring provided file.")
    elif metadata is None:
        raise RuntimeError(
            "You must provide either a dictionary with metadata or a file to "
            "read from.")

    # Ignored keys are those we don't need to do anything with
    ignored_keys = ['timestamp', 'context', 'hoomd']
    objects = {}
    for key, vals in metadata.items():
        if key in ignored_keys:
            continue

        # Top level is always hoomd, but may be removed
        parts = key.split('.')
        if parts[0] == 'hoomd':
            parts.pop(0)
        obj = hoomd
        while parts:
            obj = getattr(obj, parts.pop(0))

        if obj.__name__ == "system_data":
            # System data is the special case that needs the actual system
            # object to be passed through as well.
            instance = obj.from_metadata(vals, system)
        else:
            instance = obj.from_metadata(vals)

        name = obj.__module__ + '.' + obj.__class__.__name__
        objects[name] = instance
    return objects
