//! Unit 5 Reference Solution: Algebraically factored `GrantEnvelope` (`M * (A + B + C)`).
//!
//! Impossible credential mixes and contradictory permission flags cannot be written:
//!
//! ```compile_fail
//! use unit_05_algebraic_refactoring::*;
//! let _ = PrincipalCredential::ServiceAccount {
//!     service_name: "bot".into(),
//!     cert_fingerprint: "sha256:1".into(),
//!     mfa_verified: true,
//! };
//! ```
//!
//! ```compile_fail
//! use unit_05_algebraic_refactoring::*;
//! let _ = PermissionLevel::ReadOnlyWithExec;
//! ```

/// Exact 3-inhabitant sum type replacing `(is_readonly: bool, allow_exec: bool)`.
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum PermissionLevel {
    ReadOnly,
    ReadWrite,
    AdminExec,
}

impl PermissionLevel {
    pub const CARDINALITY: usize = 3;

    pub fn tier(self) -> &'static str {
        match self {
            PermissionLevel::ReadOnly => "read",
            PermissionLevel::ReadWrite => "write",
            PermissionLevel::AdminExec => "exec",
        }
    }
}

/// Exact sum type of valid principal credentials (zero cross-variant `Option` clutter).
#[derive(Debug, Clone, PartialEq, Eq)]
pub enum PrincipalCredential {
    Human {
        user_id: String,
        mfa_verified: bool,
    },
    ServiceAccount {
        service_name: String,
        cert_fingerprint: String,
    },
    BreakGlass {
        operator_id: String,
        approver_id: String,
        ticket_id: String,
    },
}

impl PrincipalCredential {
    pub fn validate(&self) -> Result<(), String> {
        match self {
            PrincipalCredential::Human { user_id, .. } => {
                if user_id.is_empty() {
                    return Err("human user_id cannot be empty".into());
                }
            }
            PrincipalCredential::ServiceAccount {
                service_name,
                cert_fingerprint,
            } => {
                if service_name.is_empty() || cert_fingerprint.is_empty() {
                    return Err("service account requires name and cert fingerprint".into());
                }
            }
            PrincipalCredential::BreakGlass {
                operator_id,
                approver_id,
                ticket_id,
            } => {
                if operator_id.is_empty() || approver_id.is_empty() || ticket_id.is_empty() {
                    return Err("break-glass requires operator, approver, and ticket".into());
                }
            }
        }
        Ok(())
    }
}

/// Outer product `M * (Human + ServiceAccount + BreakGlass)` factoring shared metadata
/// so expiry and scope checks exist in exactly one place.
#[derive(Debug, Clone, PartialEq, Eq)]
pub struct GrantEnvelope {
    pub grant_id: String,
    pub scope: String,
    pub expires_at: u64,
    pub permission: PermissionLevel,
    pub principal: PrincipalCredential,
}

impl GrantEnvelope {
    pub fn new(
        grant_id: impl Into<String>,
        scope: impl Into<String>,
        expires_at: u64,
        permission: PermissionLevel,
        principal: PrincipalCredential,
    ) -> Result<Self, String> {
        let grant_id = grant_id.into();
        let scope = scope.into();
        if grant_id.is_empty() || scope.is_empty() {
            return Err("grant_id and scope must be non-empty".into());
        }
        principal.validate()?;
        Ok(Self {
            grant_id,
            scope,
            expires_at,
            permission,
            principal,
        })
    }
}

#[derive(Debug, Clone, PartialEq, Eq)]
pub enum AccessDecision {
    Allow { grant_id: String, tier: &'static str },
    Deny(&'static str),
}

/// Factored evaluation: shared outer-product checks run once before dispatching on `PrincipalCredential`.
pub fn evaluate_access(
    grant: &GrantEnvelope,
    required_scope: &str,
    now: u64,
) -> AccessDecision {
    if now >= grant.expires_at {
        return AccessDecision::Deny("expired");
    }
    if grant.scope != required_scope {
        return AccessDecision::Deny("scope mismatch");
    }

    match &grant.principal {
        PrincipalCredential::Human { mfa_verified, .. } => {
            if grant.permission == PermissionLevel::AdminExec && !mfa_verified {
                return AccessDecision::Deny("mfa required for exec");
            }
            AccessDecision::Allow {
                grant_id: grant.grant_id.clone(),
                tier: grant.permission.tier(),
            }
        }
        PrincipalCredential::ServiceAccount { .. } => AccessDecision::Allow {
            grant_id: grant.grant_id.clone(),
            tier: grant.permission.tier(),
        },
        PrincipalCredential::BreakGlass { .. } => AccessDecision::Allow {
            grant_id: grant.grant_id.clone(),
            tier: match grant.permission {
                PermissionLevel::AdminExec => "emergency-exec",
                other => other.tier(),
            },
        },
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn valid_human_and_service_grants_work() {
        let human = GrantEnvelope::new(
            "g-1",
            "prod:db",
            200,
            PermissionLevel::AdminExec,
            PrincipalCredential::Human {
                user_id: "alice".into(),
                mfa_verified: true,
            },
        )
        .unwrap();
        assert_eq!(
            evaluate_access(&human, "prod:db", 100),
            AccessDecision::Allow {
                grant_id: "g-1".into(),
                tier: "exec"
            }
        );

        let svc = GrantEnvelope::new(
            "g-2",
            "prod:logs",
            200,
            PermissionLevel::ReadOnly,
            PrincipalCredential::ServiceAccount {
                service_name: "ingester".into(),
                cert_fingerprint: "sha256:abcd".into(),
            },
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

    #[test]
    fn expired_break_glass_grant_is_uniformly_denied() {
        let grant = GrantEnvelope::new(
            "bg-1",
            "prod:cluster",
            50,
            PermissionLevel::AdminExec,
            PrincipalCredential::BreakGlass {
                operator_id: "oncall".into(),
                approver_id: "vp-sec".into(),
                ticket_id: "INC-99".into(),
            },
        )
        .unwrap();
        assert_eq!(
            evaluate_access(&grant, "prod:cluster", 100),
            AccessDecision::Deny("expired")
        );
    }

    #[test]
    fn permission_cardinality_is_three() {
        assert_eq!(PermissionLevel::CARDINALITY, 3);
    }
}
