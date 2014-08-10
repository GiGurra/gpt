@echo off

cp -rf lib/turbojpeg.ja_ lib/turbojpeg.jar

sbt compile package publish-local
