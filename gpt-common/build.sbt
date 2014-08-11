
// Remember, sbt needs empty lines between active settings

name := "gpt-common"

organization := "se.gigurra"

version := scala.util.Properties.envOrElse("GPT_BUILD_VERSION", "SNAPSHOT")

isSnapshot := version.value.contains("SNAPSHOT")

crossPaths := false

libraryDependencies += "net.java.dev.jna" % "jna" % "4.1.0"

libraryDependencies += "net.java.dev.jna" % "jna-platform" % "4.1.0"

libraryDependencies += "se.culvertsoft" % "mgen-javalib" % "SNAPSHOT"

libraryDependencies += "se.gigurra" % "libgurra" % "SNAPSHOT"

unmanagedSourceDirectories in Compile += baseDirectory.value / "src_generated/main/java"

EclipseKeys.withSource := true