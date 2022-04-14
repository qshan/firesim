.. _manager-configuration-files:

Manager Configuration Files
===============================

This page contains a centralized reference for all of the configuration options
in ``config_runtime.yaml``, ``config_build.yaml``, ``config_build_farm.yaml``,
``config_build_recipes.yaml``, and ``config_hwdb.yaml``.

.. _config-runtime:

``config_runtime.yaml``
--------------------------

Here is a sample of this configuration file:

.. include:: /../deploy/sample-backup-configs/sample_config_runtime.yaml
   :code: yaml

Below, we outline each section and parameter in detail.

``run_farm``
^^^^^^^^^^^^^^^^^^^

The ``run_farm`` options below allow you to specify the number, types, and
other characteristics of instances in your FireSim Run Farm, so that the
manager can automatically launch them, run workloads on them, and terminate
them.

``run_farm_tag``
""""""""""""""""

Use ``run_farm_tag`` to differentiate between different Run Farms in FireSim.
Having multiple ``config_runtime.yaml`` files with different ``run_farm_tag``
values allows you to run many experiments at once from the same manager instance.

The instances launched by the ``launchrunfarm`` command will be tagged with
this value. All later operations done by the manager rely on this tag, so
you should not change it unless you are done with your current Run Farm.

Per AWS restrictions, this tag can be no longer than 255 characters.

``always_expand_runfarm``
"""""""""""""""""""""""""
When ``yes`` (the default behavior when not given) the number of instances
of each type (see ``f1_16xlarges`` etc. below)  are launched every time you
run ``launchrunfarm``.

When ``no``, ``launchrunfarm`` looks for already existing instances that
match ``run_farm_tag`` and treat ``f1_16xlarges`` (and other 'instance-type'
values below) as a total count.

For example, if you have ``f1_2xlarges`` set to 100 and the first time you
run ``launchrunfarm`` you have ``launch_instances_timeout_minutes`` set to 0
(i.e. giveup after receiving a ``ClientError`` for each AvailabilityZone) and
AWS is only able to provide you 75 ``f1_2xlarges`` because of capacity issues,
``always_expand_runfarm`` changes the behavior of ``launchrunfarm`` in subsequent
attempts.  ``yes`` means ``launchrunfarm`` will try to launch 100 ``f1_2xlarges``
again.  ``no`` means that ``launchrunfarm`` will only try to launch an additional
25 ``f1_2xlarges`` because it will see that there are already 75 that have been launched
with the same ``run_farm_tag``.

``f1_16xlarges``, ``m4_16xlarges``, ``f1_4xlarges``, ``f1_2xlarges``
"""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""""

Set these values respectively based on the number and types of instances
you need. While we could automate this setting, we choose not to, so that
users are never surprised by how many instances they are running.

Note that these values are ONLY used to launch instances. After launch, the
manager will query the AWS API to find the instances of each type that have the
``run_farm_tag`` set above assigned to them.

Also refer to ``always_expand_runfarm`` which determines whether ``launchrunfarm``
treats these counts as an incremental amount to be launched every time it is envoked
or a total number of instances of that type and ``run_farm_tag`` that should be made
to exist.  Note, ``launchrunfarm`` will never terminate instances.

``launch_instances_timeout_minutes``
""""""""""""""""""""""""""""""""""""

Integer number of minutes that the ``launchrunfarm`` command will attempt to
request new instances before giving up.  This limit is used for each of the types
of instances being requested.  For example, if you set to 60,
and you are requesting all four types of instances, ``launchrunfarm`` will try
to launch each instance type for 60 minutes, possibly trying up to a total of
four hours.

This limit starts to be applied from the first time ``launchrunfarm`` receives a
``ClientError`` response in all AvailabilityZones (AZs) for your region.  In other words,
if you request more instances than can possibly be requested in the given limit but AWS
is able to satisfy all of the requests, the limit will not be enforced.

To experience the old (<= 1.12) behavior, set this limit to 0 and ``launchrunfarm``
will exit the first time it receives ``ClientError`` across all AZ's. The old behavior
is also the default if ``launch_instances_timeout_minutes`` is not included in the
``config_runtime.ini``.

``run_instance_market``
""""""""""""""""""""""""

You can specify either ``spot`` or ``ondemand`` here, to use one of those
markets on AWS.

``spot_interruption_behavior``
""""""""""""""""""""""""""""""

When ``run_instance_market: spot``, this value determines what happens to an instance
if it receives the interruption signal from AWS. You can specify either
``hibernate``, ``stop``, or ``terminate``.

``spot_max_price``
"""""""""""""""""""""""""""""

When ``run_instance_market: spot``, this value determines the max price you are
willing to pay per instance, in dollars. You can also set it to ``ondemand``
to set your max to the on-demand price for the instance.

``target_config``
^^^^^^^^^^^^^^^^^^^

The ``target_config`` options below allow you to specify the high-level
configuration of the target you are simulating. You can change these parameters
after launching a Run Farm (assuming you have the correct number of instances),
but in many cases you will need to re-run the ``infrasetup`` command to make
sure the correct simulation infrastructure is available on your instances.

``topology``
"""""""""""""""""""""""""""""

This field dictates the network topology of the simulated system. Some examples:

``no_net_config``: This runs N (see ``no_net_num_nodes`` below) independent
simulations, without a network simulation. You can currently only use this
option if you build one of the NoNIC hardware configs of FireSim.

``example_8config``: This requires a single ``f1.16xlarge``, which will
simulate 1 ToR switch attached to 8 simulated servers.

``example_16config``: This requires two ``f1.16xlarge`` instances and one
``m4.16xlarge`` instance, which will
simulate 2 ToR switches, each attached to 8 simulated servers, with the two
ToR switches connected by a root switch.

``example_64config``: This requires eight ``f1.16xlarge`` instances and one
``m4.16xlarge`` instance, which will simulate 8 ToR switches, each attached to
8 simulated servers (for a total of 64 nodes), with the eight ToR switches
connected by a root switch.

Additional configurations are available in ``deploy/runtools/user_topology.py``
and more can be added there. See the :ref:`usertopologies` section
for more info.

``no_net_num_nodes``
"""""""""""""""""""""""""""""

This determines the number of simulated nodes when you are using
``topology: no_net_config``.

``link_latency``
"""""""""""""""""

In a networked simulation, this allows you to specify the link latency of the
simulated network in CYCLES. For example, 6405 cycles is roughly 2 microseconds
at 3.2 GHz. A current limitation is that this value (in cycles) must be
a multiple of 7. Furthermore, you must not exceed the buffer size specified
in the NIC's simulation widget.

``switching_latency``
""""""""""""""""""""""

In a networked simulation, this specifies the minimum port-to-port switching
latency of the switch models, in CYCLES.

``net_bandwidth``
""""""""""""""""""""""

In a networked simulation, this specifies the maximum output bandwidth that a
NIC is allowed to produce as an integer in Gbit/s. Currently, this must be a
number between 1 and 200, allowing you to model NICs between 1 and 200 Gbit/s.

``profile_interval``
"""""""""""""""""""""""""""""

The simulation driver periodically samples performance counters in FASED timing model instances and dumps the result to a file on the host. ``profileinterval`` defines the number of target cycles between samples; setting this field to -1 disables polling.


``default_hwconfig``
"""""""""""""""""""""""""""""

This sets the server configuration launched by default in the above topologies.
Heterogeneous configurations can be achieved by manually specifying different
names within the topology itself, but all the ``example_Nconfig`` configurations
are homogeneous and use this value for all nodes.

You should set this to one of the hardware configurations you have defined already in
``config_hwdb.yaml``.  You should set this to the NAME (section title) of the
hardware configuration from ``config_hwdb.yaml``, NOT the actual agfi itself
(NOT something like ``agfi-XYZ...``).


``tracing``
^^^^^^^^^^^^^^^^^^^

This section manages TracerV-based tracing at simulation runtime. For more
details, see the :ref:`tracerv` page for more details.

``enable``
""""""""""""""""""

This turns tracing on, when set to ``yes`` and off when set to ``no``. See the :ref:`tracerv-enabling`.

``output_format``
""""""""""""""""""""

This sets the output format for TracerV tracing. See the :ref:`tracerv-output-format` section.

``selector``, ``start``, and ``end``
"""""""""""""""""""""""""""""""""""""

These configure triggering for TracerV. See the :ref:`tracerv-trigger` section.


``autocounter``
^^^^^^^^^^^^^^^^^^^^^

This section configures AutoCounter. See the :ref:`autocounter` page for more details.

``readrate``
"""""""""""""""""

This sets the rate at which AutoCounters are read. See the :ref:`autocounter-runtime-parameters` section for more details.


``workload``
^^^^^^^^^^^^^^^^^^^

This section defines the software that will run on the simulated system.

``workload_name``
"""""""""""""""""

This selects a workload to run across the set of simulated nodes.
A workload consists of a series of jobs that need to be run on simulated
nodes (one job per node).

Workload definitions are located in ``firesim/deploy/workloads/*.json``.

Some sample workloads:

``linux-uniform.json``: This runs the default FireSim Linux distro on as many nodes
as you specify when setting the ``target_config`` parameters.

``spec17-intrate.json``: This runs SPECint 2017's rate benchmarks. In this type of
workload, you should launch EXACTLY the correct number of nodes necessary to run the
benchmark. If you specify fewer nodes, the manager will warn that not all jobs were
assigned to a simulation. If you specify too many simulations and not enough
jobs, the manager will not launch the jobs.

Others can be found in the aforementioned directory. For a description of the
JSON format, see :ref:`defining-custom-workloads`.


``terminate_on_completion``
"""""""""""""""""""""""""""

Set this to ``no`` if you want your Run Farm to keep running once the workload
has completed. Set this to ``yes`` if you want your Run Farm to be TERMINATED
after the workload has completed and results have been copied off.

``suffix_tag``
""""""""""""""""""""""""""

This allows you to append a string to a workload's output directory name,
useful for differentiating between successive runs of the same workload,
without renaming the entire workload. For example, specifying
``suffix_tag: test-v1`` with a workload named ``super-application`` will result
in a workload results directory named
``results-workload/DATE--TIME-super-application-test-v1/``.

``host_debug``
^^^^^^^^^^^^^^^^^^

``zero_out_dram``
"""""""""""""""""""""""""""""

Set this to ``yes`` to zero-out FPGA-attached DRAM before simulation begins.
This process takes 2-5 minutes. In general, this is not required to produce
deterministic simulations on target machines running linux, but should be
enabled if you observe simulation non-determinism.

``disable_synth_asserts``
"""""""""""""""""""""""""""""

Set this to ``yes`` to make the simulation ignore synthesized assertions when
they fire. Otherwise, simulation will print the assertion message and terminate
when an assertion fires.


.. _config-build:

``config_build.yaml``
--------------------------

Here is a sample of this configuration file:

.. include:: /../deploy/sample-backup-configs/sample_config_build.yaml
   :code: yaml

Below, we outline each section and parameter in detail.

``build_farm``
^^^^^^^^^^^^^^^^^^^^^

In this section, you specify the specific build farm name that you wish to use to build FPGA bitstreams (see ``config_build_farm.yaml`` for build farm options and specification).
For example, if you want to run your FPGA builds on a pre-specified build farm that automatically spawns EC2 instances per FPGA build, you can
write ``build_farm: ec2_build_farm``. To see all mainline build farm options, refer to ``config_build_farm.yaml``.


``builds_to_run``
^^^^^^^^^^^^^^^^^^^^^

In this section, you can list as many build entries as you want to run
for a particular call to the ``buildafi`` command (see
``config_build_recipes.yaml`` below for how to define a build entry). For
example, if we want to run the builds named ``awesome_firesim_config`` and ``quad_core_awesome_firesim_config``, we would
write:

::

    builds_to_run:
        awesome_firesim_config
        quad_core_awesome_firesim_config


``agfis_to_share``
^^^^^^^^^^^^^^^^^^^^^^^^^^^

This is used by the ``shareagfi`` command to share the specified agfis with the
users specified in the next (``share_with_accounts``) section. In this section,
you should specify the section title (i.e. the name you made up) for a hardware
configuration in ``config_hwdb.yaml``. For example, to share the hardware config:

::

    firesim_rocket_quadcore_nic_l2_llc4mb_ddr3:
        # this is a comment that describes my favorite configuration!
        agfi: agfi-0a6449b5894e96e53
        deploy_triplet_override: null
        custom_runtime_config: null

you would use:

::

    agfis_to_share:
        firesim_rocket_quadcore_nic_l2_llc4mb_ddr3


``share_with_accounts``
^^^^^^^^^^^^^^^^^^^^^^^^^^^

A list of AWS account IDs that you want to share the AGFIs listed in
``agfis_to_share`` with when calling the manager's ``shareagfi`` command. You
should specify names in the form ``usersname: AWSACCTID``. The left-hand-side is
just for human readability, only the actual account IDs listed here matter. If you specify ``public=public`` here, the AGFIs are shared publicly, regardless of any other entires that are present.

.. _config-build-farm:

``config_build_farm.yaml``
--------------------------

Here is a sample of this configuration file:

.. include:: /../deploy/sample-backup-configs/sample_config_build_farm.yaml
   :code: yaml

Below, we outline each section and parameter in detail.

Build farm configuration sections, e.g. ``awesome_build_farm``
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

In this file, you can specify as many build farm configuration sections as you want,
each with a header like ``awesome_build_farm`` (i.e. a nice, short name
you made up). Such a section must contain the following fields:

``build_farm_type``
"""""""""""""""""""""

This field maps the build farm configuration to a specific build farm class name
(or in other words, a build farm definition). By default, build farm classes can
be found in :gh-file-ref:`deploy/buildtools/buildfarm.py`. However, you can specify
your own custom build farm classes by adding your python file to the ``PYTHONPATH``.
For example, to use the ``AWSEC2`` build farm class, you would write ``build_farm_type: AWSEC2``.

``args``
"""""""""""""""""""""

This section specifies all arguments needed for the specific ``build_farm_type`` used.
For a list of arguments needed for a specific build farm class, users should refer to
the ``_parse_args`` function in the build farm class given by ``build_farm_type``.

Mainline build farm configurations
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

There are two mainline build farm configurations provided (with their corresponding build farm classes in :gh-file-ref:`deploy/buildtools/buildfarm.py`).

``ec2_build_farm (AWSEC2)``
"""""""""""""""""""""""""""

.. literalinclude:: /../deploy/sample-backup-configs/sample_config_build_farm.yaml
   :language: yaml
   :start-after: DOCREF START: EC2 Build Farm
   :end-before: DOCREF END: EC2 Build Farm

This build farm configuration provides the default EC2 build farm functionality
of FireSim 1.13.x and earlier. For each AGFI (FireSim AWS EC2 F1 FPGA Image) being
built, a new EC2 instance (build host) will be spawned according to the build farm configuration
arguments. Its ``build_farm_type`` is set as the ``AWSEC2`` build farm class.

The ``instance_type`` argument defines the type of instance that the build will run on.
Generally, running on a ``z1d.2xlarge`` is sufficient. In our experience, using more
powerful instances than this provides little gain.

The ``build_instance_market`` argument specifies the AWS market used. You can specify either
``spot`` or ``ondemand`` here.

When ``build_instance_market: spot``, the ``spot_interruption_behavior`` and ``spot_max_price``
arguments determines spot instance behaviors. The ``spot_interruption_behavior`` value
determines what happens to an instance if it receives the interruption signal from AWS. You can specify
either ``hibernate``, ``stop``, or ``terminate`` for this value. The ``spot_max_price``
value determines the max price you are willing to pay per instance, in dollars.
You can also set it to ``ondemand`` to set your max to the on-demand price for the instance.

The ``default_build_dir`` argument specifies the default location of the build directory used
on the spawned build host which will hold build outputs from Vivado and other tools.

``local_build_farm (ExternallyProvisioned)``
""""""""""""""""""""""""""""""""""""""""""""

.. literalinclude:: /../deploy/sample-backup-configs/sample_config_build_farm.yaml
   :language: yaml
   :start-after: DOCREF START: Local Build Farm
   :end-before: DOCREF END: Local Build Farm

This build farm configuration allows users to provide their own build hosts
(build machines) to be used for building FPGA images.
In the case of AGFI builds and this specific configuration, this allows for users to build AGFIs locally
(and potentially on other build hosts).
Its ``build_farm_type`` is set as the ``ExternallyProvisioned`` build farm class.
This build farm class assumes that the list of IPs given to it are pre-setup to work
with FireSim (Vivado installed, FireSim dependencies installed, etc).

The ``default_build_dir`` argument specifies the default location of the build directory used
on the build host which will hold build outputs from Vivado and other tools.
This location can be overridden by specific build farm hosts (see ``build_farm_hosts`` description).

The ``build_farm_hosts`` argument section provides a list of IP addresses (and ``localhost``) that
can be used for builds. IP addresses must follow `Fabric syntax guidelines <https://ploxiln.github.io/fab-classic/usage/execution.html#hosts>`_.
An example of a list of IPs to run builds on is as follows:

::

    build_farm_hosts:
        - localhost
        - "111.111.1.111"
        - "centos@222.222.2.222"

Additionally, if a specific build host wants to override the build directory provided by ``default_build_dir``,
they can provide the ``override_build_dir`` field. For example:

::

    build_farm_hosts:
        - localhost
        # Note the : after the IP address
        - "111.111.1.111":
            override_build_dir: /my/awesome/path
        - "centos@222.222.2.222"

.. _config-build-recipes:

``config_build_recipes.yaml``
--------------------------------

Here is a sample of this configuration file:

.. include:: /../deploy/sample-backup-configs/sample_config_build_recipes.yaml
   :code: yaml

Below, we outline each section and parameter in detail.


Build definition sections, e.g. ``awesome_firesim_config``
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

In this file, you can specify as many build definition sections as you want,
each with a header like ``awesome_firesim_config`` (i.e. a nice, short name
you made up). Such a section must contain the following fields:

``DESIGN``
"""""""""""""""""""""""""""""

This specifies the basic target design that will be built. Unless you
are defining a custom system, this should be set to ``FireSim``.
We describe this in greater detail in :ref:`Generating Different
Targets<generating-different-targets>`).

``TARGET_CONFIG``
"""""""""""""""""""

This specifies the hardware configuration of the target being simulated. Some
examples include ``FireSimRocketConfig`` and ``FireSimQuadRocketConfig``.
We describe this in greater detail in :ref:`Generating Different
Targets<generating-different-targets>`).


``PLATFORM_CONFIG``
"""""""""""""""""""""

This specifies hardware parameters of the simulation environment - for example,
selecting between a Latency-Bandwidth Pipe or DDR3 memory models.
These are defined in :gh-file-ref:`sim/firesim-lib/src/main/scala/configs/CompilerConfigs.scala`.
We specify the host FPGA frequency in the ``PLATFORM_CONFIG`` by appending a frequency
``Config`` with an underscore (ex. BaseF1Config_F160MHz).
We describe this in greater detail in :ref:`Generating Different
Targets<generating-different-targets>`).

``deploy_triplet``
""""""""""""""""""

This allows you to override the ``deploytriplet`` stored with the AGFI.
Otherwise, the ``DESIGN``/``TARGET_CONFIG``/``PLATFORM_CONFIG`` you specify
above will be used. See the AGFI Tagging section for more details. Most likely,
you should leave this set to ``null``. This is usually only used if you have
proprietary RTL that you bake into an FPGA image, but don't want to share with
users of the simulator.

``TARGET_PROJECT`` `(Optional)`
"""""""""""""""""""""""""""""""

This specifies the target project in which the target is defined (this is described
in greater detail :ref:`here<generating-different-targets>`).  If
``TARGET_PROJECT`` is undefined the manager will default to ``firesim``.
Setting ``TARGET_PROJECT`` is required for building the MIDAS examples
(``TARGET_PROJECT: midasexamples``) with the manager, or for building a
user-provided target project.

``s3_bucket_name``
""""""""""""""""""""""""""

This is used behind the scenes in the AGFI creation process. You will only
ever need to access this bucket manually if there is a failure in AGFI creation
in Amazon's backend.

Naming rules: this must be all lowercase and you should stick to letters and numbers.

The first time you try to run a build, the FireSim manager will try to create
the bucket you name here. If the name is unavailable, it will complain and you
will need to change this name. Once you choose a working name, you should never
need to change it.

In general, ``firesim-yournamehere`` is a good choice.

``post_build_hook``
"""""""""""""""""""""""

(Optional) Provide an a script to run on the results copied back
from a _single_ build instance. Upon completion of each design's build,
the manager invokes this script and passing the absolute path to that instance's
build-results directory as it's first argument.

.. _config-hwdb:

``config_hwdb.yaml``
---------------------------

Here is a sample of this configuration file:

.. include:: /../deploy/sample-backup-configs/sample_config_hwdb.yaml
   :code: yaml


This file tracks hardware configurations that you can deploy as simulated nodes
in FireSim. Each such configuration contains a name for easy reference in higher-level
configurations, defined in the section header, an agfi, which represents the
FPGA image, a custom runtime config, if one is needed, and a deploy triplet
override if one is necessary.

When you build a new AGFI, you should put the default version of it in this
file so that it can be referenced from your other configuration files.

The following is an example section from this file - you can add as many of
these as necessary:

.. literalinclude:: /../deploy/sample-backup-configs/sample_config_hwdb.yaml
   :language: yaml
   :start-after: DOCREF START: Example HWDB Entry
   :end-before: DOCREF END: Example HWDB Entry

``NAME_GOES_HERE``
^^^^^^^^^^^^^^^^^^^^^^^^^^^^

In this example, ``firesim_rocket_quadcore_nic_l2_llc4mb_ddr3`` is the name that will be
used to reference this hardware design in other configuration locations. The following
items describe this hardware configuration:

``agfi``
"""""""""""""""

This represents the AGFI (FPGA Image) used by this hardware configuration.


``deploy_triplet_override``
"""""""""""""""""""""""""""""

This is an advanced feature - under normal conditions, you should leave this set to ``null``, so that the
manager uses the configuration triplet that is automatically stored with the
AGFI at build time. Advanced users can set this to a different
value to build and use a different driver when deploying simulations. Since
the driver depends on logic now hardwired into the
FPGA bitstream, drivers cannot generally be changed without requiring FPGA
recompilation.


``custom_runtime_config``
"""""""""""""""""""""""""""""

This is an advanced feature - under normal conditions, you can use the default
parameters generated automatically by the simulator by setting this field to
``null``. If you want to customize runtime parameters for certain parts of
the simulation (e.g. the DRAM model's runtime parameters), you can place
a custom config file in ``sim/custom-runtime-configs/``. Then, set this field
to the relative name of the config. For example,
``sim/custom-runtime-configs/GREATCONFIG.conf`` becomes
``custom_runtime_config: GREATCONFIG.conf``.


Add more hardware config sections, like ``NAME_GOES_HERE_2``
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

You can add as many of these entries to ``config_hwdb.yaml`` as you want, following the format
discussed above (i.e. you provide ``agfi``, ``deploy_triplet_override``, or ``custom_runtime_config``).
