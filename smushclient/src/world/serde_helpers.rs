use smushclient_plugins::Sender;

pub(super) fn skip_temporary<S, T>(vec: &[T], serializer: S) -> Result<S::Ok, S::Error>
where
    S: serde::Serializer,
    T: serde::Serialize + AsRef<Sender> + Ord,
{
    // must collect in a vec because serialization needs to know the size ahead of time
    let filtered: Vec<&T> = vec.iter().filter(|x| !x.as_ref().temporary).collect();
    serializer.collect_seq(filtered)
}
