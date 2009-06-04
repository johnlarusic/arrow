##############################################################################
# BUILD LOCATIONS
##############################################################################
options_file = 'build_options.py'

dir_src = 'src'
lib_src = dir_src + '/lib'
bin_src = dir_src + '/bin'
inc_src = dir_src + '/include'

dir_build = 'build'
lib_build = dir_build + '/lib'
bin_build = dir_build + '/bin'
inc_build = dir_build + '/include'

dir_bin = 'bin'


# List of library files
lib_files = (
    'baltsp/baltsp.c',
    'btsp/btsp.c',
    'btsp/feasible.c',
    'btsp/fun.c',
    'btsp/fun_cbtsp.c',
    'btsp/fun_btsp.c',
    'btsp/result.c',
    'btsp/params.c',
    'btsp/solve_plan.c',
    'common/bintree.c', 
    'common/hash.c',
    'common/heap.c',
    'common/llist.c',
    'common/options.c', 
    'common/problem.c', 
    'common/util.c',
    'common/xml.c',
    'lb/2mb.c', 
    'lb/bap.c', 
    'lb/bbssp.c', 
    'lb/bscssp.c', 
    'lb/cbap.c',
    'lb/cbst.c',
    'lb/dcbpb.c',
    'tsp/cc.c',
    'tsp/rai.c',
    'tsp/result.c',
    'tsp/tsp.c'
)

# List of executables in the form of (NAME, SRC)
executables = (
    ('2mb',         '2mb.c'),
    ('abtsp-rai',   'abtsp-rai.c'),
    ('baltsp',      'baltsp.c'),
    ('bap',         'bap.c'),
    ('bbssp',       'bbssp.c'),
    ('bscssp',      'bscssp.c'),
    ('btsp',        'btsp.c'),
    ('cbap',        'cbap.c'),
    ('cbst',        'cbst.c'),
    ('cbtsp',       'cbtsp.c'),
    ('cost-list',   'cost-list.c'),
    ('dcbpb',       'dcbpb.c'),
    ('delta-print', 'delta-print.c'),
    ('hash',        'hash.c'),
    ('histdata',    'histdata.c'),
    ('subprob',     'subprob.c'),
    ('tourinfo',    'tourinfo.c'),
    ('tsp',         'tsp.c')
)

##############################################################################
# GET USERS OPTIONS
##############################################################################
opts = Variables(options_file)
opts.AddVariables(
    PathVariable('concorde_a', 'Path to Concorde library (concorde.a)', 
                 '.', PathVariable.PathIsFile),
    PathVariable('concorde_h_dir', 'Path to Concorde header directory (concorde.h)',
                 '.', PathVariable.PathIsDir),
    PathVariable('lpsolver_a', 'Path to LP Solver library (qsopt.a/libcplex.a)', 
                 '.', PathVariable.PathIsFile),
    PathVariable('lpsolver_h_dir', 'Path to LP Solver header (qsopt.h/cplex80.h)',
                 '.', PathVariable.PathIsDir),
    ('lib_ccflags', 'General options to pass to C compiler for library', ''),
    ('bin_ccflags', 'General options to pass to C compiler for binaries', ''),
    ('bin_linkflags', 'General options to pass to linker for binaries', '')
)

##############################################################################
# COMPILE THE CALLABLE LIBRARY
##############################################################################
# Add full paths to library files
lib_files_fullpath = []
for lib_file in lib_files:
    lib_files_fullpath.append(lib_build + '/' + lib_file)

# Setup environment and build library
env_lib = Environment(
            variables = opts,
            CCFLAGS = '$lib_ccflags',
            CPPPATH = ['"$concorde_h_dir"', '"$lpsolver_h_dir"', inc_build]
          )
env_lib.BuildDir(dir_build, dir_src)
env_lib.Library(target = 'arrow', 
                source = lib_files_fullpath)

##############################################################################
# COMPILE THE EXECUTABLES
##############################################################################
env_bin = Environment(
            variables = opts,
            CCFLAGS = '$bin_ccflags',
            LINKFLAGS = '$bin_linkflags',
            CPPPATH = ['"$concorde_h_dir"', '"$lpsolver_h_dir"', inc_build],
            LIBS = ['arrow', 'cmph'],
            LIBPATH = ['.']
          )
env_bin.Append(LIBS = [File(env_bin.subst('$concorde_a'))])
env_bin.Append(LIBS = [File(env_bin.subst('$lpsolver_a'))])
env_bin.BuildDir(bin_build, bin_src)

for (name, src) in executables:
    env_bin.Program(dir_bin + '/' + name, bin_build + '/' + src)

##############################################################################
# RUN TEST FRAMEWORK
##############################################################################
#if 'check' in COMMAND_LINE_TARGETS:
#    print "Run unit tests!"