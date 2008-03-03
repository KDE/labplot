#!/bin/bash

if [ `arch` == "x86_64" ]; then
	echo "lib64"
else
	echo "lib"
fi
