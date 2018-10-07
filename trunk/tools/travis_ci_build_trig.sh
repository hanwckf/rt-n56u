#!/bin/sh

cd /opt
git config --global user.name "hanwckf"
git config --global user.email "my375229675@gmail.com"

git clone --depth=1 https://github.com/hanwckf/pdv-7621-ci.git && cd pdv-7621-ci
echo $(LANG=C date) >> Build.log
git add .
git commit -m "build trigger"
git remote set-url origin https://hanwckf:$GITHUB_KEY@github.com/hanwckf/pdv-7621-ci.git
git push
