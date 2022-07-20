#!/bin/env bash

export PATH=`pwd`/.vscode/php/bin:$PATH
phpize --clean && phpize && ./configure && make test
