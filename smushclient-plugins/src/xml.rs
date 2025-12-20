pub mod bool_serde {
    use serde::{Serialize, Serializer};
    pub use serde_this_or_that::as_bool as deserialize;

    #[allow(clippy::trivially_copy_pass_by_ref)]
    pub fn serialize<S: Serializer>(value: &bool, serializer: S) -> Result<S::Ok, S::Error> {
        (if *value { "y" } else { "n" }).serialize(serializer)
    }
}
