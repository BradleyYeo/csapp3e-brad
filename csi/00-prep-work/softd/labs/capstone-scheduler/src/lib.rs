//! Capstone Starter: Mini Container Workload Scheduler containing 6 unit flaws.

use std::collections::BTreeMap;
use std::rc::Rc;

#[derive(Debug, Clone, PartialEq, Eq)]
pub struct NodeResources {
    pub cap_cpu: u32,
    pub used_cpu: u32,
    pub cap_mem_mb: u64,
    pub used_mem_mb: u64,
}

impl NodeResources {
    pub fn new(cap_cpu: u32, cap_mem_mb: u64) -> Self {
        Self {
            cap_cpu,
            used_cpu: 0,
            cap_mem_mb,
            used_mem_mb: 0,
        }
    }

    pub fn free_cpu(&self) -> u32 {
        self.cap_cpu.saturating_sub(self.used_cpu)
    }

    pub fn free_mem_mb(&self) -> u64 {
        self.cap_mem_mb.saturating_sub(self.used_mem_mb)
    }

    /// FLAW (Unit 1 - Non-atomic reservation):
    /// Mutates `used_cpu` before checking memory capacity, leaving CPU leaked on failure.
    pub fn reserve(&mut self, cpu: u32, mem_mb: u64) -> Result<(), String> {
        if self.used_cpu + cpu > self.cap_cpu {
            return Err("insufficient cpu".into());
        }
        self.used_cpu += cpu;

        if self.used_mem_mb + mem_mb > self.cap_mem_mb {
            return Err("insufficient memory".into());
        }
        self.used_mem_mb += mem_mb;
        Ok(())
    }
}

#[derive(Debug, Clone)]
pub struct Node {
    pub id: String,
    pub labels_raw: String,
    pub has_gpu: bool,
    pub is_spot: bool,
    pub resources: NodeResources,
}

impl Node {
    pub fn labels_map(&self) -> Result<BTreeMap<String, String>, String> {
        let mut map = BTreeMap::new();
        if self.labels_raw.is_empty() {
            return Ok(map);
        }
        // FLAW (Unit 2 - Delimiter coupling): `;` or `=` in values breaks parsing.
        for pair in self.labels_raw.split(';') {
            let (k, v) = pair
                .split_once('=')
                .ok_or_else(|| format!("malformed label: {pair}"))?;
            map.insert(k.to_string(), v.to_string());
        }
        Ok(map)
    }
}

#[derive(Debug, Clone, PartialEq, Eq)]
pub struct AuditEvent {
    pub workload_name: String,
    pub node_id: String,
}

#[derive(Debug, Default)]
pub struct AuditRelay {
    pub events: Vec<AuditEvent>,
}

impl AuditRelay {
    pub fn record(&mut self, event: AuditEvent) {
        // FLAW (Unit 2 - Prying courier):
        // Silently drops events whose workload name starts with `"canary-"`.
        if event.workload_name.starts_with("canary-") {
            return;
        }
        self.events.push(event);
    }
}

/// FLAW (Unit 3 - Permissive flat state allows illegal workload combinations).
#[derive(Clone)]
pub struct Workload {
    pub name: String,
    pub status: String,
    pub req_cpu: u32,
    pub req_mem_mb: u64,
    pub assigned_node: Option<String>,
    pub allocated_cpu: u32,
    pub policy: SchedulingPolicy,
    pub constraint: PlacementConstraint,
}

impl Workload {
    pub fn from_raw(
        name: String,
        status: String,
        req_cpu: u32,
        req_mem_mb: u64,
        assigned_node: Option<String>,
        allocated_cpu: u32,
        policy: SchedulingPolicy,
        constraint: PlacementConstraint,
    ) -> Result<Self, String> {
        // FLAW: accepts `"bound"` with `assigned_node: None` and `"pending"` with `allocated_cpu > 0`.
        Ok(Self {
            name,
            status,
            req_cpu,
            req_mem_mb,
            assigned_node,
            allocated_cpu,
            policy,
            constraint,
        })
    }
}

/// FLAW (Unit 4 - Opaque closure constraint cannot serialize or explain rejection).
#[derive(Clone)]
pub struct PlacementConstraint {
    pred: Rc<dyn Fn(&Node) -> bool>,
}

impl PlacementConstraint {
    pub fn always() -> Self {
        Self {
            pred: Rc::new(|_| true),
        }
    }

    pub fn require_label(key: impl Into<String>, value: impl Into<String>) -> Self {
        let k = key.into();
        let v = value.into();
        Self {
            pred: Rc::new(move |n| n.labels_map().ok().and_then(|m| m.get(&k).cloned()) == Some(v.clone())),
        }
    }

    pub fn matches(&self, node: &Node) -> bool {
        (self.pred)(node)
    }

    pub fn to_dsl(&self) -> Option<String> {
        None
    }

    pub fn explain_rejection(&self, node: &Node) -> String {
        if self.matches(node) {
            "ok".into()
        } else {
            "constraint failed".into()
        }
    }
}

/// FLAW (Unit 5 - Unfactored boolean flags with duplicated logic).
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub struct SchedulingPolicy {
    pub require_gpu: bool,
    pub allow_spot: bool,
    pub preemptible: bool,
}

impl SchedulingPolicy {
    pub fn try_new(require_gpu: bool, allow_spot: bool, preemptible: bool) -> Result<Self, String> {
        // FLAW: permits `!allow_spot && preemptible` contradictory combination.
        Ok(Self {
            require_gpu,
            allow_spot,
            preemptible,
        })
    }
}

pub fn score_node(node: &Node, policy: SchedulingPolicy) -> Option<u32> {
    // FLAW (Unit 5 - Duplicated branches forget GPU requirement on spot nodes!).
    if policy.allow_spot {
        if node.is_spot {
            Some(node.resources.free_cpu() + 10)
        } else {
            if policy.require_gpu && !node.has_gpu {
                return None;
            }
            Some(node.resources.free_cpu())
        }
    } else {
        if node.is_spot {
            return None;
        }
        if policy.require_gpu && !node.has_gpu {
            return None;
        }
        Some(node.resources.free_cpu() + 5)
    }
}

/// FLAW (Unit 6 - Fat trait forces LSP-violating `DryRunBackend`).
pub trait ClusterBackend {
    fn nodes(&self) -> Vec<Node>;
    fn bind_workload(&mut self, node_id: &str, cpu: u32, mem_mb: u64) -> Result<(), String>;
}

#[derive(Default)]
pub struct MemoryCluster {
    nodes: BTreeMap<String, Node>,
}

impl MemoryCluster {
    pub fn add_node(&mut self, node: Node) {
        self.nodes.insert(node.id.clone(), node);
    }

    pub fn get_node(&self, id: &str) -> Option<&Node> {
        self.nodes.get(id)
    }
}

impl ClusterBackend for MemoryCluster {
    fn nodes(&self) -> Vec<Node> {
        self.nodes.values().cloned().collect()
    }

    fn bind_workload(&mut self, node_id: &str, cpu: u32, mem_mb: u64) -> Result<(), String> {
        let node = self
            .nodes
            .get_mut(node_id)
            .ok_or_else(|| format!("missing node {node_id}"))?;
        node.resources.reserve(cpu, mem_mb)
    }
}

/// FLAW (Unit 6 - LSP violation): claims `ClusterBackend` while silently discarding binds!
pub struct DryRunBackend {
    inner: MemoryCluster,
}

impl DryRunBackend {
    pub fn from_cluster(inner: MemoryCluster) -> Self {
        Self { inner }
    }
}

impl ClusterBackend for DryRunBackend {
    fn nodes(&self) -> Vec<Node> {
        self.inner.nodes()
    }

    fn bind_workload(&mut self, _node_id: &str, _cpu: u32, _mem_mb: u64) -> Result<(), String> {
        // Silently succeeds without updating resources!
        Ok(())
    }
}

pub fn verify_cluster_backend_contract<B: ClusterBackend>(
    backend: &mut B,
    node_id: &str,
) -> Result<(), String> {
    let before = backend
        .nodes()
        .into_iter()
        .find(|n| n.id == node_id)
        .ok_or_else(|| "node missing".to_string())?;
    backend.bind_workload(node_id, 2, 256)?;
    let after = backend
        .nodes()
        .into_iter()
        .find(|n| n.id == node_id)
        .ok_or_else(|| "node missing after bind".to_string())?;
    if after.resources.used_cpu != before.resources.used_cpu + 2 {
        return Err("contract violated: bind_workload did not reserve CPU".into());
    }
    Ok(())
}

pub fn schedule<B: ClusterBackend>(
    backend: &mut B,
    relay: &mut AuditRelay,
    workload: &mut Workload,
) -> Result<String, String> {
    let mut best: Option<(u32, String)> = None;
    for node in backend.nodes() {
        if !workload.constraint.matches(&node) {
            continue;
        }
        if node.resources.free_cpu() < workload.req_cpu
            || node.resources.free_mem_mb() < workload.req_mem_mb
        {
            continue;
        }
        if let Some(score) = score_node(&node, workload.policy) {
            match &best {
                Some((best_score, _)) if *best_score >= score => {}
                _ => best = Some((score, node.id.clone())),
            }
        }
    }

    let (_, node_id) = best.ok_or_else(|| "no eligible node".to_string())?;
    backend.bind_workload(&node_id, workload.req_cpu, workload.req_mem_mb)?;
    workload.status = "bound".into();
    workload.assigned_node = Some(node_id.clone());
    workload.allocated_cpu = workload.req_cpu;
    relay.record(AuditEvent {
        workload_name: workload.name.clone(),
        node_id: node_id.clone(),
    });
    Ok(node_id)
}

#[cfg(test)]
mod baseline_tests {
    use super::*;

    #[test]
    fn schedules_standard_workload_onto_healthy_node() {
        let mut cluster = MemoryCluster::default();
        cluster.add_node(Node {
            id: "node-a".into(),
            labels_raw: "zone=us-east;tier=compute".into(),
            has_gpu: false,
            is_spot: false,
            resources: NodeResources::new(8, 16_384),
        });
        let mut relay = AuditRelay::default();
        let mut wl = Workload::from_raw(
            "api-web".into(),
            "pending".into(),
            2,
            1024,
            None,
            0,
            SchedulingPolicy::try_new(false, false, false).unwrap(),
            PlacementConstraint::require_label("zone", "us-east"),
        )
        .unwrap();

        let bound = schedule(&mut cluster, &mut relay, &mut wl).unwrap();
        assert_eq!(bound, "node-a");
        assert_eq!(cluster.get_node("node-a").unwrap().resources.used_cpu, 2);
        assert_eq!(relay.events.len(), 1);
    }
}

#[cfg(test)]
mod lab_goals {
    use super::*;

    #[test]
    #[ignore = "Remove #[ignore] after refactoring to verify lab goal"]
    fn unit_1_failed_memory_reservation_does_not_leak_cpu() {
        let mut res = NodeResources::new(8, 1024);
        assert!(res.reserve(4, 2048).is_err());
        assert_eq!(res.used_cpu, 0);
        assert_eq!(res.used_mem_mb, 0);
    }

    #[test]
    #[ignore = "Remove #[ignore] after refactoring to verify lab goal"]
    fn unit_2_canary_audit_not_muted_and_labels_handle_semicolons() {
        let mut cluster = MemoryCluster::default();
        cluster.add_node(Node {
            id: "node-b".into(),
            labels_raw: "note=a=b;c;zone=eu".into(),
            has_gpu: false,
            is_spot: false,
            resources: NodeResources::new(4, 4096),
        });
        let mut relay = AuditRelay::default();
        let mut wl = Workload::from_raw(
            "canary-checkout".into(),
            "pending".into(),
            1,
            512,
            None,
            0,
            SchedulingPolicy::try_new(false, false, false).unwrap(),
            PlacementConstraint::always(),
        )
        .unwrap();

        assert_eq!(
            cluster.get_node("node-b").unwrap().labels_map().unwrap().get("note").map(String::as_str),
            Some("a=b;c")
        );
        schedule(&mut cluster, &mut relay, &mut wl).unwrap();
        assert_eq!(relay.events.len(), 1);
    }

    #[test]
    #[ignore = "Remove #[ignore] after refactoring to verify lab goal"]
    fn unit_3_illegal_workload_states_are_rejected() {
        let bad = Workload::from_raw(
            "broken".into(),
            "bound".into(),
            2,
            512,
            None,
            2,
            SchedulingPolicy::try_new(false, false, false).unwrap(),
            PlacementConstraint::always(),
        );
        assert!(bad.is_err());
    }

    #[test]
    #[ignore = "Remove #[ignore] after refactoring to verify lab goal"]
    fn unit_4_placement_constraint_serializes_and_explains_reason() {
        let rule = PlacementConstraint::require_label("gpu-arch", "hopper");
        assert!(rule.to_dsl().is_some());

        let node = Node {
            id: "n1".into(),
            labels_raw: "gpu-arch=ampere".into(),
            has_gpu: true,
            is_spot: false,
            resources: NodeResources::new(4, 4096),
        };
        let reason = rule.explain_rejection(&node);
        assert!(reason.contains("gpu-arch"));
        assert!(reason.contains("hopper"));
    }

    #[test]
    #[ignore = "Remove #[ignore] after refactoring to verify lab goal"]
    fn unit_5_spot_node_never_bypasses_gpu_requirement() {
        let spot_no_gpu = Node {
            id: "spot-cpu".into(),
            labels_raw: "".into(),
            has_gpu: false,
            is_spot: true,
            resources: NodeResources::new(16, 16_384),
        };
        let policy = SchedulingPolicy::try_new(true, true, true).unwrap();
        assert_eq!(score_node(&spot_no_gpu, policy), None);
    }

    #[test]
    #[ignore = "Remove #[ignore] after refactoring to verify lab goal"]
    fn unit_6_dry_run_must_not_violate_cluster_backend_contract() {
        let mut cluster = MemoryCluster::default();
        cluster.add_node(Node {
            id: "n1".into(),
            labels_raw: "".into(),
            has_gpu: false,
            is_spot: false,
            resources: NodeResources::new(8, 8192),
        });
        let mut dry = DryRunBackend::from_cluster(cluster);
        assert_eq!(verify_cluster_backend_contract(&mut dry, "n1"), Ok(()));
    }
}
