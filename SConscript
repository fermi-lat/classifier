# -*- python -*-
# $Header$
# Authors: T. Burnett <tburnett@u.washington.edu>
# Version: classifier-06-01-01
Import('baseEnv')
Import('listFiles')
Import('packages')
progEnv = baseEnv.Clone()
libEnv = baseEnv.Clone()

libEnv.Tool('classifierLib', depsOnly = 1)

classifier = libEnv.StaticLibrary('classifier', listFiles(['classifier/*.cpp']) + listFiles(['src/*.cpp']))


progEnv.Tool('classifierLib')
test_classifier = progEnv.Program('test_classifier', listFiles(['src/test/*.cpp']))
progEnv.Tool('registerObjects', package = 'classifier', libraries = [classifier], testApps = [test_classifier])
