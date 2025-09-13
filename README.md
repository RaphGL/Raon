<!-- PROJECT LOGO -->
<br />
<p align="center">
  <a href="https://github.com/RaphGL/Raon">
    <h1>Raon</h1>
  </a>

  <h3 align="center">A reasonable alternative to TOML and JSON</h3>
  <p align="center">
    <br />
    <a href="https://github.com/RaphGL/Raon"><strong>Explore the docs »</strong></a>
    <br />
    <br />
    ·
    <a href="https://github.com/RaphGL/Raon/issues">Report Bug</a>
    ·
    <a href="https://github.com/RaphGL/Raon/issues">Request Feature</a>
  </p>
</p>

Raon stand's for Raph's Object Notation but it also means "happy" or "joyful" in Korean.

Raon is a portable serialization format meant to take the best parts (in my opinion) of TOML and JSON and
create a language that's intuitive to use with no footguns. The intended use of this format is for configuration files
and for sending and receiving responses.

## Raon vs TOML

```python
workspace = {
  resolver = 2
  members = [
    # tidy-alphabetical-start
    "compiler/rustc"
    "src/build_helper"
    "src/rustc-std-workspace/rustc-std-workspace-alloc"
    "src/rustc-std-workspace/rustc-std-workspace-core"
    "src/rustc-std-workspace/rustc-std-workspace-std"
    "src/rustdoc-json-types"
    "src/tools/build-manifest"
    "src/tools/bump-stage0"
    "src/tools/cargotest"
    "src/tools/clippy"
    # tidy-alphabetical-end
  ]

  exclude = [
    "build"
    "compiler/rustc_codegen_cranelift"
    "compiler/rustc_codegen_gcc"
    "src/bootstrap"
    "tests/rustdoc-gui"
    # HACK(eddyb) This hardcodes the fact that our CI uses `/checkout/obj`.
    "obj"
  ]
}

profile.release.package = {
  rustc_thread_pool = {
    # The rustc fork of Rayon has deadlock detection code which intermittently
    # causes overflows in the CI (see https://github.com/rust-lang/rust/issues/90227)
    # so we turn overflow checks off for now.
    # FIXME: This workaround should be removed once #90227 is fixed.
    overflow-checks = false
  }

  # These are very thin wrappers around executing lld with the right binary name.
  # Basically nothing within them can go wrong without having been explicitly logged anyway.
  # We ship these in every rustc tarball and even after compression they add up
  # to around 0.6MB of data every user needs to download (and 15MB on disk).
  lld-wrapper = { debug = 0, strip = true }  

  wasm-component-ld-wrapper = {
    debug = 0
    strip = true
  }

  test-float-parse = {
    opt-level = 3
    codegen-units = 1
  }
}

# Speed up the binary as much as possible
profile.dev.package = {
  # Bigint libraries are slow without optimization, speed up testing
  test-float-parse = { opt-level = 3 }
}
```

```toml
[workspace]
resolver = "2"
members = [
    # tidy-alphabetical-start
    "compiler/rustc",
    "src/build_helper",
    "src/rustc-std-workspace/rustc-std-workspace-alloc",
    "src/rustc-std-workspace/rustc-std-workspace-core",
    "src/rustc-std-workspace/rustc-std-workspace-std",
    "src/rustdoc-json-types",
    "src/tools/build-manifest",
    "src/tools/bump-stage0",
    "src/tools/cargotest",
    "src/tools/clippy",
    # tidy-alphabetical-end
]

exclude = [
  "build",
  "compiler/rustc_codegen_cranelift",
  "compiler/rustc_codegen_gcc",
  "src/bootstrap",
  "tests/rustdoc-gui",
  # HACK(eddyb) This hardcodes the fact that our CI uses `/checkout/obj`.
  "obj",
]

[profile.release.package.rustc_thread_pool]
# The rustc fork of Rayon has deadlock detection code which intermittently
# causes overflows in the CI (see https://github.com/rust-lang/rust/issues/90227)
# so we turn overflow checks off for now.
# FIXME: This workaround should be removed once #90227 is fixed.
overflow-checks = false

# These are very thin wrappers around executing lld with the right binary name.
# Basically nothing within them can go wrong without having been explicitly logged anyway.
# We ship these in every rustc tarball and even after compression they add up
# to around 0.6MB of data every user needs to download (and 15MB on disk).
[profile.release.package.lld-wrapper]
debug = 0
strip = true
[profile.release.package.wasm-component-ld-wrapper]
debug = 0
strip = true

# Bigint libraries are slow without optimization, speed up testing
[profile.dev.package.test-float-parse]
opt-level = 3

# Speed up the binary as much as possible
[profile.release.package.test-float-parse]
opt-level = 3
codegen-units = 1
```
## License

Distributed under MIT License. See [`LICENSE`](https://github.com/RaphGL/Raon/blob/main/LICENSE) for more information.

<!-- ACKNOWLEDGEMENTS -->

