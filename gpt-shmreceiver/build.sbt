
// Remember, sbt needs empty lines between active settings

name := "gpt-shmreceiver"

organization := "se.gigurra"

version := scala.util.Properties.envOrElse("GPT_BUILD_VERSION", "SNAPSHOT")

isSnapshot := version.value.contains("SNAPSHOT")

crossPaths := false

libraryDependencies += "se.gigurra" % "gpt-common" % version.value

libraryDependencies += "se.gigurra" % "libgurra" % "SNAPSHOT"

EclipseKeys.withSource := true
