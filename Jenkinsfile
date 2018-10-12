#!/usr/bin/env groovy

// new jenkinsfile for gitlab
// last old jenkinsfile commit
// commit 3303304be14d3637fb5aea3339c2ef9adc71f542
// Merge: c4be227 33a3d24
// Author: Tigusoft Admin <admin@tigusoft.pl>
// Date:   Wed Oct 10 15:14:46 2018 +0100

def handleCheckout = {
  if (env.gitlabMergeRequestId) {
    sh "echo 'Merge request detected. Merging...'"
    def credentialsId = scm.userRemoteConfigs[0].credentialsId
    checkout ([
      $class: 'GitSCM',
      branches: [[name: "${env.gitlabSourceNamespace}/${env.gitlabSourceBranch}"]],
      extensions: [
        [$class: 'PruneStaleBranch'],
        [$class: 'CleanCheckout'],
        [
          $class: 'PreBuildMerge',
          options: [
            fastForwardMode: 'NO_FF',
            mergeRemote: env.gitlabTargetNamespace,
            mergeTarget: env.gitlabTargetBranch
          ]
        ]
      ],
      userRemoteConfigs: [
        [
          credentialsId: credentialsId,
          name: env.gitlabTargetNamespace,
          url: env.gitlabTargetRepoHttpUrl
        ],
        [
          credentialsId: credentialsId,
          name: env.gitlabSourceNamespace,
          url: env.gitlabSourceRepoHttpUrl
        ]
      ]
    ])
  } else {
    sh "echo 'No merge request detected. Checking out current branch'"
    checkout ([
      $class: 'GitSCM',
      branches: scm.branches,
      extensions: [
          [$class: 'PruneStaleBranch'],
          [$class: 'CleanCheckout']
      ],
      userRemoteConfigs: scm.userRemoteConfigs
    ])
  }
}

def setup = {
  try {
    sh "env | sort"
    checkout scm
    handleCheckout()
    sh "git branch -vv"
    currentBuild.result = 'SUCCESS'
  } catch (all) {
    currentBuild.result = 'FAILURE'
  }
}

def build_clang = {
    echo 'Build using clang'
    sh "git submodule update --init --recursive"
    sh "cmake  -D CMAKE_C_COMPILER=clang -D CMAKE_CXX_COMPILER=clang++ ."
    sh "make -j4 tunserver.elf"
}

def build_gcc = {
    echo 'Build using gcc'
    sh "git submodule update --init --recursive"
    sh "cmake  -D CMAKE_C_COMPILER=gcc -D CMAKE_CXX_COMPILER=g++ ."
    sh "make -j4 tunserver.elf"
}

def build_cygwin_64bit = {
    echo 'Build using cygwin 64bit compiler'
    sh "git submodule update --init --recursive"
    sh "cmake -DCMAKE_TOOLCHAIN_FILE=cmake/toolchain_cygwin_64bit.cmake.in ."
    sh "make -j4 tunserver.elf"
}

def build_cygwin_32bit = {
    echo 'Build using cygwin 32bit compiler'
    sh "git submodule update --init --recursive"
    sh "cmake -DCMAKE_TOOLCHAIN_FILE=cmake/toolchain_cygwin_32bit.cmake.in ."
    sh "make -j4 tunserver.elf"
}

def build_msvc = {
    echo 'Build using MSVC compiler'
    bat "git submodule update --init --recursive"
    bat "cmake -G \"Visual Studio 15 2017 Win64\" -DBOOST_LIBRARYDIR=\"C:\\msvc2017_libs\\boost_1_64_0\\lib64-msvc-14.1\" -DSODIUM_ROOT_DIR=\"C:\\msvc2017_libs\\libsodium-1.0.13\" -DBOOST_ROOT=\"C:\\msvc2017_libs\\boost_1_64_0\" -DSODIUM_LIBRARY=\"C:\\msvc2017_libs\\libsodium-1.0.13\\x64\\Debug\\v141\\dynamic\\libsodium.lib\" ."
    //bat "msbiuld tunserver.elf.vcxproj"
    bat "\"C:\\Program Files (x86)\\Microsoft Visual Studio\\2017\\Community\\MSBuild\\15.0\\Bin\\MSBuild.exe\" tunserver.elf.vcxproj"
}
////////////////////////////////////////////////////////////////

stage('Build') {
    parallel windows: {
        node('win64_MSVC') {
            stage('checkout') {
                setup()
            }
            gitlabBuilds(builds: ["Build MSVC"]) {
                stage('Build MSVC') {
                    try {
                        updateGitlabCommitStatus name: 'Build MSVC', state: 'pending'
                            build_msvc()
                            currentBuild.result = 'SUCCESS'
                        updateGitlabCommitStatus name: 'Build MSVC', state: 'success'
                    } catch (all) {
                        currentBuild.result = 'FAILURE'
                        updateGitlabCommitStatus name: 'Build MSVC', state: 'failed'
                    }
                    deleteDir()
                } // stange
            }
        } // node msvc
        node('RB_win64_cygwin') {
            stage('checkout') {
                    setup()
            }
            gitlabBuilds(builds: ["Build cygwin64", "Build cygwin32"]) {
                stage('Build cygwin64') {
                    try {
                        updateGitlabCommitStatus name: 'Build cygwin64', state: 'pending'
                        build_cygwin_64bit()
                        currentBuild.result = 'SUCCESS'
                        updateGitlabCommitStatus name: 'Build cygwin64', state: 'success'
                    } catch (all) {
                        currentBuild.result = 'FAILURE'
                        updateGitlabCommitStatus name: 'Build cygwin64', state: 'failed'
                    }
                    sh "git clean -fdx"
                } // stange
                stage('Build cygwin32') {
                    try {
                        updateGitlabCommitStatus name: 'Build cygwin32', state: 'pending'
                        build_cygwin_32bit()
                        currentBuild.result = 'SUCCESS'
                        updateGitlabCommitStatus name: 'Build cygwin32', state: 'success'
                    } catch (all) {
                        currentBuild.result = 'FAILURE'
                        updateGitlabCommitStatus name: 'Build cygwin32', state: 'failed'
                    }
                    sh "git clean -fdx"
                } // stange
                deleteDir()
            }
        }
    },
    linux: {
        node('master') {
            sh "echo node master"
            stage('checkout') {
                    setup()
            }
            gitlabBuilds(builds: ["Build debian clang", "Build debian gcc"]) {
                stage('Build debian clang') {
                    try {
                        updateGitlabCommitStatus name: 'Build debian clang', state: 'pending'
                        build_clang()
                        currentBuild.result = 'SUCCESS'
                        updateGitlabCommitStatus name: 'Build debian clang', state: 'success'
                    } catch (all) {
                        currentBuild.result = 'FAILURE'
                        updateGitlabCommitStatus name: 'Build debian clang', state: 'failed'
                    }
                    sh "git clean -fdx"
                } // stange
                stage('Build debian gcc') {
                    try {
                        updateGitlabCommitStatus name: 'Build debian gcc', state: 'pending'
                        build_gcc()
                        currentBuild.result = 'SUCCESS'
                        updateGitlabCommitStatus name: 'Build debian gcc', state: 'success'
                    } catch (all) {
                        currentBuild.result = 'FAILURE'
                        updateGitlabCommitStatus name: 'Build debian gcc', state: 'failed'
                    }
                    sh "git clean -fdx"
                } // stange
                deleteDir()
            }
        }// node master
    },
    mac: {
        node('mac') {
        stage('setup') {
                setup()
        }
        gitlabBuilds(builds: ["Build mac"]) {
            stage('Build mac') {
                try {
                    updateGitlabCommitStatus name: 'Build mac', state: 'pending'
                    build_clang()
                    currentBuild.result = 'SUCCESS'
                    updateGitlabCommitStatus name: 'Build mac', state: 'success'
                } catch (all) {
                    currentBuild.result = 'FAILURE'
                    updateGitlabCommitStatus name: 'Build mac', state: 'failed'
                }
            } // stange
            deleteDir()
            }
        } // node mac
    }
} // stage build
