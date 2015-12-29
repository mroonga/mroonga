.. highlightlang:: none

``mroonga_normalize()``
=======================

Summary
-------

``mroonga_normalize()`` UDF normalizes text by Groonga's normalizer.

Syntax
------

``mroonga_normalize()`` has required parameter and optional parameter::

  mroonga_normalize(string)
  mroonga_normalize(string, normalizer_name)

Usage
-----

Here is the example query which use Groonga's ``NormalizerAuto`` normalizer to be normalized::

  SELECT mroonga_normalize("ABCDあぃうぇ㍑");
  abcdあぃうぇリットル

Here is the example query which use Groonga's ``NormalizerMySQLUnicodeCIExceptKanaCIKanaWithVoicedSoundMark`` normalizer to be normalized::

  SELECT mroonga_normalize("aBｃＤあぃウェ㍑", "NormalizerMySQLUnicodeCIExceptKanaCIKanaWithVoicedSoundMark");
  ABCDあぃうぇ㍑


Parameters
----------

Required parameters
^^^^^^^^^^^^^^^^^^^

There is one required parameter, ``string``.

``string``
""""""""""

It specifies text which you want to normalize.

Optional parameters
^^^^^^^^^^^^^^^^^^^

There is one optional parameter, ``normalizer_name``.

``normalizer_name``
"""""""""""""""""""

It specifies Groonga's normalizer name to normalize.

The default value is ``NormalizerAuto``.

Return value
------------

It returns normalized string.

