#!/bin/bash
# source-date.sh
# Outputs the commit time of HEAD as a Unix epoch (seconds, UTC), used as the
# SOURCE_DATE_EPOCH fallback for reproducible builds when no .git is available
# (e.g. a git-archive release tarball).
# Run this before tagging/archiving a release: ./source-date.sh | tee ./source-date.last

git log -1 --pretty=%ct
