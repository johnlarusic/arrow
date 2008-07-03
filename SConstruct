# Where everything is located
dir_src = 'src'
lib_src = dir_src + '/lib'
bin_src = dir_src + '/bin'

dir_build = 'build'
lib_build = dir_build + '/lib'
bin_build = dir_build + '/bin'

# Ask the user for options
opts = Options('build_options.py')
opts.AddOptions(
    PathOption('concorde_a', 'Path to Concorde library (concorde.a)', 
               '.', PathOption.PathIsFile),
    PathOption('concorde_h_dir', 'Path to Concorde header directory (concorde.h)',
               '.', PathOption.PathIsDir),
    PathOption('qsopt_a', 'Path to QSopt library (qsopt.a)', 
               '.', PathOption.PathIsFile),
    PathOption('qsopt_h_dir', 'Path to QSopt header (qsopt.h)',
               '.', PathOption.PathIsDir)
)

# Compile the callable library
env_lib = Environment(options = opts,
                      CPPPATH = ['"$concorde_h_dir"', '"$qsopt_h_dir"'])
env_lib.BuildDir(lib_build, lib_src)
env_lib.Library(target = 'arrow', 
                source = Glob(lib_build + '/*.c') + [lib_build + '/arrow.h'])

# Compile all the programs
env_bin = Environment(options = opts,
                      CPPPATH = ['"$concorde_h_dir"', '"$qsopt_h_dir"', lib_build],
                      LIBS = ['arrow'],
                      LIBPATH = ['.'])
env_bin.Append(LIBS = [File(env_bin.subst('$concorde_a'))])
env_bin.Append(LIBS = [File(env_bin.subst('$qsopt_a'))])
env_bin.BuildDir(bin_build, bin_src)

env_bin.Program('2mb', bin_build + '/2mb.c')
env_bin.Program('bap', bin_build + '/bap.c')
env_bin.Program('bbssp', bin_build + '/bbssp.c')
env_bin.Program('bscssp', bin_build + '/bscssp.c')
env_bin.Program('btsp', bin_build + '/btsp.c')
env_bin.Program('cbtsp', bin_build + '/cbtsp.c')
env_bin.Program('dcbpb', bin_build + '/dcbpb.c')
env_bin.Program('histogram_data', bin_build + '/histogram_data.c')
env_bin.Program('abtsp', bin_build + '/abtsp.c')
#env_bin.Program('linkern', bin_build + '/linkern.c')
#env_bin.Program('tsp', bin_build + '/tsp.c')