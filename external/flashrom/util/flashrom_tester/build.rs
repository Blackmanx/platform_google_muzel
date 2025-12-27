#[cfg(not(feature = "build-info"))]
fn main() {
    use std::env;
    use std::fs;
    use std::path::Path;

    let out_dir = env::var("OUT_DIR").unwrap();
    let dest_path = Path::new(&out_dir).join("built.rs");

    // Create a stub built.rs file with only the placeholder values
    // that are actually used in main.rs.
    let code = r#"
        pub const PKG_VERSION: &str = "N/A";
        pub const TARGET: &str = "N/A";
        pub const PROFILE: &str = "N/A";
        pub const FEATURES: [&'static str; 0] = [];
        pub const BUILT_TIME_UTC: &str = "N/A";
        pub const RUSTC_VERSION: &str = "N/A";
    "#;

    fs::write(&dest_path, code).unwrap();
    println!("cargo:rerun-if-changed=build.rs");
}

// The `built` crate is only needed when the `build-info` feature is enabled.
#[cfg(feature = "build-info")]
extern crate built;

#[cfg(feature = "build-info")]
fn main() {
    built::write_built_file().expect("Failed to acquire build-time information");
}
