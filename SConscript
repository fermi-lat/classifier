# -*- python -*-
# $Header: /nfs/slac/g/glast/ground/cvs/classifier/SConscript,v 1.9 2010/06/12 23:31:27 glastrm Exp $
# Authors: T. Burnett <tburnett@u.washington.edu>
# Version: classifier-01-06-05
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




