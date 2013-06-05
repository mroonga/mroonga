.. highlightlang:: none

Trouble Shooting
================

There are some frequently asked questions in mroonga storage engine.

How to avoid mmap Cannot allocate memory error
----------------------------------------------

There is a case following mmap error in log file::

  2013-06-04 08:19:34.835218|A|4e86e700|mmap(4194304,551,432017408)=Cannot allocate memory <13036498944>

Note that ``<13036498944>`` means total size of mmap (almost 12GB) in this case.

So you need to confirm following point of views.

* Are there enough free memory?
* Are maximum number of mappings exceeded?

To check there are enough free memory, you can use ``vmstat`` command.

To check whether maximum number of mappings are exceeded, you can investigate the value of ``vm.max_map_count``.

If this issue is fixed by modifying the value of ``vm.max_map_count``, it's exactly the reason.

You can modify ``vm.max_map_count`` temporary by ``sudo sysctl -w vm.max_map_count=LARGER VALUE``.

Then save the configuration value to ``/etc/sysctl.conf`` or ``/etc/sysctl.d/*.conf``.




