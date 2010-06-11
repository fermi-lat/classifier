# -*- python -*-
# $Header: /nfs/slac/g/glast/ground/cvs/GlastRelease-scons/classifier/SConscript,v 1.5 2009/11/10 01:21:57 jrb Exp $
# Authors: T. Burnett <tburnett@u.washington.edu>
# Version: classifier-01-06-02
Import('baseEnv')
Import('listFiles')
Import('packages')
progEnv = baseEnv.Clone()
libEnv = baseEnv.Clone()

classifier = libEnv.StaticLibrary('classifier',
                                  listFiles(['classifier/*.cpp', 'src/*.cpp']))


progEnv.Tool('classifierLib')
test_classifier = progEnv.Program('test_classifier',
                                  listFiles(['src/test/*.cpp']))
progEnv.Tool('registerTargets', package = 'classifier',
             staticLibraryCxts = [[classifier, libEnv]],
             testAppCxts = [[test_classifier, progEnv]],
             includes = listFiles(['classifier/*.h']))




