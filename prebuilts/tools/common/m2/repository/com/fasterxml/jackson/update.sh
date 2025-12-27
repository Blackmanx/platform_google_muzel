#! /bin/bash

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )

JACKSON_VERSION=2.15.0
FASTERXML_WOODSTOX_CORE_VERSION=6.5.1
WOODSTOX_STAX2_API_VERSION=4.2.1

mvn org.apache.maven.plugins:maven-dependency-plugin:get \
  -DrepoUrl=https://repo1.maven.org/maven2/ \
  -Dartifact=com.fasterxml.jackson.module:jackson-module-kotlin:$JACKSON_VERSION

REPOSITORY_DIR=$SCRIPT_DIR/../../..

function copy-dir {
  local ARTIFACT_PATH=$1
  local TARGET_DIR=$REPOSITORY_DIR/$ARTIFACT_PATH
  
  rm -fr $TARGET_DIR
  mkdir -p $(dirname $TARGET_DIR)
  cp -r ~/.m2/repository/$ARTIFACT_PATH/ $TARGET_DIR
}

copy-dir com/fasterxml/jackson/core/jackson-annotations/$JACKSON_VERSION
copy-dir com/fasterxml/jackson/core/jackson-core/$JACKSON_VERSION
copy-dir com/fasterxml/jackson/core/jackson-databind/$JACKSON_VERSION
copy-dir com/fasterxml/jackson/dataformat/jackson-dataformat-xml/$JACKSON_VERSION
copy-dir com/fasterxml/jackson/module/jackson-module-kotlin/$JACKSON_VERSION
copy-dir com/fasterxml/woodstox/woodstox-core/$FASTERXML_WOODSTOX_CORE_VERSION
copy-dir org/codehaus/woodstox/stax2-api/$WOODSTOX_STAX2_API_VERSION
