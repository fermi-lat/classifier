# -*- python -*-
# $Header: /nfs/slac/g/glast/ground/cvs/classifier/SConscript,v 1.11 2012/01/23 19:29:09 jrb Exp $
# Authors: T. Burnett <tburnett@u.washington.edu>
# Version: classifier-01-06-07
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




