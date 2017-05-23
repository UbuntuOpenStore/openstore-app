#!/bin/bash

BZR_PLUGIN_PATH=$HOME/.bazaar/plugins bzr fast-export --plain | git fast-import
git reset
git push

