#!/bin/bash

# assumption:
#  - pwd must be inside the repo's homed directory (android-ndk)
#  - upon completion, we are still in the same directory ( no change )


# parse all build.gradle to find the specified tokens' version
# usage:
#    retrive_versions  token version_file
# where
#    token: the token to search for inside build.grade
#           version string is right after the token string
#    version_file: file to hold the given versions
#                  one version at a line
retrieve_versions() {
#   $1: token; $2 version_file
    if [[ -z $1 ]] || [[ -z $2 ]]; then
        echo "input string(s) may be empty: token: $1; version_file: $2"
        return 1
    fi

    find . -type f -name 'build.gradle' -exec grep $1 {} +  | \
    sed "s/^.*$1//" | sed 's/[=+]//g' |   \
    sed 's/"//g' | sed "s/'//g" |     \
    sed 's/[[:space:]]//g' | \
    awk '!seen[$0]++' > $2
 
    return 0
}

# helper function for src_str > target_str
# Usage
#     comp_ver_string src_str target_str
# return:
#   0: src_str <= target_str
#   1: otherwise
comp_ver_string () {
#   $1: src_str, $2: target_str
    if [[ $1 == $2 ]]
    then
        return 0
    fi
    local IFS=.
    local i ver1=($1) ver2=($2)
    # fill empty fields in ver1 with zeros
    for ((i=${#ver1[@]}; i<${#ver2[@]}; i++))
    do
        ver1[i]=0
    done
    for ((i=0; i<${#ver1[@]}; i++))
    do
        if [[ -z ${ver2[i]} ]]
        then
            # fill empty fields in ver2 with zeros
            ver2[i]=0
        fi
        if ((10#${ver1[i]} < 10#${ver2[i]}))
        then
            return 0
        fi

        if ((10#${ver1[i]} > 10#${ver2[i]}))
        then
            return 1
        fi
    done
    return 0
}

# prepare to install necessary packages
if [ -f ~/.android/repositories.cfg ]; then
  touch ~/.android/repositories.cfg
fi

TMP_SETUP_FILENAME=versions_.txt

## Retrieve all necessary Android Platforms and install them all
retrieve_versions compileSdkVersion $TMP_SETUP_FILENAME

# Install platforms
while read -r version_; do
  version_=${version_//android-/}
  echo y | $ANDROID_HOME/tools/bin/sdkmanager "platforms;android-$version_";
done < $TMP_SETUP_FILENAME
# echo "Android platforms:"; cat $TMP_SETUP_FILENAME

# Install side by side ndks
retrieve_versions ndkVersion $TMP_SETUP_FILENAME
while read -r version_; do
    echo y | $ANDROID_HOME/tools/bin/sdkmanager "ndk;$version_" --channel=3;
done < $TMP_SETUP_FILENAME
# echo "NDK versions:"; cat $TMP_SETUP_FILENAME

# add customized cmake installation
echo y | $ANDROID_HOME/cmdline-tools/latest/bin/sdkmanager "cmake;3.18.1"

rm -f $TMP_SETUP_FILENAME


