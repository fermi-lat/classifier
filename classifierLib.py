# $Header: /nfs/slac/g/glast/ground/cvs/GlastRelease-scons/classifier/classifierLib.py,v 1.4 2009/11/10 01:20:50 jrb Exp $
def generate(env, **kw):
    if not kw.get('depsOnly', 0):
        env.Tool('addLibrary', library = ['classifier'])
        if env['PLATFORM'] == "win32" and env.get('CONTAINERNAME','') == 'GlastRelease':
            env.Tool('findPkgPath', package = 'classifier') 
    env.Tool('addLibrary', library = env['clhepLibs'])
    env.Tool('addLibrary', library = env['rootLibs'])
    # no need for incsOnly section since classifier doesn't reference
    # other packages
def exists(env):
    return 1;
