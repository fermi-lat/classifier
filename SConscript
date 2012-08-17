# -*- python -*-
# $Header: /nfs/slac/g/glast/ground/cvs/GlastRelease-scons/classifier/SConscript,v 1.12 2012/06/28 21:51:53 jrb Exp $
# Authors: T. Burnett <tburnett@u.washington.edu>
# Version: classifier-01-06-07
Import('baseEnv')
Import('listFiles')
Import('packages')
progEnv = baseEnv.Clone()
libEnv = baseEnv.Clone()

libEnv.Tool('addLinkDeps', package = 'classifier', toBuild='static')
classifier = libEnv.StaticLibrary('classifier',
                                  listFiles(['classifier/*.cpp', 'src/*.cpp']))


progEnv.Tool('classifierLib')
test_classifier = progEnv.Program('test_classifier',
                                  listFiles(['src/test/*.cpp']))
progEnv.Tool('registerTargets', package = 'classifier',
             staticLibraryCxts = [[classifier, libEnv]],
             testAppCxts = [[test_classifier, progEnv]],
             includes = listFiles(['classifier/*.h']))




