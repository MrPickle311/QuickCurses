#!/usr/bin/bash

git status
echo "Add, commit and push all ? y -yes , n -no"
read pred
if [ $pred = "y" ]; then
	echo "Pass a message : "
	read msg
	git add .
	git commit -m "$msg"
	git push -u origin master
fi