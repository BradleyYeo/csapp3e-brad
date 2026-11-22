//! Unit 2 Starter: Stringly-coupled release pipeline and prying artifact courier.

use std::collections::HashMap;

#[derive(Debug, Clone, PartialEq, Eq)]
pub struct DeliveryRecord {
    pub destination: String,
    pub target_env: String,
    pub target_region: String,
    pub stages_executed: Vec<String>,
    pub image_summary: String,
}

#[derive(Debug, Default)]
pub struct ArtifactRelay {
    pub delivered: Vec<DeliveryRecord>,
}

impl ArtifactRelay {
    pub fn new() -> Self {
        Self::default()
    }

    /// Dispatches a raw release map to the pipeline destination.
    pub fn dispatch(
        &mut self,
        payload: &mut HashMap<String, String>,
        custom_stages: &[&str],
    ) -> Result<DeliveryRecord, String> {
        let target_raw = payload
            .get("target")
            .cloned()
            .ok_or_else(|| "missing target".to_string())?;

        let (mut env, region) = parse_target(&target_raw)?;
        let image = payload
            .get("image")
            .cloned()
            .ok_or_else(|| "missing image".to_string())?;

        // FLAW (Unit 2 - Prying courier):
        // The relay inspects payload image contents and silently rewrites routing!
        if image.contains("hotfix-") {
            env = "prod-fasttrack".to_string();
        }

        let mut stages = Vec::new();
        // FLAW (Unit 2 - Closed hardcoded stage dispatch + stringly coupling):
        // Stages are matched by string literals rather than a composable stage abstraction.
        let mut all_stages = vec!["build", "scan"];
        if env != "prod-fasttrack" {
            all_stages.push("staging");
        }
        for stage in custom_stages {
            if *stage != "canary" {
                return Err(format!("unknown stage: {stage}"));
            }
            all_stages.push(stage);
        }
        all_stages.push("deploy");

        for stage in all_stages {
            // Simulated stage work coupled to `"repo:tag"` string layout.
            let (repo, tag) = image
                .split_once(':')
                .ok_or_else(|| "invalid image format".to_string())?;
            if repo.is_empty() || tag.is_empty() {
                return Err("empty repo or tag".to_string());
            }
            stages.push(format!("{stage}({env})"));
        }

        let record = DeliveryRecord {
            destination: format!("{env}/{region}"),
            target_env: env,
            target_region: region,
            stages_executed: stages,
            image_summary: image,
        };
        self.delivered.push(record.clone());
        Ok(record)
    }
}

fn parse_target(raw: &str) -> Result<(String, String), String> {
    let mut env = None;
    let mut region = None;
    // FLAW (Unit 2 - Format / implicit-delimiter coupling):
    // Any `|` or `:` inside `region` breaks parsing.
    for part in raw.split('|') {
        let (k, v) = part
            .split_once(':')
            .ok_or_else(|| format!("malformed target segment: {part}"))?;
        match k {
            "env" => env = Some(v.to_string()),
            "region" => region = Some(v.to_string()),
            _ => return Err(format!("unknown target key: {k}")),
        }
    }
    Ok((
        env.ok_or_else(|| "missing env".to_string())?,
        region.ok_or_else(|| "missing region".to_string())?,
    ))
}

#[cfg(test)]
mod baseline_tests {
    use super::*;

    #[test]
    fn standard_release_deploys_through_pipeline() {
        let mut relay = ArtifactRelay::new();
        let mut payload = HashMap::from([
            ("target".into(), "env:staging|region:us-east".into()),
            ("image".into(), "payments/api:v1.4.2".into()),
        ]);

        let record = relay.dispatch(&mut payload, &[]).unwrap();
        assert_eq!(record.target_env, "staging");
        assert_eq!(record.target_region, "us-east");
        assert_eq!(
            record.stages_executed,
            vec![
                "build(staging)",
                "scan(staging)",
                "staging(staging)",
                "deploy(staging)"
            ]
        );
    }
}

#[cfg(test)]
mod lab_goals {
    use super::*;

    #[test]
    #[ignore = "Remove #[ignore] after refactoring to verify lab goal"]
    fn courier_must_not_pry_into_hotfix_payload() {
        let mut relay = ArtifactRelay::new();
        let mut payload = HashMap::from([
            ("target".into(), "env:staging|region:eu-west".into()),
            ("image".into(), "auth/hotfix-login:v2".into()),
        ]);

        let record = relay.dispatch(&mut payload, &[]).unwrap();
        // The relay MUST honor the explicit destination environment without reading image names.
        assert_eq!(record.target_env, "staging");
        assert_eq!(record.destination, "staging/eu-west");
    }

    #[test]
    #[ignore = "Remove #[ignore] after refactoring to verify lab goal"]
    fn delimiters_in_region_do_not_corrupt_target() {
        let mut relay = ArtifactRelay::new();
        let mut payload = HashMap::from([
            ("target".into(), "env:prod|region:us-east|az:1".into()),
            ("image".into(), "payments/api:v1.5.0".into()),
        ]);

        let record = relay.dispatch(&mut payload, &[]).unwrap();
        assert_eq!(record.target_region, "us-east|az:1");
    }

    #[test]
    #[ignore = "Remove #[ignore] after refactoring to verify lab goal"]
    fn custom_stage_like_rollout_plugs_in_without_hardcoding() {
        let mut relay = ArtifactRelay::new();
        let mut payload = HashMap::from([
            ("target".into(), "env:prod|region:us-west".into()),
            ("image".into(), "payments/api:v1.6.0".into()),
        ]);

        // Starter hardcodes stage allow-list instead of composing trait/stage objects.
        let record = relay.dispatch(&mut payload, &["blue-green"]).unwrap();
        assert!(record.stages_executed.iter().any(|s| s.contains("blue-green")));
    }
}
