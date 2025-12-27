#!/usr/bin/env bash

skips="cros
manibuilder"

comm -1 <(git ls-tree -r --name-only cros/master | sort) \
  <(git ls-tree -r --name-only cros/upstream_validation | sort) \
  | grep -vf <(echo "${skips}") \
  | xargs git diff cros/master cros/upstream_validation --stat -- \
  | cat

# writeprotect.c                                 |   18 +-
# 96 files changed, 1887 insertions(+), 9391 deletions(-)
