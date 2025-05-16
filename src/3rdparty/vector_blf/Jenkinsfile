// SPDX-FileCopyrightText: 2013-2021 Tobias Lorenz <tobias.lorenz@gmx.net>
//
// SPDX-License-Identifier: GPL-3.0-or-later

pipeline {
    agent none
    stages {
        stage('Build and Test') {
            parallel {
                stage('Debian Testing') {
                    agent {
                        dockerfile {
                            filename 'debian_testing'
                            label 'linux'
                            dir 'Dockerfiles'
                        }
                    }
                    steps {
                        sh '''
                            cmake --version
                            gcc --version
                            g++ --version
                        '''
                        dir('build-debian_testing') {
                            sh '''
                                cmake \
                                    -DCMAKE_INSTALL_PREFIX:PATH=sysroot \
                                    -DOPTION_BUILD_EXAMPLES=ON \
                                    -DOPTION_BUILD_TESTS=ON \
                                    -DOPTION_RUN_CCCC=ON \
                                    -DOPTION_RUN_CPPCHECK=ON \
                                    -DOPTION_USE_GCOV=ON \
                                    -DOPTION_USE_GPROF=OFF \
                                    -DOPTION_ADD_LCOV=ON \
                                    -DOPTION_RUN_DOXYGEN=ON \
                                    ..
                            '''
                            sh 'cmake --build .'
                            sh 'ctest'
                            sh 'make install'
                        }
                    }
                    post {
                        success {
                            archiveArtifacts artifacts: 'build-debian_testing/sysroot/**'
                            publishHTML target: [
                                reportName: 'Doxygen',
                                reportDir: 'build-debian_testing/src/Vector/BLF/docs/doxygen/html',
                                reportFiles: 'index.html',
                                includes: '**'
                            ]
                        }
                    }
                }
                stage('Debian 9 Stretch Backports') {
                    agent {
                        dockerfile {
                            filename 'debian_stretch-backports'
                            label 'linux'
                            dir 'Dockerfiles'
                        }
                    }
                    steps {
                        sh '''
                            cmake --version
                            gcc --version
                            g++ --version
                        '''
                        dir('build-debian_stretch-backports') {
                            sh '''
                                cmake \
                                    -DCMAKE_INSTALL_PREFIX:PATH=sysroot \
                                    -DOPTION_BUILD_EXAMPLES=ON \
                                    -DOPTION_BUILD_TESTS=ON \
                                    -DOPTION_RUN_CCCC=OFF \
                                    -DOPTION_RUN_CPPCHECK=OFF \
                                    -DOPTION_USE_GCOV=OFF \
                                    -DOPTION_USE_GPROF=OFF \
                                    -DOPTION_ADD_LCOV=OFF \
                                    -DOPTION_RUN_DOXYGEN=OFF \
                                    ..
                            '''
                            sh 'cmake --build .'
                            sh 'ctest'
                        }
                    }
                }
                stage('Debian 10 Buster') {
                    agent {
                        dockerfile {
                            filename 'debian_buster'
                            label 'linux'
                            dir 'Dockerfiles'
                        }
                    }
                    steps {
                        sh '''
                            cmake --version
                            gcc --version
                            g++ --version
                        '''
                        dir('build-debian_buster') {
                            sh '''
                                cmake \
                                    -DCMAKE_INSTALL_PREFIX:PATH=sysroot \
                                    -DOPTION_BUILD_EXAMPLES=ON \
                                    -DOPTION_BUILD_TESTS=ON \
                                    -DOPTION_RUN_CCCC=OFF \
                                    -DOPTION_RUN_CPPCHECK=OFF \
                                    -DOPTION_USE_GCOV=OFF \
                                    -DOPTION_USE_GPROF=OFF \
                                    -DOPTION_ADD_LCOV=OFF \
                                    -DOPTION_RUN_DOXYGEN=OFF \
                                    ..
                            '''
                            sh 'cmake --build .'
                            sh 'ctest'
                        }
                    }
                }
                stage('Visual Studio 2017') {
                    agent {
                        /* disabled till this is solved: https://issues.jenkins-ci.org/browse/JENKINS-36776
                        dockerfile {
                            filename 'windowsservercore_1709-visualstudio_2017'
                            label 'windows'
                            dir 'Dockerfiles'
                        }
                        */
                        node {
                            label 'windows'
                            customWorkspace 'C:\\workspace\\Vector_BLF'
                        }
                    }
                    steps {
                        bat '''
                            cmake --version
                        '''
                        dir('build-windowsservercore_1709-visualstudio_2017') {
                            bat '''
                                cmake \
                                    -DCMAKE_INSTALL_PREFIX:PATH=sysroot \
                                    -DOPTION_BUILD_EXAMPLES=ON \
                                    -DOPTION_BUILD_TESTS=OFF \
                                    -DOPTION_RUN_CCCC=OFF \
                                    -DOPTION_RUN_CPPCHECK=OFF \
                                    -DOPTION_USE_GCOV=OFF \
                                    -DOPTION_USE_GPROF=OFF \
                                    -DOPTION_ADD_LCOV=OFF \
                                    -DOPTION_RUN_DOXYGEN=OFF \
                                    ..
                            '''
                            bat 'cmake --build .'
                        }
                    }
                }
            }
        }
    }
}
