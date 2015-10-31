name := "gpt-shmreceiver"

organization := "se.gigurra"

version := scala.util.Properties.envOrElse("GPT_BUILD_VERSION", "SNAPSHOT")

scalaVersion := "2.11.7"

libraryDependencies += "se.gigurra" % "gpt-common" % version.value
