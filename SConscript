# -*- python -*-
# $Header: /nfs/slac/g/glast/ground/cvs/GlastRelease-scons/classifier/SConscript,v 1.3 2009/01/23 00:06:53 ecephas Exp $
# Authors: T. Burnett <tburnett@u.washington.edu>
# Version: classifier-01-06-01
Import('baseEnv')
Import('listFiles')
Import('packages')
progEnv = baseEnv.Clone()
libEnv = baseEnv.Clone()

libEnv.Tool('classifierLib', depsOnly = 1)

classifier = libEnv.StaticLibrary('classifier', listFiles(['classifier/*.cpp']) + listFiles(['src/*.cpp']))


progEnv.Tool('classifierLib')
test_classifier = progEnv.Program('test_classifier', listFiles(['src/test/*.cpp']))
progEnv.Tool('registerTargets', package = 'classifier',
             staticLibraryCxts = [[classifier, libEnv]],
             testAppCxts = [[test_classifier, progEnv]],
             includes = listFiles(['classifier/*.h']))




