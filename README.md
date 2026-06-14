<!-- PROJECT LOGO -->
<br />
<h3 align="center">
  <a href="https://github.com/RaphGL/Raon">
    <h1>Raon</h1>
  </a>
</h3>

<h3 align="center">A portable serialization format</h3>
<p align="center">
  <br />
  <a href="https://github.com/RaphGL/Raon"><strong>Read the specification »</strong></a>
  <br />
  <br />
  ·
  <a href="https://github.com/RaphGL/Raon/issues">Report Bug</a>
  ·
  <a href="https://github.com/RaphGL/Raon/issues">Request Feature</a>
</p>

Raon stands for Raph's Object Notation but it also means "happy" or "joyful" in Korean.

Raon is a portable serialization format inspired by formats such as JSON, TOML and KDL. It is meant to be:
- intuitive: people should generally just understand what is going on without having to learn anything new
- have no footguns: things are what they are, we should never be too clever
- expressive: if you need to express nested data, you should be able to do so without verbosity

## Raon vs TOML
While the project's goal is not to just provide a subset/superset of another format. Raon is still mostly compatible, except for certain data types that TOML supports and Raon doesn't.
Here's a stripped down example from the ripgrep repo in both TOML and Raon:

```toml
[package]
name = "grep-cli"
version = "0.1.12" #:version
license = "Unlicense OR MIT"
edition = "2024"

[dependencies]
bstr = { version = "1.6.2", features = ["std"] }
globset = { version = "0.4.18", path = "../globset" }
log = "0.4.20"
termcolor = "1.3.0"

[target.'cfg(windows)'.dependencies.winapi-util]
version = "0.1.6"

[target.'cfg(unix)'.dependencies.libc]
version = "0.2.148"
```

```toml
package = {
  name = "grep-cli"
  version = "0.1.12" #:version
  license = "Unlicense OR MIT"
  edition = "2024"
}

dependencies = {
  bstr = { version = "1.6.2", features = ["std"] }
  globset = { version = "0.4.18", path = "../globset" }
  log = "0.4.20"
  termcolor = "1.3.0"
}

target = {
  "cfg(windows)".dependencies.winapi-util = {
    version = "0.1.6"
  }

  "cfg(unix)".dependencies.libc = {
    version = "0.2.148"
  }
}
```

## License

Distributed under MIT License. See [`LICENSE`](https://github.com/RaphGL/Raon/blob/main/LICENSE) for more information.

<!-- ACKNOWLEDGEMENTS -->

