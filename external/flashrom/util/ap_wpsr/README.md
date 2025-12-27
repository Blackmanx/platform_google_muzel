
ap_wpsr tool
============

The ap_wpsr tool was built to address specific requirements of AP RO verification
that intends to measure SR contents for invariance after signature validation.

The tool leverages the flashrom SPI flash chips database to produce theoretical
mask,value pairs for a given WP configuration (i.e. protection range start and
length) and flash chip part name.

limitations
-----------

There are however some limitations. While the tool is pure logic (no side-effects) and
intended to calculate values with reasonable fidelity there could however be inaccuracies.

These inaccuracies can be the result of any combination of the following factors:

 * Incorrect writeprotoct SR topology data within the database.
 * Duplicate chips or otherwise ambiguous chip identifiers (id or string name).
 * OTP fused values within the SPI flash chip itself.

A precise flash chip name is required to be cited physically on what chip is being worked
with (and not via probing or other inferences). This precise name needs to be mapped to
the corresponding name within the database to produce the tool output products. The
output products *must* be cross-validated with the datasheet to ensure the products are
consistent with expectations and any errors in the database are corrected with a patch
back to the canonical flashrom flashchips database upstream.

usage
-----

An example usage could be `./ap_wpsr -n "W25Q128.V" -s 0x40000 -l 0xFC0000`.

finding chip names
------------------

The ap_wpsr tool needs to be given the name of the flashrom chip entry to use
for calculating mask,value pairs. This name may not match the vendor name, for
example the chip "W25Q128FVSIG" corresponds to a flashrom entry called
"W25Q128.V".

These steps should be followed to find the flashrom chip name for your chip:

* Check the chip's datasheet to find the vendor and device ID values.
* Inspect flashchips.h to find matching ID define macros.
* Inspect flashchips.c to find the chip entry that uses those ID macros
* If there are multiple flashrom chip entries with the same IDs, check all
  of them against the datasheet to find the correct one.
* Finally, check that the chip entry has a reg_bits field and that it matches
  the datasheet *exactly*. Verify that all WP-related bits in the datasheet
  are included in reg_bits and vice versa.
* If there are no matching chip entries, create one for the chip and send a
  patch to upstream flashrom for review.
* Pass the matching entry's name to the tool via the `-n` option.

src structure
-------------

There are two key parts to the tool `imports/` and `shim/`. The `shim/` directory just
contains enough symbols to allow linking of the `imports/` source taken from upstream
flashrom. The `imports/` contains two key ingredients - the chip database and the
writeprotection calculation code. These should be kept up to date by importing fresh
copies from flashrom.
