
// Remember, sbt needs empty lines between active settings

name := "gpt"

organization := "se.gigurra"

version := scala.util.Properties.envOrElse("GPT_BUILD_VERSION", "SNAPSHOT")

scalaVersion := "2.11.7"

isSnapshot := version.value.contains("SNAPSHOT")

crossPaths := false

lazy val gpt_common = project in file("gpt-common")

lazy val gpt_displaysreceiver = (project in file("gpt-displaysreceiver")).dependsOn(gpt_common)

lazy val gpt_keyreceiver = (project in file("gpt-keyreceiver")).dependsOn(gpt_common)

lazy val gpt_keytransmitter = (project in file("gpt-keytransmitter")).dependsOn(gpt_common)

lazy val gpt_shmtransmitter = (project in file("gpt-shmtransmitter")).dependsOn(gpt_common)

lazy val gpt_shmreceiver = (project in file("gpt-shmreceiver")).dependsOn(gpt_common)
