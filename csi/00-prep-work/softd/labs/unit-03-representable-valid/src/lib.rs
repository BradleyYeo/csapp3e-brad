//! Unit 3 Starter: Permissive flat `Pod` struct with illegal states and defensive checks.

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

#[derive(Debug, Clone, PartialEq, Eq)]
pub struct Pod {
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

impl Pod {
    /// Boundary parser — currently permissive and fails to reject illegal states.
    pub fn from_spec(spec: PodSpec) -> Result<Self, String> {
        // FLAW (Unit 3 - Representable/Valid violation):
        // Accepts empty names, contradictory phase fields, zero exit codes on failure,
        // and `restart_count > max_restarts`.
        Ok(Self {
            name: spec.name,
            phase: spec.phase,
            container_id: spec.container_id,
            ip_addr: spec.ip_addr,
            exit_code: spec.exit_code,
            failure_reason: spec.failure_reason,
            is_ready: spec.is_ready,
            restart_count: spec.restart_count,
            max_restarts: spec.max_restarts,
        })
    }
}

/// Core routing function cluttered with defensive boilerplate because `Pod` is permissive.
pub fn route_traffic(pod: &Pod) -> Result<[u8; 4], String> {
    if pod.phase != "running" {
        return Err("pod is not running".into());
    }
    if !pod.is_ready {
        return Err("pod is not ready".into());
    }
    pod.ip_addr.ok_or_else(|| "running pod missing IP".into())
}

/// Core diagnostic function cluttered with defensive boilerplate.
pub fn failure_summary(pod: &Pod) -> Result<String, String> {
    if pod.phase != "failed" {
        return Err("pod did not fail".into());
    }
    let code = pod
        .exit_code
        .ok_or_else(|| "missing exit code".to_string())?;
    if code == 0 {
        return Err("failed pod cannot have exit code 0".into());
    }
    let reason = pod
        .failure_reason
        .as_deref()
        .ok_or_else(|| "missing failure reason".to_string())?;
    Ok(format!("{} failed (code {}): {}", pod.name, code, reason))
}

pub fn can_restart(pod: &Pod) -> Result<bool, String> {
    if pod.restart_count > pod.max_restarts {
        return Err("restart count exceeded max restarts".into());
    }
    Ok(pod.phase == "failed" && pod.restart_count < pod.max_restarts)
}

#[cfg(test)]
mod baseline_tests {
    use super::*;

    #[test]
    fn valid_ready_pod_routes_traffic() {
        let pod = Pod::from_spec(PodSpec {
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

        assert_eq!(route_traffic(&pod), Ok([10, 0, 0, 5]));
    }

    #[test]
    fn valid_failed_pod_formats_summary() {
        let pod = Pod::from_spec(PodSpec {
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

        assert_eq!(
            failure_summary(&pod),
            Ok("worker-1 failed (code 137): OOMKilled".into())
        );
        assert_eq!(can_restart(&pod), Ok(true));
    }
}

#[cfg(test)]
mod lab_goals {
    use super::*;

    #[test]
    #[ignore = "Remove #[ignore] after refactoring to verify lab goal"]
    fn illegal_pending_pod_with_ready_and_exit_code_is_rejected() {
        let res = Pod::from_spec(PodSpec {
            name: "bad-pending".into(),
            phase: "pending".into(),
            container_id: None,
            ip_addr: None,
            exit_code: Some(0),
            failure_reason: None,
            is_ready: true,
            restart_count: 0,
            max_restarts: 2,
        });
        assert!(res.is_err());
    }

    #[test]
    #[ignore = "Remove #[ignore] after refactoring to verify lab goal"]
    fn illegal_failed_pod_with_zero_exit_code_or_missing_reason_is_rejected() {
        let zero_exit = Pod::from_spec(PodSpec {
            name: "bad-failed".into(),
            phase: "failed".into(),
            container_id: Some("ctr-1".into()),
            ip_addr: None,
            exit_code: Some(0),
            failure_reason: Some("weird".into()),
            is_ready: false,
            restart_count: 0,
            max_restarts: 1,
        });
        assert!(zero_exit.is_err());
    }

    #[test]
    #[ignore = "Remove #[ignore] after refactoring to verify lab goal"]
    fn restart_count_exceeding_max_and_empty_name_are_rejected() {
        let bad_budget = Pod::from_spec(PodSpec {
            name: "".into(),
            phase: "pending".into(),
            container_id: None,
            ip_addr: None,
            exit_code: None,
            failure_reason: None,
            is_ready: false,
            restart_count: 5,
            max_restarts: 2,
        });
        assert!(bad_budget.is_err());
    }
}
