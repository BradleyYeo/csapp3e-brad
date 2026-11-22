//! Unit 6 Starter: Fat `KvStore` trait with LSP violations, leaked key assumptions, and closed backends.

use std::collections::{BTreeMap, VecDeque};

#[derive(Debug, Clone, PartialEq, Eq)]
pub struct StoreError(pub String);

pub trait KvStore {
    fn get(&self, key: &str) -> Option<Vec<u8>>;
    fn put(&mut self, key: String, value: Vec<u8>) -> Result<(), StoreError>;
    fn delete(&mut self, key: &str) -> Result<bool, StoreError>;
    fn keys_sorted(&self) -> Vec<String>;
}

#[derive(Debug, Default)]
pub struct InMemoryStore {
    data: BTreeMap<String, Vec<u8>>,
}

impl KvStore for InMemoryStore {
    fn get(&self, key: &str) -> Option<Vec<u8>> {
        self.data.get(key).cloned()
    }

    fn put(&mut self, key: String, value: Vec<u8>) -> Result<(), StoreError> {
        self.data.insert(key, value);
        Ok(())
    }

    fn delete(&mut self, key: &str) -> Result<bool, StoreError> {
        Ok(self.data.remove(key).is_some())
    }

    fn keys_sorted(&self) -> Vec<String> {
        self.data.keys().cloned().collect()
    }
}

/// FLAW (Unit 6 - LSP Violation):
/// Strengthens `put`/`delete` preconditions by failing on all writes while claiming `KvStore`.
#[derive(Debug, Default)]
pub struct ReadOnlyReplica {
    snapshot: BTreeMap<String, Vec<u8>>,
}

impl ReadOnlyReplica {
    pub fn from_map(snapshot: BTreeMap<String, Vec<u8>>) -> Self {
        Self { snapshot }
    }
}

impl KvStore for ReadOnlyReplica {
    fn get(&self, key: &str) -> Option<Vec<u8>> {
        self.snapshot.get(key).cloned()
    }

    fn put(&mut self, _key: String, _value: Vec<u8>) -> Result<(), StoreError> {
        Err(StoreError("read-only replica rejects writes".into()))
    }

    fn delete(&mut self, _key: &str) -> Result<bool, StoreError> {
        Err(StoreError("read-only replica rejects deletes".into()))
    }

    fn keys_sorted(&self) -> Vec<String> {
        self.snapshot.keys().cloned().collect()
    }
}

/// FLAW (Unit 6 - LSP Violation):
/// Weakens `KvStore` durability postcondition by silently evicting prior keys.
#[derive(Debug)]
pub struct BoundedStore {
    capacity: usize,
    order: VecDeque<String>,
    data: BTreeMap<String, Vec<u8>>,
}

impl BoundedStore {
    pub fn new(capacity: usize) -> Self {
        Self {
            capacity: capacity.max(1),
            order: VecDeque::new(),
            data: BTreeMap::new(),
        }
    }
}

impl KvStore for BoundedStore {
    fn get(&self, key: &str) -> Option<Vec<u8>> {
        self.data.get(key).cloned()
    }

    fn put(&mut self, key: String, value: Vec<u8>) -> Result<(), StoreError> {
        if !self.data.contains_key(&key) && self.data.len() >= self.capacity {
            if let Some(oldest) = self.order.pop_front() {
                self.data.remove(&oldest);
            }
        }
        if !self.data.contains_key(&key) {
            self.order.push_back(key.clone());
        }
        self.data.insert(key, value);
        Ok(())
    }

    fn delete(&mut self, key: &str) -> Result<bool, StoreError> {
        self.order.retain(|k| k != key);
        Ok(self.data.remove(key).is_some())
    }

    fn keys_sorted(&self) -> Vec<String> {
        self.data.keys().cloned().collect()
    }
}

/// Behavioral contract test for true durable `KvStore` implementations.
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
        return Err("contract violated: alpha was lost after inserting gamma".into());
    }
    if store.get("beta").as_deref() != Some(b"2") {
        return Err("contract violated: beta was lost".into());
    }
    if store.delete("beta").map_err(|e| e.0)? != true {
        return Err("contract violated: delete(beta) must return true".into());
    }
    if store.get("beta").is_some() {
        return Err("contract violated: deleted key still visible".into());
    }
    Ok(())
}

/// FLAW (Unit 6 - Closed backend enum + scattered raw `/` key assumption).
pub enum BackendKind {
    Memory(InMemoryStore),
    Bounded(BoundedStore),
    ReadOnly(ReadOnlyReplica),
}

pub struct CheckpointManager {
    backend: BackendKind,
}

impl CheckpointManager {
    pub fn new_memory() -> Self {
        Self {
            backend: BackendKind::Memory(InMemoryStore::default()),
        }
    }

    pub fn new_bounded(capacity: usize) -> Self {
        Self {
            backend: BackendKind::Bounded(BoundedStore::new(capacity)),
        }
    }

    pub fn save(
        &mut self,
        namespace: &str,
        checkpoint: &str,
        bytes: Vec<u8>,
    ) -> Result<(), StoreError> {
        // FLAW (Unit 6 - Leaked assumption): `/` validation & formatting duplicated across methods.
        if namespace.contains('/') || checkpoint.contains('/') {
            return Err(StoreError("delimiter '/' not allowed in key parts".into()));
        }
        let key = format!("{namespace}/{checkpoint}");
        match &mut self.backend {
            BackendKind::Memory(s) => s.put(key, bytes),
            BackendKind::Bounded(s) => s.put(key, bytes),
            BackendKind::ReadOnly(s) => s.put(key, bytes),
        }
    }

    pub fn load(&self, namespace: &str, checkpoint: &str) -> Result<Option<Vec<u8>>, StoreError> {
        if namespace.contains('/') || checkpoint.contains('/') {
            return Err(StoreError("delimiter '/' not allowed in key parts".into()));
        }
        let key = format!("{namespace}/{checkpoint}");
        Ok(match &self.backend {
            BackendKind::Memory(s) => s.get(&key),
            BackendKind::Bounded(s) => s.get(&key),
            BackendKind::ReadOnly(s) => s.get(&key),
        })
    }

    pub fn list_namespace(&self, namespace: &str) -> Result<Vec<String>, StoreError> {
        if namespace.contains('/') {
            return Err(StoreError("delimiter '/' not allowed".into()));
        }
        let prefix = format!("{namespace}/");
        let keys = match &self.backend {
            BackendKind::Memory(s) => s.keys_sorted(),
            BackendKind::Bounded(s) => s.keys_sorted(),
            BackendKind::ReadOnly(s) => s.keys_sorted(),
        };
        Ok(keys
            .into_iter()
            .filter_map(|k| k.strip_prefix(&prefix).map(ToString::to_string))
            .collect())
    }

    pub fn supports_open_custom_backends() -> bool {
        false
    }
}

#[cfg(test)]
mod baseline_tests {
    use super::*;

    #[test]
    fn memory_checkpoint_manager_saves_and_lists() {
        let mut mgr = CheckpointManager::new_memory();
        mgr.save("jobs", "ckpt-1", b"state-a".to_vec()).unwrap();
        assert_eq!(
            mgr.load("jobs", "ckpt-1").unwrap(),
            Some(b"state-a".to_vec())
        );
        assert_eq!(mgr.list_namespace("jobs").unwrap(), vec!["ckpt-1"]);
    }

    #[test]
    fn in_memory_store_passes_contract() {
        let mut mem = InMemoryStore::default();
        assert_eq!(verify_kv_store_contract(&mut mem), Ok(()));
    }
}

#[cfg(test)]
mod lab_goals {
    use super::*;

    #[test]
    #[ignore = "Remove #[ignore] after refactoring to verify lab goal"]
    fn bounded_store_must_not_masquerade_as_violating_kv_store() {
        // Starter lets BoundedStore implement KvStore and fail the contract!
        // Refactor into Parnas subsets (`EvictingCache` vs `KvStore`) and update this test
        // to prove every `KvStore` implementation passes `verify_kv_store_contract`.
        let mut bounded = BoundedStore::new(2);
        assert_eq!(verify_kv_store_contract(&mut bounded), Ok(()));
    }

    #[test]
    #[ignore = "Remove #[ignore] after refactoring to verify lab goal"]
    fn readonly_replica_must_not_fail_kv_store_contract() {
        let mut ro = ReadOnlyReplica::from_map(BTreeMap::new());
        assert_eq!(verify_kv_store_contract(&mut ro), Ok(()));
    }

    #[test]
    #[ignore = "Remove #[ignore] after refactoring to verify lab goal"]
    fn checkpoint_manager_sequesters_slashes_and_special_keys() {
        let mut mgr = CheckpointManager::new_memory();
        mgr.save("team/a", "../snap/v1", b"payload-1".to_vec())
            .expect("sequestered codec must encode slashes safely");
        mgr.save("team", "a/../snap/v1", b"payload-2".to_vec())
            .expect("different segments must not collide");

        assert_eq!(
            mgr.load("team/a", "../snap/v1").unwrap(),
            Some(b"payload-1".to_vec())
        );
        assert_eq!(
            mgr.load("team", "a/../snap/v1").unwrap(),
            Some(b"payload-2".to_vec())
        );
        assert_eq!(mgr.list_namespace("team/a").unwrap(), vec!["../snap/v1"]);
    }

    #[test]
    #[ignore = "Remove #[ignore] after refactoring to verify lab goal"]
    fn checkpoint_manager_is_open_to_custom_backends() {
        assert!(CheckpointManager::supports_open_custom_backends());
    }
}
