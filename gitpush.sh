#!/bin/bash

git add .

msg=$1

if [ -z "$msg" ]; then
    msg="update"
fi

git commit -m "$msg"
git push

