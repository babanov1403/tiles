# How to run


```bash
# Build tools:
bazel build //tools:all

# Run binary:
bazel-bin/tools/comp_mertics --param1=param1 --param2=param2

# Run benchmarks, clang-format and clang-tidy checks:
bazel build --config=clang-format -k //...
bazel build --config=clang-tidy -k //...

# Generate compile_commands.json for your IDE:
bazel run @hedron_compile_commands//:refresh_all
bazel run @bazel-compile-commands//bcc:bazel-compile-commands-bin
```
