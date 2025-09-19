<!-- PROJECT LOGO -->
<br />
<p align="center">
  <a href="https://github.com/RaphGL/Raon">
    <h1>Raon</h1>
  </a>
</p>

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

Raon stand's for Raph's Object Notation but it also means "happy" or "joyful" in Korean.

Raon is a portable serialization format meant to take the best parts (in my opinion) of TOML and JSON and
create a language that's intuitive to use with no footguns. The intended use of this format is for configuration files
and for sending and receiving responses.

## Raon vs TOML

```c
workspace = {
  members = [
    "alacritty",
    "alacritty_terminal",
    "alacritty_config",
    "alacritty_config_derive",
  ]
  resolver = "2"

  dependencies = {
    toml = "0.9.2"
    toml_edit = "0.23.1"
  }

  package = {
    edition = "2024"
    rust-version = "1.85.0"
  }
}

profile.release = {
  lto = "thin"
  debug = 1
  incremental = false
}
```

```toml
[workspace]
members = [
    "alacritty",
    "alacritty_terminal",
    "alacritty_config",
    "alacritty_config_derive",
]
resolver = "2"

[workspace.dependencies]
toml = "0.9.2"
toml_edit = "0.23.1"

[workspace.package]
edition = "2024"
rust-version = "1.85.0"

[profile.release]
lto = "thin"
debug = 1
incremental = false
```

## License

Distributed under MIT License. See [`LICENSE`](https://github.com/RaphGL/Raon/blob/main/LICENSE) for more information.

<!-- ACKNOWLEDGEMENTS -->

