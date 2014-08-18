
// Remember, sbt needs empty lines between active settings

name := "gpt-keytransmitter"

organization := "se.gigurra"

version := scala.util.Properties.envOrElse("GPT_BUILD_VERSION", "SNAPSHOT")

isSnapshot := version.value.contains("SNAPSHOT")

crossPaths := false

libraryDependencies += "se.gigurra" % "gpt-common" % version.value

EclipseKeys.withSource := true
