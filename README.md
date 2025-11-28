# How to run


```bash
# Build tools:
bazel build //tools:all

# Run binary:
./bazel-bin/tools/comp_metrics 

# Generate compile_commands.json for your IDE:
bazel run @hedron_compile_commands//:refresh_all
```
