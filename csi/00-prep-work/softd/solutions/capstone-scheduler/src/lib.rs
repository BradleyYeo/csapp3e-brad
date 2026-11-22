//! Capstone Reference Solution: Unified synthesis of Units 1 through 6.
//!
//! Compile-time checks prove Unit 3 & Unit 6 invariants:
//!
//! ```compile_fail
//! use capstone_scheduler::*;
//! let pending = PendingWorkload::new(
//!     WorkloadName::parse("web").unwrap(),
//!     ResourceRequest::new(1, 256).unwrap(),
//!     SchedulingPolicy::default(),
//!     PlacementRule::Always,
//! );
//! let _node = pending.assigned_node();
//! ```
//!
//! ```compile_fail
//! use capstone_scheduler::*;
//! let mut view = ClusterSnapshotView::new(MemoryCluster::default());
//! let mut relay = AuditRelay::new();
//! let pending = PendingWorkload::new(
//!     WorkloadName::parse("web").unwrap(),
//!     ResourceRequest::new(1, 256).unwrap(),
//!     SchedulingPolicy::default(),
//!     PlacementRule::Always,
//! );
//! // Read-only `ClusterSnapshotView` does not implement `ClusterMutator`:
//! let _ = schedule(&mut view, &mut relay, pending);
//! ```

use std::collections::BTreeMap;

/// Unit 1: Resources with explicit Hoare invariants (`used_cpu <= cap_cpu`, `used_mem_mb <= cap_mem_mb`)
/// and atomic multi-resource reservations.
#[derive(Debug, Clone, PartialEq, Eq)]
pub struct NodeResources {
    cap_cpu: u32,
    used_cpu: u32,
    cap_mem_mb: u64,
    used_mem_mb: u64,
}

impl NodeResources {
    pub fn new(cap_cpu: u32, cap_mem_mb: u64) -> Self {
        let res = Self {
            cap_cpu,
            used_cpu: 0,
            cap_mem_mb,
            used_mem_mb: 0,
        };
        res.assert_invariants();
        res
    }

    fn assert_invariants(&self) {
        debug_assert!(self.used_cpu <= self.cap_cpu);
        debug_assert!(self.used_mem_mb <= self.cap_mem_mb);
    }

    pub fn used_cpu(&self) -> u32 {
        self.used_cpu
    }

    pub fn used_mem_mb(&self) -> u64 {
        self.used_mem_mb
    }

    pub fn free_cpu(&self) -> u32 {
        self.cap_cpu - self.used_cpu
    }

    pub fn free_mem_mb(&self) -> u64 {
        self.cap_mem_mb - self.used_mem_mb
    }

    /// Atomically reserves `cpu` and `mem_mb`.
    ///
    /// # Postcondition
    /// - On `Err(_)`: both `used_cpu` and `used_mem_mb` are unchanged.
    /// - On `Ok(())`: `used_cpu' == used_cpu + cpu` and `used_mem_mb' == used_mem_mb + mem_mb`,
    ///   preserving `used <= cap`.
    pub fn reserve(&mut self, cpu: u32, mem_mb: u64) -> Result<(), String> {
        self.assert_invariants();
        if cpu > self.free_cpu() {
            return Err(format!("insufficient cpu: need {cpu}, free {}", self.free_cpu()));
        }
        if mem_mb > self.free_mem_mb() {
            return Err(format!(
                "insufficient memory: need {mem_mb}, free {}",
                self.free_mem_mb()
            ));
        }
        self.used_cpu += cpu;
        self.used_mem_mb += mem_mb;
        self.assert_invariants();
        Ok(())
    }
}

/// Unit 2: Structured labels preserve arbitrary characters without delimiter collisions.
#[derive(Debug, Clone, PartialEq, Eq)]
pub struct Node {
    pub id: String,
    pub labels: BTreeMap<String, String>,
    pub has_gpu: bool,
    pub is_spot: bool,
    pub resources: NodeResources,
}

#[derive(Debug, Clone, PartialEq, Eq)]
pub struct AuditEvent {
    pub workload_name: String,
    pub node_id: String,
}

/// Unit 2: Parametric courier `AuditRelay<E>` cannot inspect fields inside `E`.
#[derive(Debug)]
pub struct AuditRelay<E> {
    events: Vec<E>,
}

impl<E> Default for AuditRelay<E> {
    fn default() -> Self {
        Self::new()
    }
}

impl<E> AuditRelay<E> {
    pub fn new() -> Self {
        Self { events: Vec::new() }
    }

    pub fn record(&mut self, event: E) {
        self.events.push(event);
    }

    pub fn events(&self) -> &[E] {
        &self.events
    }
}

/// Unit 3: Boundary-validated workload name and non-zero resource request.
#[derive(Debug, Clone, PartialEq, Eq)]
pub struct WorkloadName(String);

impl WorkloadName {
    pub fn parse(raw: impl Into<String>) -> Result<Self, String> {
        let s = raw.into();
        if s.is_empty() {
            return Err("workload name cannot be empty".into());
        }
        Ok(Self(s))
    }

    pub fn as_str(&self) -> &str {
        &self.0
    }
}

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub struct ResourceRequest {
    pub cpu: u32,
    pub mem_mb: u64,
}

impl ResourceRequest {
    pub fn new(cpu: u32, mem_mb: u64) -> Result<Self, String> {
        if cpu == 0 || mem_mb == 0 {
            return Err("resource request cpu and mem_mb must be positive".into());
        }
        Ok(Self { cpu, mem_mb })
    }
}

/// Unit 5: Algebraically factored scheduling policy (no contradictory boolean combinations).
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum WorkloadTier {
    OnDemand,
    Spot { preemptible: bool },
}

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum AcceleratorNeed {
    None,
    RequireGpu,
}

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub struct SchedulingPolicy {
    pub tier: WorkloadTier,
    pub accelerator: AcceleratorNeed,
}

impl Default for SchedulingPolicy {
    fn default() -> Self {
        Self {
            tier: WorkloadTier::OnDemand,
            accelerator: AcceleratorNeed::None,
        }
    }
}

/// Unit 5: Shared accelerator check runs once before tier-specific scoring.
pub fn score_node(node: &Node, policy: SchedulingPolicy) -> Option<u32> {
    if policy.accelerator == AcceleratorNeed::RequireGpu && !node.has_gpu {
        return None;
    }

    match policy.tier {
        WorkloadTier::OnDemand => {
            if node.is_spot {
                None
            } else {
                Some(node.resources.free_cpu() + 5)
            }
        }
        WorkloadTier::Spot { preemptible } => {
            let bonus = match (node.is_spot, preemptible) {
                (true, true) => 10,
                (true, false) => 6,
                (false, _) => 0,
            };
            Some(node.resources.free_cpu() + bonus)
        }
    }
}

/// Unit 4: Defunctionalized placement rule AST supporting evaluation, explanation, and DSL.
#[derive(Debug, Clone, PartialEq, Eq)]
pub enum PlacementRule {
    Always,
    RequireLabel { key: String, value: String },
    And(Box<PlacementRule>, Box<PlacementRule>),
    Not(Box<PlacementRule>),
}

impl PlacementRule {
    pub fn eval(&self, node: &Node) -> bool {
        match self {
            PlacementRule::Always => true,
            PlacementRule::RequireLabel { key, value } => {
                node.labels.get(key) == Some(value)
            }
            PlacementRule::And(l, r) => l.eval(node) && r.eval(node),
            PlacementRule::Not(inner) => !inner.eval(node),
        }
    }

    pub fn explain_rejection(&self, node: &Node) -> Option<String> {
        if self.eval(node) {
            return None;
        }
        Some(match self {
            PlacementRule::Always => unreachable!(),
            PlacementRule::RequireLabel { key, value } => format!(
                "node {} missing label {}={} (actual: {:?})",
                node.id,
                key,
                value,
                node.labels.get(key)
            ),
            PlacementRule::And(l, r) => l
                .explain_rejection(node)
                .or_else(|| r.explain_rejection(node))
                .unwrap_or_else(|| "and rule failed".into()),
            PlacementRule::Not(_) => format!("node {} matched negated rule", node.id),
        })
    }

    pub fn serialize(&self) -> String {
        match self {
            PlacementRule::Always => "always".into(),
            PlacementRule::RequireLabel { key, value } => {
                format!("label:{key}={value}")
            }
            PlacementRule::And(l, r) => format!("({} & {})", l.serialize(), r.serialize()),
            PlacementRule::Not(inner) => format!("!({})", inner.serialize()),
        }
    }
}

/// Unit 3: Exact lifecycle states make unbound-with-node or bound-without-node unrepresentable.
#[derive(Debug, Clone, PartialEq, Eq)]
pub struct PendingWorkload {
    pub name: WorkloadName,
    pub request: ResourceRequest,
    pub policy: SchedulingPolicy,
    pub rule: PlacementRule,
}

impl PendingWorkload {
    pub fn new(
        name: WorkloadName,
        request: ResourceRequest,
        policy: SchedulingPolicy,
        rule: PlacementRule,
    ) -> Self {
        Self {
            name,
            request,
            policy,
            rule,
        }
    }

    pub fn bind(self, node_id: String) -> BoundWorkload {
        BoundWorkload {
            name: self.name,
            allocated: self.request,
            policy: self.policy,
            rule: self.rule,
            assigned_node: node_id,
        }
    }
}

#[derive(Debug, Clone, PartialEq, Eq)]
pub struct BoundWorkload {
    pub name: WorkloadName,
    pub allocated: ResourceRequest,
    pub policy: SchedulingPolicy,
    pub rule: PlacementRule,
    pub assigned_node: String,
}

impl BoundWorkload {
    pub fn assigned_node(&self) -> &str {
        &self.assigned_node
    }
}

/// Unit 6: Parnas subsets separating read-only cluster observation from mutation.
pub trait ClusterView {
    fn nodes(&self) -> Vec<Node>;
}

pub trait ClusterMutator: ClusterView {
    fn bind_workload(&mut self, node_id: &str, req: ResourceRequest) -> Result<(), String>;
}

#[derive(Debug, Default, Clone)]
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

impl ClusterView for MemoryCluster {
    fn nodes(&self) -> Vec<Node> {
        self.nodes.values().cloned().collect()
    }
}

impl ClusterMutator for MemoryCluster {
    fn bind_workload(&mut self, node_id: &str, req: ResourceRequest) -> Result<(), String> {
        let node = self
            .nodes
            .get_mut(node_id)
            .ok_or_else(|| format!("missing node {node_id}"))?;
        node.resources.reserve(req.cpu, req.mem_mb)
    }
}

/// Honest read-only view (cannot violate `ClusterMutator` LSP contract).
pub struct ClusterSnapshotView {
    cluster: MemoryCluster,
}

impl ClusterSnapshotView {
    pub fn new(cluster: MemoryCluster) -> Self {
        Self { cluster }
    }
}

impl ClusterView for ClusterSnapshotView {
    fn nodes(&self) -> Vec<Node> {
        self.cluster.nodes()
    }
}

/// Honest dry-run planner uses `ClusterView` only and returns candidate placements without mutating.
pub fn preview_best_node<V: ClusterView>(
    view: &V,
    workload: &PendingWorkload,
) -> Result<String, Vec<String>> {
    let mut rejections = Vec::new();
    let mut best: Option<(u32, String)> = None;

    for node in view.nodes() {
        if let Some(reason) = workload.rule.explain_rejection(&node) {
            rejections.push(reason);
            continue;
        }
        if node.resources.free_cpu() < workload.request.cpu
            || node.resources.free_mem_mb() < workload.request.mem_mb
        {
            rejections.push(format!("node {} lacks capacity", node.id));
            continue;
        }
        match score_node(&node, workload.policy) {
            Some(score) => match &best {
                Some((best_score, _)) if *best_score >= score => {}
                _ => best = Some((score, node.id.clone())),
            },
            None => rejections.push(format!("node {} violates policy", node.id)),
        }
    }

    best.map(|(_, id)| id).ok_or(rejections)
}

/// Commits placement onto any `ClusterMutator` and emits an audit event via opaque `AuditRelay`.
pub fn schedule<C: ClusterMutator>(
    cluster: &mut C,
    relay: &mut AuditRelay<AuditEvent>,
    workload: PendingWorkload,
) -> Result<BoundWorkload, String> {
    let node_id = preview_best_node(cluster, &workload)
        .map_err(|reasons| reasons.join("; "))?;
    cluster.bind_workload(&node_id, workload.request)?;
    let bound = workload.bind(node_id.clone());
    relay.record(AuditEvent {
        workload_name: bound.name.as_str().to_string(),
        node_id,
    });
    Ok(bound)
}

pub fn verify_cluster_mutator_contract<C: ClusterMutator>(
    cluster: &mut C,
    node_id: &str,
) -> Result<(), String> {
    let before = cluster
        .nodes()
        .into_iter()
        .find(|n| n.id == node_id)
        .ok_or_else(|| "node missing".to_string())?;
    let req = ResourceRequest::new(2, 256)?;
    cluster.bind_workload(node_id, req)?;
    let after = cluster
        .nodes()
        .into_iter()
        .find(|n| n.id == node_id)
        .ok_or_else(|| "node missing after bind".to_string())?;
    if after.resources.used_cpu() != before.resources.used_cpu() + 2 {
        return Err("contract violated: bind_workload did not reserve CPU".into());
    }
    Ok(())
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn unit_1_failed_memory_reservation_does_not_leak_cpu() {
        let mut res = NodeResources::new(8, 1024);
        assert!(res.reserve(4, 2048).is_err());
        assert_eq!(res.used_cpu(), 0);
        assert_eq!(res.used_mem_mb(), 0);
    }

    #[test]
    fn unit_2_canary_audit_not_muted_and_labels_handle_semicolons() {
        let mut cluster = MemoryCluster::default();
        cluster.add_node(Node {
            id: "node-b".into(),
            labels: BTreeMap::from([
                ("note".into(), "a=b;c".into()),
                ("zone".into(), "eu".into()),
            ]),
            has_gpu: false,
            is_spot: false,
            resources: NodeResources::new(4, 4096),
        });
        let mut relay = AuditRelay::new();
        let wl = PendingWorkload::new(
            WorkloadName::parse("canary-checkout").unwrap(),
            ResourceRequest::new(1, 512).unwrap(),
            SchedulingPolicy::default(),
            PlacementRule::RequireLabel {
                key: "note".into(),
                value: "a=b;c".into(),
            },
        );

        let bound = schedule(&mut cluster, &mut relay, wl).unwrap();
        assert_eq!(bound.assigned_node(), "node-b");
        assert_eq!(relay.events().len(), 1);
    }

    #[test]
    fn unit_3_and_4_rule_serializes_and_explains_rejection() {
        let rule = PlacementRule::RequireLabel {
            key: "gpu-arch".into(),
            value: "hopper".into(),
        };
        assert_eq!(rule.serialize(), "label:gpu-arch=hopper");

        let node = Node {
            id: "n1".into(),
            labels: BTreeMap::from([("gpu-arch".into(), "ampere".into())]),
            has_gpu: true,
            is_spot: false,
            resources: NodeResources::new(4, 4096),
        };
        let reason = rule.explain_rejection(&node).unwrap();
        assert!(reason.contains("gpu-arch"));
        assert!(reason.contains("hopper"));
    }

    #[test]
    fn unit_5_spot_node_never_bypasses_gpu_requirement() {
        let spot_no_gpu = Node {
            id: "spot-cpu".into(),
            labels: BTreeMap::new(),
            has_gpu: false,
            is_spot: true,
            resources: NodeResources::new(16, 16_384),
        };
        let policy = SchedulingPolicy {
            tier: WorkloadTier::Spot { preemptible: true },
            accelerator: AcceleratorNeed::RequireGpu,
        };
        assert_eq!(score_node(&spot_no_gpu, policy), None);
    }

    #[test]
    fn unit_6_mutator_contract_and_dry_run_view_both_work() {
        let mut cluster = MemoryCluster::default();
        cluster.add_node(Node {
            id: "n1".into(),
            labels: BTreeMap::new(),
            has_gpu: false,
            is_spot: false,
            resources: NodeResources::new(8, 8192),
        });
        let snapshot = ClusterSnapshotView::new(cluster.clone());
        let wl = PendingWorkload::new(
            WorkloadName::parse("batch-1").unwrap(),
            ResourceRequest::new(2, 256).unwrap(),
            SchedulingPolicy::default(),
            PlacementRule::Always,
        );
        assert_eq!(preview_best_node(&snapshot, &wl), Ok("n1".into()));
        assert_eq!(verify_cluster_mutator_contract(&mut cluster, "n1"), Ok(()));
    }
}
