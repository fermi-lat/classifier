# $Header: /nfs/slac/g/glast/ground/cvs/GlastRelease-scons/classifier/classifierLib.py,v 1.3 2009/01/23 00:20:53 ecephas Exp $
def generate(env, **kw):
    if not kw.get('depsOnly', 0):
        env.Tool('addLibrary', library = ['classifier'])
    env.Tool('addLibrary', library = env['clhepLibs'])
    env.Tool('addLibrary', library = env['rootLibs'])
def exists(env):
    return 1;
