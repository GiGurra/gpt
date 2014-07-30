
// Remember, sbt needs empty lines between active settings

name := "gpt"

organization := "se.gigurra"

version := scala.util.Properties.envOrElse("GPT_BUILD_VERSION", "SNAPSHOT")

isSnapshot := version.value.contains("SNAPSHOT")

crossPaths := false

retrieveManaged := true

// lazy val mgen_api = project in file("mgen-api")

// lazy val mgen_compiler = (project in file("mgen-compiler")).dependsOn(mgen_api)

