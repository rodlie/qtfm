#!/bin/sh
# QtFM CI (c) 2019 Ole-André Rodlie

set -e

SETUP=${SETUP:-1}

if [ "${SETUP}" = 1 ]; then
  sudo chmod 777 /opt
fi

OS=`uname -s`
CWD=`pwd`
SDK=/opt/qtfm
DEPLOY=/opt/deploy

COMMIT="${TRAVIS_COMMIT}"
if [ "${TRAVIS_TAG}" != "" ]; then
  COMMIT=""
fi

if [ "${SETUP}" = 1 ]; then
  rm -f download.tar.bz2 || true
  if [ "${OS}" = "Linux" ]; then
    echo "Setup ubuntu ..."
    sudo apt-get update
    sudo apt-get install qt4-dev-tools qtbase5-dev libxss-dev libmagic-dev qt4-qmake qt5-qmake tree
    #echo "Extracting linux64 sdk ..."
  elif [ "${OS}" = "Darwin" ]; then
    curl -L https://sourceforge.net/projects/qt-file-manager/files/sdk/qtfm-mac-sdk-6.1.tar.bz2/download --output download.tar.bz2
    tar xf download.tar.bz2 -C /opt
    rm -f download.tar.bz2
  fi
fi

mkdir -p ${CWD}/deploy ${DEPLOY}
TAG=`date "+%Y%m%d%H%M"`
if [ "${TRAVIS_TAG}" != "" ]; then
  echo "===> RELEASE MODE"
  TAG="${TRAVIS_TAG}"
fi

if [ "${OS}" = "Linux" ]; then
    export CWD=`pwd`
    mkdir $CWD/build1
    cd $CWD/build1
    qmake -qt=qt5 CONFIG+=release PREFIX=/usr  ..
    make
    make INSTALL_ROOT=`pwd`/pkg install
    tree pkg
    mkdir $CWD/build2
    cd $CWD/build2
    qmake -qt=qt4 CONFIG+=release PREFIX=/usr  ..
    make
    make INSTALL_ROOT=`pwd`/pkg install
    tree pkg
    mkdir $CWD/build3
    cd $CWD/build3
    qmake -qt=qt5 CONFIG+=debug PREFIX=/usr  ..
    make
    make INSTALL_ROOT=`pwd`/pkg install
    tree pkg
    mkdir $CWD/build4
    cd $CWD/build4
    qmake -qt=qt4 CONFIG+=debug PREFIX=/usr  ..
    make
    make INSTALL_ROOT=`pwd`/pkg install
    tree pkg
    mkdir $CWD/build5
    cd $CWD/build5
    qmake -qt=qt5 CONFIG+=debug PREFIX=/usr CONFIG+=basic ..
    make
    make INSTALL_ROOT=`pwd`/pkg install
    tree pkg
    mkdir $CWD/build6
    cd $CWD/build6
    qmake -qt=qt4 CONFIG+=debug PREFIX=/usr CONFIG+=basic ..
    make
    make INSTALL_ROOT=`pwd`/pkg install
    tree pkg
    mkdir $CWD/build7
    cd $CWD/build7
    qmake -qt=qt5 CONFIG+=release PREFIX=/usr CONFIG+=basic ..
    make
    make INSTALL_ROOT=`pwd`/pkg install
    tree pkg
    mkdir $CWD/build8
    cd $CWD/build8
    qmake -qt=qt4 CONFIG+=release PREFIX=/usr CONFIG+=basic ..
    make
    make INSTALL_ROOT=`pwd`/pkg install
    tree pkg
elif [ "${OS}" = "Darwin" ]; then
    echo "===> Building mac64 ..."
    PKG_CONFIG=${SDK}/bin/pkg-config
    PKG_CONFIG_PATH="${SDK}/lib/pkgconfig:${PKG_CONFIG_PATH}"
    PATH=${SDK}/bin:/usr/bin:/bin
    rm -rf build || true
    mkdir build && cd build
    qmake CONFIG+=release CONFIG+=deploy ..
    make
    strip -u -r fm/QtFM.app/Contents/MacOS/*
    mkdir release
    mv fm/QtFM.app release/
    hdiutil create -volname "QtFM $TAG" -srcfolder release -ov -format UDBZ $CWD/deploy/qtfm-$TAG-Mac.dmg
fi

if [ "${OS}" = "Linux" ]; then
    #LIN_CHECKSUM=`sha256sum qtfm-${TAG}-Linux.txz | awk '{print $1}'`
    echo "===> Linux checksum ${LIN_CHECKSUM}"
elif [ "${OS}" = "Darwin" ]; then
    MAC_CHECKSUM=`shasum -a 256 $CWD/deploy/qtfm-${TAG}-Mac.dmg | awk '{print $1}'`
    echo "===> Mac checksum ${MAC_CHECKSUM}"
fi

if [ "${TRAVIS_PULL_REQUEST}" != "false" ] && [ "${TRAVIS_PULL_REQUEST}" != "" ]; then
    echo "===> Uploading archives to transfer.sh ..."
    cd $CWD/deploy
    if [ "${OS}" = "Linux" ]; then
      #UPLOAD_LIN=`curl --upload-file ./qtfm-${TAG}-Linux.txz https://transfer.sh/qtfm-${TAG}-Linux.txz`
      echo "===> Linux snapshot ${UPLOAD_LIN}"
      #if [ "${UPLOAD_LIN}" != "" ]; then
      #    COMMENT="**CI:** Linux build is available at ${UPLOAD_LIN} with SHA256 checksum ${LIN_CHECKSUM}."
      #    curl -H "Authorization: token ${GITHUB_TOKEN}" -X POST -d "{\"body\": \"${COMMENT}\"}" "https://api.github.com/repos/${TRAVIS_REPO_SLUG}/issues/${TRAVIS_PULL_REQUEST}/comments"
      #fi
    elif [ "${OS}" = "Darwin" ]; then
      UPLOAD_MAC=`curl --upload-file ./qtfm-${TAG}-Mac.dmg https://transfer.sh/qtfm-${TAG}-Mac.dmg`
      echo "===> Mac snapshot ${UPLOAD_MAC}"
      if [ "${UPLOAD_MAC}" != "" ]; then
        COMMENT="**CI:** Mac build is available at ${UPLOAD_MAC} with SHA256 checksum ${MAC_CHECKSUM}."
        curl -H "Authorization: token ${GITHUB_TOKEN}" -X POST -d "{\"body\": \"${COMMENT}\"}" "https://api.github.com/repos/${TRAVIS_REPO_SLUG}/issues/${TRAVIS_PULL_REQUEST}/comments"
      fi
    fi
fi
