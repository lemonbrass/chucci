cmake -B build -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DCMAKE_BUILD_TYPE=Debug && ln -sf build/compile_commands.json .
cd build
make -j$(nproc)


for arg in "$@"; do
    if [ $arg == "test" ]; then
      valgrind --leak-check=full ./test/all_tests
    fi
done

