#!/bin/sh

TRIGS="pdv-7621-ci pdv-7628-ci"

git config --global user.name "hanwckf"
git config --global user.email "my375229675@gmail.com"

for repo in $TRIGS ; do
	cd /opt
	git clone --depth=1 https://hanwckf:$GITHUB_KEY@github.com/hanwckf/$repo.git && cd $repo
	echo $(LANG=C date) >> Build.log
	[ -f /opt/$repo.yml ] && cp -f /opt/$repo.yml .travis.yml
	git add .
	git commit -m "build trigger"
	git remote set-url origin https://hanwckf:$GITHUB_KEY@github.com/hanwckf/$repo.git
	git push
done
