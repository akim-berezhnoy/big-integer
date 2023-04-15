#!/bin/bash
# Данный скрипт следует вызвать после первого клонирования
# репозитория. Подробная информация:
# https://cpp-kt.github.io/course/instruction.html

set -euo pipefail
IFS=$' \t\n'

if [[ ! $(git remote get-url origin) =~ ^git@ ]]; then
    echo "The project is not cloned via SSH"
    exit 1
fi

if git remote | grep --quiet upstream; then
    echo "init-repo.sh was already done on this repository."
    exit 1
fi

if [ -n "$(git status --untracked-files=no --porcelain)" ]; then
    echo "init-repo.sh is intended to work on a freshly created repository."
    echo "The working copy of this reposity is dirty. Please checkout or"
    echo "stash changes and run ./init-repo.sh again."
    exit 1
fi

source ci-extra/set-upstream.sh

git remote add upstream "git@github.com:$UPSTREAM_REPO.git"
git fetch upstream
git branch feedback upstream/master
git push -u origin feedback:feedback
git checkout feedback
git branch -f master feedback
git checkout master
git push -f origin master
