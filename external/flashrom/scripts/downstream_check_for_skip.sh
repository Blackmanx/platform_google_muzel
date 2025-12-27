#!/usr/bin/env bash

git log --since="two weeks ago" --format=format:"%s" cros/upstream_validation | sort > upstream.log
git log --since="two weeks ago" --format=format:"%s" | grep 'UPSTREAM\|BACKPORT' | sed 's/BACKPORT: \|UPSTREAM: //g' | sort > downstream.log
comm upstream.log downstream.log -23 > missing.log

git log --oneline --since="two weeks ago" cros/upstream_validation | grep -Ff missing.log
