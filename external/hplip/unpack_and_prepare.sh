#!/bin/sh

# This script is used to unpack a tarball from upstream, copy over just the
# specific files that are needed, and apply patches.  This is intended to be run
# from inside the external/hplip directory.

set -eu

# These two values (VER and SHA1) need to be updated to the new version in order
# to uprev this package.  OLD_VER should be updated to the current version (the
# value of VER before it was updated).  OLD_VER is used in this script to
# automatically update the versions in the Android.bp file.
VER=3.25.2
SHA1=9fb975ef6a7793c3d6de214cc51300d351b811f3
OLD_VER=3.22.6

SRC=https://sourceforge.net/projects/hplip/files/hplip/"$VER"/hplip-"$VER".tar.gz
FN=$(basename "$SRC")

wget "$SRC"
actual_sha1=$(sha1sum "$FN" | cut -d' ' -f1)
if [ "$actual_sha1" != "$SHA1" ]; then
  echo "sha1 does not match:"
  echo "Actual:   $actual_sha1"
  echo "Expected: $SHA1"
  rm -f "$FN"
  exit 1
fi

datetime=$(date +%y%m%d_%H%M%S)
src_dir=extracted_$datetime
mkdir "$src_dir"
echo "Unpacking..."
tar xfz "$FN" --directory "$src_dir"
rm -f "$FN"

echo "Syncing..."
rsync -ar "$src_dir"/*/prnt/hpps prnt/
rsync -ar --exclude=libImageProcessor*.so "$src_dir"/*/prnt/hpcups prnt/
rsync -ar "$src_dir"/*/common ./
rsync -ar "$src_dir"/*/io/hpmud io/

rm -rf "$src_dir"

for file in patches/*.patch; do
  echo "Applying $file..."
  git apply "$file"
done

sed -i s/$OLD_VER/$VER/g Android.bp
