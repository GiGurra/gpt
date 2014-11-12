
// Remember, sbt needs empty lines between active settings

name := "gpt-shmreceiver"

organization := "se.gigurra"

version := scala.util.Properties.envOrElse("GPT_BUILD_VERSION", "SNAPSHOT")

scalaVersion := "2.11.4"

isSnapshot := version.value.contains("SNAPSHOT")

crossPaths := false

libraryDependencies += "se.gigurra" % "gpt-common" % version.value

EclipseKeys.withSource := true
