pip3 install --user pyelftools

mkdir -p build; cd build;

cmake -DBOARD=native_posix ..

make zephyr/include/generated/autoconf.h
make syscall_list_h_target
make syscall_macros_h_target
make kobj_types_h_target
