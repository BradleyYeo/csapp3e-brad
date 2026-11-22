//! Unit 4 Reference Solution: Defunctionalized `FilterExpr` AST and Parnas-encapsulated `EventIndex`.

use std::collections::{BTreeSet, HashMap};

#[derive(Debug, Clone, Copy, PartialEq, Eq, PartialOrd, Ord)]
pub enum Severity {
    Info = 1,
    Warn = 2,
    Error = 3,
    Critical = 4,
}

impl Severity {
    fn as_str(self) -> &'static str {
        match self {
            Severity::Info => "info",
            Severity::Warn => "warn",
            Severity::Error => "error",
            Severity::Critical => "critical",
        }
    }

    fn parse(s: &str) -> Result<Self, String> {
        match s {
            "info" => Ok(Severity::Info),
            "warn" => Ok(Severity::Warn),
            "error" => Ok(Severity::Error),
            "critical" => Ok(Severity::Critical),
            _ => Err(format!("unknown severity: {s}")),
        }
    }
}

#[derive(Debug, Clone, PartialEq, Eq)]
pub struct LogEvent {
    pub service: String,
    pub severity: Severity,
    pub message: String,
    pub tags: Vec<String>,
}

/// Defunctionalized predicate AST: transparent, comparable, serializable, and optimizable.
#[derive(Debug, Clone, PartialEq, Eq)]
pub enum FilterExpr {
    Always,
    Never,
    ServiceEq(String),
    MinSeverity(Severity),
    HasTag(String),
    Not(Box<FilterExpr>),
    And(Box<FilterExpr>, Box<FilterExpr>),
    Or(Box<FilterExpr>, Box<FilterExpr>),
}

impl FilterExpr {
    pub fn and(self, other: Self) -> Self {
        Self::And(Box::new(self), Box::new(other))
    }

    pub fn or(self, other: Self) -> Self {
        Self::Or(Box::new(self), Box::new(other))
    }

    pub fn not(self) -> Self {
        Self::Not(Box::new(self))
    }

    /// Central interpreter for the defunctionalized rule language.
    pub fn eval(&self, event: &LogEvent) -> bool {
        match self {
            FilterExpr::Always => true,
            FilterExpr::Never => false,
            FilterExpr::ServiceEq(svc) => &event.service == svc,
            FilterExpr::MinSeverity(min) => event.severity >= *min,
            FilterExpr::HasTag(tag) => event.tags.iter().any(|t| t == tag),
            FilterExpr::Not(inner) => !inner.eval(event),
            FilterExpr::And(lhs, rhs) => lhs.eval(event) && rhs.eval(event),
            FilterExpr::Or(lhs, rhs) => lhs.eval(event) || rhs.eval(event),
        }
    }

    pub fn referenced_services(&self) -> Vec<String> {
        let mut set = BTreeSet::new();
        self.collect_services(&mut set);
        set.into_iter().collect()
    }

    fn collect_services(&self, out: &mut BTreeSet<String>) {
        match self {
            FilterExpr::ServiceEq(s) => {
                out.insert(s.clone());
            }
            FilterExpr::Not(inner) => inner.collect_services(out),
            FilterExpr::And(lhs, rhs) | FilterExpr::Or(lhs, rhs) => {
                lhs.collect_services(out);
                rhs.collect_services(out);
            }
            FilterExpr::Always
            | FilterExpr::Never
            | FilterExpr::MinSeverity(_)
            | FilterExpr::HasTag(_) => {}
        }
    }

    /// Algebraic simplification of redundant boolean connectives.
    pub fn simplify(self) -> Self {
        match self {
            FilterExpr::Not(inner) => match inner.simplify() {
                FilterExpr::Always => FilterExpr::Never,
                FilterExpr::Never => FilterExpr::Always,
                FilterExpr::Not(double_inner) => *double_inner,
                other => FilterExpr::Not(Box::new(other)),
            },
            FilterExpr::And(lhs, rhs) => {
                let l = lhs.simplify();
                let r = rhs.simplify();
                match (l, r) {
                    (FilterExpr::Never, _) | (_, FilterExpr::Never) => FilterExpr::Never,
                    (FilterExpr::Always, x) | (x, FilterExpr::Always) => x,
                    (a, b) if a == b => a,
                    (a, b) => FilterExpr::And(Box::new(a), Box::new(b)),
                }
            }
            FilterExpr::Or(lhs, rhs) => {
                let l = lhs.simplify();
                let r = rhs.simplify();
                match (l, r) {
                    (FilterExpr::Always, _) | (_, FilterExpr::Always) => FilterExpr::Always,
                    (FilterExpr::Never, x) | (x, FilterExpr::Never) => x,
                    (a, b) if a == b => a,
                    (a, b) => FilterExpr::Or(Box::new(a), Box::new(b)),
                }
            }
            leaf => leaf,
        }
    }

    /// Serializes to an unambiguous S-expression DSL.
    pub fn serialize(&self) -> String {
        match self {
            FilterExpr::Always => "true".into(),
            FilterExpr::Never => "false".into(),
            FilterExpr::ServiceEq(s) => format!("(service {})", quote(s)),
            FilterExpr::MinSeverity(sev) => format!("(min_sev {})", sev.as_str()),
            FilterExpr::HasTag(tag) => format!("(tag {})", quote(tag)),
            FilterExpr::Not(inner) => format!("(not {})", inner.serialize()),
            FilterExpr::And(l, r) => format!("(and {} {})", l.serialize(), r.serialize()),
            FilterExpr::Or(l, r) => format!("(or {} {})", l.serialize(), r.serialize()),
        }
    }

    pub fn parse(input: &str) -> Result<Self, String> {
        let mut cursor = Parser { rest: input };
        let expr = cursor.parse_expr()?;
        cursor.skip_ws();
        if !cursor.rest.is_empty() {
            return Err(format!("trailing input: {:?}", cursor.rest));
        }
        Ok(expr)
    }

    /// Query planner helper: if this predicate can only succeed for one exact service, return it.
    fn required_service(&self) -> Option<&str> {
        match self {
            FilterExpr::ServiceEq(s) => Some(s.as_str()),
            FilterExpr::And(l, r) => l.required_service().or_else(|| r.required_service()),
            _ => None,
        }
    }
}

fn quote(s: &str) -> String {
    let mut out = String::with_capacity(s.len() + 2);
    out.push('"');
    for ch in s.chars() {
        match ch {
            '\\' => out.push_str("\\\\"),
            '"' => out.push_str("\\\""),
            c => out.push(c),
        }
    }
    out.push('"');
    out
}

struct Parser<'a> {
    rest: &'a str,
}

impl<'a> Parser<'a> {
    fn skip_ws(&mut self) {
        self.rest = self.rest.trim_start();
    }

    fn parse_expr(&mut self) -> Result<FilterExpr, String> {
        self.skip_ws();
        if self.rest.starts_with('(') {
            self.rest = &self.rest[1..];
            let op = self.parse_atom()?;
            let expr = match op.as_str() {
                "service" => FilterExpr::ServiceEq(self.parse_quoted()?),
                "tag" => FilterExpr::HasTag(self.parse_quoted()?),
                "min_sev" => {
                    let sev = self.parse_atom()?;
                    FilterExpr::MinSeverity(Severity::parse(&sev)?)
                }
                "not" => FilterExpr::Not(Box::new(self.parse_expr()?)),
                "and" => {
                    let l = self.parse_expr()?;
                    let r = self.parse_expr()?;
                    FilterExpr::And(Box::new(l), Box::new(r))
                }
                "or" => {
                    let l = self.parse_expr()?;
                    let r = self.parse_expr()?;
                    FilterExpr::Or(Box::new(l), Box::new(r))
                }
                other => return Err(format!("unknown operator: {other}")),
            };
            self.skip_ws();
            if !self.rest.starts_with(')') {
                return Err("expected ')'".into());
            }
            self.rest = &self.rest[1..];
            Ok(expr)
        } else {
            match self.parse_atom()?.as_str() {
                "true" => Ok(FilterExpr::Always),
                "false" => Ok(FilterExpr::Never),
                other => Err(format!("unexpected atom: {other}")),
            }
        }
    }

    fn parse_atom(&mut self) -> Result<String, String> {
        self.skip_ws();
        let end = self
            .rest
            .find(|c: char| c.is_whitespace() || c == ')' || c == '(')
            .unwrap_or(self.rest.len());
        if end == 0 {
            return Err("expected token".into());
        }
        let tok = self.rest[..end].to_string();
        self.rest = &self.rest[end..];
        Ok(tok)
    }

    fn parse_quoted(&mut self) -> Result<String, String> {
        self.skip_ws();
        if !self.rest.starts_with('"') {
            return Err("expected quoted string".into());
        }
        let mut chars = self.rest[1..].char_indices();
        let mut out = String::new();
        while let Some((idx, ch)) = chars.next() {
            match ch {
                '\\' => {
                    let (_, esc) = chars.next().ok_or_else(|| "unterminated escape".to_string())?;
                    out.push(esc);
                }
                '"' => {
                    self.rest = &self.rest[1 + idx + 1..];
                    return Ok(out);
                }
                c => out.push(c),
            }
        }
        Err("unterminated string literal".into())
    }
}

/// Parnas-encapsulated event index: callers cannot depend on storage layout or indexing secret.
#[derive(Debug, Default)]
pub struct EventIndex {
    events: Vec<LogEvent>,
    by_service: HashMap<String, Vec<usize>>,
}

impl EventIndex {
    pub fn new() -> Self {
        Self::default()
    }

    pub fn insert(&mut self, event: LogEvent) {
        let idx = self.events.len();
        self.by_service
            .entry(event.service.clone())
            .or_default()
            .push(idx);
        self.events.push(event);
    }

    pub fn len(&self) -> usize {
        self.events.len()
    }

    pub fn is_empty(&self) -> bool {
        self.events.is_empty()
    }

    /// Queries matching events while secretly exploiting `by_service` when `FilterExpr` constrains service.
    pub fn query(&self, rule: &FilterExpr) -> Vec<&LogEvent> {
        match rule.required_service() {
            Some(svc) => self
                .by_service
                .get(svc)
                .map(|indices| {
                    indices
                        .iter()
                        .map(|&idx| &self.events[idx])
                        .filter(|ev| rule.eval(ev))
                        .collect()
                })
                .unwrap_or_default(),
            None => self.events.iter().filter(|ev| rule.eval(ev)).collect(),
        }
    }

    pub fn query_candidate_count(&self, rule: &FilterExpr) -> usize {
        match rule.required_service() {
            Some(svc) => self.by_service.get(svc).map(|v| v.len()).unwrap_or(0),
            None => self.events.len(),
        }
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn rule_roundtrips_through_dsl_and_compares_structurally() {
        let rule = FilterExpr::ServiceEq("auth".into())
            .and(FilterExpr::MinSeverity(Severity::Error))
            .or(FilterExpr::HasTag("pager \"urgent\"".into()).not());

        let dsl = rule.serialize();
        let parsed = FilterExpr::parse(&dsl).expect("DSL must parse");
        assert_eq!(rule, parsed);
    }

    #[test]
    fn rule_inspects_referenced_services() {
        let rule = FilterExpr::ServiceEq("payments".into())
            .or(FilterExpr::ServiceEq("auth".into()))
            .and(FilterExpr::MinSeverity(Severity::Warn));
        assert_eq!(
            rule.referenced_services(),
            vec!["auth".to_string(), "payments".to_string()]
        );
    }

    #[test]
    fn rule_simplifies_double_negation_and_identities() {
        let bloated = FilterExpr::Always
            .and(FilterExpr::ServiceEq("auth".into()).not().not())
            .or(FilterExpr::Never);
        assert_eq!(bloated.simplify(), FilterExpr::ServiceEq("auth".into()));
    }

    #[test]
    fn event_index_narrows_candidates_via_private_service_index() {
        let mut index = EventIndex::new();
        for i in 0..50 {
            index.insert(LogEvent {
                service: if i == 0 { "auth".into() } else { format!("svc-{i}") },
                severity: Severity::Error,
                message: "msg".into(),
                tags: vec![],
            });
        }
        let rule = FilterExpr::ServiceEq("auth".into()).and(FilterExpr::MinSeverity(Severity::Error));
        assert_eq!(index.query(&rule).len(), 1);
        assert_eq!(index.query_candidate_count(&rule), 1);
    }
}
