project(
  'deinvert',
  'cpp',
  default_options: [
    'warning_level=3',
    'buildtype=release',
    'optimization=3',
    'prefix=/usr/local',
    'cpp_std=c++14',
  ],
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
  override_options = ['cpp_std=gnu++14']
else
  override_options = []
endif

####################
### Dependencies ###
####################

# Find libsndfile
sndfile = dependency('sndfile')

# Find liquid-dsp
liquid = cc.find_library('liquid', required: false)
# macOS: The above mechanism sometimes fails, so let's look deeper
if not liquid.found() and build_machine.system() == 'darwin'
  fs = import('fs')
  brew = find_program('brew', required: false)
  if brew.found()
    # Homebrew system
    liquid_prefix = run_command(brew, '--prefix', 'liquid-dsp', check: true).stdout().strip()
    liquid_lib = cc.find_library('liquid', dirs: [liquid_prefix + '/lib'])
    liquid_inc = include_directories(liquid_prefix + '/include')
    liquid = declare_dependency(dependencies: liquid_lib, include_directories: liquid_inc)
  elif fs.is_dir('/opt/local/lib')
    # MacPorts system
    liquid_lib = cc.find_library('liquid', dirs: ['/opt/local/lib'])
    liquid_inc = include_directories('/opt/local/include')
    liquid = declare_dependency(dependencies: liquid_lib, include_directories: liquid_inc)
  endif
endif

############################
### Sources & Executable ###
############################

sources = [
  'src/deinvert.cc',
  'src/liquid_wrappers.cc',
]

executable(
  'deinvert',
  sources,
  dependencies: [liquid, sndfile],
  install: true,
  override_options: override_options,
)
