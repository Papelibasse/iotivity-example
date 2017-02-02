#! /usrbin/make -f

default: all

url?=file://${CURDIR}/

all: src help

help: README.md
	cat README.md

distclean:
	rm -rf src .gitmodules

src: rule/src

rule/src: README.md
	grep -o '^* branch=[^\:]*' $< | cut -d= -f2 | while read branch ; do \
  mkdir -p src/$${branch}.tmp ; rmdir src/$${branch}.tmp ;\
  git submodule add --force -b $${branch} ${url} src/$${branch} ;\
  done
	git submodule update

rule/branches:
	git branch -a | grep -v '*' | grep -v remotes | while read branch; do \
  mkdir -p src/$${branch}.tmp ; rmdir src/$${branch}.tmp ;\
  git submodule add --force -b $${branch} ${url} src/$${branch} ;\
  done
	git submodule update
