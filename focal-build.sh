#!/bin/bash

clickable clean

find  -name "*.qml" -exec sed -i 's/Ubuntu/Lomiri/g' {} +
sed -i 's/ubuntu-sdk-16.04.5/ubuntu-sdk-20.04/g' manifest.json.in
sed -i 's/16.04/20.04/g' openstore.apparmor
sed -i 's/X-Ubuntu/X-Lomiri/g' openstore.desktop.in

export CLICKABLE_FRAMEWORK=ubuntu-sdk-20.04
clickable $@

find  -name "*.qml" -exec sed -i 's/Lomiri/Ubuntu/g' {} +
sed -i 's/ubuntu-sdk-20.04/ubuntu-sdk-16.04.5/g' manifest.json.in
sed -i 's/20.04/16.04/g' openstore.apparmor
sed -i 's/X-Lomiri/X-Ubuntu/g' openstore.desktop.in
