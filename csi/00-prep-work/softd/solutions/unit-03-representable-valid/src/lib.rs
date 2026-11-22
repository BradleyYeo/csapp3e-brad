//! Unit 3 Reference Solution: Parse, don't validate — illegal pod states are unrepresentable.
//!
//! Compile-time checks prove invalid calls fail to compile:
//!
//! ```compile_fail
//! use unit_03_representable_valid::*;
//! let pending = PendingPod::new(
//!     PodName::parse("web").unwrap(),
//!     RestartBudget::new(0, 3).unwrap(),
//! );
//! let _ = route_traffic(&pending);
//! ```
//!
//! ```compile_fail
//! use unit_03_representable_valid::*;
//! let _ = FailedPod::new(
//!     PodName::parse("worker").unwrap(),
//!     ContainerId::parse("ctr-1").unwrap(),
//!     0i32,
//!     "cannot pass raw 0 as NonZeroExitCode".into(),
//!     RestartBudget::new(0, 1).unwrap(),
//! );
//! ```

use std::num::NonZeroI32;

#[derive(Debug, Clone, PartialEq, Eq)]
pub struct PodName(String);

impl PodName {
    pub fn parse(raw: impl Into<String>) -> Result<Self, String> {
        let s = raw.into();
        if s.is_empty() || !s.chars().all(|c| c.is_ascii_alphanumeric() || c == '-') {
            return Err(format!("invalid pod name: {s:?}"));
        }
        Ok(Self(s))
    }

    pub fn as_str(&self) -> &str {
        &self.0
    }
}

#[derive(Debug, Clone, PartialEq, Eq)]
pub struct ContainerId(String);

impl ContainerId {
    pub fn parse(raw: impl Into<String>) -> Result<Self, String> {
        let s = raw.into();
        if s.is_empty() {
            return Err("container id cannot be empty".into());
        }
        Ok(Self(s))
    }

    pub fn as_str(&self) -> &str {
        &self.0
    }
}

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub struct NonZeroExitCode(NonZeroI32);

impl NonZeroExitCode {
    pub fn new(code: i32) -> Result<Self, String> {
        NonZeroI32::new(code)
            .map(Self)
            .ok_or_else(|| "failed pod exit code cannot be 0".into())
    }

    pub fn get(self) -> i32 {
        self.0.get()
    }
}

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub struct RestartBudget {
    used: u32,
    max: u32,
}

impl RestartBudget {
    pub fn new(used: u32, max: u32) -> Result<Self, String> {
        if used > max {
            return Err(format!("restart count {used} exceeds max {max}"));
        }
        Ok(Self { used, max })
    }

    pub fn can_restart(&self) -> bool {
        self.used < self.max
    }
}

#[derive(Debug, Clone, PartialEq, Eq)]
pub struct PendingPod {
    pub name: PodName,
    pub restarts: RestartBudget,
}

impl PendingPod {
    pub fn new(name: PodName, restarts: RestartBudget) -> Self {
        Self { name, restarts }
    }
}

#[derive(Debug, Clone, PartialEq, Eq)]
pub struct StartingPod {
    pub name: PodName,
    pub container_id: ContainerId,
    pub ip_addr: [u8; 4],
    pub restarts: RestartBudget,
}

#[derive(Debug, Clone, PartialEq, Eq)]
pub struct ReadyPod {
    pub name: PodName,
    pub container_id: ContainerId,
    pub ip_addr: [u8; 4],
    pub restarts: RestartBudget,
}

#[derive(Debug, Clone, PartialEq, Eq)]
pub struct SucceededPod {
    pub name: PodName,
    pub container_id: ContainerId,
    pub restarts: RestartBudget,
}

#[derive(Debug, Clone, PartialEq, Eq)]
pub struct FailedPod {
    pub name: PodName,
    pub container_id: ContainerId,
    pub exit_code: NonZeroExitCode,
    pub reason: String,
    pub restarts: RestartBudget,
}

impl FailedPod {
    pub fn new(
        name: PodName,
        container_id: ContainerId,
        exit_code: NonZeroExitCode,
        reason: String,
        restarts: RestartBudget,
    ) -> Result<Self, String> {
        if reason.is_empty() {
            return Err("failure reason cannot be empty".into());
        }
        Ok(Self {
            name,
            container_id,
            exit_code,
            reason,
            restarts,
        })
    }
}

#[derive(Debug, Clone, PartialEq, Eq)]
pub enum PodPhase {
    Pending(PendingPod),
    Starting(StartingPod),
    Ready(ReadyPod),
    Succeeded(SucceededPod),
    Failed(FailedPod),
}

#[derive(Debug, Clone, PartialEq, Eq)]
pub struct PodSpec {
    pub name: String,
    pub phase: String,
    pub container_id: Option<String>,
    pub ip_addr: Option<[u8; 4]>,
    pub exit_code: Option<i32>,
    pub failure_reason: Option<String>,
    pub is_ready: bool,
    pub restart_count: u32,
    pub max_restarts: u32,
}

impl PodPhase {
    /// Parses a raw external specification into an exact valid lifecycle variant.
    pub fn from_spec(spec: PodSpec) -> Result<Self, String> {
        let name = PodName::parse(spec.name)?;
        let restarts = RestartBudget::new(spec.restart_count, spec.max_restarts)?;

        match spec.phase.as_str() {
            "pending" => {
                if spec.container_id.is_some()
                    || spec.ip_addr.is_some()
                    || spec.exit_code.is_some()
                    || spec.failure_reason.is_some()
                    || spec.is_ready
                {
                    return Err("pending pod cannot carry runtime or exit fields".into());
                }
                Ok(PodPhase::Pending(PendingPod::new(name, restarts)))
            }
            "running" => {
                if spec.exit_code.is_some() || spec.failure_reason.is_some() {
                    return Err("running pod cannot carry exit fields".into());
                }
                let container_id = ContainerId::parse(
                    spec.container_id
                        .ok_or_else(|| "running pod requires container_id".to_string())?,
                )?;
                let ip_addr = spec
                    .ip_addr
                    .ok_or_else(|| "running pod requires ip_addr".to_string())?;
                if spec.is_ready {
                    Ok(PodPhase::Ready(ReadyPod {
                        name,
                        container_id,
                        ip_addr,
                        restarts,
                    }))
                } else {
                    Ok(PodPhase::Starting(StartingPod {
                        name,
                        container_id,
                        ip_addr,
                        restarts,
                    }))
                }
            }
            "succeeded" => {
                if spec.ip_addr.is_some() || spec.failure_reason.is_some() || spec.is_ready {
                    return Err("succeeded pod has invalid runtime/failure fields".into());
                }
                if spec.exit_code != Some(0) {
                    return Err("succeeded pod requires exit code 0".into());
                }
                let container_id = ContainerId::parse(
                    spec.container_id
                        .ok_or_else(|| "succeeded pod requires container_id".to_string())?,
                )?;
                Ok(PodPhase::Succeeded(SucceededPod {
                    name,
                    container_id,
                    restarts,
                }))
            }
            "failed" => {
                if spec.ip_addr.is_some() || spec.is_ready {
                    return Err("failed pod cannot have ip_addr or is_ready".into());
                }
                let container_id = ContainerId::parse(
                    spec.container_id
                        .ok_or_else(|| "failed pod requires container_id".to_string())?,
                )?;
                let exit_code = NonZeroExitCode::new(
                    spec.exit_code
                        .ok_or_else(|| "failed pod requires exit_code".to_string())?,
                )?;
                let reason = spec
                    .failure_reason
                    .ok_or_else(|| "failed pod requires failure_reason".to_string())?;
                Ok(PodPhase::Failed(FailedPod::new(
                    name,
                    container_id,
                    exit_code,
                    reason,
                    restarts,
                )?))
            }
            other => Err(format!("unknown phase: {other}")),
        }
    }
}

/// Total, infallible routing function: zero defensive checks needed.
///
/// ```
/// use unit_03_representable_valid::*;
/// let ready = ReadyPod {
///     name: PodName::parse("api-1").unwrap(),
///     container_id: ContainerId::parse("ctr-1").unwrap(),
///     ip_addr: [10, 0, 0, 5],
///     restarts: RestartBudget::new(0, 3).unwrap(),
/// };
/// assert_eq!(route_traffic(&ready), [10, 0, 0, 5]);
/// ```
pub fn route_traffic(pod: &ReadyPod) -> [u8; 4] {
    pod.ip_addr
}

/// Total, infallible failure summary: zero defensive checks needed.
pub fn failure_summary(pod: &FailedPod) -> String {
    format!(
        "{} failed (code {}): {}",
        pod.name.as_str(),
        pod.exit_code.get(),
        pod.reason
    )
}

/// Total restart check: `FailedPod` guarantees valid phase and `RestartBudget` guarantees `used <= max`.
pub fn can_restart(pod: &FailedPod) -> bool {
    pod.restarts.can_restart()
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn valid_ready_pod_routes_traffic() {
        let phase = PodPhase::from_spec(PodSpec {
            name: "api-1".into(),
            phase: "running".into(),
            container_id: Some("ctr-99".into()),
            ip_addr: Some([10, 0, 0, 5]),
            exit_code: None,
            failure_reason: None,
            is_ready: true,
            restart_count: 0,
            max_restarts: 3,
        })
        .unwrap();

        match phase {
            PodPhase::Ready(ready) => assert_eq!(route_traffic(&ready), [10, 0, 0, 5]),
            other => panic!("expected Ready, got {other:?}"),
        }
    }

    #[test]
    fn valid_failed_pod_formats_summary() {
        let phase = PodPhase::from_spec(PodSpec {
            name: "worker-1".into(),
            phase: "failed".into(),
            container_id: Some("ctr-12".into()),
            ip_addr: None,
            exit_code: Some(137),
            failure_reason: Some("OOMKilled".into()),
            is_ready: false,
            restart_count: 1,
            max_restarts: 3,
        })
        .unwrap();

        match phase {
            PodPhase::Failed(failed) => {
                assert_eq!(failure_summary(&failed), "worker-1 failed (code 137): OOMKilled");
                assert!(can_restart(&failed));
            }
            other => panic!("expected Failed, got {other:?}"),
        }
    }

    #[test]
    fn illegal_specs_are_rejected_at_boundary() {
        assert!(PodPhase::from_spec(PodSpec {
            name: "bad-pending".into(),
            phase: "pending".into(),
            container_id: None,
            ip_addr: None,
            exit_code: Some(0),
            failure_reason: None,
            is_ready: true,
            restart_count: 0,
            max_restarts: 2,
        })
        .is_err());

        assert!(PodPhase::from_spec(PodSpec {
            name: "bad-failed".into(),
            phase: "failed".into(),
            container_id: Some("ctr-1".into()),
            ip_addr: None,
            exit_code: Some(0),
            failure_reason: Some("weird".into()),
            is_ready: false,
            restart_count: 0,
            max_restarts: 1,
        })
        .is_err());

        assert!(PodPhase::from_spec(PodSpec {
            name: "".into(),
            phase: "pending".into(),
            container_id: None,
            ip_addr: None,
            exit_code: None,
            failure_reason: None,
            is_ready: false,
            restart_count: 5,
            max_restarts: 2,
        })
        .is_err());
    }
}
