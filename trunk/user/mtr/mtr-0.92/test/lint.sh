#!/bin/sh

#  Check the Python test source for good style

PYTHON_SOURCE=*.py

pep8 $PYTHON_SOURCE
pylint --reports=n $PYTHON_SOURCE 2>/dev/null
mypy --py2 $PYTHON_SOURCE
