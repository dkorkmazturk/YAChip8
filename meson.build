project('YAChip8',
        'cpp',
        default_options : 'cpp_std=c++17')

sdl2_dep   = dependency('sdl2')
thread_dep = dependency('threads')
boost_dep  = dependency('boost', modules : ['program_options'])

srcs = ['src/main.cpp', 'src/chip8.cpp', 'src/gui.cpp']

executable('yachip8',
            sources : srcs,
            dependencies : [sdl2_dep, thread_dep, boost_dep])
