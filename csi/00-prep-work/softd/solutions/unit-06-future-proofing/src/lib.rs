//! Unit 6 Reference Solution: RAD, Liskov behavioral subtyping, and Parnas subsets.
//!
//! Read-only snapshots and bounded caches cannot satisfy `CheckpointManager<S: KvStore>`:
//!
//! ```compile_fail
//! use unit_06_future_proofing::*;
//! let snapshot = ReadOnlySnapshot::from_pairs(vec![]);
//! let _mgr = CheckpointManager::new(snapshot);
//! ```
//!
//! ```compile_fail
//! use unit_06_future_proofing::*;
//! let cache = BoundedCache::new(4);
//! let _mgr = CheckpointManager::new(cache);
//! ```

use std::collections::{BTreeMap, VecDeque};

#[derive(Debug, Clone, PartialEq, Eq)]
pub struct StoreError(pub String);

/// Minimal Parnas subset: read-only key/value access.
pub trait KvReader {
    fn get(&self, key: &str) -> Option<Vec<u8>>;
    fn keys_sorted(&self) -> Vec<String>;
}

/// Behavioral subtype adding durable writes and deletes.
///
/// # Contract (LSP)
/// - `put(k, v)` preserves all previously inserted distinct keys until `delete`.
/// - After `Ok(()) = put(k, v)`, `get(&k) == Some(v)`.
/// - After `Ok(_) = delete(&k)`, `get(&k) == None`.
pub trait KvStore: KvReader {
    fn put(&mut self, key: String, value: Vec<u8>) -> Result<(), StoreError>;
    fn delete(&mut self, key: &str) -> Result<bool, StoreError>;
}

/// Honest sibling subset for bounded caches: explicitly advertises eviction rather than durability.
pub trait EvictingCache: KvReader {
    fn capacity(&self) -> usize;
    fn insert_evicting(&mut self, key: String, value: Vec<u8>);
}

#[derive(Debug, Default)]
pub struct InMemoryStore {
    data: BTreeMap<String, Vec<u8>>,
}

impl KvReader for InMemoryStore {
    fn get(&self, key: &str) -> Option<Vec<u8>> {
        self.data.get(key).cloned()
    }

    fn keys_sorted(&self) -> Vec<String> {
        self.data.keys().cloned().collect()
    }
}

impl KvStore for InMemoryStore {
    fn put(&mut self, key: String, value: Vec<u8>) -> Result<(), StoreError> {
        self.data.insert(key, value);
        Ok(())
    }

    fn delete(&mut self, key: &str) -> Result<bool, StoreError> {
        Ok(self.data.remove(key).is_some())
    }
}

#[derive(Debug, Default)]
pub struct JournaledStore {
    data: BTreeMap<String, Vec<u8>>,
    journal_ops: usize,
}

impl JournaledStore {
    pub fn journal_ops(&self) -> usize {
        self.journal_ops
    }
}

impl KvReader for JournaledStore {
    fn get(&self, key: &str) -> Option<Vec<u8>> {
        self.data.get(key).cloned()
    }

    fn keys_sorted(&self) -> Vec<String> {
        self.data.keys().cloned().collect()
    }
}

impl KvStore for JournaledStore {
    fn put(&mut self, key: String, value: Vec<u8>) -> Result<(), StoreError> {
        self.journal_ops += 1;
        self.data.insert(key, value);
        Ok(())
    }

    fn delete(&mut self, key: &str) -> Result<bool, StoreError> {
        self.journal_ops += 1;
        Ok(self.data.remove(key).is_some())
    }
}

/// Read-only snapshot implements ONLY `KvReader`, never masquerading as `KvStore`.
#[derive(Debug, Default)]
pub struct ReadOnlySnapshot {
    snapshot: BTreeMap<String, Vec<u8>>,
}

impl ReadOnlySnapshot {
    pub fn from_pairs(pairs: impl IntoIterator<Item = (String, Vec<u8>)>) -> Self {
        Self {
            snapshot: pairs.into_iter().collect(),
        }
    }
}

impl KvReader for ReadOnlySnapshot {
    fn get(&self, key: &str) -> Option<Vec<u8>> {
        self.snapshot.get(key).cloned()
    }

    fn keys_sorted(&self) -> Vec<String> {
        self.snapshot.keys().cloned().collect()
    }
}

/// Bounded cache implements ONLY `KvReader` + `EvictingCache`, matching its true semantics.
#[derive(Debug)]
pub struct BoundedCache {
    capacity: usize,
    order: VecDeque<String>,
    data: BTreeMap<String, Vec<u8>>,
}

impl BoundedCache {
    pub fn new(capacity: usize) -> Self {
        Self {
            capacity: capacity.max(1),
            order: VecDeque::new(),
            data: BTreeMap::new(),
        }
    }
}

impl KvReader for BoundedCache {
    fn get(&self, key: &str) -> Option<Vec<u8>> {
        self.data.get(key).cloned()
    }

    fn keys_sorted(&self) -> Vec<String> {
        self.data.keys().cloned().collect()
    }
}

impl EvictingCache for BoundedCache {
    fn capacity(&self) -> usize {
        self.capacity
    }

    fn insert_evicting(&mut self, key: String, value: Vec<u8>) {
        if !self.data.contains_key(&key) && self.data.len() >= self.capacity {
            if let Some(oldest) = self.order.pop_front() {
                self.data.remove(&oldest);
            }
        }
        if !self.data.contains_key(&key) {
            self.order.push_back(key.clone());
        }
        self.data.insert(key, value);
    }
}

/// Behavioral contract suite satisfied by every `KvStore`.
pub fn verify_kv_store_contract<S: KvStore>(store: &mut S) -> Result<(), String> {
    store
        .put("alpha".into(), b"1".to_vec())
        .map_err(|e| e.0)?;
    store
        .put("beta".into(), b"2".to_vec())
        .map_err(|e| e.0)?;
    store
        .put("gamma".into(), b"3".to_vec())
        .map_err(|e| e.0)?;

    if store.get("alpha").as_deref() != Some(b"1") {
        return Err("contract violated: alpha lost".into());
    }
    if store.get("beta").as_deref() != Some(b"2") {
        return Err("contract violated: beta lost".into());
    }
    if !store.delete("beta").map_err(|e| e.0)? {
        return Err("contract violated: delete(beta) must return true".into());
    }
    if store.get("beta").is_some() {
        return Err("contract violated: deleted key still present".into());
    }
    Ok(())
}

/// Sequestered key codec: escaping lives in one place so arbitrary `/` and `%` strings
/// round-trip bijectively without colliding.
mod key_codec {
    pub fn encode(namespace: &str, checkpoint: &str) -> String {
        format!("{}/{}", escape(namespace), escape(checkpoint))
    }

    pub fn namespace_prefix(namespace: &str) -> String {
        format!("{}/", escape(namespace))
    }

    pub fn decode_checkpoint(encoded_rest: &str) -> Option<String> {
        unescape(encoded_rest)
    }

    fn escape(s: &str) -> String {
        let mut out = String::with_capacity(s.len());
        for b in s.bytes() {
            match b {
                b'%' => out.push_str("%25"),
                b'/' => out.push_str("%2F"),
                other => out.push(other as char),
            }
        }
        out
    }

    fn unescape(s: &str) -> Option<String> {
        let bytes = s.as_bytes();
        let mut out = Vec::with_capacity(bytes.len());
        let mut i = 0;
        while i < bytes.len() {
            if bytes[i] == b'%' {
                if i + 2 >= bytes.len() {
                    return None;
                }
                match &bytes[i + 1..i + 3] {
                    b"25" => out.push(b'%'),
                    b"2F" => out.push(b'/'),
                    _ => return None,
                }
                i += 3;
            } else {
                out.push(bytes[i]);
                i += 1;
            }
        }
        String::from_utf8(out).ok()
    }
}

/// Read-only checkpoint inspector built on the minimal `KvReader` subset.
pub struct CheckpointInspector<R: KvReader> {
    reader: R,
}

impl<R: KvReader> CheckpointInspector<R> {
    pub fn new(reader: R) -> Self {
        Self { reader }
    }

    pub fn load(&self, namespace: &str, checkpoint: &str) -> Option<Vec<u8>> {
        self.reader.get(&key_codec::encode(namespace, checkpoint))
    }

    pub fn list_namespace(&self, namespace: &str) -> Vec<String> {
        let prefix = key_codec::namespace_prefix(namespace);
        self.reader
            .keys_sorted()
            .into_iter()
            .filter_map(|k| {
                k.strip_prefix(&prefix)
                    .and_then(key_codec::decode_checkpoint)
            })
            .collect()
    }
}

/// Open, generic checkpoint manager requiring full `KvStore` durability.
pub struct CheckpointManager<S: KvStore> {
    store: S,
}

impl<S: KvStore> CheckpointManager<S> {
    pub fn new(store: S) -> Self {
        Self { store }
    }

    pub fn save(
        &mut self,
        namespace: &str,
        checkpoint: &str,
        bytes: Vec<u8>,
    ) -> Result<(), StoreError> {
        self.store
            .put(key_codec::encode(namespace, checkpoint), bytes)
    }

    pub fn load(&self, namespace: &str, checkpoint: &str) -> Option<Vec<u8>> {
        self.store.get(&key_codec::encode(namespace, checkpoint))
    }

    pub fn list_namespace(&self, namespace: &str) -> Vec<String> {
        let prefix = key_codec::namespace_prefix(namespace);
        self.store
            .keys_sorted()
            .into_iter()
            .filter_map(|k| {
                k.strip_prefix(&prefix)
                    .and_then(key_codec::decode_checkpoint)
            })
            .collect()
    }

    pub fn store(&self) -> &S {
        &self.store
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use std::collections::HashMap;

    #[derive(Default)]
    struct CustomHashBackend {
        inner: HashMap<String, Vec<u8>>,
    }

    impl KvReader for CustomHashBackend {
        fn get(&self, key: &str) -> Option<Vec<u8>> {
            self.inner.get(key).cloned()
        }

        fn keys_sorted(&self) -> Vec<String> {
            let mut keys: Vec<_> = self.inner.keys().cloned().collect();
            keys.sort();
            keys
        }
    }

    impl KvStore for CustomHashBackend {
        fn put(&mut self, key: String, value: Vec<u8>) -> Result<(), StoreError> {
            self.inner.insert(key, value);
            Ok(())
        }

        fn delete(&mut self, key: &str) -> Result<bool, StoreError> {
            Ok(self.inner.remove(key).is_some())
        }
    }

    #[test]
    fn all_kv_store_implementations_satisfy_contract() {
        let mut mem = InMemoryStore::default();
        let mut journal = JournaledStore::default();
        let mut custom = CustomHashBackend::default();

        assert_eq!(verify_kv_store_contract(&mut mem), Ok(()));
        assert_eq!(verify_kv_store_contract(&mut journal), Ok(()));
        assert_eq!(verify_kv_store_contract(&mut custom), Ok(()));
    }

    #[test]
    fn checkpoint_manager_sequesters_slashes_without_collisions() {
        let mut mgr = CheckpointManager::new(InMemoryStore::default());
        mgr.save("team/a", "../snap/v1", b"payload-1".to_vec())
            .unwrap();
        mgr.save("team", "a/../snap/v1", b"payload-2".to_vec())
            .unwrap();

        assert_eq!(mgr.load("team/a", "../snap/v1"), Some(b"payload-1".to_vec()));
        assert_eq!(mgr.load("team", "a/../snap/v1"), Some(b"payload-2".to_vec()));
        assert_eq!(mgr.list_namespace("team/a"), vec!["../snap/v1"]);
    }

    #[test]
    fn readonly_and_bounded_backends_work_with_honest_parnas_subsets() {
        let mut mgr = CheckpointManager::new(CustomHashBackend::default());
        mgr.save("ns", "c1", b"hello".to_vec()).unwrap();
        assert_eq!(mgr.load("ns", "c1"), Some(b"hello".to_vec()));

        let snap = ReadOnlySnapshot::from_pairs(vec![(
            key_codec::encode("ns", "c1"),
            b"readonly-snap".to_vec(),
        )]);
        let inspector = CheckpointInspector::new(snap);
        assert_eq!(inspector.load("ns", "c1"), Some(b"readonly-snap".to_vec()));

        let mut cache = BoundedCache::new(2);
        cache.insert_evicting("k1".into(), b"1".to_vec());
        cache.insert_evicting("k2".into(), b"2".to_vec());
        cache.insert_evicting("k3".into(), b"3".to_vec());
        assert_eq!(cache.get("k1"), None);
        assert_eq!(cache.get("k3"), Some(b"3".to_vec()));
    }
}
