//! Unit 5 Starter: Unfactored product bloat and duplicated evaluation ladders.

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum PrincipalKind {
    Human,
    ServiceAccount,
    BreakGlass,
}

/// FLAW (Unit 5 - Unfactored product bloat):
/// Independent `Option` fields and contradictory booleans multiply illegal states,
/// while unfactored control flow lets branches diverge on shared invariants.
#[derive(Debug, Clone, PartialEq, Eq)]
pub struct AccessGrant {
    pub grant_id: String,
    pub scope: String,
    pub expires_at: u64,
    pub kind: PrincipalKind,
    pub principal_id: String,
    pub mfa_verified: Option<bool>,
    pub cert_fingerprint: Option<String>,
    pub approver_id: Option<String>,
    pub ticket_id: Option<String>,
    pub is_readonly: bool,
    pub allow_exec: bool,
}

#[derive(Debug, Clone, PartialEq, Eq)]
pub enum AccessDecision {
    Allow { grant_id: String, tier: &'static str },
    Deny(&'static str),
}

impl AccessGrant {
    #[allow(clippy::too_many_arguments)]
    pub fn try_new(
        grant_id: impl Into<String>,
        scope: impl Into<String>,
        expires_at: u64,
        kind: PrincipalKind,
        principal_id: impl Into<String>,
        mfa_verified: Option<bool>,
        cert_fingerprint: Option<String>,
        approver_id: Option<String>,
        ticket_id: Option<String>,
        is_readonly: bool,
        allow_exec: bool,
    ) -> Result<Self, String> {
        // FLAW: Accepts contradictory permissions and mismatched credential options.
        Ok(Self {
            grant_id: grant_id.into(),
            scope: scope.into(),
            expires_at,
            kind,
            principal_id: principal_id.into(),
            mfa_verified,
            cert_fingerprint,
            approver_id,
            ticket_id,
            is_readonly,
            allow_exec,
        })
    }
}

pub fn permission_state_space_size() -> usize {
    // `(is_readonly: bool, allow_exec: bool)` has 2 * 2 = 4 states even though 1 is illegal!
    4
}

pub fn evaluate_access(
    grant: &AccessGrant,
    required_scope: &str,
    now: u64,
) -> AccessDecision {
    // FLAW (Unit 5 - Unfactored A*B + A*C + A*D logic):
    // Duplicated scope/expiry checks across branches led to omitting expiry on BreakGlass.
    match grant.kind {
        PrincipalKind::Human => {
            if now >= grant.expires_at {
                return AccessDecision::Deny("expired");
            }
            if grant.scope != required_scope {
                return AccessDecision::Deny("scope mismatch");
            }
            if grant.allow_exec && grant.mfa_verified != Some(true) {
                return AccessDecision::Deny("mfa required for exec");
            }
            let tier = if grant.allow_exec {
                "exec"
            } else if grant.is_readonly {
                "read"
            } else {
                "write"
            };
            AccessDecision::Allow {
                grant_id: grant.grant_id.clone(),
                tier,
            }
        }
        PrincipalKind::ServiceAccount => {
            if now >= grant.expires_at {
                return AccessDecision::Deny("expired");
            }
            if grant.scope != required_scope {
                return AccessDecision::Deny("scope mismatch");
            }
            if grant.cert_fingerprint.is_none() {
                return AccessDecision::Deny("missing cert");
            }
            let tier = if grant.allow_exec {
                "exec"
            } else if grant.is_readonly {
                "read"
            } else {
                "write"
            };
            AccessDecision::Allow {
                grant_id: grant.grant_id.clone(),
                tier,
            }
        }
        PrincipalKind::BreakGlass => {
            // BUG: forgot `now >= grant.expires_at` check because envelope checks were copy-pasted!
            if grant.scope != required_scope {
                return AccessDecision::Deny("scope mismatch");
            }
            if grant.approver_id.is_none() || grant.ticket_id.is_none() {
                return AccessDecision::Deny("missing approval");
            }
            AccessDecision::Allow {
                grant_id: grant.grant_id.clone(),
                tier: "emergency-exec",
            }
        }
    }
}

#[cfg(test)]
mod baseline_tests {
    use super::*;

    #[test]
    fn valid_human_and_service_grants_work() {
        let human = AccessGrant::try_new(
            "g-1",
            "prod:db",
            200,
            PrincipalKind::Human,
            "alice",
            Some(true),
            None,
            None,
            None,
            false,
            true,
        )
        .unwrap();
        assert_eq!(
            evaluate_access(&human, "prod:db", 100),
            AccessDecision::Allow {
                grant_id: "g-1".into(),
                tier: "exec"
            }
        );

        let svc = AccessGrant::try_new(
            "g-2",
            "prod:logs",
            200,
            PrincipalKind::ServiceAccount,
            "ingester",
            None,
            Some("sha256:abcd".into()),
            None,
            None,
            true,
            false,
        )
        .unwrap();
        assert_eq!(
            evaluate_access(&svc, "prod:logs", 100),
            AccessDecision::Allow {
                grant_id: "g-2".into(),
                tier: "read"
            }
        );
    }
}

#[cfg(test)]
mod lab_goals {
    use super::*;

    #[test]
    #[ignore = "Remove #[ignore] after refactoring to verify lab goal"]
    fn expired_break_glass_grant_is_uniformly_denied() {
        let grant = AccessGrant::try_new(
            "bg-1",
            "prod:cluster",
            50,
            PrincipalKind::BreakGlass,
            "oncall",
            None,
            None,
            Some("vp-sec".into()),
            Some("INC-99".into()),
            false,
            true,
        )
        .unwrap();
        assert_eq!(
            evaluate_access(&grant, "prod:cluster", 100),
            AccessDecision::Deny("expired")
        );
    }

    #[test]
    #[ignore = "Remove #[ignore] after refactoring to verify lab goal"]
    fn contradictory_readonly_and_exec_is_rejected() {
        let bad = AccessGrant::try_new(
            "g-bad",
            "prod:db",
            200,
            PrincipalKind::Human,
            "alice",
            Some(true),
            None,
            None,
            None,
            true,
            true,
        );
        assert!(bad.is_err());
    }

    #[test]
    #[ignore = "Remove #[ignore] after refactoring to verify lab goal"]
    fn mismatched_principal_credential_fields_are_rejected() {
        let bad_svc = AccessGrant::try_new(
            "g-svc-mfa",
            "prod:db",
            200,
            PrincipalKind::ServiceAccount,
            "bot",
            Some(true),
            Some("sha256:xyz".into()),
            None,
            None,
            true,
            false,
        );
        assert!(bad_svc.is_err());
    }

    #[test]
    #[ignore = "Remove #[ignore] after refactoring to verify lab goal"]
    fn tightened_permission_algebra_has_three_states() {
        assert_eq!(permission_state_space_size(), 3);
    }
}
