#!/bin/bash

echo "Capturing..."
lcov --directory . --capture --output-file coverage.info
echo "Removing unnecessary files..."
lcov --remove coverage.info 'tests/*' '/usr/include/*' 'protobuf-3.0.0-bin/*' 'googletest/*' 'googlemock/*' --output-file coverage.info
echo "Listing..."
lcov --list coverage.info
echo "Submitting..."
coveralls-lcov --repo-token $COVERALLS_TOKEN coverage.info
