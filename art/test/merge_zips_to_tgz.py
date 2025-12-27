#!/usr/bin/env python3

import tarfile
import zipfile
import sys
import io
from pathlib import Path

def merge_zips_to_tgz(tgz_filename, zip_filenames):
  with open(tgz_filename, 'wb') as outfile:
    with tarfile.open(fileobj=outfile, mode='w:gz') as tgz:
      for zip_filename in zip_filenames:
        with zipfile.ZipFile(zip_filename, 'r') as zip_file:
          for zip_info in zip_file.infolist():
            with zip_file.open(zip_info) as zip_member:
              buffer = io.BytesIO(zip_member.read())
              tar_info = tarfile.TarInfo(name=zip_info.filename)
              tar_info.size = len(buffer.getbuffer())
              tgz.addfile(tarinfo=tar_info, fileobj=buffer)


if __name__ == "__main__":
  if len(sys.argv) < 2:
    print(f"Usage: {Path(__file__).name} output.tgz zip1.zip zip2.zip ...")
    sys.exit(1)

  tgz_filename = sys.argv[1]
  zip_filenames = sys.argv[2:]
  merge_zips_to_tgz(tgz_filename, zip_filenames)

