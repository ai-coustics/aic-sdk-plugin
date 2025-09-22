#!/bin/bash

# Create zip package from release folder
cd ./release/ && zip -r ../aic-sdk-plugin-macos.zip . && cd ..
