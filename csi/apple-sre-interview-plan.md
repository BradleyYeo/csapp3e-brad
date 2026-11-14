# Target Role & Baseline

- **Target Role**: [Apple Senior Site Reliability Engineer (200675997-3278)](https://jobs.apple.com/en-sg/details/200675997-3278/senior-site-reliability-engineer?team=SFTWR) — Singapore, Software & Services (B2B Team).
- **Timeline**: **3 months (13 weeks)** immediately following [Curriculum.md](Curriculum.md), from **5 Apr 2028 to 4 Jul 2028**.
- **Baseline You Bring from `Curriculum.md`**: Deep Linux/OS internals, virtual memory, networking (`epoll`, `io_uring`, `tcpdump`), storage/streaming engines (SQLite, Redis, Kafka), and performance profiling.
- **Role-Specific Gaps This Plan Closes**: End-to-end Kubernetes platform engineering, Service Mesh (Istio/Envoy/Cilium), NoSQL distributed data stores, GitOps (ArgoCD/Flux), observability (Prometheus, Grafana, Splunk-style log pipelines), B2B distributed systems design, and timed SRE coding/troubleshooting rounds.

## Weekly Cadence

- **SRE Coding & Automation (6 hrs/week)**: Timed Python/Go DSA (arrays, graphs, heaps, concurrency) + practical SRE scripting (log parsing, concurrent API polling, rate limiters).
- **Platform, Service Mesh & NoSQL Labs (8 hrs/week)**: Hands-on Kubernetes, Service Mesh (mTLS, traffic shaping, circuit breaking), NoSQL operations/tuning, ArgoCD/Flux, Prometheus/Grafana, and fault-injection drills.
- **System Design & Incident Drills (5 hrs/week)**: B2B distributed architecture whiteboarding + live Linux/K8s debugging and blameless RCA practice.
- **Mock Interviews & Behavioral Prep (2.5 hrs/week)**: Timed 45–60 min simulations and STAR story refinement.

---

# Month 1: Kubernetes, Service Mesh, GitOps & Observability (Apr 5 – Apr 30, 2028)

## Week 1: Apr 5 – Apr 9, 2028 | Kubernetes Control Plane, Scheduling & Networking
- **Platform Lab**:
  - Spin up a multi-node Kubernetes cluster (`kind`/k3s/EKS) using declarative manifests; trace `kube-apiserver` -> `etcd` -> `kube-scheduler` -> `kubelet` -> `containerd` lifecycle.
  - Map `cgroups` v2 CPU/memory limits and Linux namespaces back to your OS fundamentals; configure `NetworkPolicy`, `PodDisruptionBudget`, topology spread constraints, and HPA/VPA.
- **Coding & Design**:
  - 3 Python/Go DSA mediums (graphs/BFS/topological sort) + explain end-to-end packet flow from external load balancer -> NodePort/ClusterIP (`iptables`/IPVS/eBPF) -> Pod `veth` pair.

## Week 2: Apr 10 – Apr 16, 2028 | Service Mesh Architecture (Istio / Envoy / Cilium)
- **Platform Lab**:
  - Deploy Istio (sidecar or ambient mode) or Cilium Service Mesh; inspect Envoy xDS dynamic configuration (`istioctl proxy-config`), `VirtualService`, `DestinationRule`, and `PeerAuthentication`.
  - Enforce strict mTLS across namespaces, configure L7 traffic splitting, outlier detection, retries with per-try timeouts, and circuit breaking.
- **Coding & Design**:
  - Compare sidecar proxy vs eBPF/ambient data-plane latency and memory overhead; debug a broken mTLS / 503 `UH`/`UO` Envoy upstream failure.

## Week 3: Apr 17 – Apr 23, 2028 | GitOps Delivery (ArgoCD / Flux) & Mesh Canary Rollouts
- **Platform Lab**:
  - Stand up ArgoCD (or Flux) using the App-of-Apps / ApplicationSet pattern with declarative environment promotion.
  - Integrate Argo Rollouts with your Service Mesh for automated canary traffic shifting (5% -> 25% -> 100%) and metric-based rollback.
- **Coding & Design**:
  - Design a zero-downtime deployment pipeline for multi-region B2B microservices with automated rollback triggers.

## Week 4: Apr 24 – Apr 30, 2028 | Telemetry & Observability Stack
- **Platform Lab**:
  - Deploy Prometheus + Alertmanager + Grafana + OpenTelemetry collector + structured log aggregation (Loki/ELK/Splunk-style query drills).
  - Scrape Envoy/Service Mesh L7 metrics (`istio_requests_total`, `istio_request_duration_milliseconds`) alongside node/pod metrics; author PromQL Golden Signals and multi-window burn-rate alerts.
- **Coding & Design**:
  - Write a Python/Go exporter that exposes custom `/metrics` and propagates W3C `traceparent` headers across mesh hops.

---

# Month 2: B2B Distributed Systems, Incident Response & SRE Coding (May 1 – Jun 4, 2028)

## Week 5: May 1 – May 7, 2028 | NoSQL Internals & B2B Integration Architecture
- **System Design & NoSQL Lab**:
  - Master NoSQL architecture (Cassandra/Dynamo-style wide-column/KV and document stores like MongoDB): consistent hashing, partition/clustering keys, replication factor, tunable consistency quorums ($R + W > N$), LSM-tree compaction, tombstones, and hot-partition mitigation.
  - Design reliable external-partner integrations (manufacturers, logistics, banks, resellers): NoSQL-backed idempotency stores with TTL, outbox pattern, dead-letter queues, exponential backoff with full jitter, and Kafka backpressure.
- **Coding**:
  - Implement a thread-safe token-bucket rate limiter and an idempotent webhook ingestion worker backed by NoSQL in Python/Go.

## Week 6: May 8 – May 14, 2028 | SLIs, SLOs, Error Budgets & Multi-Region Resilience
- **System Design**:
  - Define crisp SLIs/SLOs for an internet-facing B2B API gateway and async order-processing pipeline.
  - Practice capacity planning, load shedding, circuit breakers, and active-active vs active-passive failover tradeoffs.
- **Coding**:
  - 4 timed DSA problems (sliding windows, heaps, intervals, LRU cache) under 35 minutes each.

## Week 7: May 15 – May 21, 2028 | Live Linux, Service Mesh & NoSQL Fault-Injection Drills
- **Troubleshooting Lab**:
  - Deliberately break and diagnose scenarios under a 25-minute timer: OOMKilled pods vs memory leaks, Envoy sidecar 503s / mTLS cert expiry, CoreDNS/conntrack exhaustion, NoSQL read/write timeout or compaction stall, `CrashLoopBackOff`, and high `iowait`.
  - Drill `ss`, `tcpdump`, `strace`, `lsof`, `dmesg`, `istioctl`, `kubectl describe/logs/top`, and `perf` workflows.

## Week 8: May 22 – May 28, 2028 | SRE Automation & Python/Go Coding Intensive
- **Coding & Automation**:
  - Build a concurrent health-check & synthetic-probe runner in Python (`asyncio`) or Go (`errgroup` + `context` timeouts).
  - Build a streaming log anomaly/latency-percentile analyzer handling multi-gigabyte logs with bounded memory.
  - 4 timed DSA problems focusing on concurrency, tries, and union-find.

## Week 9: May 29 – Jun 4, 2028 | End-to-End Platform Capstone & Blameless RCA (Buffer)
- **Capstone Drill**:
  - Trigger a cascading failure in your Kubernetes + GitOps + Prometheus lab (e.g., bad canary config + downstream latency spike).
  - Mitigate using runbooks, restore SLOs via GitOps revert, and write a full blameless RCA (timeline, root cause, 5 whys, action items with owners/deadlines).

---

# Month 3: Apple Interview Simulation & Calibration (Jun 5 – Jul 4, 2028)

## Week 10: Jun 5 – Jun 11, 2028 | Coding Round Simulations
- **Mocks**:
  - Complete 3–4 full 45-minute mock coding interviews (Python/Go): clean problem clarification, complexity analysis, edge-case testing, and bug-free execution.
  - Drill 2 practical SRE scripting prompts (parsing, API retry logic, concurrency).

## Week 11: Jun 12 – Jun 18, 2028 | System Design & Architecture Simulations
- **Mocks**:
  - Whiteboard 3 full 60-minute SRE system design prompts tailored to Apple B2B:
    - Global B2B supply-chain event ingestion & partner webhook delivery platform backed by NoSQL + Kafka.
    - Multi-cluster Kubernetes + Service Mesh (mTLS, canary, zero-trust) GitOps platform with zero-downtime upgrades.
    - High-cardinality metrics & log observability pipeline at scale.

## Week 12: Jun 19 – Jun 25, 2028 | Incident Leadership & Behavioral Storytelling
- **Preparation**:
  - Prepare 6–8 structured STAR stories covering: high-severity on-call incident leadership, hardest RCA you drove, building a DevOps/GitOps platform from scratch, resolving partner/cross-team reliability conflicts, and automating away toil.
  - Pair each story with concrete metrics (MTTR reduction, deployment frequency, error budget saved, p99 latency drop).

## Week 13: Jun 26 – Jul 4, 2028 | Final Calibration & Application Launch
- **Final Polish**:
  - Light review of weak areas from Weeks 10–12 mocks (no heavy cramming).
  - Run 2 final mixed mocks (1 coding + 1 troubleshooting/architecture).
  - Finalize resume highlighting measurable reliability/platform impact, secure referrals, and apply for [Apple Senior Site Reliability Engineer (200675997-3278)](https://jobs.apple.com/en-sg/details/200675997-3278/senior-site-reliability-engineer?team=SFTWR).
