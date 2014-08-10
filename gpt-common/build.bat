@echo off

mgen project="models/project.xml"

sbt compile package publish-local
