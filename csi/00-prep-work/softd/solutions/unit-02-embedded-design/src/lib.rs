//! Unit 2 Reference Solution: Embedded design with typed domain models,
//! composable stages, and an opaque parametric courier.

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum Environment {
    Dev,
    Staging,
    Prod,
}

impl Environment {
    pub fn as_str(&self) -> &'static str {
        match self {
            Environment::Dev => "dev",
            Environment::Staging => "staging",
            Environment::Prod => "prod",
        }
    }
}

#[derive(Debug, Clone, PartialEq, Eq)]
pub struct DeployTarget {
    pub env: Environment,
    pub region: String,
}

impl DeployTarget {
    pub fn new(env: Environment, region: impl Into<String>) -> Result<Self, String> {
        let region = region.into();
        if region.is_empty() {
            return Err("region cannot be empty".into());
        }
        Ok(Self { env, region })
    }
}

#[derive(Debug, Clone, PartialEq, Eq)]
pub struct ReleaseArtifact {
    pub repository: String,
    pub tag: String,
}

impl ReleaseArtifact {
    pub fn new(repository: impl Into<String>, tag: impl Into<String>) -> Result<Self, String> {
        let repository = repository.into();
        let tag = tag.into();
        if repository.is_empty() || tag.is_empty() {
            return Err("repository and tag must be non-empty".into());
        }
        Ok(Self { repository, tag })
    }
}

/// Envelope separating routing destination from opaque payload `T`.
#[derive(Debug, Clone, PartialEq, Eq)]
pub struct RoutingEnvelope<T> {
    pub destination: DeployTarget,
    pub payload: T,
}

#[derive(Debug, Clone, PartialEq, Eq)]
pub struct Delivered<T> {
    pub destination: DeployTarget,
    pub payload: T,
}

/// Parametric courier: because `T` is completely unconstrained during routing,
/// `ArtifactRelay` is statically unable to inspect or pry into `T`.
#[derive(Debug, Default)]
pub struct ArtifactRelay<T> {
    delivered: Vec<Delivered<T>>,
}

impl<T> ArtifactRelay<T> {
    pub fn new() -> Self {
        Self {
            delivered: Vec::new(),
        }
    }

    pub fn dispatch(&mut self, envelope: RoutingEnvelope<T>) -> &Delivered<T> {
        self.delivered.push(Delivered {
            destination: envelope.destination,
            payload: envelope.payload,
        });
        self.delivered
            .last()
            .expect("just pushed delivered record")
    }

    pub fn delivered(&self) -> &[Delivered<T>] {
        &self.delivered
    }
}

/// Composable deployment stage: stages operate on typed context without string encoding.
pub trait PipelineStage {
    fn name(&self) -> &str;
    fn run(&self, target: &DeployTarget, artifact: &ReleaseArtifact) -> Result<String, String>;
}

pub struct NamedStage {
    name: &'static str,
}

impl NamedStage {
    pub const fn new(name: &'static str) -> Self {
        Self { name }
    }
}

impl PipelineStage for NamedStage {
    fn name(&self) -> &str {
        self.name
    }

    fn run(&self, target: &DeployTarget, artifact: &ReleaseArtifact) -> Result<String, String> {
        Ok(format!(
            "{}({}:{}:{})",
            self.name,
            target.env.as_str(),
            artifact.repository,
            artifact.tag
        ))
    }
}

pub struct DeployPipeline {
    stages: Vec<Box<dyn PipelineStage>>,
}

impl DeployPipeline {
    pub fn new() -> Self {
        Self { stages: Vec::new() }
    }

    pub fn with_stage(mut self, stage: impl PipelineStage + 'static) -> Self {
        self.stages.push(Box::new(stage));
        self
    }

    pub fn execute(
        &self,
        target: &DeployTarget,
        artifact: &ReleaseArtifact,
    ) -> Result<Vec<String>, String> {
        let mut executed = Vec::with_capacity(self.stages.len());
        for stage in &self.stages {
            executed.push(stage.run(target, artifact)?);
        }
        Ok(executed)
    }
}

impl Default for DeployPipeline {
    fn default() -> Self {
        Self::new()
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    struct CustomBlueGreenStage;
    impl PipelineStage for CustomBlueGreenStage {
        fn name(&self) -> &str {
            "blue-green"
        }

        fn run(&self, target: &DeployTarget, _artifact: &ReleaseArtifact) -> Result<String, String> {
            Ok(format!("blue-green({})", target.env.as_str()))
        }
    }

    #[test]
    fn courier_cannot_pry_into_hotfix_payload() {
        let mut relay = ArtifactRelay::new();
        let target = DeployTarget::new(Environment::Staging, "eu-west").unwrap();
        let artifact = ReleaseArtifact::new("auth/hotfix-login", "v2").unwrap();

        let delivered = relay.dispatch(RoutingEnvelope {
            destination: target.clone(),
            payload: artifact.clone(),
        });

        assert_eq!(delivered.destination, target);
        assert_eq!(delivered.payload, artifact);
    }

    #[test]
    fn delimiters_in_region_are_preserved_losslessly() {
        let mut relay = ArtifactRelay::new();
        let target = DeployTarget::new(Environment::Prod, "us-east|az:1").unwrap();
        let artifact = ReleaseArtifact::new("payments/api", "v1.5.0").unwrap();

        let delivered = relay.dispatch(RoutingEnvelope {
            destination: target,
            payload: artifact,
        });
        assert_eq!(delivered.destination.region, "us-east|az:1");
    }

    #[test]
    fn custom_stage_plugs_into_pipeline_openly() {
        let pipeline = DeployPipeline::new()
            .with_stage(NamedStage::new("build"))
            .with_stage(NamedStage::new("scan"))
            .with_stage(CustomBlueGreenStage)
            .with_stage(NamedStage::new("deploy"));

        let target = DeployTarget::new(Environment::Prod, "us-west").unwrap();
        let artifact = ReleaseArtifact::new("payments/api", "v1.6.0").unwrap();
        let trace = pipeline.execute(&target, &artifact).unwrap();

        assert!(trace.iter().any(|s| s == "blue-green(prod)"));
        assert_eq!(trace.len(), 4);
    }
}
