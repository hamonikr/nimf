#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")"

pkg-config --exists gtk4 || { echo "gtk4 pkg-config not found"; exit 1; }

cc -o gtk4-im-test gtk4-im-test.c $(pkg-config --cflags --libs gtk4) -O2 -g

echo "Built ./gtk4-im-test"
