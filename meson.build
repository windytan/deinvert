project(
  'deinvert',
  'cpp',
  default_options: ['warning_level=3', 'buildtype=release', 'optimization=3'],
  version: '1.0',
)

# Store version number to be compiled in
conf = configuration_data()
conf.set_quoted('VERSION', meson.project_version())
configure_file(output: 'config.h', configuration: conf)

########################
### Compiler options ###
########################

cc = meson.get_compiler('cpp')
add_project_arguments(cc.get_supported_arguments(['-Wno-unknown-pragmas']), language: 'cpp')

# We want to use M_PI on Windows
if build_machine.system() == 'windows'
  add_project_arguments('-D_USE_MATH_DEFINES=1', language: 'cpp')
endif

# Explicit GNU extensions on Cygwin
if build_machine.system() == 'cygwin'
  add_project_arguments('-std=gnu++14', language: 'cpp')
else
  add_project_arguments('-std=c++14', language: 'cpp')
endif

####################
### Dependencies ###
####################

# Find libsndfile
sndfile = dependency('sndfile')

# Find liquid-dsp
if build_machine.system() == 'darwin'
  fs = import('fs')
  # Homebrew system
  if fs.is_dir('/opt/homebrew/lib')
    liquid_lib = cc.find_library('liquid', dirs: ['/opt/homebrew/lib'])
    liquid_inc = include_directories('/opt/homebrew/include')
    # MacPorts system
  else
    liquid_lib = cc.find_library('liquid', dirs: ['/opt/local/lib'])
    liquid_inc = include_directories('/opt/local/include')
  endif
  liquid = declare_dependency(dependencies: liquid_lib, include_directories: liquid_inc)
else
  liquid = cc.find_library('liquid')
endif

############################
### Sources & Executable ###
############################

sources = [
  'src/deinvert.cc',
  'src/liquid_wrappers.cc',
]

executable('deinvert', sources, dependencies: [liquid, sndfile], install: true)
