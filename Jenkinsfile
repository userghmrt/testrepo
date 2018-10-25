#!/usr/bin/env groovy

// This is the new jenkinsfile (for gitlab).
//
// The last old jenkinsfile (for github.com) was in commit:
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
    bat "\"C:\\Program Files (x86)\\Microsoft Visual Studio\\2017\\Community\\MSBuild\\15.0\\Bin\\MSBuild.exe\" tunserver.elf.vcxproj"
}

def run_unittests = {
    echo 'Build unittests'
    sh "git submodule update --init --recursive"
    sh "cmake ."
    sh "make unittests.elf -j2"
    sh "./unittests.elf --gtest_filter=*-privileges.*:utility.parse_ip_number:time_utils.daylight_saving"
}

def run_unittests_thread_ub = {
    echo 'Build unittests thread ub'
    sh "git submodule update --init --recursive"
    sh "cmake  -D CMAKE_C_COMPILER=clang -D CMAKE_CXX_COMPILER=clang++ -D SANITIZER_THREAD=ON -D SANITIZER_UNDEFINED_BEHAVIOR=ON ."
    sh "make -j2"
    sh "./qa/run-safe/run-safe-thread-ub TESTS --gtest_filter=*-privileges.*:utility.parse_ip_number:time_utils.daylight_saving"
}

def run_unittests_safe_memory = {
    echo 'Build unittests safe memory'
    sh "git submodule update --init --recursive"
    sh "cmake  -D CMAKE_C_COMPILER=clang -D CMAKE_CXX_COMPILER=clang++ ."
    sh "make -j2"
    sh "./qa/run-safe/run-safe-mem TESTS --gtest_filter=*-privileges.*:*abort*:utils_wrap_thread.*:utility.parse_ip_number:time_utils.daylight_saving"
}

////////////////////////////////////////////////////////////////

stage('Build') {
    parallel windows: {
        node('win64_MSVC') {
            stage('checkout') {
                setup()
            }
            gitlabBuilds(builds: ["Build_MSVC"]) {
                stage('Build_MSVC') {
                    try {
                        updateGitlabCommitStatus name: 'Build_MSVC', state: 'pending'
                            build_msvc()
                            currentBuild.result = 'SUCCESS'
                        updateGitlabCommitStatus name: 'Build_MSVC', state: 'success'
                    } catch (all) {
                        currentBuild.result = 'FAILURE'
                        updateGitlabCommitStatus name: 'Build_MSVC', state: 'failed'
                    }
                    deleteDir()
                } // stange
            }
        } // node msvc
        node('RB_win64_cygwin') {
            stage('checkout') {
                    setup()
            }
            gitlabBuilds(builds: ["Build_cygwin64", "Build_cygwin32"]) {
                stage('Build_cygwin64') {
                    try {
                        updateGitlabCommitStatus name: 'Build_cygwin64', state: 'pending'
                        build_cygwin_64bit()
                        currentBuild.result = 'SUCCESS'
                        updateGitlabCommitStatus name: 'Build_cygwin64', state: 'success'
                    } catch (all) {
                        currentBuild.result = 'FAILURE'
                        updateGitlabCommitStatus name: 'Build_cygwin64', state: 'failed'
                    }
                    sh "git clean -fdx"
                } // stange
                stage('Build_cygwin32') {
                    try {
                        updateGitlabCommitStatus name: 'Build_cygwin32', state: 'pending'
                        build_cygwin_32bit()
                        currentBuild.result = 'SUCCESS'
                        updateGitlabCommitStatus name: 'Build_cygwin32', state: 'success'
                    } catch (all) {
                        currentBuild.result = 'FAILURE'
                        updateGitlabCommitStatus name: 'Build_cygwin32', state: 'failed'
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
            gitlabBuilds(builds: ["Build_debian_clang", "Build_debian_gcc", "Run_Unittests" , "Run_Unittests_thread_ub" , "Run_Unittests_safe_memory"]) {
                stage('Build_debian_clang') {
                    try {
                        updateGitlabCommitStatus name: 'Build_debian_clang', state: 'pending'
                        build_clang()
                        currentBuild.result = 'SUCCESS'
                        updateGitlabCommitStatus name: 'Build_debian_clang', state: 'success'
                    } catch (all) {
                        currentBuild.result = 'FAILURE'
                        updateGitlabCommitStatus name: 'Build_debian_clang', state: 'failed'
                    }
                    sh "git clean -fdx"
                } // stange
                stage('Build_debian_gcc') {
                    try {
                        updateGitlabCommitStatus name: 'Build_debian_gcc', state: 'pending'
                        build_gcc()
                        currentBuild.result = 'SUCCESS'
                        updateGitlabCommitStatus name: 'Build_debian_gcc', state: 'success'
                    } catch (all) {
                        currentBuild.result = 'FAILURE'
                        updateGitlabCommitStatus name: 'Build_debian_gcc', state: 'failed'
                    }
                    sh "git clean -fdx"
                } // stange
                stage('Run_Unittests') {
                    try {
                        updateGitlabCommitStatus name: 'Run_Unittests', state: 'pending'
                        run_unittests()
                        currentBuild.result = 'SUCCESS'
                        updateGitlabCommitStatus name: 'Run_Unittests', state: 'success'
                    } catch (all) {
                        currentBuild.result = 'FAILURE'
                        updateGitlabCommitStatus name: 'Run_Unittests', state: 'failed'
                    }
                    sh "git clean -fdx"
                } // stage
                stage('Run_Unittests_thread_ub') {
                    try {
                        updateGitlabCommitStatus name: 'Run_Unittests_thread_ub', state: 'pending'
                        run_unittests_thread_ub()
                        currentBuild.result = 'SUCCESS'
                        updateGitlabCommitStatus name: 'Run_Unittests_thread_ub', state: 'success'
                    } catch (all) {
                        currentBuild.result = 'FAILURE'
                        updateGitlabCommitStatus name: 'Run_Unittests_thread_ub', state: 'failed'
                    }
                    sh "git clean -fdx"
                } // stage
                stage('Run_Unittests_safe_memory') {
                    try {
                        updateGitlabCommitStatus name: 'Run_Unittests_safe_memory', state: 'pending'
                        run_unittests_safe_memory()
                        currentBuild.result = 'SUCCESS'
                        updateGitlabCommitStatus name: 'Run_Unittests_safe_memory', state: 'success'
                    } catch (all) {
                        currentBuild.result = 'FAILURE'
                        updateGitlabCommitStatus name: 'Run_Unittests_safe_memory', state: 'failed'
                    }
                    sh "git clean -fdx"
                } // stage
                deleteDir()
            }
        }// node master
    },
    mac: {
        node('mac') {
        stage('setup') {
                setup()
        }
        gitlabBuilds(builds: ["Build_mac"]) {
            stage('Build_mac') {
                try {
                    updateGitlabCommitStatus name: 'Build_mac', state: 'pending'
                    build_clang()
                    currentBuild.result = 'SUCCESS'
                    updateGitlabCommitStatus name: 'Build_mac', state: 'success'
                } catch (all) {
                    currentBuild.result = 'FAILURE'
                    updateGitlabCommitStatus name: 'Build_mac', state: 'failed'
                }
            } // stange
            deleteDir()
            }
        } // node mac
    }
} // stage build

