.. highlightlang:: none

Upgrade
=======

There is a case that incompatible change is introduced at new release.
It is announced by release announce if new release contains such a incompatible change.

Here is the list of recommended way of upgrading Mroonga from old release.

See following URL about upgrade sequence if you use previous version.

If you upgrade prior to 1.20, refer to :ref:`release-1-20`

If you upgrade from 1.20, refer to :ref:`release-2-00`

If you upgrade from 2.00 or 2.01, refer to :ref:`release-2-02`

If you upgrade from 2.00 or later and using multiple column indexes on storage mode, refer to :ref:`release-2-03`

If you upgrade from 2.04 or later and using SET column or ENUM that has the number of elements < 256 in Storage mode, refer to :ref:`release-2-05`

If you upgrade from 2.05 or later and using multiple column indexes against VARCHAR or CHAR, refer to :ref:`release-2-08`

If you upgrade from 2.08 or later and using TIMESTAMP column, please recreate database. If you upgrade from 2.08 or later and using CHAR(N) as primary key, please recreate index. Refer to :ref:`release-2-09` for each case.

If you upgrade prior to 5.03 and satisfies following the conditions, refer to :ref:`release-5-04` and upgrade schema.

* Using custom tokenizer in index comment
* Using :ref:`mroonga_default_parser` as server variable
* Using `index_flags` parameter for index column
* Using `type` parameter for Groonga's column type in storage mode

