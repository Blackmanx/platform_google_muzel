#!/usr/bin/env python3
# SPDX-License-Identifier: BSD-3-Clause
# Copyright (C) 2020, Google Inc.
#
# Author: Paul Elder <paul.elder@ideasonboard.com>
#
# parser.py - Run mojo parser with python3

import os
import sys

import mojom_parser as parser

parser.Run(sys.argv[1:])
