//! Unit 4 Starter: Opaque closure rules and leaked storage internals.

use std::rc::Rc;

#[derive(Debug, Clone, Copy, PartialEq, Eq, PartialOrd, Ord)]
pub enum Severity {
    Info = 1,
    Warn = 2,
    Error = 3,
    Critical = 4,
}

#[derive(Debug, Clone, PartialEq, Eq)]
pub struct LogEvent {
    pub service: String,
    pub severity: Severity,
    pub message: String,
    pub tags: Vec<String>,
}

/// FLAW (Unit 4 - Code over Data):
/// Opaque closures prevent serialization, structural equality, simplification, and query planning.
#[derive(Clone)]
pub struct AlertRule {
    pred: Rc<dyn Fn(&LogEvent) -> bool>,
}

impl AlertRule {
    pub fn always() -> Self {
        Self {
            pred: Rc::new(|_| true),
        }
    }

    pub fn never() -> Self {
        Self {
            pred: Rc::new(|_| false),
        }
    }

    pub fn service_eq(service: impl Into<String>) -> Self {
        let target = service.into();
        Self {
            pred: Rc::new(move |ev| ev.service == target),
        }
    }

    pub fn min_severity(min: Severity) -> Self {
        Self {
            pred: Rc::new(move |ev| ev.severity >= min),
        }
    }

    pub fn has_tag(tag: impl Into<String>) -> Self {
        let target = tag.into();
        Self {
            pred: Rc::new(move |ev| ev.tags.iter().any(|t| t == &target)),
        }
    }

    pub fn and(self, other: Self) -> Self {
        Self {
            pred: Rc::new(move |ev| (self.pred)(ev) && (other.pred)(ev)),
        }
    }

    pub fn or(self, other: Self) -> Self {
        Self {
            pred: Rc::new(move |ev| (self.pred)(ev) || (other.pred)(ev)),
        }
    }

    pub fn not(self) -> Self {
        Self {
            pred: Rc::new(move |ev| !(self.pred)(ev)),
        }
    }

    pub fn matches(&self, event: &LogEvent) -> bool {
        (self.pred)(event)
    }

    pub fn to_dsl(&self) -> Option<String> {
        // Cannot inspect closure AST!
        None
    }

    pub fn from_dsl(_dsl: &str) -> Result<Self, String> {
        Err("opaque closure AlertRule cannot parse DSL".into())
    }

    pub fn referenced_services(&self) -> Option<Vec<String>> {
        None
    }

    pub fn simplify(self) -> Option<Self> {
        None
    }

    pub fn structural_eq(&self, _other: &Self) -> Option<bool> {
        None
    }
}

/// FLAW (Unit 4 - Violates Parnas information hiding):
/// Exposing `pub events` leaks linear storage representation to all callers.
#[derive(Default)]
pub struct EventStore {
    pub events: Vec<LogEvent>,
}

impl EventStore {
    pub fn new() -> Self {
        Self::default()
    }

    pub fn insert(&mut self, event: LogEvent) {
        self.events.push(event);
    }

    pub fn query(&self, rule: &AlertRule) -> Vec<&LogEvent> {
        self.events.iter().filter(|ev| rule.matches(ev)).collect()
    }

    pub fn query_candidate_count(&self, _rule: &AlertRule) -> usize {
        // Forced to linearly scan every event because rule is an opaque closure.
        self.events.len()
    }
}

#[cfg(test)]
mod baseline_tests {
    use super::*;

    fn sample_events() -> Vec<LogEvent> {
        vec![
            LogEvent {
                service: "auth".into(),
                severity: Severity::Error,
                message: "token expired".into(),
                tags: vec!["security".into()],
            },
            LogEvent {
                service: "billing".into(),
                severity: Severity::Info,
                message: "invoice ok".into(),
                tags: vec!["batch".into()],
            },
            LogEvent {
                service: "auth".into(),
                severity: Severity::Critical,
                message: "signing key missing".into(),
                tags: vec!["pager".into(), "security".into()],
            },
        ]
    }

    #[test]
    fn closure_rule_filters_matching_events() {
        let mut store = EventStore::new();
        for ev in sample_events() {
            store.insert(ev);
        }

        let rule = AlertRule::service_eq("auth")
            .and(AlertRule::min_severity(Severity::Error))
            .and(AlertRule::has_tag("pager"));

        let hits = store.query(&rule);
        assert_eq!(hits.len(), 1);
        assert_eq!(hits[0].message, "signing key missing");
    }
}

#[cfg(test)]
mod lab_goals {
    use super::*;

    #[test]
    #[ignore = "Remove #[ignore] after refactoring to verify lab goal"]
    fn rule_roundtrips_through_dsl_and_supports_structural_equality() {
        let rule = AlertRule::service_eq("auth")
            .and(AlertRule::min_severity(Severity::Error))
            .or(AlertRule::has_tag("pager").not());

        let dsl = rule.to_dsl().expect("rule must serialize to DSL");
        let parsed = AlertRule::from_dsl(&dsl).expect("DSL must parse back");
        assert_eq!(rule.structural_eq(&parsed), Some(true));
    }

    #[test]
    #[ignore = "Remove #[ignore] after refactoring to verify lab goal"]
    fn rule_inspects_referenced_services() {
        let rule = AlertRule::service_eq("payments")
            .or(AlertRule::service_eq("auth"))
            .and(AlertRule::min_severity(Severity::Warn));
        let services = rule
            .referenced_services()
            .expect("rule data must expose referenced services");
        assert_eq!(services, vec!["auth".to_string(), "payments".to_string()]);
    }

    #[test]
    #[ignore = "Remove #[ignore] after refactoring to verify lab goal"]
    fn rule_simplifies_double_negation_and_identities() {
        let bloated = AlertRule::always()
            .and(AlertRule::service_eq("auth").not().not())
            .or(AlertRule::never());
        let simplified = bloated.simplify().expect("rule data must simplify");
        assert_eq!(
            simplified.structural_eq(&AlertRule::service_eq("auth")),
            Some(true)
        );
    }

    #[test]
    #[ignore = "Remove #[ignore] after refactoring to verify lab goal"]
    fn event_index_narrows_candidates_via_service_index() {
        let mut store = EventStore::new();
        for i in 0..50 {
            store.insert(LogEvent {
                service: if i == 0 { "auth".into() } else { format!("svc-{i}") },
                severity: Severity::Error,
                message: "msg".into(),
                tags: vec![],
            });
        }
        let rule = AlertRule::service_eq("auth").and(AlertRule::min_severity(Severity::Error));
        assert_eq!(store.query(&rule).len(), 1);
        // Once defunctionalized + Parnas-indexed, querying "auth" scans only the 1 auth event!
        assert_eq!(store.query_candidate_count(&rule), 1);
    }
}
