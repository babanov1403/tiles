# bazel-project-template

Install Bazel build system: https://bazel.build/install

```bash
# Build library:
bazel build //:all

# Run tests and benchmark:
bazel run //:tests
bazel run //:benchmark

# Build tools:
bazel build //tools:all

# Run binary:
bazel-bin/tools/say_hello --user World

# Run benchmarks, clang-format and clang-tidy checks:
bazel build --config=clang-format -k //...
bazel build --config=clang-tidy -k //...

# Generate compile_commands.json for your IDE:
bazel run @hedron_compile_commands//:refresh_all
bazel run @bazel-compile-commands//bcc:bazel-compile-commands-bin
```
